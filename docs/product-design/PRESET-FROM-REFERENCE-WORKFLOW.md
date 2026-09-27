# Preset from reference: the workflow

*27 September 2026. How to turn a reference (image, video, or a mood in words) into a finished, schema-correct new scene: one shader plus one preset. Everything here was checked against the code and the 16 existing scenes: `engine/Source/PresetV2.cpp`, `Modulation.cpp`, `ISFShader.cpp`, `PresetInstance.cpp`, `FinishPass.cpp`, `LookPass.*`, `MainComponent.cpp`, and `engine/tools/build_instrument_presets.py`. The fps numbers were measured on the owner's machine on 27 Sept. The subagent `.claude/agents/preset-builder.md` runs this workflow. Say "use the preset-builder subagent" plus the reference.*

---

## In brief (for the owner)

1. **Every new scene is two files:** a shader (`engine/Shaders/Instrument/<name>.fs`) and a settings file (`engine/Presets/NN - Name.json`). The settings file is **generated** by `engine/tools/build_instrument_presets.py`, not written by hand.
2. **Before any code comes an intake of 8 questions:** mood, palette, motion, what the picture looks like in silence, what happens on a hit, what the bass does, what the build does, and what the drop does.
3. **For calm or ambient music, the steady signals drive the picture** (loudness, bass, build), and hits are only small flashes. For rhythmic music, it's the other way round.
4. **Fixed rules for every style:** no faces, eyes or masks; generative only; the colour comes from your 3-colour palette; grain and film belong to the global LOOK.
5. **The GPU is an integrated Iris Xe.** The measured baseline is in section 7. A new scene must run at least as well as Membrane.
6. **The last step is an automatic check:** schema validation, house rules, a run of the engine on the new scene with a frame dump (it must not be black, flat or frozen), and an fps measurement.

---

## 0. What a scene is (mental model)

```
analysis (plug-in) ─► ReactionShaper ─► react.* signals + event.accent/tick/drop
                                           │                    │
                         macros (8 knobs)  │     triggers ──► envelopes (env.hit/snare/drop/pulse)
                                  │        │                    │
                                  ▼        ▼                    ▼
                  routes: additive, in 0-1 units of each parameter's [min,max]
                                  │
                                  ▼
          stage parameters (clamped / wrapped; "integrate" ones → running phase)
                                  │
                                  ▼
    ISF shader(s) → FinishPass (bloom, exposure, Reinhard, vignette, sRGB)
    → LookPass (grain, crush, palette gradient map on LUMINANCE, halation, dust…)
```

Consequences you must design for:
- **The scene delivers light, shape and motion, not colour.** The look pass maps luminance to the performer's shadow / mid / light palette (`LookPass.cpp gradientMap`). Hue written by the shader only shows under the "Scene Colors" palette. The existing shaders multiply by a warm tint like `vec3(1.0, 0.35, 0.25)`, but only its luminance matters.
- **Audio reaches the shader only through routed parameters.** No Instrument shader reads the raw `level/bass/mid/high/onset/beatphase` uniforms (verified by grep). That is what makes REACT, CALM, LISTEN TO and the character (BREATHE / PULSE / PUNCH) work. Keep it that way.
- **Animate with `vj_time`, and with integrated rate parameters.** Never use `TIME`. The scene clock stops at Speed 0 / Freeze, runs backwards on Reverse, and gets "time kicks" on accents.

---

## 1. Intake: answer these before writing anything

Write the answers down in the task report. Each one decides concrete fields.

| # | Question | Decides |
|---|---|---|
| 1 | **Mood in one sentence**, plus what it must *not* feel like (e.g. "restful, lying on your back; not cheesy screensaver") | description, shader character |
| 2 | **Luminance structure:** how much of the frame is near-black (the house grammar is 60-80 %), where the light lives (edges, a line, particles, a mass), and the brightest element | shader output levels, bloom |
| 3 | **Palette direction** within the 3-colour system: which element should land in shadow / mid / light, and whether the scene needs two layers (body + detail) → `post.palette: "duo"` | output luminance ranges; duo or gradient |
| 4 | **Motion character:** drift, flow, orbit, scroll, growth (simulation), cut-based; and its **designed speed** at Speed x1 | integrated rate params and their defaults |
| 5 | **REST:** what silence looks like (darker, sparser, never dead: one line, a quiet grid, a faint trace) | the `react.energy` route (centre 0.5) |
| 6 | **HIT:** what an accent does (surge, tear, flare, seed, stone). Must be visible for 2-3 frames, ideally leave a trace | the `env.hit` or `env.pulse` route, trace mechanics |
| 7 | **BODY and BUILD:** what bass does (mass, size, swell), what a build-up does (an "inhale": fray, tighten, narrow focus, haze), and optionally the DROP extra | `react.body`, `react.tension`, `env.drop` routes |
| 8 | **Signal emphasis: sustained or transient?** (see below) | route amounts, character recommendation |

### Why question 8 matters

- **Transient-led** (kicks, snares; downtempo, techno, anything with drums). The picture's big moments are `env.hit` / `env.pulse` routes with amount 1.0, and continuous routes are secondary (0.1-0.3).
- **Sustained-led** (ambient, pads, drones, no drums). The accent detector has almost nothing to rank, so `env.hit` rarely fires. BREATHE also has no ticks, and the drop needs `primed` plus a kick-like transient (REACTION-DESIGN §3.6).
  - A scene that routes its main change only from `env.hit` will look **dead** on ambient music.
  - So the main life must come from `react.energy` (the REST look vs the full look, amounts 0.5-0.8), `react.body` (0.3-0.9), `react.tension` (0.4-0.7) and `react.breath` (tempo-locked inhale). Note: `react.breath` is valid in the engine, but the generator's `check()` does not require it.
  - Also slow LFOs: `lfo.phrase` is 64 beats, and you can add your own `lfo` modulators with other periods.
  - Keep the `env.hit` route, since the generator requires a HIT route, but make it gentle (0.3-0.6) and make it swell, not snap.
  - Recommend the **BREATHE** character in the description.
- Either way, **at most one audio-driven source per parameter** (checked). If two things should react, give them two parameters.

---

## 2. Naming and files

| Item | Convention (from the 16 scenes) |
|---|---|
| Shader | `engine/Shaders/Instrument/<snake_case>.fs` (e.g. `halo_ring.fs`). One generator per scene. An effect stage can be reused (`negative_split.fs` serves 13 and 14) |
| Preset file | `engine/Presets/NN - Title Case.json`. NN = next free two-digit number. **Check the folder and the generator's `PRESETS` list first:** other sessions add scenes too (17/18 were being built on 27 Sept). The engine sorts files by name; the index shown in `/v2/preset <index>` is that sort position |
| `id` | kebab-case of the name (`signal-fog`) |
| Stage `id` | a short noun (`fog`, `ring`, `sheet`, `hall`). Route ids are generated as `<source>-<stage>-<param>` |
| Shader parameter names | snake_case, identical in the ISF `INPUTS` and the preset `parameters`. A mismatch fails the load: `shader has no input named ...` |
| `deterministicSeed` | the next in the series: 202, 303 … 1717, so 1818, 1919 … |

**Source of truth: `engine/tools/build_instrument_presets.py`.** All 16 JSONs are its output. Verified: the generator's output is byte-identical to the files on disk.
- Add the new scene as a `preset(...)` entry in `PRESETS`, then run `python engine/tools/build_instrument_presets.py`.
- **Danger:** running the script **deletes every `engine/Presets/*.json` that is not in `PRESETS`.** If another session wrote a JSON by hand, running the script destroys it. Before running it, compare `engine/Presets/*.json` with the `PRESETS` file names. If there is any orphan, don't run the script. Write your JSON by hand in the generator's exact format, and validate it with the non-destructive check in §6.1.
- After running it, `git status engine/Presets` must show **only your new file**. If the 16 existing files show as modified, stop and revert them (`git checkout -- "engine/Presets/NN - X.json"`).

---

## 3. The shader: house format

### 3.1 Header (ISF, JSON inside the first `/* ... */`)

```glsl
/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Name: one or two sentences of what it looks like and how sound moves it.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "drift", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 },
    { "NAME": "zoom", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.5, "MAX": 3.0 },
    { "NAME": "surge", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 2.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.2, "MAX": 4.0 }
  ]
}*/
#include "common.glsl"

void main()
{
    vec2 p = vjCentered();
    ...
    gl_FragColor = vec4 (vec3 (v * emission), 1.0);
}
```

- **Effect stages** use `"CATEGORIES": ["Effect", "Instrument"]` and declare `{ "NAME": "inputImage", "TYPE": "image" }`. The previous stage's output is bound there.
- **Image-source generators** declare `{ "NAME": "source", "TYPE": "image" }`, which the stage's `sources` block binds. `source_size` (a vec2) is declared automatically.
- **Multi-pass / feedback:** `"PASSES": [ { "TARGET": "state", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.25", "HEIGHT": "0.25" }, { } ]`, then branch on `PASSINDEX`.
  - Morphogen runs 16 passes at quarter resolution. Hot Blobs keeps a 2 %-size state buffer read texel by texel.
  - WIDTH / HEIGHT must be **plain numbers**. Expressions like `$WIDTH*0.5` are not evaluated.
- **ISF MIN/MAX/DEFAULT are not what the engine uses.** The engine clamps to the *preset's* min/max and sets every parameter every frame. Keep the header roughly consistent anyway, since it's documentation.
  - For an **integrated** parameter, the shader receives an ever-growing phase, not the rate. So give it `"MIN": 0.0, "MAX": 100000.0` (or 1000.0) and DEFAULT 0.

### 3.2 GLSL dialect

The engine prepends its uniforms and runs JUCE's `translateFragmentShaderToV3`. Write **legacy GLSL**: `texture2D`, `gl_FragColor`, `varying`-era style. Derivatives (`fwidth`, `dFdx`) are fine (Hot Blobs and Dot Relief use them).

Every shader gets these for free:

| Uniform | Meaning |
|---|---|
| `RENDERSIZE` (vec2), `PASSINDEX` (int), `isf_FragNormCoord` (varying, 0-1) | ISF standard |
| `vj_time` | scene clock, seconds. **Use this for all animation** |
| `vj_dt` | this frame's signed step of `vj_time`. Use it for simulations: `min(rate * abs(vj_dt) * 60.0, 1.0)` |
| `vj_beat` | musical position in quarter notes |
| `vj_seed` | preset seed, bumped by `reseed` actions |
| `vj_speed` | current speed multiplier |
| `vj_palette` | smoothed palette-advance count |
| `TIME`, `TIMEDELTA`, `level`, `bass`, `mid`, `high`, `beatphase`, `onset` | exist for legacy shaders. **Do not use them in Instrument shaders** |

Helpers: `IMG_NORM_PIXEL`, `IMG_PIXEL`, `IMG_THIS_PIXEL`, `IMG_THIS_NORM_PIXEL`.

**Reserved names.** Never use these as an INPUT name, a global, or a function name. A collision is a compile error; the engine logged `function name already used for non function` while Negative was being written.
- all the uniforms above, and every other INPUT name
- pass TARGET names and `<imageInput>_size`
- the helpers from `common.glsl`
- GLSL builtins (use `step1`, not `step`, as Morphogen does)

### 3.3 `common.glsl` (included with `#include "common.glsl"`, one level deep)

| Helper | Use |
|---|---|
| `VJ_PI`, `VJ_TAU` | constants |
| `mat2 vjRot(a)` | 2D rotation |
| `vjHash12(vec2)`, `vjHash22(vec2)` | hashes (per-cell random, per-frame speckle) |
| `vjNoise(vec2)` | value noise (4 hashes) |
| `vjFbm(vec2)` | 5-octave fbm. **The house cost unit:** one fbm ≈ 20 hashes |
| `vjCentered()` | aspect-correct coordinates, y in -1..1 |
| `vjLuma(vec3)` | Rec.709 luma |
| `vjToLinear(vec3)` | gamma 2.2 → linear |
| `vjSampleFit(img, size, p, zoom)` | "contain"-fitted image sample, alpha 0 outside |
| `vjPalette(t, base)`, `vjPaletteSet`, `vjThermal`, `vjIridescent` | cosine / false-colour palettes. Only visible with Scene Colors. Avoid them in new scenes: colour belongs to the look pass |

### 3.4 House idioms (copy them)

- **Film-rate stepping:** `floor(vj_time * 24.0)` re-deals speckle, hair lengths or storm cells at 24 fps (Signal Fog, Halo Ring, Negative Split). Never step at the render fps.
- **Loops with a dynamic count:** use a constant bound and a `break`, e.g. `for (int i = 0; i < 7; ++i) { if (i >= layers) break; ... }` (Fibers, Dot Relief).
- **Pixel-exact hairlines:** `float px = 2.0 / RENDERSIZE.y;`, then `exp(-abs(d) / (px * 1.2))` for the line core (Halo Ring, Membrane).
- **Integrated phases** go into noise offsets with small multipliers (`drift * 0.05`, `morph * 0.03`). Use `floor(phase * k)` crossfades to re-roll slowly (Signal Fog `volume()`, Halo Ring lobes).
  - Phases grow without bound: after an hour at rate 6/s the value is about 20000, so avoid `sin(phase * 50.0)`-style high frequencies.
- **Output:** `gl_FragColor = vec4(vec3(v * emission), 1.0)`, linear, HDR allowed.
  - Almost every scene has an `emission` parameter (range 0.2-4.0, default 0.95-1.8) that Intensity drives.
  - A small floor such as `+ 0.004` or `haze * 0.02` keeps silence from being digitally dead.
- **Levels land in the palette like this** (post defaults: exposure +0.3 EV, Reinhard, then sRGB):
  - a linear value of ~0.22 reaches display luma 0.5 (the **mid** colour)
  - ~1.0 reaches about 0.77
  - ~3.0 reaches about 0.9 (toward the **light** colour)
  - Design the body at 0.1-0.5 and the highlights / hits at 1.5-4.

---

## 4. The preset: field-by-field checklist

The generator helpers produce everything marked *(auto)*. You only supply the rest.

| Field | Value in all 16 scenes / rule |
|---|---|
| `schemaVersion` | `"2.0"` *(auto)*. The engine only checks that it starts with "2" |
| `id`, `name` | kebab id, Title Case name |
| `description` | 1-3 sentences in the house voice: what it is, what accents / bass / build / drop do, what silence looks like. "After <artist>:" when a reference is credited. Shown in the plug-in |
| `author` / `license` | `"VJ VST"` / `"project-internal"` *(auto)* |
| `engine` | `{minimumVersion "0.2.0", shaderProfile "isf-legacy", workingColorSpace "linear-srgb", featureContract "2.0"}` *(auto)*. Documentary: the engine does not read it |
| `sourcePolicy` | `bass kick-or-mix, mid snare-or-mix, high hat-or-mix, level mix` *(auto)* |
| `stages[]` | `stage(id, shader, params, sources=None, kind='generator', scale=1.0)`. The first stage must be a generator. `targetFormat` is `rgba16f` *(auto)*. **`scale`** (0.1-1.0) is the render-resolution fraction: all 16 use 1.0, but it is the cheapest perf lever for soft scenes |
| `parameters[]` | `param(name, lo, hi, default, cyclic=False, integrate=False)`. **max > min and min ≤ default ≤ max** (else rejected). `unit` is always `"normalized"` (documentary). `cyclic: true` wraps instead of clamping (Dot Relief `hue`). `integrate: true` = a speed in units per second; the shader gets its running integral. **Integrated params need min ≥ 0 and must receive no routes** (checked) |
| `sources` | optional. `{"source": FORMS}` (generated abstract forms in `Media/Images/Forms`, advance `bar` every 2, random), `dict(FORMS, advance='event.accent')`, or `MEDIA` (the performer's image). Advance: `none`, `beat`, `bar`, or `event.<name>`. The key must be an `image` INPUT |
| `macros` | `macros(labels)` *(auto)*: 8 slots, fixed ids and order (see §4.1). You pass only the labels |
| `modulators` | `ENVS` *(auto)*: `hit` (style hit), `snare` (style snare), `drop` (style drop), `pulse` (1 ms / 80 ms, restart), `phrase` (sine LFO, 64 beats, bipolar). You may append your own `lfo` (shapes: sine, triangle, ramp, sample-hold) or `ad` modulators |
| `routes` | see §4.2 |
| `triggers` | `triggers(reseed_on_accent=False, pulse_on_drop=False)` *(auto)*: accent → hit + pulse; tick → hit scaled `tick`; snare (minStrength 0.1, refractory 70 ms) → snare scaled `snare`; drop → drop (+ pulse if `pulse_on_drop`). Only Negative reseeds (its accents cut the camera). A reseed is a jump, so don't use it for calm scenes |
| `transition` | `cut / 0 ms / quantize beat` *(auto)*. `historyOnEnter` and `retarget` are **not read by the engine** (documentary; Morphogen says `keep`) |
| `post` | default *(auto)*: `bloom {enabled, amount 0.22, threshold 0.9, levels 5}, toneMap "reinhard", exposureEv 0.3, outputColorSpace "srgb", grain 0.0, vignette 0.25` |
| `performance` | `{qualityTier "medium", sceneBudgetMs1080p 3.0, deterministicSeed N, notes "target, not measured"}` *(auto)*. Only the seed is read. Measure real performance with §6.4 |

**Variations seen across the 16 scenes:**
- bloom amount 0.12-0.22, threshold 0.85-1.0, levels 4-5
- exposureEv 0.0-0.5, vignette 0.15-0.3
- `grain` is **always 0.0**: grain lives in the global LOOK
- duo scenes use `DUO_POST` (`palette "duo"`, toneMap `"none"`, exposure 0, bloom 0.12 / 0.85 / 4). They write body to **red** and detail to **green / blue**

Engine clamps (PresetV2.cpp):

| Field | Clamp |
|---|---|
| bloom amount | 0-4 |
| bloom threshold | 0-16 |
| bloom levels | 1-6 |
| exposure | ±8 EV |
| grain | 0-0.2 |
| vignette | 0-1 |
| route amount | ±2 |
| exponent | 0.1-8 |
| macro default | 0-1 |

### 4.1 The 8 macros (fixed ids; labels are the scene's own words)

| Slot | id | Rule | Default label | Seen as |
|---|---|---|---|---|
| 0 | `intensity` | light / energy / amount. Usually → `emission` (0.5-0.6 @ .5), sometimes plus a second param | **lowercase scene word** | glow, dots, white, lights, ink, red body |
| 1 | `speed` | GLOBAL (scene clock). **Never route it** | `Speed` | — |
| 2 | `form` | the scene's character (0.8-0.9 @ .5) | lowercase | ripples, tilt, dither, fog, knot, blots, smear, grain, opening, pattern, fur, streaks, cyan, stipple, folds |
| 3 | `scale` | **bigger as it rises**. Invert with a negative amount when the param is a frequency or zoom-in (`scale -0.7`, `rows -0.8`, `zoom -0.6`) | lowercase | blob size, zoom, width, text size, form size, size, radius, lens |
| 4 | `react` | the global REACT amount. Route it only if the scene's hit size should follow it (Hot Blobs `impact`, Negative `cuts`) | `React` unless routed (then lowercase: throw, hits + cuts) | — |
| 5 | `erode` | **pristine → worn, torn, dissolved** (0.5-1.0 @ .5) | lowercase | break-up, dissolve, speckle, dead lights, tear, dropout, holes, blur, erosion, island, fray, noise, fading lines, thinning |
| 6 | `glide` | GLOBAL (speed glide), default 0.25. **Never route it** | `Glide` | — |
| 7 | `detail` | density / fineness (0.8-1.0 @ .5) | lowercase | curl, grid, cell, grime, layers, density, torn edges, particles, mesh, relief, hair, grain, lattice, contrast, fine lines, points |

Checked by the generator:
- All five SHAPE macros must be routed.
- Their labels must start lowercase.
- Macro routes always use **centre 0.5**, so knob 50 % = the tuned look (the parameter default).

### 4.2 Routes

Route math (`Modulation.cpp`):
1. `u = clamp((raw - inputMin) / (inputMax - inputMin))`
2. apply the curve (`linear`, `power` with `exponent`, or `smoothstep`)
3. smooth with `attackMs` / `releaseMs`
4. contribution = `gain × amount × (smoothed − center)`
5. **The amount is in units of the parameter's [min,max] range.** Contributions add to the default's 0-1 position, then the sum is clamped (or wrapped) once.

`gain`:
- REACT's `contAmount` for audio / react sources
- `hitAmount` for `env.*`
- 1 for macros and LFOs
- × 2·macro if `scaleBy` is set

The helpers set the conventions:

| Helper | Source | Centre | Attack / release | Typical amounts |
|---|---|---|---|---|
| `route('macro.X', ...)` | macro | **0.5** | 60 / 60 | ±0.3-1.0 |
| `rest(dst, a)` **REST, required** | `react.energy` | **0.5** | 0 / 0 (Terminal: release 800 so text doesn't blink) | ±0.25-0.8. Silence carries the scene to `default − a/2` |
| `hit(dst, a=1.0)` **HIT, required** (or `route('env.pulse', ...)`) | `env.hit` | 0 | 0 / 0 | 0.35-1.0 into a `surge / tear / rip / flash / storm / jump` param (range 0-1…4) |
| `body(dst, a, center=0)` **BODY, required** | `react.body` | 0 or 0.4 | 0 / 0 | ±0.05-0.9 |
| `build(dst, a, **kw)` **BUILD, required** | `react.tension` | 0 | 0 / 0 | 0.2-0.7 (One Bit: `lo=0.55, release=400` → letterbox only at high tension) |
| `drop(dst, a)` optional | `env.drop` | 0 | 0 / 0 | ±0.2-1.0 |
| `air(dst, a)` optional | `react.air` | 0.3 | 0 / 0 | 0.25-0.7 (surface, hairs, speckle) |
| `snare(dst, a)` optional | `env.snare` | 0 | 0 / 0 | 0.15-1.0 |
| `scar(dst, a)` optional | `react.scar` | 0 | 0 / 0 | 0.3-0.5 (accumulates through a section, healed by rest and the drop) |
| `route('lfo.phrase', dst, a, 0.5)` optional | 16-bar sine | 0.5 | 0 / 0 | 0.08-0.25 (slow drift of zoom / focus / tilt) |

- Valid sources are listed in `PresetV2.h`:
  - `audio.level|bass|mid|high.activity|absolute`, `audio.kick|snare|hat.activity`, `audio.band0..5.activity`
  - `descriptor.centroid|flatness|rolloff|flux|energyTrend|build|presence`
  - `clock.beatPhase|barPhase`
  - `react.body|energy|air|breath|tension|rest|scar`
  - `macro.<id>`, `env.<id>`, `lfo.<id>`
- The 16 scenes use only `macro`, `react`, `env` and `lfo`. Prefer those, since the reaction layer has already auto-ranged and characterised them.
- An unknown `audio.` / `descriptor.` / `react.` name is **accepted by the parser but reads 0** (only a log warning). Double-check spelling.

---

## 5. Taste rules (every style, no exceptions)

From DESIGN-DOSSIER §5.9, REACTION-DESIGN §5.1, the REFERENCE-ZWOBOT-SHOWCASE recommendation 5, `ART-DIRECTOR-BRIEF.md` and `REFERENCE-LIBRARY.md`:

1. **Generative only.** No stock footage, no photos baked into a scene.
   - Image input may only be the generated `Images/Forms` (raymarched abstract SDFs from `make_source_assets.py`) or the performer's own media via `MEDIA`.
   - Never ship downloaded reference frames. `references/` stays local and out of git.
2. **No faces, skulls, masks or eyes.** Even two round holes at the same height read as eyes.
   - No mirror symmetry left / right across the screen centre when the content has blobs or holes.
   - A kaleidoscope only with 6 or more segments or rotational symmetry.
   - No ring with a "pupil".
   - **No audio route may open holes** (Mesh Body's `hole` has no audio route on purpose).
   - Scan about 20 frames of the finished scene and ask "are there eyes?".
3. **The 3-colour palette owns colour.** Deliver luminance with a clear shadow / mid / light structure (or duo body / detail). Don't hard-code hues that fight the palette.
   - Remember the owner's taste: red-mono, or at most black + one colour + one light, and "red only in the line / on the event".
4. **Film and grain belong to the global LOOK** (grain at 24 fps, halation, weave, dust, crush). Keep `post.grain` at 0.0.
   - In-shader texture is fine when it *is* the material (Signal Fog particles, Ink paper fibre). Step it at 24 fps.
5. **The house grammar:** 60-80 % near-black, thin lines and particles rather than filled areas, emergence and erosion, and rare correlated events rather than constant noise.
6. **Never dead, never a strobe.**
   - REST is darker and sparser but alive: the global rest floor keeps 22-45 % exposure, and the scene should keep a faint element moving.
   - Hits are capped at 3 per second by the engine. Don't build your own flashing.
7. **"0 = still".** At Speed 0 / Freeze nothing may move. So all motion comes from `vj_time`, `vj_dt` or integrated params, never from `TIME`. Per-frame re-dealt speckle (`floor(vj_time*24)`) also freezes, which is correct.

---

## 6. Build and self-check (do all of it before calling a scene done)

### 6.1 Validate the preset without touching other files

Importing the generator runs `check()` on the 16 built-in scenes but writes nothing. Run from the repo root:

```bash
python - <<'EOF'
import sys, json, re, os
sys.path.insert(0, 'engine/tools')
import build_instrument_presets as b          # import only: no files are written or deleted
f = 'engine/Presets/NN - Name.json'            # <- your file
body = json.load(open(f, encoding='utf-8'))
b.check(f, body)                               # house rules (REST/HIT/BODY/BUILD, knobs, rates, one audio source)
# engine rules (PresetV2::parse + V2Instance::create)
assert body['schemaVersion'].startswith('2') and body['stages'][0]['kind'] == 'generator'
ids = lambda k: [x['id'] for x in body[k]]
for k in ('stages', 'macros', 'modulators', 'routes', 'triggers'):
    assert len(ids(k)) == len(set(ids(k))) and all(ids(k)), 'dup/empty id in ' + k
ev = {'kick','snare','hat','bassTransient','midTransient','highTransient','midiNote','userTrigger','accent','tick','drop'}
mods = {m['id']: m['type'] for m in body['modulators']}
for t in body['triggers']:
    assert t['on'].replace('event.', '') in ev, t['on']
    for a in t['actions']:
        assert a.get('type') in ('reseed', 'palette-advance') or mods.get(a['target']) == 'ad', a
known = {'react.' + x for x in 'body energy air breath tension rest scar'.split()}
for r in body['routes']:
    s = r['source']; pre = s.split('.')[0]
    assert r['inputMax'] > r['inputMin'] and abs(r['amount']) <= 2
    assert (pre == 'macro' and s[6:] in ids('macros')) or (pre in ('env', 'lfo') and s[4:] in mods) \
        or s in known or pre in ('audio', 'descriptor', 'clock'), 'bad source ' + s
reserved = {'TIME','TIMEDELTA','RENDERSIZE','PASSINDEX','level','bass','mid','high','beatphase','onset',
            'vj_palette','vj_seed','vj_beat','vj_time','vj_speed','vj_dt','isf_FragNormCoord'}
for st in body['stages']:
    assert '..' not in st['shader']
    src = open('engine/Shaders/' + st['shader'], encoding='utf-8').read()
    hdr = json.loads(src[src.index('/*') + 2: src.index('*/')])
    inputs = {i['NAME']: i['TYPE'] for i in hdr['INPUTS']}
    assert not reserved & set(inputs), 'reserved input name'
    for p in st['parameters']:
        assert p['name'] in inputs, 'shader lacks input ' + p['name']
        assert p['max'] > p['min'] and p['min'] <= p['default'] <= p['max'], p['name']
    for k in st.get('sources', {}):
        assert inputs.get(k) == 'image', 'source needs image input ' + k
    body_glsl = src[src.index('*/') + 2:]
    assert '#include "common.glsl"' in body_glsl and 'gl_FragColor' in body_glsl
    code = re.sub(r'/\*.*?\*/|//[^\n]*', '', body_glsl, flags=re.S)     # ignore comments
    assert not re.search(r'\b(TIME|level|bass|mid|high|onset|beatphase)\b', code), \
        'reads (or shadows) a legacy audio/TIME uniform - use routed params + vj_time'
print('OK', f)
EOF
```

(The last regex is a heuristic. If it flags a local variable named e.g. `high`, rename the variable. Shadowing the engine uniforms is a trap anyway.)

### 6.2 Re-read the shader against this checklist

- [ ] The header JSON is valid (no trailing commas). Every preset param is an INPUT, with names identical.
- [ ] `#include "common.glsl"` is on its own line. The dialect is legacy (`texture2D`, `gl_FragColor`).
- [ ] No name collides with a uniform, an INPUT, a pass target, a common.glsl helper or a builtin.
- [ ] Animation uses `vj_time`, `vj_dt` or integrated params only. Integrated INPUTs have MAX 100000.
- [ ] Loops have constant bounds (with `break`). No per-pixel loop above ~30 iterations of anything heavier than a hash.
- [ ] No division by zero: guard lengths with `max(x, 1e-4)` (the house does this everywhere).
- [ ] The output is linear, alpha 1.0, and non-negative (`max(v, 0.0)`), with a faint floor in silence.
- [ ] The eye / face scan (§5.2) passes as far as can be reasoned from the code.

### 6.3 Build and run the engine on the new scene (a frame dump, no GUI clicking)

**Build.** The same commands as `Build and Install.bat`, engine only:

```
cmake -S engine -B engine/build
cmake --build engine/build --config Release
```

`POST_BUILD` mirrors `Shaders/`, `Presets/` and `Media/` next to the exe **only when the exe relinks**. After adding only a `.fs` and a `.json`, nothing relinks, so copy the files yourself.

**Run from a scratch copy.** This never locks the build output, never touches the owner's running engine, and needs no relink:

```bash
S=<your scratchpad>/enginetest
cp -r "engine/build/VJEngine_artefacts/Release/." "$S/"
cp engine/Shaders/Instrument/<name>.fs "$S/Shaders/Instrument/"
cp engine/Shaders/Instrument/common.glsl "$S/Shaders/Instrument/"   # in case it changed
cp "engine/Presets/NN - Name.json" "$S/Presets/"
```

**Run it with the frame dump**, on its own OSC port with the built-in demo groove:
- Launch `"$S/VJ Engine.exe" --osc-port 9123 --demo` with the environment variable `VJ_FRAMEDUMP=$S/dump.bin`.
- Wait until the dump has more than 30 frames.
- Send OSC (UDP, three times): `/v2/quality 1.0` (full render scale) and `/v2/preset "<Scene Name>"` (a string argument selects by name).
- Wait 8 s, then terminate the process.

**The dump format** (MainComponent.cpp `forensics::dumpFrame`):
- per frame, a 120-byte header `struct '<Iiiid8f8i8f'` (magic, w, h, nEvents, now, 8 floats, 8 event types, 8 strengths)
- then 160 x 90 RGB bytes, rows bottom-up

**What to check** (skip about 60 frames after the switch):
- **Not solid or flat:** the median per-frame spatial std of luma is > 0.03. Existing scenes: 0.07-0.21.
- **Not black:** the median mean luma is > 0.02. Existing: Halo Ring 0.036 (the darkest, by design), Membrane 0.063, Signal Fog 0.17, Corridor 0.23.
- **Moves:** the median frame-to-frame |ΔY| is > 0.001. Existing: 0.0024 (Halo Ring) to 0.02 (Fibers).
- **Near-black share** (Y < 0.06) sits in the house range: Halo Ring 0.91, Membrane 0.69, Mesh Body 0.65, Hot Blobs 0.60, Signal Fog 0.32.
- **Log** (`$S/VJEngine.log`, new lines only):
  - `PresetManager: switched to '<Name>'` must appear.
  - `rejected <file> - <reason>` means a JSON / schema error.
  - `could not load '<Name>': stage '<id>': Fragment shader error: ...` means a GLSL error, with a line number. Subtract the prepended uniform lines (about 25 + one per INPUT) to find the line in your file.

A reference implementation of this run lives in the preset-builder agent's notes (`smoke.py` pattern). It was verified on 27 Sept on Signal Fog, Halo Ring, Ink, Fibers, Membrane, Hot Blobs, Mesh Body, Corridor and Morphogen: every one passed the thresholds above.

### 6.4 Performance on the Iris Xe (measured, not guessed)

**Don't use the dump run for fps.** `glReadPixels` every frame costs 10-20 fps. Instead:
- Run without `VJ_FRAMEDUMP`.
- Send `/fullscreen 1.0` (the owner's output is fullscreen 1920x1080) and `/v2/quality 1.0`.
- Select the scene and wait 16 s.
- Read the `FPS: <n> (preset '<Name>' ...)` lines in `VJEngine.log`. They are written every 5 s; skip the first one after `switched to`, since it straddles the switch.
- **Always measure one or two reference scenes in the same run.** A windowed run caps at 60 (vsync) and can't discriminate. One run on 27 Sept showed *every* scene at about 20 fps because of external load, and the repeat was normal.

**Baseline, fullscreen 1080p, 100 % render scale, demo groove, 27 Sept 2026:**

| Scene | fps | Rough per-pixel cost |
|---|---|---|
| Halo Ring | 60 (vsync) | hashes only |
| Ink | 58-60 | 3 fbm + 2 noise |
| Signal Fog | 55 | 3 fbm + speckle |
| Hot Blobs | 54-60 | 3 fbm + 4-ring loop + derivatives |
| Membrane | 46 | about 12 fbm + a 9-cell loop |
| Fibers | 40-45 | up to 16 fbm (7 layers) |
| Mesh Body | 43 | plexus loops |
| Dot Relief | 40-52 | up to 28-row loop with image taps |
| Negative (2 stages) | 35-38 | lattice + a 3x3 effect with 18 taps |

**Budget rule for a new scene:** at least as fast as **Signal Fog (≥ 55 fps)**. Never slower than **Membrane (46)**. Only drum-driven peak scenes may approach Negative.
- When fps drops below 50, the engine's adaptive render scale kicks in (`Quality: render scale NN%` in the log), and softness reads as blur.
- Levers, cheapest first:
  - stage `scale` 0.5-0.75 (soft or ambient material hides it)
  - fewer fbm octaves or calls (warp with `vjNoise`, not `vjFbm`)
  - a low-res `PERSISTENT` pass for fields that change slowly
  - constant-bound loops with early `break`
  - no full-res multi-tap blur (use bloom, or a quarter-res pass)
- The `performance.sceneBudgetMs1080p` field stays 3.0 / "target, not measured". Put the measured fps in your report, not in the JSON.

### 6.5 Hand-off

- `git status`: only the new `.fs`, the new `.json` and (if used) the edited `build_instrument_presets.py`. **No changes to the 16 existing scenes. No commit** unless asked.
- Report:
  - the intake answers
  - the route table (REST / HIT / BODY / BUILD / DROP)
  - the macro labels
  - the self-check numbers (spatial std, mean luma, motion, near-black share, fps vs references)
  - the eye scan result
  - what is still unverified: always "not yet seen inside Live by the owner"
- If the owner will judge it visually, keep a few PNG frames: `/debug/snapshot` writes `VJEngine_snapshot.png` next to the exe.

---

## 7. Pitfalls already seen in the 16 scenes (and how to avoid them)

1. **Ambient scene driven only by hits.** It looks dead on pad music. Lead with `react.energy` / `body` / `tension` / `breath` and slow LFOs (§1 Q8).
2. **Routing audio into a rate.** It was removed on purpose: runaway speed, jumps. Hits move time through the global *time kick*. Rates are only the designed speeds (the generator asserts it).
3. **Two audio sources on one parameter.** Double hits. The generator asserts one per parameter. Use a second parameter.
4. **A hit envelope into a parameter whose range is too small.** The Mesh Body lesson: surge 0.31 × 0.12 radius gave "+3.7 %, invisible".
   - Make hit params big (`surge` max 1.6-4.0) and have the shader use them strongly: brightness ×(1 + 1.5·surge), size, tear.
   - Check the peak effect is at least 20/255 of frame difference (the REACTION-DESIGN acceptance target).
5. **`react.energy` with the wrong centre.** Always 0.5, so REACT 0 shows the tuned look. Macros are always centre 0.5.
6. **Forgetting the rest look.** Silence must be *designed* (one clean ring, a flat grid, a faint trace), not just "everything at 50 %".
7. **Holes that open with sound** (eyes), or mirror symmetry of blobs. Banned (§5).
8. **Hard-coded colour.** It only shows under Scene Colors. Everywhere else the luminance decides, so a mid-grey "colourful" scene becomes one flat palette tone.
9. **`TIME` or `sin(TIME)` animation.** It breaks Freeze / Speed 0 / Reverse. Use `vj_time`.
10. **Unbounded phases at high frequency.** Precision shimmer after long sets.
11. **Name collisions in GLSL** (§3.2). And PASSES WIDTH written as an expression: silently treated as the first number found.
12. **Running the generator with orphan JSONs present.** They are deleted (§2).
13. **Testing against a stale exe folder.** Presets and shaders are only copied on relink (§6.3).
14. **Measuring fps windowed, or while another process loads the GPU.** Compare against references in the same run (§6.4).
15. **Media / forms scenes:** the `sources` key must match an `image` INPUT, and the folder must exist under `engine/Media`. Missing images show black and don't crash, so the dump check (mean luma) is what catches it.

---

## 8. Minimal generator entry (template)

```python
preset('NN - Name.json', 'name-id', 'Name',
       'What it is. Accents …; bass …; build-ups …; the drop …. In silence ….',
       [stage('sid', 'name.fs', [
           param('drift', 0.0, 1.0, 0.2, integrate=True),       # designed speed (units/s at Speed x1)
           param('zoom', 0.5, 3.0, 1.2),
           param('density', 0, 1, 0.5),
           param('wear', 0, 1, 0.1),
           param('fold', 0, 1, 0.5),
           param('surge', 0, 2.0, 0),                            # hit target
           param('haze', 0, 1, 0.3),
           param('emission', 0.2, 4.0, 1.2)])],
       [route('macro.intensity', 'sid.emission', 0.6, 0.5), route('macro.form', 'sid.fold', 0.8, 0.5),
        route('macro.scale', 'sid.zoom', -0.6, 0.5), route('macro.erode', 'sid.wear', 0.8, 0.5),
        route('macro.detail', 'sid.density', 0.8, 0.5),
        rest('sid.density', 0.6), hit('sid.surge', 1.0), body('sid.fold', 0.3, 0.4), build('sid.haze', 0.5),
        drop('sid.zoom', -0.2), route('lfo.phrase', 'sid.haze', 0.15, 0.5)],
       {'intensity': 'glow', 'form': 'folds', 'scale': 'size', 'erode': 'wear', 'detail': 'density'},
       seed=1818)
```

The rule for the route list: every destination takes at most one of `react.* / env.*`, while macros and LFOs may stack.
