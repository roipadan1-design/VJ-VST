#include "vj/Protocol.h"

#include <algorithm>
#include <cmath>

namespace vj::protocol
{

namespace
{
    size_t padded (size_t n) noexcept { return (n + 4) & ~size_t (3); } // includes the NUL terminator

    int clampRole (int role) noexcept { return std::clamp (role, 0, numRoles - 1); }
}

void OscWriter::pushArg (const void* source, size_t n)
{
    if (size + n > args.size())
    {
        overflow = true;
        return;
    }

    // OSC is big-endian; x86/ARM Windows is little-endian.
    auto* bytes = static_cast<const char*> (source);
    for (size_t i = 0; i < n; ++i)
        args[size + i] = bytes[n - 1 - i];
    size += n;
}

void OscWriter::addString (std::string_view s)
{
    pushType ('s');
    auto total = padded (s.size());

    if (size + total > args.size())
    {
        overflow = true;
        return;
    }

    std::memcpy (args.data() + size, s.data(), s.size());
    std::memset (args.data() + size + s.size(), 0, total - s.size());
    size += total;
}

std::string_view OscWriter::finish()
{
    auto addressBytes = padded (address.size());
    auto typeBytes = padded ((size_t) typeCount + 1); // leading ','
    auto total = addressBytes + typeBytes + size;

    if (overflow || total > packet.size())
    {
        overflow = true;
        return {};
    }

    auto* p = packet.data();
    std::memset (p, 0, addressBytes + typeBytes);
    std::memcpy (p, address.data(), address.size());
    p += addressBytes;
    p[0] = ',';
    std::memcpy (p + 1, types.data(), (size_t) typeCount);
    p += typeBytes;
    std::memcpy (p, args.data(), size);

    return { packet.data(), total };
}

std::string_view onsetTypeName (OnsetRegion r) noexcept
{
    switch (r)
    {
        case OnsetRegion::bass: return "bassTransient";
        case OnsetRegion::mid:  return "midTransient";
        case OnsetRegion::high: return "highTransient";
    }
    return "bassTransient";
}

std::string_view encodeHello (OscWriter& w, int32_t sourceId, int role, std::string_view name, int replyPort)
{
    w.begin ("/v2/hello");
    w.addInt (sourceId);
    w.addString (roleNames[clampRole (role)]);
    w.addString (name);
    w.addInt (version);
    if (replyPort > 0)
        w.addInt (replyPort);
    return w.finish();
}

std::string_view encodeFrame (OscWriter& w, int32_t sourceId, int role, int32_t seq,
                              const FeatureFrame& f, const Transport& t)
{
    int32_t flags = 0;
    if (f.gateOpen)   flags |= gateOpen;
    if (f.calibrated) flags |= calibrated;
    if (t.valid)      flags |= transportValid;
    if (t.playing)    flags |= transportPlaying;

    w.begin ("/v2/frame");
    w.addInt (sourceId);
    w.addString (roleNames[clampRole (role)]);
    w.addInt (seq);
    w.addInt (flags);

    w.addFloat (f.levelRel);
    w.addFloat (f.levelAbs);
    w.addFloat (std::max (f.levelDb, -160.0f));

    for (auto v : f.aggregateRel) w.addFloat (v);
    for (auto v : f.aggregateAbs) w.addFloat (v);
    for (auto v : f.bandRel)      w.addFloat (v);

    w.addFloat (f.centroid01);
    w.addFloat (f.flatness);
    w.addFloat (f.rolloff01);
    w.addFloat (f.flux01);
    w.addFloat (f.energyTrend);

    // Beat position split into whole + fraction: a float32 of a long song
    // position would lose sub-beat precision.
    auto whole = std::floor (t.ppqPosition);
    w.addFloat ((float) t.bpm);
    w.addInt ((int32_t) whole);
    w.addFloat ((float) (t.ppqPosition - whole));
    w.addInt (t.meterNumerator);
    w.addInt (t.meterDenominator);
    w.addInt (t.epoch);
    return w.finish();
}

std::string_view encodeSpectrum (OscWriter& w, int32_t sourceId, int32_t seq, const FeatureFrame& f)
{
    w.begin ("/v2/spectrum");
    w.addInt (sourceId);
    w.addInt (seq);
    for (auto v : f.spectrum)
        w.addFloat (v);
    return w.finish();
}

std::string_view encodeEvent (OscWriter& w, int32_t sourceId, int role, int32_t eventId,
                              std::string_view type, float strength, int note, int velocity, float ageMs)
{
    w.begin ("/v2/event");
    w.addInt (sourceId);
    w.addString (roleNames[clampRole (role)]);
    w.addInt (eventId);
    w.addString (type);
    w.addFloat (strength);
    w.addInt (note);
    w.addInt (velocity);
    w.addFloat (ageMs);
    return w.finish();
}

std::string_view encodeMacro (OscWriter& w, int slot, float value)
{
    w.begin ("/v2/macro");
    w.addInt (slot);
    w.addFloat (value);
    return w.finish();
}

} // namespace vj::protocol
