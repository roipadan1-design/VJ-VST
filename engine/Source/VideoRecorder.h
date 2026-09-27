#pragma once

#include <JuceHeader.h>
#include "GLHelpers.h"

// Records the engine's live rendered output to a short (~20s) silent MP4 for
// Instagram Reels, muxed with the plug-in's captured master audio once both
// files exist. See docs/product-design/REELS-RECORDING-PLAN.md for the design
// (async PBO readback vs a stalling glReadPixels-per-frame, crop-not-re-render
// for the vertical option, two-file-then-mux instead of a new realtime audio
// channel).
//
// Threading: start()/stop()/setAudioPath() are called from the message thread
// (OSC handlers in MainComponent) and only set pending flags/atomics - all GL
// and file/process work happens inside pushFrame(), called once per render()
// frame on the GL thread, same as the rest of MainComponent's render path.
// Waiting for ffmpeg to exit and running the final mux happen on a background
// thread pool job so a slow encode can never stall a frame.
//
// Pixel path: glReadPixels into a 3-deep ring of GL_PIXEL_PACK_BUFFERs (returns
// at once - it writes into a buffer object, not client memory) in GL_BGRA (so
// it lines up with ffmpeg's "-pix_fmt bgra" raw input, no channel swap needed
// on read-back, unlike captureSnapshot's ARGB image path). Mapping and feeding
// the oldest buffer to ffmpeg's stdin happens 2 frames later, once the GPU is
// certainly done with it - adds a couple of frames of latency, irrelevant for
// a recording nobody watches live, and keeps the render thread non-blocking
// except for the (small, OS pipe buffer sized) write to ffmpeg itself.
class VideoRecorder
{
public:
    VideoRecorder();
    ~VideoRecorder();

    // Message thread. Ignored if a recording is already running.
    void start (double durationSeconds, bool vertical);

    // Message thread. The recording also stops on its own after
    // durationSeconds - this is for an explicit /record/stop.
    void stop();

    // Message thread: the plug-in's captured WAV, forwarded once it closes
    // the file on stop/timeout. Arriving late (or never, e.g. no plug-in was
    // actually on the master track) is handled - see finishRecording().
    void setAudioPath (const juce::String& path);

    // GL thread, once per render() frame, right after spoutSender.sendFrame()
    // with the same framebuffer/size. No-op unless a recording is starting,
    // running, or was just asked to stop.
    void pushFrame (unsigned int sourceFbo, int width, int height);

    bool isRecording() const noexcept { return state.load() == State::recording; }

    // Called from MainComponent::shutdown() (GL thread, context still alive),
    // same convention as PresetManager::releaseGLObjects() / MediaBin::release().
    // Aborts any in-flight recording (no mux - the engine is closing) and
    // frees the PBOs/crop target.
    void releaseGLObjects();

private:
    enum class State { idle, recording };

    void beginRecording (bool vertical, double durationSeconds, unsigned int sourceFbo, int width, int height);
    void captureFrame (unsigned int sourceFbo, int width, int height);
    void finishRecording();

    void allocatePbos (int width, int height);
    void releasePbos();
    void drainPbo (int index, int width, int height);

    bool beginFfmpeg (int width, int height);
    void closeFfmpegStdin();
    void writeToFfmpeg (const void* data, size_t numBytes);

    static juce::File recordingsFolder();
    juce::String getAudioPath() const;
    // Runs on finishPool's thread: waits for the video leg to exit, then muxes
    // it with whatever audio has arrived by then. `this` stays valid for the
    // whole call - the destructor blocks on finishPool before any member is
    // torn down (see ~VideoRecorder).
    void waitForProcessAndMux (void* processHandle, juce::File videoFile, juce::File finalFile);
    static bool muxWithAudio (const juce::File& video, const juce::File& audio, const juce::File& output);

    static double nowSeconds() { return juce::Time::getMillisecondCounterHiRes() * 0.001; }

    std::atomic<State> state { State::idle };
    std::atomic<bool> pendingStart { false }, pendingStop { false };
    std::atomic<double> pendingSeconds { 20.0 };
    std::atomic<bool> pendingVertical { false };

    double startTime = 0.0, durationSeconds = 20.0;
    bool verticalRequested = false;

    // PBO ring (GL thread only).
    static constexpr int numPbos = 3;
    unsigned int pbo[numPbos] { 0, 0, 0 };
    int pboWidth = 0, pboHeight = 0;
    int pboWriteIndex = 0, framesQueued = 0;
    bool pboRingPrimed = false;
    juce::MemoryBlock writeScratch; // reused row-flip buffer, avoids a per-frame heap allocation
    std::atomic<bool> writeFailed { false };

    // Portrait crop (GL thread only; unused/unallocated in landscape recordings).
    GLRenderTarget cropTarget;

    // ffmpeg (video leg) child process - a Win32 anonymous pipe feeds its
    // stdin (juce::ChildProcess can't write to a child's stdin), see
    // EngineLauncher.cpp for the same tier of Win32 process code, here for a
    // pipe instead of window-focus calls. Opaque void* so this header stays
    // free of <windows.h>; actual HANDLEs, cast back in the .cpp.
    void* ffmpegProcess = nullptr;
    void* ffmpegStdinWrite = nullptr;

    juce::File outputVideoFile, finalOutputFile;
    juce::String timestampStem;

    juce::SpinLock audioPathLock;
    juce::String pendingAudioPath;

    // Off-thread wait-for-ffmpeg-exit + mux, so a slow encode never touches
    // the render thread - same pattern as MediaBin's background decode pool.
    juce::ThreadPool finishPool { 1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoRecorder)
};
