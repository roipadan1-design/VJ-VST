#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "EngineLocation.h"

const juce::StringArray VJAnalyzerProcessor::roleNames { "Mix", "Kick", "Snare", "Hat", "Bass", "Texture" };
const juce::StringArray VJAnalyzerProcessor::macroNames { "Intensity", "Motion", "Color", "Space",
                                                          "Impact", "Gravity", "Viscosity", "Detail" };
const juce::StringArray VJAnalyzerProcessor::reactIds { "reactKick", "reactSnare", "reactHat", "reactBass", "reactLevel" };
const juce::StringArray VJAnalyzerProcessor::reactNames { "Kick", "Snare", "Hat", "Bass", "Level" };
const juce::StringArray VJAnalyzerProcessor::lookIds { "grain", "crush", "flash", "glitch", "trails", "symbols", "cutRate" };
const juce::StringArray VJAnalyzerProcessor::lookNames { "Grain", "Crush", "Flash", "Glitch", "Trails", "Symbols", "Cut Rate" };
const juce::StringArray VJAnalyzerProcessor::paletteNames { "Blood", "Ember", "Bone", "Ice", "Acid", "Violet", "Rust",
                                                            "Custom", "Scene Colors" };

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
    if (palette < customPalette)
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
    if (palette < customPalette)
        return presetPalette (palette);
    return { getCustomColour (0), getCustomColour (1), getCustomColour (2) };
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
    for (int i = 0; i < 8; ++i)
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "macro" + String (i + 1), 1 }, macroNames[i],
                                                           NormalisableRange<float> (0.0f, 1.0f), i == 1 ? 0.4f : 0.5f));

    layout.add (std::make_unique<AudioParameterInt> (ParameterID { "preset", 1 }, "Preset", 0, 64, 0,
                                                     AudioParameterIntAttributes().withStringFromValueFunction (
                                                         [] (int v, int) { return v == 0 ? String ("Engine") : String (v); })));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "blackout", 1 }, "Blackout", false));

    const float lookDefaults[] = { 0.25f, 0.35f, 0.2f, 0.1f, 0.1f, 0.2f, 0.0f };
    for (int i = 0; i < lookIds.size(); ++i)
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { lookIds[i], 1 }, lookNames[i],
                                                           NormalisableRange<float> (0.0f, 1.0f), lookDefaults[i]));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "palette", 1 }, "Palette", paletteNames, 0));

    for (int i = 0; i < reactIds.size(); ++i)
        layout.add (std::make_unique<AudioParameterBool> (ParameterID { reactIds[i], 1 }, "React " + reactNames[i], true));
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
    startTimerHz (30);
}

VJAnalyzerProcessor::~VJAnalyzerProcessor()
{
    stopTimer();
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
}

void VJAnalyzerProcessor::releaseResources()
{
    worker.release();
}

void VJAnalyzerProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    // Audio is untouched: this is an analyser, never a processor of the sound.
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

    vj::protocol::Transport t;
    if (auto* head = getPlayHead())
    {
        if (auto pos = head->getPosition())
        {
            t.playing = pos->getIsPlaying();
            if (auto bpm = pos->getBpm())          t.bpm = *bpm;
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

void VJAnalyzerProcessor::timerCallback()
{
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
