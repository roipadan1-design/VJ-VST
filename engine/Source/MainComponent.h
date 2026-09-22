#pragma once

#include <JuceHeader.h>
#include "PresetManager.h"
#include "SpoutSender.h"
#include "VideoPlayer.h"

// Phase 1: ISF shader hosting + preset switching. Listens for /audio/level,
// /audio/bass, /audio/mid, /audio/high, /audio/beatphase, /audio/onset,
// /preset/select, /preset/next, /preset/previous, /preset/transitionduration,
// /display/select, /display/next, /display/previous, /fullscreen, /effect/toggle,
// /effect/param, /camera/open, /video/load, and /debug/snapshot
// on UDP port 9000 by default (see "--osc-port" on the command line to run a
// second simultaneous instance on a different port). /effect/param <stageIndex> <paramName> <value> sets an ISF
// input live on the current preset (an effectChain stage, or the lone shader
// of a single-shader preset, which ignores stageIndex) - the same mechanism a
// future M4L knob UI would drive. /audio/onset (a bare bang, no argument - the M4L device sends
// it on a rising-edge bass transient) drives a decaying "onset" pulse
// (1.0 down to 0.0 over onsetPulseDurationSeconds) alongside level/bass/mid/
// high/beatphase, so any ISF shader or preset audioMappings entry can react
// to it exactly like the other audio-reactive uniforms. Press F for real OS fullscreen on whichever monitor the
// window is currently on, [ and ] to move the window (and fullscreen
// state, if active) to the previous/next monitor, Left/Right arrows to
// switch presets locally, 1-9 to toggle effect-chain stages on/off live,
// C to open the default webcam. Drag a video file (MP4/etc) onto the
// window to load it as the ISF "inputImage" source for effect-chain
// presets (e.g. the Glitch chain) - a live camera feed works the same way.
class MainComponent : public juce::OpenGLAppComponent,
                       public juce::FileDragAndDropTarget,
                       private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>
{
public:
    // oscPortIn lets more than one VJ Engine instance run at once, each on
    // its own port (see "--osc-port" on the command line) - the hardcoded
    // single port used to mean a second instance would silently bind the
    // same port alongside the first, with OSC then delivered to an
    // unpredictable one of the two.
    explicit MainComponent (int oscPortIn = 9000);
    ~MainComponent() override;

    void initialise() override;
    void shutdown() override;
    void render() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

    // Same effect as dropping the file onto the window - exposed so it can
    // also be triggered from a command-line argument (handy for scripted
    // testing without needing a GUI drag gesture) or, later, an OSC message.
    void loadVideoFile (const juce::File& file);
    static bool isSupportedVideoFile (const juce::File& file);

private:
    void oscMessageReceived (const juce::OSCMessage& message) override;
    juce::File getEngineDirectory() const;

    void moveToDisplay (int displayIndex);
    void setFullscreen (bool shouldBeFullscreen);
    void toggleFullscreen();
    void captureSnapshot (int pixelWidth, int pixelHeight);

    juce::OSCReceiver oscReceiver;
    const int oscPort;

    int currentDisplayIndex = 0;

    PresetManager presetManager;
    SpoutSender spoutSender;
    VideoPlayer videoPlayer;

    std::atomic<float> level { 0.0f };
    std::atomic<float> bass  { 0.0f };
    std::atomic<float> mid   { 0.0f };
    std::atomic<float> high  { 0.0f };
    std::atomic<float> beatPhase { 0.0f };

    // Time (same clock as startTime/render()'s `time`) at which the last
    // /audio/onset bang arrived - the render loop derives a decaying pulse
    // from this rather than storing the pulse value itself, since the OSC
    // message thread (writer) and GL thread (reader/decayer) would otherwise
    // race over who last touched it.
    std::atomic<double> lastOnsetTime { -1000.0 };
    static constexpr double onsetPulseDurationSeconds = 0.15;

    std::atomic<int> pendingPresetSelect { -1 };
    std::atomic<bool> pendingNext { false };
    std::atomic<bool> pendingPrevious { false };
    std::atomic<int> pendingStageToggle { -1 };

    // -1 sentinel = no change pending, same pattern as pendingPresetSelect.
    std::atomic<float> pendingTransitionDurationMs { -1.0f };

    // /debug/snapshot - dumps the current frame to VJEngine_snapshot.png next
    // to the .exe, overwriting any previous one. Lets a preset/effect be
    // checked from outside the process (screenshot tooling, a script, this
    // project's own headless testing) without needing eyes on the actual
    // window - there was no way to do this before except a live screenshot.
    std::atomic<bool> pendingSnapshot { false };

    // /effect/param arrives on the OSC/message thread but PresetManager's
    // NamedValueSet-backed param storage is only safe to touch from the GL
    // thread (render() reads it mid-frame) - queued here and drained at the
    // start of render(), same reasoning as the scalar pending* fields above
    // but needs a queue rather than a single atomic since a param name is
    // involved and more than one could arrive between frames.
    struct PendingEffectParam { int stageIndex; juce::String name; float value; };
    juce::CriticalSection pendingEffectParamsLock;
    juce::Array<PendingEffectParam> pendingEffectParams;

    double startTime = 0.0;

    // Periodic FPS logging (to VJEngine.log, same file/pattern VideoPlayer's
    // diagnostics use) - the only way to actually see frame rate without a
    // debugger attached, needed for any real GPU-load testing.
    int fpsFrameCount = 0;
    double fpsWindowStartSeconds = 0.0;
    static constexpr double fpsLogIntervalSeconds = 5.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
