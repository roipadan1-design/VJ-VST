#include "MainComponent.h"
#include "Diagnostics.h"

using namespace juce::gl;

MainComponent::MainComponent (int oscPortIn, bool startWithDemo)
    : oscPort (oscPortIn)
{
    featureBus.setDemoEnabled (startWithDemo);

    for (auto& open : reactValues)
        open = true;
    for (int i = 0; i < LookSettings::numSlots; ++i)
        lookValues[(size_t) i] = look.values[(size_t) i];
    for (int i = 0; i < MoveSettings::numSlots; ++i)
        moveValues[(size_t) i] = move.values[(size_t) i];
    for (int i = 0; i < 3; ++i)
    {
        paletteValues[(size_t) i * 3]     = look.palette[(size_t) i].getFloatRed();
        paletteValues[(size_t) i * 3 + 1] = look.palette[(size_t) i].getFloatGreen();
        paletteValues[(size_t) i * 3 + 2] = look.palette[(size_t) i].getFloatBlue();
    }
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

    statusSender.connect ("127.0.0.1", 9); // binds a local socket; real targets come from /v2/hello
    startTimer (200);
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
    presetManager.scanPresets (engineDir.getChildFile ("Presets"), engineDir.getChildFile ("Shaders"), engineDir.getChildFile ("Media"));

    // Start on the first schema-2 (instrument) preset if there is one.
    presetManager.requestPreset (presetManager.getFirstSchema2Index());

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

    auto requestedStageToggle = pendingStageToggle.exchange (-1);
    if (requestedStageToggle >= 0)
        presetManager.toggleEffectStage (requestedStageToggle);

    auto requestedTransitionDuration = pendingTransitionDurationMs.exchange (noTransitionChange);
    if (requestedTransitionDuration != noTransitionChange)
        presetManager.setTransitionDurationOverride (requestedTransitionDuration);

    {
        juce::Array<PendingEffectParam> paramsToApply;

        {
            const juce::ScopedLock lock (pendingEffectParamsLock);
            paramsToApply.swapWith (pendingEffectParams);
        }

        for (auto& param : paramsToApply)
            presetManager.setEffectParam (param.stageIndex, param.name, param.value);
    }

    for (int i = 0; i < MacroBank::numSlots; ++i)
    {
        macroBank.values[(size_t) i] = macroValues[(size_t) i].load();
        macroBank.set[(size_t) i] = macroSet[(size_t) i].load();
    }

    for (int i = 0; i < LookSettings::numSlots; ++i)
        look.values[(size_t) i] = lookValues[(size_t) i].load();
    for (int i = 0; i < 3; ++i)
        look.palette[(size_t) i] = juce::Colour::fromFloatRGBA (paletteValues[(size_t) i * 3].load(),
                                                               paletteValues[(size_t) i * 3 + 1].load(),
                                                               paletteValues[(size_t) i * 3 + 2].load(), 1.0f);
    look.paletteMix = paletteMixValue.load();
    for (int i = 0; i < MoveSettings::numSlots; ++i)
        move.values[(size_t) i] = moveValues[(size_t) i].load();

    auto desktopScale = (float) openGLContext.getRenderingScale();
    juce::OpenGLHelpers::clear (juce::Colours::black);

    // Physical pixel dimensions, not logical ones - glViewport calls need to
    // match the actual framebuffer size or HiDPI/display-scaled windows only
    // get partially filled.
    auto physicalWidth  = juce::roundToInt (desktopScale * (float) getWidth());
    auto physicalHeight = juce::roundToInt (desktopScale * (float) getHeight());

    auto now = nowSeconds();
    auto time = (float) (now - startTime);
    auto dt = lastFrameSeconds < 0.0 ? 1.0 / 60.0 : juce::jlimit (0.0, 0.25, now - lastFrameSeconds);
    lastFrameSeconds = now;

    ++fpsFrameCount;
    auto fpsWindowElapsed = now - fpsWindowStartSeconds;

    if (fpsWindowElapsed >= fpsLogIntervalSeconds)
    {
        auto fps = fpsFrameCount / fpsWindowElapsed;
        measuredFps = (float) fps;
        logDiagnostic ("FPS: " + juce::String (fps, 1) + " (preset '" + presetManager.getCurrentName() + "', "
                       + featureBus.describeSources (now) + ", clock " + juce::String (clock.bpm(), 1) + " BPM"
                       + (clock.isFollowingTransport() ? " host)" : " free-run)"));
        fpsFrameCount = 0;
        fpsWindowStartSeconds = now;
    }

    auto signals = featureBus.takeSnapshot (now);
    sectionTracker.update (signals, dt);
    ReactMask react;
    react.kick = reactValues[0];
    react.snare = reactValues[1];
    react.hat = reactValues[2];
    react.bass = reactValues[3];
    react.level = reactValues[4];
    {
        // Calm ramps every reaction out (and back in) over one bar; the
        // Reactivity knob itself only gets a short de-zipper.
        const bool calm = look.get (LookSettings::calm) > 0.5f;
        const auto target = calm ? 0.0f : look.get (LookSettings::reactivity);
        const auto barSeconds = clock.barBeats() * 60.0 / juce::jmax (20.0, clock.bpm());
        const auto tau = std::abs (target - reactAmount) > 0.2f ? barSeconds / 3.0 : 0.08;
        reactAmount += (target - reactAmount) * (float) (1.0 - std::exp (-dt / tau));
        react.amount = reactAmount;
    }
    applyReactMask (signals, react);
    clock.update (signals, now);
    clockBpm = (float) clock.bpm();
    clockFollowing = clock.isFollowingTransport();

    // Scene clock: Speed (macro 2), Glide (macro 7), Push, Sync, Reverse,
    // Freeze. Push leans on the music's energy (already gated by REACT TO
    // and scaled by Reactivity / CALM).
    {
        auto macro = [this] (int slot, float fallback) {
            return macroBank.set[(size_t) slot] ? macroBank.values[(size_t) slot] : fallback;
        };
        const auto drive = (0.6f * signals.bassRel + 0.4f * signals.levelRel) * react.amount;
        const auto barSeconds = clock.barBeats() * 60.0 / juce::jmax (20.0, clock.bpm());
        const auto beatDelta = clock.beat() - lastClockBeat;
        lastClockBeat = clock.beat();
        sceneClock.update (move, macro (1, 0.5f), macro (6, 0.25f), drive, clock.bpm(), barSeconds, beatDelta, dt);
        currentSpeed = (float) sceneClock.getFrame().speed;

        look.drift = move.get (MoveSettings::drift);
        look.driftTime = (float) sceneClock.getFrame().sceneTime;
    }

    videoPlayer.updateGLTexture();

    FrameContext frame { openGLContext, time, now, dt, signals, clock, macroBank, &look };
    frame.motion = sceneClock.getFrame();
    frame.videoTexture = videoPlayer.getTextureID();
    frame.width = physicalWidth;
    frame.height = physicalHeight;
    // Legacy uniforms, now fed from whichever analysis source is live.
    frame.level = signals.levelRel;
    frame.bass = signals.bassRel;
    frame.mid = signals.midRel;
    frame.high = signals.highRel;
    frame.beatphase = (float) clock.beatPhase();
    frame.onset = (float) juce::jlimit (0.0, 1.0, 1.0 - (now - signals.lastImpactTime) / onsetPulseDurationSeconds);

    updateAdaptiveQuality (now);
    presetManager.render (frame, 0);

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

    // Write to a temp file and rename, so a reader polling for the PNG never
    // sees a half-written image.
    auto outFile = getEngineDirectory().getChildFile ("VJEngine_snapshot.png");
    auto tempFile = outFile.getSiblingFile ("VJEngine_snapshot.tmp");
    juce::PNGImageFormat pngFormat;
    bool written = false;
    {
        juce::FileOutputStream stream (tempFile);
        if (stream.openedOk())
        {
            stream.setPosition (0);
            stream.truncate();
            written = pngFormat.writeImageToStream (image, stream);
        }
    }

    if (written && tempFile.moveFileTo (outFile))
        logDiagnostic ("Snapshot written to " + outFile.getFullPathName());
    else
        logDiagnostic ("Snapshot failed: could not write " + outFile.getFullPathName());
}

void MainComponent::updateAdaptiveQuality (double now)
{
    ++qualityFrames;
    if (now - qualityWindowStart < 1.0)
        return;

    auto fps = qualityFrames / (now - qualityWindowStart);
    qualityFrames = 0;
    qualityWindowStart = now;

    // Ignore the first seconds and the second after a preset switch: shader
    // compiles are one-off hitches, not a sustained load.
    auto switches = presetManager.getSwitchCount();
    if (switches != lastSwitchCount)
    {
        lastSwitchCount = switches;
        qualityHoldUntil = now + 1.5;
    }
    if (now < qualityHoldUntil || now - startTime < 3.0)
        return;

    auto setting = qualitySetting.load();
    if (setting > 0.0f)
    {
        presetManager.setRenderScale (setting);
        return;
    }

    // Drop fast when we miss the frame rate; climb back slowly once it has
    // been comfortably stable, so the scale doesn't oscillate every second.
    auto previous = autoScale;
    if (fps < 50.0 && autoScale > 0.5f)
    {
        autoScale = juce::jmax (0.5f, autoScale - (fps < 35.0 ? 0.15f : 0.08f));
        stableSeconds = 0;
    }
    else if (fps >= 58.5)
    {
        if (++stableSeconds >= 6 && autoScale < 1.0f)
        {
            autoScale = juce::jmin (1.0f, autoScale + 0.05f);
            stableSeconds = 0;
        }
    }
    else
    {
        stableSeconds = 0;
    }

    presetManager.setRenderScale (autoScale);
    if (std::abs (previous - autoScale) > 0.001f)
        logDiagnostic ("Quality: render scale " + juce::String (juce::roundToInt (autoScale * 100.0f)) + "% (" + juce::String (fps, 1) + " fps)");
}

void MainComponent::timerCallback()
{
    auto ports = featureBus.getReplyPorts (nowSeconds());
    if (ports.isEmpty())
        return;

    juce::OSCMessage status ("/v2/status");
    status.addInt32 (presetManager.getCurrentIndex());
    status.addString (presetManager.getCurrentName());
    status.addInt32 (presetManager.getNumPresets());
    status.addInt32 (presetManager.isBlackout() ? 1 : 0);
    status.addFloat32 (measuredFps.load());
    status.addFloat32 (clockBpm.load());
    status.addInt32 (clockFollowing.load() ? 1 : 0);
    status.addInt32 (featureBus.isDemoEnabled() ? 1 : 0);
    status.addFloat32 (presetManager.getRenderScale());
    status.addFloat32 (currentSpeed.load());

    // The live scene's knob names + description: on every scene change and
    // with the list every ~2 s.
    const bool sendLabels = presetManager.getCurrentIndex() != lastLabelsIndex || (statusTick % 10) == 0;
    juce::OSCMessage labels ("/v2/macros");
    if (sendLabels)
    {
        lastLabelsIndex = presetManager.getCurrentIndex();
        for (auto& l : presetManager.getCurrentMacroLabels())
            labels.addString (l);
        labels.addString (presetManager.getCurrentDescription());
    }

    // Preset names every ~2 s (cheap, and late-joining clients catch up).
    const bool sendList = (statusTick++ % 10) == 0;
    juce::OSCMessage list ("/v2/presets");
    if (sendList)
        for (int i = 0; i < juce::jmin (64, presetManager.getNumPresets()); ++i)
            list.addString (presetManager.getPresetName (i));

    for (auto port : ports)
    {
        statusSender.sendToIPAddress ("127.0.0.1", port, status);
        if (sendList)
            statusSender.sendToIPAddress ("127.0.0.1", port, list);
        if (sendLabels)
            statusSender.sendToIPAddress ("127.0.0.1", port, labels);
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
        presetManager.requestNext();
        return true;
    }

    if (key == juce::KeyPress::leftKey)
    {
        presetManager.requestPrevious();
        return true;
    }

    if (key == juce::KeyPress::spaceKey)
    {
        featureBus.pushUserTrigger (nowSeconds());
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

    if (textChar == 'b' || textChar == 'B')
    {
        presetManager.setBlackout (! presetManager.isBlackout());
        logDiagnostic (presetManager.isBlackout() ? "Blackout ON" : "Blackout OFF");
        return true;
    }

    if (textChar == 'q' || textChar == 'Q')
    {
        // Auto -> 100% -> 75% -> 50% -> Auto
        auto current = qualitySetting.load();
        auto nextSetting = current == 0.0f ? 1.0f : (current > 0.9f ? 0.75f : (current > 0.6f ? 0.5f : 0.0f));
        qualitySetting = nextSetting;
        logDiagnostic ("Quality: " + (nextSetting == 0.0f ? juce::String ("auto") : juce::String (juce::roundToInt (nextSetting * 100.0f)) + "%"));
        return true;
    }

    if (textChar == 'd' || textChar == 'D')
    {
        featureBus.setDemoEnabled (! featureBus.isDemoEnabled());
        logDiagnostic (featureBus.isDemoEnabled() ? "Demo groove ON (used while no analysis source is live)" : "Demo groove OFF");
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

    // Analysis (v2 + legacy /audio/*) goes to the FeatureBus.
    if (featureBus.handleMessage (message, nowSeconds()))
        return;

    auto numberArg = [&message] (int index, float fallback) {
        if (message.size() > index && message[index].isFloat32()) return message[index].getFloat32();
        if (message.size() > index && message[index].isInt32())   return (float) message[index].getInt32();
        return fallback;
    };

    if (address == "/preset/next" || address == "/v2/preset/next")
    {
        presetManager.requestNext();
        return;
    }

    if (address == "/preset/previous" || address == "/v2/preset/previous")
    {
        presetManager.requestPrevious();
        return;
    }

    if (address == "/preset/select" || address == "/v2/preset")
    {
        if (message.size() > 0 && message[0].isString())
        {
            // By name: first preset whose name contains the text (case-insensitive).
            for (int i = 0; i < presetManager.getNumPresets(); ++i)
            {
                if (presetManager.getPresetName (i).containsIgnoreCase (message[0].getString()))
                {
                    presetManager.requestPreset (i);
                    break;
                }
            }
        }
        else if (message.size() > 0)
        {
            presetManager.requestPreset ((int) numberArg (0, 0.0f));
        }
        return;
    }

    if (address == "/v2/macro")
    {
        auto slot = (int) numberArg (0, -1.0f);
        if (juce::isPositiveAndBelow (slot, MacroBank::numSlots) && message.size() > 1)
        {
            macroValues[(size_t) slot] = juce::jlimit (0.0f, 1.0f, numberArg (1, 0.0f));
            macroSet[(size_t) slot] = true;
        }
        return;
    }

    if (address == "/v2/look")
    {
        auto slot = (int) numberArg (0, -1.0f);
        if (juce::isPositiveAndBelow (slot, LookSettings::numSlots) && message.size() > 1)
            lookValues[(size_t) slot] = juce::jlimit (0.0f, 1.0f, numberArg (1, 0.0f));
        return;
    }

    if (address == "/v2/move")
    {
        auto slot = (int) numberArg (0, -1.0f);
        if (juce::isPositiveAndBelow (slot, MoveSettings::numSlots) && message.size() > 1)
            moveValues[(size_t) slot] = juce::jlimit (0.0f, 1.0f, numberArg (1, 0.0f));
        return;
    }

    if (address == "/v2/react")
    {
        for (int i = 0; i < juce::jmin (message.size(), (int) reactValues.size()); ++i)
            reactValues[(size_t) i] = numberArg (i, 1.0f) > 0.5f;
        return;
    }

    if (address == "/v2/palette")
    {
        if (message.size() >= 9)
            for (int i = 0; i < 9; ++i)
                paletteValues[(size_t) i] = juce::jlimit (0.0f, 1.0f, numberArg (i, 0.0f));
        if (message.size() >= 10)
            paletteMixValue = juce::jlimit (0.0f, 1.0f, numberArg (9, 1.0f));
        return;
    }

    if (address == "/v2/blackout")
    {
        presetManager.setBlackout (message.size() > 0 ? numberArg (0, 0.0f) != 0.0f : ! presetManager.isBlackout());
        return;
    }

    if (address == "/v2/trigger")
    {
        featureBus.pushUserTrigger (nowSeconds());
        return;
    }

    if (address == "/v2/quality")
    {
        qualitySetting = juce::jlimit (0.0f, 1.0f, numberArg (0, 0.0f));
        return;
    }

    if (address == "/v2/demo")
    {
        featureBus.setDemoEnabled (message.size() > 0 ? numberArg (0, 0.0f) != 0.0f : ! featureBus.isDemoEnabled());
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

    if (address == "/preset/transitionduration" || address == "/v2/transition")
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
}
