// plugin_harness - drives the VJ Analyzer processor + editor without a DAW.
//
//   plugin_harness out.png [seconds]
//
// Instantiates the processor, feeds it a synthetic 124 BPM groove (kick,
// off-beat hats, snare on 2 & 4) through processBlock in real time with a
// running transport, opens the editor, and writes a PNG snapshot of the UI
// at the end. If the VJ Engine is running, the analysis reaches it and the
// engine's status comes back, so this is an end-to-end check of
// plugin -> engine -> plugin as well as a design preview.

#include <JuceHeader.h>
#include "../Source/PluginProcessor.h"

namespace
{
struct FakePlayHead : juce::AudioPlayHead
{
    double ppq = 0.0, bpm = 124.0;
    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo info;
        info.setIsPlaying (true);
        info.setBpm (bpm);
        info.setPpqPosition (ppq);
        info.setTimeSignature (TimeSignature { 4, 4 });
        return info;
    }
};

class Groove
{
public:
    explicit Groove (double sr) : rate (sr) {}

    void render (juce::AudioBuffer<float>& buffer, double startBeat, double bpm)
    {
        auto samplesPerBeat = rate * 60.0 / bpm;
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            auto beat = startBeat + i / samplesPerBeat;
            auto phase = beat - std::floor (beat);
            auto beatInBar = (int) std::fmod (std::floor (beat), 4.0);
            auto t = phase * samplesPerBeat / rate;

            float s = 0.0f;
            // Kick: pitch-swept sine.
            s += 0.8f * (float) (std::sin (2.0 * juce::MathConstants<double>::pi * (48.0 * t + 102.0 * 0.03 * (1.0 - std::exp (-t / 0.03))))
                                 * std::exp (-t / 0.12));
            // Hat on the off-beat.
            auto th = (phase - 0.5) * samplesPerBeat / rate;
            if (th >= 0.0)
                s += 0.18f * (random.nextFloat() * 2.0f - 1.0f) * (float) std::exp (-th / 0.015);
            // Snare on 2 and 4 (noise + body).
            if (beatInBar == 1 || beatInBar == 3)
                s += (float) std::exp (-t / 0.09) * (0.3f * (random.nextFloat() * 2.0f - 1.0f)
                                                    + 0.25f * (float) std::sin (2.0 * juce::MathConstants<double>::pi * 190.0 * t));
            // Pad so the gate stays open.
            s += 0.02f * (float) std::sin (2.0 * juce::MathConstants<double>::pi * 220.0 * (beat * 60.0 / bpm));

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.setSample (ch, i, s);
        }
    }

private:
    double rate;
    juce::Random random { 42 };
};
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;

    juce::String outPath = argc > 1 ? juce::String (argv[1]) : juce::String ("plugin_ui.png");
    double seconds = argc > 2 ? juce::String (argv[2]).getDoubleValue() : 6.0;

    const double rate = 48000.0;
    const int block = 256;

    VJAnalyzerProcessor processor;
    FakePlayHead head;
    processor.setPlayHead (&head);
    processor.setPlayConfigDetails (2, 2, rate, block);
    processor.prepareToPlay (rate, block);

    std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());
    editor->setVisible (true);

    Groove groove (rate);
    juce::AudioBuffer<float> buffer (2, block);
    juce::MidiBuffer midi;

    auto start = juce::Time::getMillisecondCounterHiRes();
    int64_t samples = 0;
    while (samples < (int64_t) (seconds * rate))
    {
        head.ppq = samples / rate * head.bpm / 60.0;
        groove.render (buffer, head.ppq, head.bpm);
        processor.processBlock (buffer, midi);
        samples += block;

        // Real-time pacing, pumping the message loop for the editor/timers.
        auto due = start + samples / rate * 1000.0;
        while (juce::Time::getMillisecondCounterHiRes() < due)
            juce::MessageManager::getInstance()->runDispatchLoopUntil (1);
    }

    auto snapshot = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);
    juce::File out (juce::File::getCurrentWorkingDirectory().getChildFile (outPath));
    out.deleteFile();
    juce::FileOutputStream stream (out);
    juce::PNGImageFormat().writeImageToStream (snapshot, stream);

    auto status = processor.getWorker().getEngineStatus();
    std::printf ("snapshot: %s\nengine: %s  preset: %s  fps: %.0f  presets listed: %d\n",
                 out.getFullPathName().toRawUTF8(), status.connected ? "connected" : "not connected",
                 status.presetName.toRawUTF8(), status.fps, status.presetNames.size());

    editor = nullptr;
    processor.releaseResources();
    return 0;
}
