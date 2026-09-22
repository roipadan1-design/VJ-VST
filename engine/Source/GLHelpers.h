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
