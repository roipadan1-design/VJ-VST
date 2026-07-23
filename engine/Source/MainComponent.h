#pragma once

#include <JuceHeader.h>
#include "ISFShader.h"

// Phase 1: real ISF shader hosting. Listens for /audio/level, /audio/bass,
// /audio/mid, /audio/high, /audio/beatphase on UDP port 9000 and drives
// whichever ISF shader is currently loaded. Press F for real OS fullscreen.
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
    juce::File getShadersDirectory() const;

    juce::OSCReceiver oscReceiver;
    static constexpr int oscPort = 9000;

    ISFShader isfShader;

    std::atomic<float> level { 0.0f };
    std::atomic<float> bass  { 0.0f };
    std::atomic<float> mid   { 0.0f };
    std::atomic<float> high  { 0.0f };
    std::atomic<float> beatPhase { 0.0f };

    double startTime = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
