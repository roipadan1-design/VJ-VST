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
Every control is a host parameter: automate it or MIDI-map it with Live's MIDI Map mode (if one doesn't show up,
click *Configure* on the device and touch it). IDs never change between builds, so old sets and maps keep working.

### Lead and source instances

Exactly one instance **leads**: it sends the knobs, look, scene and blackout to the engine. It is elected
automatically: an instance pinned with MAKE LEAD, else a MIX instance, preferring a track called Master / Main,
else the oldest. So the old "Send macros" fight (two instances overwriting each other) is gone; the parameter
*Send Controls* now means "may lead" (EDIT > SETUP > CAN LEAD). The lead shows the full window; every other
instance shows a small **SOURCE** view: its role, a hit lamp, its band meter, DETECT / TRIM and MAKE LEAD.

**Role** (MIX / KICK / SNARE / HAT / BASS / TEXTURE) follows the **track name** while AUTO is on ("Kick 808"
-> KICK, "Hats" -> HAT, "Sub" -> BASS, "Master" -> MIX); picking one by hand turns AUTO off. KICK / SNARE / HAT
instances replace the drum guesses from the full mix. **MIDI** routed to an analyzer arrives as zero-guess
events; on a KICK instance a note *is* a kick, and a note of velocity 100 or more is always an accent.

### PLAY (760 x 480, the lead's main view)

- **Header**:
  - the engine pill (START ENGINE when it is off; else fps, BPM and bar)
  - FULL (engine full screen)
  - LEAD (the track name; click for setup)
  - EDIT (opens the drawer)
  - BLACKOUT (a steady red frame around the window while dark)
- **SCENE**: the live scene with its one-line description, and a 4 x 4 grid. Click a tile = cue it (NEXT, amber);
  **GO** switches on the scene's beat (at once when Live is stopped); double-click = cue + GO; `<` `>` move the
  cue. IMG marks scenes that show your media.
- **REACT**:
  - **STILL / BREATHE / PULSE / PUNCH**: the character, i.e. how the picture follows the music (parameter *React
    Style*; STILL = *Calm*: reactions fade out over one bar).
  - **The lamps** KICK / SNARE / HAT / BASS / LEVEL flash with the music. Click one to make the picture ignore
    that channel (the *React* toggles).
  - **The caption** says "resting", "build-up" or "DROP" live.
- **STYLE**: COLOUR (the palette, with its stripes), LOOK (Clean / Film / Worn / Broken / Print / Data; parameter
  *Look*) and LOOK AMOUNT (0 = clean picture, 100 % = the look as set; parameter *Look Amount*).
- **IMAGE**: the active media slot. LOAD, or drop an image or clip anywhere on the window. If the live scene
  can't show it, the plug-in switches to Media Negative and says so.
- **MOMENTS A-D** (the snapshots):
  - Click an empty moment to save the knobs, look, colour and reactions there.
  - Click a stored one to blend to it over the *morph* time.
  - Right-click to save over it or clear it.
  - A moment is named by its recipe ("Film · Blood").
- **The 8 knobs**:
  - SHAPE: Intensity, Form, Scale, Erode, Detail. The line under each says what it does in this scene.
  - MOTION: Speed and Glide, plus FREEZE.
  - **REACT** (macro 5, *React*; it was Impact): how much the music moves the picture. 0 = ignores it, 50 % = as
    designed (default), 100 % = wild. The thin outer ring flashes with every accent.
- **HIT**: a manual accent that lands even in STILL or at REACT 0.
- **DROP**: the big moment by hand, also detected automatically.
- **The info line** at the bottom explains whatever the mouse is over, or says what the plug-in is waiting for.

### EDIT drawer (the window grows to 1180 x 480)

- **LOOK**:
  - the six looks and AMOUNT
  - FILM knobs: Grain, Halation, Gate Weave, Dust (builds up through a section), Film Base
  - DIGITAL knobs: Crush, Trails, Glitch, Smear, Glyphs
  - ON ACCENTS knobs: Flash, SHOTS, HUD

  Turning a knob makes the look *Custom*.
- **COLOUR**: 14 palette cards (Scene Colors = palette off; Split = black / red / cyan for Negative) and the three
  custom colours.
- **REACT**:
  - the character row
  - REACT, FOLLOW (*Reactivity*, a trim on the steady following), DECAY (*Softness*: 50 % = as the character
    says) and MUSIC PUSH (*Push*: 30 % = as the character says)
  - the FOLLOWS toggles
  - a live readout of accents, drops, rest and build-up
  - DROP NOW
- **MOTION**: Camera Drift, FREEZE / REVERSE / TEMPO-LOCK (*Sync*), and AUTO SCENE CUTS (*Cut Rate*: off, every
  16 / 8 / 4 / 2 / 1 beats, or on every accent).
- **MEDIA**:
  - 8 slot cards (click = show it; drop a file on a card = load it there)
  - LOAD, CLEAR, and USE IN ALL IMAGE SCENES
  - LOOP / PING-PONG, and the clip length (Free, 1 beat .. 8 bars)
- **SETUP**:
  - the role (AUTO), LEAD status, MAKE LEAD and CAN LEAD
  - DETECT (*Hit Sensitivity*), TRIM, LOOKAHEAD (0-150 ms; playback / DJ sets only)
  - LEVEL Auto / Fixed (*Response*)
  - the signal meters
  - the engine path, with LOCATE

### How the picture reacts (engine: `Reaction.h`)

- **Accents, not every hit.** Each kick is ranked against the last 16. Only the strong ones (and downbeats, MIDI
  velocity 100 or more, HIT, and the first hit after a pause) become **accents**: the full event, held for a few
  frames, a short punch of light, and a small eased push forward of the scene's own motion (never a jump, never
  runaway speed). The others are small **ticks**. Snares join the ranking only in PUNCH.
- **Rest.** When the music goes quiet, the picture rests: darker (to about a third in PULSE) and sparser, with
  the grain still alive. The first hit wakes it.
- **Body, energy and air.** The bass, the loudness and the highs are measured against each song's own range, so
  a mastered track and a quiet sketch both use the whole range. They drive each scene's mass, its rest look and
  its surface.
- **Build-up and drop.**
  - A build-up counts only when the music really rises (not when it simply starts).
  - The drop fires on a loud release after a build-up (or after a rest), or with DROP: a bloom, a bigger lurch,
    and each scene's own release.
  - Scars (extra dust, Ink holes, Terminal wear) accumulate through a section and are wiped by the drop.
- **Characters.**
  - BREATHE: only the big moments land; slow, deep rest.
  - PULSE (default): every beat felt, strong hits land.
  - PUNCH: tight, more hits, snares too.
- **Safety.** At most 3 accents / flashes / cuts per second. Speed 0 or FREEZE: accents still light up, but
  nothing moves.

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
| `/v2/media/load` | i slot, s path | still (PNG / JPEG) or clip (MP4 / MOV / ...) into a media slot (same path again = no-op) |
| `/v2/media/select`, `/v2/media/clear` | i slot | active slot / empty a slot |
| `/v2/media/use` | i 0/1 | USE MEDIA: the active image replaces every `images` source |
| `/v2/media/mode` | i 0/1 | clips: 0 loop, 1 ping-pong |
| `/v2/media/sync` | i beats | clips: 0 free, else span this many beats (tempo-locked) |
| `/v2/lowlatency` | [i 0/1] | wait for the previous frame's GPU fence before sampling features (default on) |
| `/v2/react` | 5 i (kick, snare, hat, bass, level; 0/1) | REACT TO gate |
| `/v2/palette` | 9 f (rgb x3, 0-1) [, f mix] | shadow / mid / light colours of the global gradient map (mix 0 = scene colours) |
| `/v2/preset` | i index **or** s name | select preset (name = case-insensitive substring) |
| `/v2/preset/next`, `/v2/preset/previous` | | |
| `/v2/blackout` | [i 0/1] | set / toggle |
| `/v2/trigger` | [s "drop"] | manual hit (a forced accent); with "drop": the performer's DROP |
| `/v2/style` | i 0-2 | reaction character: 0 BREATHE, 1 PULSE, 2 PUNCH (REACT itself is macro slot 4) |
| `/v2/transition` | f ms | override every preset's transition length (<0 restores) |
| `/v2/quality` | f 0 = auto, else fixed scale | |
| `/v2/demo` | [i 0/1] | |
| legacy `/audio/level|bass|mid|high|beatphase|onset`, `/preset/select|next|previous`, `/preset/transitionduration`, `/effect/toggle`, `/effect/param`, `/display/*`, `/fullscreen`, `/camera/open`, `/video/load`, `/debug/snapshot` | | unchanged |

Engine -> subscribed clients (5 Hz, and on the next 50 ms tick after a scene change): `/v2/status` (index, name, count, blackout, fps, bpm, following-host, demo,
render scale, scene speed, then rest 0-1, tension 0-1, accent envelope, accents so far, drops so far; at once on a scene change), `/v2/macroActivity` (8 floats: how far the sound moves each macro slot's targets,
peak-held ~0.3 s - the plug-in's knob rings), `/v2/presets` (names, every ~2 s), `/v2/macros` (the live scene's 8 knob names by
slot + its description; on scene change and every ~2 s) and `/v2/media/status` (active slot, use-media, then per
slot name, width, height, loading).

Dropping stills or clips on the engine window fills the media slots (active slot first); other video formats
Media Foundation reads still go to the legacy video input.

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
