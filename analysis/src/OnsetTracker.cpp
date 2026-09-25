#include "vj/OnsetTracker.h"

#include <algorithm>
#include <cmath>

namespace vj
{

void OnsetTracker::prepare (int historyFrames, float hop, const Settings& s)
{
    settings = s;
    hopSeconds = hop;
    history.assign ((size_t) std::max (historyFrames, guard + 8), 0.0f);
    scratch.assign (history.size(), 0.0f);
    reset();
}

void OnsetTracker::reset() noexcept
{
    std::fill (history.begin(), history.end(), 0.0f);
    write = count = 0;
    prev2 = prev1 = prev1Threshold = lastThreshold = 0.0f;
    lastFired = -1000000;
    armed = true;
    quietHops = 0;
}

float OnsetTracker::computeThreshold() noexcept
{
    // Statistics over everything except the newest `guard` entries.
    const int size = (int) history.size();
    const int usable = count - guard;

    if (usable < 8)
        return 1.0e9f; // not enough context yet: never fire during warm-up

    for (int i = 0; i < usable; ++i)
    {
        auto idx = (write - 1 - guard - i + size * 2) % size;
        scratch[(size_t) i] = history[(size_t) idx];
    }

    auto mid = scratch.begin() + usable / 2;
    std::nth_element (scratch.begin(), mid, scratch.begin() + usable);
    auto median = *mid;

    for (int i = 0; i < usable; ++i)
        scratch[(size_t) i] = std::abs (scratch[(size_t) i] - median);

    std::nth_element (scratch.begin(), mid, scratch.begin() + usable);
    auto mad = *mid;

    auto floor = std::max (settings.absoluteFloor, settings.relativeFloor * median);
    return (median + settings.kMad * mad + floor) / std::max (0.1f, sensitivity);
}

OnsetTracker::Detection OnsetTracker::push (float novelty, int64_t frame, bool gateOpen) noexcept
{
    if (! std::isfinite (novelty) || novelty < 0.0f)
        novelty = 0.0f;

    Detection d;

    if (settings.fireOnRise)
    {
        // prev1Threshold was computed from everything up to the previous hop
        // (minus the guard), i.e. it is this hop's threshold.
        const auto threshold = prev1Threshold;
        const auto refractoryFrames = (int64_t) std::ceil (settings.refractorySeconds / std::max (1.0e-6f, hopSeconds));
        // Hysteresis: re-arm only after the novelty has stayed well below the
        // threshold for a few hops - a brief dip in one kick's tail must not
        // let it fire twice.
        quietHops = novelty < threshold * 0.6f ? quietHops + 1 : 0;
        if (quietHops >= 3)
            armed = true;
        else if (novelty > threshold && gateOpen && armed && frame - lastFired >= refractoryFrames)
        {
            d.fired = true;
            d.frame = frame;
            d.novelty = novelty;
            d.threshold = threshold;
            // The peak is still to come: estimate it one hop ahead from the slope.
            auto estimate = novelty + std::max (0.0f, novelty - prev1);
            auto ratio = estimate / std::max (1.0e-6f, threshold);
            d.strength = std::clamp (0.25f + 0.75f * (ratio - 1.0f) / 3.0f, 0.0f, 1.0f);
            lastFired = frame;
            armed = false;
        }

        history[(size_t) write] = novelty;
        write = (write + 1) % (int) history.size();
        count = std::min (count + 1, (int) history.size());
        prev2 = prev1;
        prev1 = novelty;
        prev1Threshold = computeThreshold();
        lastThreshold = prev1Threshold;
        return d;
    }

    // Decide on frame-1 now that we know its right-hand neighbour.
    const auto candidate = prev1;
    const auto threshold = prev1Threshold;
    const auto refractoryFrames = (int64_t) std::ceil (settings.refractorySeconds / std::max (1.0e-6f, hopSeconds));

    if (gateOpen && candidate > threshold && candidate >= prev2 && candidate >= novelty
        && (frame - 1) - lastFired >= refractoryFrames)
    {
        d.fired = true;
        d.frame = frame - 1;
        d.novelty = candidate;
        d.threshold = threshold;
        auto ratio = candidate / std::max (1.0e-6f, threshold);
        d.strength = std::clamp (0.25f + 0.75f * (ratio - 1.0f) / 3.0f, 0.0f, 1.0f);
        lastFired = frame - 1;
    }

    history[(size_t) write] = novelty;
    write = (write + 1) % (int) history.size();
    count = std::min (count + 1, (int) history.size());

    prev2 = prev1;
    prev1 = novelty;
    prev1Threshold = computeThreshold();
    lastThreshold = prev1Threshold;
    return d;
}

} // namespace vj
