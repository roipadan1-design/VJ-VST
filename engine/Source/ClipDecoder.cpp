#include "ClipDecoder.h"
#include "Diagnostics.h"
#include "MFFrame.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <propvarutil.h>
#include <wrl/client.h>
#include <cmath>
#include <cstring>

using Microsoft::WRL::ComPtr;

namespace
{
    constexpr DWORD videoStream = (DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM;

    struct ComScope
    {
        ComScope() : ok (SUCCEEDED (CoInitializeEx (nullptr, COINIT_MULTITHREADED))) {}
        ~ComScope() { if (ok) CoUninitialize(); }
        bool ok;
    };

    juce::String hresultToString (HRESULT hr)
    {
        return "0x" + juce::String::toHexString ((juce::int64) (juce::uint32) hr);
    }

    // The file's own (compressed) video format, for the log - "HEVC" with no
    // RGB32 output almost always means the HEVC Video Extensions aren't installed.
    juce::String nativeCodecName (IMFSourceReader* reader)
    {
        ComPtr<IMFMediaType> native;
        GUID subtype {};
        if (FAILED (reader->GetNativeMediaType (videoStream, 0, native.GetAddressOf()))
            || FAILED (native->GetGUID (MF_MT_SUBTYPE, &subtype)))
            return "unknown codec";
        if (subtype == MFVideoFormat_H264) return "H.264";
        if (subtype == MFVideoFormat_HEVC) return "HEVC/H.265 (needs the Microsoft 'HEVC Video Extensions')";
        if (subtype == MFVideoFormat_MP4V) return "MPEG-4 Part 2";
        if (subtype == MFVideoFormat_WMV3) return "WMV9";
        if (subtype == MFVideoFormat_VP90) return "VP9 (needs the Microsoft 'VP9 Video Extensions')";
        if (subtype == MFVideoFormat_AV1)  return "AV1 (needs the Microsoft 'AV1 Video Extension')";
        if (subtype == MFVideoFormat_MJPG) return "Motion JPEG";

        char fourcc[5] = { (char) (subtype.Data1 & 0xff), (char) ((subtype.Data1 >> 8) & 0xff),
                           (char) ((subtype.Data1 >> 16) & 0xff), (char) ((subtype.Data1 >> 24) & 0xff), 0 };
        return "codec '" + juce::String (fourcc) + "'";
    }

    bool fail (Clip& clip, const juce::String& message)
    {
        logDiagnostic ("ClipDecoder: " + message);
        clip.failed = true;
        clip.finished = true;
        return false;
    }
}

bool ClipDecoder::isSupportedVideo (const juce::File& file)
{
    return file.hasFileExtension ("mp4;mov;m4v;avi;wmv;mkv;webm");
}

bool ClipDecoder::decode (const juce::File& file, Clip& clip, size_t budgetBytes, int maxEdge)
{
    ComScope com;
    static const bool started = SUCCEEDED (MFStartup (MF_VERSION));
    if (! started)
        return fail (clip, "Media Foundation is not available");

    ComPtr<IMFAttributes> attributes;
    MFCreateAttributes (attributes.GetAddressOf(), 1);
    attributes->SetUINT32 (MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE); // colour conversion to RGB32

    ComPtr<IMFSourceReader> reader;
    if (auto hr = MFCreateSourceReaderFromURL (file.getFullPathName().toWideCharPointer(), attributes.Get(), reader.GetAddressOf());
        FAILED (hr))
        return fail (clip, "cannot open " + file.getFullPathName() + " (" + hresultToString (hr) + ")");

    reader->SetStreamSelection ((DWORD) MF_SOURCE_READER_ALL_STREAMS, FALSE);
    reader->SetStreamSelection (videoStream, TRUE);

    ComPtr<IMFMediaType> outType;
    MFCreateMediaType (outType.GetAddressOf());
    outType->SetGUID (MF_MT_MAJOR_TYPE, MFMediaType_Video);
    outType->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    if (auto hr = reader->SetCurrentMediaType (videoStream, nullptr, outType.Get()); FAILED (hr))
        return fail (clip, "no decoder for " + file.getFileName() + ": " + nativeCodecName (reader.Get())
                           + " (" + hresultToString (hr) + ") - H.264 MP4 always works");

    mfframe::Layout layout;
    if (! mfframe::readLayout (reader.Get(), videoStream, layout))
        return fail (clip, "no frame size for " + file.getFullPathName());

    ComPtr<IMFMediaType> current;
    reader->GetCurrentMediaType (videoStream, current.GetAddressOf());
    UINT32 rateNum = 30, rateDen = 1;
    MFGetAttributeRatio (current.Get(), MF_MT_FRAME_RATE, &rateNum, &rateDen);
    const double srcFps = rateDen > 0 && rateNum > 0 ? (double) rateNum / rateDen : 30.0;

    PROPVARIANT var;
    PropVariantInit (&var);
    double duration = 20.0;
    if (SUCCEEDED (reader->GetPresentationAttribute ((DWORD) MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var)))
        duration = (double) var.uhVal.QuadPart / 1.0e7;
    PropVariantClear (&var);

    // Size to the budget: at most 30 fps, long edge from maxEdge down to 480;
    // a clip that still doesn't fit is cut at the budget.
    clip.fps = juce::jmin (30.0, srcFps);
    const auto wanted = juce::jmax (1, (int) std::ceil (duration * clip.fps) + 2);
    const double aspect = (double) layout.width / (double) layout.height;
    int edge = juce::jmin (maxEdge, juce::jmax (layout.width, layout.height));
    auto sizeFor = [aspect] (int e, int& w, int& h) {
        if (aspect >= 1.0) { w = e; h = juce::jmax (2, (int) std::lround (e / aspect)); }
        else               { h = e; w = juce::jmax (2, (int) std::lround (e * aspect)); }
    };
    int w = 0, h = 0;
    sizeFor (edge, w, h);
    while (edge > 480 && (size_t) w * h * 3 * (size_t) wanted > budgetBytes)
    {
        edge -= 80;
        sizeFor (edge, w, h);
    }
    clip.width = w;
    clip.height = h;
    clip.capacity = (int) juce::jlimit ((size_t) 1, (size_t) wanted, budgetBytes / ((size_t) w * h * 3));
    clip.frames.resize ((size_t) clip.capacity);
    logDiagnostic ("ClipDecoder: " + file.getFileName() + " " + nativeCodecName (reader.Get()) + ", " + layout.describe()
                   + " @" + juce::String (srcFps, 2) + " fps, " + juce::String (duration, 1) + " s -> "
                   + juce::String (w) + "x" + juce::String (h) + " @" + juce::String (clip.fps, 1) + " fps, "
                   + juce::String (clip.capacity) + " frames" + (clip.capacity < wanted ? " (cut to fit memory)" : ""));

    // Area-average resample of the visible picture (RGB32, rows in picture
    // order via LockedFrame) -> BGR bottom-up. The source grid is rebuilt if
    // the decoder changes the frame layout mid-stream.
    std::vector<int> x0 ((size_t) w + 1), y0 ((size_t) h + 1);
    auto buildGrid = [&] {
        for (int x = 0; x <= w; ++x) x0[(size_t) x] = (int) ((int64_t) x * layout.width / w);
        for (int y = 0; y <= h; ++y) y0[(size_t) y] = (int) ((int64_t) y * layout.height / h);
    };
    buildGrid();

    double nextKeep = 0.0;
    const double keepStep = 1.0 / clip.fps;
    int unreadable = 0;
    HRESULT readError = S_OK;

    while (! clip.cancelled.load() && clip.decoded.load() < clip.capacity)
    {
        DWORD stream = 0, flags = 0;
        LONGLONG timestamp = 0;
        ComPtr<IMFSample> sample;
        if (auto hr = reader->ReadSample (videoStream, 0, &stream, &flags, &timestamp, sample.GetAddressOf()); FAILED (hr))
        {
            readError = hr;
            break;
        }
        if ((flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0)
            break;

        if ((flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) != 0)
        {
            mfframe::Layout changed;
            if (mfframe::readLayout (reader.Get(), videoStream, changed))
            {
                logDiagnostic ("ClipDecoder: " + file.getFileName() + " layout now " + changed.describe());
                layout = changed;
                buildGrid();
            }
        }

        if (sample == nullptr)
            continue;

        const double t = (double) timestamp / 1.0e7;
        if (t + 1.0e-4 < nextKeep)
            continue; // drop frames above 30 fps
        nextKeep += keepStep;

        mfframe::LockedFrame pixels (sample.Get(), layout);
        if (! pixels.isValid())
        {
            ++unreadable;
            continue;
        }

        auto frame = std::make_unique<uint8_t[]> (clip.frameBytes());
        for (int y = 0; y < h; ++y)
        {
            auto* dstRow = frame.get() + (size_t) (h - 1 - y) * (size_t) w * 3; // bottom row first
            const int sy0 = y0[(size_t) y], sy1 = juce::jmax (sy0 + 1, y0[(size_t) y + 1]);
            for (int x = 0; x < w; ++x)
            {
                const int sx0 = x0[(size_t) x], sx1 = juce::jmax (sx0 + 1, x0[(size_t) x + 1]);
                // Average at most a 3x3 grid of taps across the source cell.
                unsigned sum[3] = { 0, 0, 0 }, n = 0;
                const int stepY = juce::jmax (1, (sy1 - sy0) / 3), stepX = juce::jmax (1, (sx1 - sx0) / 3);
                for (int sy = sy0; sy < sy1; sy += stepY)
                {
                    auto* src = pixels.row (sy);
                    for (int sx = sx0; sx < sx1; sx += stepX)
                    {
                        auto* px = src + sx * 4; // B G R X
                        sum[0] += px[0]; sum[1] += px[1]; sum[2] += px[2];
                        ++n;
                    }
                }
                auto* d = dstRow + x * 3;
                d[0] = (uint8_t) (sum[0] / n); d[1] = (uint8_t) (sum[1] / n); d[2] = (uint8_t) (sum[2] / n);
            }
        }

        const int index = clip.decoded.load();
        clip.frames[(size_t) index] = std::move (frame);
        clip.decoded.store (index + 1); // publish after the frame is written
    }

    if (unreadable > 0)
        logDiagnostic ("ClipDecoder: " + file.getFileName() + " skipped " + juce::String (unreadable) + " unreadable frame(s)");

    if (clip.decoded.load() == 0)
        return fail (clip, "no frames decoded from " + file.getFullPathName()
                           + (readError != S_OK ? " (ReadSample " + hresultToString (readError) + ")" : juce::String()));

    clip.finished = true;
    logDiagnostic ("ClipDecoder: " + file.getFileName() + " ready, " + juce::String (clip.decoded.load()) + " frames"
                   + (readError != S_OK ? " (stopped early: ReadSample " + hresultToString (readError) + ")" : juce::String()));
    return true;
}
