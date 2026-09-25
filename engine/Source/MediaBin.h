#pragma once

#include <JuceHeader.h>
#include "SourceLibrary.h"

// The performer's own material: 8 slots of stills (PNG / JPEG) loaded at run
// time - dropped on the engine window or sent by the plug-in's LOAD button
// (OSC /v2/media/load <slot> <path>). Clips are planned as a second slot kind.
//
// Decoding (and downscaling to <= 2048 px on the long edge) runs on a
// background thread; the GL thread uploads finished images at the top of the
// next frame. Scenes see the ACTIVE slot:
//   - through a "media" source   ({"type": "media", "fallback": "Images/Forms"})
//   - and, while USE MEDIA is on, in place of every "images" source (the
//     abstract forms of Dot Relief, One Bit, Emergence...).
class MediaBin
{
public:
    static constexpr int numSlots = 8;
    static constexpr int maxDimension = 2048;

    struct Info
    {
        juce::String name, path;
        int width = 0, height = 0;
        bool loading = false;
    };

    MediaBin() = default;
    ~MediaBin();

    // --- any thread
    // Loads a PNG/JPEG into a slot (re-sending the same path is a no-op, so
    // clients may repeat it). Returns false for unsupported files.
    bool requestLoad (int slot, const juce::File& file);
    void requestClear (int slot);
    void select (int slot)             { if (juce::isPositiveAndBelow (slot, numSlots)) active = slot; }
    int getActive() const noexcept     { return active.load(); }
    void setUseMedia (bool on) noexcept { useMedia = on; }
    bool getUseMedia() const noexcept  { return useMedia.load(); }
    std::array<Info, numSlots> getInfo() const;
    int getVersion() const noexcept    { return version.load(); } // bumps on every change

    static bool isSupportedImage (const juce::File& file);

    // --- GL thread
    void uploadPending();
    SourceLibrary::Texture getActiveTexture() const; // id 0 = nothing loaded
    void release();

private:
    struct Pending
    {
        int slot = 0;
        juce::Image image;   // invalid = clear the slot
        juce::String path;
    };

    juce::ThreadPool decoder { 1 };
    mutable juce::CriticalSection lock;
    std::vector<Pending> pending;          // guarded by lock
    std::array<Info, numSlots> info;       // guarded by lock
    std::array<SourceLibrary::Texture, numSlots> textures {}; // GL thread only
    std::atomic<int> active { 0 }, version { 0 };
    std::atomic<bool> useMedia { false };
};
