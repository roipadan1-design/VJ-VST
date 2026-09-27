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
#include "../Source/PluginEditor.h"

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

    // Looks live in a scratch folder here, never in the owner's Documents\VJ VST\Looks.
    juce::SharedResourcePointer<LookLibrary> library;
    const auto looksDir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("vjvst_looks_selftest");
    looksDir.deleteRecursively();
    library->setFolder (looksDir);

    // Start-up look: a brand-new instance (nothing loaded, the only one) opens on it; a loaded set or a second instance never does.
    {
        LookLibrary::Look boot;
        boot.name = "Boot";
        boot.values.set ("macro1", 0.33f);
        boot.values.set ("grain", 0.61f);
        boot.paletteName = "Ice";
        boot.startup = true;
        const auto bootId = library->add (boot);
        check (bootId.isNotEmpty() && looksDir.getChildFile ("Boot.json").existsAsFile(), "a look is one JSON file in the looks folder");
        {
            VJAnalyzerProcessor fresh;
            pump (2.0);
            auto& fs = fresh.getState();
            check (std::abs (fs.getRawParameterValue ("macro1")->load() - 0.33f) < 0.01f
                       && (int) fs.getRawParameterValue ("palette")->load() == 3,
                   "a new instance opens on the start-up look");
            check (fresh.getActiveLook() == bootId, "the start-up look shows as the active look");
        }
        {
            VJAnalyzerProcessor first, second;
            first.getState().getParameter ("macro1")->setValueNotifyingHost (0.9f);
            juce::MemoryBlock set;
            first.getStateInformation (set);
            VJAnalyzerProcessor loaded;
            loaded.setStateInformation (set.getData(), (int) set.getSize());
            pump (2.0);
            check (std::abs (second.getState().getRawParameterValue ("macro1")->load() - 0.5f) < 0.01f,
                   "a second instance in the same set does not get the start-up look");
            check (std::abs (loaded.getState().getRawParameterValue ("macro1")->load() - 0.9f) < 0.01f,
                   "a loaded Live set keeps its own values (no start-up look)");
        }
        library->setStartup (bootId, false);
        check (library->startupLook() == nullptr, "start-up look can be cleared");
    }

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

    // Scene cue: a click only marks NEXT; GO (the MIDI-mappable parameter) fires it.
    processor.cueScene (3);
    pump (0.1);
    check (processor.getCuedScene() == 3 && (int) get ("preset") == 0, "cueing a scene does not switch it");
    set ("sceneGo", 1.0f);
    pump (0.1);
    check ((int) get ("preset") == 4 && processor.getCuedScene() == -1 && processor.getFiredScene() == 3,
           "GO fires the cued scene (preset 4 = engine scene 3)");
    set ("sceneGo", 1.0f);
    pump (0.1);
    check ((int) get ("preset") == 4, "GO with nothing cued does nothing");

    // LOOK presets: a named look writes the knobs (one-beat morph); editing one makes it Custom.
    set ("lookPreset", 3.0f); // Worn
    pump (0.9);
    check (std::abs (get ("grain") - 0.5f) < 0.01f && std::abs (get ("dust") - 0.65f) < 0.01f, "look Worn writes its knobs");
    check ((int) get ("lookPreset") == 3, "look stays Worn while untouched");
    set ("grain", 0.9f);
    pump (0.1);
    check ((int) get ("lookPreset") == 0, "editing a look knob makes the look Custom");

    // Lead election: the first instance leads; a pinned one takes over.
    check (processor.isLead(), "a single instance leads");
    {
        VJAnalyzerProcessor second;
        pump (0.1);
        check (processor.isLead() && ! second.isLead(), "a second instance does not lead");
        second.makeLead();
        pump (0.1);
        check (second.isLead() && ! processor.isLead(), "MAKE LEAD moves the lead");
    }
    pump (0.1);
    check (processor.isLead(), "the lead returns when the other instance is removed");

    // Role from the track name, until picked by hand.
    {
        juce::AudioProcessor::TrackProperties props;
        props.name = juce::String ("Kick 808");
        processor.updateTrackProperties (props);
        check ((int) get ("role") == 1, "track 'Kick 808' -> KICK");
        processor.setRoleByHand (2);
        props.name = juce::String ("Hats");
        processor.updateTrackProperties (props);
        check ((int) get ("role") == 2 && ! processor.isRoleAuto(), "a role picked by hand stays");
        processor.setRoleAuto();
        check ((int) get ("role") == 3, "AUTO follows the name again ('Hats' -> HAT)");
        processor.setRoleByHand (0);
    }

    // --- Looks: save / favourite / rename / duplicate / delete, cue + GO, scene defaults.
    {
        set ("morphTime", 0.0f); // cut, so values land at once
        st.state.setProperty ("sceneCache", "Hot Blobs\nDot Relief\nOne Bit\nCorridor\nFibers", nullptr); // engine off: names from the cache
        processor.fireScene (3); // Corridor
        pump (0.1);
        check (processor.currentSceneName() == "Corridor", "the current scene is known by name");

        set ("macro1", 0.3f); set ("grain", 0.2f); set ("palette", 8.0f); set ("reactStyle", 2.0f);
        auto look = processor.captureLook ("Night");
        LookLibrary::Look parsed;
        check (LookLibrary::fromJson (LookLibrary::toJson (look), parsed) && parsed.scene == "Corridor"
                   && std::abs ((float) parsed.values["macro1"] - 0.3f) < 1e-4f && parsed.paletteName == "Cyanotype"
                   && parsed.styleName == "Punch" && parsed.colours.size() == 3,
               "a look holds the scene name, knobs, palette, react style and colours (JSON round trip)");
        const auto night = library->add (look);
        const auto night2 = library->add (look);
        check (library->find (night2) != nullptr && library->find (night2)->name == "Night 2", "the same name twice becomes 'Night 2'");

        library->setFavourite (night2, true);
        check (library->getAll().front().getId() == night2, "a favourite sorts first");

        // Cue a look, GO fires it: scene by name + knobs.
        set ("macro1", 0.9f); set ("grain", 0.8f); set ("palette", 0.0f); set ("reactStyle", 0.0f);
        processor.fireScene (1); // Dot Relief (no default): knobs carry over
        pump (0.1);
        check (std::abs (get ("macro1") - 0.9f) < 0.01f, "a scene without a default keeps the knobs (as before)");
        processor.cueLook (night);
        pump (0.1);
        check (processor.getCuedLook() == night && (int) get ("preset") == 2, "cueing a look does not switch anything");
        set ("sceneGo", 1.0f);
        pump (0.1);
        check (std::abs (get ("macro1") - 0.3f) < 0.01f && std::abs (get ("grain") - 0.2f) < 0.01f
                   && (int) get ("palette") == 8 && (int) get ("reactStyle") == 2 && processor.getCuedLook().isEmpty(),
               "GO on a cued look recalls its knobs, palette and style");
        check ((int) get ("preset") == 2 && processor.getPendingScene() == "Corridor",
               "engine off: the look's scene waits (by name) until the engine lists its scenes");
        check (processor.getActiveLook() == night, "the recalled look is the active look");

        // Scene default: Corridor opens on 'Night' from now on.
        library->setSceneDefault (night, true);
        library->setSceneDefault (night2, true);
        check (library->sceneDefault ("corridor") != nullptr && library->sceneDefault ("Corridor")->getId() == night2
                   && ! library->find (night)->sceneDefault,
               "one default per scene (the newest wins, the other is cleared)");
        set ("macro1", 0.7f);
        processor.fireScene (0); // Hot Blobs: no default
        pump (0.1);
        check (std::abs (get ("macro1") - 0.7f) < 0.01f, "still carry-over for scenes without a default");
        processor.fireScene (3); // Corridor: default Night 2
        pump (0.1);
        check (std::abs (get ("macro1") - 0.3f) < 0.01f && processor.getActiveLook() == night2, "firing a scene with a default opens on it");
        set ("macro1", 0.75f);
        set ("preset", 2.0f); // host automation to Dot Relief
        pump (0.1);
        check (std::abs (get ("macro1") - 0.75f) < 0.01f, "automation to a scene without a default keeps the knobs");
        set ("preset", 4.0f); // automation back to Corridor
        pump (0.1);
        check (std::abs (get ("macro1") - 0.3f) < 0.01f, "automation to a scene with a default opens on it too");

        // A look whose scene is gone still recalls its knobs.
        auto orphan = look;
        orphan.name = "Orphan";
        orphan.scene = "No Such Scene";
        orphan.values.set ("macro1", 0.44f);
        const auto orphanId = library->add (orphan);
        processor.applyLook (orphanId);
        pump (0.1);
        check (std::abs (get ("macro1") - 0.44f) < 0.01f && (int) get ("preset") == 4, "a look of a missing scene lands its knobs on the current scene");

        // Rename / duplicate / delete.
        const auto renamed = library->rename (night2, "Night Blue");
        check (renamed.isNotEmpty() && library->find (night2) == nullptr && library->find (renamed)->name == "Night Blue"
                   && library->find (renamed)->sceneDefault && library->find (renamed)->favourite
                   && looksDir.getChildFile ("Night Blue.json").existsAsFile() && ! looksDir.getChildFile ("Night 2.json").existsAsFile(),
               "rename moves the file and keeps favourite / default");
        const auto copy = library->duplicate (renamed);
        check (copy.isNotEmpty() && library->find (copy)->name == "Night Blue copy" && ! library->find (copy)->sceneDefault,
               "duplicate makes a copy that is not the scene default");
        const auto before = (int) library->getAll().size();
        check (library->remove (copy) && (int) library->getAll().size() == before - 1 && library->find (copy) == nullptr, "delete removes it from the library");

        // A moment becomes a look.
        set ("macro1", 0.12f);
        processor.storeSnapshot (2);
        set ("macro1", 0.5f);
        LookLibrary::Look fromMoment;
        check (processor.captureMomentLook (2, "From C", fromMoment) && std::abs ((float) fromMoment.values["macro1"] - 0.12f) < 1e-4f
                   && fromMoment.scene == "Corridor",
               "Save to library: a moment becomes a look on the scene playing now");
        processor.clearSnapshot (2);

        // Files dropped in / removed by hand are picked up.
        looksDir.getChildFile ("Night.json").copyFileTo (looksDir.getChildFile ("Hand Copy.json"));
        pump (0.6);
        library->refreshIfChanged();
        check (library->find ("Hand Copy.json") != nullptr, "a look file copied in by hand appears");

        library->setSceneDefault (renamed, false);
        check (library->sceneDefault ("Corridor") == nullptr, "a scene default can be cleared");
        set ("macro1", 0.66f);
        processor.fireScene (3);
        pump (0.1);
        check (std::abs (get ("macro1") - 0.66f) < 0.01f, "after clearing, the scene keeps the knobs again");
    }

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

    // --cuetest out.png: with a running engine, clicks a scene in the list
    // (cue), snapshots the UI, presses GO and reports how long the engine
    // took to switch (should be at most one beat at the fake 120 BPM).
    const bool cueTest = argc > 1 && juce::String (argv[1]) == "--cuetest";
    if (cueTest) { --argc; ++argv; }
    // --lookstest out.png: the Looks click paths (scratch looks folder, never the
    // owner's): right-click the scene name, type a name + Enter, star it, cue + GO
    // it from the LOOKS grid, scene default via the scene grid. PNGs of each step.
    const bool looksTest = argc > 1 && juce::String (argv[1]) == "--lookstest";
    if (looksTest) { --argc; ++argv; }
    juce::SharedResourcePointer<LookLibrary> library;
    const auto looksDir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("vjvst_looks_uitest");
    if (looksTest)
    {
        looksDir.deleteRecursively();
        library->setFolder (looksDir);
    }
    // --tab N: open the EDIT drawer on tab N (0 LOOK .. 5 SETUP) before the snapshot.
    int drawerTab = -1;
    if (argc > 2 && juce::String (argv[1]) == "--tab") { drawerTab = juce::String (argv[2]).getIntValue(); argc -= 2; argv += 2; }
    // --source: another instance leads, so this one shows the SOURCE view.
    std::unique_ptr<VJAnalyzerProcessor> otherLead;
    if (argc > 1 && juce::String (argv[1]) == "--source") { otherLead = std::make_unique<VJAnalyzerProcessor>(); --argc; ++argv; }
    int cueTarget = -1;
    double goAt = 0.0, switchedAt = 0.0;

    juce::String outPath = argc > 1 ? juce::String (argv[1]) : juce::String ("plugin_ui.png");
    double seconds = argc > 2 ? juce::String (argv[2]).getDoubleValue() : 6.0;

    const double rate = 48000.0;
    const int block = 256;

    VJAnalyzerProcessor processor;
    FakePlayHead head;
    processor.setPlayHead (&head);
    processor.setPlayConfigDetails (2, 2, rate, block);
    processor.prepareToPlay (rate, block);

    if (drawerTab >= 0)
    {
        processor.getState().state.setProperty ("drawerOpen", true, nullptr);
        processor.getState().state.setProperty ("drawerTab", drawerTab, nullptr);
    }
    {
        juce::AudioProcessor::TrackProperties props;
        props.name = juce::String (otherLead != nullptr ? "Kick 808" : "Master");
        processor.updateTrackProperties (props);
    }
    std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());
    editor->setVisible (true);

    // Optional 3rd argument: an image / clip for media slot 1, then the USE
    // MEDIA button is clicked (as a user would) once the engine is talking.
    const juce::String mediaFile = argc > 3 ? juce::String (argv[3]) : juce::String();
    bool mediaDone = mediaFile.isEmpty();

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

        if (! mediaDone && samples > (int64_t) (3.5 * rate) && processor.getWorker().getEngineStatus().connected)
        {
            mediaDone = true;
            processor.setMediaPath (0, juce::File::getCurrentWorkingDirectory().getChildFile (mediaFile).getFullPathName());
            for (auto* child : editor->getChildren())
                if (auto* b = dynamic_cast<juce::TextButton*> (child); b != nullptr && b->getButtonText() == "USE MEDIA")
                    b->triggerClick();
        }

        if (cueTest)
        {
            auto status = processor.getWorker().getEngineStatus();
            const auto t = samples / rate;
            if (cueTarget < 0 && t > 3.0 && status.connected && status.numPresets > 2)
            {
                cueTarget = (status.presetIndex + 2) % status.numPresets;
                if (auto* grid = dynamic_cast<SceneGrid*> (editor->findChildWithID ("sceneList")))
                {
                    // Click the tile as a user would (mouse down on the grid at the tile's centre).
                    auto centre = grid->tileBounds (cueTarget).getCentre().toFloat();
                    juce::MouseEvent e (juce::Desktop::getInstance().getMainMouseSource(), centre, {}, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                        grid, grid, juce::Time::getCurrentTime(), centre, juce::Time::getCurrentTime(), 1, false);
                    grid->mouseDown (e);
                }
                std::printf ("clicked scene %d, cued = %d, live = %d\n", cueTarget, processor.getCuedScene(), status.presetIndex);
            }
            else if (cueTarget >= 0 && goAt == 0.0 && t > 4.0)
            {
                auto shot = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);
                juce::File cuedFile (juce::File::getCurrentWorkingDirectory().getChildFile (outPath).withFileExtension ("").getFullPathName() + "-cued.png");
                cuedFile.deleteFile();
                juce::FileOutputStream cuedStream (cuedFile);
                juce::PNGImageFormat().writeImageToStream (shot, cuedStream);
                std::printf ("still live = %d before GO (cued %d)\n", status.presetIndex, processor.getCuedScene());
                if (auto* p = processor.getState().getParameter ("sceneGo"))
                    p->setValueNotifyingHost (1.0f);
                goAt = juce::Time::getMillisecondCounterHiRes();
            }
            else if (goAt > 0.0 && switchedAt == 0.0 && status.presetIndex == cueTarget)
            {
                switchedAt = juce::Time::getMillisecondCounterHiRes();
                std::printf ("GO -> plug-in sees the switch: %.0f ms (one beat = 500 ms)\n", switchedAt - goAt);
            }
        }

        if (looksTest)
        {
            auto* ed = dynamic_cast<VJAnalyzerEditor*> (editor.get());
            auto status = processor.getWorker().getEngineStatus();
            const auto t = samples / rate;
            auto shoot = [&] (const char* suffix) {
                auto shot = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
                juce::File f (juce::File::getCurrentWorkingDirectory().getChildFile (outPath).withFileExtension ("").getFullPathName() + suffix + ".png");
                f.deleteFile();
                juce::FileOutputStream s (f);
                juce::PNGImageFormat().writeImageToStream (shot, s);
            };
            auto click = [] (juce::Component& c, juce::Point<int> p, bool right, int clicks = 1) {
                const auto pos = p.toFloat();
                juce::MouseEvent e (juce::Desktop::getInstance().getMainMouseSource(), pos,
                                    right ? juce::ModifierKeys (juce::ModifierKeys::rightButtonModifier) : juce::ModifierKeys(),
                                    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, &c, &c, juce::Time::getCurrentTime(), pos, juce::Time::getCurrentTime(), clicks, false);
                c.mouseDown (e);
                if (clicks == 2 && ! right)
                    c.mouseDoubleClick (e);
            };
            auto knob = [&processor] (const char* id) { return processor.getState().getRawParameterValue (id)->load(); };
            auto setKnob = [&processor] (const char* id, float v) {
                auto* p = processor.getState().getParameter (id);
                p->setValueNotifyingHost (p->convertTo0to1 (v));
            };
            static int step = 0;
            static juce::String lookScene;
            static double stepAt = 0.0;
            auto& grid = ed->getLookGrid();
            auto& scenes = ed->getSceneGrid();

            if (step == 0 && t > 3.0 && (status.connected || t > 5.0))
            {
                std::printf ("engine %s, live scene '%s'\n", status.connected ? "connected" : "NOT connected", status.presetName.toRawUTF8());
                // The switch: click LOOKS (middle cell), as a user would.
                if (auto* sw = ed->findChildWithID ("gridSwitch"))
                    click (*sw, { sw->getWidth() / 2, sw->getHeight() / 2 }, false);
                std::printf ("LOOKS view: %s, look grid visible: %s\n", ed->getGridView() == 1 ? "yes" : "NO", grid.isVisible() ? "yes" : "NO");
                shoot ("-1-empty");
                // Right-click the live scene's name: a menu must open (its "Save look..." = saveCurrentLook).
                click (*ed, { 120, 76 }, true);
                const bool menu = juce::PopupMenu::dismissAllActiveMenus();
                std::printf ("right-click on the scene name opened a menu: %s\n", menu ? "yes" : "NO");
                setKnob ("macro1", 0.21f);
                setKnob ("palette", 3.0f); // Ice
                lookScene = processor.currentSceneName();
                ed->saveCurrentLook (false);
                std::printf ("name field open: %s, pre-filled '%s'\n", ed->isNaming() ? "yes" : "NO",
                             dynamic_cast<juce::TextEditor*> (ed->findChildWithID ("lookName"))->getText().toRawUTF8());
                shoot ("-2-naming");
                step = 1;
                stepAt = t;
            }
            else if (step == 1 && t > stepAt + 0.5)
            {
                auto* field = dynamic_cast<juce::TextEditor*> (ed->findChildWithID ("lookName"));
                field->setText ("Test Ice", false);
                field->keyPressed (juce::KeyPress (juce::KeyPress::returnKey)); // Enter (JUCE delivers it on the next message loop turn)
                step = 2;
                stepAt = t;
            }
            else if (step == 2 && t > stepAt + 0.1)
            {
                std::printf ("after Enter: naming %s, looks in library %d, tile 0 '%s' on '%s'\n", ed->isNaming() ? "STILL OPEN" : "closed",
                             (int) library->getAll().size(), grid.getState().tiles.empty() ? "-" : grid.getState().tiles[0].name.toRawUTF8(),
                             grid.getState().tiles.empty() ? "-" : grid.getState().tiles[0].scene.toRawUTF8());
                shoot ("-3-saved"); // the SAVED flash
                // A second look (different knobs), then star the first.
                setKnob ("macro1", 0.8f);
                setKnob ("palette", 1.0f); // Ember
                ed->saveCurrentLook (false);
                ed->commitName ("Test Ember");
                const auto idx = grid.indexOf ("Test Ice.json");
                click (grid, grid.starBounds (idx).getCentre(), false);
                std::printf ("star clicked: 'Test Ice' favourite = %s, first tile now '%s'\n",
                             library->find ("Test Ice.json") != nullptr && library->find ("Test Ice.json")->favourite ? "yes" : "NO",
                             grid.getState().tiles[0].name.toRawUTF8());
                // Move away: another scene and other knobs.
                if (status.connected && status.numPresets > 2)
                    processor.fireScene ((status.presetIndex + 3) % status.numPresets);
                setKnob ("macro1", 0.95f);
                setKnob ("palette", 0.0f);
                step = 3;
                stepAt = t;
            }
            else if (step == 3 && t > stepAt + 1.5)
            {
                std::printf ("moved away to '%s', macro1 %.2f\n", status.presetName.toRawUTF8(), knob ("macro1"));
                setKnob ("morphTime", 0.0f); // cut, so the result is immediate
                const auto idx = grid.indexOf ("Test Ice.json");
                click (grid, grid.tileBounds (idx).getCentre().translated (-20, 0), false); // click = cue
                std::printf ("clicked the look tile: cued look '%s', live scene unchanged '%s'\n", processor.getCuedLook().toRawUTF8(),
                             status.presetName.toRawUTF8());
                step = 4;
                stepAt = t;
            }
            else if (step == 4 && t > stepAt + 0.3)
            {
                shoot ("-4-cued");
                processor.getState().getParameter ("sceneGo")->setValueNotifyingHost (1.0f); // GO (as the button / MIDI does)
                step = 5;
                stepAt = t;
            }
            else if (step == 5 && t > stepAt + 1.5)
            {
                std::printf ("after GO: live scene '%s' (look's scene '%s'), macro1 %.2f (look 0.21), palette %d (look 3 = Ice)\n",
                             status.presetName.toRawUTF8(), lookScene.toRawUTF8(), knob ("macro1"), (int) knob ("palette"));
                shoot ("-5-after-go");
                // Right-click the look tile: its menu opens. Then "Set as this scene's default" (the menu's action).
                const auto idx = grid.indexOf ("Test Ice.json");
                click (grid, grid.tileBounds (idx).getCentre(), true);
                std::printf ("right-click on a look tile opened a menu: %s\n", juce::PopupMenu::dismissAllActiveMenus() ? "yes" : "NO");
                library->setSceneDefault ("Test Ice.json", true);
                // Leave the scene, turn the knob, come back by double-clicking the scene tile.
                if (status.connected && status.numPresets > 2)
                    processor.fireScene ((status.presetIndex + 1) % status.numPresets);
                setKnob ("macro1", 0.6f);
                step = 6;
                stepAt = t;
            }
            else if (step == 6 && t > stepAt + 1.5)
            {
                std::printf ("left to '%s' (no default): macro1 %.2f (carried over: expect 0.60)\n", status.presetName.toRawUTF8(), knob ("macro1"));
                if (auto* sw = ed->findChildWithID ("gridSwitch"))
                    click (*sw, { sw->getWidth() / 6, sw->getHeight() / 2 }, false); // SCENES
                const auto target = status.presetNames.indexOf (lookScene, true);
                if (target >= 0)
                    click (scenes, scenes.tileBounds (target).getCentre(), false, 2); // double-click = cue + GO
                step = 7;
                stepAt = t;
            }
            else if (step == 7 && t > stepAt + 1.5)
            {
                std::printf ("back on '%s' (default 'Test Ice'): macro1 %.2f (expect 0.21), scene tile has default dot: %s\n",
                             status.presetName.toRawUTF8(), knob ("macro1"),
                             scenes.getState().hasDefault[juce::jmax (0, status.presetIndex)] ? "yes" : "NO");
                shoot ("-6-scenes-default-dot");
                if (auto* sw = ed->findChildWithID ("gridSwitch"))
                    click (*sw, { sw->getWidth() * 5 / 6, sw->getHeight() / 2 }, false); // favourites
                std::printf ("star view: %d tile(s) (expect 1)\n", (int) grid.getState().tiles.size());
                // Moment A -> library.
                processor.storeSnapshot (0);
                ed->saveMomentToLibrary (0);
                ed->commitName ({}); // Enter on the suggested name
                std::printf ("moment A saved to library: %d looks now (expect 3)\n", (int) library->getAll().size());
                step = 8;
            }
        }

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
    std::printf ("snapshot: %s\nengine: %s  preset: %s  fps: %.0f  presets listed: %d  media slot 1: '%s' %dx%d\n",
                 out.getFullPathName().toRawUTF8(), status.connected ? "connected" : "not connected",
                 status.presetName.toRawUTF8(), status.fps, status.presetNames.size(),
                 status.media[0].name.toRawUTF8(), status.media[0].width, status.media[0].height);

    editor = nullptr;
    processor.releaseResources();
    return 0;
}
