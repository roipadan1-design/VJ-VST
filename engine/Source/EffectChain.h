#pragma once

#include <JuceHeader.h>
#include "ISFShader.h"
#include "Preset.h"

// Chains an ordered list of ISF effect shaders (each expected to declare a
// standard "inputImage" sampler2D input) over a single source image,
// ping-ponging between two owned offscreen FBOs so each stage's output
// feeds the next stage's inputImage. The last non-bypassed stage renders
// straight to the caller-supplied target (normally the screen).
//
// Built for layered, individually toggleable video-glitch presets: stage 0's
// inputImage is bound to an externally supplied texture (a VideoPlayer frame,
// not one of this chain's own buffers).
//
// Known scope limit (documented, not hidden): each stage is treated as a
// single-pass effect - a stage shader that itself declares ISF PASSES for
// internal multi-pass feedback would still run its own passes correctly,
// but only its final pass takes part in the chain (this mirrors ISFShader's
// own "PASSES always end at the screen/target" model, just redirected).
class EffectChain
{
public:
    EffectChain() = default;
    ~EffectChain();

    // Compiles every stage in order, resolving shader filenames against
    // shadersDir. Returns false and leaves the chain empty if ANY stage
    // fails to load/compile - a partially-working glitch chain would be
    // confusing to debug live, so we fail the whole preset instead.
    bool load (const juce::Array<EffectStageConfig>& stageConfigs,
               const juce::File& shadersDir, juce::OpenGLContext& context);

    int getNumStages() const noexcept { return stages.size(); }
    juce::String getStageName (int index) const;

    void setStageBypassed (int index, bool bypassed);
    bool isStageBypassed (int index) const;
    void toggleStageBypassed (int index);

    // sourceImageTexture feeds stage 0's inputImage (typically a VideoPlayer
    // frame). If every stage is bypassed, renders sourceImageTexture
    // straight through via a trivial passthrough draw, so toggling all
    // stages off still shows the video instead of a black screen.
    void render (juce::OpenGLContext& context, unsigned int sourceImageTexture,
                 float timeSeconds, int pixelWidth, int pixelHeight,
                 float level, float bass, float mid, float high, float beatphase);

    void releaseGLObjects();

private:
    struct PingPongTarget
    {
        unsigned int fbo = 0, texture = 0;
        int width = 0, height = 0;
    };

    void ensureTarget (PingPongTarget& t, int width, int height);
    void drawPassthrough (juce::OpenGLContext& context, unsigned int sourceTexture, int pixelWidth, int pixelHeight);
    void ensurePassthroughResources (juce::OpenGLContext& context);

    juce::OwnedArray<ISFShader> stages;
    juce::Array<EffectStageConfig> configs; // parallel to `stages`
    juce::Array<bool> stageBypassed;
    PingPongTarget targets[2];

    // Minimal unlit textured-quad program used only when every stage is
    // bypassed, so the source image still reaches the screen.
    std::unique_ptr<juce::OpenGLShaderProgram> passthroughProgram;
    unsigned int passthroughVertexBuffer = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectChain)
};
