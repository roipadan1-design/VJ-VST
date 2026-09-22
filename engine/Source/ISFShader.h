#pragma once

#include <JuceHeader.h>

// Minimal ISF (Interactive Shader Format, https://isf.video) host.
//
// Supports: INPUTS (float/bool/long/point2D/color/image), multi-pass PASSES
// with named TARGET buffers and PERSISTENT (ping-pong feedback) buffers,
// PASSINDEX, and our own audio-reactive uniforms (level/bass/mid/high/
// beatphase/onset) alongside the ISF-standard ones (TIME, RENDERSIZE,
// isf_FragNormCoord).
//
// Intermediate pass targets are floating point (RGBA16F, or RGBA32F for a
// pass declaring "FLOAT": true), so persistent feedback buffers accumulate
// without 8-bit banding and schema-2 generators can write HDR values.
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
    bool fullFloat = false;   // ISF "FLOAT": true -> RGBA32F (otherwise RGBA16F)
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

    // Runs every pass (allocating/resizing FBOs as needed).
    //
    // externalImageTexture, if non-zero, is bound to the ISF input named
    // exactly "inputImage" (the standard ISF convention for an effect's
    // main image input) instead of the black placeholder - this is how a
    // loaded video frame, or a previous EffectChain stage's output, feeds
    // in as this shader's source image.
    //
    // finalTargetFbo selects where the shader's last pass renders to
    // (default 0 = the default framebuffer/screen, as before). EffectChain
    // uses a non-zero value to redirect a non-final chain stage into its
    // own ping-pong buffer instead of the screen.
    void render (juce::OpenGLContext& context,
                 float timeSeconds, int pixelWidth, int pixelHeight,
                 float level, float bass, float mid, float high, float beatphase, float onset,
                 unsigned int externalImageTexture = 0, unsigned int finalTargetFbo = 0);

    void releaseGLObjects();

    const juce::Array<ISFInput>& getInputs() const noexcept { return inputs; }
    juce::String getLastError() const noexcept { return lastError; }
    bool isCompiled() const noexcept { return program != nullptr; }

    void setValue (const juce::String& name, juce::var value);
    juce::var getValue (const juce::String& name) const;

    // Engine-provided uniforms every shader may declare-and-use for free:
    //   vj_palette  smoothed palette-advance step count (schema-2 triggers)
    //   vj_seed     per-preset seed, bumped by "reseed" actions
    //   vj_beat     musical position in quarter notes (host transport or free-run clock)
    // Binds a texture to a named "image" INPUT (schema-2 sources: still images,
    // rendered text). width/height feed the shader's `uniform vec2 <name>_size`,
    // declared automatically for every image input and IMPORTED image.
    void setImageTexture (const juce::String& inputName, unsigned int texture, int width, int height)
    {
        boundImages[inputName] = { texture, (float) width, (float) height };
    }

    void setEngineUniforms (float palette, float seed, float beat) noexcept
    {
        enginePalette = palette;
        engineSeed = seed;
        engineBeat = beat;
    }

private:
    struct RenderTarget
    {
        unsigned int fbo = 0;
        unsigned int textures[2] { 0, 0 };
        int width = 0, height = 0;
        bool persistent = false;
        bool fullFloat = false;
        int writeIndex = 0;
    };

    juce::String buildVertexShaderSource() const;
    juce::String buildFragmentShaderSource() const;
    static juce::String glslTypeFor (const juce::String& isfType);
    static double parseSimpleDimensionExpression (const juce::var& value);

    void ensureRenderTarget (RenderTarget& rt, int width, int height, bool persistent, bool fullFloat);
    void ensureBlackPlaceholderTexture();
    void runPass (int passIndex, int mainWidth, int mainHeight,
                  float timeSeconds, float level, float bass, float mid, float high, float beatphase, float onset,
                  unsigned int externalImageTexture, unsigned int finalTargetFbo);

    struct BoundImage { unsigned int texture = 0; float width = 1.0f, height = 1.0f; };
    struct ImportedImage { juce::String name; juce::File file; unsigned int texture = 0; int width = 1, height = 1; };

    std::map<juce::String, BoundImage> boundImages;
    juce::Array<ImportedImage> imported;          // ISF "IMPORTED" stills, loaded on compile()
    juce::OwnedArray<juce::OpenGLShaderProgram::Uniform> sizeUniforms;     // parallel to inputs (image inputs only)
    juce::OwnedArray<juce::OpenGLShaderProgram::Uniform> importedUniforms, importedSizeUniforms;
    juce::File shaderFile;

    juce::String rawBody;      // GLSL source after the JSON header comment
    juce::var headerJson;
    juce::Array<ISFInput> inputs;
    juce::Array<ISFPass> passes;
    juce::NamedValueSet currentValues; // name -> current value, seeded from DEFAULTs on load
    juce::String lastError;

    std::unique_ptr<juce::OpenGLShaderProgram> program;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformTime, uniformRenderSize, uniformPassIndex;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformLevel, uniformBass, uniformMid, uniformHigh, uniformBeatPhase, uniformOnset;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformPalette, uniformSeed, uniformBeat;
    float enginePalette = 0.0f, engineSeed = 0.0f, engineBeat = 0.0f;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uniformTimeDelta;
    float lastRenderTime = -1.0f, timeDelta = 1.0f / 60.0f;
    juce::OwnedArray<juce::OpenGLShaderProgram::Uniform> inputUniforms;  // parallel to `inputs`
    juce::OwnedArray<juce::OpenGLShaderProgram::Uniform> targetUniforms; // one per unique TARGET name referenced
    juce::StringArray targetUniformNames;                                // parallel to targetUniforms

    std::map<juce::String, RenderTarget> renderTargets; // keyed by TARGET name
    unsigned int blackPlaceholderTexture = 0;

    unsigned int vertexBuffer = 0;
    int positionAttribute = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ISFShader)
};
