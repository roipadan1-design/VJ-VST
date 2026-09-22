#pragma once

#include <cstdint>
#include <vector>

namespace vj
{

// Adaptive peak-picker for one onset-novelty stream (ENGINEERING_SPEC 4.5).
//
// Threshold = median + kMad * MAD + floor over ~1 s of recent novelty, where
// floor = max(absoluteFloor, relativeFloor * median). The relative term is our
// addition to the spec's constant floor: median + 3*MAD alone is scale
// invariant, so steady noise would still cross it by chance a few times a
// second - requiring a real jump *relative to the typical level* is what
// keeps stationary material (noise, pads) quiet.
//
// The newest `guard` frames are excluded from the statistics so a transient
// cannot raise its own threshold. Decisions use one hop of lookahead: frame
// t-1 fires if it exceeds the threshold, is a local maximum, and the region's
// refractory period has elapsed. No allocation after prepare().
class OnsetTracker
{
public:
    struct Settings
    {
        float kMad = 3.0f;
        float absoluteFloor = 0.02f;
        float relativeFloor = 0.6f;
        float refractorySeconds = 0.07f;
    };

    struct Detection
    {
        bool fired = false;
        int64_t frame = 0;     // hop index of the detected peak
        float strength = 0.0f; // 0..1, from exceedance above threshold
        float novelty = 0.0f, threshold = 0.0f;
    };

    void prepare (int historyFrames, float hopSeconds, const Settings& s);
    void reset() noexcept;
    void setSensitivity (float s) noexcept { sensitivity = s; }

    // Push the novelty for hop `frame`; returns a detection for frame-1 if any.
    Detection push (float novelty, int64_t frame, bool gateOpen) noexcept;

    float getLastThreshold() const noexcept { return lastThreshold; }

private:
    float computeThreshold() noexcept;

    Settings settings;
    float hopSeconds = 0.0f;
    float sensitivity = 1.0f;
    std::vector<float> history, scratch;
    int write = 0, count = 0;
    static constexpr int guard = 3;

    float prev2 = 0.0f, prev1 = 0.0f;
    float prev1Threshold = 0.0f;
    int64_t lastFired = -1000000;
    float lastThreshold = 0.0f;
};

} // namespace vj
