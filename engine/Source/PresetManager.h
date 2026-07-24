#pragma once

#include <JuceHeader.h>
#include "ISFShader.h"
#include "EffectChain.h"
#include "Preset.h"

// Owns the list of presets and whichever of the two is currently active -
// a single compiled ISFShader (the original Phase 1 model), or an
// EffectChain (layered, individually-toggleable video-glitch presets) -
// applying each frame's audio-reactive mappings before rendering.
class PresetManager
{
public:
    PresetManager() = default;

    // Scans presetsDir for *.json preset files (sorted by filename). Shader
    // paths inside preset files are resolved relative to shadersDir.
    void scanPresets (const juce::File& presetsDir, const juce::File& shadersDir);

    // Compiles and switches to the preset at `index` (wraps around). Returns
    // false (and logs via DBG) if the shader(s) failed to load/compile - in
    // that case the previous preset, if any, keeps rendering.
    bool selectPreset (int index, juce::OpenGLContext& context);
    void nextPreset (juce::OpenGLContext& context);
    void previousPreset (juce::OpenGLContext& context);

    // videoTexture (0 = none loaded) feeds the ISF "inputImage" input - of
    // the current single shader if it declares one, or of an effectChain's
    // first non-bypassed stage.
    void render (juce::OpenGLContext& context, float timeSeconds, int pixelWidth, int pixelHeight,
                 float level, float bass, float mid, float high, float beatphase,
                 unsigned int videoTexture = 0);

    void releaseGLObjects();

    int getNumPresets() const noexcept { return presets.size(); }
    int getCurrentIndex() const noexcept { return currentIndex; }
    juce::String getCurrentName() const;

    // Live manipulation of the current preset's effect chain, if it has one
    // (no-op otherwise) - this is the "glitch layer toggling" control surface.
    int getNumEffectStages() const;
    void toggleEffectStage (int stageIndex);

private:
    juce::Array<Preset> presets;
    juce::File shadersDirectory;
    std::unique_ptr<ISFShader> currentShader;
    std::unique_ptr<EffectChain> currentEffectChain;
    int currentIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
