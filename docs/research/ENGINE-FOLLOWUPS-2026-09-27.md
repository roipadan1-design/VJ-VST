# Engine follow-ups, Sept 27 2026

Follow-ups from `SHADER-TECHNIQUES.md` (section 3.3, item 1) and `AMBIENT-STYLE-DIRECTION.md` (section 5, item 1). None of this has been checked in Ableton Live yet.

## 1. REST recovery in BREATHE is now gentle

`engine/Source/Reaction.cpp`, `styles[]`, BREATHE row: only `restOutSeconds` changed.

| Character | restInSeconds (into rest) | restOutSeconds before | restOutSeconds after |
|---|---|---|---|
| Breathe | 2.0 | **0.08** | **1.5** |
| Pulse | 1.2 | 0.06 | 0.06 (unchanged) |
| Punch | 0.6 | 0.03 | 0.03 (unchanged) |

**What it does.** `rest` follows its target through `follow()`, a one-pole smoother where the value is the time constant (tau). Exposure is `1 - rest * depth * (1 - restFloor)`, with BREATHE's floor at 0.22. Before the change, the image came back from the rest floor to full brightness in about 80 ms, which reads as a cut. Now:
- it gets 50 % of the way back in about 1.0 s (roughly 2 beats at 120 BPM),
- 63 % in 1.5 s,
- 95 % in about 4.5 s.

**Why 1.5 s.**
- It sits in the middle of the requested 1.2-1.8 s range.
- It is slightly quicker than the 2.0 s way in, so music coming back is still acknowledged within a bar rather than seeming ignored.
- The ambient research suggested 2-4 s. That would feel slow for BREATHE, which is also used for non-ambient material. A dedicated "ambient" character or a per-preset reaction hint is the better place for 2-4 s.

**Side effects, all bounded and small.** `rest > 0.5` now lasts about 1.0 s after sound returns, instead of about 55 ms. The logic that reads `rest > 0.5` therefore sees rest for about a second longer:
- The first candidate hit after rest is a forced accent. BREATHE's 2-beat minimum gap allows at most about one extra forced accent in that second, and its accent punch is small (0.15 stops).
- Tension arming waits about 1 s longer.
- `lastRestTime`, the "primed" window for drop detection, is extended by about 1 s.
- Scar healing at the fast rate lasts about 1 s longer.

A drop still sets `rest = 0` immediately, so drops land as hard as before. The `react.rest` modulation source now ramps out gently in BREATHE too, which is intended.

**Why Pulse and Punch were left alone.** They drive kinetic, hit-led scenes. In those scenes, snapping awake on the first beat after a break is part of the effect (a break followed by a slam).

## 2. Optional GPU timing (`VJ_GPUTIMING`)

This is instrumentation only. It never changes what gets rendered.

**How it works.**
- Set `VJ_GPUTIMING=1` (any value except empty or `0`) before starting `VJ Engine.exe`.
- The engine brackets GPU work with `glQueryCounter (GL_TIMESTAMP)` queries. Timestamps rather than `GL_TIME_ELAPSED` because timestamp queries can nest, and `GL_TIME_ELAPSED` cannot.
- It polls `GL_QUERY_RESULT_AVAILABLE` and reads results back only once the GPU has finished them. It never waits: frames still unresolved after 8 newer ones are dropped.
- Every 5 s it appends median / p90 / max ms per section to `VJEngine.log` as `GPU timing:` lines.
- A new window starts whenever the preset, output size or render scale changes, so each block of lines covers one configuration. The first block after a switch can include one leftover frame of the previous scene (`n=1`) and any crossfade.
- When the variable is unset, `gpuTiming::enabled()` returns false once it has been cached, and no GL call is made.
- If the driver lacks timer queries, it logs one line and stays off.

**Sections measured.**
- `frame total (preset render)`: all of `PresetManager::render`.
- `scene total (stages + finish)`: the current preset's stages plus its finish pass.
- `<shader>.fs pass N [target xScale] @WxH`: every ISF pass, measured in `ISFShader::render`. This also covers legacy `EffectChain` stages.
- `finish (scene post)`, `look pass (output)`, `outgoing scene (transition)`.

**Files.**
- `engine/Source/GLHelpers.h/.cpp`: the `gpuTiming` namespace.
- One-line hooks in:
  - `ISFShader.cpp` (the per-pass loop)
  - `PresetInstance.cpp` (finish)
  - `PresetManager.cpp` (scene, outgoing and look)
  - `MainComponent.cpp` (frame total and `endFrame`)
- Two call sites were split into `const bool x = call(); end(); if (x) ...`. Their behaviour is identical.
- There are no new source files and no CMake changes.

## 3. Build

`Build and Install.bat`'s three CMake builds (Release) were run by hand, without the install step. All three succeed:
- analysis: `analysis_tests` all passed
- engine
- plugin: VST3 and PluginHarness

No build breakage was found in today's other changes, so nothing else was touched. The only new compiler warning is MSVC's `getenv` C4996, the same one the existing `VJ_FRAMEDUMP` code already produces.

## 4. Measurements (real, Iris Xe)

**Setup:**
- Scratch copy of the Release engine, run with `--demo` (built-in groove, no Live, no analyzer).
- `/fullscreen 1` at 1920×1080.
- `/v2/quality 1.0`, which pins render scale at 100 % with no adaptive ladder.
- About 16 s per scene. Steady-state 5 s windows only, with transitions excluded.
- Uncontrolled conditions: laptop power state unknown, one run.

Values are GPU ms per frame, median (p90 in brackets).

| Scene | Scene passes | Finish | Look pass | Frame total | FPS log |
|---|---|---|---|---|---|
| 17 Lumia | lumia.fs 7.9-8.3 (8.7) | 1.1 | 3.0-3.1 | 12.8-13.3 | 58-59 |
| 18 Liquid Light | pass 0 (liquid, x0.5) 1.72 (1.9) + pass 1 3.17 (3.5) | 1.3 | 4.3-4.4 | 11.3-11.4 | 56-60 |
| 16 Membrane | membrane.fs 15.0-15.8 (17.1) | 1.1 | 3.1 | 20.2-20.8 | 39-47 |
| 05 Fibers | fibers.fs 14.9 (15.8) | 1.1 | 3.2-3.4 | 20.1-20.2 | 42-43 |
| 08 Signal Fog (reference) | signal_fog.fs 5.2 (5.7) | 1.3 | 4.2-4.3 | 11.6-11.7 | 60 |

During the 5 s crossfade into Liquid Light, the frame total was 16.9 ms: the outgoing Lumia cost 9.1 ms on top of the new scene.

**What the numbers say:**
- **No scene is inside the 3 ms `sceneBudgetMs1080p` at full 1080p.**
  - Liquid Light is closest: 4.9 ms of passes, thanks to its half-resolution state pass.
  - Lumia is about 8 ms.
  - Membrane and Fibers are about 15 ms each. On their own they rule out 60 fps at 1080p, which is why the adaptive ladder drops them.
- **The fixed per-frame overhead is large.**
  - Finish costs about 1.1-1.3 ms and the look pass about 3-4.4 ms, so about 4-5.5 ms before any scene runs.
  - Look pass cost varies with the scene (2.9-4.4 ms), probably because of content-dependent halation and bloom.
  - This is the next thing to profile. Breaking the look pass into sub-sections is a one-line addition per sub-pass in `LookPass.cpp`.
- The analysis doc estimated Membrane and Fibers as the heaviest scenes, and the measurements confirm it: about 5× the budget.

Re-measure under the doc's test conditions (Live and the analyzer running, plugged in, best-performance power mode, after 10 minutes) before writing any of these into presets.
