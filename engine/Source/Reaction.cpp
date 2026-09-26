#include "Reaction.h"

namespace
{
    // REACTION-DESIGN.md 4.3. Times in ms unless named otherwise.
    const ReactionStyle styles[] = {
        // BREATHE: the music breathes the image; only big moments land.
        { "Breathe", 0.80f, false, 2.0f,
          60.0f, 0.0f, 650.0f,   0.0f,   0.25f, 300.0f,   400.0f, 4.0f, 8.0f,
          150.0f, 900.0f, 0.8f,  600.0f, 2500.0f,  60.0f, 600.0f,
          2.0f, 0.025f,  0.25f,  0.6f, 400.0f,  0.0f, 40.0f,
          0.22f, 2.5f, 2.0f, 0.08f,  7.0f, 0.5f,  0.04f,  0.15f, false },
        // PULSE (default): every beat is felt, accents land.
        { "Pulse", 0.45f, true, 1.0f,
          0.0f, 50.0f, 240.0f,   0.45f,  0.50f, 120.0f,   0.0f, 1.0f, 4.0f,
          30.0f, 350.0f, 1.2f,   250.0f, 1200.0f,  30.0f, 350.0f,
          1.0f, 0.015f,  0.45f,  0.5f, 150.0f,  0.0f, 40.0f,
          0.32f, 1.5f, 1.2f, 0.06f,  6.0f, 0.8f,  0.06f,  0.30f, false },
        // PUNCH: tight hits, cut-ready.
        { "Punch", 0.20f, true, 0.5f,
          0.0f, 50.0f, 120.0f,   0.60f,  0.80f, 70.0f,    0.0f, 2.0f, 2.0f,
          10.0f, 180.0f, 1.6f,   120.0f, 700.0f,   15.0f, 200.0f,
          0.5f, 0.008f,  0.60f,  0.35f, 60.0f,  0.10f, 40.0f,
          0.45f, 1.0f, 0.6f, 0.03f,  5.0f, 1.0f,  0.08f,  0.40f, true },
    };

    double smoothstep01 (double x) noexcept
    {
        x = juce::jlimit (0.0, 1.0, x);
        return x * x * (3.0 - 2.0 * x);
    }

    // Kicks lead (they are what an audience feels as "the hit"); snares join
    // only in characters that ask for it (PUNCH) - otherwise they compete in
    // the ranking and take accents away from the kick.
    bool isCandidate (EventType t, bool snares) noexcept
    {
        return t == EventType::kick || t == EventType::midiNote || t == EventType::userTrigger || (snares && t == EventType::snare);
    }
}

const ReactionStyle& reactionStyle (int index) noexcept
{
    return styles[juce::jlimit (0, 2, index)];
}

float ReactCurves::contGain (float r) noexcept
{
    r = juce::jlimit (0.0f, 1.0f, r);
    return r <= 0.5f ? 1.0f - (1.0f - 2.0f * r) * (1.0f - 2.0f * r) : 1.0f + (r - 0.5f);
}

float ReactCurves::hitGain (float r) noexcept
{
    r = juce::jlimit (0.0f, 1.0f, r);
    if (r < 0.1f) return 0.0f;
    if (r <= 0.5f) return (float) smoothstep01 ((r - 0.1) / 0.4);
    return 1.0f + 1.6f * (r - 0.5f);
}

float ReactCurves::accentShift (float r) noexcept { return -0.5f * (juce::jlimit (0.0f, 1.0f, r) - 0.5f); }
float ReactCurves::pushFactor (float r) noexcept  { r = juce::jlimit (0.0f, 1.0f, r); return r < 0.5f ? 2.0f * r : 1.0f + 0.8f * (r - 0.5f); }
float ReactCurves::restDepth (float r) noexcept   { return juce::jmin (1.0f, juce::jlimit (0.0f, 1.0f, r) / 0.3f); }
float ReactCurves::dropShiftDb (float r) noexcept { return -4.0f * (juce::jlimit (0.0f, 1.0f, r) - 0.5f); }

float ReactionShaper::Stretch::process (float x, double dt, double floorUpTau, double ceilDownTau, float minSpan) noexcept
{
    if (! primed)
    {
        floor = x;
        ceil = x + minSpan;
        primed = true;
    }
    follow (floor, x, x < floor ? 0.3 : floorUpTau, dt);   // tracks the troughs
    follow (ceil, x, x > ceil ? 0.02 : ceilDownTau, dt);   // tracks the peaks
    const auto span = juce::jmax ((double) minSpan, ceil - floor);
    return (float) juce::jlimit (0.0, 1.0, (x - floor) / span);
}

void ReactionShaper::update (Signals& s, const Clock& clock, const Inputs& in, double dt, double now)
{
    const auto& st = reactionStyle (in.style);
    const auto r = juce::jlimit (0.0f, 1.0f, in.react);
    const auto bpm = juce::jlimit (20.0, 400.0, clock.bpm());
    const auto beatSeconds = 60.0 / bpm;
    const auto barSeconds = clock.barBeats() * beatSeconds;
    const auto& mask = s.react;

    kickBeats = 0.0;
    kickTau = st.kickTauMs * 0.001;

    // A manual HIT / DROP always lands, even with REACT at 0 or during CALM:
    // the performer asked for it.
    follow (manualBoost, 0.0, 0.6, dt);
    double contAmount = ReactCurves::contGain (r) * in.reactTrim * in.calmFade;
    double hitAmount = ReactCurves::hitGain (r) * in.reactTrim * in.calmFade;

    // --- continuous: auto-ranged against this song --------------------------
    const bool live = s.anyLive && s.levelAbs >= 0.15f; // about -53 dBFS
    const auto& kickRole = s.roles[(size_t) SourceRole::kick];
    const float bassIn = kickRole.live ? kickRole.levelRel : s.bassRel;

    auto shaped = [&] (double& state, float target, float attackMs, float releaseMs) {
        follow (state, target, (target > state ? attackMs : releaseMs) * 0.001, dt);
    };

    {
        const auto b = bodyStretch.process (bassIn, dt, 2.0, 6.0, 0.15f);
        const auto target = live && mask.bass ? std::pow (b, st.bodyGamma) : 0.0f;
        shaped (body, target, live ? st.bodyAttackMs : 300.0f, live ? st.bodyReleaseMs : 300.0f);
    }
    {
        const auto e = energyStretch.process (s.levelRel, dt, 20.0, 30.0, 0.2f);
        const auto target = live && mask.level ? 0.5f * s.levelRel + 0.5f * e : (mask.level ? 0.0f : 0.5f);
        shaped (energy, target, st.energyAttackMs, live ? st.energyReleaseMs : 300.0f);
    }
    {
        const auto a = airStretch.process (juce::jmax (s.highRel, s.flux), dt, 3.0, 8.0, 0.15f);
        const auto target = live && mask.hat ? a : 0.0f;
        shaped (air, target, st.airAttackMs, live ? st.airReleaseMs : 300.0f);
    }

    // Breath: an inhale toward each downbeat (period per character), deeper when loud.
    const auto periodBars = (double) st.breathBars;
    const auto bars = clock.beat() / juce::jmax (1.0, clock.barBeats());
    const auto phase = bars / periodBars - std::floor (bars / periodBars);
    const auto breath = (0.5 + 0.5 * std::cos (juce::MathConstants<double>::twoPi * phase)) * (0.25 + 0.75 * energy);

    // --- REST: silence as an instrument ---------------------------------------
    const bool sinceCandidateLong = now - lastCandidate > st.restEnterSeconds;
    const bool quiet = ! s.anyLive || s.levelAbs < 0.25f
                    || (s.levelRel < 0.25f && s.presence < 0.2f && sinceCandidateLong);
    quietSeconds = quiet ? quietSeconds + dt : 0.0;
    const bool resting = mask.level && quietSeconds >= st.restEnterSeconds;
    follow (rest, resting ? 1.0 : 0.0, resting ? st.restInSeconds : st.restOutSeconds, dt);
    if (rest > 0.5)
        lastRestTime = now;

    // --- tension (build-ups) --------------------------------------------------
    // Candidate rate, fast vs slow: rolls and fills read as tension too.
    const auto rateDecayFast = std::exp (-dt / 2.0), rateDecaySlow = std::exp (-dt / 16.0);
    candRateFast *= rateDecayFast;
    candRateSlow *= rateDecaySlow;
    follow (tensionCap, 1.0, 4.0 * barSeconds / 3.0, dt);
    {
        // The section tracker also rises when music simply starts (its slow
        // average lags): count it only after it has come back down once, so
        // "build-up" means a fresh rise - after the start, after every drop.
        if (s.build < 0.25f)
            tensionArmed = true;
        if (rest > 0.5)
            tensionArmed = false;
        const auto roll = juce::jlimit (0.0, 1.0, (candRateFast / 2.0 - candRateSlow / 16.0) * 0.5);
        const auto build = tensionArmed ? (double) s.build : 0.0;
        const auto raw = mask.level ? juce::jmin (tensionCap, juce::jmax (build, tensionArmed ? roll * 0.8 : 0.0)) : 0.0;
        follow (tension, raw, raw > tension ? 0.8 : barSeconds / 3.0, dt);
    }
    if (tension >= tensionPeak || now - lastTensionPeakTime > 4.0 * barSeconds)
    {
        tensionPeak = tension;
        lastTensionPeakTime = now;
    }

    // --- accents: not every hit -----------------------------------------------
    recentCandidates.removeIf ([now] (double t) { return now - t > 4.0; });
    const bool governor = now < governorUntil;
    const auto q = juce::jlimit (0.05f, 0.95f, st.accentQuantile + ReactCurves::accentShift (r) + (governor ? 0.15f : 0.0f));
    const auto minGap = juce::jmax (0.34, st.minGapBeats * beatSeconds);
    const auto barPos = clock.barPhase() * barSeconds;
    const bool onDownbeat = clock.isFollowingTransport() && (barPos < 0.07 || barSeconds - barPos < 0.07);

    juce::Array<SignalEvent> added;
    bool kickLike = false, manualHit = false;

    for (auto& e : s.events)
    {
        if (e.type == EventType::kick || e.type == EventType::bassTransient)
            kickLike = true;
        if (! isCandidate (e.type, st.snareAccents))
            continue;

        const auto strength = juce::jlimit (0.0f, 1.0f, e.strength);
        const bool manual = e.type == EventType::userTrigger;
        manualHit = manualHit || manual;

        if (manual || (e.type == EventType::midiNote && e.velocity >= 100))
        {
            added.add ({ EventType::accent, 1.0f, e.note, e.velocity });
            lastAccent = lastCandidate = now;
            continue;
        }

        // Rank against the last 16 candidates, then remember this one.
        int below = 0;
        for (int i = 0; i < ringCount; ++i)
            below += ring[(size_t) i] < strength ? 1 : 0;
        const auto rank = ringCount > 0 ? (float) below / (float) ringCount : 1.0f;
        ring[(size_t) ringNext] = strength;
        ringNext = (ringNext + 1) % (int) ring.size();
        ringCount = juce::jmin ((int) ring.size(), ringCount + 1);

        const bool afterGap = now - lastCandidate > juce::jmax (barSeconds, 2.0);
        const bool afterRest = rest > 0.5;
        const bool downbeat = st.downbeatAccents && onDownbeat && e.type == EventType::kick;
        const bool forced = afterGap || afterRest || downbeat;
        const bool gapOk = now - lastAccent >= minGap;

        recentCandidates.add (now);
        candRateFast += 1.0;
        candRateSlow += 1.0;
        lastCandidate = now;

        if ((forced || rank >= q) && gapOk)
        {
            added.add ({ EventType::accent, forced ? juce::jmax (strength, 0.7f) : 0.7f + 0.3f * rank, e.note, e.velocity });
            lastAccent = now;
        }
        else if (now - lastTick >= 0.34)
        {
            added.add ({ EventType::tick, strength * (governor ? 0.5f : 1.0f), e.note, e.velocity });
            lastTick = now;
        }
    }
    if (kickLike)
        lastKickLike = now;

    // Governor: a flood of candidates (a busy bass line read as kicks) raises
    // the bar for accents until it calms down - never a strobe.
    if (recentCandidates.size() > 12)
        governorUntil = now + 2.0;

    if (manualHit)
        manualBoost = 1.0;
    hitAmount = juce::jmax (hitAmount, manualBoost);

    // --- the DROP ---------------------------------------------------------------
    const double levelDb = s.levelAbs * 48.0 - 60.0;
    if (! levelsPrimed)
    {
        fastDb = slowDb = levelDb;
        levelsPrimed = true;
    }
    follow (fastDb, levelDb, 0.3, dt);
    follow (slowDb, levelDb, 8.0, dt);

    const bool primed = tensionPeak >= 0.35 || now - lastRestTime < 2.0 * barSeconds;
    const bool jumped = fastDb - slowDb >= st.dropJumpDb + ReactCurves::dropShiftDb (r);
    const bool spaced = now - lastDrop >= juce::jmax (8.0 * barSeconds, 12.0);
    const bool detected = mask.level && r >= 0.15f && primed && jumped && spaced && now - lastKickLike < 0.25;
    const bool manual = manualDrop.exchange (false);

    if (detected || manual)
    {
        added.add ({ EventType::drop, manual ? 1.0f : (float) juce::jlimit (0.6, 1.0, 0.6 + (fastDb - slowDb - st.dropJumpDb) / 6.0) });
        lastDrop = now;
        ++drops;
        tension = 0.0;
        tensionCap = 0.0;          // recovers over 4 bars
        tensionArmed = false;      // and needs a fresh rise
        rest = 0.0;
        quietSeconds = 0.0;
        dropElapsed = 0.0;
        dropValue = 1.0;
        if (manual)
            manualBoost = 1.0;
    }

    // Global drop envelope: attack, hold, then decay (half-life in beats).
    {
        dropElapsed += dt;
        const auto attack = st.dropAttackMs * 0.001;
        const auto hold = st.dropHoldBeats * beatSeconds;
        const auto halfLife = st.dropHalfLifeBeats * beatSeconds;
        if (dropElapsed < attack)
            dropValue = dropElapsed / juce::jmax (1.0e-3, attack);
        else if (dropElapsed < attack + hold)
            dropValue = 1.0;
        else
            dropValue = std::pow (0.5, (dropElapsed - attack - hold) / juce::jmax (1.0e-3, halfLife));
        if (dropValue < 0.001)
            dropValue = 0.0;
    }

    // Scars: accents leave marks through a section; rest heals, a drop wipes.
    for (auto& e : added)
        if (e.type == EventType::accent)
            scar = juce::jmin (1.0, scar + st.scarPerAccent * juce::jmin (1.0, hitAmount) * e.strength);
    {
        const bool wiping = now - lastDrop < 2.0 * barSeconds;
        const auto tau = wiping ? 2.0 * barSeconds / 3.0 : (rest > 0.5 ? 4.0 : 32.0 * barSeconds);
        follow (scar, 0.0, tau, dt);
    }

    // Time kicks: accents (and PUNCH ticks) push the scene clock forward.
    for (auto& e : added)
    {
        if (e.type == EventType::accent)
            kickBeats += st.kickBeats * juce::jmin (2.0, hitAmount) * e.strength;
        else if (e.type == EventType::tick && st.tickKickBeats > 0.0f)
            kickBeats += st.tickKickBeats * juce::jmin (2.0, hitAmount) * e.strength;
        else if (e.type == EventType::drop)
            kickBeats += 3.0 * st.kickBeats * juce::jmin (2.0, hitAmount);
        if (e.type == EventType::accent)
        {
            accentEnv = 1.0;
            ++accents;
        }
    }
    accentEnv *= std::pow (0.5, dt / 0.15);

    s.events.addArray (added);

    auto& out = s.reaction;
    out.body = (float) body;
    out.energy = (float) energy;
    out.air = (float) air;
    out.breath = (float) breath;
    out.tension = (float) tension;
    out.rest = (float) rest;
    out.scar = (float) scar;
    out.dropEnv = (float) dropValue;
    out.contAmount = (float) contAmount;
    out.hitAmount = (float) hitAmount;
    // Exposure: silence darkens to the rest floor; every accent opens up a
    // little (a short punch of light, every scene); a drop blooms.
    out.exposure = (float) ((1.0 - rest * ReactCurves::restDepth (r) * in.calmFade * (1.0 - st.restFloor))
                            * std::pow (2.0, (st.dropBloomStops * dropValue + st.accentStops * accentEnv) * juce::jmin (1.0, hitAmount)));
    out.accentEnv = (float) accentEnv;
    out.style = juce::jlimit (0, 2, in.style);
    out.accents = accents;
    out.drops = drops;
}
