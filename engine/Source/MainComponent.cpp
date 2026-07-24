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

juce::File MainComponent::getEngineDirectory() const
{
    return juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
}

void MainComponent::initialise()
{
    auto engineDir = getEngineDirectory();
    presetManager.scanPresets (engineDir.getChildFile ("Presets"), engineDir.getChildFile ("Shaders"));
    presetManager.selectPreset (0, openGLContext);

    if (! spoutSender.initialise ("VJ Engine"))
        DBG ("VJEngine: Spout sender unavailable - continuing without Spout output");

    startTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
}

void MainComponent::shutdown()
{
    spoutSender.shutdown();
    presetManager.releaseGLObjects();
}

void MainComponent::render()
{
    jassert (juce::OpenGLHelpers::isContextActive());

    // Preset switches are requested from the OSC/message thread but must happen
    // here, on the GL thread, since selecting a preset compiles a new shader.
    auto requestedIndex = pendingPresetSelect.exchange (-1);
    if (requestedIndex >= 0)
        presetManager.selectPreset (requestedIndex, openGLContext);

    if (pendingNext.exchange (false))
        presetManager.nextPreset (openGLContext);

    if (pendingPrevious.exchange (false))
        presetManager.previousPreset (openGLContext);

    auto desktopScale = (float) openGLContext.getRenderingScale();
    juce::OpenGLHelpers::clear (juce::Colours::black);

    // Physical pixel dimensions, not logical ones - runPass()'s glViewport calls need
    // to match the actual framebuffer size or HiDPI/display-scaled windows only get
    // partially filled (the rest stays black - looks like a cropped render).
    auto physicalWidth  = juce::roundToInt (desktopScale * (float) getWidth());
    auto physicalHeight = juce::roundToInt (desktopScale * (float) getHeight());

    auto time = (float) (juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);

    presetManager.render (openGLContext, time, physicalWidth, physicalHeight,
                          level.load(), bass.load(), mid.load(), high.load(), beatPhase.load());

    // presetManager.render() leaves the default framebuffer (0) holding this
    // frame's final image - share it as-is, no extra copy/blit needed.
    spoutSender.sendFrame (0, physicalWidth, physicalHeight);
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

    if (key == juce::KeyPress::rightKey)
    {
        pendingNext = true;
        return true;
    }

    if (key == juce::KeyPress::leftKey)
    {
        pendingPrevious = true;
        return true;
    }

    return false;
}

void MainComponent::oscMessageReceived (const juce::OSCMessage& message)
{
    const auto address = message.getAddressPattern().toString();

    if (address == "/preset/next")
    {
        pendingNext = true;
        return;
    }

    if (address == "/preset/previous")
    {
        pendingPrevious = true;
        return;
    }

    if (address == "/preset/select")
    {
        if (message.size() > 0 && message[0].isFloat32())
            pendingPresetSelect = (int) message[0].getFloat32();
        else if (message.size() > 0 && message[0].isInt32())
            pendingPresetSelect = message[0].getInt32();
        return;
    }

    if (message.size() == 0 || ! message[0].isFloat32())
        return;

    const auto value = message[0].getFloat32();

    if (address == "/audio/level")          level = value;
    else if (address == "/audio/bass")      bass = value;
    else if (address == "/audio/mid")       mid = value;
    else if (address == "/audio/high")      high = value;
    else if (address == "/audio/beatphase") beatPhase = value;
}
