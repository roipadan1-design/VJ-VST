#pragma once

#include <JuceHeader.h>
#include "PresetManager.h"
#include "SpoutSender.h"

// Phase 1: ISF shader hosting + preset switching. Listens for /audio/level,
// /audio/bass, /audio/mid, /audio/high, /audio/beatphase, and /preset/select,
// /preset/next, /preset/previous on UDP port 9000. Press F for real OS
// fullscreen, Left/Right arrows to switch presets locally.
class MainComponent : public juce::OpenGLAppComponent,
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

private:
    void oscMessageReceived (const juce::OSCMessage& message) override;
    juce::File getEngineDirectory() const;

    juce::OSCReceiver oscReceiver;
    static constexpr int oscPort = 9000;

    PresetManager presetManager;
    SpoutSender spoutSender;

    std::atomic<float> level { 0.0f };
    std::atomic<float> bass  { 0.0f };
    std::atomic<float> mid   { 0.0f };
    std::atomic<float> high  { 0.0f };
    std::atomic<float> beatPhase { 0.0f };

    std::atomic<int> pendingPresetSelect { -1 };
    std::atomic<bool> pendingNext { false };
    std::atomic<bool> pendingPrevious { false };

    double startTime = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
