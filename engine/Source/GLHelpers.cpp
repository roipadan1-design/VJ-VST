#include "GLHelpers.h"

using namespace juce::gl;

void GLRenderTarget::ensure (int w, int h, unsigned int format)
{
    w = juce::jmax (1, w);
    h = juce::jmax (1, h);

    if (fbo != 0 && width == w && height == h && internalFormat == format)
        return;

    if (fbo == 0)     glGenFramebuffers (1, &fbo);
    if (texture == 0) glGenTextures (1, &texture);

    const bool isFloat = (format == GL_RGBA16F || format == GL_RGBA32F);

    glBindTexture (GL_TEXTURE_2D, texture);
    glTexImage2D (GL_TEXTURE_2D, 0, (GLint) format, w, h, 0, GL_RGBA, isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture (GL_TEXTURE_2D, 0);

    glBindFramebuffer (GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glViewport (0, 0, w, h);
    glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
    glClear (GL_COLOR_BUFFER_BIT);
    glBindFramebuffer (GL_FRAMEBUFFER, 0);

    width = w;
    height = h;
    internalFormat = format;
}

void GLRenderTarget::bind() const
{
    glBindFramebuffer (GL_FRAMEBUFFER, fbo);
    glViewport (0, 0, width, height);
}

void GLRenderTarget::release()
{
    if (texture != 0) { glDeleteTextures (1, &texture); texture = 0; }
    if (fbo != 0)     { glDeleteFramebuffers (1, &fbo); fbo = 0; }
    width = height = 0;
    internalFormat = 0;
}

void FullscreenQuad::draw (juce::OpenGLShaderProgram& program)
{
    if (vertexBuffer == 0)
    {
        glGenBuffers (1, &vertexBuffer);
        glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
        static const GLfloat quad[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
        glBufferData (GL_ARRAY_BUFFER, sizeof (quad), quad, GL_STATIC_DRAW);
    }

    auto position = glGetAttribLocation (program.getProgramID(), "position");
    if (position < 0)
        return;

    glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray ((GLuint) position);
    glVertexAttribPointer ((GLuint) position, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray ((GLuint) position);
    glBindBuffer (GL_ARRAY_BUFFER, 0);
}

void FullscreenQuad::release()
{
    if (vertexBuffer != 0)
    {
        glDeleteBuffers (1, &vertexBuffer);
        vertexBuffer = 0;
    }
}

unsigned int uploadImageTexture (const juce::Image& source)
{
    auto image = source.convertedToFormat (juce::Image::ARGB);
    const int w = image.getWidth(), h = image.getHeight();

    // JUCE ARGB is BGRA in memory on little-endian; copy bottom row first.
    juce::HeapBlock<juce::uint8> pixels ((size_t) w * (size_t) h * 4);
    {
        juce::Image::BitmapData bitmap (image, juce::Image::BitmapData::readOnly);
        for (int y = 0; y < h; ++y)
            std::memcpy (pixels.getData() + (size_t) (h - 1 - y) * (size_t) w * 4, bitmap.getLinePointer (y), (size_t) w * 4);
    }

    unsigned int texture = 0;
    glGenTextures (1, &texture);
    glBindTexture (GL_TEXTURE_2D, texture);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels.getData());
    glGenerateMipmap (GL_TEXTURE_2D);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture (GL_TEXTURE_2D, 0);
    return texture;
}

std::unique_ptr<juce::OpenGLShaderProgram> buildFullscreenProgram (juce::OpenGLContext& context,
                                                                   const juce::String& fragmentSource,
                                                                   juce::String& error)
{
    auto program = std::make_unique<juce::OpenGLShaderProgram> (context);

    auto vertex = juce::OpenGLHelpers::translateVertexShaderToV3 (R"(
        attribute vec2 position;
        varying vec2 uv;
        void main() { gl_Position = vec4 (position, 0.0, 1.0); uv = position * 0.5 + 0.5; }
    )");

    auto fragment = juce::OpenGLHelpers::translateFragmentShaderToV3 ("varying vec2 uv;\n" + fragmentSource);

    if (! program->addVertexShader (vertex) || ! program->addFragmentShader (fragment) || ! program->link())
    {
        error = program->getLastError();
        return nullptr;
    }

    return program;
}
