#pragma once

#include "vj/AdaptiveNormalizer.h"
#include "vj/Fft.h"
#include "vj/OnsetTracker.h"

#include <array>
#include <complex>
#include <cstdint>
#include <memory>
#include <vector>

namespace vj
{

constexpr int numBands = 6;       // 20-60-150-400-2k-6k-16k Hz
constexpr int numSpectrumBands = 32;

enum class Aggregate { bass = 0, mid = 1, high = 2 }; // bands 0-1, 2-3, 4-5
enum class OnsetRegion { bass = 0, mid = 1, high = 2 };

// One analysis snapshot, published every main hop (256 samples at 48 kHz).
// "rel" = adaptive relative activity (AdaptiveNormalizer), "abs" = fixed
// calibrated mapping clamp((dBFS + 60) / 48) - presets use rel for texture and
// motion, abs/energyTrend for section contrast (REPORT C4).
struct FeatureFrame
{
    int64_t sampleIndex = 0; // newest analysed sample (exclusive end of window)
    double sampleRate = 48000.0;

    float levelDb = -100.0f, peakDb = -100.0f;
    float levelRel = 0.0f, levelAbs = 0.0f;
    bool gateOpen = false, calibrated = false;

    std::array<float, numBands> bandDb {}, bandRel {}, bandAbs {};
    std::array<float, 3> aggregateRel {}, aggregateAbs {};
    std::array<float, numSpectrumBands> spectrum {};

    float centroid01 = 0.0f, flatness = 0.0f, rolloff01 = 0.0f, flux01 = 0.0f;
    float energyTrend = 0.0f; // -1..1, pre-normalisation 0.5 s vs 4 s level slope
};

struct OnsetEvent
{
    int64_t sampleIndex = 0; // end of the analysis window that confirmed it
    OnsetRegion region = OnsetRegion::bass;
    float strength = 0.0f;
};

// Host-independent audio analyser (ENGINEERING_SPEC 4). Not real-time safe to
// call from an audio callback (FFTs, percentile work) - the host pushes audio
// through an SpscFifo and calls process() from a worker thread. No allocation
// after prepare().
class Analyzer
{
public:
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void featureFrame (const FeatureFrame&) = 0;
        virtual void onset (const OnsetEvent&) = 0;

        // Diagnostics: per-region novelty and its adaptive threshold every short hop.
        virtual void novelty (OnsetRegion, float /*novelty*/, float /*threshold*/) {}
    };

    void prepare (double sampleRate);
    void reset();

    void setNormalizerMode (NormalizerMode m) noexcept;
    void setOnsetSensitivity (float s) noexcept;     // 1 = default, higher = more hits
    void setInputTrimDb (float db) noexcept;         // analysis only, never the audio path

    // right may be nullptr for mono input.
    void process (const float* left, const float* right, int numSamples, Listener& listener);

    double getSampleRate() const noexcept { return sampleRate; }
    int64_t getSamplesProcessed() const noexcept { return sampleCounter; }

private:
    struct WeightedBin { int bin; float weight; };
    using BandWeights = std::vector<WeightedBin>;

    void runShortHop (Listener&);
    void runMainHop (Listener&);
    void computeSpectrum (int fftSize, const std::vector<float>& window, float windowPowerSum,
                          std::vector<float>& powerOut);
    BandWeights makeBandWeights (double loHz, double hiHz, int fftSize) const;
    static float integrate (const std::vector<float>& power, const BandWeights& w) noexcept;

    double sampleRate = 48000.0;
    int scale = 1;
    int shortSize = 1024, shortHop = 128, mainSize = 2048, mainHop = 256;

    // Input history (power-of-two ring, >= mainSize).
    std::vector<float> ringL, ringR;
    int ringMask = 0, ringWrite = 0;
    bool stereo = false;
    int64_t sampleCounter = 0;
    int samplesToShortHop = 0, samplesToMainHop = 0;
    float trimGain = 1.0f;

    // Time-domain envelopes.
    float meanPower = 0.0f, envPower = 0.0f, envPeak = 0.0f;
    float meanCoeff = 0.0f, envRelease = 0.0f, peakRelease = 0.0f;

    // Bass onsets come from the time domain: a 160 Hz low-pass band, fast
    // (12 ms) vs slow (150 ms) mean power. A 1024-point FFT has only ~3 bins
    // below 160 Hz - far too few for a stable spectral-flux statistic.
    struct LowPass
    {
        float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
        void design (double cutoffHz, double sampleRate);
        float process (float x) noexcept
        {
            auto y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }
    };
    LowPass lowPassL, lowPassR;
    float lowFast = 0.0f, lowSlow = 0.0f, lowFastCoeff = 0.0f, lowSlowCoeff = 0.0f;

    // Gate (-60 open / -66 close, 200 ms hold).
    bool gateOpen = false;
    float gateHoldRemaining = 0.0f;

    std::unique_ptr<Fft> shortFft, mainFft;
    std::vector<float> shortWindow, mainWindow;
    float shortWindowPower = 1.0f, mainWindowPower = 1.0f;
    std::vector<std::complex<float>> fftBuffer;
    std::vector<float> shortPower, mainPower;

    // Onsets: whitened log-magnitude SuperFlux-style novelty per region.
    std::vector<float> whitenPeak, logMag, prevLogMag;
    std::array<std::pair<int, int>, 3> regionBins {}; // [first, last] bin, inclusive
    std::pair<int, int> fullRangeBins { 1, 1 };
    std::array<OnsetTracker, 3> trackers;
    int64_t shortFrameIndex = 0;
    float fluxAverage = 0.0f, fluxSmoothed = 0.0f;
    bool havePrevShort = false;

    // Bands / spectrum / descriptors.
    std::array<BandWeights, numBands> bandWeights;
    std::array<BandWeights, numSpectrumBands> spectrumWeights;
    std::array<float, numBands> bandPowerSmoothed {};
    std::array<float, numSpectrumBands> spectrumPowerSmoothed {};
    std::array<AdaptiveNormalizer, numBands> bandNormalizers;
    std::array<AdaptiveNormalizer, 3> aggregateNormalizers;
    AdaptiveNormalizer levelNormalizer;
    int descriptorFirstBin = 1, descriptorLastBin = 1;
    float centroidSmoothed = 0.0f, flatnessSmoothed = 0.0f, rolloffSmoothed = 0.0f;
    float trendFastDb = -100.0f, trendSlowDb = -100.0f;

    // Release-only smoothing of the published relative values, so a gate
    // closing fades activity out instead of stepping it to zero.
    float levelRelOut = 0.0f;
    std::array<float, numBands> bandRelOut {};
    std::array<float, 3> aggregateRelOut {};
};

} // namespace vj
