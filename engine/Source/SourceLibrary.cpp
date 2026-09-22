#include "SourceLibrary.h"
#include "Diagnostics.h"
#include "GLHelpers.h"

using namespace juce::gl;

const std::vector<SourceLibrary::Texture>& SourceLibrary::imageFolder (const juce::String& relativeFolder)
{
    auto found = folders.find (relativeFolder);
    if (found != folders.end())
        return found->second;

    auto& list = folders[relativeFolder];

    if (relativeFolder.contains ("..") || juce::File::isAbsolutePath (relativeFolder))
    {
        logDiagnostic ("SourceLibrary: rejected unsafe folder path '" + relativeFolder + "'");
        return list;
    }

    auto folder = mediaRoot.getChildFile (relativeFolder);
    auto files = folder.findChildFiles (juce::File::findFiles, false, "*.png;*.jpg;*.jpeg");
    files.sort();

    for (auto& file : files)
    {
        if ((int) list.size() >= maxImagesPerFolder)
            break;

        auto image = juce::ImageFileFormat::loadFrom (file);
        if (! image.isValid())
            continue;

        auto longest = juce::jmax (image.getWidth(), image.getHeight());
        if (longest > maxImageDimension)
        {
            auto scale = (float) maxImageDimension / (float) longest;
            image = image.rescaled (juce::roundToInt (image.getWidth() * scale), juce::roundToInt (image.getHeight() * scale),
                                    juce::Graphics::highResamplingQuality);
        }

        list.push_back ({ uploadImageTexture (image), image.getWidth(), image.getHeight() });
    }

    logDiagnostic ("SourceLibrary: " + juce::String ((int) list.size()) + " image(s) from " + folder.getFullPathName());
    return list;
}

SourceLibrary::Texture SourceLibrary::text (const juce::String& word, const juce::String& fontName)
{
    auto key = fontName + "|" + word;
    auto found = texts.find (key);
    if (found != texts.end())
        return found->second;

    // Render large and centred with generous padding, so extrusion/smear
    // shaders have room around the glyphs before hitting the texture edge.
    const int width = 2048, height = 768;
    juce::Image image (juce::Image::ARGB, width, height, true);
    {
        juce::Graphics g (image);
        auto font = juce::Font (juce::FontOptions (fontName.isNotEmpty() ? fontName : juce::String ("Arial Black"), 420.0f, juce::Font::bold));
        auto textWidth = juce::GlyphArrangement::getStringWidth (font, word);
        if (textWidth > width * 0.82f)
            font = font.withHeight (font.getHeight() * (width * 0.82f) / textWidth);

        g.setFont (font);
        g.setColour (juce::Colours::white);
        g.drawText (word, image.getBounds(), juce::Justification::centred, false);
    }

    Texture t { uploadImageTexture (image), width, height };
    texts[key] = t;
    return t;
}

void SourceLibrary::release()
{
    for (auto& [name, list] : folders)
        for (auto& t : list)
            glDeleteTextures (1, &t.id);
    for (auto& [key, t] : texts)
        glDeleteTextures (1, &t.id);
    folders.clear();
    texts.clear();
}
