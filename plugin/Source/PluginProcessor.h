#pragma once

#include <JuceHeader.h>
#include "AnalysisWorker.h"

// VJ Analyzer: drop on any track in Live 10.1+ (audio effect). Audio passes
// through bit-for-bit; a copy feeds the AnalysisWorker. Every control is a
// real host parameter, so Live automation and MIDI Map work natively and the
// state is saved with the set.
class VJAnalyzerProcessor : public juce::AudioProcessor,
                            private juce::Timer
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
    static const juce::StringArray macroNames;

    // Global look (engine /v2/look slots 0-6, same order).
    static const juce::StringArray lookIds, lookNames;

    // Palette presets (choice parameter "palette"); the last two entries are
    // "Custom" (the three colours stored in the state) and "Scene Colors"
    // (palette mapping off - each scene's own colours).
    static const juce::StringArray paletteNames;
    static constexpr int customPalette = 7, sceneColours = 8;
    static std::array<juce::Colour, 3> presetPalette (int index);

    // The three colours actually sent to the engine (preset or custom).
    std::array<juce::Colour, 3> getPaletteColours() const;
    juce::Colour getCustomColour (int i) const;
    // Sets one custom colour; if a preset palette was active it becomes the
    // starting point, so editing a preset turns it into "Custom".
    void setCustomColour (int i, juce::Colour c);

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void timerCallback() override; // pushes parameter values to the worker (message thread)

    juce::AudioProcessorValueTreeState state;
    AnalysisWorker worker;

    // Transport discontinuity detection (audio thread only).
    double lastPpq = 0.0, lastSampleRate = 48000.0;
    bool lastPlaying = false;
    int epoch = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VJAnalyzerProcessor)
};
