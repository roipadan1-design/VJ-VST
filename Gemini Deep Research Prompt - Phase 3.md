# Deep Research Request — Phase 3: From Working Prototype to a Pro-Grade Audio-Reactive Visual Instrument for Ableton Live

## 0. Your role and mission

You are a senior research analyst with deep, hands-on expertise in four fields at once:
1. **Real-time graphics engineering** (GPU pipelines, GLSL/HLSL, OpenGL/DirectX/Vulkan, shader-based generative art, post-processing).
2. **Music Information Retrieval and real-time audio DSP** (FFT, onset detection, beat tracking, feature extraction, low-latency ML audio).
3. **Audio plugin and DAW engineering** (JUCE, VST3, Max for Live / Max/MSP / Jitter, Ableton Live internals, MIDI).
4. **Product teardown and competitive analysis** for creative/music software (VJ tools, visual synths, M4L devices).

Your mission is to run a **thorough, exhaustive, source-backed deep-research investigation** and produce a report that a senior developer can use directly to **refactor our existing codebase and specify and build the next version of our product**. We aim very high: we want to reach the quality level of the best products in this category. We will move step by step, but we don't compromise on the destination. Go deep, not wide-and-shallow. When in doubt, include more technical detail, not less.

You do not know anything about us yet. Section 1–3 give you full context. Read them carefully before researching.

---

## 1. Who we are and our fixed constraints

- A solo music producer / performer + an AI coding assistant, building our own audio-reactive visual instrument for live performance.
- **Fixed environment (non-negotiable for now):**
  - **Windows 11 Pro**, NVIDIA-class discrete GPU assumed.
  - **Ableton Live 10 Suite** (NOT Live 11/12) with **Max for Live running on Max 8** (NOT Max 9).
  - We are staying on Live 10 + Max 8. **Every product, device, library and technique you recommend must be checked for compatibility with Live 10 + Max 8 on Windows.** If something needs Live 11/12, Max 9, or macOS-only features (e.g. Syphon only, Metal only, Apple Silicon only), say so explicitly and propose the Windows / Live 10 alternative.
- Our long-term product ambition is a **VST3 plugin and/or Max for Live device** that musicians drop on a track in Ableton. We have *not* finally settled M4L vs VST3 vs a hybrid (see §2 and §5-G). Our owner's current intuition is "we're on Max 8, so VST3 is probably the right vehicle" — **validate or refute this intuition with evidence**. (Note: Max 8 by itself does not prevent M4L devices — e.g. some commercial M4L VJ devices only require Max 7.2.5 — but many *newer* M4L visual devices require Live 12 / Max 9. We need the real picture.)

---

## 2. What we have already built (current state, verified working unless noted)

### Architecture (hybrid "thin control device + separate render engine")
```
Ableton Live 10 ──► M4L devices (analysis + control) ──OSC/UDP :9000──► VJ Engine (standalone C++ app) ──Spout──► Resolume / any Spout receiver
                                                                          └─► own fullscreen window on any monitor
```

### Render engine ("VJ Engine") — standalone Windows app, ~3,000 lines of C++17
- Built with **JUCE** (as a GUI app, *not* a plugin), CMake, MSVC.
- **OpenGL** via JUCE's `OpenGLContext`, legacy-style GLSL (`#version 120`, `gl_FragColor`) for ISF compatibility.
- **Own minimal ISF (Interactive Shader Format) host** written from scratch: supports INPUTS (float/bool/long/point2D/color/image), multi-pass PASSES with named TARGET buffers, PERSISTENT (ping-pong feedback) buffers, PASSINDEX, TIME, RENDERSIZE. Not supported yet: companion `.vs` vertex shaders, expression-based pass sizes (`$WIDTH*0.5`).
- **Preset system**: JSON files. A preset is either a single ISF shader or an **effect chain** (ordered list of ISF stages rendered through FBOs). Each preset has static `params` and `audioMappings` of the form `{param: {source, scale, offset}}` where source ∈ `level | bass | mid | high | beatphase | onset`. Linear mapping only — no curves, no smoothing per mapping, no modulation matrix, no LFOs, no envelopes.
- **Preset transitions**: crossfade between presets (default 600 ms, adjustable, 0 = hard cut; retargets mid-fade).
- **Video input**: MP4 via Windows Media Foundation, webcam input, drag-and-drop a file onto the window.
- **Output**: fullscreen on any monitor, move between monitors, **Spout** texture sharing (SpoutLibrary.dll), `/debug/snapshot` writes the current frame to PNG (used for automated visual regression tests).
- Multiple instances possible on separate OSC ports.
- Content today: 9 presets, mostly test shaders (a star, noise) and **video-dependent glitch chains** (RGB shift, block glitch, scanline jitter, a real persistent-buffer datamosh, chroma smear, edge etch, kaleidoscope, pixelate, signal-drop). **No polished generative (no-video) presets yet.**

### Max for Live side (Max 8, Live 10)
- **VJ Audio Analyzer** (audio effect device): `plugin~` → one-pole filters to split into **3 bands (bass/mid/high)** → `abs~`/`average~` envelope → `snapshot~` at control rate → OSC via `udpsend`. Also: overall level, **beat phase from Live's transport** (`live.object` on song time — this is very accurate), and a **primitive onset detector** (bass envelope crossing a fixed 0.3 threshold). Preset select is a real `live.numbox` parameter → automatable and MIDI-mappable via Live's MIDI Map mode. Scene-name → preset linking exists but is unverified.
- **VJ Effect Controls** (4 generic knobs → `/effect/param` for live parameter control) — unverified in practice.
- **VJ Control Center** (launch/show engine window, preset nav) and **VJ Signal Portrait** (one-click preset + video load).
- No direct MIDI input in the engine itself; all MIDI goes through Live's MIDI mapping of M4L parameters.

### OSC contract (engine listens on UDP 9000)
`/audio/level|bass|mid|high|beatphase` (float 0–1), `/audio/onset` (bang), `/preset/select|next|previous`, `/preset/transitionduration`, `/effect/toggle`, `/effect/param <stage> <name> <value>`, `/display/select|next|previous`, `/fullscreen`, `/camera/open`, `/video/load`, `/debug/snapshot`.

### Known weaknesses we already see
- Audio analysis is basic: 3 crude bands, no adaptive normalization (quiet vs loud material behaves very differently), no proper onset detection, no per-band transients, no spectral features.
- Mapping is linear and flat — visuals "wobble with volume" instead of feeling musical.
- Visual quality is prototype-level: no bloom/tone-mapping/HDR/float pipeline, no particles, no raymarching, no curated color system.
- MIDI control is indirect and thin.

---

## 3. What we already know (prior research — do NOT redo, only update/correct if you find newer or contradicting evidence)

A previous deep-research round concluded:
- **Rendering GPU graphics inside a VST3 plugin editor window is fragile** (JUCE forum evidence 2019–2026: multi-instance slowdowns, mixed-DPI corruption, secondary-monitor freezes in Live). Recommended pattern: **thin plugin (analysis + parameters) + separate render process/window**, as shipped by Spettro VST (renderer keeps running with editor closed).
- **Spout (Windows) / Syphon (macOS) / NDI (network)** are the standard texture-sharing trio.
- **ISF** is the right shader interchange format (VVISF-GL, ISF4AE, px-stream, ShadertoyVST are reference implementations).
- **Don't build projection mapping** into the product; emit a clean texture and let MadMapper/Resolume do it.
- Market landscape already profiled at a high level: Resolume, VDMX, TouchDesigner, Synesthesia, Magic Music Visuals, VPT 8, ossia score, Chataigne, Vuo, Millumin, Modul8, Smode, HeavyM, GrandVJ, CoGe, Spettro, px-stream, T3X2R, Synesthetic Devices (Geometrum etc.), ShadertoyVST, VS by Imaginando, Arkestra, EboSuite.
- Claims we'd like you to **re-verify specifically for Live 10 + Max 8 on Windows**: (a) "Jitter display (`jit.window`) is restricted when Max is authorized only through Live" — is this true, for which versions, and does it matter if rendering happens in an external process? (b) Is Max 8's `gl3` / GL3 engine usable on Windows inside M4L in Live 10? (c) Live 10's VST3 support details and limitations (MIDI input to audio-effect plugins, parameter automation, state saving, sidechain).

---

## 4. The new direction (what we're building next)

We are refocusing on a **small set of simple, lightweight, but beautiful presets** as the core of the next version, with:
1. **Fast switching between presets** (with musical transitions).
2. **Control from a hardware MIDI controller** (knobs/faders/pads → macros, preset select, triggers).
3. **Triggers driven by the sound itself** (kick → flash/cut, snare → color change, build-up → intensity, etc.), and continuous motion driven by audio features.
4. Visual quality comparable to the reference products below. Starting simple, aiming high.

---

## 5. Research tasks

### A. Deep teardown of our six reference products (highest priority)

For **each** of the following, research everything you can find: official site, manuals/docs, changelogs/release notes, demo and tutorial videos (YouTube/Vimeo/Instagram — describe what you see), forum threads, reviews (CDM, MusicRadar, Sound On Sound, KVR, Gearspace, Reddit r/ableton, r/vjing, Cycling '74 forums, maxforlive.com), developer interviews/talks, GitHub if any.

1. **Showsync** — https://www.showsync.com/ (Videosync, Beam for Live, Sync Tools)
2. **Zwobot** — https://www.zwobotmax.com/
3. **VS – Visual Synthesizer (Imaginando)** — https://www.imaginando.pt/products/vs-visual-synthesizer
4. **Arkestra** — https://www.arkestra.app/articles/vj-software-ableton-mac (the product itself, not just the article)
5. **Photism** — https://photism.app/
6. **T3X2R** — https://www.t3x2r.com/

For each, fill this template (be concrete; say "unknown" rather than guess, and label inferences as inferences):
- **What it is / positioning / target user.** Price and licensing model.
- **Format & architecture:** M4L / VST3 / AU / standalone / hybrid. Where does rendering happen (inside Live's process? inside the plugin editor? a separate window/process?). What graphics tech (Jitter GL2/GL3, OpenGL, Metal, DirectX, Unity/Unreal, web/WebGL)? What evidence supports this?
- **Compatibility:** minimum Live and Max versions, **does it run on Live 10 + Max 8 on Windows?**, OS, GPU requirements.
- **Audio analysis:** what features does it extract (bands, number of bands, envelopes, peaks, onsets, beat/BPM, spectral features)? Configurable? Per-track or master? Sidechain?
- **Modulation / mapping system:** how do audio features, LFOs, envelopes, MIDI, and Live automation reach visual parameters? Modulation matrix? Curves, smoothing, ranges? Macros?
- **MIDI:** MIDI learn, notes/velocity as triggers, CC, clock, polyphony, controller templates, LED feedback.
- **Content model:** scenes / presets / layers / clips / effect chains. How presets are stored, browsed, switched, and transitioned. Session View / scene / clip integration with Live.
- **Visual look & techniques:** describe the aesthetic in detail (style, palette, motion, density, typical frame). Then **infer the rendering techniques** that produce it (e.g. SDF raymarching, domain-warped noise, feedback buffers, particles, fluid sim, fractals, kaleidoscopic symmetry, bloom, chromatic aberration, film grain, tone mapping). This is critical: we want to know *how to reproduce that level of look in code*.
- **UI/UX:** what the device/plugin UI looks like, how many controls are exposed, how preview works, how fullscreen/output is handled.
- **Output:** fullscreen, second monitor, Spout/Syphon/NDI, recording.
- **Performance & stability:** reported FPS, known bugs, user complaints, CPU/GPU impact on Live.
- **What we should copy, what we should do better, what we should avoid.**

Then produce a **comparison matrix** across the six (rows = features above, columns = products), plus a short "best-in-class for each capability" list.

### B. Extended landscape — only what's relevant to "simple presets + audio triggers + MIDI"
Briefly profile anything not covered in §3 that is directly relevant, especially **preset-based audio-reactive systems**:
- **MilkDrop / projectM / Butterchurn / NestDrop** — the canonical preset-driven audio-reactive system; explain its preset language (per-frame / per-pixel equations, warp & composite shaders, bass/mid/treb + `_att` smoothed variants, beat detection) and what we can learn from its model for our preset format.
- **Synesthesia** (its SSF scene format, audio uniforms like `syn_BassLevel`, `syn_BassHits`, `syn_BPMTwitcher`, etc.) — document its audio uniform set in detail; it is a strong reference for what a mature audio→shader interface looks like.
- **Resolume's audio FFT / envelope parameter animation, TouchDesigner's Audio Analysis component, Magic Music Visuals, Kaleidoscope-type apps, Plane9, Spettro, px-stream** — only the parts relevant to analysis, mapping, and preset design.
- Any new (2025–2026) products or open-source projects in this niche we're missing, especially **Windows-compatible** and **Live 10 / Max 8 compatible** ones.

### C. State of the art in audio-reactive analysis ("is it just frequencies?")
Give a rigorous, engineering-level survey — with algorithm details, typical parameters, latency, CPU cost, and recommended libraries (**with licenses**; we need options compatible with a closed-source commercial product, flag GPL):
1. **Spectral analysis setup:** FFT size/hop/window for visuals at 60 fps, latency trade-offs, log-frequency / mel / Bark / 1/3-octave banding, how many bands pro tools use and why.
2. **Envelope following:** separate attack/release, peak vs RMS, perceptual loudness (LUFS/momentary), dB scaling.
3. **Adaptive normalization / AGC:** how to make visuals react equally well to quiet ambient passages and loud drops (running min/max, percentile tracking, slow-release peak normalization). This is one of our biggest problems — go deep.
4. **Onset / transient detection:** spectral flux, SuperFlux, high-frequency content, complex-domain; adaptive thresholding / peak picking; **per-band onsets** (kick vs snare vs hat); how to avoid double triggers; latency figures.
5. **Beat tracking & tempo:** BTrack, aubio, madmom, Essentia, etc. — and **why/when we don't need them** because we have Live's transport (beat phase, bar, tempo) directly in M4L. How the best products combine transport clock + audio onsets.
6. **Timbral/spectral descriptors** useful for visuals: centroid (brightness), flatness (noise vs tone), rolloff, spread, flux, zero-crossing, chroma/pitch class, MFCC. Which ones actually produce good-looking mappings and to what visual parameters.
7. **Musical-structure awareness:** build-up / drop / breakdown detection (energy trend, novelty curves), section changes → preset changes.
8. **Cutting edge / ML (2024–2026):** real-time source separation (e.g. Demucs variants, HS-TasNet, other low-latency models) — actual latency and GPU cost on Windows; neural onset/beat models; audio-driven real-time image generation (StreamDiffusion and successors, audio-conditioned diffusion, TouchDesigner integrations). Give an honest verdict: what is production-usable live today vs. demo-ware.
9. **The Ableton advantage:** in a DAW we have separated tracks ("stems for free") and MIDI clip data. Research how to exploit this: per-track analyzers, reading MIDI notes from clips as zero-latency triggers (in Live 10 / M4L and in a VST3), Live API data (clip launch, scene launch, track names, macro values). Compare the precision of MIDI-note triggering vs audio onset detection.

### D. Mapping / modulation system design (how to make visuals feel *musical*)
- How the best products structure modulation: sources (audio features, onsets, LFOs synced to Live tempo, envelopes/ADSR triggered by onsets or MIDI notes, random/S&H, MIDI CC, Live automation) → **modulation matrix** → destinations, with per-route amount, curve (exp/log/S-curve), smoothing, range, polarity, and quantization to beat.
- **Continuous vs trigger mappings**: when to use each; "decay envelopes on hits" vs raw levels.
- **Macros**: one knob → many parameters with different curves (like Ableton Rack macros). How many macros per preset is ideal for live play (4? 8?).
- **Preset morphing and transitions** synced to bar/beat; transition types beyond crossfade (feedback-dissolve, luma-key, glitch cut, zoom).
- Anti-patterns: what makes audio-reactive visuals look cheap/jittery (unsmoothed jitter, everything reacting to everything, no hierarchy between rhythm and texture).
- **Propose a concrete data model / JSON schema** for a preset in our next version (shaders/stages, parameters with ranges and defaults, macros, modulation routes, triggers, transition settings), informed by MilkDrop, Synesthesia SSF, VS's mod matrix, and ISF.

### E. MIDI control design
- MIDI Learn UX patterns in the references and in best-in-class plugins; CC vs NRPN vs 14-bit; relative/endless encoders; soft takeover / pickup mode; LED/value feedback to controllers.
- Popular controllers for this use (e.g. Akai APC40/APC mini, Novation Launchpad/Launch Control XL, MIDI Fighter Twister, Push) and recommended default mapping layouts for a visual instrument (preset grid on pads, macros on knobs, intensity/blackout/strobe on faders/buttons).
- **Routing reality in Live 10**: how MIDI reaches (a) an M4L audio effect, (b) a VST3 audio effect, (c) a VST3 instrument / MIDI effect, (d) an external standalone app (loopMIDI, virtual ports). What Live 10 allows and blocks. Which vehicle gives the smoothest "hardware knob → visual" path with lowest latency and least setup.

### F. Visual quality: how to reach "premium" look in code
For each technique, explain what it contributes aesthetically, how it's implemented (with algorithm outline / key GLSL snippets where useful), its GPU cost at 1080p/4K 60 fps, and which reference products appear to use it:
- Signed distance fields & raymarching (2D and 3D), domain repetition, smooth min.
- Noise: value/gradient/simplex, fBm, **domain warping** (Inigo Quilez style), curl noise.
- Feedback loops and persistent buffers (zoom/rotate feedback, trails, video-feedback aesthetics), reaction–diffusion, cellular automata.
- GPU particle systems (compute shaders or transform feedback / texture-based state), flow fields, fluid simulation (stable fluids on GPU).
- Kaleidoscopic / polar symmetry, fractals (Mandelbox/Julia/IFS), tunnels.
- **Post-processing stack**: HDR float render targets, bloom (dual-filter / Kawase), tone mapping (ACES/AgX), color grading LUTs, film grain, vignette, chromatic aberration, lens distortion, motion blur, temporal accumulation/anti-aliasing.
- **Color systems**: cosine palettes (IQ), curated palette sets, OKLab/OKLCH interpolation, palette changes triggered by music.
- **Proposal: 8–12 concrete starter presets** for our next version that are simple to implement but look professional. For each: name, visual description, technique(s), 4–8 exposed parameters, which 4 macros, audio mappings (continuous + triggers, with suggested smoothing/curves), and MIDI mapping. Order them from easiest to most ambitious.

### G. Architecture decision for the next version (Live 10 + Max 8 + Windows)
Evaluate with evidence, then **recommend one path with a migration plan from what we have**:
1. **Pure M4L + Jitter (Max 8)** — rendering inside Max. Performance ceiling, GL2 vs GL3 engine on Windows, Spout for Max, the authorization issue, scheduler/timing, multi-device behavior.
2. **VST3 plugin with rendering inside the plugin** (JUCE + OpenGL/DirectX in editor or in a separate top-level window owned by the plugin) — risks from §3, how VS by Imaginando and Photism handle it (they ship VST3 with in-plugin rendering and fullscreen — how do they do it robustly?).
3. **Hybrid (our current architecture):** thin VST3 and/or M4L device for analysis + parameters + MIDI + state saving, talking to a separate render process. Research IPC options (OSC/UDP, shared memory ring buffers, named pipes, Spout's own data channel), how the plugin launches/finds the engine, how Live-set state (preset, macros) is saved/recalled, multi-instance handling, latency budget end-to-end (audio → analysis → IPC → render → display).
4. **Graphics API for the engine going forward:** stay on OpenGL (move from GLSL 1.20 to 3.3/4.x core?) vs DirectX 11/12 vs Vulkan vs abstraction layers (bgfx, Diligent Engine, sokol_gfx, WebGPU via Dawn/wgpu). Consider: Spout's native DX11 path, ISF/GLSL shader compatibility (SPIR-V cross-compilation via glslang/SPIRV-Cross), compute shader availability, JUCE integration, developer velocity. Give a clear recommendation.
5. Where should audio analysis live — in M4L (Max 8 DSP), in a VST3 (C++), or in the engine (receive raw audio via loopback/WASAPI/shared memory)? Consider latency, per-track analysis, and CPU load on Live.

### H. Engineering practices & open-source code to study
- Real-time-safe plugin design in JUCE: lock-free FIFOs between audio thread and other threads, parameter smoothing, `AudioProcessorValueTreeState`, state save/restore, host automation in Live 10.
- Render loop design: fixed vs variable timestep, frame pacing, vsync on multi-monitor, GPU timing queries, shader hot-reload, graceful shader compile failure.
- Testing: visual regression (golden images), soak tests, performance budgets.
- A curated list of **open-source repos worth studying** for each subsystem (audio analysis, onset detection, mod matrix, ISF hosting, GPU particles/fluids, post-processing, JUCE+GL plugins, Spout integration), with URL, license, last activity date, and exactly what to learn from each.

### I. Product & UX
- The minimum lovable v1 feature set for "few simple presets + MIDI + audio triggers."
- Preset browser/thumbnails, device UI layout in Live (how references fit a useful UI into Live's device strip height), onboarding, first-run experience, fail-safe behavior on stage (what happens if the engine crashes, GPU stalls, monitor disconnects).

---

## 6. Research standards (please follow strictly)

- **Cite everything** with URLs and publication/access dates. Prefer primary sources: official docs, manuals, changelogs, source code, developer posts, conference talks. Use forums/Reddit for real-world pain points and label them as such.
- **Clearly separate verified facts, vendor claims, and your own inferences.** Use explicit labels ([Verified], [Vendor claim], [Inference]).
- **Recency matters**: prioritize 2024–2026 information; note when something may be outdated.
- **Compatibility check is mandatory**: for every product, library, and technique, state Live 10 / Max 8 / Windows compatibility.
- **Licensing check is mandatory** for every library you recommend (MIT/BSD/Apache vs GPL/LGPL vs commercial).
- Be quantitative wherever possible (latency in ms, FPS, GPU cost, CPU %, band counts, FFT sizes, prices).
- When you describe a visual look from videos, describe concrete observable features (motion type, palette, symmetry, texture, what reacts to what) — not marketing adjectives.
- Don't pad. Don't repeat §3 except for corrections/updates. If you can't find something, say so and suggest how we could find out.

---

## 7. Output format

1. **Executive summary in Hebrew** (1 page max): the key findings and your top recommendations.
2. **Full report in English**, following sections A–I above, with tables where useful.
3. **Reference comparison matrix** (§5-A) as a table.
4. **"Engineering spec for the next version"** — the most important deliverable. Concrete enough that a developer can start coding:
   - Recommended architecture (diagram in text/ASCII), components and responsibilities, IPC protocol, threading model.
   - Audio analysis module spec: feature list, algorithms, parameters (FFT size, hop, band edges, attack/release times, normalization method, onset thresholds), output format and rate.
   - Modulation system spec and **preset JSON schema** (with one fully worked example preset).
   - MIDI control spec (default mapping for one recommended controller + MIDI learn behavior).
   - Rendering pipeline spec (render targets, formats, post-processing chain, color management).
   - The 8–12 starter presets from §5-F.
   - **Phased roadmap** (small steps, each with clear acceptance criteria that can be tested), from our current state to the target.
   - Top risks and how to de-risk each one.
5. **Open questions** we need to answer ourselves (things only testing on our machine can settle).
6. **Bibliography** grouped by topic.

Take your time and be exhaustive. Depth and technical precision matter more than brevity.
