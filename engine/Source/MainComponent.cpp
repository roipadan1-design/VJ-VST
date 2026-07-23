#include "MainComponent.h"

using namespace juce::gl;

MainComponent::MainComponent()
{
    setSize (1280, 720);
    // Deliberately NOT forcing a Core profile: defaultGLVersion gives the GPU's newest
    // compatibility-profile context, which is what ISF/Shadertoy-style shaders expect
    // (gl_FragColor, varying, #version 120) without VVISF-GL having to rewrite them.
    // Core 4.1 would actually be a downgrade here, not an upgrade - Windows doesn't
    // gate performance behind Core profile.
    setWantsKeyboardFocus (true);

    if (! oscReceiver.connect (oscPort))
        DBG ("VJEngine: failed to bind OSC port " << oscPort << " - is another instance already running?");

    oscReceiver.addListener (this);
}

MainComponent::~MainComponent()
{
    shutdownOpenGL();
}

juce::File MainComponent::getShadersDirectory() const
{
    return juce::File::getSpecialLocation (juce::File::currentExecutableFile)
             .getParentDirectory()
             .getChildFile ("Shaders");
}

void MainComponent::initialise()
{
    auto shaderFile = getShadersDirectory().getChildFile ("Poly Star.fs");

    if (! isfShader.loadFromFile (shaderFile))
    {
        DBG ("ISF load failed: " << isfShader.getLastError());
        jassertfalse;
        return;
    }

    if (! isfShader.compile (openGLContext))
    {
        DBG ("ISF compile failed: " << isfShader.getLastError());
        jassertfalse;
        return;
    }

    startTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
}

void MainComponent::shutdown()
{
    isfShader.releaseGLObjects();
}

void MainComponent::render()
{
    jassert (juce::OpenGLHelpers::isContextActive());

    auto desktopScale = (float) openGLContext.getRenderingScale();
    juce::OpenGLHelpers::clear (juce::Colours::black);

    glViewport (0, 0,
                juce::roundToInt (desktopScale * (float) getWidth()),
                juce::roundToInt (desktopScale * (float) getHeight()));

    auto time = (float) (juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);

    isfShader.render (openGLContext, time, getWidth(), getHeight(),
                       level.load(), bass.load(), mid.load(), high.load(), beatPhase.load());
}

void MainComponent::paint (juce::Graphics&) {}

void MainComponent::resized() {}

bool MainComponent::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::F11Key || key.getTextCharacter() == 'f' || key.getTextCharacter() == 'F')
    {
        auto& desktop = juce::Desktop::getInstance();
        auto* topLevel = getTopLevelComponent();
        bool isCurrentlyFullscreen = (desktop.getKioskModeComponent() == topLevel);
        desktop.setKioskModeComponent (isCurrentlyFullscreen ? nullptr : topLevel);
        return true;
    }

    return false;
}

void MainComponent::oscMessageReceived (const juce::OSCMessage& message)
{
    if (message.size() == 0 || ! message[0].isFloat32())
        return;

    const auto address = message.getAddressPattern().toString();
    const auto value = message[0].getFloat32();

    if (address == "/audio/level")         level = value;
    else if (address == "/audio/bass")     bass = value;
    else if (address == "/audio/mid")      mid = value;
    else if (address == "/audio/high")     high = value;
    else if (address == "/audio/beatphase") beatPhase = value;
}
