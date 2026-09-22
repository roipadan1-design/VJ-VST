#include "FeatureBus.h"

namespace
{
    float argFloat (const juce::OSCMessage& m, int i, float fallback = 0.0f)
    {
        if (i >= m.size()) return fallback;
        if (m[i].isFloat32()) return std::isfinite (m[i].getFloat32()) ? m[i].getFloat32() : fallback;
        if (m[i].isInt32())   return (float) m[i].getInt32();
        return fallback;
    }

    int argInt (const juce::OSCMessage& m, int i, int fallback = 0)
    {
        if (i >= m.size()) return fallback;
        if (m[i].isInt32())   return m[i].getInt32();
        if (m[i].isFloat32()) return (int) m[i].getFloat32();
        return fallback;
    }

    juce::String argString (const juce::OSCMessage& m, int i)
    {
        return i < m.size() && m[i].isString() ? m[i].getString() : juce::String();
    }

    float clamp01 (float v) { return juce::jlimit (0.0f, 1.0f, v); }
}

SourceRole FeatureBus::parseRole (const juce::String& r) noexcept
{
    static const char* const names[] = { "mix", "kick", "snare", "hat", "bass", "texture" };
    for (int i = 0; i < (int) SourceRole::count; ++i)
        if (r == names[i])
            return (SourceRole) i;
    return SourceRole::mix;
}

bool FeatureBus::isNewer (int incoming, int last) noexcept
{
    // Accept a large backwards jump as a sender restart rather than a stale packet.
    return incoming > last || last - incoming > 1000;
}

bool FeatureBus::handleMessage (const juce::OSCMessage& message, double now)
{
    const auto address = message.getAddressPattern().toString();

    if (address == "/v2/frame")    { handleFrame (message, now); return true; }
    if (address == "/v2/event")    { handleEvent (message, now); return true; }
    if (address == "/v2/spectrum") { handleSpectrum (message); return true; }

    if (address == "/v2/hello")
    {
        const juce::ScopedLock sl (lock);
        auto& s = sources[argInt (message, 0)];
        s.role = parseRole (argString (message, 1));
        s.name = argString (message, 2);
        auto replyPort = argInt (message, 4, 0);
        if (replyPort > 1024 && replyPort < 65536)
            replyPorts[replyPort] = now;
        return true;
    }

    if (address.startsWith ("/audio/"))
    {
        handleLegacy (address, message, now);
        return true;
    }

    return false;
}

void FeatureBus::handleFrame (const juce::OSCMessage& m, double now)
{
    if (m.size() < 30) // see vj/Protocol.h for the argument layout
        return;

    const juce::ScopedLock sl (lock);
    auto& s = sources[argInt (m, 0)];
    auto seq = argInt (m, 2);

    if (! isNewer (seq, s.lastSeq))
        return; // duplicate or reordered snapshot - latest wins

    s.lastSeq = seq;
    s.role = parseRole (argString (m, 1));
    s.lastFrameTime = now;

    auto flags = argInt (m, 3);
    int i = 4;
    s.levelRel = clamp01 (argFloat (m, i++));
    s.levelAbs = clamp01 (argFloat (m, i++));
    ++i; // levelDb (diagnostic only)
    s.bassRel = clamp01 (argFloat (m, i++));
    s.midRel  = clamp01 (argFloat (m, i++));
    s.highRel = clamp01 (argFloat (m, i++));
    s.bassAbs = clamp01 (argFloat (m, i++));
    s.midAbs  = clamp01 (argFloat (m, i++));
    s.highAbs = clamp01 (argFloat (m, i++));
    for (auto& b : s.bandRel) b = clamp01 (argFloat (m, i++));
    s.centroid = clamp01 (argFloat (m, i++));
    s.flatness = clamp01 (argFloat (m, i++));
    s.rolloff  = clamp01 (argFloat (m, i++));
    s.flux     = clamp01 (argFloat (m, i++));
    s.energyTrend = juce::jlimit (-1.0f, 1.0f, argFloat (m, i++));

    auto bpm = argFloat (m, i++, 120.0f);
    auto whole = argInt (m, i++);
    auto fraction = argFloat (m, i++);
    s.meterNumerator = juce::jlimit (1, 32, argInt (m, i++, 4));
    s.meterDenominator = juce::jlimit (1, 32, argInt (m, i++, 4));
    s.epoch = argInt (m, i++);
    s.transportValid = (flags & 4) != 0;
    s.playing = (flags & 8) != 0;

    if (s.transportValid)
    {
        s.bpm = juce::jlimit (20.0, 400.0, (double) bpm);
        s.beatPosition = whole + (double) fraction;
        s.transportReceivedAt = now;
    }
}

void FeatureBus::handleSpectrum (const juce::OSCMessage& m)
{
    if (m.size() < 34)
        return;

    const juce::ScopedLock sl (lock);
    auto found = sources.find (argInt (m, 0));
    if (found == sources.end())
        return;

    for (int b = 0; b < 32; ++b)
        found->second.spectrum[(size_t) b] = clamp01 (argFloat (m, 2 + b));
}

void FeatureBus::handleEvent (const juce::OSCMessage& m, double now)
{
    if (m.size() < 8)
        return;

    const juce::ScopedLock sl (lock);
    auto& s = sources[argInt (m, 0)];
    auto eventId = argInt (m, 2);

    if (! isNewer (eventId, s.lastEventId))
        return; // retransmission

    s.lastEventId = eventId;
    s.role = parseRole (argString (m, 1));

    PendingEvent e;
    e.role = s.role;
    e.strength = clamp01 (argFloat (m, 4, 1.0f));
    e.note = argInt (m, 5, -1);
    e.velocity = argInt (m, 6);
    e.time = now - juce::jlimit (0.0, 1.0, (double) argFloat (m, 7) / 1000.0);

    auto type = argString (m, 3);
    if (type == "note")
    {
        e.rawType = EventType::midiNote;
        s.lastMidiNoteTime = now;
    }
    else if (type == "user")
        e.rawType = EventType::userTrigger;
    else if (! parseEventName (type, e.rawType))
        return;

    e.midiRecent = (now - s.lastMidiNoteTime) < 2.0;
    pending.add (e);
}

void FeatureBus::handleLegacy (const juce::String& address, const juce::OSCMessage& m, double now)
{
    const juce::ScopedLock sl (lock);
    auto& s = sources[legacySourceId];
    s.role = SourceRole::mix;
    s.name = "M4L analyzer (legacy)";

    if (address == "/audio/onset")
    {
        pending.add ({ EventType::bassTransient, SourceRole::mix, false, 1.0f, -1, 0, now });
        return;
    }

    auto value = clamp01 (argFloat (m, 0));
    s.lastFrameTime = now;

    if (address == "/audio/level")     { s.levelRel = s.levelAbs = value; }
    else if (address == "/audio/bass") { s.bassRel = s.bassAbs = value; }
    else if (address == "/audio/mid")  { s.midRel = s.midAbs = value; }
    else if (address == "/audio/high") { s.highRel = s.highAbs = value; }
    else if (address == "/audio/beatphase")
    {
        if (value < legacyLastPhase - 0.5)
        {
            legacyBeatCount += 1.0;
            if (legacyLastWrapTime > 0.0)
            {
                auto period = now - legacyLastWrapTime;
                if (period > 0.15 && period < 3.0)
                    legacyBpm = legacyBpm * 0.8 + (60.0 / period) * 0.2;
            }
            legacyLastWrapTime = now;
        }

        legacyLastPhase = value;
        s.transportValid = s.playing = true;
        s.bpm = legacyBpm;
        s.beatPosition = legacyBeatCount + value;
        s.transportReceivedAt = now;
    }
}

void FeatureBus::pushUserTrigger (double now)
{
    const juce::ScopedLock sl (lock);
    pending.add ({ EventType::userTrigger, SourceRole::mix, false, 1.0f, -1, 0, now });
}

void FeatureBus::synthesizeDemo (Signals& out, double now)
{
    // A 120 BPM groove with a 16-bar arc: 12 bars full, 4 bars breakdown
    // (no kick, lower level) - enough to see continuous motion, hits and
    // section contrast without any audio connected.
    const double bpm = 120.0;
    auto beat = now * bpm / 60.0;
    auto bar = std::floor (beat / 4.0);
    bool breakdown = std::fmod (bar, 16.0) >= 12.0;
    auto phase = beat - std::floor (beat);
    auto kickEnv = breakdown ? 0.0f : (float) std::exp (-phase * 6.0);
    auto swell = (float) (0.5 + 0.5 * std::sin (beat * juce::MathConstants<double>::pi / 16.0));

    out.anyLive = true;
    out.levelRel = breakdown ? 0.25f + 0.1f * swell : 0.55f + 0.35f * kickEnv;
    out.levelAbs = out.levelRel;
    out.bassRel = out.bassAbs = breakdown ? 0.1f : 0.3f + 0.7f * kickEnv;
    out.midRel = out.midAbs = 0.35f + 0.25f * swell;
    out.highRel = out.highAbs = 0.3f + 0.4f * (float) std::exp (-std::abs (phase - 0.5) * 10.0);
    for (int b = 0; b < 6; ++b)
        out.bandRel[(size_t) b] = b < 2 ? out.bassRel : (b < 4 ? out.midRel : out.highRel);
    for (int b = 0; b < 32; ++b)
        out.spectrum[(size_t) b] = juce::jlimit (0.0f, 1.0f, (b < 8 ? out.bassRel : out.midRel * 0.8f) * (1.0f - b / 48.0f));
    out.centroid = 0.4f + 0.2f * swell;
    out.flatness = 0.2f;
    out.energyTrend = breakdown ? -0.4f : 0.1f;
    out.transportValid = out.playing = true;
    out.bpm = bpm;
    out.beatPosition = beat;
    out.transportReceivedAt = now;

    // Hits on half-beat crossings: kick on beats, hat on off-beats, snare on 2 & 4.
    auto half = std::floor (beat * 2.0);
    if (demoLastBeat >= 0.0 && half != demoLastBeat)
    {
        bool onBeat = std::fmod (half, 2.0) == 0.0;
        auto beatInBar = (int) std::fmod (std::floor (beat), 4.0);

        if (onBeat && ! breakdown)
        {
            out.events.add ({ EventType::kick, 1.0f });
            out.events.add ({ EventType::bassTransient, 1.0f });
            out.lastImpactTime = now;
            lastImpactTime = now;
        }
        if (onBeat && (beatInBar == 1 || beatInBar == 3))
            out.events.add ({ EventType::snare, 0.8f });
        if (! onBeat)
            out.events.add ({ EventType::hat, 0.6f });
    }
    demoLastBeat = half;
}

Signals FeatureBus::takeSnapshot (double now)
{
    Signals out;
    const juce::ScopedLock sl (lock);

    // Drop sources that have been silent too long.
    for (auto it = sources.begin(); it != sources.end();)
        it = (now - it->second.lastFrameTime > dropAfterSeconds) ? sources.erase (it) : std::next (it);

    auto fadeFor = [&] (const Source& s) {
        auto age = now - s.lastFrameTime;
        return age <= staleAfterSeconds ? 1.0f : (float) juce::jlimit (0.0, 1.0, 1.0 - (age - staleAfterSeconds) / fadeSeconds);
    };

    const Source* mix = nullptr;
    const Source* transport = nullptr;

    for (auto& [id, s] : sources)
    {
        auto fade = fadeFor (s);
        auto& role = out.roles[(size_t) s.role];
        role.live = role.live || fade > 0.0f;
        role.levelRel = juce::jmax (role.levelRel, s.levelRel * fade);
        role.levelAbs = juce::jmax (role.levelAbs, s.levelAbs * fade);

        if (s.role == SourceRole::mix && (mix == nullptr || s.lastFrameTime > mix->lastFrameTime))
            mix = &s;
        if (s.transportValid && (transport == nullptr || s.role == SourceRole::mix))
            transport = &s;
    }

    if (mix == nullptr && ! sources.empty())
        mix = &sources.begin()->second;

    if (mix != nullptr)
    {
        auto fade = fadeFor (*mix);
        out.anyLive = fade > 0.0f;
        out.levelRel = mix->levelRel * fade;  out.levelAbs = mix->levelAbs * fade;
        out.bassRel = mix->bassRel * fade;    out.bassAbs = mix->bassAbs * fade;
        out.midRel = mix->midRel * fade;      out.midAbs = mix->midAbs * fade;
        out.highRel = mix->highRel * fade;    out.highAbs = mix->highAbs * fade;
        for (size_t b = 0; b < 6; ++b)  out.bandRel[b] = mix->bandRel[b] * fade;
        for (size_t b = 0; b < 32; ++b) out.spectrum[b] = mix->spectrum[b] * fade;
        out.centroid = mix->centroid;
        out.flatness = mix->flatness;
        out.rolloff = mix->rolloff;
        out.flux = mix->flux * fade;
        out.energyTrend = mix->energyTrend * fade;
    }

    if (transport != nullptr)
    {
        out.transportValid = true;
        out.playing = transport->playing;
        out.bpm = transport->bpm;
        out.beatPosition = transport->beatPosition;
        out.transportReceivedAt = transport->transportReceivedAt;
        out.meterNumerator = transport->meterNumerator;
        out.meterDenominator = transport->meterDenominator;
        out.epoch = transport->epoch;
    }

    // Role resolution for this frame's events.
    const bool kickSource = out.roles[(size_t) SourceRole::kick].live;
    const bool snareSource = out.roles[(size_t) SourceRole::snare].live;
    const bool hatSource = out.roles[(size_t) SourceRole::hat].live;

    auto emit = [&] (EventType t, const PendingEvent& e) {
        out.events.add ({ t, e.strength, e.note, e.velocity });
        if (t == EventType::kick || t == EventType::bassTransient)
            lastImpactTime = now;
    };

    for (auto& e : pending)
    {
        if (now - e.time > eventExpirySeconds)
            continue; // expired: dropping a late hit beats flashing off-beat

        switch (e.rawType)
        {
            case EventType::userTrigger:
                emit (EventType::userTrigger, e);
                break;

            case EventType::midiNote:
                emit (EventType::midiNote, e);
                if (e.role == SourceRole::kick)  emit (EventType::kick, e);
                if (e.role == SourceRole::snare) emit (EventType::snare, e);
                if (e.role == SourceRole::hat)   emit (EventType::hat, e);
                break;

            case EventType::bassTransient:
            case EventType::midTransient:
            case EventType::highTransient:
            {
                const bool fromRoleSource = e.role == SourceRole::kick || e.role == SourceRole::snare || e.role == SourceRole::hat;

                if (fromRoleSource)
                {
                    if (e.midiRecent)
                        break; // MIDI is authoritative for this role
                    if (e.role == SourceRole::kick && e.rawType == EventType::bassTransient)  emit (EventType::kick, e);
                    if (e.role == SourceRole::snare && e.rawType == EventType::midTransient)  emit (EventType::snare, e);
                    if (e.role == SourceRole::hat && e.rawType == EventType::highTransient)   emit (EventType::hat, e);
                    break;
                }

                emit (e.rawType, e);
                if (e.rawType == EventType::bassTransient && ! kickSource) emit (EventType::kick, e);
                if (e.rawType == EventType::midTransient && ! snareSource) emit (EventType::snare, e);
                if (e.rawType == EventType::highTransient && ! hatSource)  emit (EventType::hat, e);
                break;
            }

            default:
                break;
        }
    }
    pending.clearQuick();

    if (demoEnabled && ! out.anyLive)
        synthesizeDemo (out, now);

    out.lastImpactTime = lastImpactTime;
    return out;
}

juce::Array<int> FeatureBus::getReplyPorts (double now) const
{
    const juce::ScopedLock sl (lock);
    juce::Array<int> ports;
    for (auto& [port, seen] : replyPorts)
        if (now - seen < 3.0)
            ports.add (port);
    return ports;
}

juce::String FeatureBus::describeSources (double now) const
{
    static const char* const roleNames[] = { "mix", "kick", "snare", "hat", "bass", "texture" };
    const juce::ScopedLock sl (lock);

    juce::StringArray parts;
    for (auto& [id, s] : sources)
        if (now - s.lastFrameTime < dropAfterSeconds)
            parts.add (juce::String (roleNames[(int) s.role]) + (s.name.isNotEmpty() ? " (" + s.name + ")" : juce::String()));

    if (parts.isEmpty())
        return demoEnabled ? "no sources (demo groove active)" : "no sources";

    return juce::String (parts.size()) + " source(s): " + parts.joinIntoString (", ");
}
