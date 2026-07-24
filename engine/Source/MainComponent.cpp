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

    // Figure out which physical monitor we actually started on, so [ and ]
    // cycle relative to reality instead of assuming index 0 is "here".
    if (auto* topLevel = getTopLevelComponent())
    {
        auto centre = topLevel->getScreenBounds().getCentre().toFloat();
        auto& displays = juce::Desktop::getInstance().getDisplays().displays;

        for (int i = 0; i < displays.size(); ++i)
        {
            if (displays.getReference (i).logicalBounds.contains (centre))
            {
                currentDisplayIndex = i;
                break;
            }
        }
    }

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

    auto requestedStageToggle = pendingStageToggle.exchange (-1);
    if (requestedStageToggle >= 0)
        presetManager.toggleEffectStage (requestedStageToggle);

    auto desktopScale = (float) openGLContext.getRenderingScale();
    juce::OpenGLHelpers::clear (juce::Colours::black);

    // Physical pixel dimensions, not logical ones - runPass()'s glViewport calls need
    // to match the actual framebuffer size or HiDPI/display-scaled windows only get
    // partially filled (the rest stays black - looks like a cropped render).
    auto physicalWidth  = juce::roundToInt (desktopScale * (float) getWidth());
    auto physicalHeight = juce::roundToInt (desktopScale * (float) getHeight());

    auto time = (float) (juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);

    videoPlayer.updateGLTexture();

    presetManager.render (openGLContext, time, physicalWidth, physicalHeight,
                          level.load(), bass.load(), mid.load(), high.load(), beatPhase.load(),
                          videoPlayer.getTextureID());

    // presetManager.render() leaves the default framebuffer (0) holding this
    // frame's final image - share it as-is, no extra copy/blit needed.
    spoutSender.sendFrame (0, physicalWidth, physicalHeight);
}

void MainComponent::paint (juce::Graphics&) {}

void MainComponent::resized() {}

bool MainComponent::isSupportedVideoFile (const juce::File& file)
{
    static const juce::StringArray videoExtensions { ".mp4", ".mov", ".m4v", ".avi", ".wmv", ".mkv" };
    return videoExtensions.contains (file.getFileExtension().toLowerCase());
}

bool MainComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& path : files)
        if (isSupportedVideoFile (juce::File (path)))
            return true;

    return false;
}

void MainComponent::filesDropped (const juce::StringArray& files, int, int)
{
    for (auto& path : files)
    {
        juce::File file (path);

        if (isSupportedVideoFile (file))
        {
            loadVideoFile (file);
            break;
        }
    }
}

void MainComponent::loadVideoFile (const juce::File& file)
{
    DBG ("VJEngine: loading video " << file.getFullPathName());
    videoPlayer.load (file); // kicks off the decode thread; safe from the message thread
}

// All three of these run on the message thread (called from keyPressed or
// oscMessageReceived, which JUCE's OSCReceiver::Listener<MessageLoopCallback>
// already delivers there) - unlike preset switching, moving/resizing a
// DocumentWindow is a UI operation and must not be deferred to the GL thread.
void MainComponent::moveToDisplay (int displayIndex)
{
    auto* topLevel = getTopLevelComponent();
    if (topLevel == nullptr)
        return;

    auto& displays = juce::Desktop::getInstance().getDisplays().displays;
    if (displays.isEmpty())
        return;

    displayIndex = ((displayIndex % displays.size()) + displays.size()) % displays.size();
    currentDisplayIndex = displayIndex;

    auto& desktop = juce::Desktop::getInstance();
    bool wasFullscreen = (desktop.getKioskModeComponent() == topLevel);

    // Kiosk mode has to be released before moving the window, or Windows
    // leaves it maximised on the old monitor instead of actually relocating.
    if (wasFullscreen)
        desktop.setKioskModeComponent (nullptr);

    topLevel->setBounds (displays.getReference (displayIndex).userBounds.toNearestInt());

    if (wasFullscreen)
        desktop.setKioskModeComponent (topLevel);

    DBG ("VJEngine: moved to display " << (displayIndex + 1) << "/" << displays.size());
}

void MainComponent::setFullscreen (bool shouldBeFullscreen)
{
    auto& desktop = juce::Desktop::getInstance();
    auto* topLevel = getTopLevelComponent();
    if (topLevel == nullptr)
        return;

    desktop.setKioskModeComponent (shouldBeFullscreen ? topLevel : nullptr);
}

void MainComponent::toggleFullscreen()
{
    auto& desktop = juce::Desktop::getInstance();
    setFullscreen (desktop.getKioskModeComponent() != getTopLevelComponent());
}

bool MainComponent::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::F11Key || key.getTextCharacter() == 'f' || key.getTextCharacter() == 'F')
    {
        toggleFullscreen();
        return true;
    }

    if (key.getTextCharacter() == ']')
    {
        moveToDisplay (currentDisplayIndex + 1);
        return true;
    }

    if (key.getTextCharacter() == '[')
    {
        moveToDisplay (currentDisplayIndex - 1);
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

    // Number keys 1-9 toggle that effect-chain stage on/off live (no-op if
    // the current preset isn't an effectChain preset, or has fewer stages).
    auto textChar = key.getTextCharacter();
    if (textChar >= '1' && textChar <= '9')
    {
        pendingStageToggle = (textChar - '1');
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

    if (address == "/effect/toggle")
    {
        if (message.size() > 0 && message[0].isFloat32())
            pendingStageToggle = (int) message[0].getFloat32();
        else if (message.size() > 0 && message[0].isInt32())
            pendingStageToggle = message[0].getInt32();
        return;
    }

    if (address == "/display/next")
    {
        moveToDisplay (currentDisplayIndex + 1);
        return;
    }

    if (address == "/display/previous")
    {
        moveToDisplay (currentDisplayIndex - 1);
        return;
    }

    if (address == "/display/select")
    {
        if (message.size() > 0 && message[0].isFloat32())
            moveToDisplay ((int) message[0].getFloat32());
        else if (message.size() > 0 && message[0].isInt32())
            moveToDisplay (message[0].getInt32());
        return;
    }

    if (address == "/fullscreen")
    {
        if (message.size() > 0 && message[0].isFloat32())
            setFullscreen (message[0].getFloat32() != 0.0f);
        else if (message.size() > 0 && message[0].isInt32())
            setFullscreen (message[0].getInt32() != 0);
        else
            toggleFullscreen();
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
