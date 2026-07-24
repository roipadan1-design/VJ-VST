#pragma once

#include <JuceHeader.h>

// A named source for driving an ISF parameter live: one of our 5 audio-reactive
// signals, remapped by scale+offset so e.g. "bass" (0..1) can drive a shader
// parameter whose useful range is 0..10.
struct AudioMapping
{
    juce::String source; // "level", "bass", "mid", "high", "beatphase"
    float scale = 1.0f;
    float offset = 0.0f;
};

// One stage of an effectChain preset: its own ISF effect shader (expected to
// declare a standard "inputImage" sampler2D input), param overrides, and
// audio-reactive mappings - same shape as the top-level Preset fields, just
// scoped to this one stage.
struct EffectStageConfig
{
    juce::String shaderFile;
    juce::NamedValueSet paramOverrides;
    std::map<juce::String, AudioMapping> audioMappings;
};

// One visual preset. Either:
//  - a single ISF shader ("shader" field) - the original Phase 1 model, or
//  - an ordered chain of ISF effect shaders ("effectChain" field) fed by the
//    currently loaded video (see VideoPlayer/EffectChain) - for layered,
//    toggleable video-glitch presets.
// A preset can't usefully declare both; if it does, effectChain wins.
struct Preset
{
    juce::String name;
    juce::String shaderFile;
    juce::NamedValueSet paramOverrides;
    std::map<juce::String, AudioMapping> audioMappings;

    juce::Array<EffectStageConfig> effectChain;

    bool isEffectChain() const noexcept { return ! effectChain.isEmpty(); }

    // Parses a preset .json file. On failure, ok() is false and getError() explains why.
    static Preset loadFromFile (const juce::File& jsonFile, bool& ok, juce::String& error);
};

// Shared by PresetManager and EffectChain: maps an AudioMapping::source
// name to its current value. Returns 0 for an unrecognised name.
float resolveAudioMappingSource (const juce::String& source,
                                  float level, float bass, float mid, float high, float beatphase);
