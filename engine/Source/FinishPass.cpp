#include "FinishPass.h"
#include "Diagnostics.h"

using namespace juce::gl;

namespace
{
    // Dual-filter (Kawase-style) downsample: centre plus four diagonal taps
    // at half-texel offsets - wide, smooth and cheap at every mip level.
    const char* const downsampleBody = R"(
        uniform sampler2D source;
        uniform vec2 texel;
        vec3 sampleDown (vec2 p)
        {
            vec3 s = texture2D (source, p).rgb * 4.0;
            s += texture2D (source, p + vec2 (-texel.x, -texel.y)).rgb;
            s += texture2D (source, p + vec2 ( texel.x, -texel.y)).rgb;
            s += texture2D (source, p + vec2 (-texel.x,  texel.y)).rgb;
            s += texture2D (source, p + vec2 ( texel.x,  texel.y)).rgb;
            return s / 8.0;
        }
    )";

    const char* const prefilterMain = R"(
        uniform float threshold;
        void main()
        {
            vec3 c = sampleDown (uv);
            // Soft-knee threshold: only energy above `threshold` feeds the
            // bloom, with a smooth shoulder instead of a hard cut.
            float bright = max (c.r, max (c.g, c.b));
            float knee = threshold * 0.5 + 1e-4;
            float soft = clamp (bright - threshold + knee, 0.0, 2.0 * knee);
            soft = soft * soft / (4.0 * knee);
            float contribution = max (soft, bright - threshold) / max (bright, 1e-4);
            gl_FragColor = vec4 (c * contribution, 1.0);
        }
    )";

    const char* const downMain = R"(
        void main() { gl_FragColor = vec4 (sampleDown (uv), 1.0); }
    )";

    const char* const upSource = R"(
        uniform sampler2D source;
        uniform vec2 texel;
        void main()
        {
            // 8-tap tent upsample; blended additively onto the next level up.
            vec3 s = texture2D (source, uv + vec2 (-texel.x * 2.0, 0.0)).rgb;
            s += texture2D (source, uv + vec2 (-texel.x, texel.y)).rgb * 2.0;
            s += texture2D (source, uv + vec2 (0.0, texel.y * 2.0)).rgb;
            s += texture2D (source, uv + vec2 (texel.x, texel.y)).rgb * 2.0;
            s += texture2D (source, uv + vec2 (texel.x * 2.0, 0.0)).rgb;
            s += texture2D (source, uv + vec2 (texel.x, -texel.y)).rgb * 2.0;
            s += texture2D (source, uv + vec2 (0.0, -texel.y * 2.0)).rgb;
            s += texture2D (source, uv + vec2 (-texel.x, -texel.y)).rgb * 2.0;
            gl_FragColor = vec4 (s / 12.0, 1.0);
        }
    )";

    const char* const compositeSource = R"(
        uniform sampler2D scene;
        uniform sampler2D bloom;
        uniform float bloomAmount;
        uniform float exposure;
        uniform float toneMap;
        uniform float vignette;
        uniform float grain;
        uniform float time;
        uniform vec2 resolution;

        float hash (vec2 p)
        {
            p = fract (p * vec2 (443.897, 441.423));
            p += dot (p, p.yx + 19.19);
            return fract ((p.x + p.y) * p.x);
        }

        vec3 linearToSrgb (vec3 c)
        {
            c = max (c, vec3 (0.0));
            vec3 lo = c * 12.92;
            vec3 hi = 1.055 * pow (c, vec3 (1.0 / 2.4)) - 0.055;
            return mix (lo, hi, step (vec3 (0.0031308), c));
        }

        void main()
        {
            vec3 c = texture2D (scene, uv).rgb + texture2D (bloom, uv).rgb * bloomAmount;
            c *= exposure;
            if (toneMap > 0.5)
                c = c / (1.0 + c); // Reinhard (per channel), named honestly - not "ACES"

            vec2 d = (uv - 0.5) * vec2 (resolution.x / resolution.y, 1.0);
            c *= mix (1.0, smoothstep (1.15, 0.25, length (d)), vignette);

            vec3 display = linearToSrgb (c);
            // Grain in display space, scaled by output resolution so it stays fine.
            display += (hash (uv * resolution + fract (time) * 1000.0) - 0.5) * grain;
            gl_FragColor = vec4 (clamp (display, 0.0, 1.0), 1.0);
        }
    )";
}

bool FinishPass::ensurePrograms (juce::OpenGLContext& context)
{
    if (composite != nullptr)
        return true;
    if (buildFailed)
        return false;

    juce::String error;
    prefilter = buildFullscreenProgram (context, juce::String (downsampleBody) + prefilterMain, error);
    if (prefilter != nullptr) down = buildFullscreenProgram (context, juce::String (downsampleBody) + downMain, error);
    if (down != nullptr)      up = buildFullscreenProgram (context, upSource, error);
    if (up != nullptr)        composite = buildFullscreenProgram (context, compositeSource, error);

    if (composite == nullptr)
    {
        logDiagnostic ("FinishPass: shader build failed - " + error);
        buildFailed = true;
        return false;
    }
    return true;
}

bool FinishPass::render (juce::OpenGLContext& context, unsigned int sceneTexture, int width, int height,
                         const V2Post& post, float timeSeconds, const GLRenderTarget& output)
{
    if (! ensurePrograms (context))
        return false;

    const int numLevels = post.bloom ? juce::jlimit (1, (int) levels.size(), post.bloomLevels) : 0;

    auto bindTexture = [] (int unit, unsigned int tex) {
        glActiveTexture ((GLenum) (GL_TEXTURE0 + unit));
        glBindTexture (GL_TEXTURE_2D, tex);
    };

    glDisable (GL_BLEND);

    if (numLevels > 0)
    {
        int w = width, h = height;
        for (int i = 0; i < numLevels; ++i)
        {
            w = juce::jmax (1, w / 2);
            h = juce::jmax (1, h / 2);
            levels[(size_t) i].ensure (w, h, GL_RGBA16F);
        }

        // Prefilter: scene -> level 0 (half resolution).
        levels[0].bind();
        prefilter->use();
        bindTexture (0, sceneTexture);
        prefilter->setUniform ("source", 0);
        prefilter->setUniform ("texel", 1.0f / (float) width, 1.0f / (float) height);
        prefilter->setUniform ("threshold", post.bloomThreshold);
        quad.draw (*prefilter);

        for (int i = 1; i < numLevels; ++i)
        {
            auto& src = levels[(size_t) i - 1];
            levels[(size_t) i].bind();
            down->use();
            bindTexture (0, src.texture);
            down->setUniform ("source", 0);
            down->setUniform ("texel", 1.0f / (float) src.width, 1.0f / (float) src.height);
            quad.draw (*down);
        }

        // Upsample back to level 0, accumulating additively.
        glEnable (GL_BLEND);
        glBlendFunc (GL_ONE, GL_ONE);
        for (int i = numLevels - 1; i > 0; --i)
        {
            auto& src = levels[(size_t) i];
            levels[(size_t) i - 1].bind();
            up->use();
            bindTexture (0, src.texture);
            up->setUniform ("source", 0);
            up->setUniform ("texel", 0.5f / (float) src.width, 0.5f / (float) src.height);
            quad.draw (*up);
        }
        glDisable (GL_BLEND);
    }

    output.bind();
    composite->use();
    bindTexture (0, sceneTexture);
    bindTexture (1, numLevels > 0 ? levels[0].texture : 0u);
    composite->setUniform ("scene", 0);
    composite->setUniform ("bloom", 1);
    composite->setUniform ("bloomAmount", numLevels > 0 ? post.bloomAmount : 0.0f);
    composite->setUniform ("exposure", std::exp2 (post.exposureEv));
    composite->setUniform ("toneMap", post.reinhard ? 1.0f : 0.0f);
    composite->setUniform ("vignette", post.vignette);
    composite->setUniform ("grain", post.grain);
    composite->setUniform ("time", timeSeconds);
    composite->setUniform ("resolution", (float) output.width, (float) output.height);
    quad.draw (*composite);

    bindTexture (1, 0);
    bindTexture (0, 0);
    return true;
}

void FinishPass::release()
{
    for (auto& l : levels)
        l.release();
    quad.release();
    prefilter.reset();
    down.reset();
    up.reset();
    composite.reset();
    buildFailed = false;
}
