#include "Modulation.h"

namespace
{
    float hash01 (int a, int b) noexcept
    {
        auto h = (uint32_t) a * 374761393u + (uint32_t) b * 668265263u;
        h = (h ^ (h >> 13)) * 1274126177u;
        return (float) ((h ^ (h >> 16)) & 0xffffff) / (float) 0xffffff;
    }

    float smoothingCoefficient (double dt, float ms) noexcept
    {
        return ms <= 0.0f ? 0.0f : (float) std::exp (-dt * 1000.0 / ms);
    }
}

void ModulationRuntime::Envelope::trigger (float amount) noexcept
{
    auto newPeak = juce::jlimit (0.0f, 1.0f, amount * peak);

    // "max" retrigger never pulls a running envelope down; "restart" always
    // starts a fresh attack from zero.
    if (retriggerMax && value >= newPeak)
        return;

    start = retriggerMax ? value : 0.0f;
    target = newPeak;
    elapsed = 0.0f;
    attacking = true;
}

void ModulationRuntime::Envelope::advance (float dt) noexcept
{
    elapsed += dt;

    if (attacking)
    {
        if (attackSeconds <= 0.0f || elapsed >= attackSeconds)
        {
            attacking = false;
            elapsed = attackSeconds <= 0.0f ? elapsed : elapsed - attackSeconds;
            value = target;
        }
        else
        {
            value = start + (target - start) * (elapsed / attackSeconds);
            return;
        }
    }

    // Exponential decay reaching ~1% of the peak at decaySeconds, then zero.
    value = target * std::exp (-4.6f * elapsed / decaySeconds);
    if (value < 0.001f)
        value = 0.0f;
}

ModulationRuntime::ModulationRuntime (const PresetV2& p) : preset (p)
{
    seed = p.seed;

    for (auto& stage : p.stages)
    {
        juce::Array<float> v, b, acc;
        juce::Array<double> integral;
        for (auto& param : stage.parameters)
        {
            v.add (param.defaultValue);
            b.add ((param.defaultValue - param.min) / (param.max - param.min));
            acc.add (0.0f);
            integral.add (0.0);
        }
        integrals.add (integral);
        values.add (v);
        base01.add (b);
        accumulators.add (acc);
    }

    for (auto& m : p.modulators)
    {
        Envelope env;
        env.attackSeconds = m.attackMs * 0.001f;
        env.decaySeconds = m.decayMs * 0.001f;
        env.peak = m.peak;
        env.retriggerMax = m.retriggerMax;
        envelopes.add (env);

        Lfo lfo;
        lfo.shape = m.shape;
        lfo.periodBeats = m.periodBeats;
        lfo.phaseOffset = m.phaseOffset;
        lfos.add (lfo);
    }

    auto indexOf = [] (auto& items, const juce::String& id) {
        for (int i = 0; i < items.size(); ++i)
            if (items.getReference (i).id == id)
                return i;
        return -1;
    };

    for (auto& def : p.routes)
    {
        ResolvedRoute r;
        r.def = def;

        auto parts = juce::StringArray::fromTokens (def.destination, ".", "");
        r.stage = indexOf (p.stages, parts[1]);
        for (int i = 0; i < p.stages.getReference (r.stage).parameters.size(); ++i)
            if (p.stages.getReference (r.stage).parameters.getReference (i).name == parts[2])
                r.parameter = i;

        const auto& s = def.source;
        auto rest = s.fromFirstOccurrenceOf (".", false, false);

        if (s.startsWith ("macro."))      { r.kind = SourceKind::macro; r.index = indexOf (p.macros, rest); }
        else if (s.startsWith ("env."))   { r.kind = SourceKind::envelope; r.index = indexOf (p.modulators, rest); }
        else if (s.startsWith ("lfo."))   { r.kind = SourceKind::lfo; r.index = indexOf (p.modulators, rest); }
        else if (s == "audio.level.activity") r.kind = SourceKind::levelRel;
        else if (s == "audio.level.absolute") r.kind = SourceKind::levelAbs;
        else if (s == "audio.bass.activity")  r.kind = SourceKind::bassRel;
        else if (s == "audio.mid.activity")   r.kind = SourceKind::midRel;
        else if (s == "audio.high.activity")  r.kind = SourceKind::highRel;
        else if (s == "audio.bass.absolute")  r.kind = SourceKind::bassAbs;
        else if (s == "audio.mid.absolute")   r.kind = SourceKind::midAbs;
        else if (s == "audio.high.absolute")  r.kind = SourceKind::highAbs;
        else if (s == "audio.kick.activity")  r.kind = SourceKind::kickActivity;
        else if (s == "audio.snare.activity") r.kind = SourceKind::snareActivity;
        else if (s == "audio.hat.activity")   r.kind = SourceKind::hatActivity;
        else if (s.startsWith ("audio.band") && s.endsWith (".activity"))
        {
            r.kind = SourceKind::band;
            r.index = juce::jlimit (0, 5, s.substring (10, 11).getIntValue());
        }
        else if (s == "descriptor.centroid")    r.kind = SourceKind::centroid;
        else if (s == "descriptor.flatness")    r.kind = SourceKind::flatness;
        else if (s == "descriptor.rolloff")     r.kind = SourceKind::rolloff;
        else if (s == "descriptor.flux")        r.kind = SourceKind::flux;
        else if (s == "descriptor.energyTrend") r.kind = SourceKind::energyTrend;
        else if (s == "clock.beatPhase")        r.kind = SourceKind::beatPhase;
        else if (s == "clock.barPhase")         r.kind = SourceKind::barPhase;
        else
            warnings.add ("route '" + def.id + "': unknown source '" + s + "' reads as 0");

        routes.push_back (r);
    }

    for (auto& def : p.triggers)
    {
        ResolvedTrigger t;
        t.def = def;
        for (auto& action : def.actions)
            t.envelopeTargets.add (action.type == V2Action::Type::envelope ? indexOf (p.modulators, action.target) : -1);
        triggers.push_back (t);
    }
}

float ModulationRuntime::readSource (const ResolvedRoute& r, const Signals& s, const Clock& clock, const MacroBank& macros) const noexcept
{
    const auto& kick = s.roles[(size_t) SourceRole::kick];
    const auto& snare = s.roles[(size_t) SourceRole::snare];
    const auto& hat = s.roles[(size_t) SourceRole::hat];

    switch (r.kind)
    {
        case SourceKind::levelRel: return s.levelRel;
        case SourceKind::levelAbs: return s.levelAbs;
        // sourcePolicy: a dedicated role source replaces the mix band when present.
        case SourceKind::bassRel:  return preset.bassFromKick && kick.live ? kick.levelRel : s.bassRel;
        case SourceKind::midRel:   return preset.midFromSnare && snare.live ? snare.levelRel : s.midRel;
        case SourceKind::highRel:  return preset.highFromHat && hat.live ? hat.levelRel : s.highRel;
        case SourceKind::bassAbs:  return preset.bassFromKick && kick.live ? kick.levelAbs : s.bassAbs;
        case SourceKind::midAbs:   return preset.midFromSnare && snare.live ? snare.levelAbs : s.midAbs;
        case SourceKind::highAbs:  return preset.highFromHat && hat.live ? hat.levelAbs : s.highAbs;
        case SourceKind::kickActivity:  return kick.live ? kick.levelRel : s.bassRel;
        case SourceKind::snareActivity: return snare.live ? snare.levelRel : s.midRel;
        case SourceKind::hatActivity:   return hat.live ? hat.levelRel : s.highRel;
        case SourceKind::band:        return s.bandRel[(size_t) r.index];
        case SourceKind::centroid:    return s.centroid;
        case SourceKind::flatness:    return s.flatness;
        case SourceKind::rolloff:     return s.rolloff;
        case SourceKind::flux:        return s.flux;
        case SourceKind::energyTrend: return s.energyTrend;
        case SourceKind::beatPhase:   return (float) clock.beatPhase();
        case SourceKind::barPhase:    return (float) clock.barPhase();
        case SourceKind::macro:
        {
            if (r.index < 0) return 0.0f;
            auto& m = preset.macros.getReference (r.index);
            return macros.set[(size_t) m.slot] ? macros.values[(size_t) m.slot] : m.defaultValue;
        }
        case SourceKind::envelope: return r.index >= 0 ? envelopes.getReference (r.index).value : 0.0f;
        case SourceKind::lfo:      return r.index >= 0 ? lfos.getReference (r.index).value : 0.5f;
        case SourceKind::zero:
        default:                   return 0.0f;
    }
}

void ModulationRuntime::fireTrigger (ResolvedTrigger& t, float strength)
{
    for (int a = 0; a < t.def.actions.size(); ++a)
    {
        auto& action = t.def.actions.getReference (a);

        switch (action.type)
        {
            case V2Action::Type::envelope:
                if (auto target = t.envelopeTargets[a]; target >= 0)
                    envelopes.getReference (target).trigger (action.amount * (0.4f + 0.6f * strength));
                break;
            case V2Action::Type::paletteAdvance:
                paletteTarget += (float) action.steps;
                break;
            case V2Action::Type::reseed:
                ++seed;
                break;
        }
    }
}

void ModulationRuntime::process (const Signals& signals, const Clock& clock, const MacroBank& macros, double dt, double now)
{
    // 1. Triggers.
    for (auto& t : triggers)
    {
        for (auto& e : signals.events)
        {
            if (e.type != t.def.on || e.strength < t.def.minStrength)
                continue;
            if ((now - t.lastFired) * 1000.0 < t.def.refractoryMs)
                continue;

            t.lastFired = now;
            if (t.def.quantize == Quantize::none)
                fireTrigger (t, e.strength);
            else
            {
                t.pending = true;
                t.pendingStrength = juce::jmax (t.pendingStrength, e.strength);
            }
        }

        bool boundary = (t.def.quantize == Quantize::beat && clock.crossedBeat())
                     || (t.def.quantize == Quantize::bar && clock.crossedBar());
        if (t.pending && boundary)
        {
            fireTrigger (t, t.pendingStrength);
            t.pending = false;
            t.pendingStrength = 0.0f;
        }
    }

    // 2. Modulators.
    for (int i = 0; i < preset.modulators.size(); ++i)
    {
        auto& def = preset.modulators.getReference (i);

        if (def.type == V2Modulator::Type::ad)
        {
            envelopes.getReference (i).advance ((float) dt);
            continue;
        }

        auto& lfo = lfos.getReference (i);
        auto position = clock.beat() / lfo.periodBeats + lfo.phaseOffset;
        auto phase = (float) (position - std::floor (position));

        switch (lfo.shape)
        {
            case V2Modulator::Shape::sine:       lfo.value = 0.5f + 0.5f * std::sin (phase * juce::MathConstants<float>::twoPi); break;
            case V2Modulator::Shape::triangle:   lfo.value = 1.0f - std::abs (2.0f * phase - 1.0f); break;
            case V2Modulator::Shape::ramp:       lfo.value = phase; break;
            case V2Modulator::Shape::sampleHold: lfo.value = hash01 (seed + i * 7919, (int) std::floor (position)); break;
        }
    }

    auto paletteCoeff = (float) std::exp (-dt / 0.35);
    paletteValue = paletteCoeff * paletteValue + (1.0f - paletteCoeff) * paletteTarget;

    // 3-4. Routes into per-parameter accumulators.
    for (auto& acc : accumulators)
        for (auto& a : acc)
            a = 0.0f;

    for (auto& r : routes)
    {
        auto raw = readSource (r, signals, clock, macros);
        auto u = juce::jlimit (0.0f, 1.0f, (raw - r.def.inputMin) / (r.def.inputMax - r.def.inputMin));

        float q = u;
        if (r.def.curve == V2Route::Curve::power)           q = std::pow (u, r.def.exponent);
        else if (r.def.curve == V2Route::Curve::smoothstep) q = u * u * (3.0f - 2.0f * u);

        if (! r.initialised)
        {
            r.smoothed = q; // start at the first valid value, never sweep in from zero
            r.initialised = true;
        }
        else
        {
            auto coeff = smoothingCoefficient (dt, q > r.smoothed ? r.def.attackMs : r.def.releaseMs);
            r.smoothed = coeff * r.smoothed + (1.0f - coeff) * q;
        }

        accumulators.getReference (r.stage).getReference (r.parameter) += r.def.amount * (r.smoothed - r.def.center);
    }

    // 5-6. Clamp (or wrap) once, then to physical units.
    for (int s = 0; s < preset.stages.size(); ++s)
    {
        auto& params = preset.stages.getReference (s).parameters;
        for (int p = 0; p < params.size(); ++p)
        {
            auto& def = params.getReference (p);
            auto v = base01.getReference (s)[p] + accumulators.getReference (s)[p];
            v = def.cyclic ? v - std::floor (v) : juce::jlimit (0.0f, 1.0f, v);
            auto physical = def.min + v * (def.max - def.min);

            if (def.integrate)
            {
                auto& phase = integrals.getReference (s).getReference (p);
                phase += physical * dt;
                physical = (float) phase;
            }
            values.getReference (s).set (p, physical);
        }
    }
}

bool ModulationRuntime::setBaseValue (int stageIndex, const juce::String& name, float physical)
{
    if (! juce::isPositiveAndBelow (stageIndex, preset.stages.size()))
        stageIndex = 0;

    auto& params = preset.stages.getReference (stageIndex).parameters;
    for (int p = 0; p < params.size(); ++p)
    {
        auto& def = params.getReference (p);
        if (def.name == name)
        {
            base01.getReference (stageIndex).set (p, juce::jlimit (0.0f, 1.0f, (physical - def.min) / (def.max - def.min)));
            return true;
        }
    }
    return false;
}
