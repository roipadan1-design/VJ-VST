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

// One visual preset: which ISF shader to load, static parameter overrides
// (values that differ from the shader's own ISF DEFAULTs), and optional
// audio-reactive mappings per parameter.
struct Preset
{
    juce::String name;
    juce::String shaderFile;
    juce::NamedValueSet paramOverrides;
    std::map<juce::String, AudioMapping> audioMappings;

    // Parses a preset .json file. On failure, ok() is false and getError() explains why.
    static Preset loadFromFile (const juce::File& jsonFile, bool& ok, juce::String& error);
};
