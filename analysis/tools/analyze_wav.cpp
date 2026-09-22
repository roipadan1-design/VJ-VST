// analyze_wav - run AnalysisCore over a WAV file.
//
//   analyze_wav song.wav                    summary: onset counts, activity statistics
//   analyze_wav song.wav --csv out.csv      + per-frame CSV (100 Hz) and event list
//   analyze_wav song.wav --send [host:port] stream the analysis to the VJ Engine in
//                                           real time as protocol-v2 OSC (default
//                                           127.0.0.1:9000), with a simulated transport
//        [--bpm 120] [--role mix] [--loop] [--offset seconds]
//
// --send exercises the exact wire format the VJ Analyzer plugin emits, so the
// engine's v2 path can be tested end to end with real music and no DAW.

#include "vj/Analyzer.h"
#include "vj/Protocol.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace
{

struct Audio
{
    double sampleRate = 0.0;
    int channels = 0;
    std::vector<float> left, right;
};

template <typename T>
bool readValue (std::ifstream& in, T& v) { return (bool) in.read (reinterpret_cast<char*> (&v), sizeof v); }

bool loadWav (const std::string& path, Audio& out, std::string& error)
{
    std::ifstream in (path, std::ios::binary);
    if (! in) { error = "cannot open file"; return false; }

    char riff[4], wave[4];
    uint32_t riffSize;
    in.read (riff, 4); readValue (in, riffSize); in.read (wave, 4);
    if (std::memcmp (riff, "RIFF", 4) != 0 || std::memcmp (wave, "WAVE", 4) != 0) { error = "not a RIFF/WAVE file"; return false; }

    uint16_t format = 0, channels = 0, bits = 0;
    uint32_t rate = 0;
    std::vector<char> data;

    while (in)
    {
        char id[4];
        uint32_t size;
        if (! in.read (id, 4) || ! readValue (in, size))
            break;

        if (std::memcmp (id, "fmt ", 4) == 0)
        {
            std::vector<char> fmt (size);
            in.read (fmt.data(), size);
            std::memcpy (&format, fmt.data(), 2);
            std::memcpy (&channels, fmt.data() + 2, 2);
            std::memcpy (&rate, fmt.data() + 4, 4);
            std::memcpy (&bits, fmt.data() + 14, 2);
            if (format == 0xFFFE && size >= 26)
                std::memcpy (&format, fmt.data() + 24, 2); // WAVE_FORMAT_EXTENSIBLE sub-format
        }
        else if (std::memcmp (id, "data", 4) == 0)
        {
            data.resize (size);
            in.read (data.data(), size);
            data.resize ((size_t) in.gcount());
        }
        else
        {
            in.seekg (size, std::ios::cur);
        }

        if (size & 1)
            in.seekg (1, std::ios::cur);
    }

    if (channels == 0 || rate == 0 || data.empty()) { error = "missing fmt or data chunk"; return false; }
    if (! ((format == 1 && (bits == 16 || bits == 24 || bits == 32)) || (format == 3 && bits == 32)))
    {
        error = "unsupported sample format (need 16/24/32-bit PCM or 32-bit float)";
        return false;
    }

    const auto bytesPerSample = bits / 8;
    const auto frames = data.size() / ((size_t) bytesPerSample * channels);
    out.sampleRate = rate;
    out.channels = channels;
    out.left.resize (frames);
    out.right.resize (frames);

    auto sampleAt = [&] (size_t frame, int ch) -> float {
        auto* p = reinterpret_cast<const unsigned char*> (data.data()) + (frame * channels + (size_t) ch) * bytesPerSample;
        if (format == 3) { float f; std::memcpy (&f, p, 4); return f; }
        if (bits == 16) { int16_t v; std::memcpy (&v, p, 2); return v / 32768.0f; }
        if (bits == 24) { int32_t v = (p[0] << 8) | (p[1] << 16) | (p[2] << 24); return (float) (v / 2147483648.0); }
        int32_t v; std::memcpy (&v, p, 4); return (float) (v / 2147483648.0);
    };

    for (size_t i = 0; i < frames; ++i)
    {
        out.left[i] = sampleAt (i, 0);
        out.right[i] = sampleAt (i, channels > 1 ? 1 : 0);
    }
    return true;
}

struct SummaryListener : vj::Analyzer::Listener
{
    std::FILE* csv = nullptr;
    double sampleRate = 48000.0;
    int onsets[3] = { 0, 0, 0 };
    double relSum = 0.0, absSum = 0.0;
    long frames = 0, gatedFrames = 0;
    int64_t lastCsvSample = -1000000;
    std::vector<std::string> eventLines;
    std::vector<float> noveltyValues[3], thresholdValues[3];

    void novelty (vj::OnsetRegion r, float n, float t) override
    {
        noveltyValues[(int) r].push_back (n);
        thresholdValues[(int) r].push_back (std::min (t, 10.0f));
    }

    void featureFrame (const vj::FeatureFrame& f) override
    {
        ++frames;
        if (f.gateOpen) { ++gatedFrames; relSum += f.levelRel; absSum += f.levelAbs; }

        if (csv != nullptr && f.sampleIndex - lastCsvSample >= (int64_t) (sampleRate / 100.0))
        {
            lastCsvSample = f.sampleIndex;
            std::fprintf (csv, "%.4f,%.2f,%.3f,%.3f,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                          f.sampleIndex / sampleRate, f.levelDb, f.levelRel, f.levelAbs, f.gateOpen ? 1 : 0,
                          f.aggregateRel[0], f.aggregateRel[1], f.aggregateRel[2],
                          f.centroid01, f.flatness, f.flux01, f.energyTrend);
        }
    }

    void onset (const vj::OnsetEvent& e) override
    {
        ++onsets[(int) e.region];
        char line[96];
        std::snprintf (line, sizeof line, "%.4f,%s,%.3f", e.sampleIndex / sampleRate,
                       std::string (vj::protocol::onsetTypeName (e.region)).c_str(), e.strength);
        eventLines.emplace_back (line);
    }
};

int runSummary (const Audio& audio, const std::string& csvPath)
{
    vj::Analyzer analyzer;
    analyzer.prepare (audio.sampleRate);

    SummaryListener listener;
    listener.sampleRate = audio.sampleRate;

    if (! csvPath.empty())
    {
        listener.csv = std::fopen (csvPath.c_str(), "w");
        if (listener.csv == nullptr) { std::fprintf (stderr, "cannot write %s\n", csvPath.c_str()); return 1; }
        std::fputs ("time,levelDb,levelRel,levelAbs,gate,bassRel,midRel,highRel,centroid,flatness,flux,energyTrend\n", listener.csv);
    }

    auto started = std::chrono::steady_clock::now();
    const int block = 512;
    for (size_t pos = 0; pos < audio.left.size(); pos += block)
    {
        auto n = (int) std::min<size_t> (block, audio.left.size() - pos);
        analyzer.process (audio.left.data() + pos, audio.right.data() + pos, n, listener);
    }
    auto elapsed = std::chrono::duration<double> (std::chrono::steady_clock::now() - started).count();

    if (listener.csv != nullptr)
    {
        std::fclose (listener.csv);
        auto eventsPath = csvPath + ".events.csv";
        if (auto* ev = std::fopen (eventsPath.c_str(), "w"))
        {
            std::fputs ("time,type,strength\n", ev);
            for (auto& l : listener.eventLines) std::fprintf (ev, "%s\n", l.c_str());
            std::fclose (ev);
        }
    }

    auto duration = audio.left.size() / audio.sampleRate;
    std::printf ("duration        %.1f s @ %.0f Hz, %d ch\n", duration, audio.sampleRate, audio.channels);
    std::printf ("analysis speed  %.0fx real time\n", duration / std::max (1.0e-9, elapsed));
    std::printf ("gate open       %.0f%% of frames\n", 100.0 * listener.gatedFrames / std::max (1L, listener.frames));
    std::printf ("mean activity   rel %.2f  abs %.2f (while gate open)\n",
                listener.relSum / std::max (1L, listener.gatedFrames), listener.absSum / std::max (1L, listener.gatedFrames));
    std::printf ("onsets          bass %d (%.2f/s)  mid %d (%.2f/s)  high %d (%.2f/s)\n",
                listener.onsets[0], listener.onsets[0] / duration, listener.onsets[1], listener.onsets[1] / duration,
                listener.onsets[2], listener.onsets[2] / duration);

    const char* names[3] = { "bass", "mid ", "high" };
    for (int r = 0; r < 3; ++r)
    {
        auto v = listener.noveltyValues[r], t = listener.thresholdValues[r];
        if (v.empty()) continue;
        std::sort (v.begin(), v.end());
        std::sort (t.begin(), t.end());
        auto q = [] (const std::vector<float>& x, double p) { return x[(size_t) (p * (x.size() - 1))]; };
        std::printf ("novelty %s    p50 %.3f  p90 %.3f  p99 %.3f  max %.3f   threshold p50 %.3f\n",
                     names[r], q (v, 0.5), q (v, 0.9), q (v, 0.99), v.back(), q (t, 0.5));
    }
    return 0;
}

// Streams frames/events to the engine while "playing" the file in real time.
struct Sender : vj::Analyzer::Listener
{
    SOCKET sock = INVALID_SOCKET;
    sockaddr_in target {};
    int32_t sourceId = 0;
    int role = 0;
    int32_t frameSeq = 0, eventSeq = 0;
    vj::protocol::Transport transport;
    vj::protocol::OscWriter writer;
    std::chrono::steady_clock::time_point lastFrameSent {}, lastSpectrumSent {};

    void send (std::string_view packet)
    {
        if (! packet.empty())
            sendto (sock, packet.data(), (int) packet.size(), 0, reinterpret_cast<sockaddr*> (&target), sizeof target);
    }

    void featureFrame (const vj::FeatureFrame& f) override
    {
        auto now = std::chrono::steady_clock::now();
        if (now - lastFrameSent >= std::chrono::microseconds (8333)) // ~120 Hz latest-wins
        {
            lastFrameSent = now;
            send (vj::protocol::encodeFrame (writer, sourceId, role, ++frameSeq, f, transport));
        }
        if (now - lastSpectrumSent >= std::chrono::microseconds (16666)) // ~60 Hz
        {
            lastSpectrumSent = now;
            send (vj::protocol::encodeSpectrum (writer, sourceId, frameSeq, f));
        }
    }

    void onset (const vj::OnsetEvent& e) override
    {
        send (vj::protocol::encodeEvent (writer, sourceId, role, ++eventSeq,
                                         vj::protocol::onsetTypeName (e.region), e.strength, -1, 0, 0.0f));
    }
};

int runSend (const Audio& audio, const std::string& hostPort, double bpm, int role, bool loop, double offsetSeconds)
{
    WSADATA wsa;
    if (WSAStartup (MAKEWORD (2, 2), &wsa) != 0) { std::fprintf (stderr, "WSAStartup failed\n"); return 1; }

    auto colon = hostPort.find (':');
    auto host = hostPort.substr (0, colon);
    auto port = colon == std::string::npos ? vj::protocol::defaultPort : std::stoi (hostPort.substr (colon + 1));

    Sender sender;
    sender.sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sender.target.sin_family = AF_INET;
    sender.target.sin_port = htons ((u_short) port);
    inet_pton (AF_INET, host.c_str(), &sender.target.sin_addr);
    sender.sourceId = (int32_t) (std::random_device {}() & 0x7fffffff);
    sender.role = role;
    sender.transport.valid = sender.transport.playing = true;
    sender.transport.bpm = bpm;

    vj::Analyzer analyzer;
    analyzer.prepare (audio.sampleRate);

    std::printf ("streaming to %s:%d as source %d (role %s), %.1f BPM - Ctrl+C to stop\n",
                 host.c_str(), port, sender.sourceId, std::string (vj::protocol::roleNames[role]).c_str(), bpm);

    const int block = 256;
    auto start = (size_t) std::clamp (offsetSeconds * audio.sampleRate, 0.0, (double) audio.left.size());
    auto clockStart = std::chrono::steady_clock::now();
    auto lastHello = clockStart - std::chrono::seconds (2);
    int64_t played = 0;

    do
    {
        for (size_t pos = start; pos < audio.left.size(); pos += block)
        {
            auto n = (int) std::min<size_t> (block, audio.left.size() - pos);
            played += n;
            sender.transport.ppqPosition = (played / audio.sampleRate) * bpm / 60.0;
            analyzer.process (audio.left.data() + pos, audio.right.data() + pos, n, sender);

            auto now = std::chrono::steady_clock::now();
            if (now - lastHello > std::chrono::seconds (1))
            {
                lastHello = now;
                sender.send (vj::protocol::encodeHello (sender.writer, sender.sourceId, role, "analyze_wav"));
            }

            // Pace to real time.
            auto due = clockStart + std::chrono::duration_cast<std::chrono::steady_clock::duration> (
                                        std::chrono::duration<double> (played / audio.sampleRate));
            std::this_thread::sleep_until (due);
        }
        start = 0;
        ++sender.transport.epoch; // loop wrap is a transport discontinuity
    } while (loop);

    closesocket (sender.sock);
    WSACleanup();
    return 0;
}

} // namespace

int main (int argc, char** argv)
{
    if (argc < 2)
    {
        std::puts ("usage: analyze_wav file.wav [--csv out.csv] [--send host:port] [--bpm 120] [--role mix] [--loop] [--offset s]");
        return 1;
    }

    std::string path = argv[1], csv, sendTo;
    bool send = false, loop = false;
    double bpm = 120.0, offset = 0.0;
    int role = 0;

    for (int i = 2; i < argc; ++i)
    {
        std::string a = argv[i];
        auto next = [&] { return i + 1 < argc ? std::string (argv[++i]) : std::string(); };

        if (a == "--csv") csv = next();
        else if (a == "--send")
        {
            send = true;
            sendTo = "127.0.0.1:9000";
            if (i + 1 < argc && argv[i + 1][0] != '-') sendTo = next();
        }
        else if (a == "--bpm") bpm = std::stod (next());
        else if (a == "--offset") offset = std::stod (next());
        else if (a == "--loop") loop = true;
        else if (a == "--role")
        {
            auto r = next();
            for (int k = 0; k < vj::protocol::numRoles; ++k)
                if (r == vj::protocol::roleNames[k]) role = k;
        }
    }

    Audio audio;
    std::string error;
    if (! loadWav (path, audio, error))
    {
        std::fprintf (stderr, "%s: %s\n", path.c_str(), error.c_str());
        return 1;
    }

    return send ? runSend (audio, sendTo, bpm, role, loop, offset) : runSummary (audio, csv);
}
