#include "MainComponent.h"
#include "Diagnostics.h"

using namespace juce::gl;

MainComponent::MainComponent (int oscPortIn)
    : oscPort (oscPortIn)
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

    // Distinct Spout sender name when running on a non-default port, so a
    // second simultaneous instance doesn't collide with the first's sender.
    auto spoutName = oscPort == 9000 ? juce::String ("VJ Engine") : ("VJ Engine (" + juce::String (oscPort) + ")");

    if (! spoutSender.initialise (spoutName))
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
    fpsWindowStartSeconds = startTime;
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

    auto requestedTransitionDuration = pendingTransitionDurationMs.exchange (-1.0f);
    if (requestedTransitionDuration >= 0.0f)
        presetManager.setTransitionDuration (requestedTransitionDuration);

    {
        juce::Array<PendingEffectParam> paramsToApply;

        {
            const juce::ScopedLock lock (pendingEffectParamsLock);
            paramsToApply.swapWith (pendingEffectParams);
        }

        for (auto& param : paramsToApply)
            presetManager.setEffectParam (param.stageIndex, param.name, param.value);
    }

    auto desktopScale = (float) openGLContext.getRenderingScale();
    juce::OpenGLHelpers::clear (juce::Colours::black);

    // Physical pixel dimensions, not logical ones - runPass()'s glViewport calls need
    // to match the actual framebuffer size or HiDPI/display-scaled windows only get
    // partially filled (the rest stays black - looks like a cropped render).
    auto physicalWidth  = juce::roundToInt (desktopScale * (float) getWidth());
    auto physicalHeight = juce::roundToInt (desktopScale * (float) getHeight());

    auto nowSeconds = juce::Time::getMillisecondCounterHiRes() * 0.001;
    auto time = (float) (nowSeconds - startTime);

    ++fpsFrameCount;
    auto fpsWindowElapsed = nowSeconds - fpsWindowStartSeconds;

    if (fpsWindowElapsed >= fpsLogIntervalSeconds)
    {
        auto fps = fpsFrameCount / fpsWindowElapsed;
        logDiagnostic ("FPS: " + juce::String (fps, 1) + " (preset '" + presetManager.getCurrentName() + "')");
        fpsFrameCount = 0;
        fpsWindowStartSeconds = nowSeconds;
    }

    auto elapsedSinceOnset = nowSeconds - lastOnsetTime.load();
    auto onset = (float) juce::jlimit (0.0, 1.0, 1.0 - elapsedSinceOnset / onsetPulseDurationSeconds);

    videoPlayer.updateGLTexture();

    presetManager.render (openGLContext, time, physicalWidth, physicalHeight,
                          level.load(), bass.load(), mid.load(), high.load(), beatPhase.load(), onset,
                          videoPlayer.getTextureID());

    // presetManager.render() leaves the default framebuffer (0) holding this
    // frame's final image - share it as-is, no extra copy/blit needed.
    spoutSender.sendFrame (0, physicalWidth, physicalHeight);

    if (pendingSnapshot.exchange (false))
        captureSnapshot (physicalWidth, physicalHeight);
}

void MainComponent::captureSnapshot (int pixelWidth, int pixelHeight)
{
    // Reads back whatever presetManager.render() just left in the default
    // framebuffer - same image spoutSender just sent, just written to disk
    // instead of shared over Spout. glReadPixels gives rows bottom-to-top;
    // juce::Image is top-to-bottom, so each row is copied in reverse order.
    juce::Image image (juce::Image::ARGB, pixelWidth, pixelHeight, false);
    juce::HeapBlock<unsigned char> pixels ((size_t) pixelWidth * (size_t) pixelHeight * 4);
    glReadPixels (0, 0, pixelWidth, pixelHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels.getData());

    juce::Image::BitmapData bitmap (image, juce::Image::BitmapData::writeOnly);
    for (int y = 0; y < pixelHeight; ++y)
    {
        auto* srcRow = pixels.getData() + (size_t) (pixelHeight - 1 - y) * (size_t) pixelWidth * 4;
        auto* dstRow = bitmap.getLinePointer (y);

        for (int x = 0; x < pixelWidth; ++x)
        {
            auto* src = srcRow + x * 4;
            auto* dst = dstRow + x * 4;
            // src is RGBA; JUCE's native ARGB pixel order on little-endian is BGRA in memory.
            dst[0] = src[2];
            dst[1] = src[1];
            dst[2] = src[0];
            dst[3] = src[3];
        }
    }

    auto outFile = getEngineDirectory().getChildFile ("VJEngine_snapshot.png");
    juce::PNGImageFormat pngFormat;
    juce::FileOutputStream stream (outFile);

    if (stream.openedOk())
    {
        stream.setPosition (0);
        stream.truncate();
        pngFormat.writeImageToStream (image, stream);
        logDiagnostic ("Snapshot written to " + outFile.getFullPathName());
    }
    else
    {
        logDiagnostic ("Snapshot failed: could not open " + outFile.getFullPathName() + " for writing");
    }
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

    if (textChar == 'c' || textChar == 'C')
    {
        videoPlayer.openCamera (0); // safe from the message thread, see VideoPlayer::openCamera
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

    if (address == "/effect/param")
    {
        if (message.size() >= 3 && message[1].isString())
        {
            auto& stageArg = message[0];
            auto& valueArg = message[2];

            if ((stageArg.isFloat32() || stageArg.isInt32())
                && (valueArg.isFloat32() || valueArg.isInt32()))
            {
                auto stageIndex = stageArg.isFloat32() ? (int) stageArg.getFloat32() : stageArg.getInt32();
                auto value = valueArg.isFloat32() ? valueArg.getFloat32() : (float) valueArg.getInt32();

                logDiagnostic ("effect/param received: stage=" + juce::String (stageIndex)
                               + " param=" + message[1].getString() + " value=" + juce::String (value));

                const juce::ScopedLock lock (pendingEffectParamsLock);
                pendingEffectParams.add ({ stageIndex, message[1].getString(), value });
            }
        }
        return;
    }

    if (address == "/debug/snapshot")
    {
        pendingSnapshot = true;
        return;
    }

    if (address == "/video/load")
    {
        if (message.size() > 0 && message[0].isString())
        {
            juce::File file (message[0].getString());
            if (isSupportedVideoFile (file) && file.existsAsFile())
                loadVideoFile (file);
            else
                logDiagnostic ("video/load: not a valid video file - " + message[0].getString());
        }
        return;
    }

    if (address == "/preset/transitionduration")
    {
        float ms = 0.0f;
        if (message.size() > 0 && message[0].isFloat32())
            ms = message[0].getFloat32();
        else if (message.size() > 0 && message[0].isInt32())
            ms = (float) message[0].getInt32();
        else
            return;

        logDiagnostic ("preset/transitionduration received: " + juce::String (ms) + "ms");
        pendingTransitionDurationMs = ms;
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

    if (address == "/audio/onset")
    {
        lastOnsetTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
        return;
    }

    if (address == "/camera/open")
    {
        int deviceIndex = 0;
        if (message.size() > 0 && message[0].isFloat32())
            deviceIndex = (int) message[0].getFloat32();
        else if (message.size() > 0 && message[0].isInt32())
            deviceIndex = message[0].getInt32();

        videoPlayer.openCamera (deviceIndex);
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
