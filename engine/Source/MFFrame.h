#pragma once

#include <JuceHeader.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>
#include <cstdlib>

// Reading RGB32 pixels out of a Media Foundation Source Reader sample, shared
// by VideoPlayer (webcam / engine-window drops) and ClipDecoder (MEDIA-tab
// clips). Three things make a naive "width * 4 bytes per row, row 0 = top"
// copy wrong, and each one has bitten this project:
//
//  - Row pitch. The decoder pads rows to its own alignment (16 px is
//    common), so a 504 or 1080 px wide video can arrive with 512 / 1088 px
//    rows. Assuming width * 4 reads a few bytes into the next row each time:
//    a diagonal shear that turns the picture into streaks.
//  - Row order. MF_MT_DEFAULT_STRIDE is signed: negative means the buffer is
//    bottom-up in memory. IMF2DBuffer::Lock2D always hands back the TOP row
//    plus a signed pitch; a plain IMFMediaBuffer::Lock does not.
//  - Mid-stream format changes. The H.264 decoder often only learns the real
//    geometry from the first frames and then reports
//    MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED; the layout must be re-read
//    then (readLayout again), or the old size/stride is applied to new frames.
//
// The visible picture may also be a sub-rectangle of the decoded frame
// (MF_MT_MINIMUM_DISPLAY_APERTURE) - Layout's x/y/width/height is that
// rectangle, and LockedFrame::row() is relative to it.
namespace mfframe
{
    struct Layout
    {
        int frameWidth = 0, frameHeight = 0;       // MF_MT_FRAME_SIZE (may include padding)
        int x = 0, y = 0, width = 0, height = 0;   // the visible picture inside the frame
        LONG stride = 0;                           // signed; < 0 = bottom-up; 0 = unknown

        juce::String describe() const
        {
            return juce::String (width) + "x" + juce::String (height)
                 + (width != frameWidth || height != frameHeight
                        ? " (in a " + juce::String (frameWidth) + "x" + juce::String (frameHeight) + " frame)" : juce::String())
                 + ", stride " + (stride != 0 ? juce::String ((int) stride) : juce::String ("unreported"));
        }
    };

    inline bool readLayout (IMFSourceReader* reader, DWORD stream, Layout& out)
    {
        Microsoft::WRL::ComPtr<IMFMediaType> type;
        if (FAILED (reader->GetCurrentMediaType (stream, type.GetAddressOf())))
            return false;

        UINT32 w = 0, h = 0;
        if (FAILED (MFGetAttributeSize (type.Get(), MF_MT_FRAME_SIZE, &w, &h)) || w == 0 || h == 0)
            return false;

        Layout l;
        l.frameWidth = l.width = (int) w;
        l.frameHeight = l.height = (int) h;

        MFVideoArea area {};
        UINT32 areaBytes = 0;
        if (SUCCEEDED (type->GetBlob (MF_MT_MINIMUM_DISPLAY_APERTURE, (UINT8*) &area, sizeof (area), &areaBytes))
            && areaBytes == sizeof (area))
        {
            const int ax = area.OffsetX.value, ay = area.OffsetY.value;
            const int aw = (int) area.Area.cx, ah = (int) area.Area.cy;
            if (aw > 0 && ah > 0 && ax >= 0 && ay >= 0 && ax + aw <= l.frameWidth && ay + ah <= l.frameHeight)
            {
                l.x = ax; l.y = ay;
                l.width = aw; l.height = ah;
            }
        }

        UINT32 stride = 0;
        if (SUCCEEDED (type->GetUINT32 (MF_MT_DEFAULT_STRIDE, &stride)))
        {
            l.stride = (LONG) (INT32) stride;
        }
        else
        {
            // Not reported: the documented fallback computes the format's
            // default (and its sign - RGB formats may default to bottom-up).
            GUID subtype {};
            LONG computed = 0;
            if (SUCCEEDED (type->GetGUID (MF_MT_SUBTYPE, &subtype))
                && SUCCEEDED (MFGetStrideForBitmapInfoHeader (subtype.Data1, w, &computed)))
                l.stride = computed;
        }

        out = l;
        return true;
    }

    // Locks one RGB32 sample and hands out its rows in picture order:
    // row (0) is the TOP of the visible picture, whatever the memory order.
    // isValid() is false if the buffer can't be locked or is too small for
    // the layout (the frame is then skipped rather than read out of bounds).
    class LockedFrame
    {
    public:
        LockedFrame (IMFSample* sample, const Layout& layout)
        {
            if (sample == nullptr || FAILED (sample->ConvertToContiguousBuffer (buffer.GetAddressOf())))
                return;

            BYTE* top = nullptr;   // the frame's top row (before the aperture offset)
            LONG signedPitch = 0;

            if (SUCCEEDED (buffer.As (&buffer2D)) && SUCCEEDED (buffer2D->Lock2D (&top, &signedPitch)))
            {
                locked2D = true;   // Lock2D: top row + signed pitch, already resolved
            }
            else
            {
                buffer2D.Reset();
                BYTE* data = nullptr;
                DWORD maxLength = 0, length = 0;
                if (FAILED (buffer->Lock (&data, &maxLength, &length)))
                    return;
                lockedPlain = true;

                const size_t rowBytes = (size_t) layout.frameWidth * 4;
                const size_t rows = (size_t) layout.frameHeight;
                const size_t reported = (size_t) std::labs (layout.stride);

                // The buffer's own length is the ground truth: when it is
                // exactly N whole rows, that is the pitch actually used.
                size_t pitch = 0;
                if (reported >= rowBytes && reported * rows == (size_t) length)
                    pitch = reported;
                else if (length % rows == 0 && length / rows >= rowBytes)
                    pitch = length / rows;
                else if (reported >= rowBytes && reported * (rows - 1) + rowBytes <= (size_t) length)
                    pitch = reported;
                else if (rowBytes * rows <= (size_t) length)
                    pitch = rowBytes;
                else
                    return; // too small for the frame it claims to be

                const bool bottomUp = layout.stride < 0;
                signedPitch = bottomUp ? -(LONG) pitch : (LONG) pitch;
                top = bottomUp ? data + pitch * (rows - 1) : data;
            }

            pitchBytes = signedPitch;
            first = top + (ptrdiff_t) layout.y * signedPitch + (ptrdiff_t) layout.x * 4;
            valid = true;
        }

        ~LockedFrame()
        {
            if (locked2D)
                buffer2D->Unlock2D();
            else if (lockedPlain)
                buffer->Unlock();
        }

        bool isValid() const noexcept                 { return valid; }
        const BYTE* row (int pictureRow) const noexcept { return first + (ptrdiff_t) pictureRow * pitchBytes; } // B G R X pixels

    private:
        Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;
        Microsoft::WRL::ComPtr<IMF2DBuffer> buffer2D;
        bool locked2D = false, lockedPlain = false, valid = false;
        const BYTE* first = nullptr;
        LONG pitchBytes = 0;

        JUCE_DECLARE_NON_COPYABLE (LockedFrame)
    };
}
