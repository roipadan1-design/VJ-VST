#pragma once

#include <JuceHeader.h>

// Small shared GL utilities for the v2 render path (the legacy classes keep
// their own equivalents).

// One colour texture + FBO. Reallocates only when size or format changes.
struct GLRenderTarget
{
    unsigned int fbo = 0, texture = 0;
    int width = 0, height = 0;
    unsigned int internalFormat = 0;

    // internalFormat: GL_RGBA8, GL_RGBA16F or GL_RGBA32F. Clears to black on (re)allocation.
    void ensure (int w, int h, unsigned int format);
    void bind() const;                  // bind for drawing + set the viewport
    void release();
    bool isValid() const noexcept { return fbo != 0; }
};

// A fullscreen triangle strip drawn with a program exposing `attribute vec2 position`.
class FullscreenQuad
{
public:
    void draw (juce::OpenGLShaderProgram& program);
    void release();

private:
    unsigned int vertexBuffer = 0;
};

// Uploads a juce::Image as an RGBA8 mipmapped texture (rows flipped so the
// image is upright in GL's bottom-left convention). Caller owns the texture.
unsigned int uploadImageTexture (const juce::Image& image);

// Builds a program from the standard fullscreen vertex shader (which provides
// `varying vec2 uv` in 0..1) and the given fragment source.
std::unique_ptr<juce::OpenGLShaderProgram> buildFullscreenProgram (juce::OpenGLContext& context,
                                                                   const juce::String& fragmentSource,
                                                                   juce::String& error);

// ---- optional GPU timing (measurement only; only when the VJ_GPUTIMING
// environment variable is set to anything but "" or "0"). Brackets GPU work
// with GL_TIMESTAMP queries, reads results back a few frames later without
// ever waiting on the GPU, and every ~5 s appends per-section median / p90 ms
// to VJEngine.log ("GPU timing" lines). When unset (or when the driver lacks
// timer queries) every call is a no-op and no GL call is made, so it can never
// change what gets rendered. GL render thread only.
//
//   const int t = gpuTiming::enabled() ? gpuTiming::begin ("label") : -1;
//   ...GPU work...
//   gpuTiming::end (t);
//   ...once per frame, after all work: gpuTiming::endFrame ("context");
namespace gpuTiming
{
    bool enabled();                              // cached; checks the env var and driver support once
    int begin (const juce::String& label);       // returns a section handle, or -1
    void end (int section) noexcept;             // -1 is ignored
    void endFrame (const juce::String& context); // resolves finished frames, logs periodically
}
