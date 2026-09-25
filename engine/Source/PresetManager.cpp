#include "PresetManager.h"
#include "Diagnostics.h"

using namespace juce::gl;

namespace
{
    const char* const blendSource = R"(
        uniform sampler2D outgoingImage;
        uniform sampler2D incomingImage;
        uniform float progress;
        uniform float mode; // 0 crossfade, 1 dip to background, 2 luma wipe
        void main()
        {
            vec3 a = texture2D (outgoingImage, uv).rgb;
            vec3 b = texture2D (incomingImage, uv).rgb;
            vec3 c;
            if (mode < 0.5)
                c = mix (a, b, progress);
            else if (mode < 1.5)
                c = progress < 0.5 ? a * (1.0 - 2.0 * progress) : b * (2.0 * progress - 1.0);
            else
            {
                // Dark regions of the outgoing image give way first.
                float lum = dot (a, vec3 (0.2126, 0.7152, 0.0722));
                float edge = progress * 1.2 - 0.1;
                c = mix (a, b, smoothstep (lum - 0.1, lum + 0.1, edge));
            }
            gl_FragColor = vec4 (c, 1.0);
        }
    )";

    const char* const outputSource = R"(
        uniform sampler2D source;
        uniform float gain;
        void main() { gl_FragColor = vec4 (texture2D (source, uv).rgb * gain, 1.0); }
    )";
}

PresetManager::~PresetManager() = default;

void PresetManager::scanPresets (const juce::File& presetsDir, const juce::File& shadersDir, const juce::File& mediaDir)
{
    shadersDirectory = shadersDir;
    sourceLibrary.setMediaRoot (mediaDir);
    entries.clear();

    if (! presetsDir.isDirectory())
    {
        logDiagnostic ("PresetManager: presets directory not found: " + presetsDir.getFullPathName());
        return;
    }

    auto files = presetsDir.findChildFiles (juce::File::findFiles, false, "*.json");
    files.sort();

    for (auto& file : files)
    {
        auto json = juce::JSON::parse (file);
        Entry entry;
        entry.file = file;
        juce::String error;

        if (PresetV2::isSchema2 (json))
        {
            auto def = std::make_shared<PresetV2>();
            if (! PresetV2::parse (json, *def, error))
            {
                logDiagnostic ("PresetManager: rejected " + file.getFileName() + " - " + error);
                continue;
            }
            entry.name = def->name;
            entry.v2 = def;
        }
        else
        {
            bool ok = false;
            entry.legacy = Preset::loadFromFile (file, ok, error);
            if (! ok)
            {
                logDiagnostic ("PresetManager: failed to load " + file.getFileName() + " - " + error);
                continue;
            }
            entry.name = entry.legacy.name;
        }

        entries.add (entry);
    }

    logDiagnostic ("PresetManager: loaded " + juce::String (entries.size()) + " preset(s) from " + presetsDir.getFullPathName());
}

int PresetManager::getFirstSchema2Index() const noexcept
{
    for (int i = 0; i < entries.size(); ++i)
        if (entries.getReference (i).v2 != nullptr)
            return i;
    return 0;
}

juce::String PresetManager::getPresetName (int index) const
{
    return juce::isPositiveAndBelow (index, entries.size()) ? entries.getReference (index).name : juce::String();
}

juce::String PresetManager::getCurrentName() const
{
    auto index = currentIndex.load();
    return index >= 0 ? getPresetName (index) : juce::String ("(no preset)");
}

juce::StringArray PresetManager::getCurrentMacroLabels() const
{
    juce::StringArray labels;
    for (int i = 0; i < MacroBank::numSlots; ++i)
        labels.add ({});
    auto index = currentIndex.load();
    if (juce::isPositiveAndBelow (index, entries.size()))
        if (auto& v2 = entries.getReference (index).v2; v2 != nullptr)
            for (auto& m : v2->macros)
                if (juce::isPositiveAndBelow (m.slot, MacroBank::numSlots))
                    labels.set (m.slot, m.label);
    return labels;
}

juce::String PresetManager::getCurrentDescription() const
{
    auto index = currentIndex.load();
    if (juce::isPositiveAndBelow (index, entries.size()))
        if (auto& v2 = entries.getReference (index).v2; v2 != nullptr)
            return v2->description;
    return {};
}

bool PresetManager::usesMedia (int index) const
{
    if (! juce::isPositiveAndBelow (index, entries.size()) || entries.getReference (index).v2 == nullptr)
        return false;
    for (auto& stage : entries.getReference (index).v2->stages)
        for (auto& src : stage.sources)
            if (src.type != V2Source::Type::text)
                return true;
    return false;
}

bool PresetManager::isSchema2 (int index) const
{
    return juce::isPositiveAndBelow (index, entries.size()) && entries.getReference (index).v2 != nullptr;
}

void PresetManager::requestPreset (int index)
{
    if (entries.isEmpty())
        return;
    requestedIndex = ((index % entries.size()) + entries.size()) % entries.size();
}

bool PresetManager::ensurePrograms (juce::OpenGLContext& context)
{
    if (outputProgram != nullptr)
        return true;
    if (programsFailed)
        return false;

    juce::String error;
    blendProgram = buildFullscreenProgram (context, blendSource, error);
    if (blendProgram != nullptr)
        outputProgram = buildFullscreenProgram (context, outputSource, error);

    if (outputProgram == nullptr)
    {
        logDiagnostic ("PresetManager: compositing shaders failed - " + error);
        programsFailed = true;
        return false;
    }
    return true;
}

bool PresetManager::activate (int index, FrameContext& frame)
{
    auto& entry = entries.getReference (index);
    juce::String error;
    const bool cutNow = std::exchange (forceCut, false);

    std::unique_ptr<PresetInstance> instance;
    if (prebuiltIndex == index && prebuilt != nullptr)
        instance = std::move (prebuilt);
    else
        instance = entry.v2 != nullptr
                     ? V2Instance::create (*entry.v2, shadersDirectory, sourceLibrary, frame.gl, error)
                     : LegacyInstance::create (entry.legacy, shadersDirectory, frame.gl, error);
    if (prebuilt != nullptr)
        prebuilt->releaseGLObjects();
    prebuilt.reset();
    prebuiltIndex = -1;

    if (instance == nullptr)
    {
        // Keep the current picture - never swap to a half-built preset.
        logDiagnostic ("PresetManager: could not load '" + entry.name + "': " + error);
        return false;
    }

    auto type = entry.v2 != nullptr ? entry.v2->transition.type : V2Transition::Type::crossfade;
    if (cutNow)
        type = V2Transition::Type::cut;
    auto duration = durationOverrideMs >= 0.0 ? durationOverrideMs
                  : (entry.v2 != nullptr ? (double) entry.v2->transition.durationMs : 600.0);
    if (type == V2Transition::Type::cut)
        duration = 0.0;

    if (current != nullptr && duration > 0.0)
    {
        if (transition.active)
        {
            // Interrupted fade: freeze what is on screen right now and fade
            // from that, instead of keeping a third preset alive.
            frozenTarget.ensure (compositeTarget.width, compositeTarget.height, GL_RGBA16F);
            // (sizes match: the frozen frame is always at the current internal resolution)
            glBindFramebuffer (GL_READ_FRAMEBUFFER, compositeTarget.fbo);
            glBindFramebuffer (GL_DRAW_FRAMEBUFFER, frozenTarget.fbo);
            glBlitFramebuffer (0, 0, compositeTarget.width, compositeTarget.height,
                               0, 0, frozenTarget.width, frozenTarget.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer (GL_FRAMEBUFFER, 0);

            if (outgoing != nullptr) outgoing->releaseGLObjects();
            outgoing.reset();
            current->releaseGLObjects();
            current.reset();
            transition.fromFrozen = true;
        }
        else
        {
            outgoing = std::move (current);
            transition.fromFrozen = false;
        }

        transition.active = true;
        transition.type = type;
        transition.durationMs = duration;
        transition.startTime = frame.now;
    }
    else
    {
        if (current != nullptr) current->releaseGLObjects();
        if (outgoing != nullptr) outgoing->releaseGLObjects();
        outgoing.reset();
        transition.active = false;
    }

    current = std::move (instance);
    currentIndex = index;
    lookPass.clearTrails();
    ++switchCount;
    logDiagnostic ("PresetManager: switched to '" + entry.name + "' (" + juce::String (index + 1) + "/"
                   + juce::String (entries.size()) + ", " + juce::String ((int) duration) + " ms)");
    return true;
}

void PresetManager::prebuild (int index, FrameContext& frame)
{
    if (prebuilt != nullptr)
        prebuilt->releaseGLObjects();
    prebuilt.reset();
    prebuiltIndex = index; // set even on failure: activate() retries once and logs

    auto& entry = entries.getReference (index);
    juce::String error;
    prebuilt = entry.v2 != nullptr
                 ? V2Instance::create (*entry.v2, shadersDirectory, sourceLibrary, frame.gl, error)
                 : LegacyInstance::create (entry.legacy, shadersDirectory, frame.gl, error);
}

void PresetManager::render (FrameContext& frame, unsigned int finalTargetFbo)
{
    if (entries.isEmpty() || ! ensurePrograms (frame.gl))
        return;

    const auto& look = frame.look != nullptr ? *frame.look : defaultLook;
    lookPass.update (frame.signals, look, frame.dt, frame.now);
    updateAutoCut (frame, look);

    // Pick up a new request; schema-2 presets may wait for the next beat/bar.
    auto requested = requestedIndex.exchange (-1);
    bool arrivedNow = false;
    if (requested >= 0)
    {
        pendingIndex = (requested == currentIndex.load() && current != nullptr) ? -1 : requested; // re-selecting the live preset is a no-op
        arrivedNow = pendingIndex >= 0;
    }
    if (frame.clock.crossedBeat()) lastBeatTime = frame.now;
    if (frame.clock.crossedBar())  lastBarTime = frame.now;

    if (pendingIndex >= 0)
    {
        auto quantize = Quantize::none;
        if (auto& v2 = entries.getReference (pendingIndex).v2; v2 != nullptr && current != nullptr)
            quantize = v2->transition.quantize;

        const bool waitForGrid = quantize != Quantize::none && frame.clock.isFollowingTransport();
        const bool onGrid = (quantize == Quantize::beat && frame.clock.crossedBeat())
                         || (quantize == Quantize::bar && frame.clock.crossedBar());
        // A request landing just after the grid line (a button pressed "on"
        // the beat, 30 Hz automation) belongs to that line: switch now
        // instead of waiting a whole beat or bar.
        const bool justMissed = arrivedNow
                             && ((quantize == Quantize::beat && frame.now - lastBeatTime < 0.1)
                                 || (quantize == Quantize::bar && frame.now - lastBarTime < 0.1));

        if (forceCut || ! waitForGrid || onGrid || justMissed)
        {
            activate (pendingIndex, frame);
            pendingIndex = -1;
        }
        else if (prebuiltIndex != pendingIndex)
        {
            prebuild (pendingIndex, frame);
        }
    }

    if (current == nullptr)
        return;

    const auto outputWidth = frame.width, outputHeight = frame.height;
    const auto scale = renderScale.load();
    const auto w = juce::jmax (16, juce::roundToInt (outputWidth * scale));
    const auto h = juce::jmax (16, juce::roundToInt (outputHeight * scale));
    frame.width = w;   // presets render at the internal resolution
    frame.height = h;
    compositeTarget.ensure (w, h, GL_RGBA16F);
    currentTarget.ensure (w, h, GL_RGBA16F);

    float progress = 1.0f;
    if (transition.active)
    {
        progress = (float) juce::jlimit (0.0, 1.0, (frame.now - transition.startTime) * 1000.0 / transition.durationMs);
        if (progress >= 1.0f)
        {
            transition.active = false;
            if (outgoing != nullptr) outgoing->releaseGLObjects();
            outgoing.reset();
        }
    }

    auto bindTexture = [] (int unit, unsigned int tex) {
        glActiveTexture ((GLenum) (GL_TEXTURE0 + unit));
        glBindTexture (GL_TEXTURE_2D, tex);
    };

    current->render (frame, currentTarget);
    {
        const auto activity = current->getMacroActivity();
        const auto hold = (float) std::exp (-frame.dt / 0.3);
        for (size_t i = 0; i < activity.size(); ++i)
            macroActivity[i] = juce::jmax (activity[i], macroActivity[i].load() * hold);
    }

    unsigned int compositeSource = currentTarget.texture;

    if (transition.active)
    {
        unsigned int outgoingTexture = 0;
        if (transition.fromFrozen)
            outgoingTexture = frozenTarget.texture;
        else if (outgoing != nullptr)
        {
            outgoingTarget.ensure (w, h, GL_RGBA16F);
            outgoing->render (frame, outgoingTarget);
            outgoingTexture = outgoingTarget.texture;
        }

        compositeTarget.bind();
        blendProgram->use();
        bindTexture (0, outgoingTexture);
        bindTexture (1, currentTarget.texture);
        blendProgram->setUniform ("outgoingImage", 0);
        blendProgram->setUniform ("incomingImage", 1);
        blendProgram->setUniform ("progress", progress);
        blendProgram->setUniform ("mode", transition.type == V2Transition::Type::dipToBackground ? 1.0f
                                        : transition.type == V2Transition::Type::lumaWipe ? 2.0f : 0.0f);
        quad.draw (*blendProgram);
        compositeSource = compositeTarget.texture;
    }
    else
    {
        // Keep compositeTarget current so an interrupting switch can freeze it.
        glBindFramebuffer (GL_READ_FRAMEBUFFER, currentTarget.fbo);
        glBindFramebuffer (GL_DRAW_FRAMEBUFFER, compositeTarget.fbo);
        glBlitFramebuffer (0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    // Final output with the blackout gate (the last thing before the screen/Spout).
    auto target = blackout ? 0.0f : 1.0f;
    auto step = (float) (frame.dt / 0.12);
    outputGain = outputGain < target ? juce::jmin (target, outputGain + step) : juce::jmax (target, outputGain - step);

    frame.width = outputWidth;
    frame.height = outputHeight;
    auto sceneLook = look;
    sceneLook.duo = current->usesDuoPalette() ? 1.0f : 0.0f;
    if (lookPass.render (frame.gl, compositeSource, w, h, finalTargetFbo, outputWidth, outputHeight,
                         sceneLook, frame.time, outputGain))
        return;

    glBindFramebuffer (GL_FRAMEBUFFER, finalTargetFbo);
    glViewport (0, 0, outputWidth, outputHeight); // bilinear upscale when renderScale < 1
    outputProgram->use();
    bindTexture (0, compositeSource);
    outputProgram->setUniform ("source", 0);
    outputProgram->setUniform ("gain", outputGain);
    quad.draw (*outputProgram);

    bindTexture (1, 0);
    bindTexture (0, 0);
}

void PresetManager::updateAutoCut (const FrameContext& frame, const LookSettings& look)
{
    const auto rate = look.get (LookSettings::cutRate);
    if (rate < 0.04f || current == nullptr || pendingIndex >= 0 || requestedIndex.load() >= 0)
    {
        beatsSinceCut = 0;
        return;
    }

    bool cut = false;
    if (rate >= 0.9f)
    {
        // Top of the range: every kick is a cut (with a short guard).
        for (auto& e : frame.signals.events)
            if ((e.type == EventType::kick || e.type == EventType::userTrigger) && frame.now - lastKickCut > 0.34
                && (e.type == EventType::userTrigger || frame.signals.react.amount > 0.5f)) // max 3 cuts/s; none while calm
                cut = true;
        if (cut)
            lastKickCut = frame.now;
    }
    else if (frame.clock.crossedBeat())
    {
        const int beatsPerCut = rate < 0.2f ? 16 : rate < 0.4f ? 8 : rate < 0.6f ? 4 : rate < 0.75f ? 2 : 1;
        cut = ++beatsSinceCut >= beatsPerCut;
    }

    if (! cut)
        return;

    juce::Array<int> candidates;
    for (int i = 0; i < entries.size(); ++i)
        if (entries.getReference (i).v2 != nullptr && i != currentIndex.load())
            candidates.add (i);
    if (candidates.isEmpty())
        return;

    beatsSinceCut = 0;
    forceCut = true;
    pendingIndex = candidates[juce::Random::getSystemRandom().nextInt (candidates.size())];
}

void PresetManager::toggleEffectStage (int stageIndex)
{
    if (current != nullptr)
        current->toggleStage (stageIndex);
}

void PresetManager::setEffectParam (int stageIndex, const juce::String& name, const juce::var& value)
{
    if (current != nullptr)
        current->setParam (stageIndex, name, value);
}

void PresetManager::releaseGLObjects()
{
    if (current != nullptr)  current->releaseGLObjects();
    if (outgoing != nullptr) outgoing->releaseGLObjects();
    current.reset();
    outgoing.reset();
    currentIndex = -1;

    for (auto* t : { &currentTarget, &outgoingTarget, &frozenTarget, &compositeTarget })
        t->release();
    sourceLibrary.release();

    quad.release();
    lookPass.release();
    if (prebuilt != nullptr)
        prebuilt->releaseGLObjects();
    prebuilt.reset();
    prebuiltIndex = -1;
    blendProgram.reset();
    outputProgram.reset();
}
