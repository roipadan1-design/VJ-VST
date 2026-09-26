#pragma once

#include <JuceHeader.h>
#include <array>
#include "Signals.h"

// The reaction layer (docs/product-design/REACTION-DESIGN.md): turns the raw
// analysis into something that *reads* as a reaction.
//   - contrast: continuous signals are auto-ranged against this song, and
//     silence is a state of its own (REST: darker, sparser, never dead)
//   - selection: only meaningful hits become full events (event.accent);
//     the others are small echoes (event.tick) or nothing
//   - persistence: accents nudge the scene clock forward (time kick) and
//     leave scars through a section
//   - anticipation and release: tension through build-ups, one DROP event
// The performer plays it with one amount (REACT, macro slot 4) and a
// character (BREATHE / PULSE / PUNCH, OSC /v2/style).

// One character: every number that shapes how the picture follows the music.
struct ReactionStyle
{
    const char* name;
    float accentQuantile;       // candidates ranked below this (of the last 16) become ticks
    bool downbeatAccents;       // a kick on the bar's downbeat is always an accent
    float minGapBeats;          // between accents (never under 0.34 s)
    float hitAttackMs, hitHoldMs, hitHalfLifeMs;
    float tickLevel;            // ticks retrigger the hit envelope at this level (0 = none)
    float snareLevel, snareHalfLifeMs;
    float dropAttackMs, dropHoldBeats, dropHalfLifeBeats;
    float bodyAttackMs, bodyReleaseMs, bodyGamma;
    float energyAttackMs, energyReleaseMs;
    float airAttackMs, airReleaseMs;
    float breathBars, breathZoom;
    float pushC;                // how hard the music leans on the speed (0-1)
    float kickBeats, kickTauMs; // time kick per accent: beats of designed motion, ease
    float tickKickBeats, tickKickTauMs;
    float restFloor, restEnterSeconds, restInSeconds, restOutSeconds;
    float dropJumpDb, dropBloomStops;
    float scarPerAccent;
    float accentStops;          // exposure punch on each accent (stops, decays in ~0.15 s)
    bool snareAccents;          // snares compete with kicks for accents
};

const ReactionStyle& reactionStyle (int index) noexcept; // 0 BREATHE, 1 PULSE, 2 PUNCH (clamped)

// REACT (0-1) -> internal amounts. 0 = still, 0.5 = as designed, 1 = wild.
struct ReactCurves
{
    static float contGain (float r) noexcept;     // continuous routes, Push drive
    static float hitGain (float r) noexcept;      // hit envelopes, time kicks, look events
    static float accentShift (float r) noexcept;  // added to the accent quantile
    static float pushFactor (float r) noexcept;
    static float restDepth (float r) noexcept;
    static float dropShiftDb (float r) noexcept;
};

class ReactionShaper
{
public:
    struct Inputs
    {
        float react = 0.5f;      // macro slot 4
        int style = 1;
        float reactTrim = 1.0f;  // the old Reactivity knob (expert trim)
        float calmFade = 1.0f;   // CALM: 1 -> 0 over one bar, already smoothed
    };

    // Call once per frame after applyReactMask and Clock::update. Appends
    // accent / tick / drop events to s.events and fills s.reaction.
    void update (Signals& s, const Clock& clock, const Inputs& in, double dt, double now);

    // The performer's DROP (plug-in action, OSC /v2/trigger drop): fires on
    // the next frame whatever the detector thinks. Any thread.
    void requestDrop() noexcept { manualDrop = true; }

    // Time kicks produced this frame (for SceneClock::kick): beats of designed
    // motion and the ease time constant. Zero when nothing fired.
    double getKickBeats() const noexcept { return kickBeats; }
    double getKickTauSeconds() const noexcept { return kickTau; }

private:
    struct Stretch
    {
        double floor = 0.0, ceil = 0.0;
        bool primed = false;
        float process (float x, double dt, double floorUpTau, double ceilDownTau, float minSpan) noexcept;
    };

    static void follow (double& state, double target, double tau, double dt) noexcept
    {
        state += (target - state) * (tau <= 0.0 ? 1.0 : 1.0 - std::exp (-dt / tau));
    }

    std::atomic<bool> manualDrop { false };

    Stretch bodyStretch, energyStretch, airStretch;
    double body = 0.0, energy = 0.5, air = 0.0;
    double tension = 0.0, rest = 0.0, scar = 0.0;
    double quietSeconds = 0.0;
    double fastDb = -60.0, slowDb = -60.0;
    bool levelsPrimed = false;

    std::array<float, 16> ring {};
    int ringCount = 0, ringNext = 0;
    double lastCandidate = -1000.0, lastAccent = -1000.0, lastTick = -1000.0, lastKickLike = -1000.0;
    double lastDrop = -1000.0, lastRestTime = -1000.0, lastTensionPeakTime = -1000.0;
    double tensionPeak = 0.0;
    juce::Array<double> recentCandidates;       // times within the last 4 s (governor)
    double candRateFast = 0.0, candRateSlow = 0.0;
    double governorUntil = -1000.0;
    double tensionCap = 1.0;                    // held down after a drop, recovers over 4 bars
    bool tensionArmed = false;                  // the build signal fell back once since the start / last drop

    // Global drop envelope: attack, hold, then exponential decay.
    double dropValue = 0.0, dropElapsed = 1000.0;
    double accentEnv = 0.0, manualBoost = 0.0;

    double kickBeats = 0.0, kickTau = 0.1;
    int accents = 0, drops = 0;
};
