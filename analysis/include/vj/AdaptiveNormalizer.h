#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace vj
{

enum class NormalizerMode
{
    locked,   // anchors freeze once calibrated
    balanced, // default: fast-up / slow-down anchors over an 8 s history
};

// Turns a dB measurement into a 0..1 "relative activity" that stays usable
// across quiet and loud material without erasing the contrast inside a track
// (ENGINEERING_SPEC 4.4):
//
//  - 8 s of 100 Hz gated history in a fixed 0.5 dB histogram (-100..+12 dB),
//    so percentiles cost O(bins) with no sorting and no allocation.
//  - Q20 / Q95 recomputed at 10 Hz. Upper anchor U* = Q95 + 3 dB, span
//    W* = clamp(U* - Q20, 24, 60) dB - a narrow dynamic range can never
//    blow tiny fluctuations up to full scale.
//  - U rises with a 150 ms time constant (a loud arrival reduces sensitivity
//    promptly) but falls no faster than 2 dB/s (a breakdown does not
//    instantly pump back to full brightness). W is smoothed over 3 s.
//  - While the caller reports the gate closed, nothing enters the history and
//    the anchors hold - silence is never learned as the "quiet end" of the music.
//  - Before 2 s of gated history exists, a conservative fixed reference is
//    used, then blended to the learned anchors over 1 s.
class AdaptiveNormalizer
{
public:
    void reset() noexcept;
    void setMode (NormalizerMode m) noexcept { mode = m; }

    // Feed one measurement. `dtSeconds` is the time since the previous call.
    // Returns relative activity in 0..1 (0 while the gate is closed).
    float process (float db, bool gateOpen, float dtSeconds) noexcept;

    float getUpperAnchorDb() const noexcept { return upper; }
    float getSpanDb() const noexcept { return span; }
    bool isCalibrated() const noexcept { return calibrated; }

    static constexpr float minDb = -100.0f, maxDb = 12.0f, binWidthDb = 0.5f;
    static constexpr int numBins = (int) ((maxDb - minDb) / binWidthDb);
    static constexpr int historyLength = 800; // 8 s at 100 Hz

private:
    void admit (float db) noexcept;
    void updateTargets() noexcept;
    float quantile (float q) const noexcept;

    NormalizerMode mode = NormalizerMode::balanced;

    std::array<uint16_t, numBins> histogram {};
    std::array<uint16_t, historyLength> ring {}; // bin index of each admitted entry
    int ringWrite = 0, ringCount = 0;

    float admitClock = 0.0f, statsClock = 0.0f, blendClock = 0.0f;
    float targetUpper = fixedUpperDb, targetSpan = fixedSpanDb;
    float upper = fixedUpperDb, span = fixedSpanDb;
    bool calibrated = false;

    static constexpr float fixedUpperDb = -6.0f, fixedSpanDb = 48.0f;
};

} // namespace vj
