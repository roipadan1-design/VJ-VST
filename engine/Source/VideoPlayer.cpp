#include "VideoPlayer.h"
#include "Diagnostics.h"
#include "MFFrame.h"

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
        mfframe::Layout layout;
        juce::String sourceDescription = isCameraMode() ? ("camera device " + juce::String (cameraDeviceIndex))
                                                          : file.getFullPathName();

        if (! openReader (reader))
        {
            logDiagnostic ("openReader() failed for " + sourceDescription);
            if (comInitialisedHere) CoUninitialize();
            return;
        }

        if (! mfframe::readLayout (reader.Get(), videoStreamIndex, layout))
        {
            logDiagnostic ("no frame size for " + sourceDescription);
            if (comInitialisedHere) CoUninitialize();
            return;
        }

        logDiagnostic ("opened " + sourceDescription + " at " + layout.describe() + ", starting decode loop");

        std::vector<uint8_t> frameBuffer;
        int unreadable = 0;
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

            // The decoder may only settle the real geometry once frames flow
            // (typical for H.264): re-read it, or the old stride/size would be
            // applied to the new frames.
            if ((flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) != 0)
            {
                mfframe::Layout changed;
                if (mfframe::readLayout (reader.Get(), videoStreamIndex, changed))
                {
                    logDiagnostic (sourceDescription + " layout now " + changed.describe());
                    layout = changed;
                }
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

            if (copySampleToFrameBuffer (sample.Get(), layout, frameBuffer))
                owner.pushFrame (frameBuffer.data(), layout.width, layout.height);
            else if (unreadable++ == 0)
                logDiagnostic ("unreadable frame from " + sourceDescription + " (buffer smaller than " + layout.describe() + ")");

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

    static void seekToStart (IMFSourceReader* reader)
    {
        PROPVARIANT var;
        PropVariantInit (&var);
        var.vt = VT_I8;
        var.hVal.QuadPart = 0;
        reader->SetCurrentPosition (GUID_NULL, var);
        PropVariantClear (&var);
    }

    // Copies the sample's visible picture into frameBuffer (resized to fit)
    // as tightly packed B,G,R,X rows, BOTTOM row first - the orientation of
    // every other texture the engine samples (stage FBOs, still images,
    // MEDIA-tab clips), so the v2 scenes show it upright. The previous
    // straight top-down copy put the webcam upside down on screen: rows are
    // now taken in picture order via LockedFrame (which resolves Lock2D vs
    // plain buffers, signed strides and decoder row padding) and reversed.
    // Returns false (frame skipped) if the buffer is unreadable.
    static bool copySampleToFrameBuffer (IMFSample* sample, const mfframe::Layout& layout, std::vector<uint8_t>& frameBuffer)
    {
        mfframe::LockedFrame pixels (sample, layout);
        if (! pixels.isValid())
            return false;

        const size_t rowBytes = (size_t) layout.width * 4;
        frameBuffer.resize (rowBytes * (size_t) layout.height);

        for (int row = 0; row < layout.height; ++row)
            std::memcpy (frameBuffer.data() + (size_t) (layout.height - 1 - row) * rowBytes, pixels.row (row), rowBytes);

        return true;
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

void VideoPlayer::pushFrame (const void* bgraBottomUp, int width, int height)
{
    const juce::ScopedLock sl (frameLock);
    latestFrameRGBA.replaceAll (bgraBottomUp, (size_t) width * (size_t) height * 4);
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
