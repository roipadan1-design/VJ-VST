# VJ VST - Usage (v2)

Technical reference, written from the code. Hebrew quick start:
[docs/QUICKSTART-HE.md](docs/QUICKSTART-HE.md). Target: Windows 11, Ableton Live 10.1.43,
Max 8 (only for the legacy M4L devices).

## The pieces

```
Ableton Live 10                                   VJ Engine (separate process)
  VJ Analyzer VST3 (one per track you want)  ---OSC v2 (UDP 9000)-->  FeatureBus -> Clock -> Modulation
    audio analysis (AnalysisCore)                                     -> preset stages (ISF) -> Finish (bloom, tone map)
    played MIDI notes, host transport                                 -> transition / blackout -> window + Spout
    8 macros, preset, blackout, HIT     <--/v2/status, /v2/presets--  (current preset, fps, bpm, quality)
  legacy M4L devices (still work: /audio/*, /preset/*, /effect/*)
```

| Folder | What |
|---|---|
| `analysis/` | AnalysisCore (C++17, no deps): FFT, adaptive normaliser, onsets, descriptors, OSC v2 encoder. `analysis_tests`, `analyze_wav`. |
| `engine/` | VJ Engine (JUCE app, OpenGL): presets, modulation, render graph, output. |
| `plugin/` | VJ Analyzer VST3 + `plugin_harness` (runs the plug-in with a synthetic groove, no DAW). |
| `m4l-device/` | Legacy Max for Live devices (v1 protocol, unchanged). |
| `visual_instrument_research/` | Research package (report, engineering spec, preset schema). |
| `docs/` | Reference analyses and guides. |

## Build / install

Double-click **`Build and Install.bat`**: builds AnalysisCore (and runs its 26 tests), the engine and the
plug-in in Release, and copies `VJ Analyzer.vst3` to `C:\Program Files\Common Files\VST3`.
In Live: *Preferences > Plug-ins > Use VST3 Plug-in System Folders: On*, then *Rescan*.

**`Start VJ Session.bat`** launches the engine (Release build preferred) and Live.

## VJ Analyzer (VST3)

Drop it on any audio track (Audio Effects > Plug-ins > VJVST > VJ Analyzer). Audio passes through untouched.

- **Role** (MIX / KICK / SNARE / HAT / BASS / TEXTURE). Put one on the master (or a drum bus) as **MIX**. For
  tighter hits add more instances on the kick / snare / hat tracks with those roles: the engine then uses them
  instead of guessing drums from the full mix. With no role sources, the mix's bass/mid/high transients stand in.
- **MIDI**: route a MIDI track's output to the analyzer's track (*MIDI To*). Played notes arrive as zero-guess
  events; on a KICK-role analyzer a note *is* a kick (and suppresses audio-detected kicks for 2 s).
- **Controls** - four groups, one rule each (every knob is a host parameter: automate it or MIDI-map it with
  Live's MIDI Map mode; if one doesn't show up for mapping, click *Configure* on the device and touch it). Only
  enable **Send macros** on one analyzer (normally the MIX one). The line under each knob says what it does in the
  current scene (sent by the engine), or shows a live readout.
  - **SHAPE** ("visible in every scene, always the same direction"): macros Intensity (1), Form (3), Scale (4,
    always bigger as it rises), Erode (6, pristine -> worn / torn / dissolved), Detail (8). Each scene names them.
  - **MOVE** ("0 = still"): Speed (macro 2: 0 frozen, 0.5 designed speed, 1 = x4), Glide (macro 7: inertia of
    speed changes, 0-4 bars), Drift (slow camera over the whole picture), Push (music leans on the speed, up to
    x2 - never moves a frozen scene), FREEZE, REVERSE, SYNC (speeds follow tempo; designed speed at 120 BPM).
    One engine scene clock drives every scene's motion, so these act everywhere.
  - **REACT** ("hits never touch speed"): Impact (macro 5, scales every hit reaction; 0 = none), Softness
    (hit decays x0.5-x4), Reactivity, CALM, VISUALS REACT TO.
  - **LOOK**: FILM and DIGITAL rows (below), palette, snapshots, MEDIA.
  Macro IDs `macro1`..`macro8` are unchanged from earlier builds, so old automation still points at the same slots.
- **Preset**: 0 = leave the engine's choice; the list on the right selects directly and records the choice in
  the parameter, so the Live set recalls it.
- **VISUALS REACT TO** (KICK / SNARE / HAT / BASS / LEVEL): global gate on what drives the picture. A closed
  channel's hits are dropped and its continuous signals fade to neutral (LEVEL = whole-mix loudness, brightness
  and build-ups). HIT, MIDI notes and the tempo clock always pass. The role buttons at the top say what is on
  *this* track; REACT TO says what the *visuals* follow. Macros, LOOK and REACT TO are sent only by the instance
  with **Send macros** on - on other instances they are shown dimmed.
- **FILM** (in the LOOK panel): a real 35mm chain - band-limited grain that follows luminance and steps at
  24 fps (added *before* Crush, so it dithers the threshold), Halation (red glow out of highlights), Weave
  (gate weave, rare frame slips, flicker, soft fringing), Dust (specks, hairs, scratches - rare and
  correlated), Blacks (lifted, tinted film base); output is blue-noise dithered against banding.
- **Reactivity** scales every audio-driven movement (1 = as designed, 0 = only knobs and LFOs); **CALM** fades
  all reactions out over one bar and back in. Flash, kick-cuts and SHOTS are each capped at 3 per second.
- **LOOK** (global, on top of every scene): Grain, Crush (soft -> posterised -> hard two-tone), Flash (chance a kick
  drops a black / colour frame), Glitch (row tears, pixel blocks, RGB split), Trails, Symbols (braille strips,
  re-dealt on snares), Cut Rate (automatic cuts between scenes: off, every 16/8/4/2/1 beats, every kick),
  **SHOTS** (cut grammar after the un_source reel: selected strong hits become short cuts - a new framing held
  until the next shot, 3-5 frames of negative, a circle / crescent / lens / half-disc punch showing the light
  colour or an inverted close-up, a black frame) and **HUD** (crosshairs, squares linked by hairlines, faint
  circles, re-dealt on every shot). Digital disturbances default to 0 (ambient).
  **Palette** maps the image through 3 colours (shadow / mid / light): pick a preset (Blood, Ember, Bone, Ice,
  Acid, Violet, Rust, the film families Nitrate, Cyanotype, Tungsten, Ash, and *Split* - black / red / cyan, made
  for the Negative scene; it is last in the list), click a colour chip to edit it (that makes it *Custom*), or
  *Scene Colors* to switch the mapping off. Two-layer scenes (Negative) are coloured per layer instead: body =
  mid colour, detail = light colour, over the shadow colour.
- **SNAPSHOTS** A-D: light STORE, then click a slot to save the 8 macros, every LOOK knob and the palette.
  Clicking a slot recalls it, morphing over the chosen time (Cut / 1 beat / 1 bar / 4 bars / 16 bars at the
  host tempo). Snapshots are saved with the Live set; the morph time is an automatable parameter.
- **HIT** fires a manual hit (event `userTrigger`); **BLACKOUT** fades the output to black. Momentary host
  parameters for MIDI buttons: Hit, Snapshot A-D, Previous Scene, Next Scene (fire on the rising edge, reset
  themselves). Toggles: Freeze, Reverse, Sync, Use Media, Blackout, Calm.
- **MEDIA**: LOAD IMAGE (PNG / JPEG) into media slot 1, USE MEDIA (host parameter), CLEAR. The path is saved with
  the Live set and re-sent to the engine on connect.
- **Lookahead** (0-150 ms, default Off): delays the audio the plug-in passes and reports it as latency, so Live's
  delay compensation delays everything else too while the analysis reads the undelayed input - the visuals see
  the music early. For playback / DJ sets only (live instruments through Live would be late as well).
- **Hit Sens** (0.25-4) scales onset thresholds; **Trim** adjusts analysis input only; **Adaptive / Locked**
  selects the normaliser mode.
- Header pill: engine connection, engine fps, tempo (host / free), internal render scale.

## VJ Engine

| Key | Action |
|---|---|
| `F` / `F11` | Fullscreen on the current monitor |
| `[` / `]` | Move to previous / next monitor |
| `Left` / `Right` | Previous / next preset (schema-2 presets may wait for the next bar) |
| `Space` | Manual hit |
| `B` | Blackout toggle |
| `D` | Demo groove on/off (drives visuals when no analysis source is live) |
| `Q` | Quality: Auto / 100% / 75% / 50% render scale |
| `1`-`9` | Toggle effect stages |
| `C` | Webcam as video input; drop a video file onto the window for video presets |

Command line: `"VJ Engine.exe" [--osc-port N] [--demo] [video file]`.

**Adaptive quality**: the engine renders internally at a fraction of the output resolution and upscales,
lowering the scale when it falls under 50 fps and climbing back after 6 stable seconds (logged to
`VJEngine.log`). Measured on this machine's Intel Iris Xe: Hot Blobs holds ~42 fps at 76 % (with the look pass).

`VJEngine.log` (next to the exe) records preset loads/rejections, shader errors, fps with the live sources and
clock, and quality changes.

## OSC

Engine listens on UDP 9000 (loopback). All of the following are accepted:

| Address | Args | |
|---|---|---|
| `/v2/hello` | i sourceId, s role, s name, i version, [i replyPort] | analysis source announcement; replyPort subscribes to status |
| `/v2/frame` | see `analysis/include/vj/Protocol.h` | latest-wins analysis snapshot (~120 Hz) |
| `/v2/spectrum` | i sourceId, i seq, 32 f | log spectrum (~60 Hz) |
| `/v2/event` | i sourceId, s role, i eventId, s type, f strength, i note, i velocity, f ageMs | hits and notes (de-duplicated, expire after 150 ms) |
| `/v2/macro` | i slot 0-7, f 0-1 | macro base value |
| `/v2/look` | i slot 0-15, f 0-1 | global look: 0 grain, 1 crush, 2 flash, 3 glitch, 4 trails, 5 symbols, 6 cut rate, 7 smear, 8 halation, 9 weave, 10 dust, 11 blacks, 12 reactivity, 13 calm (0/1), 14 shots, 15 hud |
| `/v2/move` | i slot 0-5, f 0-1 | MOVE / REACT globals: 0 drift, 1 push, 2 softness, 3 sync, 4 reverse, 5 freeze (Speed / Glide are macro slots 1 / 6) |
| `/v2/media/load` | i slot, s path | PNG / JPEG into a media slot (same path again = no-op) |
| `/v2/media/select`, `/v2/media/clear` | i slot | active slot / empty a slot |
| `/v2/media/use` | i 0/1 | USE MEDIA: the active image replaces every `images` source |
| `/v2/lowlatency` | [i 0/1] | wait for the previous frame's GPU fence before sampling features (default on) |
| `/v2/react` | 5 i (kick, snare, hat, bass, level; 0/1) | REACT TO gate |
| `/v2/palette` | 9 f (rgb x3, 0-1) [, f mix] | shadow / mid / light colours of the global gradient map (mix 0 = scene colours) |
| `/v2/preset` | i index **or** s name | select preset (name = case-insensitive substring) |
| `/v2/preset/next`, `/v2/preset/previous` | | |
| `/v2/blackout` | [i 0/1] | set / toggle |
| `/v2/trigger` | | manual hit |
| `/v2/transition` | f ms | override every preset's transition length (<0 restores) |
| `/v2/quality` | f 0 = auto, else fixed scale | |
| `/v2/demo` | [i 0/1] | |
| legacy `/audio/level|bass|mid|high|beatphase|onset`, `/preset/select|next|previous`, `/preset/transitionduration`, `/effect/toggle`, `/effect/param`, `/display/*`, `/fullscreen`, `/camera/open`, `/video/load`, `/debug/snapshot` | | unchanged |

Engine -> subscribed clients (5 Hz): `/v2/status` (index, name, count, blackout, fps, bpm, following-host, demo,
render scale, scene speed), `/v2/presets` (names, every ~2 s), `/v2/macros` (the live scene's 8 knob names by
slot + its description; on scene change and every ~2 s) and `/v2/media/status` (active slot, use-media, then per
slot name, width, height, loading).

Dropping PNG / JPEG files on the engine window fills the media slots (active slot first); video files still go
to the legacy video input.

## Presets

`engine/Presets/*.json`, sorted by file name (indices are file order). Two formats:

- **Legacy** (no `schemaVersion`): `shader` or `effectChain` + `{source, scale, offset}` audio mappings. Still
  loadable; the shipped set no longer contains any.
- **Schema 2** (`"schemaVersion": "2.0"`, the sixteen shipped scenes `01`-`16`): the format in
  `visual_instrument_research/preset.schema.json`, plus engine extensions:
  - `stages[].sources`: raw material bound to an ISF image input -
    `{"type": "images", "folder": "Images/Forms", "advance": "bar"|"beat"|"event.snare"|"none", "every": 2, "order": "random"}` or
    `{"type": "text", "words": [...], "font": "Arial Black", "advance": "beat"}`.
  - `parameters[].integrate: true`: the value is a rate; the shader receives its running integral (speed changes never jump).
  - Route sources: `audio.level|bass|mid|high.activity|absolute`, `audio.kick|snare|hat.activity`, `audio.band0..5.activity`,
    `descriptor.centroid|flatness|rolloff|flux|energyTrend|build|presence`, `clock.beatPhase|barPhase`, `macro.<id>`, `env.<id>`, `lfo.<id>`.
    `build` rises through build-ups (short-term energy above the long-term average; fast rise, slow release),
    `presence` is a ~2.5 s smoothed level.
  - `routes[].scaleBy: "macro.<id>"`: multiplies the route by 2 x that macro (the shipped scenes scale every hit
    envelope by Impact).
  - Trigger events: `event.kick|snare|hat|bassTransient|midTransient|highTransient|midiNote|userTrigger`.
  - `post.palette: "duo"`: a two-layer scene - the last stage writes BODY to red and DETAIL to green (and blue).
    The look pass grains, crushes and colours each layer on its own (body = palette mid, detail = palette light,
    screened over the shadow colour); *Scene Colors* shows them as red / cyan. Use with `"toneMap": "none"`.
  - Speeds: `integrate: true` parameters are rates advanced by the engine's scene clock (Speed x Push, Freeze,
    Reverse, Sync), so a scene only states its designed speed (the default) and never routes into a rate.
  - `sources` of `"type": "media"` (`{"type": "media", "fallback": "Images/Forms"}`) bind the active media slot;
    image and word advances hold while the scene is frozen.
  - The generator checks the control-model rules: nothing routes into a speed, at most one audio-driven source
    (audio / descriptor / hit envelope) per parameter, every SHAPE macro routed with a scene-specific label,
    speeds never negative, no reseed on kicks unless the scene asks (Negative's camera cuts).
  - Stages after the first can be effects (`"kind": "effect"`): they read the previous stage as `inputImage`
    (Negative = `pylon.fs` generator -> `negative_split.fs` treatment, which will also take loaded media).

The instrument presets are generated by `engine/tools/build_instrument_presets.py` (edit there, re-run). Shaders
live in `engine/Shaders/Instrument/` and share `common.glsl` via `#include`. Generators write **linear** colour;
the engine's finish pass does bloom, exposure, Reinhard tone mapping, vignette, grain and the sRGB encode.

Every preset shader may use the engine uniforms `vj_time` (the scene clock - animate with this, not `TIME`, so
Speed 0 / Freeze stop the scene), `vj_dt` (this frame's step of it, for simulations), `vj_speed`, `vj_beat`
(musical position), `vj_palette` (palette-advance steps), `vj_seed` (reseed counter), and `<imageInput>_size` for
every image input.

### Source material

`engine/Media/Images/Forms` holds original abstract 3D forms raymarched by `engine/tools/make_source_assets.py`. Put your own
PNG/JPG (transparent PNG works best) into `engine/Media/Images/User` and point a preset's source `folder` at it.

## Tests and tools

| Command | Checks |
|---|---|
| `analysis\build\Release\analysis_tests.exe` | 26 DSP acceptance tests (silence, calibration, kicks, hats, gain invariance, noise, vibrato, drop recovery, anti-phase, OSC encoding) |
| `analysis_tests --stats` | Novelty distributions per region (for tuning onset floors) |
| `analyze_wav song.wav [--csv out.csv]` | Offline analysis summary / per-frame CSV |
| `analyze_wav song.wav --send [--bpm 120] [--loop]` | Streams a WAV's analysis to the engine in real time - full test without Live |
| `python engine\preset_regression_test.py --demo --keep <dir>` | Loads every preset, snapshots it, flags black frames / load errors |
| `plugin\build\PluginHarness_artefacts\Release\PluginHarness.exe ui.png 8` | Runs the plug-in on a synthetic groove against a running engine, writes a PNG of its UI |
| `PluginHarness.exe --selftest` | Snapshot store / recall / timed morph / state round-trip checks |
