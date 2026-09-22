// AnalysisCore acceptance tests (ENGINEERING_SPEC 11, Phase 1 fixtures),
// run on synthetic signals so they need no audio files, no DAW and no GPU.
//
//   analysis_tests            run everything, exit code = number of failures

#include "vj/Analyzer.h"
#include "vj/Protocol.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <numeric>
#include <random>
#include <string>
#include <vector>

namespace
{

constexpr double sr = 48000.0;
constexpr double pi = 3.14159265358979323846;
int failures = 0;

void check (bool condition, const std::string& what)
{
    std::printf ("  [%s] %s\n", condition ? "PASS" : "FAIL", what.c_str());
    if (! condition)
        ++failures;
}

struct Capture : vj::Analyzer::Listener
{
    std::vector<vj::FeatureFrame> frames;
    std::vector<vj::OnsetEvent> onsets;
    void featureFrame (const vj::FeatureFrame& f) override { frames.push_back (f); }
    void onset (const vj::OnsetEvent& e) override { onsets.push_back (e); }

    int countOnsets (vj::OnsetRegion r, double fromSec, double toSec) const
    {
        return (int) std::count_if (onsets.begin(), onsets.end(), [&] (auto& e) {
            auto t = e.sampleIndex / sr;
            return e.region == r && t >= fromSec && t < toSec;
        });
    }

    double mean (double fromSec, double toSec, const std::function<float (const vj::FeatureFrame&)>& get) const
    {
        double sum = 0.0;
        int n = 0;
        for (auto& f : frames)
        {
            auto t = f.sampleIndex / sr;
            if (t >= fromSec && t < toSec) { sum += get (f); ++n; }
        }
        return n > 0 ? sum / n : 0.0;
    }
};

struct Signal
{
    std::vector<float> left, right;
    explicit Signal (double seconds) : left ((size_t) (seconds * sr), 0.0f), right (left.size(), 0.0f) {}
    size_t size() const { return left.size(); }
};

// Pitched-sine kick: 150 -> 48 Hz sweep, ~120 ms amplitude decay.
void addKick (Signal& s, double atSec, float gain)
{
    auto start = (size_t) (atSec * sr);
    double phase = 0.0;
    for (size_t i = 0; i < (size_t) (0.4 * sr) && start + i < s.size(); ++i)
    {
        auto t = i / sr;
        auto freq = 48.0 + 102.0 * std::exp (-t / 0.03);
        phase += 2.0 * pi * freq / sr;
        auto v = (float) (gain * std::sin (phase) * std::exp (-t / 0.12));
        s.left[start + i] += v;
        s.right[start + i] += v;
    }
}

// Hi-hat: short high-passed noise burst.
void addHat (Signal& s, double atSec, float gain, std::mt19937& rng)
{
    std::normal_distribution<float> noise (0.0f, 1.0f);
    auto start = (size_t) (atSec * sr);
    float prev = 0.0f;
    for (size_t i = 0; i < (size_t) (0.05 * sr) && start + i < s.size(); ++i)
    {
        auto n = noise (rng);
        auto hp = n - prev; // crude first-difference high-pass
        prev = n;
        auto v = gain * 0.5f * hp * (float) std::exp (-(i / sr) / 0.012);
        s.left[start + i] += v;
        s.right[start + i] += v;
    }
}

void addNoise (Signal& s, double fromSec, double toSec, float gain, std::mt19937& rng)
{
    std::normal_distribution<float> noise (0.0f, 1.0f);
    for (auto i = (size_t) (fromSec * sr); i < (size_t) (toSec * sr) && i < s.size(); ++i)
    {
        s.left[i] += gain * noise (rng);
        s.right[i] += gain * noise (rng);
    }
}

void addSine (Signal& s, double freq, float gain, double fromSec, double toSec, double vibratoHz = 0.0, double vibratoCents = 0.0)
{
    double phase = 0.0;
    for (auto i = (size_t) (fromSec * sr); i < (size_t) (toSec * sr) && i < s.size(); ++i)
    {
        auto t = i / sr;
        auto f = freq * std::pow (2.0, vibratoCents / 1200.0 * std::sin (2.0 * pi * vibratoHz * t));
        phase += 2.0 * pi * f / sr;
        auto v = gain * (float) std::sin (phase);
        s.left[i] += v;
        s.right[i] += v;
    }
}

// A simple 120 BPM groove: kick on every beat, hats on 8th off-beats, a
// quiet noise bed so the gate stays open between hits.
Signal makeGroove (double seconds, float gain, std::mt19937& rng, std::vector<double>* kickTimes = nullptr)
{
    Signal s (seconds);
    addNoise (s, 0.0, seconds, 0.003f * gain, rng);
    for (double t = 0.25; t < seconds - 0.5; t += 0.5)
    {
        addKick (s, t, 0.8f * gain);
        addHat (s, t + 0.25, 0.25f * gain, rng);
        if (kickTimes) kickTimes->push_back (t);
    }
    return s;
}

Capture run (const Signal& s, bool mono = false, std::function<void (vj::Analyzer&)> configure = {})
{
    vj::Analyzer analyzer;
    analyzer.prepare (sr);
    if (configure) configure (analyzer);

    Capture capture;
    const int block = 512;
    for (size_t pos = 0; pos < s.size(); pos += block)
    {
        auto n = (int) std::min<size_t> (block, s.size() - pos);
        analyzer.process (s.left.data() + pos, mono ? nullptr : s.right.data() + pos, n, capture);
    }
    return capture;
}

void testSilence()
{
    std::puts ("Silence (10 s)");
    Signal s (10.0);
    auto c = run (s);
    auto maxRel = 0.0f;
    for (auto& f : c.frames)
        maxRel = std::max ({ maxRel, f.levelRel, f.aggregateRel[0], f.aggregateRel[1], f.aggregateRel[2] });
    check (c.onsets.empty(), "no onset events (" + std::to_string (c.onsets.size()) + ")");
    check (maxRel == 0.0f, "relative activity stays at zero");
    check (std::none_of (c.frames.begin(), c.frames.end(), [] (auto& f) { return f.gateOpen; }), "gate never opens");
}

void testSineCalibration()
{
    std::puts ("Full-scale 1 kHz sine calibration");
    Signal s (3.0);
    addSine (s, 1000.0, 1.0f, 0.0, 3.0);
    auto c = run (s);
    auto& f = c.frames.back();
    char buf[160];
    std::snprintf (buf, sizeof buf, "RMS level ~ -3.01 dBFS (got %.2f)", f.levelDb);
    check (std::abs (f.levelDb + 3.01f) < 0.3f, buf);
    std::snprintf (buf, sizeof buf, "400-2000 Hz band holds the energy (got %.2f dB)", f.bandDb[3]);
    check (std::abs (f.bandDb[3] + 3.01f) < 0.5f, buf);
    std::snprintf (buf, sizeof buf, "neighbouring bands are >40 dB lower (%.1f / %.1f dB)", f.bandDb[2], f.bandDb[4]);
    check (f.bandDb[2] < -43.0f && f.bandDb[4] < -43.0f, buf);
}

void testKickDetection()
{
    std::puts ("Kick groove at 120 BPM (20 s)");
    std::mt19937 rng (1);
    std::vector<double> kicks;
    auto s = makeGroove (20.0, 1.0f, rng, &kicks);
    auto c = run (s);

    int expected = 0, matched = 0;
    double worstLate = 0.0;
    for (auto k : kicks)
    {
        if (k < 3.0) continue; // warm-up
        ++expected;
        for (auto& e : c.onsets)
        {
            auto dt = e.sampleIndex / sr - k;
            if (e.region == vj::OnsetRegion::bass && dt > -0.01 && dt < 0.06)
            {
                ++matched;
                worstLate = std::max (worstLate, dt);
                break;
            }
        }
    }

    auto bassTotal = c.countOnsets (vj::OnsetRegion::bass, 3.0, 20.0);
    char buf[160];
    std::snprintf (buf, sizeof buf, "detects >= 95%% of kicks (%d / %d)", matched, expected);
    check (matched >= expected * 95 / 100, buf);
    std::snprintf (buf, sizeof buf, "no more than 10%% extra bass events (%d events for %d kicks)", bassTotal, expected);
    check (bassTotal <= expected * 110 / 100, buf);
    std::snprintf (buf, sizeof buf, "confirmation latency under 60 ms (worst %.1f ms)", worstLate * 1000.0);
    check (worstLate < 0.06, buf);

    auto hats = c.countOnsets (vj::OnsetRegion::high, 3.0, 20.0);
    std::snprintf (buf, sizeof buf, "hats register in the high region (%d events, ~%d hats)", hats, expected);
    check (hats >= expected * 80 / 100 && hats <= expected * 220 / 100, buf);
}

void testGainInvariance()
{
    std::puts ("Same groove at 0 / -12 / -24 dB");
    double relMeans[3], absMeans[3];
    const float gains[3] = { 1.0f, 0.2512f, 0.0631f };

    for (int i = 0; i < 3; ++i)
    {
        std::mt19937 rng (7);
        auto c = run (makeGroove (16.0, gains[i], rng));
        relMeans[i] = c.mean (8.0, 16.0, [] (auto& f) { return f.levelRel; });
        absMeans[i] = c.mean (8.0, 16.0, [] (auto& f) { return f.levelAbs; });
    }

    char buf[200];
    std::snprintf (buf, sizeof buf, "relative activity similar across gains (%.2f / %.2f / %.2f)", relMeans[0], relMeans[1], relMeans[2]);
    check (std::abs (relMeans[0] - relMeans[1]) < 0.15 && std::abs (relMeans[0] - relMeans[2]) < 0.15, buf);
    std::snprintf (buf, sizeof buf, "absolute activity keeps the difference (%.2f > %.2f > %.2f)", absMeans[0], absMeans[1], absMeans[2]);
    check (absMeans[0] > absMeans[1] + 0.15 && absMeans[1] > absMeans[2] + 0.15, buf);
    check (relMeans[2] > 0.2, "quiet copy is still usable (rel mean > 0.2)");
}

void testSteadyNoise()
{
    std::puts ("Steady noise at -30 dBFS (12 s)");
    std::mt19937 rng (3);
    Signal s (12.0);
    addNoise (s, 0.0, 12.0, 0.0316f, rng);
    auto c = run (s);
    auto n = (int) std::count_if (c.onsets.begin(), c.onsets.end(), [] (auto& e) { return e.sampleIndex / sr > 2.0; });
    check (n <= 3, "stationary noise produces almost no onsets (" + std::to_string (n) + " in 10 s)");
}

void testVibratoTone()
{
    std::puts ("Sustained tone with 6 Hz / 50 cent vibrato (10 s)");
    Signal s (10.0);
    addSine (s, 440.0, 0.3f, 0.0, 10.0, 6.0, 50.0);
    addSine (s, 880.0, 0.15f, 0.0, 10.0, 6.0, 50.0);
    auto c = run (s);
    auto mids = c.countOnsets (vj::OnsetRegion::mid, 2.0, 10.0);
    check (mids <= 4, "vibrato does not machine-gun the mid region (" + std::to_string (mids) + " events)");
}

void testQuietAfterDrop()
{
    std::puts ("Loud section, then quiet breakdown");
    std::mt19937 rng (11);
    auto loud = makeGroove (10.0, 1.0f, rng);
    auto quiet = makeGroove (10.0, 0.1f, rng);
    Signal s (20.0);
    std::copy (loud.left.begin(), loud.left.end(), s.left.begin());
    std::copy (loud.right.begin(), loud.right.end(), s.right.begin());
    std::copy (quiet.left.begin(), quiet.left.end(), s.left.begin() + (long long) loud.size());
    std::copy (quiet.right.begin(), quiet.right.end(), s.right.begin() + (long long) loud.size());

    auto c = run (s);
    auto loudRel = c.mean (7.0, 10.0, [] (auto& f) { return f.levelRel; });
    auto breakdownRel = c.mean (10.5, 12.5, [] (auto& f) { return f.levelRel; });
    auto laterRel = c.mean (17.0, 20.0, [] (auto& f) { return f.levelRel; });
    char buf[200];
    std::snprintf (buf, sizeof buf, "breakdown stays visibly darker right after the drop (%.2f vs %.2f)", breakdownRel, loudRel);
    check (breakdownRel < loudRel - 0.25, buf);
    std::snprintf (buf, sizeof buf, "sensitivity recovers gradually over the breakdown (%.2f -> %.2f)", breakdownRel, laterRel);
    check (laterRel > breakdownRel + 0.1, buf);
}

void testAntiPhase()
{
    std::puts ("Anti-phase stereo");
    Signal s (3.0);
    addSine (s, 200.0, 0.5f, 0.0, 3.0);
    for (auto& v : s.right) v = -v;
    auto c = run (s);
    char buf[120];
    std::snprintf (buf, sizeof buf, "level does not cancel (got %.1f dBFS)", c.frames.back().levelDb);
    check (c.frames.back().levelDb > -12.0f, buf);
}

void testMonoInput()
{
    std::puts ("Mono input path");
    Signal s (3.0);
    addSine (s, 1000.0, 1.0f, 0.0, 3.0);
    auto c = run (s, true);
    check (std::abs (c.frames.back().levelDb + 3.01f) < 0.3f, "mono sine reads the same level as stereo");
}

void testProtocolEncoding()
{
    std::puts ("OSC encoding");
    vj::protocol::OscWriter w;
    vj::FeatureFrame f;
    vj::protocol::Transport t;
    t.ppqPosition = 12345.75;
    auto frame = vj::protocol::encodeFrame (w, 42, 1, 7, f, t);
    check (! frame.empty() && frame.size() % 4 == 0, "frame datagram is 4-byte aligned (" + std::to_string (frame.size()) + " bytes)");
    check (frame.size() < 1200, "frame datagram stays under 1200 bytes");
    check (std::string (frame.data()) == "/v2/frame", "address is /v2/frame");

    auto spectrum = vj::protocol::encodeSpectrum (w, 42, 7, f);
    check (! spectrum.empty() && spectrum.size() < 1200, "spectrum datagram fits (" + std::to_string (spectrum.size()) + " bytes)");

    auto event = vj::protocol::encodeEvent (w, 42, 1, 3, "bassTransient", 0.8f, -1, 0, 2.5f);
    check (! event.empty() && event.size() % 4 == 0, "event datagram is aligned");
}

} // namespace

// --stats: novelty distribution per region for each fixture (for tuning floors).
struct NoveltyStats : Capture
{
    std::vector<float> nov[3], thr[3];
    void novelty (vj::OnsetRegion r, float n, float t) override { nov[(int) r].push_back (n); thr[(int) r].push_back (t); }
};

void printStats (const char* name, const Signal& s)
{
    vj::Analyzer a;
    a.prepare (sr);
    NoveltyStats st;
    for (size_t pos = 0; pos < s.size(); pos += 512)
        a.process (s.left.data() + pos, s.right.data() + pos, (int) std::min<size_t> (512, s.size() - pos), st);

    std::puts (name);
    const char* names[3] = { "bass", "mid ", "high" };
    for (int r = 0; r < 3; ++r)
    {
        auto v = st.nov[r];
        v.erase (v.begin(), v.begin() + (long long) std::min<size_t> (v.size(), 750)); // skip 2 s warm-up
        if (v.empty()) continue;
        std::sort (v.begin(), v.end());
        auto q = [&] (double p) { return v[(size_t) (p * (v.size() - 1))]; };
        std::printf ("  %s  p50 %.4f  p90 %.4f  p99 %.4f  max %.4f  | onsets %d\n", names[r], q (0.5), q (0.9), q (0.99), v.back(),
                     st.countOnsets ((vj::OnsetRegion) r, 2.0, 1.0e9));
    }
}

int main (int argc, char** argv)
{
    if (argc > 1 && std::string (argv[1]) == "--stats")
    {
        std::mt19937 rng (5);
        Signal noise (10.0); addNoise (noise, 0.0, 10.0, 0.0316f, rng);
        Signal vib (10.0); addSine (vib, 440.0, 0.3f, 0.0, 10.0, 6.0, 50.0); addSine (vib, 880.0, 0.15f, 0.0, 10.0, 6.0, 50.0);
        printStats ("steady noise -30 dB", noise);
        printStats ("vibrato tone", vib);
        printStats ("groove", makeGroove (10.0, 1.0f, rng));
        return 0;
    }

    testSilence();
    testSineCalibration();
    testKickDetection();
    testGainInvariance();
    testSteadyNoise();
    testVibratoTone();
    testQuietAfterDrop();
    testAntiPhase();
    testMonoInput();
    testProtocolEncoding();

    std::printf ("\n%s - %d failure(s)\n", failures == 0 ? "ALL PASSED" : "FAILED", failures);
    return failures;
}
