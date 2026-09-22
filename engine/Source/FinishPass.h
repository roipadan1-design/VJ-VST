#pragma once

#include <JuceHeader.h>
#include "GLHelpers.h"
#include "PresetV2.h"

// The engine-owned "finish" for schema-2 presets (REPORT F4): generators
// write linear HDR colour into a float target and never do their own output
// conversion; this pass applies, in order,
//   bloom (soft-threshold prefilter, dual-filter down/up chain at half res and below)
//   -> exposure -> Reinhard tone map -> vignette -> sRGB encode -> film grain
// and writes a display-referred image into the caller's target.
class FinishPass
{
public:
    // Returns false (and logs) if the shaders failed to build; callers then
    // fall back to a plain copy so the picture never goes black.
    bool render (juce::OpenGLContext& context, unsigned int sceneTexture, int width, int height,
                 const V2Post& post, float timeSeconds, const GLRenderTarget& output);

    void release();

private:
    bool ensurePrograms (juce::OpenGLContext& context);

    std::unique_ptr<juce::OpenGLShaderProgram> prefilter, down, up, composite;
    std::array<GLRenderTarget, 6> levels;
    FullscreenQuad quad;
    bool buildFailed = false;
};
