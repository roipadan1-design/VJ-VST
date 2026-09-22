#include "vj/AdaptiveNormalizer.h"

#include <algorithm>
#include <cmath>

namespace vj
{

namespace
{
    constexpr float admitInterval = 0.01f;   // 100 Hz history
    constexpr float statsInterval = 0.1f;    // 10 Hz quantile updates
    constexpr float calibrationSeconds = 2.0f;
    constexpr float blendSeconds = 1.0f;
    constexpr float riseTau = 0.15f;
    constexpr float maxFallDbPerSecond = 2.0f;
    constexpr float spanTau = 3.0f;
    constexpr float minSpan = 24.0f, maxSpan = 60.0f;

    float smoothingCoefficient (float dt, float tau) noexcept
    {
        return tau <= 0.0f ? 0.0f : std::exp (-dt / tau);
    }
}

void AdaptiveNormalizer::reset() noexcept
{
    histogram.fill (0);
    ringWrite = ringCount = 0;
    admitClock = statsClock = blendClock = 0.0f;
    targetUpper = upper = fixedUpperDb;
    targetSpan = span = fixedSpanDb;
    calibrated = false;
}

void AdaptiveNormalizer::admit (float db) noexcept
{
    auto bin = (int) ((std::clamp (db, minDb, maxDb - 0.001f) - minDb) / binWidthDb);

    if (ringCount == historyLength)
        --histogram[ring[(size_t) ringWrite]];
    else
        ++ringCount;

    ring[(size_t) ringWrite] = (uint16_t) bin;
    ++histogram[(size_t) bin];
    ringWrite = (ringWrite + 1) % historyLength;
}

float AdaptiveNormalizer::quantile (float q) const noexcept
{
    auto rank = (int) std::ceil (q * (float) ringCount);
    rank = std::clamp (rank, 1, ringCount);

    int cumulative = 0;
    for (int b = 0; b < numBins; ++b)
    {
        cumulative += histogram[(size_t) b];
        if (cumulative >= rank)
            return minDb + ((float) b + 0.5f) * binWidthDb;
    }
    return maxDb;
}

void AdaptiveNormalizer::updateTargets() noexcept
{
    if (ringCount == 0)
        return;

    auto q20 = quantile (0.20f);
    auto q95 = quantile (0.95f);
    targetUpper = q95 + 3.0f;
    targetSpan = std::clamp (targetUpper - q20, minSpan, maxSpan);
}

float AdaptiveNormalizer::process (float db, bool gateOpen, float dt) noexcept
{
    if (! std::isfinite (db))
        db = minDb;

    const bool frozen = (mode == NormalizerMode::locked && calibrated);

    if (gateOpen && ! frozen)
    {
        admitClock += dt;
        while (admitClock >= admitInterval)
        {
            admit (db);
            admitClock -= admitInterval;
        }

        statsClock += dt;
        if (statsClock >= statsInterval)
        {
            statsClock = 0.0f;
            updateTargets();
        }

        if (! calibrated && (float) ringCount * admitInterval >= calibrationSeconds)
            calibrated = true;

        if (calibrated)
        {
            // Blend from the fixed startup reference to the learned anchors
            // over blendSeconds, then track them with the asymmetric rule.
            blendClock = std::min (blendSeconds, blendClock + dt);
            auto blend = blendClock / blendSeconds;
            auto desiredUpper = fixedUpperDb + (targetUpper - fixedUpperDb) * blend;
            auto desiredSpan = fixedSpanDb + (targetSpan - fixedSpanDb) * blend;

            if (desiredUpper > upper)
            {
                auto a = smoothingCoefficient (dt, riseTau);
                upper = a * upper + (1.0f - a) * desiredUpper;
            }
            else
            {
                upper = std::max (desiredUpper, upper - maxFallDbPerSecond * dt);
            }

            auto s = smoothingCoefficient (dt, spanTau);
            span = std::clamp (s * span + (1.0f - s) * desiredSpan, minSpan, maxSpan);
        }
    }

    if (! gateOpen)
        return 0.0f;

    return std::clamp ((db - (upper - span)) / span, 0.0f, 1.0f);
}

} // namespace vj
