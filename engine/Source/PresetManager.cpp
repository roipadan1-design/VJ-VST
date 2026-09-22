#include "PresetManager.h"
#include "Diagnostics.h"

using namespace juce::gl;

PresetManager::~PresetManager() = default;

void PresetManager::scanPresets (const juce::File& presetsDir, const juce::File& shadersDir)
{
    shadersDirectory = shadersDir;
    presets.clear();

    if (! presetsDir.isDirectory())
    {
        logDiagnostic ("PresetManager: presets directory not found: " + presetsDir.getFullPathName());
        return;
    }

    auto jsonFiles = presetsDir.findChildFiles (juce::File::findFiles, false, "*.json");
    jsonFiles.sort();

    for (auto& file : jsonFiles)
    {
        bool ok = false;
        juce::String error;
        auto preset = Preset::loadFromFile (file, ok, error);

        if (ok)
            presets.add (preset);
        else
            logDiagnostic ("PresetManager: failed to load " + file.getFileName() + " - " + error);
    }

    logDiagnostic ("PresetManager: loaded " + juce::String (presets.size()) + " preset(s) from " + presetsDir.getFullPathName());
}

bool PresetManager::selectPreset (int index, juce::OpenGLContext& context)
{
    if (presets.isEmpty())
        return false;

    index = ((index % presets.size()) + presets.size()) % presets.size();
    auto& preset = presets.getReference (index);

    std::unique_ptr<ISFShader> newShader;
    std::unique_ptr<EffectChain> newEffectChain;

    if (preset.isEffectChain())
    {
        auto chain = std::make_unique<EffectChain>();

        if (! chain->load (preset.effectChain, shadersDirectory, context))
        {
            logDiagnostic ("PresetManager: could not load effect chain for preset '" + preset.name + "'");
            return false;
        }

        newEffectChain = std::move (chain);
    }
    else
    {
        auto shaderFile = shadersDirectory.getChildFile (preset.shaderFile);
        auto shader = std::make_unique<ISFShader>();

        if (! shader->loadFromFile (shaderFile))
        {
            logDiagnostic ("PresetManager: could not load shader for preset '" + preset.name + "': " + shader->getLastError());
            return false;
        }

        if (! shader->compile (context))
        {
            logDiagnostic ("PresetManager: could not compile shader for preset '" + preset.name + "': " + shader->getLastError());
            return false;
        }

        for (auto& override : preset.paramOverrides)
            shader->setValue (override.name.toString(), override.value);

        newShader = std::move (shader);
    }

    // Whatever was current becomes "outgoing" and keeps rendering (crossfading
    // out) instead of being torn down immediately - unless a fade was already
    // in flight, in which case that half-finished outgoing preset is what's
    // being replaced, so it's released now rather than accumulating a third.
    if (currentShader != nullptr || currentEffectChain != nullptr)
    {
        if (outgoingShader != nullptr)      outgoingShader->releaseGLObjects();
        if (outgoingEffectChain != nullptr) outgoingEffectChain->releaseGLObjects();

        outgoingShader = std::move (currentShader);
        outgoingEffectChain = std::move (currentEffectChain);
        outgoingIndex = currentIndex;
        transitioning = true;
        transitionStartMs = juce::Time::getMillisecondCounterHiRes();
    }

    currentShader = std::move (newShader);
    currentEffectChain = std::move (newEffectChain);
    currentIndex = index;

    logDiagnostic ("PresetManager: switched to preset '" + preset.name + "' (" + juce::String (index + 1) + "/" + juce::String (presets.size()) + ")");
    return true;
}

void PresetManager::nextPreset (juce::OpenGLContext& context)
{
    if (! presets.isEmpty())
        selectPreset (currentIndex + 1, context);
}

void PresetManager::previousPreset (juce::OpenGLContext& context)
{
    if (! presets.isEmpty())
        selectPreset (currentIndex - 1, context);
}

juce::String PresetManager::getCurrentName() const
{
    if (juce::isPositiveAndBelow (currentIndex, presets.size()))
        return presets.getReference (currentIndex).name;

    return "(no preset)";
}

int PresetManager::getNumEffectStages() const
{
    return currentEffectChain != nullptr ? currentEffectChain->getNumStages() : 0;
}

void PresetManager::toggleEffectStage (int stageIndex)
{
    if (currentEffectChain != nullptr)
        currentEffectChain->toggleStageBypassed (stageIndex);
}

void PresetManager::setEffectParam (int stageIndex, const juce::String& name, const juce::var& value)
{
    if (currentEffectChain != nullptr)
        currentEffectChain->setStageParam (stageIndex, name, value);
    else if (currentShader != nullptr)
        currentShader->setValue (name, value);
}

void PresetManager::renderActive (ISFShader* shader, EffectChain* effectChain, int presetIndexForMappings,
                                   juce::OpenGLContext& context, float timeSeconds, int pixelWidth, int pixelHeight,
                                   float level, float bass, float mid, float high, float beatphase, float onset,
                                   unsigned int videoTexture, unsigned int targetFbo)
{
    if (effectChain != nullptr)
    {
        effectChain->render (context, videoTexture, timeSeconds, pixelWidth, pixelHeight,
                              level, bass, mid, high, beatphase, onset, targetFbo);
        return;
    }

    if (shader != nullptr)
    {
        if (juce::isPositiveAndBelow (presetIndexForMappings, presets.size()))
        {
            auto& preset = presets.getReference (presetIndexForMappings);

            for (auto& mappingEntry : preset.audioMappings)
            {
                auto& paramName = mappingEntry.first;
                auto& mapping = mappingEntry.second;

                auto raw = resolveAudioMappingSource (mapping.source, level, bass, mid, high, beatphase, onset);
                shader->setValue (paramName, raw * mapping.scale + mapping.offset);
            }
        }

        shader->render (context, timeSeconds, pixelWidth, pixelHeight,
                        level, bass, mid, high, beatphase, onset, videoTexture, targetFbo);
    }
}

void PresetManager::ensureCrossfadeTarget (CrossfadeTarget& t, int width, int height)
{
    width = juce::jmax (1, width);
    height = juce::jmax (1, height);

    if (t.fbo != 0 && t.width == width && t.height == height)
        return;

    if (t.fbo == 0)     glGenFramebuffers (1, &t.fbo);
    if (t.texture == 0) glGenTextures (1, &t.texture);

    glBindTexture (GL_TEXTURE_2D, t.texture);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture (GL_TEXTURE_2D, 0);

    glBindFramebuffer (GL_FRAMEBUFFER, t.fbo);
    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t.texture, 0);
    glBindFramebuffer (GL_FRAMEBUFFER, 0);

    t.width = width;
    t.height = height;
}

void PresetManager::ensureBlendResources (juce::OpenGLContext& context)
{
    if (blendProgram != nullptr)
        return;

    auto newProgram = std::make_unique<juce::OpenGLShaderProgram> (context);

    auto vertexSrc = juce::OpenGLHelpers::translateVertexShaderToV3 (R"(
        attribute vec2 position;
        varying vec2 texCoord;
        void main() { gl_Position = vec4 (position, 0.0, 1.0); texCoord = position * 0.5 + 0.5; }
    )");

    // Straight alpha-mix crossfade - a classic VJ cut/fade, not a fancier
    // additive/luma-wipe transition. Alpha forced to 1.0 for the same reason
    // EffectChain's passthrough does: downstream Spout/Resolume consumers
    // would otherwise see a partially transparent frame.
    auto fragmentSrc = juce::OpenGLHelpers::translateFragmentShaderToV3 (R"(
        #ifdef GL_ES
        precision mediump float;
        #endif
        uniform sampler2D outgoingImage;
        uniform sampler2D incomingImage;
        uniform float alpha;
        varying vec2 texCoord;
        void main()
        {
            vec3 blended = mix (texture2D (outgoingImage, texCoord).rgb,
                                 texture2D (incomingImage, texCoord).rgb, alpha);
            gl_FragColor = vec4 (blended, 1.0);
        }
    )");

    if (! newProgram->addVertexShader (vertexSrc) || ! newProgram->addFragmentShader (fragmentSrc) || ! newProgram->link())
    {
        logDiagnostic ("PresetManager: blend shader failed to build: " + newProgram->getLastError());
        return;
    }

    blendProgram = std::move (newProgram);

    if (blendVertexBuffer == 0)
    {
        glGenBuffers (1, &blendVertexBuffer);
        glBindBuffer (GL_ARRAY_BUFFER, blendVertexBuffer);
        static const GLfloat quad[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
        glBufferData (GL_ARRAY_BUFFER, sizeof (quad), quad, GL_STATIC_DRAW);
        glBindBuffer (GL_ARRAY_BUFFER, 0);
    }
}

void PresetManager::drawBlend (juce::OpenGLContext& context, unsigned int outgoingTexture, unsigned int incomingTexture,
                                float alpha, int pixelWidth, int pixelHeight, unsigned int finalTargetFbo)
{
    ensureBlendResources (context);

    if (blendProgram == nullptr)
        return;

    glBindFramebuffer (GL_FRAMEBUFFER, finalTargetFbo);
    glViewport (0, 0, pixelWidth, pixelHeight);

    blendProgram->use();

    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, outgoingTexture);
    juce::OpenGLShaderProgram::Uniform (*blendProgram, "outgoingImage").set (0);

    glActiveTexture (GL_TEXTURE1);
    glBindTexture (GL_TEXTURE_2D, incomingTexture);
    juce::OpenGLShaderProgram::Uniform (*blendProgram, "incomingImage").set (1);

    juce::OpenGLShaderProgram::Uniform (*blendProgram, "alpha").set (alpha);

    auto positionAttribute = glGetAttribLocation (blendProgram->getProgramID(), "position");

    if (positionAttribute >= 0)
    {
        glBindBuffer (GL_ARRAY_BUFFER, blendVertexBuffer);
        glEnableVertexAttribArray ((GLuint) positionAttribute);
        glVertexAttribPointer ((GLuint) positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray ((GLuint) positionAttribute);
        glBindBuffer (GL_ARRAY_BUFFER, 0);
    }

    glActiveTexture (GL_TEXTURE1);
    glBindTexture (GL_TEXTURE_2D, 0);
    glActiveTexture (GL_TEXTURE0);
}

void PresetManager::render (juce::OpenGLContext& context, float timeSeconds, int pixelWidth, int pixelHeight,
                             float level, float bass, float mid, float high, float beatphase, float onset,
                             unsigned int videoTexture, unsigned int finalTargetFbo)
{
    if (! juce::isPositiveAndBelow (currentIndex, presets.size()))
        return;

    if (transitioning)
    {
        auto elapsedMs = juce::Time::getMillisecondCounterHiRes() - transitionStartMs;
        auto alpha = (float) juce::jlimit (0.0, 1.0, elapsedMs / transitionDurationMs);

        if (alpha >= 1.0f || (outgoingShader == nullptr && outgoingEffectChain == nullptr))
        {
            // Fade complete (or nothing was actually kept to fade from) - drop
            // the outgoing preset and fall through to the normal single-render path.
            if (outgoingShader != nullptr)      outgoingShader->releaseGLObjects();
            if (outgoingEffectChain != nullptr) outgoingEffectChain->releaseGLObjects();
            outgoingShader.reset();
            outgoingEffectChain.reset();
            outgoingIndex = -1;
            transitioning = false;
        }
        else
        {
            ensureCrossfadeTarget (outgoingTarget, pixelWidth, pixelHeight);
            ensureCrossfadeTarget (incomingTarget, pixelWidth, pixelHeight);

            renderActive (outgoingShader.get(), outgoingEffectChain.get(), outgoingIndex,
                          context, timeSeconds, pixelWidth, pixelHeight,
                          level, bass, mid, high, beatphase, onset, videoTexture, outgoingTarget.fbo);

            renderActive (currentShader.get(), currentEffectChain.get(), currentIndex,
                          context, timeSeconds, pixelWidth, pixelHeight,
                          level, bass, mid, high, beatphase, onset, videoTexture, incomingTarget.fbo);

            drawBlend (context, outgoingTarget.texture, incomingTarget.texture, alpha,
                       pixelWidth, pixelHeight, finalTargetFbo);
            return;
        }
    }

    renderActive (currentShader.get(), currentEffectChain.get(), currentIndex,
                  context, timeSeconds, pixelWidth, pixelHeight,
                  level, bass, mid, high, beatphase, onset, videoTexture, finalTargetFbo);
}

void PresetManager::releaseGLObjects()
{
    if (currentShader != nullptr)      currentShader->releaseGLObjects();
    if (currentEffectChain != nullptr) currentEffectChain->releaseGLObjects();
    if (outgoingShader != nullptr)      outgoingShader->releaseGLObjects();
    if (outgoingEffectChain != nullptr) outgoingEffectChain->releaseGLObjects();

    for (auto* target : { &outgoingTarget, &incomingTarget })
    {
        if (target->texture != 0) { glDeleteTextures (1, &target->texture); target->texture = 0; }
        if (target->fbo != 0)     { glDeleteFramebuffers (1, &target->fbo); target->fbo = 0; }
        target->width = target->height = 0;
    }

    if (blendVertexBuffer != 0)
    {
        glDeleteBuffers (1, &blendVertexBuffer);
        blendVertexBuffer = 0;
    }

    blendProgram.reset();
}
