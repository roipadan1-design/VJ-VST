#pragma once

#include <JuceHeader.h>
#include "ISFShader.h"
#include "Preset.h"

// Owns the list of presets, the currently-compiled ISFShader, and applies
// each frame's audio-reactive mappings before rendering.
class PresetManager
{
public:
    PresetManager() = default;

    // Scans presetsDir for *.json preset files (sorted by filename). Shader
    // paths inside preset files are resolved relative to shadersDir.
    void scanPresets (const juce::File& presetsDir, const juce::File& shadersDir);

    // Compiles and switches to the preset at `index` (wraps around). Returns
    // false (and logs via DBG) if the shader failed to load/compile - in that
    // case the previous preset, if any, keeps rendering.
    bool selectPreset (int index, juce::OpenGLContext& context);
    void nextPreset (juce::OpenGLContext& context);
    void previousPreset (juce::OpenGLContext& context);

    void render (juce::OpenGLContext& context, float timeSeconds, int pixelWidth, int pixelHeight,
                 float level, float bass, float mid, float high, float beatphase);

    void releaseGLObjects();

    int getNumPresets() const noexcept { return presets.size(); }
    int getCurrentIndex() const noexcept { return currentIndex; }
    juce::String getCurrentName() const;

private:
    float valueForSource (const juce::String& source, float level, float bass, float mid, float high, float beatphase) const;

    juce::Array<Preset> presets;
    juce::File shadersDirectory;
    std::unique_ptr<ISFShader> currentShader;
    int currentIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
