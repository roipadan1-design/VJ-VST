#include "MediaBin.h"
#include "Diagnostics.h"
#include "GLHelpers.h"

using namespace juce::gl;

MediaBin::~MediaBin()
{
    decoder.removeAllJobs (true, 2000);
}

bool MediaBin::isSupportedImage (const juce::File& file)
{
    return file.hasFileExtension ("png;jpg;jpeg");
}

bool MediaBin::requestLoad (int slot, const juce::File& file)
{
    if (! juce::isPositiveAndBelow (slot, numSlots) || ! isSupportedImage (file) || ! file.existsAsFile())
    {
        logDiagnostic ("MediaBin: cannot load '" + file.getFullPathName() + "' into slot " + juce::String (slot + 1));
        return false;
    }

    {
        const juce::ScopedLock sl (lock);
        auto& i = info[(size_t) slot];
        if (i.path == file.getFullPathName() && (i.loading || i.width > 0))
            return true; // already there (clients re-send their state every second)
        i.path = file.getFullPathName();
        i.name = file.getFileName();
        i.loading = true;
    }
    ++version;

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
            pending.push_back ({ slot, image, file.getFullPathName() });
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
    pending.push_back ({ slot, {}, {} });
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
}
