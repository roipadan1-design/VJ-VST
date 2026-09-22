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
- **Macros 1-8** (Intensity, Motion, Color, Space, 5-8): real host parameters - automate them, or MIDI-map with
  Live's MIDI Map mode (if a parameter doesn't show up for mapping, click *Configure* on the device and touch it).
  Only enable **Send macros** on one analyzer (normally the MIX one).
- **Preset**: 0 = leave the engine's choice; the list on the right selects directly and records the choice in
  the parameter, so the Live set recalls it.
- **HIT** fires a manual hit (event `userTrigger`); **BLACKOUT** fades the output to black.
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
`VJEngine.log`). Measured on this machine's Intel Iris Xe: Liquid Chrome holds ~57 fps at 84 %.

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
| `/v2/preset` | i index **or** s name | select preset (name = case-insensitive substring) |
| `/v2/preset/next`, `/v2/preset/previous` | | |
| `/v2/blackout` | [i 0/1] | set / toggle |
| `/v2/trigger` | | manual hit |
| `/v2/transition` | f ms | override every preset's transition length (<0 restores) |
| `/v2/quality` | f 0 = auto, else fixed scale | |
| `/v2/demo` | [i 0/1] | |
| legacy `/audio/level|bass|mid|high|beatphase|onset`, `/preset/select|next|previous`, `/preset/transitionduration`, `/effect/toggle`, `/effect/param`, `/display/*`, `/fullscreen`, `/camera/open`, `/video/load`, `/debug/snapshot` | | unchanged |

Engine -> subscribed clients (5 Hz): `/v2/status` (index, name, count, blackout, fps, bpm, following-host, demo,
render scale) and `/v2/presets` (names, every ~2 s).

## Presets

`engine/Presets/*.json`, sorted by file name (indices are file order). Two formats:

- **Legacy** (`00`-`08`): `shader` or `effectChain` + `{source, scale, offset}` audio mappings. Unchanged.
- **Schema 2** (`10`+, `"schemaVersion": "2.0"`): the format in
  `visual_instrument_research/preset.schema.json`, plus engine extensions:
  - `stages[].sources`: raw material bound to an ISF image input -
    `{"type": "images", "folder": "Images/Masks", "advance": "bar"|"beat"|"event.snare"|"none", "every": 2, "order": "random"}` or
    `{"type": "text", "words": [...], "font": "Arial Black", "advance": "beat"}`.
  - `parameters[].integrate: true`: the value is a rate; the shader receives its running integral (speed changes never jump).
  - Route sources: `audio.level|bass|mid|high.activity|absolute`, `audio.kick|snare|hat.activity`, `audio.band0..5.activity`,
    `descriptor.centroid|flatness|rolloff|flux|energyTrend`, `clock.beatPhase|barPhase`, `macro.<id>`, `env.<id>`, `lfo.<id>`.
  - Trigger events: `event.kick|snare|hat|bassTransient|midTransient|highTransient|midiNote|userTrigger`.

The instrument presets are generated by `engine/tools/build_instrument_presets.py` (edit there, re-run). Shaders
live in `engine/Shaders/Instrument/` and share `common.glsl` via `#include`. Generators write **linear** colour;
the engine's finish pass does bloom, exposure, Reinhard tone mapping, vignette, grain and the sRGB encode.

Every preset shader may use the engine uniforms `vj_beat` (musical position), `vj_palette` (palette-advance steps),
`vj_seed` (reseed counter), `TIMEDELTA`, and `<imageInput>_size` for every image input.

### Source material

`engine/Media/Images/Masks` holds original generated masks (`engine/tools/make_source_assets.py`). Put your own
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
