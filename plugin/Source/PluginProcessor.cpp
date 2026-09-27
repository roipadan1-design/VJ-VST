#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "EngineLocation.h"

const juce::StringArray VJAnalyzerProcessor::roleNames { "Mix", "Kick", "Snare", "Hat", "Bass", "Texture" };
const juce::StringArray VJAnalyzerProcessor::macroNames { "Intensity", "Speed", "Form", "Scale",
                                                          "React", "Erode", "Glide", "Detail" };
const juce::StringArray VJAnalyzerProcessor::styleNames { "Breathe", "Pulse", "Punch" };
const juce::StringArray VJAnalyzerProcessor::lookPresetNames { "Custom", "Clean", "Film", "Worn", "Broken", "Print", "Data" };
const juce::StringArray VJAnalyzerProcessor::lookVectorIds { "grain", "crush", "flash", "glitch", "trails", "symbols", "smear",
                                                             "halation", "weave", "dust", "blacks", "shots", "hud" };

const std::array<float, 13>& VJAnalyzerProcessor::lookVector (int preset)
{
    // docs/product-design/CONTROL-MAP.md 5.4. Order = lookVectorIds.
    static const std::array<std::array<float, 13>, 7> vectors { {
        {},                                                                          // Custom (unused)
        { 0.05f, 0.25f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.10f, 0.0f, 0.0f, 0.20f, 0.0f, 0.0f },       // Clean
        { 0.30f, 0.35f, 0.0f, 0.0f, 0.10f, 0.0f, 0.0f, 0.35f, 0.30f, 0.30f, 0.35f, 0.0f, 0.0f },    // Film (the defaults)
        { 0.50f, 0.40f, 0.0f, 0.05f, 0.20f, 0.0f, 0.15f, 0.45f, 0.55f, 0.65f, 0.50f, 0.0f, 0.0f },  // Worn
        { 0.40f, 0.55f, 0.30f, 0.50f, 0.30f, 0.0f, 0.45f, 0.30f, 0.40f, 0.30f, 0.30f, 0.35f, 0.0f },// Broken
        { 0.55f, 0.85f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.20f, 0.50f, 0.60f, 0.15f, 0.0f },    // Print
        { 0.20f, 0.50f, 0.15f, 0.15f, 0.10f, 0.50f, 0.10f, 0.20f, 0.10f, 0.10f, 0.30f, 0.40f, 0.60f } // Data
    } };
    return vectors[(size_t) juce::jlimit (0, 6, preset)];
}
const juce::StringArray VJAnalyzerProcessor::moveIds { "drift", "push", "softness", "sync", "reverse", "freeze" };
const juce::StringArray VJAnalyzerProcessor::moveNames { "Drift", "Push", "Softness", "Sync", "Reverse", "Freeze" };
const juce::StringArray VJAnalyzerProcessor::clipSyncNames { "Free", "1 Beat", "1 Bar", "2 Bars", "4 Bars", "8 Bars" };
const juce::StringArray VJAnalyzerProcessor::actionIds { "hit", "snap1", "snap2", "snap3", "snap4", "scenePrev", "sceneNext", "sceneGo", "drop" };
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
    ids.addArray ({ "lookAmount", "reactKick", "reactSnare", "reactHat", "reactBass", "reactLevel" });
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
    s.setProperty ("lookPreset", (int) state.getRawParameterValue ("lookPreset")->load(), nullptr);
    s.setProperty ("reactStyle", (int) state.getRawParameterValue ("reactStyle")->load(), nullptr);
    for (int i = 0; i < 3; ++i)
        s.setProperty ("colour" + juce::String (i), getCustomColour (i).toString(), nullptr);
    snapshots.addChild (s, -1, nullptr);
    activeSnapshot = slot;
}

void VJAnalyzerProcessor::clearSnapshot (int slot)
{
    auto snapshots = state.state.getChildWithName ("Snapshots");
    auto s = snapshots.getChildWithName ("S" + juce::String (slot));
    if (s.isValid())
        snapshots.removeChild (s, nullptr);
    if (activeSnapshot == slot)
        activeSnapshot = -1;
}

float VJAnalyzerProcessor::getMorphProgress() const
{
    if (! morph.active)
        return -1.0f;
    const auto now = juce::Time::getMillisecondCounterHiRes() * 0.001;
    return morph.seconds <= 0.0 ? 1.0f : (float) juce::jlimit (0.0, 1.0, (now - morph.start) / morph.seconds);
}

bool VJAnalyzerProcessor::getSnapshotRecipe (int slot, int& palette, int& look) const
{
    auto s = state.state.getChildWithName ("Snapshots").getChildWithName ("S" + juce::String (slot));
    if (! s.isValid())
        return false;
    palette = (int) s.getProperty ("palette", 0);
    look = (int) s.getProperty ("lookPreset", customLook);
    return true;
}

void VJAnalyzerProcessor::recallSnapshot (int slot)
{
    auto s = state.state.getChildWithName ("Snapshots").getChildWithName ("S" + juce::String (slot));
    if (! s.isValid())
        return;

    startMorph (s, false);
    activeSnapshot = slot;
    activeLook = {};
}

void VJAnalyzerProcessor::startMorph (const juce::ValueTree& s, bool cut)
{
    morph = {};
    for (auto& id : snapshotParamIds())
    {
        morph.from.add (state.getRawParameterValue (id)->load());
        morph.to.add ((float) s.getProperty (id, state.getRawParameterValue (id)->load()));
    }
    morph.toPalette = (int) s.getProperty ("palette", 0);
    morph.toLookPreset = (int) s.getProperty ("lookPreset", -1);
    morph.toStyle = (int) s.getProperty ("reactStyle", -1);
    lookMorph.active = false; // the moment owns the look knobs now
    for (int i = 0; i < 3; ++i)
        morph.toColours.add (s.getProperty ("colour" + juce::String (i)).toString());

    const double beatsFor[] = { 0.0, 1.0, 4.0, 16.0, 64.0 };
    const auto choice = juce::jlimit (0, 4, (int) state.getRawParameterValue ("morphTime")->load());
    morph.seconds = cut ? 0.0 : beatsFor[choice] * 60.0 / juce::jlimit (20.0, 400.0, hostBpm.load());
    morph.start = juce::Time::getMillisecondCounterHiRes() * 0.001;
    morph.active = true;
    advanceMorph(); // a cut lands immediately
}

//==============================================================================
// Looks (LookLibrary): a scene name + exactly what a Moment holds.

juce::String VJAnalyzerProcessor::sceneNameAt (int engineIndex) const
{
    const auto status = worker.getEngineStatus();
    if (status.connected && ! status.presetNames.isEmpty())
        return status.presetNames[engineIndex];
    return juce::StringArray::fromLines (state.state.getProperty ("sceneCache").toString())[engineIndex];
}

juce::String VJAnalyzerProcessor::currentSceneName() const
{
    const auto status = worker.getEngineStatus();
    if (status.connected && juce::isPositiveAndBelow (status.presetIndex, status.presetNames.size()))
        return status.presetNames[status.presetIndex];
    if (status.connected && status.presetName.isNotEmpty())
        return status.presetName;
    const auto preset = (int) state.getRawParameterValue ("preset")->load();
    return preset > 0 ? sceneNameAt (preset - 1) : juce::String();
}

int VJAnalyzerProcessor::findScene (const juce::String& name) const
{
    const auto status = worker.getEngineStatus();
    if (name.isEmpty() || ! status.connected)
        return -1;
    return status.presetNames.indexOf (name, true);
}

void VJAnalyzerProcessor::fillLookRecipe (LookLibrary::Look& l, int palette, int lookPreset, int style) const
{
    l.palette = palette;
    l.paletteName = paletteNames[palette];
    l.lookPreset = lookPreset;
    l.lookName = lookPreset >= 0 ? lookPresetNames[lookPreset] : juce::String();
    l.reactStyle = style;
    l.styleName = style >= 0 ? styleNames[style] : juce::String();
}

LookLibrary::Look VJAnalyzerProcessor::captureLook (const juce::String& name) const
{
    LookLibrary::Look l;
    l.name = name;
    l.scene = currentSceneName();
    for (auto& id : snapshotParamIds())
        l.values.set (id, state.getRawParameterValue (id)->load());
    fillLookRecipe (l, (int) state.getRawParameterValue ("palette")->load(), (int) state.getRawParameterValue ("lookPreset")->load(),
                    (int) state.getRawParameterValue ("reactStyle")->load());
    for (int i = 0; i < 3; ++i)
        l.colours.add (getCustomColour (i).toString());
    return l;
}

bool VJAnalyzerProcessor::captureMomentLook (int slot, const juce::String& name, LookLibrary::Look& l) const
{
    auto s = state.state.getChildWithName ("Snapshots").getChildWithName ("S" + juce::String (slot));
    if (! s.isValid())
        return false;
    l = {};
    l.name = name;
    l.scene = currentSceneName(); // a moment holds no scene: the look is built on the scene playing now
    for (auto& id : snapshotParamIds())
        l.values.set (id, (float) s.getProperty (id, state.getRawParameterValue (id)->load()));
    const auto palette = juce::jlimit (0, paletteNames.size() - 1, (int) s.getProperty ("palette", 0));
    const auto look = (int) s.getProperty ("lookPreset", -1);
    const auto style = (int) s.getProperty ("reactStyle", -1);
    fillLookRecipe (l, palette, juce::isPositiveAndBelow (look, lookPresetNames.size()) ? look : -1,
                    juce::isPositiveAndBelow (style, styleNames.size()) ? style : -1);
    for (int i = 0; i < 3; ++i)
        l.colours.add (s.getProperty ("colour" + juce::String (i), getCustomColour (i).toString()).toString());
    return true;
}

juce::ValueTree VJAnalyzerProcessor::lookToSnapshot (const LookLibrary::Look& l)
{
    // Names first (stable if a choice list is ever re-ordered), stored index second.
    auto resolve = [] (const juce::StringArray& names, const juce::String& name, int index, int fallback) {
        if (auto i = names.indexOf (name, true); name.isNotEmpty() && i >= 0)
            return i;
        return juce::isPositiveAndBelow (index, names.size()) ? index : fallback;
    };
    juce::ValueTree s ("S");
    for (auto& v : l.values)
        s.setProperty (v.name, v.value, nullptr);
    s.setProperty ("palette", resolve (paletteNames, l.paletteName, l.palette, 0), nullptr);
    if (auto look = resolve (lookPresetNames, l.lookName, l.lookPreset, -1); look >= 0)
        s.setProperty ("lookPreset", look, nullptr);
    if (auto style = resolve (styleNames, l.styleName, l.reactStyle, -1); style >= 0)
        s.setProperty ("reactStyle", style, nullptr);
    for (int i = 0; i < juce::jmin (3, l.colours.size()); ++i)
        s.setProperty ("colour" + juce::String (i), l.colours[i], nullptr);
    return s;
}

std::array<juce::Colour, 3> VJAnalyzerProcessor::lookColours (const LookLibrary::Look& l) const
{
    const auto palette = (int) lookToSnapshot (l).getProperty ("palette", 0);
    if (isPresetPalette (palette))
        return presetPalette (palette);
    if (palette == sceneColours)
        return { juce::Colour (0xff202020), juce::Colour (0xff606060), juce::Colour (0xffb0b0b0) };
    std::array<juce::Colour, 3> c;
    for (int i = 0; i < 3; ++i)
        c[(size_t) i] = i < l.colours.size() ? juce::Colour::fromString (l.colours[i]) : presetPalette (0)[(size_t) i];
    return c;
}

juce::String VJAnalyzerProcessor::lookRecipe (const LookLibrary::Look& l)
{
    auto s = lookToSnapshot (l);
    const auto dot = juce::String (juce::CharPointer_UTF8 (" \xc2\xb7 "));
    juce::String text = lookPresetNames[juce::jlimit (0, lookPresetNames.size() - 1, (int) s.getProperty ("lookPreset", customLook))]
                      + dot + paletteNames[juce::jlimit (0, paletteNames.size() - 1, (int) s.getProperty ("palette", 0))];
    if (s.hasProperty ("reactStyle"))
        text << dot << styleNames[juce::jlimit (0, styleNames.size() - 1, (int) s.getProperty ("reactStyle"))];
    return text;
}

void VJAnalyzerProcessor::cueLook (const juce::String& id)
{
    cuedLook = id;
    if (id.isNotEmpty())
        cuedScene = -1;
}

bool VJAnalyzerProcessor::applyLook (const juce::String& id)
{
    looks->refreshIfChanged();
    auto* look = looks->find (id);
    if (look == nullptr)
        return false;
    const auto copy = *look;
    applyLookObject (copy, false);
    activeLook = id;
    return true;
}

void VJAnalyzerProcessor::applyLookObject (const LookLibrary::Look& look, bool cut)
{
    cuedLook = {};
    pendingScene = {};
    if (look.scene.isNotEmpty())
    {
        const auto status = worker.getEngineStatus();
        if (status.connected && ! status.presetNames.isEmpty())
        {
            // Scene by NAME. A scene that no longer exists: the values still land on the current one.
            const auto index = status.presetNames.indexOf (look.scene, true);
            const auto fired = firedScene.load();
            if (index >= 0 && (index != status.presetIndex || (fired >= 0 && fired != index)))
                fireScene (index, false); // the look's own values, not the scene default
        }
        else
            pendingScene = look.scene; // fired as soon as the engine lists its scenes
    }
    startMorph (lookToSnapshot (look), cut);
    activeSnapshot = -1;
}

void VJAnalyzerProcessor::applySceneDefault (int engineIndex)
{
    looks->refreshIfChanged();
    if (auto* d = looks->sceneDefault (sceneNameAt (engineIndex)))
    {
        const auto id = d->getId();
        startMorph (lookToSnapshot (*d), false);
        activeSnapshot = -1;
        activeLook = id;
    }
}

void VJAnalyzerProcessor::watchPresetParameter()
{
    // "preset" moved without fireScene (Live automation, a MIDI-mapped knob,
    // the device's parameter list): that scene is being fired too.
    const auto preset = (int) state.getRawParameterValue ("preset")->load();
    if (stateLoaded.exchange (false))
    {
        lastPresetSeen = preset; // a loaded Live set keeps its own knobs
        return;
    }
    if (preset != lastPresetSeen)
    {
        lastPresetSeen = preset;
        if (preset > 0)
        {
            activeLook = {};
            applySceneDefault (preset - 1);
        }
    }
}

void VJAnalyzerProcessor::resolvePendingScene()
{
    const auto status = worker.getEngineStatus();
    if (! status.connected || status.presetNames.isEmpty())
        return;

    // Remember the scene list with the set (scene names while the engine is off).
    auto joined = status.presetNames.joinIntoString ("\n");
    if (state.state.getProperty ("sceneCache").toString() != joined)
        state.state.setProperty ("sceneCache", joined, nullptr);

    if (pendingScene.isEmpty())
        return;
    const auto index = status.presetNames.indexOf (pendingScene, true);
    pendingScene = {};
    if (index >= 0 && index != status.presetIndex)
    {
        const auto keep = activeLook;
        fireScene (index, false);
        activeLook = keep;
    }
}

void VJAnalyzerProcessor::applyStartupLook()
{
    if (! startupPending)
        return;
    if (stateEverLoaded.load())
    {
        startupPending = false; // a saved Live set opens as it was saved
        return;
    }
    if (++timerTicks < 45) // ~1.5 s of a running message loop: Live restores a set before that
        return;
    startupPending = false;
    // Only the first VJ Analyzer of a new set, never a second one added later.
    // With audio running, only instances that really run count (Live keeps
    // deleted devices alive for undo); with audio off, it must be the only one.
    const bool meRunning = juce::Time::getMillisecondCounter() - lastProcessMs.load() < 2000;
    if (meRunning ? leads->anyOtherRunning (this) : leads->count() != 1)
        return;
    looks->refreshIfChanged();
    if (auto* l = looks->startupLook())
    {
        const auto copy = *l;
        applyLookObject (copy, true);
        activeLook = copy.getId();
    }
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
        if (morph.toLookPreset >= 0)
            if (auto* p = state.getParameter ("lookPreset"))
            {
                lastLookPreset = morph.toLookPreset; // the knobs are morphing there already: no second write
                p->setValueNotifyingHost (p->convertTo0to1 ((float) morph.toLookPreset));
            }
        if (morph.toStyle >= 0)
            if (auto* p = state.getParameter ("reactStyle"))
                p->setValueNotifyingHost (p->convertTo0to1 ((float) morph.toStyle));
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
    const float macroDefaults[] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.25f, 0.5f };
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
    layout.add (std::make_unique<AudioParameterInt> (ParameterID { "mediaSlot", 1 }, "Media Slot", 1, 8, 1));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "clipMode", 1 }, "Clip Mode", StringArray { "Loop", "Ping-Pong" }, 0));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "clipSync", 1 }, "Clip Sync", clipSyncNames, 0));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "shots", 1 }, "Shots", NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "hud", 1 }, "HUD", NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "lookahead", 1 }, "Visual Lookahead",
                                                       NormalisableRange<float> (0.0f, 150.0f, 1.0f), 0.0f,
                                                       AudioParameterFloatAttributes().withStringFromValueFunction (
                                                           [] (float v, int) { return v < 0.5f ? String ("Off") : String (roundToInt (v)) + " ms"; })));
    const char* actionNames[] = { "Hit", "Snapshot A", "Snapshot B", "Snapshot C", "Snapshot D", "Previous Scene", "Next Scene", "Go", "Drop" };
    for (int i = 0; i < 8; ++i)
        layout.add (std::make_unique<AudioParameterBool> (ParameterID { actionIds[i], 1 }, actionNames[i], false));

    // --- added 2026-09-26 (appended; nothing above moves)
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "reactStyle", 1 }, "React Style", styleNames, 1));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "lookPreset", 1 }, "Look", lookPresetNames, 2));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "lookAmount", 1 }, "Look Amount", NormalisableRange<float> (0.0f, 1.0f), 1.0f,
                                                       AudioParameterFloatAttributes().withStringFromValueFunction (percent)));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { actionIds[8], 1 }, actionNames[8], false));
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
    leads->add (this);
    startTimerHz (30);
}

void VJAnalyzerProcessor::makeLead()
{
    state.state.setProperty ("leadPin", (double) juce::Time::currentTimeMillis() * 0.001, nullptr); // wall clock: the latest pin wins
    if (auto* p = state.getParameter ("sendControls"))   // and it may lead, whatever an old set said
        p->setValueNotifyingHost (1.0f);
}

void VJAnalyzerProcessor::setRoleByHand (int role)
{
    state.state.setProperty ("roleAuto", false, nullptr);
    if (auto* p = state.getParameter ("role"))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 ((float) role));
        p->endChangeGesture();
    }
}

void VJAnalyzerProcessor::setRoleAuto()
{
    state.state.setProperty ("roleAuto", true, nullptr);
    applyRoleFromName();
}

void VJAnalyzerProcessor::updateTrackProperties (const TrackProperties& properties)
{
    if (properties.name.has_value())
    {
        trackName = *properties.name;
        applyRoleFromName();
    }
}

void VJAnalyzerProcessor::applyRoleFromName()
{
    if (! isRoleAuto() || trackName.isEmpty())
        return;
    // Whole words, first match wins (CONTROL-MAP 4.3); anything else keeps the role.
    auto words = juce::StringArray::fromTokens (trackName.toLowerCase().replaceCharacters ("-_.()[]0123456789", "                  "), " ", "");
    words.removeEmptyStrings();
    auto has = [&words] (std::initializer_list<const char*> list) {
        for (auto* w : list)
            if (words.contains (w))
                return true;
        return false;
    };
    const auto lower = trackName.toLowerCase();
    int role = -1;
    if (has ({ "kick", "bd", "kik" }) || lower.contains ("bass drum")) role = 1;
    else if (has ({ "snare", "sd", "clap", "rim" }))                     role = 2;
    else if (has ({ "hat", "hh", "hihat", "hats", "cymbal", "ride", "shaker" }) || lower.contains ("hi-hat") || lower.contains ("hi hat")) role = 3;
    else if (has ({ "bass", "sub" }) || lower.contains ("808"))          role = 4;
    else if (has ({ "master", "main", "mix", "bus" }))                  role = 0;
    if (role >= 0)
        if (auto* p = state.getParameter ("role"); p != nullptr && (int) state.getRawParameterValue ("role")->load() != role)
            p->setValueNotifyingHost (p->convertTo0to1 ((float) role));
}

void VJAnalyzerProcessor::updateLookPreset()
{
    // A running look morph (a named look was picked): advance it.
    if (lookMorph.active)
    {
        const auto now = juce::Time::getMillisecondCounterHiRes() * 0.001;
        auto t = lookMorph.seconds <= 0.0 ? 1.0 : juce::jlimit (0.0, 1.0, (now - lookMorph.start) / lookMorph.seconds);
        const auto eased = (float) (t * t * (3.0 - 2.0 * t));
        for (int i = 0; i < lookVectorIds.size(); ++i)
            if (auto* p = state.getParameter (lookVectorIds[i]))
                p->setValueNotifyingHost (p->convertTo0to1 (lookMorph.from[(size_t) i] + (lookMorph.to[(size_t) i] - lookMorph.from[(size_t) i]) * eased));
        if (t >= 1.0)
            lookMorph.active = false;
        return;
    }

    const auto preset = (int) state.getRawParameterValue ("lookPreset")->load();
    if (preset != lastLookPreset)
    {
        lastLookPreset = preset;
        if (preset != customLook && ! morph.active)
        {
            // A named look: morph the knobs there over one beat (never pops, even from the Korg).
            lookMorph.from = {};
            for (int i = 0; i < lookVectorIds.size(); ++i)
                lookMorph.from[(size_t) i] = state.getRawParameterValue (lookVectorIds[i])->load();
            lookMorph.to = lookVector (preset);
            lookMorph.seconds = 60.0 / juce::jlimit (20.0, 400.0, hostBpm.load());
            lookMorph.start = juce::Time::getMillisecondCounterHiRes() * 0.001;
            lookMorph.active = true;
        }
        return;
    }

    // Any knob edited away from the named look: it is Custom now.
    if (preset != customLook && ! morph.active)
    {
        const auto& v = lookVector (preset);
        for (int i = 0; i < lookVectorIds.size(); ++i)
            if (std::abs (state.getRawParameterValue (lookVectorIds[i])->load() - v[(size_t) i]) > 0.005f)
            {
                lastLookPreset = customLook;
                if (auto* p = state.getParameter ("lookPreset"))
                    p->setValueNotifyingHost (p->convertTo0to1 ((float) customLook));
                break;
            }
    }
}

VJAnalyzerProcessor::~VJAnalyzerProcessor()
{
    stopTimer();
    for (auto& id : actionIds)
        state.removeParameterListener (id, this);
    leads->remove (this);
    recordingWriter.reset(); // before recordingWriteThread tears down (see its declaration)
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

void VJAnalyzerProcessor::startRecording (double seconds, bool vertical)
{
    if (recording.load())
        return; // one at a time

    auto folder = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("VJ VST").getChildFile ("Recordings");
    folder.createDirectory();
    recordingFile = folder.getChildFile ("recording_" + juce::String (juce::Time::getCurrentTime().toMilliseconds()) + ".wav");

    std::unique_ptr<juce::FileOutputStream> stream (recordingFile.createOutputStream());
    if (stream == nullptr)
    {
        DBG ("VJAnalyzer: could not create " << recordingFile.getFullPathName());
        return;
    }

    juce::WavAudioFormat wavFormat;
    auto options = juce::AudioFormatWriterOptions().withSampleRate (lastSampleRate).withNumChannels (2).withBitsPerSample (24);
    std::unique_ptr<juce::OutputStream> streamBase (stream.release());
    auto writer = wavFormat.createWriterFor (streamBase, options);
    if (writer == nullptr)
        return;

    if (! recordingWriteThread.isThreadRunning())
        recordingWriteThread.startThread();

    // 32768 samples buffered before the background thread has to catch up -
    // standard JUCE ThreadedWriter idiom, keeps the audio thread's write() lock-free.
    recordingWriter = std::make_unique<juce::AudioFormatWriter::ThreadedWriter> (writer.release(), recordingWriteThread, 32768);

    worker.startRecording (seconds, vertical); // tells the engine (/record/start)
    recordDeadline = juce::Time::getMillisecondCounterHiRes() * 0.001 + seconds;
    recording = true;
}

void VJAnalyzerProcessor::stopRecording()
{
    if (! recording.exchange (false))
        return;

    recordingWriter.reset(); // flushes and closes the WAV
    worker.stopRecording();  // tells the engine (/record/stop) - it also times out on its own

    // Hand the WAV back to the engine so it can mux once its own video leg is
    // done - the exact samples processBlock saw, no WASAPI loopback needed
    // (REELS-RECORDING-PLAN.md #3).
    worker.sendAudioPath (recordingFile.getFullPathName());
}

void VJAnalyzerProcessor::checkRecordingTimeout()
{
    if (recording.load() && juce::Time::getMillisecondCounterHiRes() * 0.001 >= recordDeadline)
        stopRecording();
}

void VJAnalyzerProcessor::releaseResources()
{
    worker.release();
}

void VJAnalyzerProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    lastProcessMs = juce::Time::getMillisecondCounter(); // this instance is really running (LeadRegistry)

    // Audio is untouched (unless Visual Lookahead delays it, below): this is an analyser.
    const auto numSamples = buffer.getNumSamples();
    const float* left = buffer.getNumChannels() > 0 ? buffer.getReadPointer (0) : nullptr;
    const float* right = buffer.getNumChannels() > 1 ? buffer.getReadPointer (1) : nullptr;
    if (left != nullptr)
    {
        worker.pushAudio (left, right, numSamples);

        // Reels recording: the WAV is fed the same buffer the analyser sees,
        // not a WASAPI loopback tap - simpler, and it's already gated on
        // isOnMasterTrack() so this is genuinely the full mix (see startRecording).
        if (recording.load() && recordingWriter != nullptr)
        {
            const float* channels[2] { left, right != nullptr ? right : left };
            recordingWriter->write (channels, numSamples);
        }
    }

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
    if (index == 8)
        worker.sendDrop();
    actionPending[(size_t) index] = true;
}

void VJAnalyzerProcessor::cueScene (int engineIndex)
{
    cuedScene = engineIndex;
    cuedLook = {}; // one cue at a time (and cueScene (-1) cancels a cued look too)
}

void VJAnalyzerProcessor::fireScene (int engineIndex, bool withSceneDefault)
{
    if (engineIndex < 0)
        return;
    cuedScene = -1;
    cuedLook = {};
    pendingScene = {};
    lastPresetSeen = engineIndex + 1; // our own change, not automation (watchPresetParameter)
    // Through the host parameter (so Live records/recalls it) and immediately.
    if (auto* param = state.getParameter ("preset"))
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost (param->convertTo0to1 ((float) (engineIndex + 1)));
        param->endChangeGesture();
    }
    worker.selectPresetNow (engineIndex);
    firedScene = engineIndex;
    firedAt = juce::Time::getMillisecondCounterHiRes() * 0.001;

    // A scene with a default look always opens on it; without one the knobs
    // carry over from the previous scene, as before.
    if (withSceneDefault)
    {
        activeLook = {};
        applySceneDefault (engineIndex);
    }
}

void VJAnalyzerProcessor::stepCue (int direction)
{
    cuedLook = {};
    const auto status = worker.getEngineStatus();
    const int n = status.numPresets;
    if (! status.connected || n <= 0)
    {
        worker.stepScene (direction); // no list to cue from: plain previous / next
        return;
    }
    auto base = cuedScene.load();
    if (base < 0) base = firedScene.load();
    if (base < 0) base = status.presetIndex;
    const auto target = (((base + direction) % n) + n) % n;
    cuedScene = target == status.presetIndex ? -1 : target;
}

void VJAnalyzerProcessor::goCue()
{
    if (cuedLook.isNotEmpty())
    {
        const auto id = cuedLook;
        cuedLook = {};
        applyLook (id);
        return;
    }
    fireScene (cuedScene.exchange (-1));
}

void VJAnalyzerProcessor::updateCue()
{
    const auto fired = firedScene.load();
    if (fired < 0 && cuedScene.load() < 0)
        return;
    const auto live = worker.getEngineStatus().presetIndex;
    // Fired: shown as "switching" until the engine reports it live (or gives up).
    if (fired >= 0 && (fired == live || juce::Time::getMillisecondCounterHiRes() * 0.001 - firedAt > 4.0))
        firedScene = -1;
    // Cueing the scene that is already live means nothing is cued.
    if (cuedScene.load() == live)
        cuedScene = -1;
}

void VJAnalyzerProcessor::timerCallback()
{
    updateLatency();
    checkRecordingTimeout();

    for (int i = 0; i < actionIds.size(); ++i)
    {
        if (! actionPending[(size_t) i].exchange (false))
            continue;
        if (i >= 1 && i <= 4)
            recallSnapshot (i - 1);
        else if (i == 5 || i == 6)
            stepCue (i == 5 ? -1 : 1);
        else if (i == 7)
            goCue();
        // Reset, so the next press (whatever the controller sends) fires again.
        if (auto* p = state.getParameter (actionIds[i]))
            p->setValueNotifyingHost (0.0f);
    }

    watchPresetParameter();
    resolvePendingScene();
    applyStartupLook();
    advanceMorph();
    updateLookPreset();
    updateCue();

    {
        LeadRegistry::Info info;
        info.canLead = state.getRawParameterValue ("sendControls")->load() > 0.5f;
        info.role = (int) state.getRawParameterValue ("role")->load();
        info.trackName = trackName;
        info.pinTime = (double) state.state.getProperty ("leadPin", 0.0);
        info.running = juce::Time::getMillisecondCounter() - lastProcessMs.load() < 2000;
        leads->update (this, info);
    }

    AnalysisWorker::Controls c;
    c.role = (int) state.getRawParameterValue ("role")->load();
    c.sensitivity = state.getRawParameterValue ("sensitivity")->load();
    c.trimDb = state.getRawParameterValue ("trim")->load();
    c.lockedNormalizer = state.getRawParameterValue ("normalizer")->load() > 0.5f;
    c.sendControls = leads->isLead (this); // exactly one instance sends the controls
    for (int i = 0; i < 8; ++i)
        c.macros[(size_t) i] = state.getRawParameterValue ("macro" + juce::String (i + 1))->load();
    c.preset = (int) state.getRawParameterValue ("preset")->load();
    c.blackout = state.getRawParameterValue ("blackout")->load() > 0.5f;
    // Look Amount scales the look on the way out; Cut Rate (it switches
    // scenes) and the Reactivity trim are not part of the look.
    const auto lookAmount = state.getRawParameterValue ("lookAmount")->load();
    for (int i = 0; i < lookIds.size(); ++i)
        c.look[(size_t) i] = state.getRawParameterValue (lookIds[i])->load() * (i == 6 || i == 12 ? 1.0f : lookAmount);
    c.look[13] = state.getRawParameterValue ("calm")->load() > 0.5f ? 1.0f : 0.0f;
    c.look[14] = state.getRawParameterValue ("shots")->load() * lookAmount;
    c.look[15] = state.getRawParameterValue ("hud")->load() * lookAmount;
    c.style = (int) state.getRawParameterValue ("reactStyle")->load();
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
    for (int s = 0; s < (int) c.mediaPaths.size(); ++s)
        c.mediaPaths[(size_t) s] = getMediaPath (s);
    c.mediaSlot = getMediaSlot();
    c.clipMode = (int) state.getRawParameterValue ("clipMode")->load();
    c.clipSyncBeats = clipSyncBeats ((int) state.getRawParameterValue ("clipSync")->load());
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
        {
            // The restored "preset" is not a scene being fired (no scene default),
            // and a loaded set never gets the start-up look.
            stateEverLoaded = true;
            stateLoaded = true;
            state.replaceState (juce::ValueTree::fromXml (*xml));
            stateLoaded = true;
            // The loaded look is whatever was saved: never re-write it; if it no
            // longer matches its named look, it shows as Custom (updateLookPreset).
            lastLookPreset = (int) state.getRawParameterValue ("lookPreset")->load();
            lookMorph.active = false;
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VJAnalyzerProcessor();
}
