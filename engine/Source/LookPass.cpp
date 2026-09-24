#include "LookPass.h"
#include "Diagnostics.h"

using namespace juce::gl;

namespace
{
    const char* const trailSource = R"(
        uniform sampler2D scene;
        uniform sampler2D previous;
        uniform float retain;   // 0 = no trails
        uniform float push;     // memory slowly zooms in: the smear flows outward
        void main()
        {
            vec3 cur = texture2D (scene, uv).rgb;
            vec3 mem = texture2D (previous, 0.5 + (uv - 0.5) * (1.0 - push)).rgb;
            // The small subtraction lets fp16 memory actually reach black.
            gl_FragColor = vec4 (max (cur, mem * retain - 0.002), 1.0);
        }
    )";

    // Highlights above a threshold, downsampled to quarter res (4 taps).
    const char* const extractSource = R"(
        uniform sampler2D source;
        uniform vec2 texel;
        uniform float threshold;
        void main()
        {
            vec3 c = texture2D (source, uv + texel * vec2 (-1.0, -1.0)).rgb
                   + texture2D (source, uv + texel * vec2 ( 1.0, -1.0)).rgb
                   + texture2D (source, uv + texel * vec2 (-1.0,  1.0)).rgb
                   + texture2D (source, uv + texel * vec2 ( 1.0,  1.0)).rgb;
            c *= 0.25;
            float l = dot (c, vec3 (0.2126, 0.7152, 0.0722));
            float k = max (l - threshold, 0.0) / max (1.0 - threshold, 1e-3);
            gl_FragColor = vec4 (c * k / max (l, 1e-3), 1.0);
        }
    )";

    // 9-tap separable Gaussian using bilinear tap merging (5 fetches).
    const char* const blurSource = R"(
        uniform sampler2D source;
        uniform vec2 direction; // texel step along the blur axis
        void main()
        {
            vec3 c = texture2D (source, uv).rgb * 0.227027;
            c += texture2D (source, uv + direction * 1.384615).rgb * 0.316216;
            c += texture2D (source, uv - direction * 1.384615).rgb * 0.316216;
            c += texture2D (source, uv + direction * 3.230769).rgb * 0.070270;
            c += texture2D (source, uv - direction * 3.230769).rgb * 0.070270;
            gl_FragColor = vec4 (c, 1.0);
        }
    )";

    const char* const finalSource = R"(
        uniform sampler2D image;
        uniform sampler2D halo;
        uniform sampler2D grainTex;   // 256^2 white noise, 4 independent channels, LINEAR + REPEAT
        uniform sampler2D blueTex;    // 64^2 blue noise, REPEAT
        uniform vec2 resolution;
        uniform float time;
        uniform float gain;
        uniform float grain;
        uniform float crush;
        uniform float glitch;
        uniform float glitchSeed;
        uniform float smear;
        uniform float symbols;
        uniform float symbolSeed;
        uniform float flashAmount;
        uniform float flashType;
        uniform float halation;
        uniform float weave;
        uniform float dust;
        uniform float blacks;
        uniform float filmFrame;      // 24 fps frame counter (grain, dust, flicker clock)
        uniform vec2 weaveOffset;     // px
        uniform float flicker;
        uniform vec4 scratchA;        // x (px), width (px), visibility, seed
        uniform vec4 scratchB;
        uniform vec4 hairA;           // x, y (px), angle, length (px)
        uniform vec2 hairB;           // seed, visibility
        uniform vec3 c0;
        uniform vec3 c1;
        uniform vec3 c2;
        uniform float paletteMix;
        uniform float duo;            // 1 = two-layer scene: red = body, green = detail

        float hash (vec2 p)
        {
            vec3 p3 = fract (vec3 (p.xyx) * 0.1031);
            p3 += dot (p3, p3.yzx + 33.33);
            return fract ((p3.x + p3.y) * p3.z);
        }

        float vnoise (float x)
        {
            float i = floor (x), f = fract (x);
            return mix (hash (vec2 (i, 7.1)), hash (vec2 (i + 1.0, 7.1)), f * f * (3.0 - 2.0 * f));
        }

        float luma (vec3 c) { return dot (c, vec3 (0.2126, 0.7152, 0.0722)); }

        // Soft S-curve at low crush, stepped posterise in the middle, a hard
        // two-tone threshold at the top.
        float crushTone (float l)
        {
            float k = crush;
            float w = mix (0.5, 0.015, k * k);
            float s = smoothstep (0.5 - w, 0.5 + w, l);
            float curved = mix (l, s, smoothstep (0.0, 0.5, k));
            float levels = floor (mix (7.0, 2.0, smoothstep (0.45, 1.0, k)));
            float stepped = floor (curved * levels * 0.999) / max (levels - 1.0, 1.0);
            return mix (curved, stepped, smoothstep (0.35, 0.75, k));
        }

        vec3 gradientMap (float t)
        {
            return t < 0.5 ? mix (c0, c1, t * 2.0) : mix (c1, c2, t * 2.0 - 1.0);
        }

        // Braille strips: runs of 2x3 dot cells on a grid, re-dealt by symbolSeed.
        float braille (vec2 px, out float tone)
        {
            tone = 0.0;
            float cellH = resolution.y / 46.0;
            vec2 cellSize = vec2 (cellH * 0.72, cellH);
            vec2 cell = floor (px / cellSize);
            vec2 f = fract (px / cellSize);

            float rowSeed = hash (vec2 (cell.y, symbolSeed * 7.13));
            float runLen = floor (3.0 + rowSeed * 9.0);
            vec2 run = vec2 (floor ((cell.x + rowSeed * 37.0) / runLen), cell.y);
            float density = symbols * symbols * 0.55;
            if (hash (run + symbolSeed * 3.1) > density)
                return 0.0;
            if (hash (cell + symbolSeed * 1.7) > 0.88)
                return 0.0;

            vec2 g = (f - vec2 (0.14, 0.1)) / vec2 (0.72, 0.8);
            if (g.x < 0.0 || g.x > 1.0 || g.y < 0.0 || g.y > 1.0)
                return 0.0;
            vec2 d = floor (g * vec2 (2.0, 3.0));
            vec2 local = fract (g * vec2 (2.0, 3.0)) - 0.5;
            float on = step (0.45, hash (cell * 3.0 + d + symbolSeed));
            float dotMask = 1.0 - smoothstep (0.26, 0.32, length (local));
            tone = step (0.7, hash (run * 1.3 + symbolSeed));
            return on * dotMask;
        }

        // Band-limited film grain: bilinear-filtered noise tile at the grain
        // cell size plus a finer octave; a new phase every film frame (24 fps).
        vec4 filmGrain (vec2 px)
        {
            float scale = resolution.y / 1080.0;
            float cell = (1.25 + 0.75 * grain) * scale;
            vec2 phase = fract (filmFrame * vec2 (0.6180339887, 0.7548776662));
            vec4 a = texture2D (grainTex, px / (256.0 * cell) + phase);
            vec4 b = texture2D (grainTex, px / (128.0 * cell) + phase.yx);
            // bilinear filtering shrinks the spread; rescale to ~unit sigma
            return ((a - 0.5) + 0.45 * (b - 0.5)) * 4.2;
        }

        float lineMask (float d, float width)
        {
            return 1.0 - smoothstep (width * 0.5, width * 0.5 + 1.0, abs (d));
        }

        void main()
        {
            vec2 px = gl_FragCoord.xy;
            float scale = resolution.y / 1080.0;

            // --- gate weave: the whole frame drifts a pixel or so, stepped at 24 fps
            vec2 p = uv + weaveOffset / resolution;

            // --- smear: VHS tracking drift on rows, in bursts, with dropout dashes
            float srow = floor (px.y / (2.0 * scale));
            float burst = smoothstep (0.45, 0.8, vnoise (time * 0.9 + 3.0));
            float drift = (vnoise (srow * 0.045 + filmFrame * 0.37) - 0.5) * 2.0;
            float band = smoothstep (0.55, 0.9, vnoise (srow * 0.011 + time * 0.7));
            p.x += drift * smear * 38.0 * scale / resolution.x * (0.25 + 0.75 * burst) * (0.3 + band);

            // --- glitch: row tears, pixel blocks, RGB split
            float t = floor (time * 14.0) + glitchSeed * 13.0;
            float g = glitch;
            float bands = floor (mix (10.0, 70.0, hash (vec2 (t, 1.3))));
            float row = floor (p.y * bands);
            float tear = step (1.0 - g * 0.55, hash (vec2 (row, t)));
            p.x += tear * (hash (vec2 (row, t + 5.0)) - 0.5) * 0.35 * g;
            vec2 block = floor (p * vec2 (16.0, 9.0));
            if (hash (block + t * 0.37) > 1.0 - g * 0.22)
            {
                vec2 coarse = resolution / mix (6.0, 24.0, hash (block + 2.0));
                p = (floor (p * coarse) + 0.5) / coarse;
            }

            // --- fetch with soft colour fringing (stronger at the frame edges)
            vec2 fromCentre = p - 0.5;
            vec2 fringe = fromCentre * (0.0012 + 0.0022 * weave) * (0.4 + 1.6 * dot (fromCentre, fromCentre) * 4.0);
            float split = g * (0.002 + 0.01 * tear);
            vec3 col = vec3 (texture2D (image, p + fringe + vec2 (split, 0.0)).r,
                             texture2D (image, p).g,
                             texture2D (image, p - fringe - vec2 (split, 0.0)).b);

            // --- acutance: a small unsharp mask, so grain sits *in* the image
            vec2 o = 1.5 / resolution;
            vec3 soft = (texture2D (image, p + vec2 (o.x, 0.0)).rgb + texture2D (image, p - vec2 (o.x, 0.0)).rgb
                       + texture2D (image, p + vec2 (0.0, o.y)).rgb + texture2D (image, p - vec2 (0.0, o.y)).rgb) * 0.25;
            col = max (col + (col - soft) * (0.12 + 0.3 * grain), 0.0);

            // --- grain on luminance, before the threshold (dithers the Crush edge)
            float lum = clamp (luma (col), 0.0, 1.0);
            vec4 n = filmGrain (px);
            float response = 0.25 + 0.75 * pow (clamp (4.0 * lum * (1.0 - lum), 0.0, 1.0), 0.7);
            float sigma = grain * 0.1;
            float l = lum + n.r * sigma * response;
            l *= 1.0 + flicker;

            // --- crush + palette
            float lt = crushTone (clamp (l, 0.0, 1.0));
            vec3 own = col * (lt / max (lum, 1e-3)) + n.gba * sigma * 0.2 * response; // some colour grain when the palette is off
            vec3 mapped = gradientMap (lt);
            if (duo > 0.5)
            {
                // Two layers, each with its own grain and crush (so Grain +
                // Crush break them into independent 1-bit noise); body takes
                // the mid colour, detail screens the light colour on top.
                float body = crushTone (clamp ((col.r + n.r * sigma) * (1.0 + flicker), 0.0, 1.0));
                float detail = crushTone (clamp ((col.g + n.g * sigma) * (1.0 + flicker), 0.0, 1.0));
                own = vec3 (body, detail, detail);                     // scene colours: red and cyan
                mapped = 1.0 - (1.0 - mix (c0, c1, body)) * (1.0 - c2 * detail);
            }
            col = mix (own, mapped, paletteMix);

            // --- lifted, tinted blacks (the film base)
            vec3 base = (c1 * 0.35 + vec3 (0.02)) * blacks * 0.12;
            col = base + col * (1.0 - base);

            // --- halation: red-biased glow out of the highlights, after the palette
            vec3 h = texture2D (halo, uv).rgb;
            vec3 tint = mix (vec3 (1.0, 0.3, 0.14), c2, 0.35 * paletteMix);
            vec3 glow = tint * luma (h) * halation * 1.8;
            col = 1.0 - (1.0 - col) * (1.0 - clamp (glow, 0.0, 1.0));

            // --- braille symbols on top, in the light (or mid) colour
            float tone;
            float sym = symbols > 0.001 ? braille (px, tone) : 0.0;
            col = mix (col, mix (c2, c1, tone), sym);

            // --- dust: a few specks per film frame, mostly dark, some light
            if (dust > 0.001)
            {
                float cellPx = 22.0 * scale;
                vec2 cid = floor (px / cellPx);
                float h1 = hash (cid + filmFrame * 17.13);
                if (h1 < dust * dust * 0.006)
                {
                    vec2 at = (cid + vec2 (hash (cid + filmFrame), hash (cid - filmFrame))) * cellPx;
                    float r = (1.0 + 2.5 * hash (cid * 1.7 + filmFrame)) * scale;
                    float speck = 1.0 - smoothstep (r * 0.6, r, length ((px - at) * vec2 (1.0, 0.8 + 0.6 * hash (cid))));
                    col = mix (col, h1 < dust * dust * 0.0015 ? c2 : c0 * 0.3, speck * 0.85);
                }

                // hair: a thin wavy strand, dark
                if (hairB.y > 0.0)
                {
                    vec2 d = px - hairA.xy;
                    vec2 dir = vec2 (cos (hairA.z), sin (hairA.z));
                    float along = dot (d, dir);
                    float across = dot (d, vec2 (-dir.y, dir.x));
                    float wave = sin (along * 0.05 + hairB.x * 6.28) * 6.0 * scale + sin (along * 0.013 + hairB.x) * 10.0 * scale;
                    float on = step (0.0, along) * step (along, hairA.w);
                    col = mix (col, c0 * 0.2, on * lineMask (across - wave, 1.2 * scale) * hairB.y * 0.8);
                }

                // scratches: vertical, light, intermittent along their length
                vec4 sc[2];
                sc[0] = scratchA;
                sc[1] = scratchB;
                for (int i = 0; i < 2; ++i)
                {
                    if (sc[i].z <= 0.0)
                        continue;
                    float x = sc[i].x + sin (px.y * 0.004 + sc[i].w * 9.0) * 3.0 * scale;
                    float broken = step (0.3, vnoise (px.y * 0.02 + sc[i].w * 50.0 + filmFrame * 0.2));
                    col = mix (col, c2, lineMask (px.x - x, sc[i].y * scale) * broken * sc[i].z * 0.6);
                }
            }

            // --- smear dropouts: sparse light dashes on single rows during bursts
            float dash = step (1.0 - smear * 0.006 * burst, hash (vec2 (floor (px.x / (46.0 * scale)), srow + filmFrame * 3.1)));
            col = mix (col, c2, dash * 0.65);

            // --- kick flash: black / mid / light frame
            vec3 flashColour = flashType < 0.5 ? vec3 (0.0) : (flashType < 1.5 ? c1 : c2);
            col = mix (col, flashColour, flashAmount);

            // --- triangular blue-noise dither (+-1 LSB): no banding in lifted blacks
            vec2 bp = px / 64.0;
            float d1 = texture2D (blueTex, bp + fract (filmFrame * vec2 (0.6180339887, 0.7548776662))).r;
            float d2 = texture2D (blueTex, bp + fract (filmFrame * vec2 (0.7548776662, 0.5698402910)) + 0.37).r;
            col += (d1 + d2 - 1.0) / 255.0;

            gl_FragColor = vec4 (clamp (col, 0.0, 1.0) * gain, 1.0);
        }
    )";

    // 1D value noise for the CPU-side film clock (weave, flicker).
    float valueNoise (double x, int salt)
    {
        auto h = [salt] (int i) {
            auto v = (uint32_t) (i * 374761393 + salt * 668265263);
            v = (v ^ (v >> 13)) * 1274126177u;
            return (float) ((v ^ (v >> 16)) & 0xffffff) / (float) 0xffffff;
        };
        auto i = (int) std::floor (x);
        auto f = (float) (x - std::floor (x));
        f = f * f * (3.0f - 2.0f * f);
        return h (i) + (h (i + 1) - h (i)) * f;
    }

    // Blue noise by void-and-cluster (Ulichney), generated once at start-up:
    // 64x64 ranks -> 0-255. ~16M multiply-adds, a few milliseconds.
    std::vector<uint8_t> makeBlueNoise (int size)
    {
        const int n = size * size;
        std::vector<float> kernel ((size_t) n);
        const float sigma = 1.5f;
        for (int y = 0; y < size; ++y)
            for (int x = 0; x < size; ++x)
            {
                auto dx = (float) juce::jmin (x, size - x), dy = (float) juce::jmin (y, size - y);
                kernel[(size_t) (y * size + x)] = std::exp (-(dx * dx + dy * dy) / (2.0f * sigma * sigma));
            }

        std::vector<float> energy ((size_t) n, 0.0f);
        std::vector<uint8_t> on ((size_t) n, 0);
        auto apply = [&] (int idx, float sign) {
            const int ix = idx % size, iy = idx / size;
            for (int y = 0; y < size; ++y)
                for (int x = 0; x < size; ++x)
                    energy[(size_t) (y * size + x)] += sign * kernel[(size_t) (((y - iy + size) % size) * size + ((x - ix + size) % size))];
        };
        auto extreme = [&] (bool wantOn, bool largest) {
            int best = -1;
            for (int i = 0; i < n; ++i)
                if ((bool) on[(size_t) i] == wantOn
                    && (best < 0 || (largest ? energy[(size_t) i] > energy[(size_t) best] : energy[(size_t) i] < energy[(size_t) best])))
                    best = i;
            return best;
        };

        // Initial pattern: 10 % random points, relaxed until stable.
        juce::Random rng (1234);
        const int initial = n / 10;
        for (int placed = 0; placed < initial;)
        {
            auto i = rng.nextInt (n);
            if (! on[(size_t) i]) { on[(size_t) i] = 1; apply (i, 1.0f); ++placed; }
        }
        for (int guard = 0; guard < n; ++guard)
        {
            auto cluster = extreme (true, true);
            on[(size_t) cluster] = 0; apply (cluster, -1.0f);
            auto voidIdx = extreme (false, false);
            on[(size_t) voidIdx] = 1; apply (voidIdx, 1.0f);
            if (voidIdx == cluster)
                break;
        }

        std::vector<int> rank ((size_t) n, 0);
        auto prototype = on;
        auto protoEnergy = energy;
        // Phase 1: remove tightest clusters from the prototype, ranks descending.
        for (int r = initial - 1; r >= 0; --r)
        {
            auto cluster = extreme (true, true);
            on[(size_t) cluster] = 0; apply (cluster, -1.0f);
            rank[(size_t) cluster] = r;
        }
        on = prototype;
        energy = protoEnergy;
        // Phase 2+3: fill the largest voids, ranks ascending.
        for (int r = initial; r < n; ++r)
        {
            auto voidIdx = extreme (false, false);
            on[(size_t) voidIdx] = 1; apply (voidIdx, 1.0f);
            rank[(size_t) voidIdx] = r;
        }

        std::vector<uint8_t> out ((size_t) n * 4);
        for (int i = 0; i < n; ++i)
        {
            auto v = (uint8_t) ((rank[(size_t) i] * 256) / n);
            for (int c = 0; c < 4; ++c)
                out[(size_t) i * 4 + (size_t) c] = v;
        }
        return out;
    }

    unsigned int uploadNoise (const uint8_t* data, int size, bool linear)
    {
        unsigned int tex = 0;
        glGenTextures (1, &tex);
        glBindTexture (GL_TEXTURE_2D, tex);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture (GL_TEXTURE_2D, 0);
        return tex;
    }
}

void LookPass::update (const Signals& signals, const LookSettings& look, double dt, double timeSeconds)
{
    lastDt = dt;
    reactAmount = signals.react.amount;
    const auto decay = [dt] (float v, double seconds) { return v * (float) std::exp (-dt / seconds); };
    kickEnv = decay (kickEnv, 0.18);
    snareEnv = decay (snareEnv, 0.22);
    hatEnv = decay (hatEnv, 0.08);
    flashTimer = juce::jmax (0.0f, flashTimer - (float) dt);

    for (auto& e : signals.events)
    {
        switch (e.type)
        {
            case EventType::kick:
            case EventType::userTrigger:
            {
                kickEnv = 1.0f;
                // Flash: scaled by Reactivity/Calm and limited to 3 per second
                // (photosensitivity guideline) whatever the tempo.
                auto flash = look.get (LookSettings::flash) * (e.type == EventType::userTrigger ? 1.0f : reactAmount);
                if (flash > 0.001f && timeSeconds - lastFlashTime > 0.34 && random.nextFloat() < flash * 0.75f)
                {
                    lastFlashTime = timeSeconds;
                    flashTimer = 0.045f + flash * 0.05f;
                    auto r = random.nextFloat();
                    flashType = r < 0.5f ? 0.0f : (r < 0.8f ? 1.0f : 2.0f);
                }
                break;
            }
            case EventType::snare:
                snareEnv = 1.0f;
                symbolSeed = (float) random.nextInt (997) + 1.0f;
                glitchSeed = (float) random.nextInt (997) + 1.0f;
                break;
            case EventType::hat:
                hatEnv = 1.0f;
                break;
            default:
                break;
        }
    }

    // Film clock: weave, flicker, slips and artefacts step at 24 fps.
    filmTime += dt;
    const auto frame = (int) std::floor (filmTime * 24.0);
    if (frame == filmFrame)
        return;
    const int steps = juce::jlimit (1, 6, frame - filmFrame);
    filmFrame = frame;
    const auto frameDt = steps / 24.0f;

    const auto weave = look.get (LookSettings::weave);
    const auto ft = filmTime;
    weaveOffset = { (valueNoise (ft * 1.1, 1) - 0.5f) * 2.0f * weave * 1.4f + (random.nextFloat() - 0.5f) * weave * 0.5f,
                    (valueNoise (ft * 0.8, 2) - 0.5f) * 2.0f * weave * 1.1f + (random.nextFloat() - 0.5f) * weave * 0.5f };
    if (slipFrames > 0.0f)
    {
        weaveOffset.y += 6.0f * weave;
        slipFrames -= (float) steps;
    }
    else if (weave > 0.25f && random.nextFloat() < frameDt / 35.0f)
        slipFrames = 2.0f;
    flicker = weave * (0.035f * (random.nextFloat() - 0.5f) + 0.018f * (float) std::sin (ft * juce::MathConstants<double>::twoPi * 0.45));

    const auto dust = look.get (LookSettings::dust);
    for (auto& s : scratches)
    {
        if (s.life > 0.0f)
        {
            s.age += frameDt;
            s.x += s.drift * (float) steps;
            if (s.age > s.life)
                s.life = 0.0f;
        }
        else if (dust > 0.05f && random.nextFloat() < frameDt * dust * (0.3f + signals.build) / 10.0f) // scratches come with build-ups
            s = { random.nextFloat(), (random.nextFloat() - 0.5f) * 0.0006f, 2.0f + 8.0f * random.nextFloat(), 0.0f,
                  1.0f + random.nextFloat() };
    }
    if (hair.life > 0.0f)
    {
        hair.age += frameDt;
        hair.x += (random.nextFloat() - 0.5f) * 0.002f;
        if (hair.age > hair.life)
            hair.life = 0.0f;
    }
    else if (dust > 0.05f && random.nextFloat() < frameDt * dust / 10.0f)
        hair = { random.nextFloat(), random.nextFloat(), random.nextFloat() * juce::MathConstants<float>::twoPi,
                 60.0f + 120.0f * random.nextFloat(), random.nextFloat(), 0.5f + 2.5f * random.nextFloat(), 0.0f };
}

void LookPass::ensureNoiseTextures()
{
    if (grainTexture != 0)
        return;

    std::vector<uint8_t> white (256 * 256 * 4);
    juce::Random rng (99);
    for (auto& v : white)
        v = (uint8_t) rng.nextInt (256);
    grainTexture = uploadNoise (white.data(), 256, true);

    auto blue = makeBlueNoise (64);
    blueTexture = uploadNoise (blue.data(), 64, false);
}

bool LookPass::ensurePrograms (juce::OpenGLContext& context)
{
    if (finalProgram != nullptr)
        return true;
    if (buildFailed)
        return false;

    juce::String error;
    trailProgram = buildFullscreenProgram (context, trailSource, error);
    if (trailProgram != nullptr)   extractProgram = buildFullscreenProgram (context, extractSource, error);
    if (extractProgram != nullptr) blurProgram = buildFullscreenProgram (context, blurSource, error);
    if (blurProgram != nullptr)    finalProgram = buildFullscreenProgram (context, finalSource, error);

    if (finalProgram == nullptr)
    {
        logDiagnostic ("LookPass: shader build failed - " + error);
        buildFailed = true;
        return false;
    }
    ensureNoiseTextures();
    return true;
}

bool LookPass::render (juce::OpenGLContext& context, unsigned int sceneTexture, int sceneWidth, int sceneHeight,
                       unsigned int outputFbo, int outputWidth, int outputHeight,
                       const LookSettings& look, float timeSeconds, float gain)
{
    if (! ensurePrograms (context))
        return false;

    auto bindTexture = [] (int unit, unsigned int tex) {
        glActiveTexture ((GLenum) (GL_TEXTURE0 + unit));
        glBindTexture (GL_TEXTURE_2D, tex);
    };

    glDisable (GL_BLEND);

    // Trails: max (scene, memory * retain), time-constant based so the smear
    // lasts the same at any frame rate. The tail is capped (~0.5 s): a longer
    // max-memory saturates into a flat wash once Crush pushes it to full tone.
    const auto trails = look.get (LookSettings::trails);
    const auto tau = 0.03 + 0.45 * std::pow ((double) trails, 1.6);
    auto retain = trails > 0.001f ? (float) std::exp (-lastDt / tau) : 0.0f;
    if (std::exchange (trailsCleared, false))
        retain = 0.0f;

    auto& prev = history[(size_t) historyIndex];
    auto& next = history[(size_t) (1 - historyIndex)];
    prev.ensure (sceneWidth, sceneHeight, GL_RGBA16F);
    next.ensure (sceneWidth, sceneHeight, GL_RGBA16F);

    next.bind();
    trailProgram->use();
    bindTexture (0, sceneTexture);
    bindTexture (1, prev.texture);
    trailProgram->setUniform ("scene", 0);
    trailProgram->setUniform ("previous", 1);
    trailProgram->setUniform ("retain", retain);
    trailProgram->setUniform ("push", retain > 0.0f ? trails * 0.004f * (float) (lastDt * 60.0) : 0.0f);
    quad.draw (*trailProgram);
    historyIndex = 1 - historyIndex;

    // Halation: highlights at quarter res, blurred twice (H, V).
    const auto halation = look.get (LookSettings::halation);
    const int qw = juce::jmax (8, sceneWidth / 4), qh = juce::jmax (8, sceneHeight / 4);
    for (auto& h : halo)
        h.ensure (qw, qh, GL_RGBA16F);

    halo[0].bind();
    extractProgram->use();
    bindTexture (0, next.texture);
    extractProgram->setUniform ("source", 0);
    extractProgram->setUniform ("texel", 1.0f / (float) sceneWidth, 1.0f / (float) sceneHeight);
    extractProgram->setUniform ("threshold", 0.62f);
    quad.draw (*extractProgram);

    blurProgram->use();
    blurProgram->setUniform ("source", 0);
    for (int pass = 0; pass < 2; ++pass)
    {
        const float spread = pass == 0 ? 1.0f : 2.2f; // second pass reaches wider
        halo[1].bind();
        bindTexture (0, halo[0].texture);
        blurProgram->setUniform ("direction", spread / (float) qw, 0.0f);
        quad.draw (*blurProgram);
        halo[0].bind();
        bindTexture (0, halo[1].texture);
        blurProgram->setUniform ("direction", 0.0f, spread / (float) qh);
        quad.draw (*blurProgram);
    }

    // Film composite, straight to the output.
    glBindFramebuffer (GL_FRAMEBUFFER, outputFbo);
    glViewport (0, 0, outputWidth, outputHeight);
    finalProgram->use();
    bindTexture (0, next.texture);
    bindTexture (1, halo[0].texture);
    bindTexture (2, grainTexture);
    bindTexture (3, blueTexture);
    finalProgram->setUniform ("image", 0);
    finalProgram->setUniform ("halo", 1);
    finalProgram->setUniform ("grainTex", 2);
    finalProgram->setUniform ("blueTex", 3);
    finalProgram->setUniform ("resolution", (float) outputWidth, (float) outputHeight);
    finalProgram->setUniform ("time", timeSeconds);
    finalProgram->setUniform ("gain", gain);
    finalProgram->setUniform ("grain", look.get (LookSettings::grain));
    finalProgram->setUniform ("crush", look.get (LookSettings::crush));
    finalProgram->setUniform ("smear", look.get (LookSettings::smear));
    finalProgram->setUniform ("halation", halation);
    finalProgram->setUniform ("weave", look.get (LookSettings::weave));
    finalProgram->setUniform ("dust", look.get (LookSettings::dust));
    finalProgram->setUniform ("blacks", look.get (LookSettings::blacks));
    finalProgram->setUniform ("filmFrame", (float) (filmFrame % 100000));

    const auto px = (float) outputHeight / 1080.0f;
    finalProgram->setUniform ("weaveOffset", weaveOffset.x * px, weaveOffset.y * px);
    finalProgram->setUniform ("flicker", flicker);

    auto scratchUniform = [&] (const char* name, const Scratch& s) {
        const auto fade = s.life > 0.0f ? juce::jmin (1.0f, s.age * 3.0f, (s.life - s.age) * 3.0f) : 0.0f;
        finalProgram->setUniform (name, s.x * (float) outputWidth, s.width, fade, s.drift * 1000.0f + s.life);
    };
    scratchUniform ("scratchA", scratches[0]);
    scratchUniform ("scratchB", scratches[1]);
    const auto hairFade = hair.life > 0.0f ? juce::jmin (1.0f, hair.age * 4.0f, (hair.life - hair.age) * 4.0f) : 0.0f;
    finalProgram->setUniform ("hairA", hair.x * (float) outputWidth, hair.y * (float) outputHeight, hair.angle, hair.length * px);
    finalProgram->setUniform ("hairB", hair.seed, hairFade);

    const auto hits = reactAmount;
    const auto glitch = look.get (LookSettings::glitch);
    finalProgram->setUniform ("glitch", juce::jlimit (0.0f, 1.0f, glitch * (0.35f + hits * (0.45f * snareEnv + 0.3f * hatEnv + 0.25f * kickEnv))));
    finalProgram->setUniform ("glitchSeed", glitchSeed);

    const auto symbols = look.get (LookSettings::symbols);
    finalProgram->setUniform ("symbols", juce::jlimit (0.0f, 1.0f, symbols * (0.55f + hits * 0.45f * snareEnv) + symbols * 0.2f * hits * hatEnv));
    finalProgram->setUniform ("symbolSeed", symbolSeed);

    finalProgram->setUniform ("flashAmount", flashTimer > 0.0f ? 1.0f : 0.0f);
    finalProgram->setUniform ("flashType", flashType);

    const char* names[] = { "c0", "c1", "c2" };
    for (int i = 0; i < 3; ++i)
    {
        auto c = look.palette[(size_t) i];
        finalProgram->setUniform (names[i], c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue());
    }
    finalProgram->setUniform ("paletteMix", look.paletteMix);
    finalProgram->setUniform ("duo", look.duo);
    quad.draw (*finalProgram);

    for (int unit = 3; unit >= 0; --unit)
        bindTexture (unit, 0);
    return true;
}

void LookPass::release()
{
    for (auto& h : history)
        h.release();
    for (auto& h : halo)
        h.release();
    if (grainTexture != 0) glDeleteTextures (1, &grainTexture);
    if (blueTexture != 0)  glDeleteTextures (1, &blueTexture);
    grainTexture = blueTexture = 0;
    quad.release();
    trailProgram.reset();
    extractProgram.reset();
    blurProgram.reset();
    finalProgram.reset();
    buildFailed = false;
}
