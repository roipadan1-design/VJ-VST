#include "ISFShader.h"

using namespace juce::gl;

namespace
{
    double varAsDouble (const juce::var& v, double fallback = 0.0)
    {
        return v.isVoid() ? fallback : (double) v;
    }

    double varArrayElement (const juce::var& v, int index, double fallback = 0.0)
    {
        if (auto* arr = v.getArray())
            if (juce::isPositiveAndBelow (index, arr->size()))
                return (double) (*arr)[index];
        return fallback;
    }
}

ISFShader::~ISFShader() = default;

bool ISFShader::loadFromFile (const juce::File& file)
{
    lastError.clear();

    if (! file.existsAsFile())
    {
        lastError = "File does not exist: " + file.getFullPathName();
        return false;
    }

    auto text = file.loadFileAsString();

    auto commentStart = text.indexOf ("/*");
    if (commentStart < 0)
    {
        lastError = "No ISF JSON header comment found (expected a leading /* ... */ block)";
        return false;
    }

    auto jsonStart = commentStart + 2;
    auto commentEnd = text.indexOf (jsonStart, "*/");
    if (commentEnd < 0)
    {
        lastError = "ISF JSON header comment was not closed with */";
        return false;
    }

    auto jsonText = text.substring (jsonStart, commentEnd);
    auto parsed = juce::JSON::parse (jsonText);

    if (! parsed.isObject())
    {
        lastError = "Failed to parse ISF JSON header: " + jsonText.substring (0, 200);
        return false;
    }

    headerJson = parsed;
    rawBody = text.substring (commentEnd + 2);

    inputs.clear();
    currentValues.clear();

    if (auto* inputsArray = headerJson.getProperty ("INPUTS", juce::var()).getArray())
    {
        for (auto& entry : *inputsArray)
        {
            ISFInput input;
            input.name = entry.getProperty ("NAME", juce::var()).toString();
            input.type = entry.getProperty ("TYPE", juce::var()).toString();
            input.label = entry.getProperty ("LABEL", input.name).toString();
            input.defaultValue = entry.getProperty ("DEFAULT", juce::var());

            if (input.name.isNotEmpty() && input.type.isNotEmpty())
            {
                inputs.add (input);
                currentValues.set (input.name, input.defaultValue);
            }
        }
    }

    passes.clear();

    if (auto* passesArray = headerJson.getProperty ("PASSES", juce::var()).getArray())
    {
        for (auto& entry : *passesArray)
        {
            ISFPass pass;
            pass.target = entry.getProperty ("TARGET", juce::var()).toString();
            pass.persistent = (bool) entry.getProperty ("PERSISTENT", false);
            pass.widthScale = parseSimpleDimensionExpression (entry.getProperty ("WIDTH", juce::var()));
            pass.heightScale = parseSimpleDimensionExpression (entry.getProperty ("HEIGHT", juce::var()));
            passes.add (pass);
        }
    }

    if (passes.isEmpty())
        passes.add (ISFPass()); // implicit single pass straight to screen

    return true;
}

double ISFShader::parseSimpleDimensionExpression (const juce::var& value)
{
    if (value.isVoid())
        return 1.0;

    if (value.isDouble() || value.isInt() || value.isInt64())
        return (double) value;

    auto text = value.toString();

    // Plain numeric literal ("0.5") - the common case.
    if (text.containsOnly ("0123456789.-"))
        return text.getDoubleValue();

    // Anything fancier ("floor($WIDTH*min((0.2),1.0))") - heuristic only: grab
    // the first decimal literal in the expression. Not a real expression
    // evaluator; documented as a known limitation.
    juce::String digits;
    bool seenDot = false;
    for (auto i = 0; i < text.length(); ++i)
    {
        auto c = text[i];
        if (juce::CharacterFunctions::isDigit (c) || (c == '.' && ! seenDot))
        {
            digits += c;
            if (c == '.') seenDot = true;
        }
        else if (digits.isNotEmpty())
        {
            break;
        }
    }

    return digits.isNotEmpty() ? digits.getDoubleValue() : 1.0;
}

void ISFShader::setValue (const juce::String& name, juce::var value)
{
    if (currentValues.contains (name))
        currentValues.set (name, value);
}

juce::var ISFShader::getValue (const juce::String& name) const
{
    return currentValues[name];
}

juce::String ISFShader::glslTypeFor (const juce::String& isfType)
{
    if (isfType == "float")            return "float";
    if (isfType == "bool")             return "bool";
    if (isfType == "event")            return "bool";
    if (isfType == "long")             return "int";
    if (isfType == "point2D")          return "vec2";
    if (isfType == "color")            return "vec4";
    if (isfType == "image")            return "sampler2D";
    return {};
}

juce::String ISFShader::buildVertexShaderSource() const
{
    return R"(
        attribute vec2 position;
        varying vec2 isf_FragNormCoord;
        void main()
        {
            gl_Position = vec4 (position, 0.0, 1.0);
            isf_FragNormCoord = position * 0.5 + 0.5;
        }
    )";
}

juce::String ISFShader::buildFragmentShaderSource() const
{
    juce::String src;

    src << "#ifdef GL_ES\n precision mediump float;\n#endif\n";
    src << "varying vec2 isf_FragNormCoord;\n";
    src << "uniform float TIME;\n";
    src << "uniform vec2 RENDERSIZE;\n";
    src << "uniform int PASSINDEX;\n";
    src << "uniform float level;\nuniform float bass;\nuniform float mid;\nuniform float high;\nuniform float beatphase;\n";

    for (auto& input : inputs)
    {
        auto glslType = glslTypeFor (input.type);

        if (glslType.isEmpty())
        {
            src << "// skipped unsupported ISF input '" << input.name << "' (type " << input.type << ")\n";
            continue;
        }

        src << "uniform " << glslType << " " << input.name << ";\n";
    }

    juce::StringArray declaredTargets;
    for (auto& pass : passes)
    {
        if (pass.target.isNotEmpty() && ! declaredTargets.contains (pass.target))
        {
            declaredTargets.add (pass.target);
            src << "uniform sampler2D " << pass.target << ";\n";
        }
    }

    // Standard ISF image-sampling helpers.
    src << "vec4 IMG_NORM_PIXEL (sampler2D img, vec2 normCoord) { return texture2D (img, normCoord); }\n";
    src << "vec4 IMG_PIXEL (sampler2D img, vec2 pixelCoord) { return texture2D (img, pixelCoord / RENDERSIZE); }\n";
    src << "vec4 IMG_THIS_PIXEL (sampler2D img) { return texture2D (img, isf_FragNormCoord); }\n";
    src << "vec4 IMG_THIS_NORM_PIXEL (sampler2D img) { return texture2D (img, isf_FragNormCoord); }\n";

    src << rawBody;

    return src;
}

bool ISFShader::compile (juce::OpenGLContext& context)
{
    lastError.clear();

    auto newProgram = std::make_unique<juce::OpenGLShaderProgram> (context);

    auto vertexSrc = juce::OpenGLHelpers::translateVertexShaderToV3 (buildVertexShaderSource());
    auto fragmentSrc = juce::OpenGLHelpers::translateFragmentShaderToV3 (buildFragmentShaderSource());

    if (! newProgram->addVertexShader (vertexSrc))
    {
        lastError = "Vertex shader error: " + newProgram->getLastError();
        return false;
    }

    if (! newProgram->addFragmentShader (fragmentSrc))
    {
        lastError = "Fragment shader error: " + newProgram->getLastError();
        return false;
    }

    if (! newProgram->link())
    {
        lastError = "Link error: " + newProgram->getLastError();
        return false;
    }

    program.reset (newProgram.release());

    uniformTime.reset       (new juce::OpenGLShaderProgram::Uniform (*program, "TIME"));
    uniformRenderSize.reset (new juce::OpenGLShaderProgram::Uniform (*program, "RENDERSIZE"));
    uniformPassIndex.reset  (new juce::OpenGLShaderProgram::Uniform (*program, "PASSINDEX"));
    uniformLevel.reset      (new juce::OpenGLShaderProgram::Uniform (*program, "level"));
    uniformBass.reset       (new juce::OpenGLShaderProgram::Uniform (*program, "bass"));
    uniformMid.reset        (new juce::OpenGLShaderProgram::Uniform (*program, "mid"));
    uniformHigh.reset       (new juce::OpenGLShaderProgram::Uniform (*program, "high"));
    uniformBeatPhase.reset  (new juce::OpenGLShaderProgram::Uniform (*program, "beatphase"));

    inputUniforms.clear();
    for (auto& input : inputs)
    {
        if (glslTypeFor (input.type).isEmpty())
        {
            inputUniforms.add (nullptr);
            continue;
        }

        inputUniforms.add (new juce::OpenGLShaderProgram::Uniform (*program, input.name.toRawUTF8()));
    }

    targetUniforms.clear();
    targetUniformNames.clear();
    for (auto& pass : passes)
    {
        if (pass.target.isNotEmpty() && ! targetUniformNames.contains (pass.target))
        {
            targetUniformNames.add (pass.target);
            targetUniforms.add (new juce::OpenGLShaderProgram::Uniform (*program, pass.target.toRawUTF8()));
        }
    }

    positionAttribute = glGetAttribLocation (program->getProgramID(), "position");

    if (vertexBuffer == 0)
    {
        glGenBuffers (1, &vertexBuffer);
        glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
        static const GLfloat quad[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
        glBufferData (GL_ARRAY_BUFFER, sizeof (quad), quad, GL_STATIC_DRAW);
        glBindBuffer (GL_ARRAY_BUFFER, 0);
    }

    return true;
}

void ISFShader::ensureBlackPlaceholderTexture()
{
    if (blackPlaceholderTexture != 0)
        return;

    glGenTextures (1, &blackPlaceholderTexture);
    glBindTexture (GL_TEXTURE_2D, blackPlaceholderTexture);
    const unsigned char blackPixel[4] = { 0, 0, 0, 255 };
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blackPixel);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture (GL_TEXTURE_2D, 0);
}

void ISFShader::ensureRenderTarget (RenderTarget& rt, int width, int height, bool persistent)
{
    width = juce::jmax (1, width);
    height = juce::jmax (1, height);

    if (rt.fbo != 0 && rt.width == width && rt.height == height && rt.persistent == persistent)
        return;

    if (rt.fbo == 0)
        glGenFramebuffers (1, &rt.fbo);

    if (rt.textures[0] == 0)
        glGenTextures (2, rt.textures);

    for (int i = 0; i < 2; ++i)
    {
        glBindTexture (GL_TEXTURE_2D, rt.textures[i]);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBindTexture (GL_TEXTURE_2D, 0);

    // Clear both buffers once so persistent (feedback) targets don't read garbage on frame 1.
    glBindFramebuffer (GL_FRAMEBUFFER, rt.fbo);
    for (int i = 0; i < 2; ++i)
    {
        glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rt.textures[i], 0);
        glViewport (0, 0, width, height);
        juce::OpenGLHelpers::clear (juce::Colours::black);
    }
    glBindFramebuffer (GL_FRAMEBUFFER, 0);

    rt.width = width;
    rt.height = height;
    rt.persistent = persistent;
    rt.writeIndex = 0;
}

void ISFShader::runPass (int passIndex, int mainWidth, int mainHeight,
                          float timeSeconds, float level, float bass, float mid, float high, float beatphase,
                          unsigned int externalImageTexture, unsigned int finalTargetFbo)
{
    auto& pass = passes.getReference (passIndex);
    bool isLastPass = (passIndex == passes.size() - 1);

    int targetWidth = mainWidth;
    int targetHeight = mainHeight;
    RenderTarget* writingTo = nullptr;

    if (pass.target.isNotEmpty() && ! isLastPass)
    {
        targetWidth  = juce::roundToInt (mainWidth  * pass.widthScale);
        targetHeight = juce::roundToInt (mainHeight * pass.heightScale);

        auto& rt = renderTargets[pass.target];
        ensureRenderTarget (rt, targetWidth, targetHeight, pass.persistent);
        writingTo = &rt;

        glBindFramebuffer (GL_FRAMEBUFFER, rt.fbo);
        glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rt.textures[rt.writeIndex], 0);
    }
    else
    {
        glBindFramebuffer (GL_FRAMEBUFFER, finalTargetFbo);
    }

    glViewport (0, 0, targetWidth, targetHeight);

    program->use();

    if (uniformTime != nullptr)       uniformTime->set (timeSeconds);
    if (uniformRenderSize != nullptr) uniformRenderSize->set ((float) targetWidth, (float) targetHeight);
    if (uniformPassIndex != nullptr)  uniformPassIndex->set (passIndex);
    if (uniformLevel != nullptr)      uniformLevel->set (level);
    if (uniformBass != nullptr)       uniformBass->set (bass);
    if (uniformMid != nullptr)        uniformMid->set (mid);
    if (uniformHigh != nullptr)       uniformHigh->set (high);
    if (uniformBeatPhase != nullptr)  uniformBeatPhase->set (beatphase);

    int textureUnit = 0;

    for (int i = 0; i < inputs.size(); ++i)
    {
        auto& input = inputs.getReference (i);
        if (glslTypeFor (input.type) != "sampler2D")
            continue;

        auto* uniform = inputUniforms[i];
        if (uniform == nullptr)
            continue;

        // Standard ISF convention: an effect's main image input is named
        // "inputImage". That's the one slot an external source (a loaded
        // video frame, or the previous EffectChain stage's output) binds to
        // - anything else stays the black placeholder (no real multi-image
        // input support yet, documented scope limit as before).
        auto boundTexture = (externalImageTexture != 0 && input.name == "inputImage")
                                 ? externalImageTexture
                                 : blackPlaceholderTexture;

        glActiveTexture (GL_TEXTURE0 + textureUnit);
        glBindTexture (GL_TEXTURE_2D, boundTexture);
        uniform->set (textureUnit);
        ++textureUnit;
    }

    for (int i = 0; i < targetUniformNames.size(); ++i)
    {
        auto& name = targetUniformNames.getReference (i);
        auto* uniform = targetUniforms[i];
        if (uniform == nullptr)
            continue;

        auto found = renderTargets.find (name);
        auto textureId = blackPlaceholderTexture;

        if (found != renderTargets.end())
        {
            auto& rt = found->second;
            auto readIndex = rt.persistent ? (1 - rt.writeIndex) : rt.writeIndex;
            textureId = rt.textures[readIndex];
        }

        glActiveTexture (GL_TEXTURE0 + textureUnit);
        glBindTexture (GL_TEXTURE_2D, textureId);
        uniform->set (textureUnit);
        ++textureUnit;
    }

    for (int i = 0; i < inputs.size(); ++i)
    {
        auto* uniform = inputUniforms[i];
        if (uniform == nullptr)
            continue;

        auto& input = inputs.getReference (i);
        auto glslType = glslTypeFor (input.type);
        auto currentValue = currentValues[input.name];

        if (glslType == "float")
            uniform->set ((float) varAsDouble (currentValue));
        else if (glslType == "bool")
            uniform->set ((int) (varAsDouble (currentValue) != 0.0 ? 1 : 0));
        else if (glslType == "int")
            uniform->set ((int) varAsDouble (currentValue));
        else if (glslType == "vec2")
            uniform->set ((float) varArrayElement (currentValue, 0),
                           (float) varArrayElement (currentValue, 1));
        else if (glslType == "vec4")
            uniform->set ((float) varArrayElement (currentValue, 0),
                           (float) varArrayElement (currentValue, 1),
                           (float) varArrayElement (currentValue, 2),
                           (float) varArrayElement (currentValue, 3));
    }

    if (positionAttribute >= 0)
    {
        glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
        glEnableVertexAttribArray ((GLuint) positionAttribute);
        glVertexAttribPointer ((GLuint) positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray ((GLuint) positionAttribute);
        glBindBuffer (GL_ARRAY_BUFFER, 0);
    }

    if (writingTo != nullptr && writingTo->persistent)
        writingTo->writeIndex = 1 - writingTo->writeIndex;
}

void ISFShader::render (juce::OpenGLContext&, float timeSeconds, int pixelWidth, int pixelHeight,
                         float level, float bass, float mid, float high, float beatphase,
                         unsigned int externalImageTexture, unsigned int finalTargetFbo)
{
    if (program == nullptr || passes.isEmpty())
        return;

    ensureBlackPlaceholderTexture();

    for (int i = 0; i < passes.size(); ++i)
        runPass (i, pixelWidth, pixelHeight, timeSeconds, level, bass, mid, high, beatphase,
                 externalImageTexture, finalTargetFbo);

    glBindFramebuffer (GL_FRAMEBUFFER, 0);
    glViewport (0, 0, pixelWidth, pixelHeight);
}

void ISFShader::releaseGLObjects()
{
    if (vertexBuffer != 0)
    {
        glDeleteBuffers (1, &vertexBuffer);
        vertexBuffer = 0;
    }

    if (blackPlaceholderTexture != 0)
    {
        glDeleteTextures (1, &blackPlaceholderTexture);
        blackPlaceholderTexture = 0;
    }

    for (auto& entry : renderTargets)
    {
        auto& rt = entry.second;
        if (rt.textures[0] != 0) glDeleteTextures (2, rt.textures);
        if (rt.fbo != 0) glDeleteFramebuffers (1, &rt.fbo);
    }
    renderTargets.clear();

    inputUniforms.clear();
    targetUniforms.clear();
    targetUniformNames.clear();
    uniformTime.reset();
    uniformRenderSize.reset();
    uniformPassIndex.reset();
    uniformLevel.reset();
    uniformBass.reset();
    uniformMid.reset();
    uniformHigh.reset();
    uniformBeatPhase.reset();
    program.reset();
}
