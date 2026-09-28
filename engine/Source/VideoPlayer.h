#pragma once

#include <JuceHeader.h>

// Decodes a video file (MP4/H.264 and anything else Windows Media Foundation
// already ships codecs for) OR a live capture device (webcam) on a
// background thread, using Media Foundation's synchronous Source Reader,
// and uploads decoded frames to a GL texture for the render thread to
// sample as the ISF "inputImage" input. File playback loops forever - this
// is a VJ source, not a general-purpose media player.
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
//  - a camera frame is handed on the moment it arrives (never paced: any
//    wait lets frames queue up = latency). File decode pacing is driven by
//    the decode thread's own clock (sleeping to
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

    // Starts decoding a live capture device (webcam) instead of a file -
    // deviceIndex follows Media Foundation's video-capture-device
    // enumeration order (0 = first/default camera). Same threading rules
    // as load(). Logs and leaves the previous source (if any) stopped, with
    // nothing loaded, if the device can't be opened.
    void openCamera (int deviceIndex = 0);

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

    // Frames are B,G,R,X, bottom row first (GL order), tightly packed. Three
    // buffers rotate by swapping, never copying: the decode thread's own,
    // readyFrame (newest complete frame) and uploadFrame (GL thread only).
    juce::CriticalSection frameLock;
    std::vector<uint8_t> readyFrame, uploadFrame; // readyFrame guarded by frameLock
    int readyWidth = 0, readyHeight = 0;          // guarded by frameLock

    // Called from the decode thread to publish a freshly decoded frame; the
    // caller gets an older buffer back in `frame` to reuse.
    void pushFrame (std::vector<uint8_t>& frame, int width, int height);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoPlayer)
};
