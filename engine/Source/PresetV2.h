#pragma once

#include <JuceHeader.h>
#include "Signals.h"

// Schema-2 preset ("schemaVersion": "2.0"), the format specified in
// visual_instrument_research/preset.schema.json. A preset is a small
// instrument: ISF stages with ranged parameters, up to 8 macros, modulators
// (AD envelopes, tempo LFOs), additive routes from typed sources to stage
// parameters, event triggers, a transition and a finishing (post) block.
//
// Sources a route may read:
//   audio.level|bass|mid|high.activity|absolute   (bass/mid/high follow sourcePolicy)
//   audio.kick|snare|hat.activity                 (role source, else mix band)
//   audio.band0..band5.activity
//   descriptor.centroid|flatness|rolloff|flux|energyTrend
//   clock.beatPhase|barPhase
//   macro.<id>   env.<id>   lfo.<id>
// Destinations: stage.<stageId>.<parameterName>

enum class Quantize { none, beat, bar };

struct V2Parameter
{
    juce::String name, label;
    float min = 0.0f, max = 1.0f, defaultValue = 0.0f;
    bool cyclic = false;
    // Engine extension: the (modulated) value is a rate per second and the
    // shader receives its running integral - speed changes never jump phase.
    bool integrate = false;
};

// Raw material bound to one of a stage's ISF "image" inputs (engine extension,
// "sources" block on a stage):
//   { "type": "images", "folder": "Images/Portraits", "advance": "bar", "every": 2, "order": "random" }
//   { "type": "text", "words": ["MIR", "GIB", "ALLES"], "font": "Arial Black", "advance": "beat" }
// advance: none | beat | bar | event.<name> - the source steps to its next
// image/word on that clock boundary or hit, every `every` times.
struct V2Source
{
    enum class Type { images, text };
    enum class Advance { none, beat, bar, event };

    juce::String input;           // ISF image input name
    Type type = Type::images;
    juce::String folder, font;
    juce::StringArray words;
    Advance advance = Advance::bar;
    EventType event = EventType::kick;
    int every = 1;
    bool random = false;
};

struct V2Stage
{
    juce::String id, shader;
    bool generator = true;
    bool floatTarget = true;   // rgba16f/rgba32f vs rgba8
    float scale = 1.0f;        // render-resolution fraction
    juce::Array<V2Parameter> parameters;
    juce::Array<V2Source> sources;
};

struct V2Macro
{
    juce::String id, label;
    int slot = 0;
    float defaultValue = 0.5f;
};

struct V2Modulator
{
    enum class Type { ad, lfo };
    enum class Shape { sine, triangle, ramp, sampleHold };

    juce::String id;
    Type type = Type::ad;

    float attackMs = 5.0f, decayMs = 200.0f, peak = 1.0f;
    bool retriggerMax = true;      // "max": a new hit can only raise the envelope

    Shape shape = Shape::sine;
    double periodBeats = 4.0, phaseOffset = 0.0;
    bool bipolar = true;
};

struct V2Route
{
    enum class Curve { linear, power, smoothstep };

    juce::String id, source, destination;
    float inputMin = 0.0f, inputMax = 1.0f;
    Curve curve = Curve::linear;
    float exponent = 1.0f, center = 0.0f, amount = 0.0f;
    float attackMs = 0.0f, releaseMs = 0.0f;
};

struct V2Action
{
    enum class Type { envelope, paletteAdvance, reseed };
    Type type = Type::envelope;
    juce::String target;
    float amount = 1.0f;
    int steps = 1;
};

struct V2Trigger
{
    juce::String id;
    EventType on = EventType::kick;
    float minStrength = 0.0f, refractoryMs = 0.0f;
    Quantize quantize = Quantize::none;
    juce::Array<V2Action> actions;
};

struct V2Transition
{
    enum class Type { cut, crossfade, dipToBackground, lumaWipe };
    Type type = Type::crossfade;
    float durationMs = 600.0f;
    Quantize quantize = Quantize::none;
};

struct V2Post
{
    bool bloom = true;
    float bloomAmount = 0.2f, bloomThreshold = 1.0f;
    int bloomLevels = 4;
    bool reinhard = true;
    float exposureEv = 0.0f, grain = 0.0f, vignette = 0.0f;
};

struct PresetV2
{
    juce::String id, name, description;
    bool bassFromKick = true, midFromSnare = true, highFromHat = true; // sourcePolicy
    juce::Array<V2Stage> stages;
    juce::Array<V2Macro> macros;
    juce::Array<V2Modulator> modulators;
    juce::Array<V2Route> routes;
    juce::Array<V2Trigger> triggers;
    V2Transition transition;
    V2Post post;
    int seed = 1;

    static bool isSchema2 (const juce::var& json);

    // Parses and cross-validates (unique ids, route endpoints, envelope
    // targets, parameter ranges). On failure returns false with `error` set.
    static bool parse (const juce::var& json, PresetV2& out, juce::String& error);
};
