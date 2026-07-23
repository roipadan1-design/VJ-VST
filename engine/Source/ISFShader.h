#pragma once

#include <JuceHeader.h>

// Minimal ISF (Interactive Shader Format, https://isf.video) host.
//
// Supports: INPUTS (float/bool/long/point2D/color/image), multi-pass PASSES
// with named TARGET buffers and PERSISTENT (ping-pong feedback) buffers,
// PASSINDEX, and our own audio-reactive uniforms (level/bass/mid/high/
// beatphase) alongside the ISF-standard ones (TIME, RENDERSIZE,
// isf_FragNormCoord).
//
// NOT supported yet (real-world ISF effects sometimes need these):
//  - a shader-specific companion vertex shader (.vs file) - we always use our
//    own minimal vertex shader, so ISF files that declare extra varyings
//    computed per-vertex (e.g. blur tap offsets) won't compile.
//  - full WIDTH/HEIGHT math expressions for PASSES ("floor($WIDTH*0.2)") -
//    only plain numeric literals are parsed; anything else falls back to 1.0.
//  - "image" INPUTS are bound to a 1x1 black placeholder texture - there's no
//    real camera/video/upstream-image feed into this engine yet.
struct ISFInput
{
    juce::String name;
    juce::String type; // float, bool, long, point2D, color, event, image
    juce::var defaultValue;
    juce::String label;
};

struct ISFPass
{
    juce::String target;    // empty = renders to screen (always true for the last pass too)
    bool persistent = false;
    double widthScale = 1.0;  // fraction of the main render size; see limitations above
    double heightScale = 1.0;
};

class ISFShader
{
public:
    ISFShader() = default;
    ~ISFShader();

    // Reads and parses the .fs file's JSON header + GLSL body. Does not compile yet.
    bool loadFromFile (const juce::File& file);

    // Compiles the parsed source against the given context. Call whenever the
    // GL context is (re)created. Returns false and fills getLastError() on failure.
    bool compile (juce::OpenGLContext& context);

    // Runs every pass (allocating/resizing FBOs as needed) and leaves the
    // default framebuffer bound with the final pass's image on screen.
    void render (juce::OpenGLContext& context,
                 float timeSeconds, int pixelWidth, int pixelHeight,
                 float level, float bass, float mid, float high, float beatphase);

    void releaseGLObjects();

    const juce::Array<ISFInput>& getInputs() const noexcept { return inputs; }
    juce::String getLastError() const noexcept { return lastError; }
    bool isCompiled() const noexcept { return program != nullptr; }

    void setValue (const juce::String& name, juce::var value);
    juce::var getValue (const juce::String& name) const;

private:
    struct RenderTarget
    {
        unsigned int fbo = 0;
        unsigned int textures[2] { 0, 0 };
        int width = 0, height = 0;
        bool persistent = false;
        int writeIndex = 0;
    };

    juce::String buildVertexShaderSource() const;
    juce::String buildFragmentShaderSource() const;
    static juce::String glslTypeFor (const juce::String& isfType);
    static double parseSimpleDimensionExpression (const juce::var& value);

    void ensureRenderTarget (RenderTarget& rt, int width, int height, bool persistent);
    void ensureBlackPlaceholderTexture();
    void runPass (int passIndex, int mainWidth, int mainHeight,
                  float timeSeconds, float level, float bass, float mid, float high, float beatphase);

    juce::String rawBody;      // GLSL source after the JSON header comment
    juce::var headerJson;
    juce::Array<ISFInput> inputs;
    juce::Array<ISFPass> passes;
    juce::NamedValueSet currentValues; // name -> current value, seeded from DEFAULTs on load
    juce::String lastError;

    std::unique_ptr<juce::OpenGLShaderProgram> program;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformTime, uniformRenderSize, uniformPassIndex;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformLevel, uniformBass, uniformMid, uniformHigh, uniformBeatPhase;
    juce::OwnedArray<juce::OpenGLShaderProgram::Uniform> inputUniforms;  // parallel to `inputs`
    juce::OwnedArray<juce::OpenGLShaderProgram::Uniform> targetUniforms; // one per unique TARGET name referenced
    juce::StringArray targetUniformNames;                                // parallel to targetUniforms

    std::map<juce::String, RenderTarget> renderTargets; // keyed by TARGET name
    unsigned int blackPlaceholderTexture = 0;

    unsigned int vertexBuffer = 0;
    int positionAttribute = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ISFShader)
};
