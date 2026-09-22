#pragma once

#include <JuceHeader.h>
#include <map>

// GPU textures for schema-2 "sources" - the raw material a treatment works on
// (docs/REFERENCE-ZWOBOT-V3-TRAILER.md: stills, masks and type fed through
// one strong treatment per scene). Owned and used on the GL thread only.
//
//  - Image folders under the engine's Media directory (png / jpg), loaded
//    once, sorted by name, capped in count and size.
//  - Text: a word rendered white-on-transparent in a chosen font, cached per
//    (word, font), so shaders can extrude, smear or colour it.
class SourceLibrary
{
public:
    struct Texture
    {
        unsigned int id = 0;
        int width = 1, height = 1;
    };

    void setMediaRoot (const juce::File& root) { mediaRoot = root; }

    // Empty when the folder is missing or has no readable images.
    const std::vector<Texture>& imageFolder (const juce::String& relativeFolder);

    Texture text (const juce::String& word, const juce::String& fontName);

    void release();

    static constexpr int maxImagesPerFolder = 64;
    static constexpr int maxImageDimension = 2048;

private:
    juce::File mediaRoot;
    std::map<juce::String, std::vector<Texture>> folders;
    std::map<juce::String, Texture> texts;
};
