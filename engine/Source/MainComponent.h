#pragma once

#include <JuceHeader.h>
#include "PresetManager.h"
#include "SpoutSender.h"
#include "VideoPlayer.h"

// Phase 1: ISF shader hosting + preset switching. Listens for /audio/level,
// /audio/bass, /audio/mid, /audio/high, /audio/beatphase, /preset/select,
// /preset/next, /preset/previous, /display/select, /display/next,
// /display/previous, /fullscreen, /effect/toggle, and /camera/open on UDP
// port 9000. Press F for real OS fullscreen on whichever monitor the
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
    MainComponent();
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

    juce::OSCReceiver oscReceiver;
    static constexpr int oscPort = 9000;

    int currentDisplayIndex = 0;

    PresetManager presetManager;
    SpoutSender spoutSender;
    VideoPlayer videoPlayer;

    std::atomic<float> level { 0.0f };
    std::atomic<float> bass  { 0.0f };
    std::atomic<float> mid   { 0.0f };
    std::atomic<float> high  { 0.0f };
    std::atomic<float> beatPhase { 0.0f };

    std::atomic<int> pendingPresetSelect { -1 };
    std::atomic<bool> pendingNext { false };
    std::atomic<bool> pendingPrevious { false };
    std::atomic<int> pendingStageToggle { -1 };

    double startTime = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
