#include "Signals.h"

namespace
{
    const char* const eventNames[] = {
        "kick", "snare", "hat", "bassTransient", "midTransient", "highTransient", "midiNote", "userTrigger"
    };
}

const char* eventTypeName (EventType t) noexcept
{
    auto i = (int) t;
    return juce::isPositiveAndBelow (i, (int) EventType::count) ? eventNames[i] : "?";
}

bool parseEventName (const juce::String& name, EventType& out) noexcept
{
    auto bare = name.startsWith ("event.") ? name.substring (6) : name;

    for (int i = 0; i < (int) EventType::count; ++i)
    {
        if (bare == eventNames[i])
        {
            out = (EventType) i;
            return true;
        }
    }
    return false;
}

void SectionTracker::update (Signals& s, double dt) noexcept
{
    const double level = juce::jlimit (0.0f, 1.0f, s.levelAbs);
    if (! primed)
    {
        fast = slow = level;
        primed = true;
    }
    auto follow = [dt] (double& state, double target, double tau) { state += (target - state) * (1.0 - std::exp (-dt / tau)); };
    follow (fast, level, 4.0);
    follow (slow, level, 24.0);

    const double rise = (fast - slow) * 6.0 + juce::jmax (0.0f, s.energyTrend) * 0.5;
    const double target = juce::jlimit (0.0, 1.0, rise);
    follow (build, target, target > build ? 1.5 : 4.0);
    follow (presence, juce::jlimit (0.0f, 1.0f, s.levelRel), 2.5);

    s.build = (float) build;
    s.presence = (float) presence;
}

void applyReactMask (Signals& s, const ReactMask& mask)
{
    s.react = mask;
    s.events.removeIf ([&mask] (const SignalEvent& e) {
        switch (e.type)
        {
            case EventType::kick:
            case EventType::bassTransient: return ! mask.kick;
            case EventType::snare:
            case EventType::midTransient:  return ! mask.snare;
            case EventType::hat:
            case EventType::highTransient: return ! mask.hat;
            default:                       return false; // HIT button, MIDI notes
        }
    });
}

void Clock::update (const Signals& s, double now) noexcept
{
    const auto dt = lastNow < 0.0 ? 0.0 : juce::jlimit (0.0, 0.25, now - lastNow);
    lastNow = now;

    const auto previousBeat = beatPos;
    const auto previousBar = std::floor (beatPos / barLength);

    // A reported transport less than 2 s old and playing is authoritative.
    following = s.transportValid && s.playing && (now - s.transportReceivedAt) < 2.0;

    if (s.bpm > 20.0 && s.bpm < 400.0)
        tempo = s.bpm;

    if (s.meterDenominator > 0)
        barLength = juce::jmax (1.0, s.meterNumerator * 4.0 / s.meterDenominator);

    if (following)
    {
        auto target = s.beatPosition + (now - s.transportReceivedAt) * tempo / 60.0;

        // Snap on a seek/loop (epoch change or a big jump); otherwise glide
        // toward the reported position so packet jitter never jerks the phase.
        if (s.epoch != lastEpoch || std::abs (target - beatPos) > 0.5)
            beatPos = target;
        else
            beatPos += dt * tempo / 60.0 + (target - beatPos) * 0.2;

        lastEpoch = s.epoch;
    }
    else
    {
        beatPos += dt * tempo / 60.0; // free-run at the last known tempo
    }

    beatCrossed = std::floor (beatPos) != std::floor (previousBeat) && beatPos > previousBeat;
    barCrossed = std::floor (beatPos / barLength) != previousBar && beatPos > previousBeat;
}

double Clock::barPhase() const noexcept
{
    auto bars = beatPos / barLength;
    return bars - std::floor (bars);
}
