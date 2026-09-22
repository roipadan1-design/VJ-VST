#include "VideoPlayer.h"
#include "Diagnostics.h"

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
#pragma comment(lib, "mf.lib") // MFEnumDeviceSources (camera device enumeration)
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

    juce::String hresultToString (HRESULT hr)
    {
        return "0x" + juce::String::toHexString ((juce::int64) hr);
    }

    juce::String subtypeToString (const GUID& g)
    {
        if (g == MFVideoFormat_RGB32) return "RGB32";
        if (g == MFVideoFormat_RGB24) return "RGB24";
        if (g == MFVideoFormat_YUY2)  return "YUY2";
        if (g == MFVideoFormat_NV12)  return "NV12";
        if (g == MFVideoFormat_MJPG)  return "MJPG";
        if (g == MFVideoFormat_I420)  return "I420";
        if (g == MFVideoFormat_H264)  return "H264";

        OLECHAR guidStr[40] {};
        StringFromGUID2 (g, guidStr, 40);
        return juce::String (guidStr);
    }
}

class VideoPlayer::DecodeThread : public juce::Thread
{
public:
    // cameraDeviceIndexIn = -1 means "file mode" (use fileIn); >= 0 means
    // "camera mode" (open that capture device, fileIn is ignored).
    DecodeThread (VideoPlayer& ownerIn, juce::File fileIn, int cameraDeviceIndexIn)
        : juce::Thread ("VideoDecode"), owner (ownerIn), file (std::move (fileIn)),
          cameraDeviceIndex (cameraDeviceIndexIn)
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
        juce::String sourceDescription = isCameraMode() ? ("camera device " + juce::String (cameraDeviceIndex))
                                                          : file.getFullPathName();

        if (! openReader (reader))
        {
            logDiagnostic ("openReader() failed for " + sourceDescription);
            if (comInitialisedHere) CoUninitialize();
            return;
        }

        if (! queryFrameSize (reader.Get(), width, height) || width <= 0 || height <= 0)
        {
            logDiagnostic ("queryFrameSize() failed for " + sourceDescription + " (got " + juce::String (width) + "x" + juce::String (height) + ")");
            if (comInitialisedHere) CoUninitialize();
            return;
        }

        const int fallbackStride = queryStride (reader.Get());

        logDiagnostic ("opened " + sourceDescription + " at " + juce::String (width) + "x" + juce::String (height)
                        + " (fallback stride " + juce::String (fallbackStride) + "), starting decode loop");

        std::vector<uint8_t> frameBuffer ((size_t) width * (size_t) height * 4);
        double playbackStartMs = juce::Time::getMillisecondCounterHiRes();
        int samplesReceived = 0;

        while (! threadShouldExit())
        {
            DWORD streamIndex = 0, flags = 0;
            LONGLONG timestamp = 0;
            ComPtr<IMFSample> sample;

            auto hr = reader->ReadSample (videoStreamIndex, 0,
                                           &streamIndex, &flags, &timestamp, sample.GetAddressOf());

            if (FAILED (hr))
            {
                logDiagnostic ("ReadSample failed for " + sourceDescription + ": " + hresultToString (hr)
                                + " (after " + juce::String (samplesReceived) + " sample(s))");
                break;
            }

            if ((flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0)
            {
                // A live capture device isn't expected to hit end-of-stream -
                // if it does (device unplugged, etc.), treat it as gone
                // rather than looping forever on a dead source.
                if (isCameraMode())
                    break;

                seekToStart (reader.Get());
                playbackStartMs = juce::Time::getMillisecondCounterHiRes();
                continue;
            }

            if (sample == nullptr)
            {
                // Common for capture devices: a "stream tick" with no actual
                // sample data this pass. Log the first couple so a dead
                // camera (endless empty ticks) is visible, without spamming.
                if (isCameraMode() && samplesReceived == 0 && flags != 0)
                    logDiagnostic ("ReadSample returned null sample, flags=" + hresultToString ((HRESULT) flags));
                continue;
            }

            if (samplesReceived == 0)
                logDiagnostic ("first sample received from " + sourceDescription);
            ++samplesReceived;

            copySampleToFrameBuffer (sample.Get(), width, height, fallbackStride, frameBuffer);
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
    bool isCameraMode() const noexcept { return cameraDeviceIndex >= 0; }

    bool openReader (ComPtr<IMFSourceReader>& reader)
    {
        ComPtr<IMFAttributes> attributes;
        MFCreateAttributes (attributes.GetAddressOf(), 1);

        // Without this, SetCurrentMediaType() below fails to convert most
        // compressed/YUV sources (H.264 files, or a webcam's native MJPEG/
        // YUY2) to RGB32 - this attribute is what allows the source reader
        // to insert Media Foundation's video processor to do that conversion.
        attributes->SetUINT32 (MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

        HRESULT hr;

        if (isCameraMode())
        {
            ComPtr<IMFMediaSource> cameraSource;
            if (! openCameraMediaSource (cameraDeviceIndex, cameraSource))
                return false;

            hr = MFCreateSourceReaderFromMediaSource (cameraSource.Get(), attributes.Get(), reader.GetAddressOf());
            if (FAILED (hr) || reader == nullptr)
            {
                logDiagnostic ("MFCreateSourceReaderFromMediaSource failed: " + hresultToString (hr));
                return false;
            }
        }
        else
        {
            hr = MFCreateSourceReaderFromURL (file.getFullPathName().toWideCharPointer(),
                                               attributes.Get(), reader.GetAddressOf());
            if (FAILED (hr) || reader == nullptr)
            {
                logDiagnostic ("MFCreateSourceReaderFromURL failed: " + hresultToString (hr));
                return false;
            }
        }

        reader->SetStreamSelection ((DWORD) MF_SOURCE_READER_ALL_STREAMS, FALSE);
        reader->SetStreamSelection (videoStreamIndex, TRUE);

        ComPtr<IMFMediaType> outputType;

        if (isCameraMode())
        {
            // Capture devices need a fully-specified output type (frame
            // size/rate cloned from one of the device's own native types) -
            // empirically, a bare major+subtype request with no frame size
            // "succeeds" (SetCurrentMediaType returns S_OK) but the device
            // then only ever delivers STREAMTICK markers, never a real
            // sample, at least on the UVC webcam this was tested against
            // at its 1920x1080 native resolution. Requesting a modest
            // resolution native type instead fixed it.
            if (! chooseCameraOutputType (reader.Get(), outputType))
            {
                logDiagnostic ("chooseCameraOutputType found no usable native media type");
                return false;
            }
        }
        else
        {
            MFCreateMediaType (outputType.GetAddressOf());
            outputType->SetGUID (MF_MT_MAJOR_TYPE, MFMediaType_Video);
            outputType->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_RGB32);
        }

        hr = reader->SetCurrentMediaType (videoStreamIndex, nullptr, outputType.Get());
        if (FAILED (hr))
            logDiagnostic ("SetCurrentMediaType(RGB32) failed: " + hresultToString (hr) + (isCameraMode() ? " (camera)" : " (file)"));

        return SUCCEEDED (hr);
    }

    // Picks one of the capture device's own native media types - preferring
    // the largest that's still <= 1280x720, falling back to the smallest
    // available if the device has nothing that small - clones ALL of its
    // attributes (frame size, frame rate, interlace mode, etc.) and only
    // swaps MF_MT_SUBTYPE to RGB32. This is the standard robust pattern for
    // capture devices; unlike file playback, an underspecified output type
    // can silently negotiate to a resolution the device can't actually
    // stream in real time.
    static bool chooseCameraOutputType (IMFSourceReader* reader, ComPtr<IMFMediaType>& outputType)
    {
        ComPtr<IMFMediaType> chosenNative;
        UINT64 chosenArea = 0;
        constexpr UINT64 preferredMaxArea = 1280ull * 720ull;

        for (DWORD i = 0; ; ++i)
        {
            ComPtr<IMFMediaType> nativeType;
            auto hr = reader->GetNativeMediaType (videoStreamIndex, i, nativeType.GetAddressOf());

            if (hr == MF_E_NO_MORE_TYPES || FAILED (hr))
                break;

            UINT32 w = 0, h = 0;
            if (FAILED (MFGetAttributeSize (nativeType.Get(), MF_MT_FRAME_SIZE, &w, &h)) || w == 0 || h == 0)
                continue;

            GUID subtype {};
            nativeType->GetGUID (MF_MT_SUBTYPE, &subtype);
            logDiagnostic ("  native type " + juce::String ((int) i) + ": " + juce::String ((int) w) + "x" + juce::String ((int) h)
                            + " subtype=" + subtypeToString (subtype));

            UINT64 area = (UINT64) w * (UINT64) h;
            bool withinPreferred = area <= preferredMaxArea;
            bool chosenWithinPreferred = chosenArea != 0 && chosenArea <= preferredMaxArea;

            bool takeIt = chosenNative == nullptr
                        || (withinPreferred && ! chosenWithinPreferred)
                        || (withinPreferred && chosenWithinPreferred && area > chosenArea)
                        || (! withinPreferred && ! chosenWithinPreferred && area < chosenArea);

            if (takeIt)
            {
                chosenNative = nativeType;
                chosenArea = area;
            }
        }

        if (chosenNative == nullptr)
            return false;

        ComPtr<IMFMediaType> newType;
        if (FAILED (MFCreateMediaType (newType.GetAddressOf())))
            return false;

        chosenNative->CopyAllItems (newType.Get());
        newType->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_RGB32);

        outputType = newType;
        return true;
    }

    // Enumerates Media Foundation video capture devices and activates the
    // one at deviceIndex (0 = first, matching what most webcam-picker UIs
    // call "default"). Same pattern used by numerous native MF capture
    // samples: enumerate via MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP,
    // ActivateObject() the chosen IMFActivate to get a real IMFMediaSource.
    static bool openCameraMediaSource (int deviceIndex, ComPtr<IMFMediaSource>& outSource)
    {
        ComPtr<IMFAttributes> enumAttributes;
        MFCreateAttributes (enumAttributes.GetAddressOf(), 1);
        enumAttributes->SetGUID (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);

        IMFActivate** devices = nullptr;
        UINT32 count = 0;

        auto hr = MFEnumDeviceSources (enumAttributes.Get(), &devices, &count);
        if (FAILED (hr))
        {
            logDiagnostic ("MFEnumDeviceSources failed: " + hresultToString (hr));
            return false;
        }

        if (count == 0)
        {
            logDiagnostic ("MFEnumDeviceSources found 0 capture devices");
            return false;
        }

        bool ok = false;

        if (deviceIndex >= 0 && (UINT32) deviceIndex < count)
        {
            IMFMediaSource* source = nullptr;
            auto activateHr = devices[deviceIndex]->ActivateObject (IID_PPV_ARGS (&source));

            if (SUCCEEDED (activateHr))
            {
                outSource.Attach (source);
                ok = true;
            }
            else
            {
                logDiagnostic ("ActivateObject failed for camera device " + juce::String (deviceIndex)
                                + ": " + hresultToString (activateHr)
                                + " (" + juce::String ((int) count) + " device(s) found - likely in use by another app, or blocked by Windows camera privacy settings)");
            }
        }
        else
        {
            logDiagnostic ("camera device index " + juce::String (deviceIndex) + " out of range ("
                            + juce::String ((int) count) + " found)");
        }

        for (UINT32 i = 0; i < count; ++i)
            devices[i]->Release();

        CoTaskMemFree (devices);
        return ok;
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

    // MF_MT_DEFAULT_STRIDE is the row pitch the video processor MFT actually
    // produces for its RGB32 output - not necessarily width*4. Widths that
    // aren't a multiple of the decoder's internal alignment (16 pixels is a
    // common requirement) get padded, and assuming a tightly-packed stride
    // in that case reads each row a few bytes into the next, which
    // accumulates into a diagonal shear across the frame. Returns 0 if the
    // attribute isn't present (some sources don't set it); callers should
    // fall back to width*4 in that case.
    static int queryStride (IMFSourceReader* reader)
    {
        ComPtr<IMFMediaType> currentType;
        if (FAILED (reader->GetCurrentMediaType (videoStreamIndex, currentType.GetAddressOf())))
            return 0;

        UINT32 stride = 0;
        if (FAILED (currentType->GetUINT32 (MF_MT_DEFAULT_STRIDE, &stride)))
            return 0;

        return (int) stride;
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
    static void copySampleToFrameBuffer (IMFSample* sample, int width, int height, int fallbackStride, std::vector<uint8_t>& frameBuffer)
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

        // Fallback for buffers that don't support IMF2DBuffer: use the
        // stride Media Foundation actually reported (MF_MT_DEFAULT_STRIDE),
        // not width*4 - some sources pad each row to the decoder's internal
        // alignment (e.g. a 1080px-wide frame padded to 1088), and copying
        // with an assumed tightly-packed stride reads a few bytes into the
        // next row each time, producing a diagonal shear across the frame.
        BYTE* data = nullptr;
        DWORD maxLen = 0, curLen = 0;
        const size_t srcStride = fallbackStride > 0 ? (size_t) fallbackStride : rowBytes;

        if (SUCCEEDED (buffer->Lock (&data, &maxLen, &curLen)))
        {
            for (int row = 0; row < height; ++row)
            {
                auto* src = data + (size_t) row * srcStride;
                auto* dst = frameBuffer.data() + (size_t) row * rowBytes;
                std::memcpy (dst, src, rowBytes);
            }

            buffer->Unlock();
        }
    }

    VideoPlayer& owner;
    juce::File file;
    int cameraDeviceIndex = -1;
};

VideoPlayer::VideoPlayer() = default;

VideoPlayer::~VideoPlayer()
{
    close();
}

void VideoPlayer::load (const juce::File& file)
{
    close();

    decodeThread = std::make_unique<DecodeThread> (*this, file, -1);
    decodeThread->startThread();
}

void VideoPlayer::openCamera (int deviceIndex)
{
    close();

    decodeThread = std::make_unique<DecodeThread> (*this, juce::File(), deviceIndex);
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
