#include "ClipDecoder.h"
#include "Diagnostics.h"

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
        return false;

    ComPtr<IMFAttributes> attributes;
    MFCreateAttributes (attributes.GetAddressOf(), 1);
    attributes->SetUINT32 (MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE); // colour conversion to RGB32

    ComPtr<IMFSourceReader> reader;
    if (FAILED (MFCreateSourceReaderFromURL (file.getFullPathName().toWideCharPointer(), attributes.Get(), reader.GetAddressOf())))
    {
        logDiagnostic ("ClipDecoder: cannot open " + file.getFullPathName());
        return false;
    }
    reader->SetStreamSelection ((DWORD) MF_SOURCE_READER_ALL_STREAMS, FALSE);
    reader->SetStreamSelection (videoStream, TRUE);

    ComPtr<IMFMediaType> outType;
    MFCreateMediaType (outType.GetAddressOf());
    outType->SetGUID (MF_MT_MAJOR_TYPE, MFMediaType_Video);
    outType->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    if (FAILED (reader->SetCurrentMediaType (videoStream, nullptr, outType.Get())))
    {
        logDiagnostic ("ClipDecoder: no RGB32 output for " + file.getFullPathName());
        return false;
    }

    ComPtr<IMFMediaType> current;
    reader->GetCurrentMediaType (videoStream, current.GetAddressOf());
    UINT32 srcW = 0, srcH = 0, rateNum = 30, rateDen = 1;
    MFGetAttributeSize (current.Get(), MF_MT_FRAME_SIZE, &srcW, &srcH);
    MFGetAttributeRatio (current.Get(), MF_MT_FRAME_RATE, &rateNum, &rateDen);
    if (srcW == 0 || srcH == 0)
        return false;
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
    const double aspect = (double) srcW / (double) srcH;
    int edge = juce::jmin (maxEdge, (int) juce::jmax (srcW, srcH));
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
    logDiagnostic ("ClipDecoder: " + file.getFileName() + " " + juce::String ((int) srcW) + "x" + juce::String ((int) srcH)
                   + " @" + juce::String (srcFps, 2) + " fps, " + juce::String (duration, 1) + " s -> "
                   + juce::String (w) + "x" + juce::String (h) + " @" + juce::String (clip.fps, 1) + " fps, "
                   + juce::String (clip.capacity) + " frames" + (clip.capacity < wanted ? " (cut to fit memory)" : ""));

    // Area-average resample source RGB32 (top-down rows) -> BGR bottom-up.
    std::vector<int> x0 ((size_t) w + 1), y0 ((size_t) h + 1);
    for (int x = 0; x <= w; ++x) x0[(size_t) x] = (int) ((int64_t) x * srcW / w);
    for (int y = 0; y <= h; ++y) y0[(size_t) y] = (int) ((int64_t) y * srcH / h);

    double nextKeep = 0.0;
    const double keepStep = 1.0 / clip.fps;

    while (! clip.cancelled.load() && clip.decoded.load() < clip.capacity)
    {
        DWORD stream = 0, flags = 0;
        LONGLONG timestamp = 0;
        ComPtr<IMFSample> sample;
        if (FAILED (reader->ReadSample (videoStream, 0, &stream, &flags, &timestamp, sample.GetAddressOf())))
            break;
        if ((flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0)
            break;
        if (sample == nullptr)
            continue;

        const double t = (double) timestamp / 1.0e7;
        if (t + 1.0e-4 < nextKeep)
            continue; // drop frames above 30 fps
        nextKeep += keepStep;

        ComPtr<IMFMediaBuffer> buffer;
        if (FAILED (sample->ConvertToContiguousBuffer (buffer.GetAddressOf())))
            continue;

        BYTE* scan0 = nullptr;
        LONG pitch = (LONG) srcW * 4;
        ComPtr<IMF2DBuffer> buffer2D;
        bool locked2D = SUCCEEDED (buffer.As (&buffer2D)) && SUCCEEDED (buffer2D->Lock2D (&scan0, &pitch));
        DWORD maxLen = 0, curLen = 0;
        if (! locked2D && FAILED (buffer->Lock (&scan0, &maxLen, &curLen)))
            continue;

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
                    auto* src = scan0 + (ptrdiff_t) sy * pitch;
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

        if (locked2D) buffer2D->Unlock2D(); else buffer->Unlock();

        const int index = clip.decoded.load();
        clip.frames[(size_t) index] = std::move (frame);
        clip.decoded.store (index + 1); // publish after the frame is written
    }

    clip.finished = true;
    if (clip.decoded.load() == 0)
    {
        clip.failed = true;
        logDiagnostic ("ClipDecoder: no frames decoded from " + file.getFullPathName());
        return false;
    }
    logDiagnostic ("ClipDecoder: " + file.getFileName() + " ready, " + juce::String (clip.decoded.load()) + " frames");
    return true;
}
