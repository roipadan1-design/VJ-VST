#include "PresetV2.h"

namespace
{
    float num (const juce::var& obj, const char* key, float fallback)
    {
        auto v = obj.getProperty (key, juce::var());
        return v.isVoid() ? fallback : (float) (double) v;
    }

    juce::String str (const juce::var& obj, const char* key, const juce::String& fallback = {})
    {
        auto v = obj.getProperty (key, juce::var());
        return v.isVoid() ? fallback : v.toString();
    }

    Quantize parseQuantize (const juce::String& q)
    {
        if (q == "beat") return Quantize::beat;
        if (q == "bar")  return Quantize::bar;
        return Quantize::none;
    }

    template <typename T>
    bool uniqueIds (const juce::Array<T>& items, const juce::String& what, juce::String& error)
    {
        juce::StringArray seen;
        for (auto& item : items)
        {
            if (item.id.isEmpty() || seen.contains (item.id))
            {
                error = what + " id '" + item.id + "' is empty or duplicated";
                return false;
            }
            seen.add (item.id);
        }
        return true;
    }
}

bool PresetV2::isSchema2 (const juce::var& json)
{
    return json.getProperty ("schemaVersion", juce::var()).toString().startsWith ("2");
}

bool PresetV2::parse (const juce::var& json, PresetV2& p, juce::String& error)
{
    if (! isSchema2 (json))
    {
        error = "not a schemaVersion 2 preset";
        return false;
    }

    p.id = str (json, "id");
    p.name = str (json, "name", p.id);
    p.description = str (json, "description");
    p.seed = (int) num (json.getProperty ("performance", juce::var()), "deterministicSeed", 1.0f);

    auto policy = json.getProperty ("sourcePolicy", juce::var());
    p.bassFromKick = str (policy, "bass", "kick-or-mix") == "kick-or-mix";
    p.midFromSnare = str (policy, "mid", "snare-or-mix") == "snare-or-mix";
    p.highFromHat = str (policy, "high", "hat-or-mix") == "hat-or-mix";

    if (auto* stages = json.getProperty ("stages", juce::var()).getArray())
    {
        for (auto& s : *stages)
        {
            V2Stage stage;
            stage.id = str (s, "id");
            stage.shader = str (s, "shader");
            stage.generator = str (s, "kind", "generator") == "generator";
            stage.floatTarget = str (s, "targetFormat", "rgba16f") != "rgba8";
            stage.scale = juce::jlimit (0.1f, 1.0f, num (s, "scale", 1.0f));

            if (stage.shader.isEmpty() || stage.shader.contains ("..") || juce::File::isAbsolutePath (stage.shader))
            {
                error = "stage '" + stage.id + "' has a missing or unsafe shader path";
                return false;
            }

            if (auto* params = s.getProperty ("parameters", juce::var()).getArray())
            {
                for (auto& pv : *params)
                {
                    V2Parameter param;
                    param.name = str (pv, "name");
                    param.label = str (pv, "label", param.name);
                    param.min = num (pv, "min", 0.0f);
                    param.max = num (pv, "max", 1.0f);
                    param.defaultValue = num (pv, "default", param.min);
                    param.cyclic = (bool) pv.getProperty ("cyclic", false);
                    param.integrate = (bool) pv.getProperty ("integrate", false);

                    if (! (param.max > param.min) || param.defaultValue < param.min || param.defaultValue > param.max)
                    {
                        error = "parameter '" + stage.id + "." + param.name + "' has an invalid range/default";
                        return false;
                    }
                    stage.parameters.add (param);
                }
            }

            if (auto* sourcesObj = s.getProperty ("sources", juce::var()).getDynamicObject())
            {
                for (auto& prop : sourcesObj->getProperties())
                {
                    V2Source src;
                    auto& sv = prop.value;
                    src.input = prop.name.toString();
                    src.type = str (sv, "type", "images") == "text" ? V2Source::Type::text : V2Source::Type::images;
                    src.folder = str (sv, "folder");
                    src.font = str (sv, "font", "Arial Black");
                    if (auto* words = sv.getProperty ("words", juce::var()).getArray())
                        for (auto& w : *words)
                            src.words.add (w.toString());
                    src.every = juce::jmax (1, (int) num (sv, "every", 1.0f));
                    src.random = str (sv, "order", "sequence") == "random";

                    auto advance = str (sv, "advance", "bar");
                    if (advance == "none")       src.advance = V2Source::Advance::none;
                    else if (advance == "beat")  src.advance = V2Source::Advance::beat;
                    else if (advance == "bar")   src.advance = V2Source::Advance::bar;
                    else if (parseEventName (advance, src.event)) src.advance = V2Source::Advance::event;
                    else
                    {
                        error = "source '" + src.input + "' has unknown advance '" + advance + "'";
                        return false;
                    }

                    if ((src.type == V2Source::Type::images && (src.folder.isEmpty() || src.folder.contains ("..")))
                        || (src.type == V2Source::Type::text && src.words.isEmpty()))
                    {
                        error = "source '" + src.input + "' needs a safe folder (images) or words (text)";
                        return false;
                    }
                    stage.sources.add (src);
                }
            }

            p.stages.add (stage);
        }
    }

    if (p.stages.isEmpty() || ! p.stages.getReference (0).generator)
    {
        error = "a preset needs at least one stage, and the first must be a generator";
        return false;
    }

    if (auto* macros = json.getProperty ("macros", juce::var()).getArray())
    {
        for (auto& m : *macros)
        {
            V2Macro macro;
            macro.id = str (m, "id");
            macro.label = str (m, "label", macro.id);
            macro.slot = (int) num (m, "slot", 0.0f);
            macro.defaultValue = juce::jlimit (0.0f, 1.0f, num (m, "default", 0.5f));

            if (! juce::isPositiveAndBelow (macro.slot, 8))
            {
                error = "macro '" + macro.id + "' slot must be 0-7";
                return false;
            }
            p.macros.add (macro);
        }
    }

    if (auto* mods = json.getProperty ("modulators", juce::var()).getArray())
    {
        for (auto& m : *mods)
        {
            V2Modulator mod;
            mod.id = str (m, "id");
            auto type = str (m, "type");

            if (type == "ad")
            {
                mod.type = V2Modulator::Type::ad;
                mod.attackMs = juce::jmax (0.0f, num (m, "attackMs", 5.0f));
                mod.decayMs = juce::jmax (1.0f, num (m, "decayMs", 200.0f));
                mod.peak = juce::jlimit (0.0f, 1.0f, num (m, "peak", 1.0f));
                mod.retriggerMax = str (m, "retrigger", "max") == "max";
            }
            else if (type == "lfo")
            {
                mod.type = V2Modulator::Type::lfo;
                auto shape = str (m, "shape", "sine");
                mod.shape = shape == "triangle" ? V2Modulator::Shape::triangle
                          : shape == "ramp" ? V2Modulator::Shape::ramp
                          : shape == "sample-hold" ? V2Modulator::Shape::sampleHold
                          : V2Modulator::Shape::sine;
                mod.periodBeats = juce::jmax (0.0625, (double) num (m, "periodBeats", 4.0f));
                mod.phaseOffset = num (m, "phaseOffset", 0.0f);
                mod.bipolar = str (m, "polarity", "bipolar") == "bipolar";
            }
            else
            {
                error = "modulator '" + mod.id + "' has unknown type '" + type + "'";
                return false;
            }
            p.modulators.add (mod);
        }
    }

    if (auto* routes = json.getProperty ("routes", juce::var()).getArray())
    {
        for (auto& r : *routes)
        {
            V2Route route;
            route.id = str (r, "id");
            route.source = str (r, "source");
            route.destination = str (r, "destination");
            route.inputMin = num (r, "inputMin", 0.0f);
            route.inputMax = num (r, "inputMax", 1.0f);
            auto curve = str (r, "curve", "linear");
            route.curve = curve == "power" ? V2Route::Curve::power
                        : curve == "smoothstep" ? V2Route::Curve::smoothstep
                        : V2Route::Curve::linear;
            route.exponent = juce::jlimit (0.1f, 8.0f, num (r, "exponent", 1.0f));
            route.center = num (r, "center", 0.0f);
            route.amount = juce::jlimit (-2.0f, 2.0f, num (r, "amount", 0.0f));
            route.attackMs = juce::jmax (0.0f, num (r, "attackMs", 0.0f));
            route.releaseMs = juce::jmax (0.0f, num (r, "releaseMs", 0.0f));

            if (! (route.inputMax > route.inputMin))
            {
                error = "route '" + route.id + "' needs inputMin < inputMax";
                return false;
            }
            p.routes.add (route);
        }
    }

    if (auto* triggers = json.getProperty ("triggers", juce::var()).getArray())
    {
        for (auto& t : *triggers)
        {
            V2Trigger trig;
            trig.id = str (t, "id");
            if (! parseEventName (str (t, "on"), trig.on))
            {
                error = "trigger '" + trig.id + "' listens to unknown event '" + str (t, "on") + "'";
                return false;
            }
            trig.minStrength = num (t, "minStrength", 0.0f);
            trig.refractoryMs = num (t, "refractoryMs", 0.0f);
            trig.quantize = parseQuantize (str (t, "quantize", "none"));

            if (auto* actions = t.getProperty ("actions", juce::var()).getArray())
            {
                for (auto& a : *actions)
                {
                    V2Action action;
                    auto type = str (a, "type");
                    action.type = type == "palette-advance" ? V2Action::Type::paletteAdvance
                                : type == "reseed" ? V2Action::Type::reseed
                                : V2Action::Type::envelope;
                    action.target = str (a, "target");
                    action.amount = juce::jlimit (0.0f, 1.0f, num (a, "amount", 1.0f));
                    action.steps = (int) num (a, "steps", 1.0f);
                    trig.actions.add (action);
                }
            }
            p.triggers.add (trig);
        }
    }

    auto transition = json.getProperty ("transition", juce::var());
    auto ttype = str (transition, "type", "crossfade");
    p.transition.type = ttype == "cut" ? V2Transition::Type::cut
                      : ttype == "dip-to-background" ? V2Transition::Type::dipToBackground
                      : ttype == "luma-wipe" ? V2Transition::Type::lumaWipe
                      : V2Transition::Type::crossfade;
    p.transition.durationMs = juce::jmax (0.0f, num (transition, "durationMs", 600.0f));
    p.transition.quantize = parseQuantize (str (transition, "quantize", "none"));

    auto post = json.getProperty ("post", juce::var());
    auto bloom = post.getProperty ("bloom", juce::var());
    p.post.bloom = (bool) bloom.getProperty ("enabled", true);
    p.post.bloomAmount = juce::jlimit (0.0f, 4.0f, num (bloom, "amount", 0.2f));
    p.post.bloomThreshold = juce::jlimit (0.0f, 16.0f, num (bloom, "threshold", 1.0f));
    p.post.bloomLevels = juce::jlimit (1, 6, (int) num (bloom, "levels", 4.0f));
    p.post.reinhard = str (post, "toneMap", "reinhard") == "reinhard";
    p.post.exposureEv = juce::jlimit (-8.0f, 8.0f, num (post, "exposureEv", 0.0f));
    p.post.grain = juce::jlimit (0.0f, 0.2f, num (post, "grain", 0.0f));
    p.post.vignette = juce::jlimit (0.0f, 1.0f, num (post, "vignette", 0.0f));

    // Cross-reference validation (what JSON Schema alone can't check).
    if (! uniqueIds (p.stages, "stage", error) || ! uniqueIds (p.macros, "macro", error)
        || ! uniqueIds (p.modulators, "modulator", error) || ! uniqueIds (p.routes, "route", error)
        || ! uniqueIds (p.triggers, "trigger", error))
        return false;

    for (auto& route : p.routes)
    {
        auto parts = juce::StringArray::fromTokens (route.destination, ".", "");
        const V2Stage* stage = nullptr;
        for (auto& s : p.stages)
            if (parts.size() == 3 && s.id == parts[1])
                stage = &s;

        bool paramFound = false;
        if (stage != nullptr && parts[0] == "stage")
            for (auto& param : stage->parameters)
                paramFound = paramFound || param.name == parts[2];

        if (! paramFound)
        {
            error = "route '" + route.id + "' targets unknown destination '" + route.destination + "'";
            return false;
        }

        auto prefix = route.source.upToFirstOccurrenceOf (".", false, false);
        auto rest = route.source.fromFirstOccurrenceOf (".", false, false);
        auto hasId = [&] (auto& items) { for (auto& i : items) if (i.id == rest) return true; return false; };

        bool sourceOk = prefix == "audio" || prefix == "descriptor" || prefix == "clock"
                     || (prefix == "macro" && hasId (p.macros))
                     || ((prefix == "env" || prefix == "lfo") && hasId (p.modulators));
        if (! sourceOk)
        {
            error = "route '" + route.id + "' reads unknown source '" + route.source + "'";
            return false;
        }
    }

    for (auto& trig : p.triggers)
    {
        for (auto& action : trig.actions)
        {
            if (action.type != V2Action::Type::envelope)
                continue;

            bool ok = false;
            for (auto& m : p.modulators)
                ok = ok || (m.id == action.target && m.type == V2Modulator::Type::ad);
            if (! ok)
            {
                error = "trigger '" + trig.id + "' fires unknown envelope '" + action.target + "'";
                return false;
            }
        }
    }

    return true;
}
