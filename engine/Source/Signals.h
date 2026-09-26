#pragma once

#include <JuceHeader.h>
#include <array>

// Everything the render thread needs to know about the music for one frame,
// resolved from however many analysis sources are connected (FeatureBus).
// Continuous values are latest-wins snapshots; events are the discrete hits
// that arrived since the previous frame (already de-duplicated and expired).

enum class SourceRole { mix = 0, kick, snare, hat, bass, texture, count };

enum class EventType
{
    kick, snare, hat,                           // role-resolved (MIDI wins over audio)
    bassTransient, midTransient, highTransient, // raw full-mix detector output
    midiNote, userTrigger,
    // Written by the engine's ReactionShaper (Reaction.h), appended so the
    // indices above never move:
    accent,   // a hit selected as meaningful: the full event
    tick,     // a candidate hit that was not selected: a small echo
    drop,     // the section release (detected, or the performer's DROP)
    count
};

const char* eventTypeName (EventType t) noexcept;
bool parseEventName (const juce::String& name, EventType& out) noexcept; // "event.kick" or "kick"

struct SignalEvent
{
    EventType type = EventType::kick;
    float strength = 1.0f;
    int note = -1, velocity = 0;
};

struct RoleActivity
{
    bool live = false;
    float levelRel = 0.0f, levelAbs = 0.0f;
};

// The performer's REACT TO gate (plugin toggles, OSC /v2/react). Each audio
// signal belongs to exactly one channel:
//   kick   kick / bass-transient hits, kick-track activity
//   snare  snare / mid-transient hits, mid band, snare-track activity
//   hat    hat / high-transient hits, high band, hat-track activity
//   bass   the continuous low end (bass level, bands 0-1)
//   level  whole-mix loudness, brightness and build-ups (level, descriptors)
// A closed channel's hits are dropped before anything sees the frame, and
// routes reading its continuous signals glide back to neutral. The manual HIT
// (userTrigger), MIDI notes and the transport clock always pass.
struct ReactMask
{
    bool kick = true, snare = true, hat = true, bass = true, level = true;
    // Master amount of every audio-driven reaction (LOOK Reactivity x Calm),
    // already smoothed by the caller: 1 = as designed, 0 = only macros/LFOs move.
    float amount = 1.0f;
};

// The ReactionShaper's per-frame state (Reaction.h). 0-1 unless noted.
//   body     the bass, auto-ranged against this song (pulses read high)
//   energy   loudness around a neutral 0.5 (silence -> 0, full -> 1)
//   air      highs / flux, auto-ranged (surface texture)
//   breath   tempo-locked inhale toward each downbeat, x energy
//   tension  build-up (rises into a drop, reset by it)
//   rest     silence as an instrument: 1 = resting (darker, sparser)
//   scar     memory of accents through a section (healed by rest, wiped by a drop)
struct ReactionState
{
    float body = 0.0f, energy = 0.5f, air = 0.0f, breath = 0.5f;
    float tension = 0.0f, rest = 0.0f, scar = 0.0f;
    float dropEnv = 0.0f;                      // global drop envelope (look pass)
    float contAmount = 1.0f, hitAmount = 1.0f; // REACT curves x trim x CALM fade
    float exposure = 1.0f;                     // rest floor x drop bloom (look pass)
    float accentEnv = 0.0f;                    // decays after each accent (UI ring, lamps)
    int style = 1;                             // 0 BREATHE, 1 PULSE, 2 PUNCH
    int accents = 0, drops = 0;                // running counts (status)
};

struct Signals
{
    // Mix source (or the first live source if none declared "mix").
    bool anyLive = false;
    float levelRel = 0.0f, levelAbs = 0.0f;
    float bassRel = 0.0f, midRel = 0.0f, highRel = 0.0f;
    float bassAbs = 0.0f, midAbs = 0.0f, highAbs = 0.0f;
    std::array<float, 6> bandRel {};
    std::array<float, 32> spectrum {};
    float centroid = 0.0f, flatness = 0.0f, rolloff = 0.0f, flux = 0.0f, energyTrend = 0.0f;

    // Musical-section signals, computed in the engine (SectionTracker):
    //   build     0-1, rises through build-ups (short-term energy above the
    //             long-term average), falls slowly in breakdowns - react to the
    //             phrase, not to every hit
    //   presence  0-1, smoothed "how much is happening" (~2.5 s)
    float build = 0.0f, presence = 0.0f;

    std::array<RoleActivity, (size_t) SourceRole::count> roles {};

    // Transport, as last reported (the Clock extrapolates between reports).
    bool transportValid = false, playing = false;
    double bpm = 120.0;
    double beatPosition = 0.0;        // quarter notes at `transportReceivedAt`
    double transportReceivedAt = 0.0; // engine seconds
    int meterNumerator = 4, meterDenominator = 4;
    int epoch = 0;

    juce::Array<SignalEvent> events;

    ReactMask react; // which channels may drive the visuals this frame
    ReactionState reaction; // filled by the ReactionShaper after the mask

    // Seconds since the last kick-like hit (kick / bassTransient), for the
    // legacy "onset" uniform.
    double lastImpactTime = -1000.0;
};

// Derives build / presence from the running analysis (fast vs slow energy
// averages plus the analyser's energy trend), with asymmetric smoothing:
// builds register in ~1.5 s, releases take ~4 s.
class SectionTracker
{
public:
    void update (Signals&, double dt) noexcept;

private:
    double fast = 0.0, slow = 0.0, build = 0.0, presence = 0.0;
    bool primed = false, wasSilent = true;
};

// Stores the mask in the frame and drops the hits of closed channels.
void applyReactMask (Signals&, const ReactMask&);

// Musical time for the render thread. Follows the host transport when one is
// reporting (extrapolating between packets at the reported tempo), otherwise
// free-runs at the last known tempo so beat-synced motion never freezes.
class Clock
{
public:
    void update (const Signals& s, double nowSeconds) noexcept;

    double beat() const noexcept { return beatPos; }          // quarter notes
    double beatPhase() const noexcept { return beatPos - std::floor (beatPos); }
    double barBeats() const noexcept { return barLength; }    // quarter notes per bar
    double barPhase() const noexcept;
    double bpm() const noexcept { return tempo; }
    bool isFollowingTransport() const noexcept { return following; }

    // True once when the beat/bar counter advanced during the last update().
    bool crossedBeat() const noexcept { return beatCrossed; }
    bool crossedBar() const noexcept { return barCrossed; }

private:
    double beatPos = 0.0, lastNow = -1.0, tempo = 120.0, barLength = 4.0;
    bool following = false, beatCrossed = false, barCrossed = false;
    int lastEpoch = -1;
};
