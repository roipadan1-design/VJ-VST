#pragma once

#include "vj/Analyzer.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <string_view>

// VJ protocol v2 - OSC 1.0 (int32 / float32 / string only, so Max's udpsend
// and any OSC tool interoperate). One datagram per message, every message
// well under 1200 bytes. Continuous features are latest-wins snapshots;
// events carry an ID and an age so the engine can de-duplicate and expire them.
//
//   /v2/hello    i sourceId  s role  s name  i protocolVersion
//   /v2/frame    i sourceId  s role  i seq  i flags
//                f levelRel f levelAbs f levelDb
//                f bassRel f midRel f highRel  f bassAbs f midAbs f highAbs
//                f band0..band5 (rel)
//                f centroid f flatness f rolloff f flux f energyTrend
//                f bpm  i beatWhole  f beatFraction  i meterNum  i meterDen  i epoch
//   /v2/spectrum i sourceId  i seq  f x32
//   /v2/event    i sourceId  s role  i eventId  s type  f strength  i note  i velocity  f ageMs
//
// Control (any sender -> engine):
//   /v2/macro      i slot(0-7)  f value(0-1)
//   /v2/preset     i index         /v2/preset/next    /v2/preset/previous
//   /v2/blackout   i on(0/1)       /v2/trigger        (manual "user" hit)
//   /v2/transition f milliseconds
namespace vj::protocol
{

constexpr int version = 2;
constexpr int defaultPort = 9000;

enum FrameFlags : int32_t
{
    gateOpen = 1,
    calibrated = 2,
    transportValid = 4,
    transportPlaying = 8,
};

// Roles a source can declare. The engine's role resolver maps e.g. a "kick"
// source's bass transients (or its MIDI notes) to event.kick, falling back to
// the mix source when no kick source is present (preset sourcePolicy).
constexpr std::string_view roleNames[] = { "mix", "kick", "snare", "hat", "bass", "texture" };
constexpr int numRoles = 6;

struct Transport
{
    bool valid = false, playing = false;
    double bpm = 120.0;
    double ppqPosition = 0.0; // quarter notes
    int meterNumerator = 4, meterDenominator = 4;
    int32_t epoch = 0;        // bumped on seeks / discontinuous jumps
};

// Fixed-capacity OSC message builder. No allocation; overflow is reported
// through ok() rather than truncating silently.
class OscWriter
{
public:
    void begin (std::string_view newAddress)
    {
        size = 0;
        overflow = false;
        typeCount = 0;
        address = newAddress;
    }

    void addInt (int32_t v)     { pushType ('i'); pushArg (&v, 4); }
    void addFloat (float v)     { pushType ('f'); pushArg (&v, 4); }
    void addString (std::string_view s);

    // Assembles address + type tags + arguments; returns the datagram.
    std::string_view finish();
    bool ok() const noexcept { return ! overflow; }

private:
    void pushType (char t) { if (typeCount < (int) types.size()) types[(size_t) typeCount++] = t; else overflow = true; }
    void pushArg (const void* bigEndianSource, size_t n);

    std::string_view address;
    std::array<char, 64> types {};
    int typeCount = 0;
    std::array<char, 1024> args {};
    size_t size = 0;
    std::array<char, 1400> packet {};
    bool overflow = false;
};

std::string_view encodeHello (OscWriter&, int32_t sourceId, int role, std::string_view name);
std::string_view encodeFrame (OscWriter&, int32_t sourceId, int role, int32_t seq,
                              const FeatureFrame&, const Transport&);
std::string_view encodeSpectrum (OscWriter&, int32_t sourceId, int32_t seq, const FeatureFrame&);
std::string_view encodeEvent (OscWriter&, int32_t sourceId, int role, int32_t eventId,
                              std::string_view type, float strength, int note, int velocity, float ageMs);
std::string_view encodeMacro (OscWriter&, int slot, float value);

std::string_view onsetTypeName (OnsetRegion r) noexcept;

} // namespace vj::protocol
