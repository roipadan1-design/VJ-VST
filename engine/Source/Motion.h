#pragma once

#include <JuceHeader.h>
#include <cmath>

// The performer's MOVE controls - global, the same for every scene
// (OSC /v2/move <slot> <0-1>):
//
//   0 Drift     slow camera drift over the whole picture (look pass); 0 = none
//   1 Push      how much the music speeds motion up (x1 .. x2)
//   2 Softness  length of every hit reaction (decays x0.5 .. x4)
//   3 Sync      0/1: speeds follow the tempo (x bpm / 120)
//   4 Reverse   0/1: motion runs backwards
//   5 Freeze    0/1: motion stops (film grain keeps running)
//
// Speed and Glide are macro slots 1 and 6, so they share the plug-in's macro
// knobs, automation and snapshots.
struct MoveSettings
{
    static constexpr int numSlots = 6;
    enum Slot { drift = 0, push, softness, sync, reverse, freeze };

    std::array<float, numSlots> values { 0.2f, 0.3f, 0.5f, 0.0f, 0.0f, 0.0f };

    float get (Slot s) const noexcept { return values[(size_t) s]; }
};

// What one frame of scene motion looks like to presets and shaders.
struct MotionFrame
{
    double speed = 1.0;      // signed speed multiplier (1 = the scene's designed speed)
    double sceneTime = 0.0;  // integral of speed: every scene's clock (shader uniform vj_time)
    double sceneDt = 0.0;    // this frame's step of sceneTime (speed x dt, signed)
    double sceneBeat = 0.0;  // host beats scaled by speed - what tempo LFOs run on
    float decayScale = 1.0f; // Softness: multiplies every hit envelope's decay
};

// The scene clock. Every moving thing in every scene runs on it instead of
// wall time, so one Speed knob really stops (or doubles) everything:
//   - integrated ("rate") parameters advance by rate x sceneDt
//   - shaders animate with vj_time / vj_dt instead of TIME / TIMEDELTA
//   - tempo LFOs run on sceneBeat
// Hits (envelopes) keep running on real time: a frozen picture still answers
// the kick, and hits never change speed.
class SceneClock
{
public:
    // 0 -> stopped, 0.5 -> designed speed, 1 -> x4 (quadratic below the
    // centre so the bottom of the knob is fine control for slow ambient).
    static float speedCurve (float knob) noexcept
    {
        knob = juce::jlimit (0.0f, 1.0f, knob);
        return knob <= 0.5f ? (2.0f * knob) * (2.0f * knob) : std::pow (4.0f, 2.0f * knob - 1.0f);
    }

    // Glide: how long speed changes take (inertia), 0 .. 4 bars.
    static double glideSeconds (float knob, double barSeconds) noexcept
    {
        knob = juce::jlimit (0.0f, 1.0f, knob);
        return 0.05 + (double) (knob * knob) * 4.0 * barSeconds;
    }

    // Softness knob -> decay multiplier (0 -> x0.5, 0.33 -> x1, 1 -> x4).
    static float decayScale (float knob) noexcept
    {
        return 0.5f * std::pow (8.0f, juce::jlimit (0.0f, 1.0f, knob));
    }

    // speedKnob / glideKnob: macro slots 1 and 6. drive: 0-1 musical energy
    // (already gated by REACT TO and Reactivity). beatDelta: host beats since
    // the last frame.
    void update (const MoveSettings& move, float speedKnob, float glideKnob, float drive,
                 double bpm, double barSeconds, double beatDelta, double dt) noexcept
    {
        auto target = (double) speedCurve (speedKnob);
        if (move.get (MoveSettings::sync) > 0.5f)
            target *= juce::jlimit (0.25, 2.0, bpm / 120.0);
        if (move.get (MoveSettings::reverse) > 0.5f)
            target = -target;
        if (move.get (MoveSettings::freeze) > 0.5f)
            target = 0.0;

        const auto tau = glideSeconds (glideKnob, barSeconds);
        base += (target - base) * (1.0 - std::exp (-dt / tau));
        if (std::abs (base) < 1.0e-4 && target == 0.0)
            base = 0.0;

        // Push: the music leans on the speed (never adds motion of its own,
        // so a stopped scene stays stopped). Quick to rise, slow to fall.
        const auto d = (double) juce::jlimit (0.0f, 1.0f, drive);
        const auto driveTau = d > smoothedDrive ? 0.06 : 0.4;
        smoothedDrive += (d - smoothedDrive) * (1.0 - std::exp (-dt / driveTau));
        const auto push = (double) juce::jlimit (0.0f, 1.0f, move.get (MoveSettings::push));

        frame.speed = base * (1.0 + push * smoothedDrive);
        frame.sceneDt = frame.speed * dt;
        frame.sceneTime += frame.sceneDt;
        frame.sceneBeat += juce::jlimit (0.0, 1.0, beatDelta) * frame.speed;
        frame.decayScale = decayScale (move.get (MoveSettings::softness));
    }

    const MotionFrame& getFrame() const noexcept { return frame; }

private:
    MotionFrame frame;
    double base = 1.0, smoothedDrive = 0.0;
};
