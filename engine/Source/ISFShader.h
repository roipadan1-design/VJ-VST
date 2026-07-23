#pragma once

#include <JuceHeader.h>

// Minimal ISF (Interactive Shader Format, https://isf.video) host.
// Supports generator-style ISF shaders (no image/audio INPUTS chaining yet -
// that needs multi-pass FBO plumbing and is a later milestone). Parses the
// JSON header for INPUTS, exposes our own audio-reactive uniforms (level,
// bass, mid, high, beatphase) alongside the standard ISF ones (TIME,
// RENDERSIZE, isf_FragNormCoord) so shaders can use either.
struct ISFInput
{
    juce::String name;
    juce::String type; // float, bool, long, point2D, color, event, image
    juce::var defaultValue;
    juce::String label;
};

class ISFShader
{
public:
    ISFShader() = default;

    // Reads and parses the .fs file's JSON header + GLSL body. Does not compile yet.
    bool loadFromFile (const juce::File& file);

    // Compiles the parsed source against the given context. Call whenever the
    // GL context is (re)created. Returns false and fills getLastError() on failure.
    bool compile (juce::OpenGLContext& context);

    // Binds the program, pushes uniforms, and draws the fullscreen quad.
    // Assumes a valid GL context is current (called from OpenGLRenderer::renderOpenGL).
    void render (juce::OpenGLContext& context,
                 float timeSeconds, int pixelWidth, int pixelHeight,
                 float level, float bass, float mid, float high, float beatphase);

    void releaseGLObjects();

    const juce::Array<ISFInput>& getInputs() const noexcept { return inputs; }
    juce::String getLastError() const noexcept { return lastError; }
    bool isCompiled() const noexcept { return program != nullptr; }

private:
    juce::String buildVertexShaderSource() const;
    juce::String buildFragmentShaderSource() const;
    static juce::String glslTypeFor (const juce::String& isfType);

    juce::String rawBody;      // GLSL source after the JSON header comment
    juce::var headerJson;
    juce::Array<ISFInput> inputs;
    juce::String lastError;

    std::unique_ptr<juce::OpenGLShaderProgram> program;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformTime, uniformRenderSize;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformLevel, uniformBass, uniformMid, uniformHigh, uniformBeatPhase;
    juce::OwnedArray<juce::OpenGLShaderProgram::Uniform> inputUniforms; // parallel to `inputs`

    unsigned int vertexBuffer = 0;
    int positionAttribute = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ISFShader)
};
