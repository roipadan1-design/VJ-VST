#pragma once

#include <JuceHeader.h>
#include "ClipDecoder.h"
#include "SourceLibrary.h"

// The performer's own material: 8 slots, each a still (PNG / JPEG) or a short
// video clip (MP4 / MOV / ...), loaded at run time - dropped on the engine
// window or sent by the plug-in's LOAD button (OSC /v2/media/load <slot> <path>).
//
// Stills are decoded (<= 2048 px) on a background thread and uploaded at the
// top of the next frame. Clips are decoded completely into memory (see
// ClipDecoder) and play on the scene clock: Speed 0 freezes them, Reverse
// plays them backwards, Push speeds them up; jumpActive() cuts to another
// eighth of the clip (fired by SHOTS). A clip plays while it is still loading.
//
// Scenes see the ACTIVE slot:
//   - through a "media" source   ({"type": "media", "fallback": "Images/Forms"})
//   - and, while USE MEDIA is on, in place of every "images" source.
class MediaBin
{
public:
    static constexpr int numSlots = 8;
    static constexpr int maxDimension = 2048;
    static constexpr size_t clipBudgetBytes = 600u * 1024u * 1024u;   // per clip
    static constexpr size_t totalClipBytes = 1536u * 1024u * 1024u;   // all slots (RAM is shared with Live)

    struct Info
    {
        juce::String name, path;
        int width = 0, height = 0;
        bool loading = false;
        bool clip = false;
        int frames = 0;          // clips: frames decoded so far
    };

    MediaBin() = default;
    ~MediaBin();

    // --- any thread
    // Loads a still or a clip into a slot (re-sending the same path is a
    // no-op, so clients may repeat it). Returns false for unsupported files.
    bool requestLoad (int slot, const juce::File& file);
    void requestClear (int slot);
    void select (int slot)             { if (juce::isPositiveAndBelow (slot, numSlots)) active = slot; }
    int getActive() const noexcept     { return active.load(); }
    void setUseMedia (bool on) noexcept { useMedia = on; }
    bool getUseMedia() const noexcept  { return useMedia.load(); }
    void jumpActive() noexcept         { ++jumpRequests; }
    // Clip playback: 0 = loop, 1 = ping-pong (forward to the end, back to the start).
    void setPlayMode (int mode) noexcept { playMode = juce::jlimit (0, 1, mode); }
    int getPlayMode() const noexcept   { return playMode.load(); }
    // Tempo sync: 0 = free (the clip's own frame rate x speed), else the clip
    // is stretched over this many beats and phase-locked to the scene's beat
    // clock (so it restarts on the bar, still frozen by Speed 0).
    void setSyncBeats (int beats) noexcept { syncBeats = juce::jlimit (0, 64, beats); }
    std::array<Info, numSlots> getInfo() const;
    int getVersion() const noexcept    { return version.load(); } // bumps on every change

    static bool isSupportedImage (const juce::File& file);
    static bool isSupportedMedia (const juce::File& file);

    // --- GL thread
    void uploadPending();
    // Advances the active clip by the scene clock and uploads its frame.
    // sceneBeat: host beats scaled by speed (MotionFrame::sceneBeat).
    void advance (double speed, double dt, double sceneBeat);
    SourceLibrary::Texture getActiveTexture() const; // id 0 = nothing loaded
    void release();

private:
    struct Pending
    {
        int slot = 0;
        juce::Image image;             // a still, or
        std::shared_ptr<Clip> clip;    // a clip (frames arrive over time); neither = clear
        juce::String path;
    };

    struct ClipPlayback
    {
        std::shared_ptr<Clip> clip;
        double position = 0.0;         // in frames
        double direction = 1.0;        // ping-pong: +1 forward, -1 back
        double syncOffset = 0.0;       // tempo sync: phase added by SHOTS jumps (0-1)
        int uploaded = -1;
        bool allocated = false, announced = false;
    };

    juce::ThreadPool decoder { 1 };
    mutable juce::CriticalSection lock;
    std::vector<Pending> pending;          // guarded by lock
    std::array<Info, numSlots> info;       // guarded by lock
    std::array<size_t, numSlots> clipBytes {}; // guarded by lock
    std::array<SourceLibrary::Texture, numSlots> textures {}; // GL thread only
    std::array<ClipPlayback, numSlots> playback;              // GL thread only
    std::atomic<int> active { 0 }, version { 0 }, jumpRequests { 0 }, playMode { 0 }, syncBeats { 0 };
    std::atomic<bool> useMedia { false };
    int jumpsSeen = 0;
    juce::Random random;
};
