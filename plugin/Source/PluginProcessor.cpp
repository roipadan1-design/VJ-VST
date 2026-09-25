#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "EngineLocation.h"

const juce::StringArray VJAnalyzerProcessor::roleNames { "Mix", "Kick", "Snare", "Hat", "Bass", "Texture" };
const juce::StringArray VJAnalyzerProcessor::macroNames { "Intensity", "Speed", "Form", "Scale",
                                                          "Impact", "Erode", "Glide", "Detail" };
const juce::StringArray VJAnalyzerProcessor::moveIds { "drift", "push", "softness", "sync", "reverse", "freeze" };
const juce::StringArray VJAnalyzerProcessor::moveNames { "Drift", "Push", "Softness", "Sync", "Reverse", "Freeze" };
const juce::StringArray VJAnalyzerProcessor::actionIds { "hit", "snap1", "snap2", "snap3", "snap4", "scenePrev", "sceneNext" };
const juce::StringArray VJAnalyzerProcessor::reactIds { "reactKick", "reactSnare", "reactHat", "reactBass", "reactLevel" };
const juce::StringArray VJAnalyzerProcessor::reactNames { "Kick", "Snare", "Hat", "Bass", "Level" };
// Index = engine /v2/look slot. Slot 13 (Calm) is the separate bool parameter "calm".
const juce::StringArray VJAnalyzerProcessor::lookIds { "grain", "crush", "flash", "glitch", "trails", "symbols", "cutRate",
                                                       "smear", "halation", "weave", "dust", "blacks", "reactivity" };
const juce::StringArray VJAnalyzerProcessor::lookNames { "Grain", "Crush", "Flash", "Glitch", "Trails", "Symbols", "Cut Rate",
                                                         "Smear", "Halation", "Weave", "Dust", "Blacks", "Reactivity" };
const juce::StringArray VJAnalyzerProcessor::paletteNames { "Blood", "Ember", "Bone", "Ice", "Acid", "Violet", "Rust",
                                                            "Nitrate", "Cyanotype", "Tungsten", "Ash",
                                                            "Custom", "Scene Colors",
                                                            // Appended after Custom / Scene Colors so saved sets keep their indices.
                                                            "Split" };

std::array<juce::Colour, 3> VJAnalyzerProcessor::presetPalette (int index)
{
    using C = juce::Colour;
    switch (index)
    {
        case 1:  return { C (0xff080200), C (0xffff4a00), C (0xffffd27a) }; // Ember
        case 2:  return { C (0xff060606), C (0xff8a8580), C (0xfff4f0e8) }; // Bone
        case 3:  return { C (0xff00040a), C (0xff1a6cff), C (0xffc8f4ff) }; // Ice
        case 4:  return { C (0xff020600), C (0xff5cff1a), C (0xfff0ffc0) }; // Acid
        case 5:  return { C (0xff05000a), C (0xff8a1aff), C (0xffffa8f0) }; // Violet
        case 6:  return { C (0xff0a0402), C (0xff9a3a12), C (0xffe8b890) }; // Rust
        // Film families from the research style bible (docs/RESEARCH-REPORT.md).
        case 7:  return { C (0xff0d0b09), C (0xff7f6b52), C (0xfff3e6ce) }; // Nitrate (warm sepia)
        case 8:  return { C (0xff05080c), C (0xff2f5f79), C (0xffd8eef2) }; // Cyanotype night
        case 9:  return { C (0xff070504), C (0xff86461f), C (0xffffd9a8) }; // Tungsten halation
        case 10: return { C (0xff0a0a0a), C (0xff6b6a66), C (0xffedebe4) }; // Ash mono
        case 13: return { C (0xff000000), C (0xffff1a12), C (0xff1ae6ff) }; // Split: black / red / cyan (Negative scene)
        default: return { C (0xff050000), C (0xffe01008), C (0xffff9a86) }; // Blood
    }
}

juce::Colour VJAnalyzerProcessor::getCustomColour (int i) const
{
    auto fallback = presetPalette (0)[(size_t) i];
    auto v = state.state.getProperty ("colour" + juce::String (i), fallback.toString()).toString();
    return juce::Colour::fromString (v);
}

void VJAnalyzerProcessor::setCustomColour (int i, juce::Colour c)
{
    auto palette = (int) state.getRawParameterValue ("palette")->load();
    if (isPresetPalette (palette))
        for (int k = 0; k < 3; ++k)
            state.state.setProperty ("colour" + juce::String (k), presetPalette (palette)[(size_t) k].toString(), nullptr);
    state.state.setProperty ("colour" + juce::String (i), c.toString(), nullptr);

    if (palette != customPalette)
        if (auto* p = state.getParameter ("palette"))
            p->setValueNotifyingHost (p->convertTo0to1 ((float) customPalette));
}

std::array<juce::Colour, 3> VJAnalyzerProcessor::getPaletteColours() const
{
    auto palette = (int) state.getRawParameterValue ("palette")->load();
    if (isPresetPalette (palette))
        return presetPalette (palette);
    return { getCustomColour (0), getCustomColour (1), getCustomColour (2) };
}

const juce::StringArray VJAnalyzerProcessor::morphNames { "Cut", "1 Beat", "1 Bar", "4 Bars", "16 Bars" };

juce::StringArray VJAnalyzerProcessor::snapshotParamIds()
{
    juce::StringArray ids;
    for (int i = 0; i < 8; ++i)
        ids.add ("macro" + juce::String (i + 1));
    ids.addArray (lookIds);
    ids.addArray ({ "drift", "push", "softness", "shots", "hud" }); // older snapshots simply keep the current values
    return ids;
}

bool VJAnalyzerProcessor::hasSnapshot (int slot) const
{
    return state.state.getChildWithName ("Snapshots").getChildWithName ("S" + juce::String (slot)).isValid();
}

void VJAnalyzerProcessor::storeSnapshot (int slot)
{
    auto snapshots = state.state.getOrCreateChildWithName ("Snapshots", nullptr);
    auto existing = snapshots.getChildWithName ("S" + juce::String (slot));
    if (existing.isValid())
        snapshots.removeChild (existing, nullptr);

    juce::ValueTree s ("S" + juce::String (slot));
    for (auto& id : snapshotParamIds())
        s.setProperty (id, state.getRawParameterValue (id)->load(), nullptr);
    s.setProperty ("palette", (int) state.getRawParameterValue ("palette")->load(), nullptr);
    for (int i = 0; i < 3; ++i)
        s.setProperty ("colour" + juce::String (i), getCustomColour (i).toString(), nullptr);
    snapshots.addChild (s, -1, nullptr);
    activeSnapshot = slot;
}

void VJAnalyzerProcessor::recallSnapshot (int slot)
{
    auto s = state.state.getChildWithName ("Snapshots").getChildWithName ("S" + juce::String (slot));
    if (! s.isValid())
        return;

    morph = {};
    for (auto& id : snapshotParamIds())
    {
        morph.from.add (state.getRawParameterValue (id)->load());
        morph.to.add ((float) s.getProperty (id, state.getRawParameterValue (id)->load()));
    }
    morph.toPalette = (int) s.getProperty ("palette", 0);
    for (int i = 0; i < 3; ++i)
        morph.toColours.add (s.getProperty ("colour" + juce::String (i)).toString());

    const double beatsFor[] = { 0.0, 1.0, 4.0, 16.0, 64.0 };
    const auto choice = juce::jlimit (0, 4, (int) state.getRawParameterValue ("morphTime")->load());
    morph.seconds = beatsFor[choice] * 60.0 / juce::jlimit (20.0, 400.0, hostBpm.load());
    morph.start = juce::Time::getMillisecondCounterHiRes() * 0.001;
    morph.active = true;
    activeSnapshot = slot;
    advanceMorph(); // a cut lands immediately
}

void VJAnalyzerProcessor::advanceMorph()
{
    if (! morph.active)
        return;

    const auto now = juce::Time::getMillisecondCounterHiRes() * 0.001;
    auto t = morph.seconds <= 0.0 ? 1.0 : juce::jlimit (0.0, 1.0, (now - morph.start) / morph.seconds);
    const auto eased = (float) (t * t * (3.0 - 2.0 * t));

    auto ids = snapshotParamIds();
    for (int i = 0; i < ids.size(); ++i)
        if (auto* p = state.getParameter (ids[i]))
            p->setValueNotifyingHost (p->convertTo0to1 (morph.from[i] + (morph.to[i] - morph.from[i]) * eased));

    // Discrete parts (palette choice, custom colours) switch half-way.
    if (! morph.switchedDiscrete && t >= 0.5)
    {
        for (int i = 0; i < 3 && i < morph.toColours.size(); ++i)
            if (morph.toColours[i].isNotEmpty())
                state.state.setProperty ("colour" + juce::String (i), morph.toColours[i], nullptr);
        if (auto* p = state.getParameter ("palette"))
            p->setValueNotifyingHost (p->convertTo0to1 ((float) morph.toPalette));
        morph.switchedDiscrete = true;
    }

    if (t >= 1.0)
        morph.active = false;
}

juce::AudioProcessorValueTreeState::ParameterLayout VJAnalyzerProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "role", 1 }, "Role", roleNames, 0));

    NormalisableRange<float> sens (0.25f, 4.0f);
    sens.setSkewForCentre (1.0f);
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "sensitivity", 1 }, "Hit Sensitivity", sens, 1.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "trim", 1 }, "Input Trim",
                                                       NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
                                                       AudioParameterFloatAttributes().withLabel ("dB")));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "normalizer", 1 }, "Response",
                                                        StringArray { "Adaptive", "Locked" }, 0));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "sendControls", 1 }, "Send Controls", true));

    // Stable automation IDs macro1..macro8 (ENGINEERING_SPEC 6.3): labels may
    // change per preset, the IDs never do, so saved automation stays valid.
    // Values read as what they do (Speed "x1.00" / "Frozen", Glide in bars).
    const float macroDefaults[] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.4f, 0.5f, 0.25f, 0.5f };
    std::function<String (float, int)> percent = [] (float v, int) { return String (roundToInt (v * 100.0f)) + " %"; };
    std::function<String (float, int)> speedText = [] (float v, int) {
        auto s = v <= 0.5f ? (2.0f * v) * (2.0f * v) : std::pow (4.0f, 2.0f * v - 1.0f); // = engine SceneClock::speedCurve
        return s < 0.005f ? String ("Frozen") : "x" + String (s, s < 1.0f ? 2 : 1);
    };
    std::function<String (float, int)> glideText = [] (float v, int) { return String (v * v * 4.0f, 2) + " bars"; };
    for (int i = 0; i < 8; ++i)
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "macro" + String (i + 1), 1 }, macroNames[i],
                                                           NormalisableRange<float> (0.0f, 1.0f), macroDefaults[i],
                                                           AudioParameterFloatAttributes().withStringFromValueFunction (
                                                               i == 1 ? speedText : (i == 6 ? glideText : percent))));

    layout.add (std::make_unique<AudioParameterInt> (ParameterID { "preset", 1 }, "Preset", 0, 64, 0,
                                                     AudioParameterIntAttributes().withStringFromValueFunction (
                                                         [] (int v, int) { return v == 0 ? String ("Engine") : String (v); })));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "blackout", 1 }, "Blackout", false));

    // Ambient defaults: film on, every digital disturbance (flash, glitch,
    // symbols, smear, auto cuts) off until asked for.
    const float lookDefaults[] = { 0.3f, 0.35f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f,
                                   0.0f, 0.35f, 0.3f, 0.3f, 0.35f, 1.0f };
    for (int i = 0; i < lookIds.size(); ++i)
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { lookIds[i], 1 }, lookNames[i],
                                                           NormalisableRange<float> (0.0f, 1.0f), lookDefaults[i]));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "calm", 1 }, "Calm", false));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "morphTime", 1 }, "Snapshot Morph", morphNames, 2));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "palette", 1 }, "Palette", paletteNames, 0));

    for (int i = 0; i < reactIds.size(); ++i)
        layout.add (std::make_unique<AudioParameterBool> (ParameterID { reactIds[i], 1 }, "React " + reactNames[i], true));

    // --- added 2026-09 (appended, so existing IDs and their order are untouched)
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "drift", 1 }, "Drift", NormalisableRange<float> (0.0f, 1.0f), 0.2f,
                                                       AudioParameterFloatAttributes().withStringFromValueFunction (percent)));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "push", 1 }, "Push", NormalisableRange<float> (0.0f, 1.0f), 0.3f,
                                                       AudioParameterFloatAttributes().withStringFromValueFunction (
                                                           [] (float v, int) { return "up to x" + String (1.0f + v, 2); })));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "softness", 1 }, "Softness", NormalisableRange<float> (0.0f, 1.0f), 0.5f,
                                                       AudioParameterFloatAttributes().withStringFromValueFunction (
                                                           [] (float v, int) { return "decay x" + String (0.5f * std::pow (8.0f, v), 1); })));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "sync", 1 }, "Sync", false));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "reverse", 1 }, "Reverse", false));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "freeze", 1 }, "Freeze", false));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "useMedia", 1 }, "Use Media", false));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "shots", 1 }, "Shots", NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "hud", 1 }, "HUD", NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "lookahead", 1 }, "Visual Lookahead",
                                                       NormalisableRange<float> (0.0f, 150.0f, 1.0f), 0.0f,
                                                       AudioParameterFloatAttributes().withStringFromValueFunction (
                                                           [] (float v, int) { return v < 0.5f ? String ("Off") : String (roundToInt (v)) + " ms"; })));
    const char* actionNames[] = { "Hit", "Snapshot A", "Snapshot B", "Snapshot C", "Snapshot D", "Previous Scene", "Next Scene" };
    for (int i = 0; i < actionIds.size(); ++i)
        layout.add (std::make_unique<AudioParameterBool> (ParameterID { actionIds[i], 1 }, actionNames[i], false));
    return layout;
}

VJAnalyzerProcessor::VJAnalyzerProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      state (*this, nullptr, "VJAnalyzer", createLayout())
{
    state.state.setProperty ("engineHost", "127.0.0.1", nullptr);
    state.state.setProperty ("enginePort", vj::protocol::defaultPort, nullptr);
    state.state.setProperty ("enginePath", VJ_DEFAULT_ENGINE_PATH, nullptr);
    for (auto& id : actionIds)
        state.addParameterListener (id, this);
    startTimerHz (30);
}

VJAnalyzerProcessor::~VJAnalyzerProcessor()
{
    stopTimer();
    for (auto& id : actionIds)
        state.removeParameterListener (id, this);
    worker.release();
}

bool VJAnalyzerProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto in = layouts.getMainInputChannelSet(), out = layouts.getMainOutputChannelSet();
    return in == out && (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo());
}

void VJAnalyzerProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    lastSampleRate = sampleRate;
    worker.prepare (sampleRate, samplesPerBlock);
    delayBuffer.setSize (2, (int) std::ceil (sampleRate * 0.16) + samplesPerBlock);
    delayBuffer.clear();
    delayWrite = 0;
    updateLatency();
}

void VJAnalyzerProcessor::updateLatency()
{
    auto ms = state.getRawParameterValue ("lookahead")->load();
    auto samples = juce::jlimit (0, juce::jmax (0, delayBuffer.getNumSamples() - 1), (int) std::round (ms * 0.001 * lastSampleRate));
    if (samples != delaySamples.load())
    {
        delaySamples = samples;
        setLatencySamples (samples); // tells Live (delay compensation)
    }
}

void VJAnalyzerProcessor::releaseResources()
{
    worker.release();
}

void VJAnalyzerProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    // Audio is untouched (unless Visual Lookahead delays it, below): this is an analyser.
    const auto numSamples = buffer.getNumSamples();
    const float* left = buffer.getNumChannels() > 0 ? buffer.getReadPointer (0) : nullptr;
    const float* right = buffer.getNumChannels() > 1 ? buffer.getReadPointer (1) : nullptr;
    if (left != nullptr)
        worker.pushAudio (left, right, numSamples);

    for (const auto metadata : midi)
    {
        auto m = metadata.getMessage();
        if (m.isNoteOn())
            worker.pushNote (m.getNoteNumber(), m.getVelocity());
    }

    // Visual look-ahead: the analysis above saw the undelayed audio; what
    // leaves the plug-in is delayed (and reported to Live as latency).
    if (const auto delay = delaySamples.load(); delay > 0 && delayBuffer.getNumSamples() > delay)
    {
        const auto size = delayBuffer.getNumSamples();
        for (int ch = 0; ch < juce::jmin (2, buffer.getNumChannels()); ++ch)
        {
            auto* io = buffer.getWritePointer (ch);
            auto* line = delayBuffer.getWritePointer (ch);
            auto w = delayWrite;
            for (int i = 0; i < numSamples; ++i)
            {
                line[w] = io[i];
                auto r = w - delay;
                io[i] = line[r < 0 ? r + size : r];
                w = w + 1 == size ? 0 : w + 1;
            }
        }
        delayWrite = (delayWrite + numSamples) % size;
    }

    vj::protocol::Transport t;
    if (auto* head = getPlayHead())
    {
        if (auto pos = head->getPosition())
        {
            t.playing = pos->getIsPlaying();
            if (auto bpm = pos->getBpm())          { t.bpm = *bpm; hostBpm = *bpm; }
            if (auto ppq = pos->getPpqPosition()) { t.ppqPosition = *ppq; t.valid = true; }
            if (auto sig = pos->getTimeSignature()) { t.meterNumerator = sig->numerator; t.meterDenominator = sig->denominator; }

            // Seek / loop wrap / start-stop -> new epoch, so the engine snaps its clock.
            auto expected = lastPpq + (double) numSamples / lastSampleRate * t.bpm / 60.0;
            if (t.playing != lastPlaying || (t.playing && std::abs (t.ppqPosition - expected) > 0.25))
                ++epoch;
            lastPpq = t.ppqPosition;
            lastPlaying = t.playing;
        }
    }
    t.epoch = epoch;
    worker.setTransport (t);
}

void VJAnalyzerProcessor::parameterChanged (const juce::String& id, float newValue)
{
    // Rising edge of a momentary action (from the UI, automation or a MIDI
    // button, on whichever thread set it). HIT goes out at once (the worker
    // only reads an atomic flag); the rest run on the message thread.
    const auto index = actionIds.indexOf (id);
    if (index < 0 || newValue < 0.5f)
        return;
    if (index == 0)
        worker.sendUserTrigger();
    actionPending[(size_t) index] = true;
}

void VJAnalyzerProcessor::timerCallback()
{
    updateLatency();

    for (int i = 0; i < actionIds.size(); ++i)
    {
        if (! actionPending[(size_t) i].exchange (false))
            continue;
        if (i >= 1 && i <= 4)
            recallSnapshot (i - 1);
        else if (i == 5 || i == 6)
            worker.stepScene (i == 5 ? -1 : 1);
        // Reset, so the next press (whatever the controller sends) fires again.
        if (auto* p = state.getParameter (actionIds[i]))
            p->setValueNotifyingHost (0.0f);
    }

    advanceMorph();

    AnalysisWorker::Controls c;
    c.role = (int) state.getRawParameterValue ("role")->load();
    c.sensitivity = state.getRawParameterValue ("sensitivity")->load();
    c.trimDb = state.getRawParameterValue ("trim")->load();
    c.lockedNormalizer = state.getRawParameterValue ("normalizer")->load() > 0.5f;
    c.sendControls = state.getRawParameterValue ("sendControls")->load() > 0.5f;
    for (int i = 0; i < 8; ++i)
        c.macros[(size_t) i] = state.getRawParameterValue ("macro" + juce::String (i + 1))->load();
    c.preset = (int) state.getRawParameterValue ("preset")->load();
    c.blackout = state.getRawParameterValue ("blackout")->load() > 0.5f;
    for (int i = 0; i < lookIds.size(); ++i)
        c.look[(size_t) i] = state.getRawParameterValue (lookIds[i])->load();
    c.look[13] = state.getRawParameterValue ("calm")->load() > 0.5f ? 1.0f : 0.0f;
    c.look[14] = state.getRawParameterValue ("shots")->load();
    c.look[15] = state.getRawParameterValue ("hud")->load();
    auto colours = getPaletteColours();
    for (int i = 0; i < 3; ++i)
    {
        c.palette[(size_t) i * 3]     = colours[(size_t) i].getFloatRed();
        c.palette[(size_t) i * 3 + 1] = colours[(size_t) i].getFloatGreen();
        c.palette[(size_t) i * 3 + 2] = colours[(size_t) i].getFloatBlue();
    }
    c.paletteMix = (int) state.getRawParameterValue ("palette")->load() == sceneColours ? 0.0f : 1.0f;
    for (int i = 0; i < reactIds.size(); ++i)
        c.react[(size_t) i] = state.getRawParameterValue (reactIds[i])->load() > 0.5f;
    for (int i = 0; i < moveIds.size(); ++i)
        c.move[(size_t) i] = state.getRawParameterValue (moveIds[i])->load();
    c.mediaPath = getMediaPath();
    c.useMedia = state.getRawParameterValue ("useMedia")->load() > 0.5f;
    worker.setControls (c);

    worker.setTarget (state.state.getProperty ("engineHost", "127.0.0.1").toString(),
                      (int) state.state.getProperty ("enginePort", vj::protocol::defaultPort));
}

juce::AudioProcessorEditor* VJAnalyzerProcessor::createEditor()
{
    return new VJAnalyzerEditor (*this);
}

void VJAnalyzerProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = state.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void VJAnalyzerProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (state.state.getType()))
            state.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VJAnalyzerProcessor();
}
