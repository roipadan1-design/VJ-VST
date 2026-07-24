#pragma once

#include <JuceHeader.h>

// Decodes a video file (MP4/H.264 and anything else Windows Media Foundation
// already ships codecs for) on a background thread, using Media Foundation's
// synchronous Source Reader, and uploads decoded frames to a GL texture for
// the render thread to sample as the ISF "inputImage" input. Loops forever -
// this is a VJ source loop, not a general-purpose media player.
//
// Deliberately native Media Foundation rather than a third-party decoder:
// Windows already ships the codecs (H.264/HEVC/etc via MF Transforms), so
// this needs no extra dependency - same reasoning as writing our own ISF
// host instead of pulling in VVISF-GL, and using the official prebuilt
// SpoutLibrary binary instead of building Spout's full source tree.
//
// Known scope limits (documented, not hidden):
//  - no audio playback from the video file - visual-only use case.
//  - no seek/scrub controls; always loops from the start.
//  - decode pacing is driven by the decode thread's own clock (sleeping to
//    the source's frame duration), not genlocked to the render thread - the
//    render thread always just uploads whatever the latest decoded frame is,
//    so under heavy GPU load frames may repeat rather than blocking/tearing.
//  - only one video loaded at a time (single global "video in" source, not
//    a per-clip media bin).
class VideoPlayer
{
public:
    VideoPlayer();
    ~VideoPlayer();

    // Starts decoding the given file on a background thread. Safe to call
    // again to switch files - any previous decode thread is stopped first.
    // Must be called from the message thread (drag-and-drop callback).
    void load (const juce::File& file);

    void close();

    // Uploads the latest decoded frame (if a new one is ready) to the GL
    // texture. Call once per render frame, on the GL thread. Cheap no-op if
    // no video is loaded or no new frame has arrived since the last call.
    void updateGLTexture();

    unsigned int getTextureID() const noexcept { return textureId; }
    bool isLoaded() const noexcept { return hasDecodedFrame.load(); }
    int getWidth() const noexcept { return textureWidth; }
    int getHeight() const noexcept { return textureHeight; }

private:
    class DecodeThread;
    std::unique_ptr<DecodeThread> decodeThread;

    unsigned int textureId = 0;
    int textureWidth = 0, textureHeight = 0;

    std::atomic<bool> hasDecodedFrame { false };
    std::atomic<bool> newFrameReady { false };

    juce::CriticalSection frameLock;
    juce::MemoryBlock latestFrameRGBA; // top-down rows, 4 bytes/pixel, tightly packed
    int latestFrameWidth = 0, latestFrameHeight = 0;

    // Called from the decode thread to publish a freshly decoded frame.
    void pushFrame (const void* rgbaTopDown, int width, int height);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoPlayer)
};
