#include "VideoPlayer.h"

using namespace juce::gl;

#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <propvarutil.h>
#include <wrl/client.h>
#include <cstring>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")

using Microsoft::WRL::ComPtr;

namespace
{
    void ensureMediaFoundationStarted()
    {
        static const bool started = (SUCCEEDED (MFStartup (MF_VERSION)));
        juce::ignoreUnused (started);
    }

    constexpr DWORD videoStreamIndex = (DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM;
}

class VideoPlayer::DecodeThread : public juce::Thread
{
public:
    DecodeThread (VideoPlayer& ownerIn, juce::File fileIn)
        : juce::Thread ("VideoDecode"), owner (ownerIn), file (std::move (fileIn))
    {
    }

    ~DecodeThread() override
    {
        stopThread (2000);
    }

    void run() override
    {
        auto hrCo = CoInitializeEx (nullptr, COINIT_MULTITHREADED);
        bool comInitialisedHere = SUCCEEDED (hrCo);

        ensureMediaFoundationStarted();

        ComPtr<IMFSourceReader> reader;
        int width = 0, height = 0;

        if (! openReader (reader) || ! queryFrameSize (reader.Get(), width, height) || width <= 0 || height <= 0)
        {
            DBG ("VideoPlayer: failed to open/read " << file.getFullPathName());
            if (comInitialisedHere) CoUninitialize();
            return;
        }

        std::vector<uint8_t> frameBuffer ((size_t) width * (size_t) height * 4);
        double playbackStartMs = juce::Time::getMillisecondCounterHiRes();

        while (! threadShouldExit())
        {
            DWORD streamIndex = 0, flags = 0;
            LONGLONG timestamp = 0;
            ComPtr<IMFSample> sample;

            auto hr = reader->ReadSample (videoStreamIndex, 0,
                                           &streamIndex, &flags, &timestamp, sample.GetAddressOf());

            if (FAILED (hr))
                break;

            if ((flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0)
            {
                seekToStart (reader.Get());
                playbackStartMs = juce::Time::getMillisecondCounterHiRes();
                continue;
            }

            if (sample == nullptr)
                continue;

            copySampleToFrameBuffer (sample.Get(), width, height, frameBuffer);
            owner.pushFrame (frameBuffer.data(), width, height);

            // Pace playback to the source's own timestamps (100ns units) -
            // without this, ReadSample() would decode as fast as the CPU
            // allows, playing the file back far faster than real time.
            auto targetMs = playbackStartMs + (double) timestamp / 10000.0;
            auto waitMs = targetMs - juce::Time::getMillisecondCounterHiRes();

            if (waitMs > 0.5)
                wait (juce::jlimit (1, 1000, (int) waitMs));
        }

        if (comInitialisedHere)
            CoUninitialize();
    }

private:
    bool openReader (ComPtr<IMFSourceReader>& reader)
    {
        ComPtr<IMFAttributes> attributes;
        MFCreateAttributes (attributes.GetAddressOf(), 1);

        // Without this, SetCurrentMediaType() below fails to convert most
        // compressed sources (e.g. H.264, which decodes natively to NV12)
        // to RGB32 - this attribute is what allows the source reader to
        // insert Media Foundation's video processor to do that conversion.
        attributes->SetUINT32 (MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

        auto hr = MFCreateSourceReaderFromURL (file.getFullPathName().toWideCharPointer(),
                                                attributes.Get(), reader.GetAddressOf());
        if (FAILED (hr) || reader == nullptr)
            return false;

        reader->SetStreamSelection ((DWORD) MF_SOURCE_READER_ALL_STREAMS, FALSE);
        reader->SetStreamSelection (videoStreamIndex, TRUE);

        ComPtr<IMFMediaType> outputType;
        MFCreateMediaType (outputType.GetAddressOf());
        outputType->SetGUID (MF_MT_MAJOR_TYPE, MFMediaType_Video);
        outputType->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_RGB32);

        hr = reader->SetCurrentMediaType (videoStreamIndex, nullptr, outputType.Get());
        return SUCCEEDED (hr);
    }

    static bool queryFrameSize (IMFSourceReader* reader, int& width, int& height)
    {
        ComPtr<IMFMediaType> currentType;
        if (FAILED (reader->GetCurrentMediaType (videoStreamIndex, currentType.GetAddressOf())))
            return false;

        UINT32 w = 0, h = 0;
        if (FAILED (MFGetAttributeSize (currentType.Get(), MF_MT_FRAME_SIZE, &w, &h)))
            return false;

        width = (int) w;
        height = (int) h;
        return true;
    }

    static void seekToStart (IMFSourceReader* reader)
    {
        PROPVARIANT var;
        PropVariantInit (&var);
        var.vt = VT_I8;
        var.hVal.QuadPart = 0;
        reader->SetCurrentPosition (GUID_NULL, var);
        PropVariantClear (&var);
    }

    // Copies the sample into frameBuffer as straight top-down RGBA rows
    // (row 0 = the image's visual top row) for VideoPlayer::updateGLTexture()
    // to upload as-is. Empirically verified with a 4-quadrant test clip
    // rendered through Spout into Resolume: a row-reversed copy here (the
    // "should be bottom-up for GL" assumption one might reach for) produced
    // a vertically-flipped image, so row order is NOT reversed - Lock2D's
    // scanline0/pitch already lines up directly with what glTexImage2D wants.
    static void copySampleToFrameBuffer (IMFSample* sample, int width, int height, std::vector<uint8_t>& frameBuffer)
    {
        ComPtr<IMFMediaBuffer> buffer;
        if (FAILED (sample->ConvertToContiguousBuffer (buffer.GetAddressOf())))
            return;

        const size_t rowBytes = (size_t) width * 4;
        ComPtr<IMF2DBuffer> buffer2D;

        if (SUCCEEDED (buffer.As (&buffer2D)))
        {
            BYTE* scanline0 = nullptr;
            LONG pitch = 0;

            if (SUCCEEDED (buffer2D->Lock2D (&scanline0, &pitch)))
            {
                for (int row = 0; row < height; ++row)
                {
                    auto* src = scanline0 + (ptrdiff_t) row * pitch;
                    auto* dst = frameBuffer.data() + (size_t) row * rowBytes;
                    std::memcpy (dst, src, rowBytes);
                }

                buffer2D->Unlock2D();
                return;
            }
        }

        // Fallback for buffers that don't support IMF2DBuffer: assume a
        // tightly packed, top-down RGB32 layout (already contiguous, so a
        // single memcpy would do, but keep the loop for symmetry/clarity).
        BYTE* data = nullptr;
        DWORD maxLen = 0, curLen = 0;

        if (SUCCEEDED (buffer->Lock (&data, &maxLen, &curLen)))
        {
            for (int row = 0; row < height; ++row)
            {
                auto* src = data + (size_t) row * rowBytes;
                auto* dst = frameBuffer.data() + (size_t) row * rowBytes;
                std::memcpy (dst, src, rowBytes);
            }

            buffer->Unlock();
        }
    }

    VideoPlayer& owner;
    juce::File file;
};

VideoPlayer::VideoPlayer() = default;

VideoPlayer::~VideoPlayer()
{
    close();
}

void VideoPlayer::load (const juce::File& file)
{
    close();

    decodeThread = std::make_unique<DecodeThread> (*this, file);
    decodeThread->startThread();
}

void VideoPlayer::close()
{
    decodeThread.reset(); // destructor stops the decode thread and joins it

    hasDecodedFrame = false;
    newFrameReady = false;
}

void VideoPlayer::pushFrame (const void* rgbaTopDown, int width, int height)
{
    const juce::ScopedLock sl (frameLock);
    latestFrameRGBA.replaceAll (rgbaTopDown, (size_t) width * (size_t) height * 4);
    latestFrameWidth = width;
    latestFrameHeight = height;
    newFrameReady = true;
    hasDecodedFrame = true;
}

void VideoPlayer::updateGLTexture()
{
    if (! newFrameReady.exchange (false))
        return;

    int width = 0, height = 0;
    juce::MemoryBlock localCopy;

    {
        const juce::ScopedLock sl (frameLock);
        width = latestFrameWidth;
        height = latestFrameHeight;
        localCopy = latestFrameRGBA;
    }

    if (width <= 0 || height <= 0)
        return;

    if (textureId == 0)
    {
        glGenTextures (1, &textureId);
        glBindTexture (GL_TEXTURE_2D, textureId);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glBindTexture (GL_TEXTURE_2D, textureId);
    }

    // Media Foundation's RGB32 is byte-order B,G,R,X in memory (little-
    // endian 0xXXRRGGBB, same as classic D3D X8R8G8B8) - GL_BGRA/
    // GL_UNSIGNED_BYTE matches that memory layout exactly, no swizzle.
    if (width != textureWidth || height != textureHeight)
    {
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, localCopy.getData());
        textureWidth = width;
        textureHeight = height;
    }
    else
    {
        glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, localCopy.getData());
    }

    glBindTexture (GL_TEXTURE_2D, 0);
}
