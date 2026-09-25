#pragma once

#include <JuceHeader.h>
#include "AnalysisWorker.h"

// VJ Analyzer: drop on any track in Live 10.1+ (audio effect). Audio passes
// through bit-for-bit; a copy feeds the AnalysisWorker. Every control is a
// real host parameter, so Live automation and MIDI Map work natively and the
// state is saved with the set.
class VJAnalyzerProcessor : public juce::AudioProcessor,
                            private juce::Timer,
                            private juce::AudioProcessorValueTreeState::Listener
{
public:
    VJAnalyzerProcessor();
    ~VJAnalyzerProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "VJ Analyzer"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState& getState() noexcept { return state; }
    AnalysisWorker& getWorker() noexcept { return worker; }

    static const juce::StringArray roleNames;
    // Macro slots (IDs macro1..macro8 never change): SHAPE = Intensity, Form,
    // Scale, Erode, Detail (each scene names what they do); Speed and Glide
    // drive the engine's scene clock; Impact scales every hit reaction.
    static const juce::StringArray macroNames;

    // MOVE / REACT globals (engine /v2/move slots, same order): Drift, Push,
    // Softness, Sync, Reverse, Freeze.
    static const juce::StringArray moveIds, moveNames;

    // Momentary actions as host parameters, so a MIDI controller can press
    // them (Live's MIDI Map): HIT, recall snapshot A-D, previous / next scene.
    // They fire on the rising edge and reset themselves.
    static const juce::StringArray actionIds;

    // REACT TO: which channels may drive the visuals (engine /v2/react, same order).
    static const juce::StringArray reactIds, reactNames;

    // Global look (engine /v2/look slots 0-6, same order).
    static const juce::StringArray lookIds, lookNames;

    // Palette presets (choice parameter "palette"); the last two entries are
    // "Custom" (the three colours stored in the state) and "Scene Colors"
    // (palette mapping off - each scene's own colours).
    static const juce::StringArray paletteNames;
    static constexpr int customPalette = 11, sceneColours = 12;
    static bool isPresetPalette (int index) noexcept { return index != customPalette && index != sceneColours; }
    static std::array<juce::Colour, 3> presetPalette (int index);

    // Snapshots: four stored states (macros, LOOK, palette) recalled with a
    // musical morph ("morphTime": cut / 1 beat / 1 bar / 4 bars / 16 bars).
    // Stored in the plug-in state, so they are saved with the Live set.
    static constexpr int numSnapshots = 4;
    static const juce::StringArray morphNames;
    void storeSnapshot (int slot);
    void recallSnapshot (int slot);
    bool hasSnapshot (int slot) const;
    int getActiveSnapshot() const noexcept { return activeSnapshot; }
    bool isMorphing() const noexcept { return morph.active; }

    // The performer's image (media slot 1), saved with the Live set.
    juce::String getMediaPath() const { return state.state.getProperty ("mediaPath").toString(); }
    void setMediaPath (const juce::String& path) { state.state.setProperty ("mediaPath", path, nullptr); }

    // The three colours actually sent to the engine (preset or custom).
    std::array<juce::Colour, 3> getPaletteColours() const;
    juce::Colour getCustomColour (int i) const;
    // Sets one custom colour; if a preset palette was active it becomes the
    // starting point, so editing a preset turns it into "Custom".
    void setCustomColour (int i, juce::Colour c);

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void timerCallback() override; // pushes parameter values to the worker (message thread)
    void parameterChanged (const juce::String& id, float newValue) override; // momentary actions, any thread
    std::array<std::atomic<bool>, 7> actionPending {};
    void advanceMorph();
    static juce::StringArray snapshotParamIds();

    struct Morph
    {
        bool active = false;
        double start = 0.0, seconds = 0.0;
        bool switchedDiscrete = false;
        juce::Array<float> from, to;       // plain values, parallel to snapshotParamIds()
        int toPalette = 0;
        juce::StringArray toColours;
    };
    Morph morph;
    int activeSnapshot = -1;
    std::atomic<double> hostBpm { 120.0 };

    juce::AudioProcessorValueTreeState state;
    AnalysisWorker worker;

    // Transport discontinuity detection (audio thread only).
    double lastPpq = 0.0, lastSampleRate = 48000.0;
    bool lastPlaying = false;
    int epoch = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VJAnalyzerProcessor)
};
