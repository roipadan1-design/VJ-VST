# Real-time shader techniques for the Instrument family

Research notes, 2026-09-27. Audience: whoever writes or reviews the next
Instrument shaders (`engine/Shaders/Instrument/*.fs`). The target machine is a
laptop with an **Intel Iris Xe integrated GPU**, so every technique below has a
cost tag and an integrated-GPU angle.

The doc has three parts:

1. **House conventions**: what the engine actually provides, read from the
   shaders and the host code, so nobody has to work it out again.
2. **Techniques** (12 of them), each written in house style, with the mood it
   suits and a rough cost.
3. **Iris Xe budgeting**: how to keep a scene inside its 3 ms budget.

Cost tags are rough per-pixel figures at 1080p, internal scale 1.0, on Iris Xe
(96 EU, about 2 TFLOPS fp32 peak, shared LPDDR4x memory at about 68 GB/s):
- **cheap** means under about 0.7 ms (a few noise calls, or a handful of texture reads),
- **medium** means about 0.7 to 2 ms,
- **expensive** means over 2 ms, which on its own eats most of the 3 ms scene budget.

These are estimates from operation counts, not measurements. See section 3 for
how to measure.

---

## 1. House conventions (read from the code, not guessed)

Sources: `common.glsl`, `hot_blobs.fs`, `morphogen.fs`, `emergence.fs`,
`fibers.fs`, `feedback_smear.fs`, `mesh_body.fs`, `terminal.fs`, `signal_fog.fs`,
`membrane.fs`, `ink.fs`, `engine/Source/ISFShader.{h,cpp}`,
`PresetInstance.cpp`, `PresetManager.cpp`, `MainComponent.cpp`, `GLHelpers.cpp`,
`engine/Presets/01 - Hot Blobs.json`.

### 1.1 File shape

```glsl
/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Name: what it looks like, then what the sound does to it.",
  "CATEGORIES": ["Generator", "Instrument"],          // or ["Effect", "Instrument"]
  "INPUTS": [
    { "NAME": "drift",    "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 }, // integrated phase
    { "NAME": "surge",    "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },      // envelope target
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.2, "MAX": 4.0 }       // always present
  ],
  "PASSES": [                                            // optional
    { "TARGET": "state", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.02", "HEIGHT": "0.02" },
    { }                                                  // last pass = the image
  ]
}*/
#include "common.glsl"
void main() { ... }
```

- The JSON header is parsed by `ISFShader::loadFromFile`. Supported INPUT types:
  `float`, `bool`, `long`, `point2D`, `color`, `image`, `event`. An `image` input
  named `foo` also gets `uniform vec2 foo_size` automatically. `IMPORTED` stills
  are supported too: they are loaded with mipmaps (`GL_LINEAR_MIPMAP_LINEAR`,
  clamp to edge).
- `#include "common.glsl"` is a plain text include, one level deep, resolved next
  to the shader file.
- The source is written in GLSL 1.10/ES style (`texture2D`, `gl_FragColor`,
  `varying`). JUCE's `translateFragmentShaderToV3` prepends `#version 150` and
  does a **plain text replace** of `varying`→`in`, `texture2D`→`texture` and
  `gl_FragColor`→`fragColor`. As a result:
  - GLSL 1.50 features are all available: `texelFetch`, `textureLod` (writing
    `texture2DLod` also works, because it becomes `textureLod`), `dFdx`/`fwidth`,
    integer and bit operations, `uint`, and dynamic loops.
  - **Never** use an identifier that contains `varying`, `texture2D` or
    `gl_FragColor` as a substring. It will be rewritten.

### 1.2 What every shader gets for free

| Uniform | Meaning | House rule |
|---|---|---|
| `RENDERSIZE` | **Size of the pass currently being drawn.** In a state pass it is the state buffer's size, not the screen's. | Recompute other buffers' sizes from the final pass's `RENDERSIZE` (hot_blobs: `floor (RENDERSIZE * 0.02 + 0.5)`). |
| `PASSINDEX` | Index into `PASSES`. | Branch on it: `if (PASSINDEX == 0) {state} else {image}`. |
| `vj_time` | Scene clock in seconds. It stops at Speed 0 or Freeze and runs backwards on Reverse. | **Animate with this**, not `TIME`. |
| `vj_dt` | This frame's signed step of `vj_time`. | Simulations step by `clamp (abs (vj_dt), 0.0, 0.05)`, so they freeze with the clock. |
| `vj_speed` | Signed speed multiplier. | Rarely needed. |
| `vj_beat` | Musical position in quarter notes (host transport or free-run clock). | Beat-locked choices: `floor (vj_beat * 4.0)` changes once per 16th note. |
| `vj_seed` | Per-preset seed, bumped by "reseed" actions. | Add to hash inputs so a reseed gives a new layout. |
| `vj_palette` | Smoothed palette-advance count. | Used inside `vjPalette (t, base)`. |
| `TIME`, `TIMEDELTA` | Wall clock. | Use only for things that should keep running while the scene is frozen (feedback_smear decays with `TIMEDELTA`). |
| `level bass mid high beatphase onset` | Legacy raw audio. | **Do not read these in Instruments** (see 1.4). |
| `isf_FragNormCoord` | uv from 0 to 1. | Used for buffer reads and effects. |

Helpers in `common.glsl`:
- `vjRot(a)`: a 2D rotation matrix.
- `vjHash12`, `vjHash22`: Dave Hoskins' sin-free "hash without sine" hashes.
- `vjNoise`: value noise, 4 hashes plus a smoothstep blend.
- `vjFbm`: **5 octaves of `vjNoise`**, so 20 hashes per call. Between octaves it
  multiplies by `mat2(1.6,1.2,-1.2,1.6)`, which scales by 2 and rotates by about
  37°. Output range is roughly 0 to 0.97, centred near 0.48.
- `vjCentered()`: y runs from -1 to 1 and x is scaled by the aspect ratio. One
  pixel in this space is `2.0 / RENDERSIZE.y`.
- `vjLuma`, `vjToLinear`.
- `vjSampleFit(img, size, p, zoom)`: "contain" fit of an image.
- `vjPaletteSet` / `vjPalette`: six IQ-style cosine palettes, returned in linear
  light.
- `vjThermal`, `vjIridescent`.

### 1.3 Output contract

- Write **linear, possibly HDR** colour with alpha 1. Media stages use alpha 0 to
  mean "no image here".
- The engine then runs `FinishPass` (bloom, Reinhard tone map, sRGB encode) and
  `LookPass` (24 fps film grain, halation, weave, dust, lifted blacks, blue-noise
  dither).
- **Shaders must not add** bloom, tone mapping, gamma, grain or dither. They
  would be applied twice.
- House look: monochrome `vec3 (v * emission)`, or the red-mono tint
  `vec3 (1.0, 0.35, 0.25) * v * emission`. Every scene has an `emission` input
  (range 0.2 to 4) as its brightness knob. Scenes stay near black, and light is
  earned.

### 1.4 Audio-reactivity lives in the preset, not the shader

The shader exposes **semantic, sound-agnostic parameters**, and the preset JSON
wires sound to them:

- **Phase parameters** (`flow`, `drift`, `morph`, `turn`, `clock`, `breathe`,
  `scroll`, `pan`) have huge MAX values (1000 or 100000) because the preset
  declares them `"integrate": true`. The runtime integrates *rate × sceneDt*, so
  the shader receives a phase that only ever grows, and multiplies it by a small
  constant (`drift * 0.05`). A knob or a band clock can speed motion up without
  the jump you would get from multiplying time.
- **Hit parameters** (`drop`, `surge`, `tear`, `swell`, `jump`, `invert`, `rip`,
  `seed_amt`, `impact`) run from 0 to 1. The preset feeds them from envelopes
  triggered by events. Example: `event.accent` → envelope `pulse`
  (0 ms attack, 80 ms decay) → route `env.pulse` → `stage.blobs.drop`. Other
  routes come from `react.energy`, `react.body`, `react.tension`, `lfo.phrase`,
  and `macro.<id>` (8 macros).
- The preset can **narrow** a shader parameter's range: the preset's `min`/`max`
  sits inside the shader's `MIN`/`MAX`.
- **Envelope → discrete event inside the shader** (hot_blobs): keep the previous
  frame's `drop` in a state texel and fire on the rising edge
  (`drop > 0.15 && prev <= 0.15`). This is the house way to turn "a kick
  happened" into a one-off action (drop a stone, plant a seed, spawn a ring)
  without the shader knowing anything about audio.
- The consequence for new techniques: **every audio-facing idea below is
  expressed as a parameter** (an amount, a phase, or a trigger envelope), never
  as a read of `bass` or `onset`.

### 1.5 Persistent state buffers (ping-pong)

- `PASSES` entries with `"TARGET"` render to an offscreen buffer.
  - `"PERSISTENT": true` makes it double-buffered. Reading the target name gives
    the *previous* write, and each pass that writes it flips the pair.
  - `"FLOAT": true` selects RGBA32F. Without it the buffer is RGBA16F; there is
    no 8-bit option for passes.
  - Buffers use `GL_LINEAR` filtering, `CLAMP_TO_EDGE`, and **no mipmaps**.
- `"WIDTH"` / `"HEIGHT"` are **fractions of the stage's render size** (plain
  numbers only; expressions fall back to a guess).
- **The last pass always draws to the stage output**, whatever its TARGET says.
- **Repeating the same TARGET N times means N simulation steps per frame.**
  Morphogen declares 16 identical `rd` passes at 0.25 scale, which gives 16
  Gray-Scott steps per frame.
- Buffers start at **zero** (cleared black). Design the encoding so that zero is
  a valid state, or flag "initialised" in a channel. Morphogen stores `1 - A` in
  `.r` so that a zero buffer means A = 1, and sets `.b = 1` once a texel is sown.
- **Texel-addressed state** (hot_blobs pattern): a 0.02 × 0.02 buffer (38×22 at
  1080p) where only row 0 texels 0 to 5 matter.
  - Read texel `i` with `texture2D (state, vec2 (i + 0.5, 0.5) / size)`.
  - Early-out every other texel.
  - Slot 0 holds the physics body `(pos.xy, vel.xy)`, slot 1 the bookkeeping
    `(prev drop, event counter)`, and slots 2 to 5 a **ring buffer of events**
    `(centre.xy, birth vj_time, strength)`. The new event goes into slot
    `2 + mod(counter, 4)`.
  - The image pass then loops over the 4 events analytically, so there is no
    full-res simulation at all.
- **Gotcha: resolution changes wipe state.** Buffer size is a fraction of the
  render size. When the adaptive quality ladder
  (`MainComponent::updateAdaptiveQuality`) moves the render scale (it steps
  between 0.5 and 1.0 depending on fps), or the window resizes,
  `ensureRenderTarget` reallocates the buffer and clears it. The hot_blobs mass
  snaps home, and Morphogen re-sows its culture. Design for it: either accept a
  soft reset, or (engine idea, not implemented) give state passes an absolute
  pixel size so they survive scale changes.
- **Prefer `texelFetch (state, ivec2 (i, 0), 0)`** for texel-addressed state. It
  needs no half-texel maths and cannot be touched by filtering, and GLSL 1.50
  has it.

### 1.6 Stage and scene level knobs that already exist

- Preset stage `"scale"` (0.1 to 1.0) renders a whole stage at reduced
  resolution. The next stage or the finish pass upsamples it bilinearly.
- Global `renderScale` (0.4 to 1.0) is driven by the adaptive quality loop:
  below 50 fps it drops 0.08 (or 0.15 below 35 fps); after 6 seconds at 58.5 fps
  or more it climbs 0.05.
- `"targetFormat": "rgba16f" | "rgba8"` sets the stage's output buffer format.
- `performance.sceneBudgetMs1080p` is 3.0 on every preset, flagged
  "target, not measured".
- **During a crossfade, both scenes render.** Two expensive scenes crossfading
  can blow the frame even when each is fine alone.

### 1.7 Style habits worth keeping

- **Anti-aliasing** comes from analytic width:
  - `smoothstep (0.0, px * 1.4, d)` with `px = 2.0 / RENDERSIZE.y` (mesh_body).
  - `fwidth (f) * 1.5` (hot_blobs).
- **Film-rate randomness**: `floor (vj_time * 24.0)` gives a fresh speckle 24
  times a second. Used by signal_fog, emergence and mesh_body dust.
- **Per-feature determinism**: everything about a plexus link depends only on
  its two endpoint cell ids, so it never flickers at cell borders (mesh_body
  `link()`).
- **Domain warp** everywhere: `f = vjFbm (sp + k * vec2 (vjFbm (...), vjFbm (...)))`
  (hot_blobs, ink, membrane, fibers).
- **Relief lighting** from a scalar field: a finite-difference or `dFdx` normal,
  a single key light, and a spec lobe (hot_blobs, morphogen).
- **Calibration** for the cost tags below, counting `vjNoise` calls per pixel
  (1 fbm = 5 noise = 20 hashes):
  - fibers: up to 7 layers × (2 fbm + 1 noise) plus 2 fbm, about 90 noise.
  - membrane: `sheet()` is 3 fbm and runs 3 times for the gradient, plus 3 fbm,
    about 60 noise.
  - ink: about 17 noise.
  - hot_blobs: 3 fbm plus 4 analytic rings, about 15 noise.
  - morphogen: 16 passes × 9 fetches at quarter resolution, about 1.2M RGBA32F
    fetches per frame.

  Fibers and membrane are the scenes most likely to be over budget on Iris Xe
  at full scale. See section 3.

---

## 2. Techniques

Every technique below follows the same format:
- **Good for**: what it does.
- **Mood**: glitch/kinetic, ambient/organic, or either.
- **Cost**: a cost tag.
- **Source**: where the idea comes from.
- **Snippet**: code in house style.
- **Notes**: the house-specific wiring.

"Hit" parameters are assumed to be fed by preset envelopes, and "phase"
parameters are integrated by the preset (see 1.4). Snippets are pseudocode that
is close to compilable. They reuse `common.glsl` names and are not tested.

### T1. Nested domain warping, with the warp vectors used as extra channels

- **Good for:** organic marbling, smoke, flesh, oil. The warp vectors `q` and `r`
  come out as free by-products you can use for veins, folds, or colour.
- **Mood:** either. With a slow phase it is ambient. With a hit envelope added to
  the warp amount, it becomes kinetic: shock-waves travel through the texture.
- **Cost:** **expensive** in the canonical form (5 fbm, about 25 noise calls). It
  drops to **medium** with the octave-limited helper below (about 17 noise), and
  to **cheap** when it runs in a low-res pass (T10).
- **Source:**
  - Inigo Quilez, "Domain warping" (iquilezles.org/articles/warp): the recipe
    `f(p + 4·r)`, where `r = fbm(p + 4·q)` and `q = fbm(p)`, with offset
    constants (5.2,1.3), (1.7,9.2) and (8.3,2.8). The article colours with |q|
    and r.y.
  - hot_blobs, ink, membrane and fibers already use one level of this.

```glsl
// vjFbm is hard-wired to 5 octaves; warp vectors rarely need more than 2-3.
float fbmN (vec2 p, int oct)
{
    float v = 0.0, a = 0.5;
    mat2 m = mat2 (1.6, 1.2, -1.2, 1.6);
    for (int i = 0; i < 5; ++i)
    {
        if (i >= oct) break;           // uniform across the screen -> no divergence
        v += a * vjNoise (p);
        p = m * p;
        a *= 0.5;
    }
    return v;
}

// INPUTS: morph (integrated phase), warp (0..6), jolt (0..1 hit), veins (0..1), scale, emission
void main()
{
    vec2 p = vjCentered() * scale;
    float t = morph * 0.03;
    float w = warp * (1.0 + 1.5 * jolt);                 // a hit shoves the whole warp
    vec2 q = vec2 (fbmN (p + t, 3), fbmN (p + vec2 (5.2, 1.3) - t, 3));
    vec2 r = vec2 (fbmN (p + w * q + vec2 (1.7, 9.2) + 0.7 * t, 3),
                   fbmN (p + w * q + vec2 (8.3, 2.8) - 0.6 * t, 3));
    float f = vjFbm (p + w * r);

    float v = f * f * 1.6;
    v *= 0.6 + 0.8 * clamp (dot (q, q), 0.0, 1.0);                     // folds lift
    float aa = fwidth (r.x) + 1e-4;
    v += veins * (1.0 - smoothstep (0.0, aa * 2.0, abs (r.x - 0.55)));  // iso-line of r = a vein
    gl_FragColor = vec4 (vec3 (1.0, 0.35, 0.25) * v * emission, 1.0);
}
```

**Notes**
- The iso-lines of the intermediate fields (`r.x = const`) give hairline veins
  for almost nothing. That is the cheapest source of "linework inside organic
  matter".
- For glitch, quantise the phase in the preset: route a stepped LFO into
  `morph`, or add `floor (vj_beat) * 1.7` to `t` behind a `step` parameter. The
  texture then jump-cuts to a new state on every beat instead of flowing.
- Scale warning: a strong warp compresses the domain locally, which raises the
  local frequency. Aliasing sparkle usually comes from there, not from the base
  `scale`. See the band-limiting notes in section 3.

### T2. Sine-wave turbulence (a cheap replacement for fbm warps)

- **Good for:** liquid or smoky swirls, heat haze, water caustic distortions, and
  wobbling linework. The shape looks like a fluid without solving one.
- **Mood:** either. Low amplitude and a slow phase give ambient smoke. With the
  amplitude on a hit envelope, it ripples.
- **Cost:** **cheap**: 8 `sin`, 8 small matrix operations, and no hashes. `sin`
  runs on Xe-LP's quarter-rate extended-math unit, but 8 of them are still far
  below the roughly 80 ALU operations of a single `vjNoise`.
- **Source:** Xor, "Turbulence" (GM Shaders Mini, mini.gmshaders.com/p/turbulence).
  It layers sine waves, each rotated about 53° (matrix 0.6/−0.8/0.8/0.6), with
  frequency ×1.4 per wave and amplitude/frequency displacement. About 8 waves
  already convince.

```glsl
// Displaces p by `waves` layered sine waves (Xor's turbulence), phase from the preset.
vec2 vjTurbulence (vec2 p, float phase, float amp, float freq)
{
    mat2 rot = mat2 (0.6, -0.8, 0.8, 0.6);
    for (int i = 0; i < 8; ++i)
    {
        float ph = freq * (p * rot).y + phase + float (i);
        p += amp * rot[0] * sin (ph) / freq;
        rot *= mat2 (0.6, -0.8, 0.8, 0.6);
        freq *= 1.4;
    }
    return p;
}

// INPUTS: flow (integrated), turb (0..1.2), ripple (0..1 hit), bands, emission
vec2 p = vjCentered();
vec2 q = vjTurbulence (p * 1.5, flow * 0.3, turb * (1.0 + ripple), 2.0);
// Contour bands of the displaced radius: smoke rings / tree-rings / wood grain.
float d = length (q) * bands;
float line = 1.0 - smoothstep (0.0, fwidth (d) * 1.5, abs (fract (d) - 0.5) - 0.02);
```

**Notes**
- A drop-in speed-up: replace the two `vjFbm` warp calls in ink, hot_blobs or
  membrane with `vjTurbulence` and keep **one** `vjFbm` for the final field. The
  character changes (more swirl, less cauliflower), so it is an art call, but
  the saving is about 10 noise calls per pixel.
- The displacement is smooth and bounded, so **iso-lines of the displaced
  coordinates** (`fract (q.x * k)`) give fluid-looking linework with constant
  anti-aliasing through `fwidth`.

### T3. Curl flow fields (divergence-free 2D velocity from a scalar potential)

- **Good for:** a velocity field that swirls without sinks or sources, so dye,
  particles and linework carried by it look like liquid or smoke and never
  collapse into dots. It drives T4 (feedback advection), the particle and state
  buffers, fibre direction (T6), and streak shading.
- **Mood:** ambient/organic is its natural home. For kinetic use, add a
  deliberately **non**-divergence-free radial burst on accents.
- **Cost:** **cheap**. With analytic-derivative value noise, the gradient costs
  about the same as one `vjNoise`, so 2 octaves cost about 2.2 noise calls.
- **Source:**
  - Bridson, Hourihan and Nordenstam, "Curl-Noise for Procedural Fluid Flow"
    (SIGGRAPH 2007).
  - In 2D the curl of a scalar potential ψ is v = (∂ψ/∂y, −∂ψ/∂x).
  - The analytic gradient of value noise comes from Inigo Quilez, "Value noise
    derivatives" (iquilezles.org/articles/morenoise): cubic `u' = 6f(1−f)`, and
    `∂n/∂x = (k1 + k3·v)·u'(x)`.
  - atyuwen's "Bitangent noise" (atyuwen.github.io/posts/bitangent-noise) is the
    3D/4D generalisation if we ever need volumes.

```glsl
// Value noise + analytic gradient: same 4 hashes as vjNoise. Returns (n, dn/dx, dn/dy).
vec3 vjNoiseD (vec2 p)
{
    vec2 i = floor (p), f = fract (p);
    vec2 u  = f * f * (3.0 - 2.0 * f);
    vec2 du = 6.0 * f * (1.0 - f);
    float a = vjHash12 (i),                   b = vjHash12 (i + vec2 (1.0, 0.0));
    float c = vjHash12 (i + vec2 (0.0, 1.0)), d = vjHash12 (i + vec2 (1.0, 1.0));
    float k1 = b - a, k2 = c - a, k3 = a - b - c + d;
    return vec3 (a + k1 * u.x + k2 * u.y + k3 * u.x * u.y,
                 du * vec2 (k1 + k3 * u.y, k2 + k3 * u.x));
}

// Divergence-free velocity. Two octaves are plenty for motion (detail comes from advection).
vec2 vjCurl (vec2 p, float phase)
{
    vec3 n1 = vjNoiseD (p + vec2 (phase * 0.11, 0.0));
    vec3 n2 = vjNoiseD (p * 2.03 + vec2 (7.1, phase * 0.17)) * 0.5;
    vec2 g = n1.yz + n2.yz * 2.03;             // chain rule for the scaled octave
    return vec2 (g.y, -g.x);
}

// Kinetic add-on: an accent flings material outward from a stone in the event ring
// (hot_blobs state pattern). Divergent on purpose - it tears the flow open.
vec2 burst (vec2 p, vec4 stone)               // stone = (centre.xy, birth, strength)
{
    float age = vj_time - stone.z;
    vec2 d = p - stone.xy;
    return stone.w * d / (dot (d, d) + 0.02) * exp (-age * 3.0) * step (0.0, age);
}
```

**Notes**
- The same `vjNoiseD` enables Quilez's "slope-attenuated fbm"
  (`a / (1 + dot (d, d))`), which gives eroded, ridge-and-plateau shapes.
  Ambient landscapes come almost free once you have derivatives.
- Keep velocity magnitude bounded. The house already does this in hot_blobs:
  `vel *= min (1.0, 3.0 / length (vel))`. A hit envelope should scale speed, not
  add unbounded energy.

### T4. Feedback advection and image-based flow (the TouchDesigner "Feedback + Displace" staple, done right)

- **Good for:** trails, ink in water, smoke, "painting with the flow", long
  streaks, echo tunnels. Also the core trick of **temporal accumulation for
  cheap detail**: each frame is only one or two texture reads, but detail builds
  up over many frames.
- **Mood:**
  - Ambient: slow curl flow and long persistence.
  - Kinetic: zoom and rotate feedback tunnels, with a hard flush on accents.
- **Cost:** **cheap** at 0.5 buffer scale: 1 to 3 fetches plus the flow field.
  The cost is mostly bandwidth, so keep the buffer at half resolution and
  RGBA16F (see section 3).
- **Source:**
  - Semi-Lagrangian back-trace: Stam, "Stable Fluids" (1999), as implemented on
    GPUs by Mark Harris, *GPU Gems* ch. 38 "Fast Fluid Dynamics Simulation on the
    GPU".
  - Image-Based Flow Visualization (IBFV): van Wijk, SIGGRAPH 2002. Each frame
    is a blend of the previous frame, warped along the flow, and a little fresh
    filtered noise. The result emulates line-integral-convolution streaks at
    about 50 fps on 2002 hardware.
  - The TouchDesigner feedback idiom: Feedback TOP → Displace by slow, low-contrast
    noise → Level (opacity 0.9 per frame); see LucieMrc,
    "TD_feedback_love" (github.com/LucieMrc/TD_feedback_love_EN).
  - House precedent: `feedback_smear.fs`, which decays by a time constant
    instead of a fixed per-frame factor.

```glsl
/*{ ... "PASSES": [
    { "TARGET": "dye", "PERSISTENT": true, "WIDTH": "0.5", "HEIGHT": "0.5" },
    { } ] }*/
// INPUTS: drift (integrated), speed, flow_scale, persistence (s), grain_amt, ink (hit), emission
if (PASSINDEX == 0)
{
    vec2 uv = isf_FragNormCoord;
    vec2 aspect = vec2 (RENDERSIZE.x / RENDERSIZE.y, 1.0);
    vec2 p = (uv - 0.5) * 2.0 * aspect;                  // = vjCentered() in this pass
    float dt = clamp (abs (vj_dt), 0.0, 0.05);           // freezes with the scene clock
    float running = step (1e-5, dt);

    vec2 vel = vjCurl (p * flow_scale, drift) * speed;
    vec2 from = uv - vel * dt * 0.5 / aspect;            // back-trace (p-space -> uv-space)
    vec3 prev = texture2D (dye, from).rgb;

    float keep = mix (1.0, exp (-dt / max (persistence, 1e-3)), running);
    // IBFV: a trickle of fresh spots, re-rolled 8x a second, keeps streaks forming.
    float spot = step (0.992, vjHash12 (floor (gl_FragCoord.xy / 2.0) + floor (vj_time * 8.0)));
    // A hit pours ink in a band (or at the event-ring stones, hot_blobs style).
    float pour = ink * (1.0 - smoothstep (0.0, 0.25, abs (p.y + 0.3 * sin (drift * 0.1))));
    vec3 add = vec3 (spot * grain_amt + pour) * running;
    gl_FragColor = vec4 (min (prev * keep + add, vec3 (16.0)), 1.0);   // bounded HDR
}
else
{
    float v = texture2D (dye, isf_FragNormCoord).r;
    gl_FragColor = vec4 (vec3 (1.0, 0.35, 0.25) * v * emission, 1.0);
}
```

**Notes**
- **Numerical diffusion is a feature and a bug.** Every bilinear back-trace
  blurs by up to half a texel. For ambient watercolour that is ideal. For crisp
  kinetic streaks:
  - take fewer, larger steps (move the dt into the flow, not the frame rate), or
  - add an unsharp term `prev += k * (prev - blurredPrev)`, or
  - use MacCormack/BFECC, which trace back and forth and cost 2 extra fetches.
- **Kinetic tunnel variant:**
  `from = 0.5 + vjRot (spin * dt) * (uv - 0.5) * (1.0 - zoom * dt)`, plus a
  `flush` hit that multiplies `keep` by `(1 - flush)`, as feedback_smear does.
  Zoom-feedback is the single most VJ-recognisable effect, so use it with
  restraint.
- **Freeze-safety:** gate injection with `running`. Otherwise a frozen clock
  keeps adding the same spots every frame and the buffer saturates.
- **Resolution resets:** the buffer is a fraction of the render size, so every
  step of the adaptive quality ladder clears the trails (see 1.5). For
  long-memory ambient scenes this is visible. Hold the scale fixed while such a
  scene is live, or accept it.

### T5. Single-pass fluid at quarter resolution ("Simple and Fast Fluids")

- **Good for:** real smoke and ink behaviour: curling vortices, puffs that roll
  over each other. This is what T4 cannot do alone, because T4's flow is
  prescribed and never reacts to what it carries.
- **Mood:**
  - Ambient/organic: high viscosity and low vorticity give oil or ink.
  - Kinetic: low viscosity, strong vorticity confinement, and a splat on every
    kick give smoke explosions.
- **Cost:** **medium**. Take a 0.25-scale velocity buffer (480×270 at 1080p),
  3 iterations per frame, and 5 RGBA32F fetches each: about 2M fetches, plus a
  half-res dye pass (T4).
- **Source:**
  - Guay, Colin and Egli, "Simple and Fast Fluids" (*GPU Pro 2*, 2011). The
    whole Navier-Stokes step runs in one pixel shader by temporarily relaxing
    incompressibility, in under 40 lines.
  - nimitz, "Chimera's Breath" (Shadertoy 4tGfDW; an ISF port exists in
    grigM/ISF-shaders-collection). It adds vorticity confinement, with the curl
    stored in `.w`, and runs 3 sim passes per frame.
  - Constants used there: dt 0.15, K 0.2, viscosity 0.55, vorticity 0.11.

```glsl
/*{ ... "PASSES": [
    { "TARGET": "state", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.02", "HEIGHT": "0.02" }, // event ring
    { "TARGET": "fluid", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.25", "HEIGHT": "0.25" },
    { "TARGET": "fluid", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.25", "HEIGHT": "0.25" },
    { "TARGET": "fluid", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.25", "HEIGHT": "0.25" },
    { "TARGET": "dye",   "PERSISTENT": true, "WIDTH": "0.5",  "HEIGHT": "0.5" },
    { } ] }*/
// fluid: .xy velocity, .z density (pressure proxy), .w curl
// INPUTS: viscosity (0..1), swirl (0..0.3), splat (hit), emission ...
vec4 fluidStep (vec2 uv, vec2 w, float dt)
{
    const float K = 0.2;
    float nu = mix (0.1, 0.8, viscosity);
    vec4 c  = texture2D (fluid, uv);
    vec4 tr = texture2D (fluid, uv + vec2 (w.x, 0.0)), tl = texture2D (fluid, uv - vec2 (w.x, 0.0));
    vec4 tu = texture2D (fluid, uv + vec2 (0.0, w.y)), td = texture2D (fluid, uv - vec2 (0.0, w.y));
    vec3 dx = (tr.xyz - tl.xyz) * 0.5, dy = (tu.xyz - td.xyz) * 0.5;
    vec2 densDif = vec2 (dx.z, dy.z);
    c.z -= dt * dot (vec3 (densDif, dx.x + dy.y), c.xyz);                  // mass
    vec2 visc = nu * (tu.xy + td.xy + tr.xy + tl.xy - 4.0 * c.xy);         // viscosity
    c.xyw = texture2D (fluid, uv - dt * c.xy * w).xyw;                     // self-advection
    c.xy += dt * (visc - K / dt * densDif);                                 // pressure-ish
    c.w = tr.y - tl.y - tu.x + td.x;                                         // curl
    vec2 vort = vec2 (abs (tu.w) - abs (td.w), abs (tl.w) - abs (tr.w));
    c.xy += swirl / (length (vort) + 1e-9) * vort * c.w;                    // vorticity confinement
    c.xy *= 1.0 - smoothstep (0.45, 0.5, max (abs (uv.x - 0.5), abs (uv.y - 0.5))); // walls
    c.z = clamp (c.z, 0.5, 3.0);
    return c;
}
// In the fluid passes: c = fluidStep (uv, 1.0 / RENDERSIZE, 0.15 * running);
// then add forces from the event-ring stones (a Gaussian splat of velocity along a
// hashed direction, strength = stone.w * exp (-age * 8.0)).
```

**Notes**
- Forces should come from the **state event ring**, not from reading audio.
  - hot_blobs already converts the `drop` envelope's rising edge into "a stone
    at (x, y, birth time)".
  - Reuse that exact pass. Each stone becomes a splat for about 0.1 s.
  - A continuous source, such as a `breath` parameter fed by `react.body`, can
    be a steady jet from the bottom edge.
- Keep the sim `FLOAT` (RGBA32F). Velocities in fp16 drift, and the confinement
  term amplifies the error. Keep the dye at RGBA16F and half resolution.
- Couple the sim to wall time with care. The step uses a fixed dt (0.15, the
  stability sweet spot) times `running`, so the fluid runs at *frame* rate, not
  scene speed. For speed-follows-clock behaviour, scale the number of effective
  steps instead: skip passes when `vj_dt` is small. Never raise dt past about
  0.2.

### T6. Reaction–diffusion, upgraded (style maps, orientation, flow, cheaper stencils)

- **Good for:** coral, fingerprints, labyrinths, cell division, skin and bark.
  It is the most "alive" pattern generator available. Morphogen already runs
  plain Gray-Scott at quarter resolution with 16 steps per frame.
- **Mood:**
  - Ambient/organic by default.
  - Glitch variant: display it one-bit (hard threshold with ordered dither), and
    have snares erase bands (`kill` spike in a hashed row band) so the pattern
    regrows.
- **Cost:** **medium**, and it is **bandwidth-bound**, not ALU-bound. Each of
  morphogen's 16 passes writes 480×270 RGBA32F (about 2 MB) and reads it back,
  so roughly 60 to 70 MB moves per frame. That is about 1 ms of Iris Xe memory
  bandwidth before any shading.
- **Source:**
  - Karl Sims, "Reaction-Diffusion Tutorial" (karlsims.com/rd.html):
    - 3×3 Laplacian with centre −1, edges 0.2 and diagonals 0.05.
    - DA = 1, DB = 0.5, f = 0.055, k = 0.062.
    - Extensions: a *style map* (f and k vary over space), *orientation*
      (anisotropic diffusion) and *flow*.
  - Robert Munafo's xmorphia map of Pearson's parameter space
    (mrob.com/pub/comp/xmorphia) for picking f and k families.
  - Jonathan McCabe's multi-scale Turing patterns, for the "blur-difference"
    variant (GPU notes by Ricky Reusser, rreusser.github.io/notebooks/multiscale-turing-patterns).

```glsl
// Inside a morphogen-style RD pass (rd.r = 1 - A, rd.g = B).
vec2 uv = gl_FragCoord.xy / RENDERSIZE;
vec2 aspect = vec2 (RENDERSIZE.x / RENDERSIZE.y, 1.0);
vec2 p = (uv - 0.5) * 2.0 * aspect;

// 1. Style map (Sims): pattern family drifts across the frame, so one culture holds
//    coral in one region and worms in another. Evaluated at quarter res only.
float styleT = pattern + style_spread * (vjNoise (p * 1.3 + drift * 0.02) - 0.5);
vec2 fk = feedKill (styleT);

// 2. Orientation (Sims): diffuse faster along a direction -> aligned fibres/fingerprints.
vec2 dir = normalize (vjCurl (p * 0.8, drift) + 1e-5);
vec2 t = 1.0 / RENDERSIZE;
vec2 along = dir * t, across = vec2 (-dir.y, dir.x) * t;
float wA = 0.2 * (1.0 + aniso), wC = 0.2 * (1.0 - aniso);              // aniso 0..0.8
vec2 c = texture2D (rd, uv).rg;
vec2 lap = -c * (2.0 * wA + 2.0 * wC)
         + wA * (texture2D (rd, uv + along).rg + texture2D (rd, uv - along).rg)
         + wC * (texture2D (rd, uv + across).rg + texture2D (rd, uv - across).rg);
// (5 fetches instead of 9. The centre weight keeps the stencil zero-sum.)

// 3. Flow (Sims): sample the "centre" from upstream -> the pattern slides and shears.
c = texture2D (rd, uv - vjCurl (p, drift) * flow_amt * t * 2.0).rg;
```

**Notes**
- **Precision:** keep `FLOAT: true`. Per-step changes are on the order of 1e-3
  around values near 0.5, and fp16 spacing at 0.5 is about 5e-4, so fp16
  patterns stall or band.
- **Cheaper growth:** fewer passes with a larger scale factor per step do not
  work, because Gray-Scott's dt is capped at about 1. Morphogen correctly clamps
  `step1 <= 1`. To save cost, drop the buffer to 0.2 scale, or run 8 passes and
  accept half-speed growth. The relief lighting in the display pass hides the
  resolution well.
- **Multi-scale Turing patterns** (McCabe) need blurs of several radii each step.
  - With FFT or mipmaps they are fast. Reusser and janivanecky report mipmap
    approximations taking a 2048² step from about 0.66 s to 0.025 s.
  - Our pass targets have **no mipmaps**, so this variant needs an engine
    change first (see 3.5).
  - The cheap TouchDesigner imitation is a feedback loop of blur → sharpen or
    high-pass → contrast. It gives Turing-like stripes without chemistry, but it
    is less stable and less varied.
- **Seeding** already uses the house idiom: `floor (vj_beat * 4.0)` picks a new
  site every 16th note, gated by `seed_amt`. A cleaner kinetic version plants
  seeds at the event-ring stone positions, so seeds land exactly where the ring
  ripples are.

### T7. SDF linework and plexus networks (the house kinetic signature)

- **Good for:** Noise Diary-style fine plexus bodies, wireframes, constellations,
  circuit traces and scanner lines. This is the style the library is built on.
  See mesh_body.fs, and pylon.fs for projected beams.
- **Mood:** glitch/kinetic, or delicate ambient when the lines are hair-thin
  and slow.
- **Cost:** **medium to expensive.** Cost is roughly
  *segments tested per pixel × layers*. mesh_body tests 12 segments and 9 point
  constructions per layer, over 4 layers. That is about 48 segment tests plus
  36 hashed and sin-animated points per pixel.
- **Source:**
  - Segment distance: Inigo Quilez, "2D distance functions"
    (iquilezles.org/articles/distfunctions2d).
  - Anti-aliasing width: "Perfecting anti-aliasing on signed distance functions"
    (blog.pkh.me/p/44). `fwidth` (L1) against `length (vec2 (dFdx, dFdy))` (L2)
    makes little visible difference.
  - Grid-plexus layout: BigWings (Martijn Steinrucken), "The Universe Within"
    (Shadertoy lscczl), which uses jittered grid points, links to the 8
    neighbours plus cross-links, and brightness by link length.
  - The "short = bright, long = faint" rule is the After Effects Plexus look.

```glsl
float sdSegment (vec2 p, vec2 a, vec2 b)                  // IQ
{
    vec2 pa = p - a, ba = b - a;
    return length (pa - ba * clamp (dot (pa, ba) / dot (ba, ba), 0.0, 1.0));
}

// Energy-conserving hairline: a line thinner than a pixel DIMS instead of aliasing
// (pylon.fs does this with clamp (halfW / (px * 0.5), 0.15, 1.0)).
float hairline (float d, float halfW, float px)
{
    float w = max (halfW, px * 0.5);
    float cover = 1.0 - smoothstep (w - px * 0.5, w + px * 0.5, d);
    return cover * clamp (halfW / (px * 0.5), 0.0, 1.0);
}

// Optional glow tail: cheap, but widen `reach` only if the budget allows
// (every pixel within reach of a line pays the full segment test).
float glowTail (float d, float px, float k) { return px * k / (d + px * k); }

// Early reject with a bounding circle before the full segment maths
// (pays off when most links are far from most pixels - coherent across the screen).
bool nearLink (vec2 q, vec2 a, vec2 b, float reach)
{
    vec2 m = 0.5 * (a + b);
    float r = 0.5 * length (b - a) + reach;
    return dot (q - m, q - m) < r * r;
}
```

**Notes on doing it better on Iris Xe**
- **Draw the head, let feedback draw the tail.** For moving points or
  scanners, draw only a short segment from last frame's position to this
  frame's (or just the point) into a T4 feedback buffer. The trail's length
  then comes from persistence, not from per-pixel segment tests. A long glowing
  line costs one segment.
- **Register pressure:** mesh_body keeps `pts[9]` and `ids[9]` (36 floats) live
  across the link loop. Intel's guide recommends about **16 live temporaries**
  to keep SIMD16/32 compilation and avoid spills. Recomputing a neighbour's
  point inline (hash plus sin, all cheap ALU) is often faster than holding it in
  an array. Measure both.
- **Precomputing points into a tiny state texture** (one texel per grid cell)
  replaces the hash and 2 `sin` with one `texelFetch` per point. On Xe-LP this
  is roughly a wash: 36 fetches per pixel is sampler-bound. Do it only when the
  points carry *physics* (velocity, audio kicks, springs) that must persist
  between frames.
- **Layer by depth:** thin, dim far layers can use a coarser grid (fewer
  cells, so fewer segments per screen area) and skip the glow tail.
- **Determinism** is the anti-flicker rule. Everything about a link (whether it
  exists, its brightness, erosion) must be a hash of its **two endpoint cell
  ids**, never of the pixel's own cell. mesh_body's `link()` is the template.

### T8. Exact Voronoi borders (cracks, cells, shattered glass, membranes)

- **Good for:** crisp cellular linework of **constant width**. The naive
  `F2 − F1` gives borders that fatten and thin. With the owning cell id you also
  get per-cell hashes: flash, invert, or drop out single cells.
- **Mood:**
  - Kinetic/glitch: the points jump on the beat (shatter), cells flash on snares.
  - Ambient: slow drift gives living tissue, leaf veins, dried earth.
- **Cost:** **medium**. That is 9 + 25 hashed, animated points (about 34 hash22
  and 34 `sin(vec2)`). Cutting the second loop to 3×3 is **cheap-medium** and
  only occasionally misses a border when jitter is large.
- **Source:** Inigo Quilez, "Voronoi edges" (iquilezles.org/articles/voronoilines).
  - Pass 1 (3×3) finds the closest point.
  - Pass 2 (5×5 around it) computes `dot (0.5 * (mr + r), normalize (r - mr))`,
    the exact distance to each bisector.

```glsl
// Returns (border distance, owning cell id). phase animates the sites.
vec3 vjVoronoiBorder (vec2 x, float phase)
{
    vec2 n = floor (x), f = fract (x), mg = vec2 (0.0), mr = vec2 (0.0);
    float md = 8.0;
    for (int j = -1; j <= 1; ++j)
        for (int i = -1; i <= 1; ++i)
        {
            vec2 g = vec2 (float (i), float (j));
            vec2 o = 0.5 + 0.45 * sin (phase + VJ_TAU * vjHash22 (n + g));
            vec2 r = g + o - f;
            float d = dot (r, r);
            if (d < md) { md = d; mr = r; mg = g; }
        }
    md = 8.0;
    for (int j = -2; j <= 2; ++j)
        for (int i = -2; i <= 2; ++i)
        {
            vec2 g = mg + vec2 (float (i), float (j));
            vec2 o = 0.5 + 0.45 * sin (phase + VJ_TAU * vjHash22 (n + g));
            vec2 r = g + o - f;
            if (dot (mr - r, mr - r) > 1e-5)
                md = min (md, dot (0.5 * (mr + r), normalize (r - mr)));
        }
    return vec3 (md, n + mg);
}

// Usage - INPUTS: drift (integrated), shatter (step phase from the preset), flash (snare hit)
vec2 x = vjCentered() * cells;
vec3 vb = vjVoronoiBorder (x, drift * 0.2 + shatter * 3.1);
float px = 2.0 * cells / RENDERSIZE.y;
float crack = hairline (vb.x, width * px, px);
float cellFlash = flash * step (0.8, vjHash12 (vb.yz + floor (vj_beat)));   // a few cells light up
float v = crack + cellFlash * 0.4 + 0.25 * exp (-vb.x * 18.0) * glow;       // soft membrane glow
```

**Notes**
- `shatter`: route a stepped source into it (for example a trigger action that
  adds 1 per accent) so the sites *jump*. Smooth `drift` makes them *crawl*.
  Combined, cracks crawl and jump on the hit.
- The border distance is a proper distance field. Hairline, outline, onion
  rings (`abs (fract (d * k) - 0.5)`) and glow all work unchanged.
- Warp `x` first with T2 turbulence: cracks turn into tissue.

### T9. Faking volume in 2D (fog, gas, light shafts, depth)

- **Good for:** the ambient style's sense of depth and atmosphere: smoke you
  could walk into, light coming through haze, forms half-lost in fog. No
  raymarching needed.
- **Mood:** ambient/organic mainly. A shaft flare on a kick is a strong kinetic
  accent.
- **Cost:** a to d are **cheap**; e is **cheap to medium**.
- **Source:**
  - Directional-derivative lighting: Inigo Quilez, "Directional derivative"
    (iquilezles.org/articles/derivative). One extra sample toward the light
    replaces a full 3 to 6 sample gradient, which he measured at about 3.5×
    faster.
  - Light shafts: Kenny Mitchell, "Volumetric Light Scattering as a
    Post-Process", *GPU Gems 3* ch. 13. It is a radial blur of an
    occluder/emitter image toward the light, with Density, Weight, Decay and
    Exposure controls, rendered from a downsampled source.
  - Slice crossfade: house precedent in `signal_fog.fs volume()`.

The five sub-techniques:

- **a. Parallax slices.** Draw N layers of the same field, each scaled and moved
  by 1/depth and dimmed with depth. fibers.fs does exactly this, and 3 to 4
  slices usually read as volume. The cost scales linearly, so give far slices
  fewer octaves.
- **b. Evolving through a 3rd dimension without sliding.** Crossfade two noise
  slices, as signal_fog does. Or use a single-fetch 3D noise texture (see
  section 3, "texture noise"), where the fractional `z` is time.
- **c. Directional-derivative lighting of a density field.** This gives
  silver-lining rims and self-shadowed gas from **one** extra evaluation. When
  even that is too much, use the free `dFdx`/`dFdy` gradient, as hot_blobs
  does.
- **d. Beer–Lambert.** Treat density as optical depth. Transmittance `T = exp (−k·τ)`
  and glow `∝ 1 − T` turn hard thresholded blobs into luminous gas with soft,
  physically plausible edges.
- **e. Light shafts.** Radial-blur a **quarter-res** emission buffer toward a
  light point. Jitter the first sample per pixel with a hash, so banding becomes
  noise that the LookPass grain then swallows. 16 to 24 samples at quarter res
  cost less than one full-res fbm.

```glsl
// c + d: gas lit from `sunDir` (2D screen direction), density den(p) in 0..1.
float d0 = den (p);
float dif = clamp ((d0 - den (p + 0.3 * sunDir)) / 0.6, 0.0, 1.0);   // IQ clouds form
float T = exp (-thickness * d0 * 3.0);                                 // Beer-Lambert
vec3 gas = (1.0 - T) * mix (vec3 (0.05, 0.02, 0.02), vec3 (1.0, 0.45, 0.3), dif);
gas += (1.0 - T) * T * rim * 2.0 * vec3 (1.0, 0.6, 0.5);              // bright thin edges

// e: light shafts. PASSES: { "TARGET": "emit", "WIDTH": "0.25", "HEIGHT": "0.25" }, { }
// pass 0 writes the bright parts of the scene (or a cheap silhouette) to `emit`.
vec3 shafts (vec2 uv, vec2 lightUv, float density, float decay, float weight)
{
    const int N = 20;
    vec2 stepUv = (uv - lightUv) * density / float (N);
    vec2 s = uv - stepUv * vjHash12 (gl_FragCoord.xy + fract (vj_time) * 61.0);  // jittered start
    float illum = 1.0;
    vec3 acc = vec3 (0.0);
    for (int i = 0; i < N; ++i)
    {
        s -= stepUv;
        acc += texture2D (emit, s).rgb * illum * weight;
        illum *= decay;
    }
    return acc;
}
// final: col += shafts (isf_FragNormCoord, light, 0.9, 0.95, 0.06) * exposure * (1.0 + 2.0 * flare);
```

**Notes**
- Shafts are the best "big reaction for little cost" in the ambient palette. A
  `flare` hit that raises `exposure` for about 300 ms reads as the whole
  atmosphere reacting.
- Keep the emission buffer non-persistent. It is recomputed each frame. If the
  jitter noise is too visible, feed the shafts through a T11 accumulation
  buffer.

### T10. Low-res field, full-res edge (distance-field magnification)

- **Good for:** any scene that **thresholds a smooth field**: blobs, ink, fog
  forms, islands, and the membrane silhouette. This is the single biggest
  performance lever in the current library. The field is smooth, so it can be
  computed at 1/16 of the pixels. Only its *edge* needs full resolution, and a
  threshold of a bilinearly-upsampled field gives a crisp, resolution-independent
  edge.
- **Mood:** either.
- **Cost:** turns an **expensive** field into a **cheap** one. A 17-noise domain
  warp at 0.25 scale costs about the same as 1 noise call at full resolution.
- **Source:** Chris Green (Valve), "Improved Alpha-Tested Magnification for
  Vector Textures and Special Effects" (SIGGRAPH 2007 course). A distance field
  stored in a **low-resolution** texture, thresholded after bilinear filtering,
  gives sharp magnified edges and cheap outlines, glows and drop shadows.
  Morphogen already renders its simulation at 0.25 scale and lights it at full
  resolution.

```glsl
/*{ ... "PASSES": [
    { "TARGET": "field", "WIDTH": "0.25", "HEIGHT": "0.25" },   // not persistent: recomputed
    { } ] }*/
if (PASSINDEX == 0)
{
    vec2 p = vjCentered();                     // valid: RENDERSIZE is the field's size here
    vec2 sp = p * scale;
    vec2 w = vec2 (vjFbm (sp + vec2 (0.0, flow)), vjFbm (sp + vec2 (3.7, -flow * 0.7)));
    float f = vjFbm (sp + curl * w);
    gl_FragColor = vec4 (f, w, 1.0);           // keep the warp too: free extra channels
}
else
{
    vec4 F = texture2D (field, isf_FragNormCoord);
    // Restore full-res edge detail with ONE cheap noise at pixel scale (ink.fs "fibre" trick).
    float fibre = (vjNoise (gl_FragCoord.xy * 0.35) - 0.5) * 0.04 * ragged;
    float f = F.r + fibre;
    float aa = fwidth (f) * 1.5 + 1e-4;
    float inside = smoothstep (threshold - aa, threshold + aa, f);
    float rimLine = 1.0 - smoothstep (0.0, aa * 3.0, abs (f - threshold));
    // Normals: dFdx of the upsampled field is faceted at 4x magnification ->
    // shade from a SMOOTHER signal (the warp channels) or blur the normal in pass 0.
    ...
}
```

**Notes**
- **The facet artefact:** bilinear upsampling is only C0. Thresholds stay clean,
  but *gradients* (relief lighting) show a 4-pixel grid at 0.25 scale. The fixes:
  - use 0.5 scale for fields that get lit,
  - compute the normal in pass 0 and store it in `.gb`, or
  - use a cheap bicubic built from 4 bilinear fetches (Sigg and Hadwiger,
    *GPU Gems 2* ch. 20).
- **Candidates in the library:**
  - hot_blobs: the whole mass field. Ring height stays analytic at full
    resolution.
  - ink: the blot field. The fibre and dots stay full resolution.
  - signal_fog: the form. The particles stay full resolution.
  - membrane: the `sheet()` height and gradient. That is about 60 noise calls
    per pixel dropping to about 4.
- The same split explains why whole soft scenes can simply use a lower stage
  `"scale"`. The LookPass film grain and dither run at *output* resolution after
  upscaling, and they restore the apparent fine texture.

### T11. Temporal amortisation: refresh a slow field a fraction of the pixels per frame

- **Good for:** ambient scenes whose base field changes slowly. The base can be
  recomputed for 1/4 (or 1/16) of its texels each frame, and the rest keep last
  frame's value, optionally shifted by the known drift. Also for **noisy
  estimators**: jittered shafts, soft shadows, bokeh, few-step fog. Each frame
  takes one cheap noisy sample, and an exponential moving average converges it.
- **Mood:** ambient/organic only. Fast motion makes it ghost.
- **Cost:** **cheap**. The expensive field cost drops by 4 to 16×, plus 1 fetch.
- **Source:**
  - Schneider (Guerrilla), "The Real-Time Volumetric Cloudscapes of Horizon
    Zero Dawn" (SIGGRAPH 2015): a quarter-res buffer, 1 of every 16 pixels in
    each 4×4 block updated per frame, with the rest reprojected.
  - A walkthrough: vertexfragment.com "Upsampling to Improve Volumetric Cloud
    Render Performance".
  - Golden-ratio temporal jitter over blue noise: Alan Wolfe, "Ray Marching Fog
    With Blue Noise" (blog.demofox.org, 2020).

```glsl
/*{ ... "PASSES": [
    { "TARGET": "state", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.02", "HEIGHT": "0.02" }, // texel 0 = frame counter
    { "TARGET": "cache", "PERSISTENT": true, "WIDTH": "0.5", "HEIGHT": "0.5" },
    { } ] }*/
// Pass 0: texel 0.x += 1 each frame (only while running), so every phase is visited
// in turn even at variable fps.
// Pass 1 (cache):
float frame = texelFetch (state, ivec2 (0, 0), 0).x;          // just written in pass 0
ivec2 px = ivec2 (gl_FragCoord.xy);
int slot = (px.x & 1) + 2 * (px.y & 1);                      // 2x2 interleave -> 1/4 per frame
vec2 uv = isf_FragNormCoord;
vec2 driftUv = driftVel * vj_dt;                             // known bulk motion of the field
vec4 old = texture2D (cache, uv - driftUv);                  // reproject the stale 3/4
if (slot == int (mod (frame, 4.0)) || old.a < 0.5)           // .a = "valid" (buffers start at 0)
    gl_FragColor = vec4 (expensiveField (uv), 1.0);
else
    gl_FragColor = old;

// Noisy estimator accumulation (e.g. T9 shafts):
//   acc = mix (texture2D (accBuf, uv).rgb, noisySample, 0.1);   // ~10-frame memory
//   jitter = fract (vjHash12 (gl_FragCoord.xy) + frame * 0.61803398875);   // golden ratio
```

**Notes**
- **The design rule: slow base in the cache, fast accents on top.** Hits,
  flashes, ring ripples and dust are computed per pixel in the final pass, where
  they are cheap. Only the slow, expensive base goes through the cache. Then
  ghosting never touches the reactive part.
- The cache is `PERSISTENT`, so it is **wiped when the quality ladder changes
  scale** (1.5). The `.a` validity flag makes it self-heal: an invalid texel is
  recomputed immediately. After a reset, the first frame costs the full amount.
- Branch divergence: within one SIMD thread, 1 of every 4 pixels takes the
  expensive branch. On Xe-LP a thread covers 8, 16 or 32 pixels, so *every*
  thread executes the expensive path for its quarter, and the lanes idle for the
  rest. The saving is real but smaller than 4×. **Blocked** interleaving (whole
  4×4 or 8×8 tiles refreshed per frame, chosen by tile id) keeps threads
  coherent and gets close to the full 4×. Prefer it.
- There is no frame-index uniform. Keep a counter in a state texel, as shown.
  After pass 0 writes and flips, a later pass reading `state` sees the new value.

### T12. Block-state glitch, datamosh hold, and progressive pixel sorting

- **Good for:** the kinetic and glitch vocabulary: block displacement, channel
  shuffles, frozen "P-frame" smears, rows torn sideways, sorted-pixel streaks.
  It is controllable, beat-quantised, and cheap.
- **Mood:** glitch/kinetic.
- **Cost:** **cheap**. That is 1 `texelFetch` from a tiny state texture, plus
  one extra image fetch. Pixel sorting is **medium**: one pass per sort step.
- **Source:**
  - Keijiro Takahashi, KinoGlitch (github.com/keijiro/KinoGlitch):
    - The "digital glitch" is driven by a tiny random block texture refreshed by
      script.
    - Per-block thresholds pick displacement, trash-frame substitution or
      channel shuffle.
    - The "analog glitch" adds scan-line jitter, vertical jump, horizontal shake
      and colour drift.
  - Datamosh as "resample the previous output along block motion vectors while
    the trigger is held": Codrops, "Breaking the Frame: Building a Real-Time
    Datamosh Effect with Three.js" (2026).
  - Pixel sorting by odd-even transposition steps in a feedback loop: ciphrd,
    "Pixel sorting on shader using well-crafted sorting filters" (2020).
  - House precedent: the rising-edge event detection in hot_blobs, and the row
    bands in ink and terminal.

```glsl
/*{ ... "INPUTS": [ { "NAME": "inputImage", "TYPE": "image" },
     { "NAME": "glitch", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },   // env.pulse
     { "NAME": "amount", "TYPE": "float", "DEFAULT": 0.3, "MIN": 0.0, "MAX": 1.0 },   // macro
     { "NAME": "hold",   "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 } ], // mosh
  "PASSES": [
    { "TARGET": "blocks", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.02", "HEIGHT": "0.02" },
    { "TARGET": "mosh",   "PERSISTENT": true, "WIDTH": "0.5", "HEIGHT": "0.5" },
    { } ] }*/
// Pass 0 - one texel per screen block (38x22 at 1080p, ~50 px blocks).
// Texel (0,0) doubles as bookkeeping: .x = previous glitch value, .y = event counter.
vec4 book = texelFetch (blocks, ivec2 (0, 0), 0);
bool fire = glitch > 0.15 && book.x <= 0.15;                 // rising edge, hot_blobs style
ivec2 b = ivec2 (gl_FragCoord.xy);
vec4 me = texelFetch (blocks, b, 0);                         // (offset.xy, mode, life)
if (b == ivec2 (0, 0)) { gl_FragColor = vec4 (glitch, book.y + (fire ? 1.0 : 0.0), 0.0, 0.0); return; }
float dt = clamp (abs (vj_dt), 0.0, 0.05);
me.w = max (me.w - dt * 4.0, 0.0);                           // damage heals in ~250 ms
if (fire && vjHash12 (vec2 (b) + book.y * 7.3) < amount)
{
    vec2 h = vjHash22 (vec2 (b) * 1.7 + book.y);
    // Offsets snap to whole blocks horizontally: rigid slabs, not smears.
    me = vec4 (floor ((h.x - 0.5) * 6.0) / RENDERSIZE.x, 0.0, h.y, 1.0);
}
gl_FragColor = me;

// Final pass:
vec2 uv = isf_FragNormCoord;
vec2 bs = floor (RENDERSIZE * 0.02 + 0.5);                  // blocks-buffer size, house formula
vec4 g = texelFetch (blocks, ivec2 (uv * bs), 0);
vec2 guv = uv + g.xy * step (0.01, g.w);
vec3 c = IMG_NORM_PIXEL (inputImage, guv).rgb;
if (g.z > 0.7 && g.w > 0.0) c = c.gbr;                      // channel shuffle
vec3 m = texture2D (mosh, guv).rgb;                         // held, displaced past
c = mix (c, m, hold * step (0.5, g.z) * g.w);               // "P-frame" blocks keep the past
```

**Notes**
- **Glitch the domain, not only the image.** Inside a *generator*, apply the
  same block offsets to `p` before evaluating the field. You get tears with
  perfectly crisp, anti-aliased edges and nothing sampled twice. This is cheaper
  and cleaner than post-processing, and suits vector-like plexus scenes.
- **Rate limits:** LookPass already caps flashes and cuts at 3 per second. Keep
  block heal times of 150 ms or more, so strobing stays inside that
  photosensitivity guard.
- **Progressive pixel sort:** repeat one `TARGET` 8 to 16 times at 0.5 scale.
  - Each pass compares a pixel with its partner at ±1 along the sort axis,
    alternating parity per pass.
  - Parity comes from `PASSINDEX`; a frame counter goes in state.
  - Take `min` or `max` of the luma keys, and only where a mask (threshold on
    luma, or a block's `mode`) allows.
  - Streaks grow over about a second. That motion reads as "the image is being
    processed", which is a strong glitch gesture. Cost: 2 fetches × passes ×
    0.25 of the pixels.

---

## 3. Iris Xe / integrated-GPU performance budgeting

### 3.1 The machine in numbers

Sources:
- Intel oneAPI GPU optimisation guide, "Intel Iris Xe GPU Architecture".
- Intel, "Processor Graphics Xe-LP API Developer and Optimization Guide".
- Public spec aggregators: cpu-monkey, topcpu.

Treat every figure as approximate.

| Resource | Xe-LP (96 EU) | What it means for us |
|---|---|---|
| FP32 ALU | 96 EU × SIMD8, up to about 1.3 GHz: about 1 T lane-instructions/s, about 2 to 2.5 TFLOPS counting FMA | About 480 lane-instructions per pixel per ms at 1080p at 100 % efficiency. **Plan on about half.** |
| Extended math (sin, cos, exp, log, pow, sqrt, rcp) | **SIMD2**, a quarter of the ALU rate | `pow (x, 90.0)` costs exp2 + log2. Prefer multiplies, `inversesqrt`, and reuse. vjHash (no `sin`) was a good choice. |
| Threads and registers | 7 threads per EU, 128 × 32 B GRF each | Intel recommends about 16 or fewer live temporaries. More forces SIMD8 or spills. |
| Texture | about 48 TMU, about 50 GTexel/s bilinear 8-bit | About **0.04 ms per fetch per pixel** at 1080p at peak. Plan 0.08 ms. Float formats filter slower. |
| Memory | Shared LPDDR4x/DDR4, 128-bit, up to about 68 GB/s, **shared with the CPU** | One full-res RGBA16F write plus read is about 33 MB, about **0.5 ms**. RGBA32F doubles it. |
| Power | CPU and GPU share one package power and thermal budget (Intel Dynamic Tuning moves power between them) | Ableton, the analyzer and the plug-in UI take GPU clocks away. Sustained speed is lower than burst. |

Also worth knowing:
- The "Iris Xe" name itself implies dual-channel memory. Intel only allows the
  brand with 128-bit memory; single-channel parts are sold as "UHD" with
  roughly 25 to 30 % less graphics performance.
- Intel's Xe-LP guide makes three more points that apply here:
  - `textureGrad` (sample_d) runs at **a quarter of the sampler rate**. Prefer
    plain `texture2D` or `textureLod`.
  - Avoid dependent texture reads in chains.
  - Draw full-screen passes as **one triangle**, not a two-triangle quad. The
    engine currently draws a quad strip; this is a small, free engine win.
- fp16 ALU runs at double rate on Xe-LP, but desktop GLSL 1.50 ignores
  `mediump`. Only fp16 **storage** is reachable: RGBA16F targets, which are
  already the default.

### 3.2 Rules of thumb (1080p, internal scale 1.0, estimates from instruction counts, to be measured)

| Building block | Approximate cost per full 1080p frame |
|---|---|
| `vjHash12` | about 15 to 18 lane-instructions |
| `vjNoise` (4 hashes, blend) | about 80 lane-instructions, **about 0.15 to 0.3 ms** |
| `vjFbm` (5 octaves) | about 400 to 450 lane-instructions, **about 0.8 to 1.5 ms** |
| One texture fetch per pixel | about 0.04 to 0.08 ms |
| One full-res RGBA16F buffer pass (write plus read back) | about 0.5 ms of bandwidth, on top of shading |
| The same at 0.5 scale / at 0.25 scale | about 0.12 ms / about 0.03 ms |

The uncomfortable conclusion: **at full 1080p the 3 ms scene budget
(`sceneBudgetMs1080p`) buys only about 2 to 4 fbm calls per pixel.** For
comparison:
- fibers runs about 13 to 16 fbm-equivalents per pixel,
- membrane about 12,
- the canonical Quilez warp 5.

These scenes can only hit 60 fps because the adaptive ladder shrinks the pixel
count.

**Evidence from the engine's own log** (`engine/build/VJEngine_artefacts/Release/VJEngine.log`,
Sept 23 to 26, 5-second FPS samples):
- **Every** one of the 16 presets averages below 50 fps across its samples, in a
  range of about 31 to 52.
- 13 of 16 triggered quality-ladder drops, most down to the 50 % floor.
- Membrane dropped from 85 % to 70 % within two seconds of being selected
  (39 → 34 fps).
- The log also shows the ladder **hunting**: 55 % → 100 % in 5 % steps, then
  straight back down.

The conditions are uncontrolled. The window size is unknown, the runs span dev
sessions, and Live plus the analyzer may or may not have been running. So treat
this as a signal, not a benchmark. The signal is clear anyway: **the library is
not inside its budget on this machine today**, and the 3 ms figure is, as every
preset says, "target, not measured".

### 3.3 Tactics, ranked by payoff

1. **Measure per stage and per pass, on the real machine.**
   - The engine has no GPU timers. Add `GL_TIME_ELAPSED` queries around each
     `ISFShader::runPass`, read them back two frames later so the pipeline
     never stalls, and log the median per preset.
   - Replace "target, not measured" with numbers.
   - Test conditions:
     - a fixed 1920×1080 output,
     - plugged in, with the Windows power mode on best performance,
     - Live running a real set with the analyzer active,
     - measured after 10 minutes, when thermal steady state is reached.
   - RenderDoc can capture only core-profile GL 3.2+ contexts, which may not
     match how JUCE creates ours, so timer queries are the dependable path.

2. **Split every scene into three resolutions.**
   - Tiny state: 0.02 scale.
   - Smooth fields and simulations: 0.25 to 0.5 scale (T10, T5, T6).
   - Edges, lines, grain-scale detail: full resolution, but cheap.

   This is the biggest single win. It turns fibers-class and membrane-class
   costs into T10-class costs. Whole soft ambient scenes can simply declare
   stage `"scale": 0.5` to `0.67`. LookPass grain and dither run at output
   resolution after the upscale and hide most of the softness.

3. **Budget octaves, and band-limit them.**
   - Octaves whose lattice is finer than about 2 pixels cost full price, add
     nothing, and cause shimmer, especially at low render scales and inside
     strong warps.
   - Inigo Quilez, "Band-limiting" (iquilezles.org/articles/bandlimiting): fade
     each octave by its pixel footprint, `noise * smoothstep (1.0, 0.5, w)` with
     `w = fwidth (p.x) * freq`, and **stop the loop** once `w > 1`.
   - The loop count is uniform across the screen, so the `break` does not
     diverge.
   - This is also the right way to give shaders a quality knob.

4. **Replace ALU noise with texture noise where it is only texture.**
   - `IMPORTED` images are already supported **and mipmapped**. A 256² tileable
     RGBA noise PNG next to the shaders gives 4 independent value-noise octaves
     per bilinear fetch, versus about 80 ALU instructions per `vjNoise` call.
   - Use the smoothstep-coordinate trick for C1 interpolation:
     `uv = (i + f*f*(3-2f) - 0.5) / 256`, computed from the texel-space
     position. Use `textureLod` so the modified coordinate does not break mip
     selection.
   - For **evolving** noise, use Quilez's single-fetch 3D noise: the texture's
     green channel equals red shifted by (37, 17) texels, and
     `uv = p.xy + vec2 (37, 17) * floor (p.z)` blends `.r`/`.g` by `fract (p.z)`.
     See Shadertoy "Noise Volume Explanation" (Ms3SRr); used in iq's "Clouds".
   - Caveat: 8-bit precision is fine for noise but bands on large smooth ramps.
     LookPass dither helps.
   - This is an engine-free change: one PNG plus a `common.glsl` helper.

5. **Get gradients for free or nearly free.**
   - `dFdx`/`dFdy` (hot_blobs) cost nothing extra.
   - The directional derivative (T9c) costs one extra evaluation.
   - Analytic noise derivatives (T3) cost about 1.3× one evaluation.
   - membrane.fs currently evaluates `sheet()` three times (3 fbm each) purely
     for a gradient. Switching to screen-space derivatives, or to T10's
     low-res pass, roughly halves or better that scene's cost.

6. **Respect SIMD divergence and register pressure.**
   - Early-outs pay only when they are **spatially coherent**: empty regions
     outside a silhouette, or distant lines rejected by bounding circle (T7).
   - Per-pixel random branches execute both sides.
   - Avoid large local arrays across loops (mesh_body's `pts[9]`/`ids[9]`).
   - Keep loop bounds constant, using the house `break` pattern.

7. **Don't pay twice during transitions.**
   - A crossfade renders both scenes. For heavy scenes prefer `cut` or `dip`
     transitions.
   - An engine idea: drop the render scale for the transition's duration.

8. **Tame the quality ladder.**
   - Every scale change reallocates and **clears all persistent buffers**, which
     resets simulations and trails (see 1.5).
   - It also hunts (drop fast, climb every 6 s).
   - Suggestions, none implemented:
     - allow scale changes only at scene activation or on bar lines,
     - remember the last stable scale per preset,
     - let shaders degrade *content* rather than pixels through an engine
       uniform such as `vj_quality` (0 to 1) that shaders map to octaves,
       layers or segment counts,
     - add an absolute-pixel size option for state passes, so they survive
       rescaling.

9. **Bandwidth hygiene.**
   - RGBA16F by default. `FLOAT` only for Gray-Scott and fluid velocity.
   - Persistent feedback at 0.5 scale.
   - No needless full-res copies.
   - Remember the host already does several full-res passes per frame (stage
     target, finish/bloom, look pass, composite, output, Spout).
   - Morphogen's 16 RGBA32F passes are the heaviest bandwidth user in the
     library (see T6).

10. **Keep the CPU light while the GPU is busy.** On a shared-power laptop,
    CPU spikes from Ableton, the analyzer, or JUCE UI repaints lower GPU
    clocks. Throttle UI repaint rates while performing.

### 3.4 A per-scene budget template (3.0 ms at 1080p)

| Slice | ms | Typical content |
|---|---|---|
| Fields and simulation (low-res passes) | 1.0 | T1/T10 field at 0.25 to 0.5, T5/T6 sim steps, T11 cache refresh |
| Full-res shading | 1.0 | threshold and edges, hairlines (T7 and T8 at modest segment counts), texture-noise detail |
| State and feedback | 0.3 | T4 advection at 0.5, event ring, block state |
| Headroom | 0.7 | crossfades, thermal throttling, the analyzer's CPU spikes |

### 3.5 Engine gaps these techniques bump into (for a future engine pass)

- No mipmaps on pass targets. This blocks LOD blur, the mip Laplacian
  (`9·LOD1 − LOD0`; see Shadertoy-unofficial, "Advanced tricks", 2021),
  multi-scale Turing patterns, and MIPmap statistics such as average
  brightness and centre of mass.
  - The fix: a `"MIPMAP": true` pass flag that calls `glGenerateMipmap` after
    the pass and sets `GL_LINEAR_MIPMAP_LINEAR`.
- No GPU timers (3.3, item 1).
- Pass sizes are fractions only.
  - Real ISF treats `WIDTH` as a pixel expression.
  - `"$WIDTH/4"` would be parsed here as **4.0**, because the parser takes the
    first number literal. That gives a 4× oversized buffer.
  - When porting ISF shaders, rewrite sizes as plain fractions (`"0.25"`).
- No frame-index uniform. Work around it with a state-texel counter (T11).
- Quad instead of single triangle for full-screen passes (3.1).

---

## Sources

- Inigo Quilez, articles:
  - "Domain warping": https://iquilezles.org/articles/warp/
  - "Value noise derivatives": https://iquilezles.org/articles/morenoise/
  - "Voronoi edges": https://iquilezles.org/articles/voronoilines/
  - "Directional derivative": https://iquilezles.org/articles/derivative/
  - "Band-limiting": https://iquilezles.org/articles/bandlimiting/
  - "2D distance functions": https://iquilezles.org/articles/distfunctions2d/
- Xor, "Turbulence" (GM Shaders Mini): https://mini.gmshaders.com/p/turbulence
- Bridson et al., "Curl-Noise for Procedural Fluid Flow" (2007): https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph2007-curlnoise.pdf
- atyuwen, "Fast Divergence-Free Noise Generation in Shaders": https://atyuwen.github.io/posts/bitangent-noise/
- Mark Harris, *GPU Gems* ch. 38 "Fast Fluid Dynamics Simulation on the GPU": https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-38-fast-fluid-dynamics-simulation-gpu
- Guay, Colin and Egli, "Simple and Fast Fluids" (*GPU Pro 2*); nimitz, "Chimera's Breath": https://www.shadertoy.com/view/4tGfDW (ISF port: https://github.com/grigM/ISF-shaders-collection)
- van Wijk, "Image Based Flow Visualization" (SIGGRAPH 2002): https://vanwijk.win.tue.nl/ibfv/
- LucieMrc, "TD_feedback_love": https://github.com/LucieMrc/TD_feedback_love_EN
- Karl Sims, "Reaction-Diffusion Tutorial": https://www.karlsims.com/rd.html
- Robert Munafo, "xmorphia": http://www.mrob.com/pub/comp/xmorphia/index.html
- Ricky Reusser, "Multi-Scale Turing Patterns": https://rreusser.github.io/notebooks/multiscale-turing-patterns/
- BigWings, "The Universe Within": https://www.shadertoy.com/view/lscczl
- "Perfecting anti-aliasing on signed distance functions": https://blog.pkh.me/p/44-perfecting-anti-aliasing-on-signed-distance-functions.html
- Kenny Mitchell, *GPU Gems 3* ch. 13 "Volumetric Light Scattering as a Post-Process": https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-13-volumetric-light-scattering-post-process
- Chris Green, "Improved Alpha-Tested Magnification…" (2007): https://www.realtimerendering.com/advances/s2007/
- Horizon Zero Dawn cloud amortisation, as explained by Vertex Fragment: https://www.vertexfragment.com/ramblings/volumetric-cloud-upsampling/
- Alan Wolfe, "Ray Marching Fog With Blue Noise": https://blog.demofox.org/2020/05/10/ray-marching-fog-with-blue-noise/
- Shadertoy-unofficial, "Advanced tricks": https://shadertoyunofficial.wordpress.com/2021/03/09/advanced-tricks/
- Shadertoy, "Noise Volume Explanation" (the 37/17 trick): https://www.shadertoy.com/view/Ms3SRr
- Keijiro Takahashi, KinoGlitch: https://github.com/keijiro/KinoGlitch
- Codrops, "Breaking the Frame: Real-Time Datamosh": https://tympanus.net/codrops/2026/09/02/breaking-the-frame-building-a-real-time-datamosh-effect-with-three-js/
- ciphrd, "Pixel sorting on shader": https://ciphrd.com/articles/pixel-sorting-on-shader-using-well-crafted-sorting-filters/
- Intel, "Xe-LP API Developer and Optimization Guide": https://www.intel.com/content/www/us/en/developer/articles/guide/lp-api-developer-optimization-guide.html
- Intel oneAPI, "Intel Iris Xe GPU Architecture": https://www.intel.com/content/www/us/en/docs/oneapi/optimization-guide-gpu/2023-0/intel-iris-xe-gpu-architecture.html
- ISF spec, JSON reference: https://docs.isf.video/ref_json.html
- Iris Xe dual-channel branding: https://radu.link/intel-iris-xe-single-channel-ram-downgrades-uhd/
