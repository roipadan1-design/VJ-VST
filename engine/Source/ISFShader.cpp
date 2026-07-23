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
                inputs.add (input);
        }
    }

    return true;
}

juce::String ISFShader::glslTypeFor (const juce::String& isfType)
{
    if (isfType == "float")            return "float";
    if (isfType == "bool")             return "bool";
    if (isfType == "event")            return "bool";
    if (isfType == "long")             return "int";
    if (isfType == "point2D")          return "vec2";
    if (isfType == "color")            return "vec4";
    return {}; // "image" and anything unrecognised: unsupported in this minimal host
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

void ISFShader::render (juce::OpenGLContext&, float timeSeconds, int pixelWidth, int pixelHeight,
                         float level, float bass, float mid, float high, float beatphase)
{
    if (program == nullptr)
        return;

    program->use();

    if (uniformTime != nullptr)       uniformTime->set (timeSeconds);
    if (uniformRenderSize != nullptr) uniformRenderSize->set ((float) pixelWidth, (float) pixelHeight);
    if (uniformLevel != nullptr)      uniformLevel->set (level);
    if (uniformBass != nullptr)       uniformBass->set (bass);
    if (uniformMid != nullptr)        uniformMid->set (mid);
    if (uniformHigh != nullptr)       uniformHigh->set (high);
    if (uniformBeatPhase != nullptr)  uniformBeatPhase->set (beatphase);

    for (int i = 0; i < inputs.size(); ++i)
    {
        auto* uniform = inputUniforms[i];
        if (uniform == nullptr)
            continue;

        auto& input = inputs.getReference (i);
        auto glslType = glslTypeFor (input.type);

        if (glslType == "float")
            uniform->set ((float) varAsDouble (input.defaultValue));
        else if (glslType == "bool")
            uniform->set ((int) (varAsDouble (input.defaultValue) != 0.0 ? 1 : 0));
        else if (glslType == "int")
            uniform->set ((int) varAsDouble (input.defaultValue));
        else if (glslType == "vec2")
            uniform->set ((float) varArrayElement (input.defaultValue, 0),
                           (float) varArrayElement (input.defaultValue, 1));
        else if (glslType == "vec4")
            uniform->set ((float) varArrayElement (input.defaultValue, 0),
                           (float) varArrayElement (input.defaultValue, 1),
                           (float) varArrayElement (input.defaultValue, 2),
                           (float) varArrayElement (input.defaultValue, 3));
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
}

void ISFShader::releaseGLObjects()
{
    if (vertexBuffer != 0)
    {
        glDeleteBuffers (1, &vertexBuffer);
        vertexBuffer = 0;
    }

    inputUniforms.clear();
    uniformTime.reset();
    uniformRenderSize.reset();
    uniformLevel.reset();
    uniformBass.reset();
    uniformMid.reset();
    uniformHigh.reset();
    uniformBeatPhase.reset();
    program.reset();
}
