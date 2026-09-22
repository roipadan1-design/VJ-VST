#pragma once

#include <JuceHeader.h>
#include "ISFShader.h"
#include "EffectChain.h"
#include "Preset.h"

// Owns the list of presets and whichever of the two is currently active -
// a single compiled ISFShader (the original Phase 1 model), or an
// EffectChain (layered, individually-toggleable video-glitch presets) -
// applying each frame's audio-reactive mappings before rendering.
//
// Preset switches crossfade rather than cutting: selectPreset() keeps the
// previous preset alive (as "outgoing") alongside the newly compiled one
// ("current") for transitionDurationMs (default 600ms, live-adjustable via
// setTransitionDuration()/OSC /preset/transitionduration), during which
// render() draws both to offscreen buffers and alpha-blends them. Both keep
// receiving live audio data during the fade, so the outgoing preset doesn't
// visually freeze.
class PresetManager
{
public:
    PresetManager() = default;
    ~PresetManager();

    // Scans presetsDir for *.json preset files (sorted by filename). Shader
    // paths inside preset files are resolved relative to shadersDir.
    void scanPresets (const juce::File& presetsDir, const juce::File& shadersDir);

    // Compiles and switches to the preset at `index` (wraps around), starting
    // a crossfade from whatever was previously current (if anything - the
    // very first call has nothing to fade from). Returns false (and logs via
    // DBG) if the shader(s) failed to load/compile - in that case the
    // previous preset, if any, keeps rendering, uninterrupted.
    bool selectPreset (int index, juce::OpenGLContext& context);
    void nextPreset (juce::OpenGLContext& context);
    void previousPreset (juce::OpenGLContext& context);

    // videoTexture (0 = none loaded) feeds the ISF "inputImage" input - of
    // the current single shader if it declares one, or of an effectChain's
    // first non-bypassed stage.
    void render (juce::OpenGLContext& context, float timeSeconds, int pixelWidth, int pixelHeight,
                 float level, float bass, float mid, float high, float beatphase, float onset,
                 unsigned int videoTexture = 0, unsigned int finalTargetFbo = 0);

    void releaseGLObjects();

    int getNumPresets() const noexcept { return presets.size(); }
    int getCurrentIndex() const noexcept { return currentIndex; }
    juce::String getCurrentName() const;

    // Live manipulation of the current preset's effect chain, if it has one
    // (no-op otherwise) - this is the "glitch layer toggling" control surface.
    int getNumEffectStages() const;
    void toggleEffectStage (int stageIndex);

    // Live per-parameter control (OSC /effect/param). For an effectChain
    // preset, stageIndex selects which stage; for a single-shader preset
    // stageIndex is ignored (there's only the one shader). No-op if there's
    // no active preset. See EffectChain::setStageParam for the audioMappings
    // interaction caveat.
    void setEffectParam (int stageIndex, const juce::String& name, const juce::var& value);

    // Live-adjustable crossfade length (OSC /preset/transitionduration) - a
    // performer may want instant cuts for some sections of a set and long
    // fades for others. Takes effect on the next selectPreset() call; an
    // in-progress crossfade keeps whatever duration it started with.
    void setTransitionDuration (double ms) noexcept { transitionDurationMs = juce::jmax (0.0, ms); }
    double getTransitionDuration() const noexcept { return transitionDurationMs; }

private:
    struct CrossfadeTarget
    {
        unsigned int fbo = 0, texture = 0;
        int width = 0, height = 0;
    };

    // Renders whichever of shader/effectChain is non-null to targetFbo -
    // shared by the current-preset path and (during a crossfade) the
    // outgoing-preset path, which are otherwise identical render calls.
    // presetIndexForMappings looks up audioMappings for the single-shader
    // case (an EffectChain already has its stage audioMappings baked in).
    void renderActive (ISFShader* shader, EffectChain* effectChain, int presetIndexForMappings,
                        juce::OpenGLContext& context, float timeSeconds, int pixelWidth, int pixelHeight,
                        float level, float bass, float mid, float high, float beatphase, float onset,
                        unsigned int videoTexture, unsigned int targetFbo);

    void ensureCrossfadeTarget (CrossfadeTarget& t, int width, int height);
    void ensureBlendResources (juce::OpenGLContext& context);
    void drawBlend (juce::OpenGLContext& context, unsigned int outgoingTexture, unsigned int incomingTexture,
                     float alpha, int pixelWidth, int pixelHeight, unsigned int finalTargetFbo);

    juce::Array<Preset> presets;
    juce::File shadersDirectory;
    std::unique_ptr<ISFShader> currentShader;
    std::unique_ptr<EffectChain> currentEffectChain;
    int currentIndex = -1;

    std::unique_ptr<ISFShader> outgoingShader;
    std::unique_ptr<EffectChain> outgoingEffectChain;
    int outgoingIndex = -1;
    bool transitioning = false;
    double transitionStartMs = 0.0;
    double transitionDurationMs = 600.0;
    CrossfadeTarget outgoingTarget, incomingTarget;

    std::unique_ptr<juce::OpenGLShaderProgram> blendProgram;
    unsigned int blendVertexBuffer = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
