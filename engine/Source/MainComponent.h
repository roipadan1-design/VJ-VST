#pragma once

#include <JuceHeader.h>

// Phase 0 spike: proves the pipe M4L -> OSC -> render engine works.
// Listens for /audio/level, /audio/bass, /audio/mid, /audio/high, /audio/beatphase
// on UDP port 9000 and drives a single reactive GLSL shader with them.
// Press F to toggle real OS-level fullscreen (the Phase 0 success criterion).
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
    void createShaders();

    juce::OSCReceiver oscReceiver;
    static constexpr int oscPort = 9000;

    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformTime;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformResolution;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformLevel;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformBass;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformMid;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformHigh;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformBeatPhase;

    unsigned int vertexBuffer = 0;
    int positionAttribute = -1;

    std::atomic<float> level { 0.0f };
    std::atomic<float> bass  { 0.0f };
    std::atomic<float> mid   { 0.0f };
    std::atomic<float> high  { 0.0f };
    std::atomic<float> beatPhase { 0.0f };

    double startTime = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
