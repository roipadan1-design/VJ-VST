#pragma once

#include <JuceHeader.h>
#include "PresetManager.h"
#include "SpoutSender.h"
#include "VideoPlayer.h"
#include "FeatureBus.h"
#include "Modulation.h"

// The VJ Engine window: owns the GL context, the preset library/compositor
// (PresetManager), the analysis intake (FeatureBus) and the outputs (window,
// fullscreen on any monitor, Spout).
//
// OSC on UDP 9000 by default ("--osc-port N" for a second instance):
//  - analysis:  /v2/hello /v2/frame /v2/spectrum /v2/event (VJ Analyzer plugin,
//               analyze_wav --send) and the legacy /audio/* messages
//  - control:   /v2/macro <slot 0-7> <0-1>, /v2/preset <index>, /v2/preset/next|previous,
//               /v2/blackout <0|1>, /v2/trigger, /v2/transition <ms>, /v2/demo <0|1>,
//               /v2/look <slot 0-6> <0-1>, /v2/palette <9 floats rgb x3> [mix],
//               /v2/react <kick> <snare> <hat> <bass> <level> (0/1)
//  - legacy:    /preset/select|next|previous, /preset/transitionduration,
//               /effect/toggle, /effect/param, /display/*, /fullscreen,
//               /camera/open, /video/load, /debug/snapshot
// Keys: F/F11 fullscreen, [ ] move monitor, Left/Right presets, 1-9 toggle
// stages, Space manual hit, B blackout, D demo groove, C webcam; drop a video
// file onto the window to use it as the video input.
class MainComponent : public juce::OpenGLAppComponent,
                       public juce::FileDragAndDropTarget,
                       private juce::Timer,
                       private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>
{
public:
    // oscPortIn lets more than one VJ Engine instance run at once, each on
    // its own port (see "--osc-port" on the command line) - the hardcoded
    // single port used to mean a second instance would silently bind the
    // same port alongside the first, with OSC then delivered to an
    // unpredictable one of the two.
    explicit MainComponent (int oscPortIn = 9000, bool startWithDemo = false);
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

    // Status back-channel: every 200 ms tell each subscribed client (plugin
    // UIs) what is on screen - /v2/status and, when it changes, /v2/presets.
    void timerCallback() override;
    juce::OSCSender statusSender;
    std::atomic<float> measuredFps { 0.0f };
    std::atomic<float> clockBpm { 120.0f };
    std::atomic<bool> clockFollowing { false };
    int statusTick = 0;

    // Adaptive quality (Q key cycles Auto / 100 / 75 / 50 %, OSC /v2/quality <0=auto|scale>).
    void updateAdaptiveQuality (double now);
    std::atomic<float> qualitySetting { 0.0f }; // 0 = auto
    float autoScale = 1.0f;
    int qualityFrames = 0, stableSeconds = 0;
    double qualityWindowStart = 0.0, qualityHoldUntil = 0.0;
    int lastSwitchCount = 0;
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

    FeatureBus featureBus;
    Clock clock;
    MacroBank macroBank; // GL thread copy of the macro slots below
    std::array<std::atomic<float>, MacroBank::numSlots> macroValues {};
    std::array<std::atomic<bool>, MacroBank::numSlots> macroSet {};

    // Global look (/v2/look <slot> <0-1>, /v2/palette r g b r g b r g b [mix]).
    // Written on the message thread, copied into `look` at the top of render().
    LookSettings look;
    std::array<std::atomic<float>, LookSettings::numSlots> lookValues {};
    std::array<std::atomic<float>, 9> paletteValues {};
    std::atomic<float> paletteMixValue { 1.0f };

    // REACT TO gate (/v2/react <kick> <snare> <hat> <bass> <level>, 0/1 each).
    std::array<std::atomic<bool>, 5> reactValues {};
    double lastFrameSeconds = -1.0;
    static constexpr double onsetPulseDurationSeconds = 0.15;

    static double nowSeconds() { return juce::Time::getMillisecondCounterHiRes() * 0.001; }

    std::atomic<int> pendingStageToggle { -1 };

    // noTransitionChange = nothing pending; a negative value restores the
    // per-preset transition lengths.
    static constexpr float noTransitionChange = -1.0e9f;
    std::atomic<float> pendingTransitionDurationMs { noTransitionChange };

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
    bool startupPresetRequested = false;

    // Periodic FPS logging (to VJEngine.log, same file/pattern VideoPlayer's
    // diagnostics use) - the only way to actually see frame rate without a
    // debugger attached, needed for any real GPU-load testing.
    int fpsFrameCount = 0;
    double fpsWindowStartSeconds = 0.0;
    static constexpr double fpsLogIntervalSeconds = 5.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
