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

// --selftest: exercises processor logic that has no audio/visual output of
// its own (snapshots: store, cut recall, timed morph, state round-trip).
// Prints each check and returns non-zero if any failed.
static int runSelfTest()
{
    int failures = 0;
    auto check = [&failures] (bool ok, const char* what) {
        std::printf ("%s  %s\n", ok ? "PASS" : "FAIL", what);
        if (! ok) ++failures;
    };
    auto pump = [] (double seconds) {
        auto until = juce::Time::getMillisecondCounterHiRes() + seconds * 1000.0;
        while (juce::Time::getMillisecondCounterHiRes() < until)
            juce::MessageManager::getInstance()->runDispatchLoopUntil (5);
    };

    VJAnalyzerProcessor processor;
    auto& st = processor.getState();
    auto set = [&st] (const char* id, float plain) {
        auto* p = st.getParameter (id);
        p->setValueNotifyingHost (p->convertTo0to1 (plain));
    };
    auto get = [&st] (const char* id) { return st.getRawParameterValue (id)->load(); };

    set ("macro1", 0.2f); set ("grain", 0.7f); set ("palette", 8.0f);
    processor.storeSnapshot (0);
    check (processor.hasSnapshot (0) && ! processor.hasSnapshot (1), "store fills slot A only");

    set ("macro1", 0.9f); set ("grain", 0.1f); set ("palette", 0.0f);
    set ("morphTime", 0.0f); // cut
    processor.recallSnapshot (0);
    check (std::abs (get ("macro1") - 0.2f) < 0.01f && std::abs (get ("grain") - 0.7f) < 0.01f, "cut recall restores knobs");
    check ((int) get ("palette") == 8, "cut recall restores palette");

    set ("macro1", 0.9f);
    set ("morphTime", 1.0f); // 1 beat = 0.5 s at the default 120 BPM
    processor.recallSnapshot (0);
    pump (0.25);
    auto mid = get ("macro1");
    check (mid < 0.88f && mid > 0.22f && processor.isMorphing(), "morph is part-way after a quarter second");
    pump (0.5);
    check (std::abs (get ("macro1") - 0.2f) < 0.01f && ! processor.isMorphing(), "morph lands on the snapshot");

    juce::MemoryBlock saved;
    processor.getStateInformation (saved);
    VJAnalyzerProcessor restored;
    restored.setStateInformation (saved.getData(), (int) saved.getSize());
    check (restored.hasSnapshot (0) && ! restored.hasSnapshot (1), "snapshots survive a save/load of the Live set");

    std::printf ("%s (%d failure%s)\n", failures == 0 ? "ALL PASS" : "FAILED", failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;

    if (argc > 1 && juce::String (argv[1]) == "--selftest")
        return runSelfTest();

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
