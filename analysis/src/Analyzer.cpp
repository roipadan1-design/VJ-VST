#include "vj/Analyzer.h"

#include <algorithm>
#include <cmath>

namespace vj
{

namespace
{
    constexpr float silenceDb = -160.0f;
    constexpr float gateOpenDb = -60.0f, gateCloseDb = -66.0f, gateHoldSeconds = 0.2f;
    constexpr double bandEdgesHz[numBands + 1] = { 20.0, 60.0, 150.0, 400.0, 2000.0, 6000.0, 16000.0 };
    constexpr float whitenFloor = 1.0e-4f;   // absolute amplitude floor: bounded whitening gain
    constexpr float whitenRelativeFloor = 3.16e-3f; // -50 dB under the frame's loudest bin
    constexpr float whitenMemorySeconds = 3.0f;
    constexpr float logCompression = 20.0f;

    float coefficient (double dtSeconds, double tauSeconds) noexcept
    {
        return tauSeconds <= 0.0 ? 0.0f : (float) std::exp (-dtSeconds / tauSeconds);
    }

    float attackRelease (float current, float target, float attackCoeff, float releaseCoeff) noexcept
    {
        auto a = target > current ? attackCoeff : releaseCoeff;
        return a * current + (1.0f - a) * target;
    }

    float powerToDb (float p) noexcept
    {
        return p > 1.0e-16f ? 10.0f * std::log10 (p) : silenceDb;
    }

    float absoluteActivity (float db) noexcept
    {
        return std::clamp ((db + 60.0f) / 48.0f, 0.0f, 1.0f);
    }

    float logMap (double hz, double lo, double hi) noexcept
    {
        if (hz <= lo) return 0.0f;
        return (float) std::clamp (std::log (hz / lo) / std::log (hi / lo), 0.0, 1.0);
    }
}

void Analyzer::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;

    // 44.1/48 kHz plan, scaled for the 88.2/96 and 176.4/192 families so the
    // windows keep roughly the same duration and frequency resolution.
    scale = sampleRate > 150000.0 ? 4 : (sampleRate > 60000.0 ? 2 : 1);
    shortSize = 1024 * scale; shortHop = 128 * scale;
    mainSize = 2048 * scale;  mainHop = 256 * scale;

    int ringSize = 1;
    while (ringSize < mainSize)
        ringSize <<= 1;

    ringL.assign ((size_t) ringSize, 0.0f);
    ringR.assign ((size_t) ringSize, 0.0f);
    ringMask = ringSize - 1;

    shortFft = std::make_unique<Fft> (shortSize);
    mainFft = std::make_unique<Fft> (mainSize);
    shortWindow = makePeriodicHann (shortSize);
    mainWindow = makePeriodicHann (mainSize);

    shortWindowPower = 0.0f;
    for (auto w : shortWindow) shortWindowPower += w * w;
    mainWindowPower = 0.0f;
    for (auto w : mainWindow) mainWindowPower += w * w;

    fftBuffer.assign ((size_t) mainSize, {});
    shortPower.assign ((size_t) (shortSize / 2 + 1), 0.0f);
    mainPower.assign ((size_t) (mainSize / 2 + 1), 0.0f);

    whitenPeak.assign (shortPower.size(), whitenFloor);
    logMag.assign (shortPower.size(), 0.0f);
    prevLogMag.assign (shortPower.size(), 0.0f);

    auto nyquist = sampleRate * 0.5;
    auto shortBinHz = sampleRate / shortSize;
    auto binFor = [&] (double hz) { return std::clamp ((int) std::lround (hz / shortBinHz), 1, shortSize / 2 - 1); };

    regionBins[(size_t) OnsetRegion::bass] = { binFor (30.0), binFor (160.0) };
    regionBins[(size_t) OnsetRegion::mid]  = { binFor (200.0), binFor (2500.0) };
    regionBins[(size_t) OnsetRegion::high] = { binFor (5000.0), binFor (std::min (16000.0, nyquist * 0.95)) };
    fullRangeBins = { binFor (30.0), binFor (std::min (16000.0, nyquist * 0.95)) };

    auto shortHopSeconds = (float) (shortHop / sampleRate);
    auto hopsPerSecond = (int) std::lround (sampleRate / shortHop);
    // Bass fires on the rise (~10 ms before its peak), so its window is 10 ms longer.
    const float refractory[3] = { 0.10f, 0.07f, 0.045f };

    for (int r = 0; r < 3; ++r)
    {
        OnsetTracker::Settings s;
        s.refractorySeconds = refractory[r];
        // Floors calibrated with analysis_tests --stats (steady noise, vibrato:
        // no events) and analyze_wav on a mastered track (kicks only rise
        // 3-5 dB over the bass line there). Bass is in different units:
        // 0.15 = a 3 dB low-band rise over the slow envelope.
        const float floors[3] = { 0.15f, 0.07f, 0.05f };
        s.absoluteFloor = floors[r];
        s.fireOnRise = r == (int) OnsetRegion::bass; // kicks: fire on the rise, 5-10 ms sooner
        trackers[(size_t) r].prepare (hopsPerSecond, shortHopSeconds, s);
    }

    for (int b = 0; b < numBands; ++b)
        bandWeights[(size_t) b] = makeBandWeights (bandEdgesHz[b], std::min (bandEdgesHz[b + 1], nyquist), mainSize);

    auto specLo = 30.0, specHi = std::min (16000.0, nyquist);
    for (int b = 0; b < numSpectrumBands; ++b)
    {
        auto lo = specLo * std::pow (specHi / specLo, (double) b / numSpectrumBands);
        auto hi = specLo * std::pow (specHi / specLo, (double) (b + 1) / numSpectrumBands);
        spectrumWeights[(size_t) b] = makeBandWeights (lo, hi, mainSize);
    }

    auto mainBinHz = sampleRate / mainSize;
    descriptorFirstBin = std::max (1, (int) std::ceil (50.0 / mainBinHz));
    descriptorLastBin = std::min (mainSize / 2 - 1, (int) std::floor (std::min (16000.0, nyquist) / mainBinHz));

    meanCoeff = coefficient (1.0 / sampleRate, 0.010);
    lowPassL.design (160.0, sampleRate);
    lowPassR.design (160.0, sampleRate);
    lowFastCoeff = coefficient (1.0 / sampleRate, 0.012);
    lowSlowCoeff = coefficient (1.0 / sampleRate, 0.150);
    envRelease = coefficient (1.0 / sampleRate, 0.120);
    peakRelease = coefficient (1.0 / sampleRate, 0.300);

    reset();
}

void Analyzer::LowPass::design (double cutoffHz, double sr)
{
    // RBJ biquad low-pass, Q = 1/sqrt(2).
    const double w0 = 2.0 * 3.14159265358979323846 * cutoffHz / sr;
    const double alpha = std::sin (w0) / (2.0 * 0.70710678118654752);
    const double cosw = std::cos (w0);
    const double a0 = 1.0 + alpha;
    b0 = (float) ((1.0 - cosw) * 0.5 / a0);
    b1 = (float) ((1.0 - cosw) / a0);
    b2 = b0;
    a1 = (float) (-2.0 * cosw / a0);
    a2 = (float) ((1.0 - alpha) / a0);
    z1 = z2 = 0.0f;
}

void Analyzer::reset()
{
    std::fill (ringL.begin(), ringL.end(), 0.0f);
    std::fill (ringR.begin(), ringR.end(), 0.0f);
    ringWrite = 0;
    sampleCounter = 0;
    samplesToShortHop = shortHop;
    samplesToMainHop = mainHop;

    meanPower = envPower = envPeak = 0.0f;
    lowPassL.z1 = lowPassL.z2 = lowPassR.z1 = lowPassR.z2 = 0.0f;
    lowFast = lowSlow = 0.0f;
    gateOpen = false;
    gateHoldRemaining = 0.0f;

    std::fill (whitenPeak.begin(), whitenPeak.end(), whitenFloor);
    std::fill (prevLogMag.begin(), prevLogMag.end(), 0.0f);
    havePrevShort = false;
    shortFrameIndex = 0;
    fluxAverage = fluxSmoothed = 0.0f;

    for (auto& t : trackers) t.reset();
    for (auto& n : bandNormalizers) n.reset();
    for (auto& n : aggregateNormalizers) n.reset();
    levelNormalizer.reset();

    bandPowerSmoothed.fill (0.0f);
    spectrumPowerSmoothed.fill (0.0f);
    centroidSmoothed = flatnessSmoothed = rolloffSmoothed = 0.0f;
    trendFastDb = trendSlowDb = -100.0f;
    levelRelOut = 0.0f;
    bandRelOut.fill (0.0f);
    aggregateRelOut.fill (0.0f);
}

void Analyzer::setNormalizerMode (NormalizerMode m) noexcept
{
    levelNormalizer.setMode (m);
    for (auto& n : bandNormalizers) n.setMode (m);
    for (auto& n : aggregateNormalizers) n.setMode (m);
}

void Analyzer::setOnsetSensitivity (float s) noexcept
{
    for (auto& t : trackers) t.setSensitivity (s);
}

void Analyzer::setInputTrimDb (float db) noexcept
{
    trimGain = std::pow (10.0f, db / 20.0f);
}

Analyzer::BandWeights Analyzer::makeBandWeights (double loHz, double hiHz, int fftSize) const
{
    // Bin k covers [(k - 0.5) * df, (k + 0.5) * df). Fractional overlap weights
    // mean a band edge falling inside a bin contributes only its share,
    // instead of the whole bin landing in one band or the other.
    BandWeights weights;
    auto df = sampleRate / fftSize;

    for (int k = 1; k < fftSize / 2; ++k)
    {
        auto binLo = (k - 0.5) * df, binHi = (k + 0.5) * df;
        auto overlap = std::min (binHi, hiHz) - std::max (binLo, loHz);
        if (overlap > 0.0)
            weights.push_back ({ k, (float) (overlap / df) });
    }

    return weights;
}

float Analyzer::integrate (const std::vector<float>& power, const BandWeights& w) noexcept
{
    float sum = 0.0f;
    for (auto& wb : w)
        sum += power[(size_t) wb.bin] * wb.weight;
    return sum;
}

void Analyzer::computeSpectrum (int fftSize, const std::vector<float>& window, float windowPowerSum,
                                std::vector<float>& powerOut)
{
    // Pack left into the real part and right into the imaginary part: one
    // complex FFT yields both channel spectra (separated below), so stereo
    // costs the same as mono. Channel powers are averaged, never the
    // waveforms - anti-phase content must not cancel out of the analysis.
    auto* data = fftBuffer.data();
    auto start = ringWrite - fftSize;

    for (int i = 0; i < fftSize; ++i)
    {
        auto idx = (start + i) & ringMask;
        auto w = window[(size_t) i];
        data[i] = { ringL[(size_t) idx] * w, stereo ? ringR[(size_t) idx] * w : 0.0f };
    }

    (fftSize == shortSize ? *shortFft : *mainFft).transform (data);

    // One-sided energy calibration: P[k] = s[k] * (|XL|^2 + |XR|^2) / (channels * N * sum(w^2)),
    // s = 1 at DC/Nyquist and 2 elsewhere. A full-scale sine reads ~-3.01 dB.
    const auto channels = stereo ? 2.0f : 1.0f;
    const auto norm = 1.0f / (channels * (float) fftSize * windowPowerSum);
    const auto half = fftSize / 2;

    for (int k = 0; k <= half; ++k)
    {
        auto zk = data[k];
        auto zn = std::conj (data[(fftSize - k) & (fftSize - 1)]);
        auto xl = (zk + zn) * 0.5f;
        auto xr = (zk - zn) * std::complex<float> (0.0f, -0.5f);
        auto s = (k == 0 || k == half) ? 1.0f : 2.0f;
        powerOut[(size_t) k] = s * (std::norm (xl) + std::norm (xr)) * norm;
    }
}

void Analyzer::process (const float* left, const float* right, int numSamples, Listener& listener)
{
    stereo = (right != nullptr);

    for (int i = 0; i < numSamples; ++i)
    {
        auto l = left[i] * trimGain;
        auto r = stereo ? right[i] * trimGain : l;
        if (! std::isfinite (l)) l = 0.0f;
        if (! std::isfinite (r)) r = 0.0f;

        ringL[(size_t) ringWrite] = l;
        ringR[(size_t) ringWrite] = r;
        ringWrite = (ringWrite + 1) & ringMask;
        ++sampleCounter;

        auto power = stereo ? 0.5f * (l * l + r * r) : l * l;
        // Mean power first (symmetric 10 ms), then a release-only follower: an
        // asymmetric follower on raw power would ride every waveform peak and
        // read a sine ~2.5 dB hot.
        meanPower = meanCoeff * meanPower + (1.0f - meanCoeff) * power;
        envPower = meanPower > envPower ? meanPower : envRelease * envPower + (1.0f - envRelease) * meanPower;
        auto lowL = lowPassL.process (l);
        auto lowPower = stereo ? 0.5f * (lowL * lowL + std::pow (lowPassR.process (r), 2.0f)) : lowL * lowL;
        lowFast = lowFastCoeff * lowFast + (1.0f - lowFastCoeff) * lowPower;
        lowSlow = lowSlowCoeff * lowSlow + (1.0f - lowSlowCoeff) * lowPower;

        auto peak = std::max (std::abs (l), std::abs (r));
        envPeak = peak > envPeak ? peak : peakRelease * envPeak + (1.0f - peakRelease) * peak;

        if (--samplesToShortHop == 0)
        {
            samplesToShortHop = shortHop;
            if (sampleCounter >= shortSize)
                runShortHop (listener);
        }

        if (--samplesToMainHop == 0)
        {
            samplesToMainHop = mainHop;
            runMainHop (listener);
        }
    }
}

void Analyzer::runShortHop (Listener& listener)
{
    computeSpectrum (shortSize, shortWindow, shortWindowPower, shortPower);

    const auto dt = (double) shortHop / sampleRate;
    const auto memory = coefficient (dt, whitenMemorySeconds);

    // Adaptive whitening (bounded by an amplitude floor) then log compression:
    // a sustained tone settles near a constant whitened value and stops
    // producing flux; a new attack stands out regardless of its absolute level.
    // The floor is relative to the frame's loudest bin: leakage and sidelobes
    // 50 dB down must not be amplified into "events".
    float loudest = 0.0f;
    for (int k = fullRangeBins.first; k <= fullRangeBins.second; ++k)
        loudest = std::max (loudest, shortPower[(size_t) k]);
    const auto floor = std::max (whitenFloor, std::sqrt (loudest) * whitenRelativeFloor);

    for (int k = fullRangeBins.first; k <= fullRangeBins.second; ++k)
    {
        auto mag = std::sqrt (shortPower[(size_t) k]);
        auto& peak = whitenPeak[(size_t) k];
        peak = std::max ({ mag, floor, memory * peak });
        logMag[(size_t) k] = std::log1p (logCompression * mag / peak);
    }

    const auto frame = shortFrameIndex++;

    if (! havePrevShort)
    {
        std::copy (logMag.begin(), logMag.end(), prevLogMag.begin());
        havePrevShort = true;
        return;
    }

    // SuperFlux-inspired: compare each bin against the maximum of its
    // neighbours in the previous frame, so small pitch movement (vibrato,
    // glides) does not register as a new event.
    auto regionNovelty = [&] (int first, int last) {
        float sum = 0.0f;
        for (int k = first; k <= last; ++k)
        {
            auto lo = std::max (fullRangeBins.first, k - 1);
            auto hi = std::min (fullRangeBins.second, k + 1);
            auto reference = *std::max_element (prevLogMag.begin() + lo, prevLogMag.begin() + hi + 1);
            sum += std::max (0.0f, logMag[(size_t) k] - reference);
        }
        return sum / (float) std::max (1, last - first + 1);
    };

    for (int r = 0; r < 3; ++r)
    {
        auto [first, last] = regionBins[(size_t) r];
        // Bass: fast/slow low-band power ratio, 20 dB rise = 1.0.
        auto nov = r == (int) OnsetRegion::bass
                     ? std::max (0.0f, powerToDb (lowFast) - powerToDb (lowSlow + 1.0e-9f)) / 20.0f
                     : regionNovelty (first, last);
        auto detection = trackers[(size_t) r].push (nov, frame, gateOpen);
        listener.novelty ((OnsetRegion) r, nov, trackers[(size_t) r].getLastThreshold());

        if (detection.fired)
        {
            OnsetEvent e;
            e.region = (OnsetRegion) r;
            e.strength = detection.strength;
            e.sampleIndex = detection.frame * shortHop + shortSize;
            listener.onset (e);
        }
    }

    // Broadband flux for the flux01 descriptor, relative to its own slow average.
    auto broad = regionNovelty (fullRangeBins.first, fullRangeBins.second);
    auto avgCoeff = coefficient (dt, 2.0);
    fluxAverage = avgCoeff * fluxAverage + (1.0f - avgCoeff) * broad;
    auto target = gateOpen ? std::clamp (broad / std::max (1.0e-4f, 3.0f * fluxAverage), 0.0f, 1.0f) : 0.0f;
    fluxSmoothed = attackRelease (fluxSmoothed, target, coefficient (dt, 0.02), coefficient (dt, 0.2));

    std::swap (logMag, prevLogMag);
}

void Analyzer::runMainHop (Listener& listener)
{
    const auto dt = (double) mainHop / sampleRate;
    const auto dtf = (float) dt;

    FeatureFrame f;
    f.sampleIndex = sampleCounter;
    f.sampleRate = sampleRate;
    f.levelDb = powerToDb (envPower);
    f.peakDb = envPeak > 1.0e-8f ? 20.0f * std::log10 (envPeak) : silenceDb;

    // Gate with hysteresis and hold.
    if (f.levelDb > gateOpenDb)
    {
        gateOpen = true;
        gateHoldRemaining = gateHoldSeconds;
    }
    else if (f.levelDb < gateCloseDb && gateOpen)
    {
        gateHoldRemaining -= dtf;
        if (gateHoldRemaining <= 0.0f)
            gateOpen = false;
    }
    f.gateOpen = gateOpen;

    const auto relRelease = coefficient (dt, 0.12);
    auto publishRel = [&] (float& out, float rel) {
        out = rel > out ? rel : relRelease * out + (1.0f - relRelease) * rel;
        return out;
    };

    f.levelRel = publishRel (levelRelOut, levelNormalizer.process (f.levelDb, gateOpen, dtf));
    f.levelAbs = gateOpen ? absoluteActivity (f.levelDb) : 0.0f;
    f.calibrated = levelNormalizer.isCalibrated();

    const auto trendFast = coefficient (dt, 0.5), trendSlow = coefficient (dt, 4.0);
    auto trendInput = std::max (f.levelDb, -100.0f);
    trendFastDb = trendFast * trendFastDb + (1.0f - trendFast) * trendInput;
    trendSlowDb = trendSlow * trendSlowDb + (1.0f - trendSlow) * trendInput;
    f.energyTrend = std::clamp ((trendFastDb - trendSlowDb) / 12.0f, -1.0f, 1.0f);

    if (sampleCounter < mainSize)
    {
        listener.featureFrame (f);
        return;
    }

    computeSpectrum (mainSize, mainWindow, mainWindowPower, mainPower);

    const auto bandAttack = coefficient (dt, 0.010), bandRelease = coefficient (dt, 0.150);
    std::array<float, 3> aggregatePower {};

    for (int b = 0; b < numBands; ++b)
    {
        auto p = integrate (mainPower, bandWeights[(size_t) b]);
        auto& s = bandPowerSmoothed[(size_t) b];
        s = attackRelease (s, p, bandAttack, bandRelease);
        aggregatePower[(size_t) (b / 2)] += s;

        auto db = powerToDb (s);
        f.bandDb[(size_t) b] = db;
        f.bandAbs[(size_t) b] = gateOpen ? absoluteActivity (db) : 0.0f;
        auto rel = bandNormalizers[(size_t) b].process (db, gateOpen && db > -90.0f, dtf);
        f.bandRel[(size_t) b] = publishRel (bandRelOut[(size_t) b], rel);
    }

    for (int a = 0; a < 3; ++a)
    {
        auto db = powerToDb (aggregatePower[(size_t) a]);
        f.aggregateAbs[(size_t) a] = gateOpen ? absoluteActivity (db) : 0.0f;
        auto rel = aggregateNormalizers[(size_t) a].process (db, gateOpen && db > -90.0f, dtf);
        f.aggregateRel[(size_t) a] = publishRel (aggregateRelOut[(size_t) a], rel);
    }

    // 32 log bands, shown relative to the broadband anchors: a band holding
    // ~1/32 of the energy sits ~15 dB under the full level, hence the offset.
    const auto specAttack = coefficient (dt, 0.010), specRelease = coefficient (dt, 0.200);
    auto upper = levelNormalizer.getUpperAnchorDb(), span = levelNormalizer.getSpanDb();

    for (int b = 0; b < numSpectrumBands; ++b)
    {
        auto& s = spectrumPowerSmoothed[(size_t) b];
        s = attackRelease (s, integrate (mainPower, spectrumWeights[(size_t) b]), specAttack, specRelease);
        f.spectrum[(size_t) b] = gateOpen ? std::clamp ((powerToDb (s) + 15.0f - (upper - span)) / span, 0.0f, 1.0f) : 0.0f;
    }

    // Descriptors (DC and sub-50 Hz excluded).
    double total = 0.0, weighted = 0.0, logSum = 0.0;
    const auto binHz = sampleRate / mainSize;
    const auto count = descriptorLastBin - descriptorFirstBin + 1;

    for (int k = descriptorFirstBin; k <= descriptorLastBin; ++k)
    {
        auto p = (double) mainPower[(size_t) k];
        total += p;
        weighted += p * k * binHz;
        logSum += std::log (p + 1.0e-12);
    }

    if (gateOpen && total > 1.0e-12)
    {
        auto centroidHz = weighted / total;
        auto flatness = std::exp (logSum / count) / (total / count + 1.0e-12);

        double cumulative = 0.0, rolloffHz = descriptorLastBin * binHz;
        for (int k = descriptorFirstBin; k <= descriptorLastBin; ++k)
        {
            cumulative += mainPower[(size_t) k];
            if (cumulative >= 0.85 * total)
            {
                rolloffHz = k * binHz;
                break;
            }
        }

        auto c = coefficient (dt, 0.15);
        centroidSmoothed = c * centroidSmoothed + (1.0f - c) * logMap (centroidHz, 200.0, 8000.0);
        auto fl = coefficient (dt, 0.25);
        flatnessSmoothed = fl * flatnessSmoothed + (1.0f - fl) * (float) std::clamp (flatness, 0.0, 1.0);
        auto ro = coefficient (dt, 0.2);
        rolloffSmoothed = ro * rolloffSmoothed + (1.0f - ro) * logMap (rolloffHz, 200.0, 16000.0);
    }

    f.centroid01 = centroidSmoothed;
    f.flatness = flatnessSmoothed;
    f.rolloff01 = rolloffSmoothed;
    f.flux01 = fluxSmoothed;

    listener.featureFrame (f);
}

} // namespace vj
