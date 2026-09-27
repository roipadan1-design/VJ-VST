#include "VideoRecorder.h"
#include "Diagnostics.h"

// Win32 process + pipe plumbing lives in this .cpp, after the JUCE include -
// SpoutSender.cpp already mixes JuceHeader.h with a windows.h-including third
// party header in one translation unit without trouble, so there is no need
// for EngineLauncher.cpp's separate-translation-unit trick here (that one is
// about keeping the plug-in's Win32 focus calls out of JUCE's own Windows
// includes, not a hard requirement).
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

using namespace juce::gl;

namespace
{
    // ffmpeg.exe is expected on PATH - confirmed installed on this machine
    // (the same laptop used for both dev and the show, per REELS-RECORDING-
    // PLAN.md). No bundled copy for v1.
    constexpr double targetFps = 60.0; // matches updateAdaptiveQuality's own ~60fps assumption
}

VideoRecorder::VideoRecorder() = default;

VideoRecorder::~VideoRecorder()
{
    // Let an in-flight mux finish rather than leave a dangling `this` in its
    // job (it reads audioPathLock/pendingAudioPath, both still alive here -
    // this runs before any member is torn down).
    finishPool.removeAllJobs (true, 5000);

    if (ffmpegStdinWrite != nullptr) CloseHandle ((HANDLE) ffmpegStdinWrite);
    if (ffmpegProcess != nullptr)    CloseHandle ((HANDLE) ffmpegProcess);
}

juce::File VideoRecorder::recordingsFolder()
{
    // Same Documents\VJ VST convention as the plug-in's Looks (LookLibrary.cpp).
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("VJ VST").getChildFile ("Recordings");
}

void VideoRecorder::start (double durationSecondsIn, bool vertical)
{
    pendingSeconds = juce::jlimit (1.0, 60.0, durationSecondsIn);
    pendingVertical = vertical;
    pendingStart = true;
}

void VideoRecorder::stop()
{
    pendingStop = true;
}

void VideoRecorder::setAudioPath (const juce::String& path)
{
    const juce::SpinLock::ScopedLockType sl (audioPathLock);
    pendingAudioPath = path;
}

juce::String VideoRecorder::getAudioPath() const
{
    const juce::SpinLock::ScopedLockType sl (audioPathLock);
    return pendingAudioPath;
}

void VideoRecorder::pushFrame (unsigned int sourceFbo, int width, int height)
{
    if (pendingStop.exchange (false) && state.load() == State::recording)
        finishRecording();

    if (pendingStart.exchange (false))
    {
        if (state.load() == State::recording)
            logDiagnostic ("VideoRecorder: /record/start ignored - a recording is already running");
        else
            beginRecording (pendingVertical.load(), pendingSeconds.load(), sourceFbo, width, height);
    }

    if (state.load() != State::recording)
        return;

    captureFrame (sourceFbo, width, height);

    if (nowSeconds() - startTime >= durationSeconds)
        finishRecording();
}

void VideoRecorder::beginRecording (bool vertical, double durationSecondsIn, unsigned int sourceFbo, int width, int height)
{
    verticalRequested = vertical;
    durationSeconds = durationSecondsIn;

    int outW = width, outH = height;
    if (vertical)
    {
        outH = height - (height % 2);
        outW = (int) (outH * 9.0 / 16.0);
        outW -= outW % 2;
        outW = juce::jlimit (2, width, outW);
    }
    else
    {
        outW = width - (width % 2);
        outH = height - (height % 2);
    }

    if (outW <= 0 || outH <= 0)
    {
        logDiagnostic ("VideoRecorder: window too small to record (" + juce::String (width) + "x" + juce::String (height) + ")");
        return;
    }

    timestampStem = juce::Time::getCurrentTime().formatted ("%Y%m%d-%H%M%S");
    outputVideoFile = recordingsFolder().getChildFile (timestampStem + "_video.mp4");
    finalOutputFile = recordingsFolder().getChildFile (timestampStem + ".mp4");
    outputVideoFile.getParentDirectory().createDirectory();

    setAudioPath ({}); // drop anything left over from a previous recording

    if (! beginFfmpeg (outW, outH))
        return;

    allocatePbos (outW, outH);
    (void) sourceFbo; // the crop target (if any) is (re)sized inside captureFrame, once per size change

    startTime = nowSeconds();
    state = State::recording;
    logDiagnostic ("VideoRecorder: recording " + juce::String (durationSeconds, 1) + "s "
                   + (vertical ? "(vertical " + juce::String (outW) + "x" + juce::String (outH) + ")" : "(landscape)")
                   + " to " + finalOutputFile.getFullPathName());
}

void VideoRecorder::captureFrame (unsigned int sourceFbo, int width, int height)
{
    // The output size is whatever it was when the recording started (pboWidth/
    // Height) - if the window is resized mid-recording, `height` (this frame's
    // live size) is deliberately not used for that, only `width` for centering
    // the crop; the ring/ffmpeg process would have to be recreated for a real
    // size mid-flight, out of scope for v1.
    juce::ignoreUnused (height);
    unsigned int readFbo = sourceFbo;
    int outW = pboWidth, outH = pboHeight;

    if (verticalRequested)
    {
        // Center crop, native pixels: keep the full render height and take a
        // 9:16-wide vertical slice out of the middle, rather than re-rendering
        // the scene at a different resolution (see REELS-RECORDING-PLAN.md #2
        // for why a second presetManager.render() call per frame is unsafe).
        cropTarget.ensure (outW, outH, GL_RGBA8);
        const auto srcX0 = (width - outW) / 2;

        glBindFramebuffer (GL_READ_FRAMEBUFFER, sourceFbo);
        glBindFramebuffer (GL_DRAW_FRAMEBUFFER, cropTarget.fbo);
        glBlitFramebuffer (srcX0, 0, srcX0 + outW, outH, 0, 0, outW, outH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer (GL_FRAMEBUFFER, 0);

        readFbo = cropTarget.fbo;
    }

    // Kick off this frame's readback into the ring's next slot - returns at
    // once, it is writing into a buffer object, not client memory.
    glBindFramebuffer (GL_READ_FRAMEBUFFER, readFbo);
    glBindBuffer (GL_PIXEL_PACK_BUFFER, pbo[pboWriteIndex]);
    glReadPixels (0, 0, outW, outH, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
    glBindBuffer (GL_PIXEL_PACK_BUFFER, 0);
    glBindFramebuffer (GL_FRAMEBUFFER, 0);

    pboWriteIndex = (pboWriteIndex + 1) % numPbos;

    if (! pboRingPrimed)
    {
        if (++framesQueued >= numPbos)
            pboRingPrimed = true;
        return;
    }

    // The ring has wrapped: the slot about to be overwritten (pboWriteIndex,
    // now that we've advanced past it) was issued (numPbos - 1) frames ago -
    // the GPU is certainly done with it - drain it to the encoder.
    drainPbo (pboWriteIndex, outW, outH);
}

void VideoRecorder::finishRecording()
{
    // Flush whatever is still sitting in the ring. glMapBuffer can briefly
    // stall here if the very last frame's readback hasn't finished yet - a
    // one-off cost at the end of a recording, not a per-frame one.
    const int remaining = pboRingPrimed ? (numPbos - 1) : framesQueued;
    auto drainIndex = pboWriteIndex;
    for (int i = 0; i < remaining; ++i)
    {
        drainPbo (drainIndex, pboWidth, pboHeight);
        drainIndex = (drainIndex + 1) % numPbos;
    }

    closeFfmpegStdin(); // EOF: ffmpeg finishes encoding what it has and exits on its own
    state = State::idle;

    auto* process = ffmpegProcess;
    ffmpegProcess = nullptr; // ownership passed to the background job (closes the handle)
    auto videoFile = outputVideoFile;
    auto finalFile = finalOutputFile;

    if (process == nullptr)
        return;

    finishPool.addJob ([this, process, videoFile, finalFile] {
        waitForProcessAndMux (process, videoFile, finalFile);
    });
}

void VideoRecorder::waitForProcessAndMux (void* processHandle, juce::File videoFile, juce::File finalFile)
{
    auto* handle = (HANDLE) processHandle;
    WaitForSingleObject (handle, 15000); // a <=20s clip encodes fast at "veryfast"; don't hang forever if ffmpeg wedges
    CloseHandle (handle);

    if (! videoFile.existsAsFile() || videoFile.getSize() <= 0)
    {
        logDiagnostic ("VideoRecorder: ffmpeg produced no video - recording aborted");
        return;
    }

    // Give the plug-in's /record/audiopath a short window to arrive - it
    // sends its own WAV path independently on stop/timeout. Falls back to
    // keeping the silent video if it never shows (e.g. the recording instance
    // wasn't actually the one on the master track).
    juce::String audioPath;
    const auto deadline = nowSeconds() + 3.0;
    while (nowSeconds() < deadline)
    {
        audioPath = getAudioPath();
        if (audioPath.isNotEmpty())
            break;
        juce::Thread::sleep (100);
    }

    if (audioPath.isEmpty())
    {
        videoFile.moveFileTo (finalFile);
        logDiagnostic ("VideoRecorder: no audio arrived in time - saved silent video to " + finalFile.getFullPathName());
        return;
    }

    juce::File audioFile (audioPath);
    if (muxWithAudio (videoFile, audioFile, finalFile))
    {
        videoFile.deleteFile();
        audioFile.deleteFile();
        logDiagnostic ("VideoRecorder: saved " + finalFile.getFullPathName());
    }
    else
    {
        videoFile.moveFileTo (finalFile);
        logDiagnostic ("VideoRecorder: mux with audio failed - saved silent video to " + finalFile.getFullPathName());
    }
}

bool VideoRecorder::muxWithAudio (const juce::File& video, const juce::File& audio, const juce::File& output)
{
    // Stream copy only (no re-encode) - instant, and the video leg is already
    // the exact bytes we want. juce::ChildProcess is fine here: unlike the
    // video leg, this call needs no stdin, only to wait for exit.
    juce::StringArray cmd { "ffmpeg", "-y", "-i", video.getFullPathName(), "-i", audio.getFullPathName(),
                             "-c", "copy", "-shortest", output.getFullPathName() };
    juce::ChildProcess mux;
    if (! mux.start (cmd))
        return false;
    return mux.waitForProcessToFinish (10000) && output.existsAsFile() && output.getSize() > 0;
}

void VideoRecorder::releaseGLObjects()
{
    if (state.load() == State::recording)
    {
        closeFfmpegStdin();
        state = State::idle;
        logDiagnostic ("VideoRecorder: engine shutting down mid-recording - stopped ffmpeg, no mux");
    }
    releasePbos();
    cropTarget.release();
}

// ---- PBO ring ---------------------------------------------------------

void VideoRecorder::allocatePbos (int width, int height)
{
    releasePbos();
    pboWidth = width;
    pboHeight = height;

    glGenBuffers (numPbos, pbo);
    const auto bytes = (GLsizeiptr) ((size_t) width * (size_t) height * 4);
    for (auto id : pbo)
    {
        glBindBuffer (GL_PIXEL_PACK_BUFFER, id);
        glBufferData (GL_PIXEL_PACK_BUFFER, bytes, nullptr, GL_STREAM_READ);
    }
    glBindBuffer (GL_PIXEL_PACK_BUFFER, 0);

    pboWriteIndex = 0;
    framesQueued = 0;
    pboRingPrimed = false;
}

void VideoRecorder::releasePbos()
{
    if (pbo[0] != 0)
        glDeleteBuffers (numPbos, pbo);
    for (auto& id : pbo)
        id = 0;
    pboWidth = pboHeight = 0;
}

void VideoRecorder::drainPbo (int index, int width, int height)
{
    glBindBuffer (GL_PIXEL_PACK_BUFFER, pbo[index]);
    auto* mapped = (const unsigned char*) glMapBuffer (GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

    if (mapped != nullptr)
    {
        // glReadPixels gives rows bottom-to-top (GL's convention); ffmpeg's
        // rawvideo input expects top-to-bottom, so each row is copied in
        // reverse order - same reasoning as captureSnapshot's flip, but no
        // channel swap needed here since the PBO was already filled in BGRA
        // (ffmpeg's "-pix_fmt bgra"), not JUCE's ARGB.
        const size_t rowBytes = (size_t) width * 4;
        writeScratch.ensureSize (rowBytes * (size_t) height);
        auto* dst = (unsigned char*) writeScratch.getData();
        for (int y = 0; y < height; ++y)
            std::memcpy (dst + (size_t) y * rowBytes, mapped + (size_t) (height - 1 - y) * rowBytes, rowBytes);

        glUnmapBuffer (GL_PIXEL_PACK_BUFFER);
        writeToFfmpeg (dst, rowBytes * (size_t) height);
    }

    glBindBuffer (GL_PIXEL_PACK_BUFFER, 0);
}

// ---- ffmpeg process/pipe (Win32) --------------------------------------

bool VideoRecorder::beginFfmpeg (int width, int height)
{
    HANDLE readHandle = nullptr, writeHandle = nullptr;
    SECURITY_ATTRIBUTES sa {};
    sa.nLength = sizeof (sa);
    sa.bInheritHandle = TRUE;

    if (! CreatePipe (&readHandle, &writeHandle, &sa, 0))
    {
        logDiagnostic ("VideoRecorder: CreatePipe failed");
        return false;
    }

    // Only the read end is inherited by ffmpeg (as its stdin) - if our own
    // copy of the write end were inheritable too, ffmpeg would hold it open
    // and never see EOF when we close ours in closeFfmpegStdin().
    SetHandleInformation (writeHandle, HANDLE_FLAG_INHERIT, 0);

    auto command = juce::String ("ffmpeg -f rawvideo -pix_fmt bgra -s ") + juce::String (width) + "x" + juce::String (height)
                 + " -r " + juce::String (targetFps, 0) + " -i - -c:v libx264 -preset veryfast -crf 18 -pix_fmt yuv420p -y \""
                 + outputVideoFile.getFullPathName() + "\"";

    STARTUPINFOW si {};
    si.cb = sizeof (si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = readHandle;
    si.hStdOutput = GetStdHandle (STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle (STD_ERROR_HANDLE);

    PROCESS_INFORMATION pi {};
    auto* widePtr = command.toWideCharPointer();
    std::vector<wchar_t> cmdline (widePtr, widePtr + command.length());
    cmdline.push_back (L'\0');

    const bool started = CreateProcessW (nullptr, cmdline.data(), nullptr, nullptr, TRUE,
                                          CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi) != 0;
    CloseHandle (readHandle); // our copy of the child's stdin read end - ffmpeg has its own now

    if (! started)
    {
        logDiagnostic ("VideoRecorder: failed to start ffmpeg.exe - is it on PATH?");
        CloseHandle (writeHandle);
        return false;
    }

    CloseHandle (pi.hThread);
    ffmpegProcess = (void*) pi.hProcess;
    ffmpegStdinWrite = (void*) writeHandle;
    writeFailed = false;
    return true;
}

void VideoRecorder::closeFfmpegStdin()
{
    if (ffmpegStdinWrite != nullptr)
    {
        CloseHandle ((HANDLE) ffmpegStdinWrite);
        ffmpegStdinWrite = nullptr;
    }
}

void VideoRecorder::writeToFfmpeg (const void* data, size_t numBytes)
{
    if (ffmpegStdinWrite == nullptr)
        return;

    DWORD written = 0;
    if (! WriteFile ((HANDLE) ffmpegStdinWrite, data, (DWORD) numBytes, &written, nullptr) || written != numBytes)
    {
        if (! writeFailed.exchange (true))
            logDiagnostic ("VideoRecorder: write to ffmpeg's stdin failed - it may have exited early");
    }
}
