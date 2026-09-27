#include "GLHelpers.h"
#include "Diagnostics.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <vector>

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

// ---- optional GPU timing -----------------------------------------------------
namespace gpuTiming
{
    namespace
    {
        struct Section { juce::String label; GLuint start = 0, stop = 0; bool closed = false; };
        struct Frame   { std::vector<Section> sections; };
        struct Series  { juce::String label; std::vector<float> ms; };

        struct State
        {
            int status = -1;                // -1 not checked yet, 0 off, 1 on
            Frame current;
            std::deque<Frame> pending;      // issued, waiting for the GPU
            std::vector<GLuint> spare;      // resolved queries, safe to reuse
            std::vector<Series> series;     // this log window, in first-seen order
            juce::String context;
            int resolvedFrames = 0;
            double windowStart = -1.0;
        };

        constexpr int maxPendingFrames = 8;        // older unresolved frames are dropped, never waited on
        constexpr int maxSectionsPerFrame = 256;
        constexpr double logIntervalSeconds = 5.0;

        State& state()
        {
            static State s;
            return s;
        }

        GLuint takeQuery (State& s)
        {
            if (! s.spare.empty())
            {
                const auto q = s.spare.back();
                s.spare.pop_back();
                return q;
            }
            GLuint q = 0;
            glGenQueries (1, &q);
            return q;
        }

        // Queries that may still be in flight are deleted rather than reused.
        void discard (Frame& f)
        {
            for (auto& sec : f.sections)
            {
                const GLuint q[2] { sec.start, sec.stop };
                glDeleteQueries (2, q);
            }
            f.sections.clear();
        }

        bool isReady (GLuint q)
        {
            GLint available = 0;
            glGetQueryObjectiv (q, GL_QUERY_RESULT_AVAILABLE, &available);
            return available != 0;
        }

        Series& seriesFor (State& s, const juce::String& label)
        {
            for (auto& x : s.series)
                if (x.label == label)
                    return x;
            s.series.push_back ({ label, {} });
            return s.series.back();
        }

        float percentile (const std::vector<float>& sorted, double p)
        {
            if (sorted.empty())
                return 0.0f;
            return sorted[(size_t) juce::roundToInt (p * (double) (sorted.size() - 1))];
        }
    }

    bool enabled()
    {
        auto& s = state();
        if (s.status >= 0)
            return s.status == 1;

        s.status = 0;
        const char* value = std::getenv ("VJ_GPUTIMING");
        if (value == nullptr || *value == 0 || std::strcmp (value, "0") == 0)
            return false;

        if (glGenQueries == nullptr || glDeleteQueries == nullptr || glQueryCounter == nullptr
            || glGetQueryiv == nullptr || glGetQueryObjectiv == nullptr || glGetQueryObjectui64v == nullptr)
        {
            logDiagnostic ("GPU timing: VJ_GPUTIMING is set, but this GL context has no timer queries - off");
            return false;
        }

        GLint bits = 0;
        glGetQueryiv (GL_TIMESTAMP, GL_QUERY_COUNTER_BITS, &bits);
        if (bits <= 0)
        {
            logDiagnostic ("GPU timing: VJ_GPUTIMING is set, but GL_TIMESTAMP has 0 counter bits - off");
            return false;
        }

        s.status = 1;
        logDiagnostic ("GPU timing: on (GL_TIMESTAMP, " + juce::String (bits) + " bits); per-section median / p90 / max ms every "
                       + juce::String (logIntervalSeconds, 0) + " s");
        return true;
    }

    int begin (const juce::String& label)
    {
        if (! enabled())
            return -1;

        auto& s = state();
        if ((int) s.current.sections.size() >= maxSectionsPerFrame)
            return -1;

        Section sec;
        sec.label = label;
        sec.start = takeQuery (s);
        sec.stop = takeQuery (s);
        glQueryCounter (sec.start, GL_TIMESTAMP);
        s.current.sections.push_back (std::move (sec));
        return (int) s.current.sections.size() - 1;
    }

    void end (int section) noexcept
    {
        auto& s = state();
        if (section < 0 || s.status != 1 || section >= (int) s.current.sections.size())
            return;

        auto& sec = s.current.sections[(size_t) section];
        if (! sec.closed)
        {
            glQueryCounter (sec.stop, GL_TIMESTAMP);
            sec.closed = true;
        }
    }

    void endFrame (const juce::String& context)
    {
        if (! enabled())
            return;

        auto& s = state();

        // Close out this frame: drop sections that never ended.
        {
            Frame done;
            for (auto& sec : s.current.sections)
            {
                if (sec.closed)
                {
                    done.sections.push_back (std::move (sec));
                }
                else
                {
                    const GLuint q[2] { sec.start, sec.stop };
                    glDeleteQueries (2, q);
                }
            }
            s.current.sections.clear();
            if (! done.sections.empty())
                s.pending.push_back (std::move (done));
        }

        // A new preset / output size / render scale starts a fresh window.
        if (context != s.context)
        {
            s.context = context;
            s.series.clear();
            s.resolvedFrames = 0;
            s.windowStart = -1.0;
        }

        // Resolve every frame the GPU has finished; never wait on it.
        while (! s.pending.empty())
        {
            auto& f = s.pending.front();
            bool ready = true;
            for (auto& sec : f.sections)
            {
                if (! isReady (sec.start) || ! isReady (sec.stop))
                {
                    ready = false;
                    break;
                }
            }
            if (! ready)
                break;

            for (auto& sec : f.sections)
            {
                GLuint64 t0 = 0, t1 = 0;
                glGetQueryObjectui64v (sec.start, GL_QUERY_RESULT, &t0);
                glGetQueryObjectui64v (sec.stop, GL_QUERY_RESULT, &t1);
                seriesFor (s, sec.label).ms.push_back (t1 >= t0 ? (float) ((double) (t1 - t0) * 1.0e-6) : 0.0f);
                s.spare.push_back (sec.start);
                s.spare.push_back (sec.stop);
            }
            ++s.resolvedFrames;
            s.pending.pop_front();
        }

        while ((int) s.pending.size() > maxPendingFrames)
        {
            discard (s.pending.front());
            s.pending.pop_front();
        }

        const auto now = juce::Time::getMillisecondCounterHiRes() * 0.001;
        if (s.windowStart < 0.0)
            s.windowStart = now;
        if (now - s.windowStart < logIntervalSeconds || s.series.empty())
            return;

        logDiagnostic ("GPU timing: " + context + ", " + juce::String (s.resolvedFrames) + " frames over "
                       + juce::String (now - s.windowStart, 1) + " s (ms: median / p90 / max)");
        for (auto& x : s.series)
        {
            auto sorted = x.ms;
            std::sort (sorted.begin(), sorted.end());
            logDiagnostic ("GPU timing:   " + x.label + ": "
                           + juce::String (percentile (sorted, 0.5), 3) + " / "
                           + juce::String (percentile (sorted, 0.9), 3) + " / "
                           + juce::String (sorted.empty() ? 0.0f : sorted.back(), 3)
                           + "  (n=" + juce::String ((int) sorted.size()) + ")");
        }
        s.series.clear();
        s.resolvedFrames = 0;
        s.windowStart = now;
    }
}
