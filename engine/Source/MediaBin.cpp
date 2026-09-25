#include "MediaBin.h"
#include "Diagnostics.h"
#include "GLHelpers.h"

using namespace juce::gl;

MediaBin::~MediaBin()
{
    for (auto& p : playback)
        if (p.clip != nullptr)
            p.clip->cancelled = true;
    decoder.removeAllJobs (true, 3000);
}

bool MediaBin::isSupportedImage (const juce::File& file)
{
    return file.hasFileExtension ("png;jpg;jpeg");
}

bool MediaBin::isSupportedMedia (const juce::File& file)
{
    return isSupportedImage (file) || ClipDecoder::isSupportedVideo (file);
}

bool MediaBin::requestLoad (int slot, const juce::File& file)
{
    if (! juce::isPositiveAndBelow (slot, numSlots) || ! isSupportedMedia (file) || ! file.existsAsFile())
    {
        logDiagnostic ("MediaBin: cannot load '" + file.getFullPathName() + "' into slot " + juce::String (slot + 1));
        return false;
    }

    const bool isClip = ClipDecoder::isSupportedVideo (file);
    size_t budget = clipBudgetBytes;
    {
        const juce::ScopedLock sl (lock);
        auto& i = info[(size_t) slot];
        if (i.path == file.getFullPathName() && (i.loading || i.width > 0))
            return true; // already there (clients re-send their state every second)
        i = {};
        i.path = file.getFullPathName();
        i.name = file.getFileName();
        i.loading = true;
        i.clip = isClip;

        size_t used = 0;
        for (int s = 0; s < numSlots; ++s)
            if (s != slot)
                used += clipBytes[(size_t) s];
        budget = juce::jlimit ((size_t) 64u * 1024u * 1024u, clipBudgetBytes, totalClipBytes > used ? totalClipBytes - used : 0);
        clipBytes[(size_t) slot] = 0;
    }
    ++version;

    if (isClip)
    {
        auto clip = std::make_shared<Clip>();
        {
            const juce::ScopedLock sl (lock);
            pending.push_back ({ slot, {}, clip, file.getFullPathName() });
        }
        decoder.addJob ([this, slot, file, clip, budget] {
            ClipDecoder::decode (file, *clip, budget, 1280);
            const juce::ScopedLock sl (lock);
            auto& i = info[(size_t) slot];
            if (i.path == file.getFullPathName())
            {
                if (clip->failed)
                    i = {};
                else
                {
                    i.loading = false;
                    i.frames = clip->decoded.load();
                    clipBytes[(size_t) slot] = (size_t) clip->capacity * clip->frameBytes();
                }
            }
            ++version;
        });
        return true;
    }

    decoder.addJob ([this, slot, file] {
        auto image = juce::ImageFileFormat::loadFrom (file);
        if (image.isValid())
        {
            auto longest = juce::jmax (image.getWidth(), image.getHeight());
            if (longest > maxDimension)
            {
                auto scale = (float) maxDimension / (float) longest;
                image = image.rescaled (juce::roundToInt (image.getWidth() * scale), juce::roundToInt (image.getHeight() * scale),
                                        juce::Graphics::highResamplingQuality);
            }
        }
        else
            logDiagnostic ("MediaBin: could not decode " + file.getFullPathName());

        const juce::ScopedLock sl (lock);
        if (info[(size_t) slot].path != file.getFullPathName())
            return; // replaced or cleared while decoding
        if (image.isValid())
            pending.push_back ({ slot, image, {}, file.getFullPathName() });
        else
            info[(size_t) slot] = {};
        ++version;
    });
    return true;
}

void MediaBin::requestClear (int slot)
{
    if (! juce::isPositiveAndBelow (slot, numSlots))
        return;
    const juce::ScopedLock sl (lock);
    info[(size_t) slot] = {};
    clipBytes[(size_t) slot] = 0;
    pending.push_back ({ slot, {}, {}, {} });
    ++version;
}

std::array<MediaBin::Info, MediaBin::numSlots> MediaBin::getInfo() const
{
    const juce::ScopedLock sl (lock);
    return info;
}

void MediaBin::uploadPending()
{
    std::vector<Pending> work;
    {
        const juce::ScopedLock sl (lock);
        work.swap (pending);
    }

    for (auto& p : work)
    {
        auto& t = textures[(size_t) p.slot];
        if (t.id != 0)
            glDeleteTextures (1, &t.id);
        t = {};
        auto& pb = playback[(size_t) p.slot];
        if (pb.clip != nullptr)
            pb.clip->cancelled = true; // a replaced clip stops decoding
        pb = {};

        if (p.clip != nullptr)
        {
            pb.clip = p.clip;          // its texture is made once the first frame is decoded
            continue;
        }
        if (! p.image.isValid())
            continue;

        // Same upload as the preset image folders (upright, mipmapped), so a
        // loaded image looks exactly like the built-in forms to every shader.
        t = { uploadImageTexture (p.image), p.image.getWidth(), p.image.getHeight() };
        {
            const juce::ScopedLock sl (lock);
            auto& i = info[(size_t) p.slot];
            if (i.path == p.path)
            {
                i.width = t.width;
                i.height = t.height;
                i.loading = false;
            }
        }
        ++version;
        logDiagnostic ("MediaBin: slot " + juce::String (p.slot + 1) + " = " + p.path + " ("
                       + juce::String (t.width) + "x" + juce::String (t.height) + ")");
    }
}

void MediaBin::advance (double speed, double dt)
{
    const auto slot = juce::jlimit (0, numSlots - 1, active.load());
    auto& pb = playback[(size_t) slot];
    if (pb.clip == nullptr)
        return;
    auto& clip = *pb.clip;
    const auto count = clip.decoded.load();
    if (count <= 0)
        return;

    auto& t = textures[(size_t) slot];
    if (! pb.allocated)
    {
        glGenTextures (1, &t.id);
        glBindTexture (GL_TEXTURE_2D, t.id);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB8, clip.width, clip.height, 0, GL_BGR, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture (GL_TEXTURE_2D, 0);
        t.width = clip.width;
        t.height = clip.height;
        pb.allocated = true;
    }
    if (! pb.announced)
    {
        pb.announced = true;
        const juce::ScopedLock sl (lock);
        info[(size_t) slot].width = clip.width;
        info[(size_t) slot].height = clip.height;
        ++version;
    }

    // SHOTS jumps: to the start of a random eighth of what is decoded so far.
    if (const auto jumps = jumpRequests.load(); jumps != jumpsSeen)
    {
        jumpsSeen = jumps;
        pb.position = std::floor (random.nextFloat() * 8.0) / 8.0 * count;
    }

    // The scene clock plays the clip: Speed 0 = still, Reverse = backwards.
    if (playMode.load() == 1 && count > 1)
    {
        // Ping-pong: bounce off both ends (Reverse still flips the direction).
        const double last = count - 1;
        pb.position += speed * clip.fps * dt * pb.direction;
        for (int guard = 0; guard < 4 && (pb.position > last || pb.position < 0.0); ++guard)
        {
            if (pb.position > last) pb.position = 2.0 * last - pb.position;
            else                    pb.position = -pb.position;
            pb.direction = -pb.direction;
        }
        pb.position = juce::jlimit (0.0, last, pb.position);
    }
    else
    {
        pb.position += speed * clip.fps * dt;
        pb.position -= std::floor (pb.position / count) * count; // loop (both directions)
    }
    const auto frame = juce::jlimit (0, count - 1, (int) pb.position);

    if (frame != pb.uploaded)
    {
        pb.uploaded = frame;
        glBindTexture (GL_TEXTURE_2D, t.id);
        glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, clip.width, clip.height, GL_BGR, GL_UNSIGNED_BYTE, clip.frames[(size_t) frame].get());
        glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
        glBindTexture (GL_TEXTURE_2D, 0);
    }
}

SourceLibrary::Texture MediaBin::getActiveTexture() const
{
    return textures[(size_t) juce::jlimit (0, numSlots - 1, active.load())];
}

void MediaBin::release()
{
    for (auto& t : textures)
        if (t.id != 0)
            glDeleteTextures (1, &t.id);
    textures = {};
    for (auto& p : playback)
    {
        p.uploaded = -1;
        p.allocated = false;
    }
}
