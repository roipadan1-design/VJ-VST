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
        // Fire on the frame the novelty CROSSES the threshold instead of at
        // its local peak one hop later. Worth it where the novelty rises
        // slowly (the bass band's fast/slow power ratio peaks 10-20 ms after
        // the kick). Re-arms once the novelty falls back below the threshold,
        // so a long rise never fires twice; strength is estimated from the
        // slope at the crossing.
        bool fireOnRise = false;
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
    bool armed = true;
    int quietHops = 0;
    float lastThreshold = 0.0f;
};

} // namespace vj
