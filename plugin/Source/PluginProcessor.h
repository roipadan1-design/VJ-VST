#pragma once

#include <JuceHeader.h>
#include "AnalysisWorker.h"
#include "LeadRegistry.h"
#include "LookLibrary.h"

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
    // drive the engine's scene clock; React (slot 4, ID macro5 - it was Impact)
    // is how much the music moves the picture: 0 still, 50 as designed, 100 wild.
    static const juce::StringArray macroNames;

    // MOVE / REACT globals (engine /v2/move slots, same order): Drift, Push,
    // Softness, Sync, Reverse, Freeze.
    static const juce::StringArray moveIds, moveNames;

    // Momentary actions as host parameters, so a MIDI controller can press
    // them (Live's MIDI Map): HIT, recall snapshot A-D, previous / next scene, GO, DROP.
    // They fire on the rising edge and reset themselves.
    static const juce::StringArray actionIds;

    // REACT TO: which channels may drive the visuals (engine /v2/react, same order).
    static const juce::StringArray reactIds, reactNames;

    // Global look (engine /v2/look slots 0-6, same order).
    static const juce::StringArray lookIds, lookNames;

    // Reaction character (choice "reactStyle", engine /v2/style).
    static const juce::StringArray styleNames;

    // LOOK presets (choice "lookPreset"): a named look writes the look knobs
    // (one beat morph); editing any of them shows "Custom". "lookAmount" scales
    // the whole look on the way to the engine (the knobs keep their values).
    static const juce::StringArray lookPresetNames;
    static const juce::StringArray lookVectorIds;
    static constexpr int customLook = 0;
    static const std::array<float, 13>& lookVector (int preset);

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
    void clearSnapshot (int slot);
    float getMorphProgress() const;                      // 0-1 while a recall morphs, else -1
    bool getSnapshotRecipe (int slot, int& palette, int& look) const; // what a stored moment holds

    // Scene cue, theatre style: clicking a scene or Previous / Next Scene only
    // marks it NEXT; GO (parameter "Go") fires it, on the scene's beat grid.
    // Engine indices; -1 = none. "Fired" = sent, engine not switched yet.
    // Firing a scene that has a default look (LookLibrary) blends to that look,
    // exactly like GO on the look; scenes without one keep the current knobs.
    void cueScene (int engineIndex);
    void fireScene (int engineIndex, bool withSceneDefault = true);
    int getCuedScene() const noexcept { return cuedScene.load(); }

    // --- Looks (saved states in Documents\VJ VST\Looks, shared by every instance).
    // Cued / applied like scenes: cueLook marks it NEXT, GO (or applyLook) fires
    // the look's scene by name and morphs the knobs to it over the Snapshot
    // Morph time. Message thread only.
    LookLibrary& getLooks() noexcept { return *looks; }
    void cueLook (const juce::String& id);
    juce::String getCuedLook() const { return cuedLook; }
    bool applyLook (const juce::String& id);
    juce::String getActiveLook() const { return activeLook; }
    void setActiveLook (const juce::String& id) { activeLook = id; } // e.g. the look just saved from the screen
    juce::String getPendingScene() const { return pendingScene; }    // a look's scene waiting for the engine
    LookLibrary::Look captureLook (const juce::String& name) const;          // what is on screen now
    bool captureMomentLook (int slot, const juce::String& name, LookLibrary::Look&) const;
    juce::String currentSceneName() const;   // the live scene (or the last one known), "" if unknown
    juce::String sceneNameAt (int engineIndex) const;
    int findScene (const juce::String& name) const; // engine index, -1 if the engine has no such scene
    std::array<juce::Colour, 3> lookColours (const LookLibrary::Look&) const;
    static juce::String lookRecipe (const LookLibrary::Look&); // "Film · Blood · Pulse"

    // Lead: the one instance that sends knobs, look, scene and blackout
    // (LeadRegistry). MAKE LEAD pins this instance.
    bool isLead() const { return leads->isLead (this); }
    void makeLead();
    juce::String getLeadTrackName() const { return leads->leadTrackName(); }
    juce::String getTrackName() const { return trackName; }

    // True only when this instance sits on Live's Master track - independent
    // of role/lead status (isLead() is about which instance sends *control*
    // OSC when several run at once; it does not mean "this processBlock buffer
    // is the true master mix" - see REELS-RECORDING-PLAN.md). Recording gates
    // on this, not isLead(), since only the master bus has the full mix.
    bool isOnMasterTrack() const { return trackName.toLowerCase().contains ("master"); }

    // Reels recording: starts/stops the engine's video capture (worker) and
    // this instance's own WAV capture of the exact samples processBlock sees
    // (see processBlock) - no WASAPI loopback needed. Message thread only.
    void startRecording (double seconds, bool vertical);
    void stopRecording();
    bool isRecording() const noexcept { return recording.load(); }

    // Role follows the track name ("Kick 808" -> KICK) until picked by hand.
    bool isRoleAuto() const { return (bool) state.state.getProperty ("roleAuto", true); }
    void setRoleByHand (int role);
    void setRoleAuto();
    void updateTrackProperties (const TrackProperties&) override;
    int getFiredScene() const noexcept { return firedScene.load(); }

    // The performer's stills / clips (media slots 0-7), saved with the Live set.
    // Slot 0 keeps the original "mediaPath" key, so older sets still load.
    static juce::Identifier mediaKey (int slot) { return slot == 0 ? juce::Identifier ("mediaPath") : juce::Identifier ("mediaPath" + juce::String (slot)); }
    juce::String getMediaPath (int slot) const { return state.state.getProperty (mediaKey (slot)).toString(); }
    void setMediaPath (int slot, const juce::String& path) { state.state.setProperty (mediaKey (slot), path, nullptr); }
    static const juce::StringArray clipSyncNames;
    static int clipSyncBeats (int choice) noexcept { const int beats[] = { 0, 1, 4, 8, 16, 32 }; return beats[juce::jlimit (0, 5, choice)]; }
    int getMediaSlot() const { return juce::jlimit (0, 7, (int) state.getRawParameterValue ("mediaSlot")->load() - 1); }

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
    std::array<std::atomic<bool>, 9> actionPending {};
    juce::SharedResourcePointer<LeadRegistry> leads;
    std::atomic<juce::uint32> lastProcessMs { 0 };
    juce::String trackName;
    void applyRoleFromName();
    void updateLookPreset();
    int lastLookPreset = 2; // Film
    struct LookMorph
    {
        bool active = false;
        double start = 0.0, seconds = 0.0;
        std::array<float, 13> from {}, to {};
    };
    LookMorph lookMorph;
    void stepCue (int direction);
    void goCue();
    void updateCue();
    std::atomic<int> cuedScene { -1 }, firedScene { -1 };
    double firedAt = 0.0;
    void advanceMorph();
    static juce::StringArray snapshotParamIds();

    struct Morph
    {
        bool active = false;
        double start = 0.0, seconds = 0.0;
        bool switchedDiscrete = false;
        juce::Array<float> from, to;       // plain values, parallel to snapshotParamIds()
        int toPalette = 0, toLookPreset = -1, toStyle = -1;
        juce::StringArray toColours;
    };
    Morph morph;
    int activeSnapshot = -1;
    void startMorph (const juce::ValueTree& snapshot, bool cut); // recallSnapshot's blend, for moments and looks
    static juce::ValueTree lookToSnapshot (const LookLibrary::Look&);
    void fillLookRecipe (LookLibrary::Look&, int palette, int lookPreset, int style) const;
    void applyLookObject (const LookLibrary::Look&, bool cut);
    void applySceneDefault (int engineIndex);
    void watchPresetParameter();
    void resolvePendingScene();
    void applyStartupLook();

    juce::SharedResourcePointer<LookLibrary> looks;
    juce::String cuedLook, activeLook;
    juce::String pendingScene;                // a look's scene to fire once the engine lists it
    int lastPresetSeen = 0;                   // "preset" as last seen / set by us: other changes = host automation
    std::atomic<bool> stateLoaded { false };  // setStateInformation ran: resync lastPresetSeen
    std::atomic<bool> stateEverLoaded { false }; // a Live set (or a duplicate) was loaded: never the start-up look
    bool startupPending = true;
    int timerTicks = 0;
    std::atomic<double> hostBpm { 120.0 };

    juce::AudioProcessorValueTreeState state;
    AnalysisWorker worker;

    // Reels recording: the WAV is the exact samples processBlock sees, written
    // off the audio thread via JUCE's standard ThreadedWriter idiom (REELS-
    // RECORDING-PLAN.md #4). recordDeadline (message-thread clock seconds) is
    // this instance's own auto-stop - the engine times out on the same
    // requested duration independently, so either side stopping first is fine.
    std::atomic<bool> recording { false };
    // Declared before recordingWriter so it's destroyed after it (member
    // destruction is reverse-declaration-order) - the ThreadedWriter must be
    // torn down first, since its destructor removes it from this thread.
    juce::TimeSliceThread recordingWriteThread { "VJ Recording Writer" };
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> recordingWriter;
    juce::File recordingFile;
    double recordDeadline = 0.0;
    void checkRecordingTimeout(); // message thread, called from timerCallback()

    // Visual look-ahead ("lookahead" ms, default 0 = off): the audio leaving
    // the plug-in is delayed and reported as latency, so Live's delay
    // compensation delays everything else too - while the analysis reads the
    // undelayed input, i.e. the visuals see the music early.
    juce::AudioBuffer<float> delayBuffer;
    int delayWrite = 0;
    std::atomic<int> delaySamples { 0 };
    void updateLatency();

    // Transport discontinuity detection (audio thread only).
    double lastPpq = 0.0, lastSampleRate = 48000.0;
    bool lastPlaying = false;
    int epoch = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VJAnalyzerProcessor)
};
