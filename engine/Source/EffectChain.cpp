#include "EffectChain.h"

using namespace juce::gl;

EffectChain::~EffectChain()
{
    releaseGLObjects();
}

bool EffectChain::load (const juce::Array<EffectStageConfig>& stageConfigs,
                         const juce::File& shadersDir, juce::OpenGLContext& context)
{
    releaseGLObjects();
    stages.clear();
    configs.clear();
    stageBypassed.clear();

    for (auto& config : stageConfigs)
    {
        auto shaderFile = shadersDir.getChildFile (config.shaderFile);
        auto shader = std::make_unique<ISFShader>();

        if (! shader->loadFromFile (shaderFile))
        {
            DBG ("EffectChain: could not load stage shader '" << config.shaderFile << "': " << shader->getLastError());
            stages.clear();
            configs.clear();
            stageBypassed.clear();
            return false;
        }

        if (! shader->compile (context))
        {
            DBG ("EffectChain: could not compile stage shader '" << config.shaderFile << "': " << shader->getLastError());
            stages.clear();
            configs.clear();
            stageBypassed.clear();
            return false;
        }

        for (auto& override : config.paramOverrides)
            shader->setValue (override.name.toString(), override.value);

        stages.add (shader.release());
        configs.add (config);
        stageBypassed.add (false);
    }

    return true;
}

juce::String EffectChain::getStageName (int index) const
{
    if (juce::isPositiveAndBelow (index, configs.size()))
        return configs.getReference (index).shaderFile;

    return {};
}

void EffectChain::setStageBypassed (int index, bool bypassed)
{
    if (juce::isPositiveAndBelow (index, stageBypassed.size()))
        stageBypassed.set (index, bypassed);
}

bool EffectChain::isStageBypassed (int index) const
{
    return juce::isPositiveAndBelow (index, stageBypassed.size()) && stageBypassed[index];
}

void EffectChain::toggleStageBypassed (int index)
{
    if (juce::isPositiveAndBelow (index, stageBypassed.size()))
        stageBypassed.set (index, ! stageBypassed[index]);
}

void EffectChain::ensureTarget (PingPongTarget& t, int width, int height)
{
    width = juce::jmax (1, width);
    height = juce::jmax (1, height);

    if (t.fbo != 0 && t.width == width && t.height == height)
        return;

    if (t.fbo == 0)
        glGenFramebuffers (1, &t.fbo);

    if (t.texture == 0)
        glGenTextures (1, &t.texture);

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

void EffectChain::ensurePassthroughResources (juce::OpenGLContext& context)
{
    if (passthroughProgram != nullptr)
        return;

    auto newProgram = std::make_unique<juce::OpenGLShaderProgram> (context);

    auto vertexSrc = juce::OpenGLHelpers::translateVertexShaderToV3 (R"(
        attribute vec2 position;
        varying vec2 texCoord;
        void main() { gl_Position = vec4 (position, 0.0, 1.0); texCoord = position * 0.5 + 0.5; }
    )");

    // Alpha forced to 1.0 regardless of the source texture's alpha channel -
    // Media Foundation's RGB32 leaves that byte undefined/zero, and a zero
    // alpha here would make Spout/Resolume treat the whole frame as
    // transparent even though the RGB is correct.
    auto fragmentSrc = juce::OpenGLHelpers::translateFragmentShaderToV3 (R"(
        #ifdef GL_ES
        precision mediump float;
        #endif
        uniform sampler2D sourceImage;
        varying vec2 texCoord;
        void main() { gl_FragColor = vec4 (texture2D (sourceImage, texCoord).rgb, 1.0); }
    )");

    if (! newProgram->addVertexShader (vertexSrc) || ! newProgram->addFragmentShader (fragmentSrc) || ! newProgram->link())
    {
        DBG ("EffectChain: passthrough shader failed to build: " << newProgram->getLastError());
        return;
    }

    passthroughProgram = std::move (newProgram);

    if (passthroughVertexBuffer == 0)
    {
        glGenBuffers (1, &passthroughVertexBuffer);
        glBindBuffer (GL_ARRAY_BUFFER, passthroughVertexBuffer);
        static const GLfloat quad[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
        glBufferData (GL_ARRAY_BUFFER, sizeof (quad), quad, GL_STATIC_DRAW);
        glBindBuffer (GL_ARRAY_BUFFER, 0);
    }
}

void EffectChain::drawPassthrough (juce::OpenGLContext& context, unsigned int sourceTexture, int pixelWidth, int pixelHeight)
{
    ensurePassthroughResources (context);

    if (passthroughProgram == nullptr)
        return;

    glBindFramebuffer (GL_FRAMEBUFFER, 0);
    glViewport (0, 0, pixelWidth, pixelHeight);

    passthroughProgram->use();

    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, sourceTexture);
    juce::OpenGLShaderProgram::Uniform (*passthroughProgram, "sourceImage").set (0);

    auto positionAttribute = glGetAttribLocation (passthroughProgram->getProgramID(), "position");

    if (positionAttribute >= 0)
    {
        glBindBuffer (GL_ARRAY_BUFFER, passthroughVertexBuffer);
        glEnableVertexAttribArray ((GLuint) positionAttribute);
        glVertexAttribPointer ((GLuint) positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray ((GLuint) positionAttribute);
        glBindBuffer (GL_ARRAY_BUFFER, 0);
    }
}

void EffectChain::render (juce::OpenGLContext& context, unsigned int sourceImageTexture,
                           float timeSeconds, int pixelWidth, int pixelHeight,
                           float level, float bass, float mid, float high, float beatphase)
{
    juce::Array<int> activeIndices;
    for (int i = 0; i < stages.size(); ++i)
        if (! stageBypassed[i])
            activeIndices.add (i);

    if (activeIndices.isEmpty())
    {
        drawPassthrough (context, sourceImageTexture, pixelWidth, pixelHeight);
        return;
    }

    unsigned int currentInput = sourceImageTexture;
    int pingIndex = 0;

    for (int n = 0; n < activeIndices.size(); ++n)
    {
        auto stageIndex = activeIndices.getUnchecked (n);
        auto* shader = stages[stageIndex];
        auto& config = configs.getReference (stageIndex);
        bool isLastActive = (n == activeIndices.size() - 1);

        for (auto& mappingEntry : config.audioMappings)
        {
            auto raw = resolveAudioMappingSource (mappingEntry.second.source, level, bass, mid, high, beatphase);
            shader->setValue (mappingEntry.first, raw * mappingEntry.second.scale + mappingEntry.second.offset);
        }

        if (isLastActive)
        {
            shader->render (context, timeSeconds, pixelWidth, pixelHeight,
                            level, bass, mid, high, beatphase, currentInput, 0);
        }
        else
        {
            auto& target = targets[pingIndex];
            ensureTarget (target, pixelWidth, pixelHeight);
            shader->render (context, timeSeconds, pixelWidth, pixelHeight,
                            level, bass, mid, high, beatphase, currentInput, target.fbo);
            currentInput = target.texture;
            pingIndex = 1 - pingIndex;
        }
    }
}

void EffectChain::releaseGLObjects()
{
    for (auto* shader : stages)
        shader->releaseGLObjects();

    for (auto& target : targets)
    {
        if (target.texture != 0) { glDeleteTextures (1, &target.texture); target.texture = 0; }
        if (target.fbo != 0)     { glDeleteFramebuffers (1, &target.fbo); target.fbo = 0; }
        target.width = target.height = 0;
    }

    if (passthroughVertexBuffer != 0)
    {
        glDeleteBuffers (1, &passthroughVertexBuffer);
        passthroughVertexBuffer = 0;
    }

    passthroughProgram.reset();
}
