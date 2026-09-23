#pragma once

#include <JuceHeader.h>
#include "PresetV2.h"
#include "Signals.h"

// Global performance state shared by every preset: the eight stable macro
// slots. Host automation, MIDI and the plugin UI all set the *base* value
// here; modulation never writes it back (ENGINEERING_SPEC 6.3).
struct MacroBank
{
    static constexpr int numSlots = 8;
    std::array<float, numSlots> values {};
    std::array<bool, numSlots> set {};
};

// Evaluates one schema-2 preset's modulation graph each frame
// (ENGINEERING_SPEC 6.1):
//   1. fire triggers from this frame's events (refractory, optional beat/bar quantize)
//   2. advance envelopes and tempo LFOs on elapsed time / musical time
//   3. read macro bases (global bank, else the preset's defaults)
//   4. accumulate every route into a per-parameter sum (stable order)
//   5. add to the normalised base, clamp (or wrap cyclic params) once
//   6. convert to physical units
// Names are resolved to indices at construction, so the frame loop does no
// string lookups and no allocation.
class ModulationRuntime
{
public:
    explicit ModulationRuntime (const PresetV2& preset);

    void process (const Signals& signals, const Clock& clock, const MacroBank& macros, double dtSeconds, double nowSeconds);

    // Physical parameter values, [stage][parameter] in preset order.
    const juce::Array<juce::Array<float>>& getValues() const noexcept { return values; }

    // Live base override for one parameter (OSC /effect/param), in physical units.
    bool setBaseValue (int stageIndex, const juce::String& parameterName, float physicalValue);

    float getPalette() const noexcept { return paletteValue; } // smoothed palette-advance steps
    float getSeed() const noexcept { return (float) seed; }     // bumped by "reseed" actions

    // Problems found while resolving names (logged once by the caller).
    const juce::StringArray& getWarnings() const noexcept { return warnings; }

private:
    enum class SourceKind
    {
        zero, levelRel, levelAbs, bassRel, midRel, highRel, bassAbs, midAbs, highAbs,
        kickActivity, snareActivity, hatActivity, band, build, presence,
        centroid, flatness, rolloff, flux, energyTrend,
        beatPhase, barPhase, macro, envelope, lfo
    };

    struct Envelope
    {
        float attackSeconds = 0.005f, decaySeconds = 0.2f, peak = 1.0f;
        bool retriggerMax = true;
        float value = 0.0f, start = 0.0f, target = 0.0f, elapsed = 0.0f;
        bool attacking = false;
        void trigger (float amount) noexcept;
        void advance (float dt) noexcept;
    };

    struct Lfo
    {
        V2Modulator::Shape shape = V2Modulator::Shape::sine;
        double periodBeats = 4.0, phaseOffset = 0.0;
        float value = 0.5f;
    };

    struct ResolvedRoute
    {
        SourceKind kind = SourceKind::zero;
        int index = 0;             // band / macro / modulator index
        int stage = 0, parameter = 0;
        int scaleMacro = -1;       // "scaleBy" macro index, -1 = none
        V2Route def;
        float smoothed = 0.0f;
        bool initialised = false;
    };

    struct ResolvedTrigger
    {
        V2Trigger def;
        juce::Array<int> envelopeTargets; // parallel to def.actions (-1 = not an envelope action)
        double lastFired = -1000.0;
        bool pending = false;
        float pendingStrength = 0.0f;
    };

    float readSource (const ResolvedRoute&, const Signals&, const Clock&, const MacroBank&) const noexcept;
    bool isClosedByReact (const ResolvedRoute&, const ReactMask&) const noexcept;
    static bool isAudioDriven (SourceKind) noexcept;
    void fireTrigger (ResolvedTrigger&, float strength);

    const PresetV2& preset;
    juce::Array<juce::Array<float>> values, base01;
    juce::Array<juce::Array<double>> integrals; // running phase for "integrate" parameters
    juce::Array<juce::Array<float>> accumulators;
    juce::Array<Envelope> envelopes;   // parallel to preset.modulators (non-AD entries unused)
    juce::Array<Lfo> lfos;             // parallel to preset.modulators (non-LFO entries unused)
    std::vector<ResolvedRoute> routes;
    std::vector<ResolvedTrigger> triggers;
    juce::StringArray warnings;

    float paletteTarget = 0.0f, paletteValue = 0.0f;
    int seed = 1;
};
