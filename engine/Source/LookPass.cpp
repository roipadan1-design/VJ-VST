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
            gl_FragColor = vec4 (max (cur, mem * retain), 1.0);
        }
    )";

    const char* const finalSource = R"(
        uniform sampler2D image;
        uniform vec2 resolution;
        uniform float time;
        uniform float gain;
        uniform float grain;
        uniform float crush;
        uniform float glitch;
        uniform float glitchSeed;
        uniform float symbols;
        uniform float symbolSeed;
        uniform float flashAmount;
        uniform float flashType;
        uniform vec3 c0;
        uniform vec3 c1;
        uniform vec3 c2;
        uniform float paletteMix;

        float hash (vec2 p)
        {
            vec3 p3 = fract (vec3 (p.xyx) * 0.1031);
            p3 += dot (p3, p3.yzx + 33.33);
            return fract ((p3.x + p3.y) * p3.z);
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

        vec3 fetch (vec2 p, float split)
        {
            return vec3 (texture2D (image, p + vec2 (split, 0.0)).r,
                         texture2D (image, p).g,
                         texture2D (image, p - vec2 (split, 0.0)).b);
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

            // dot layout: 2 columns x 3 rows inside the cell with padding
            vec2 g = (f - vec2 (0.14, 0.1)) / vec2 (0.72, 0.8);
            if (g.x < 0.0 || g.x > 1.0 || g.y < 0.0 || g.y > 1.0)
                return 0.0;
            vec2 d = floor (g * vec2 (2.0, 3.0));
            vec2 local = fract (g * vec2 (2.0, 3.0)) - 0.5;
            float on = step (0.45, hash (cell * 3.0 + d + symbolSeed));
            float r = length (local);   // sub-cells are ~square (0.26 x 0.27 of the cell height)
            float dotMask = 1.0 - smoothstep (0.26, 0.32, r);
            tone = step (0.7, hash (run * 1.3 + symbolSeed));
            return on * dotMask;
        }

        void main()
        {
            vec2 p = uv;
            float t = floor (time * 14.0) + glitchSeed * 13.0;

            // --- glitch: row tears, pixel blocks, RGB split
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
            float split = g * (0.002 + 0.01 * tear);
            vec3 col = fetch (p, split);

            // --- grade: crush the tone, then map it through the 3-colour palette
            float l = crushTone (clamp (luma (col), 0.0, 1.0));
            vec3 own = col * (l / max (luma (col), 1e-3));
            col = mix (own, gradientMap (l), paletteMix);

            // --- braille symbols on top, in the light (or mid) colour
            float tone;
            float sym = symbols > 0.001 ? braille (gl_FragCoord.xy, tone) : 0.0;
            col = mix (col, mix (c2, c1, tone), sym);

            // --- kick flash: black / mid / light frame
            vec3 flashColour = flashType < 0.5 ? vec3 (0.0) : (flashType < 1.5 ? c1 : c2);
            col = mix (col, flashColour, flashAmount);

            // --- grain (size grows with amount) + sparse dust
            float size = 1.0 + floor (grain * 2.5);
            vec2 gp = floor (gl_FragCoord.xy / size);
            float n = hash (gp + fract (time * 7.31) * 911.0) - 0.5;
            float lum = luma (col);
            col += n * grain * 0.42 * (0.35 + 2.6 * lum * (1.0 - lum));
            float dust = step (1.0 - grain * grain * 0.004, hash (floor (gl_FragCoord.xy / 3.0) + floor (time * 12.0) * 17.0));
            col = mix (col, hash (gp + 3.0) > 0.5 ? c2 : vec3 (0.0), dust);

            gl_FragColor = vec4 (clamp (col, 0.0, 1.0) * gain, 1.0);
        }
    )";
}

void LookPass::update (const Signals& signals, const LookSettings& look, double dt)
{
    lastDt = dt;
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
                auto flash = look.get (LookSettings::flash);
                if (flash > 0.001f && random.nextFloat() < flash * 0.75f)
                {
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
}

bool LookPass::ensurePrograms (juce::OpenGLContext& context)
{
    if (finalProgram != nullptr)
        return true;
    if (buildFailed)
        return false;

    juce::String error;
    trailProgram = buildFullscreenProgram (context, trailSource, error);
    if (trailProgram != nullptr)
        finalProgram = buildFullscreenProgram (context, finalSource, error);

    if (finalProgram == nullptr)
    {
        logDiagnostic ("LookPass: shader build failed - " + error);
        buildFailed = true;
        return false;
    }
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
    // lasts the same at any frame rate.
    const auto trails = look.get (LookSettings::trails);
    const auto tau = 0.04 + 1.4 * std::pow ((double) trails, 1.6);
    const auto retain = trails > 0.001f ? (float) std::exp (-lastDt / tau) : 0.0f;

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
    trailProgram->setUniform ("push", trails * 0.004f * (float) (lastDt * 60.0));
    quad.draw (*trailProgram);
    historyIndex = 1 - historyIndex;

    // Everything else, straight to the output.
    glBindFramebuffer (GL_FRAMEBUFFER, outputFbo);
    glViewport (0, 0, outputWidth, outputHeight);
    finalProgram->use();
    bindTexture (0, next.texture);
    bindTexture (1, 0);
    finalProgram->setUniform ("image", 0);
    finalProgram->setUniform ("resolution", (float) outputWidth, (float) outputHeight);
    finalProgram->setUniform ("time", timeSeconds);
    finalProgram->setUniform ("gain", gain);
    finalProgram->setUniform ("grain", look.get (LookSettings::grain));
    finalProgram->setUniform ("crush", look.get (LookSettings::crush));

    const auto glitch = look.get (LookSettings::glitch);
    finalProgram->setUniform ("glitch", juce::jlimit (0.0f, 1.0f, glitch * (0.35f + 0.45f * snareEnv + 0.3f * hatEnv + 0.25f * kickEnv)));
    finalProgram->setUniform ("glitchSeed", glitchSeed);

    const auto symbols = look.get (LookSettings::symbols);
    finalProgram->setUniform ("symbols", juce::jlimit (0.0f, 1.0f, symbols * (0.55f + 0.45f * snareEnv) + symbols * 0.2f * hatEnv));
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
    quad.draw (*finalProgram);

    bindTexture (0, 0);
    return true;
}

void LookPass::release()
{
    for (auto& h : history)
        h.release();
    quad.release();
    trailProgram.reset();
    finalProgram.reset();
    buildFailed = false;
}
