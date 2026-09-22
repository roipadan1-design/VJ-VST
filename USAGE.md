# VJ Engine - Usage

Quick reference for running a session. Written from the actual code/patch,
not aspirational - if something here stops matching reality, the code is
the source of truth.

## Starting a session

Double-click **`Start VJ Session.bat`** (repo root). It launches `VJ Engine.exe`
(picks up a Release build if one exists, otherwise Debug) and Ableton Live.
Ableton opens with no project loaded - open your own Live set and make sure
it contains the **VJ Audio Analyzer** M4L device (`m4l-device/VJ Audio Analyzer.amxd`)
on an audio track, then press play.

If you'd rather start things by hand: run `engine/build/VJEngine_artefacts/Debug/VJ Engine.exe`,
then open Live separately. The engine binds UDP port 9000 on startup by
default - **don't run two instances on the same port.** Windows will happily
let a second `VJ Engine.exe` start and *also* bind UDP 9000 without any error
on either side - OSC messages then go to whichever instance the OS hands
them to, unpredictably, while the *other* (possibly the one you're actually
looking at) just sits there looking unresponsive. If preset/effect changes
stop showing up, check Task Manager for more than one `VJ Engine.exe` first.

**Running more than one instance on purpose** (e.g. driving two different
outputs/monitors with independently-controlled content): pass `--osc-port <N>`
to give the second instance its own port, e.g.

```bash
"VJ Engine.exe" --osc-port 9001
```

Each instance also gets its own Spout sender name (`VJ Engine (9001)` instead
of the default `VJ Engine`) so they don't collide there either. You'd then
need a second M4L device (or a second `udpsend`/OSC source) pointed at that
port to control it independently of the first.

## Loading the M4L device

Drag `m4l-device/VJ Audio Analyzer.amxd` onto an audio track in Live (a track
that's actually receiving/playing audio, so the analyzer has a signal to
read). It sends OSC to `127.0.0.1:9000`, which is where the engine listens.
The device's front panel (Presentation view) has: Level/Bass/Mid/High meters,
a Beat indicator, a Freeze toggle (pauses analysis without unloading), and
Preset Select/Next/Prev controls - "Preset Select" is a real Live device
parameter (`live.numbox`), so it's automatable and savable in Live's own
automation lanes, not just a manual control.

A second device, `m4l-device/VJ Effect Controls.amxd`, gives 4 generic knobs
for live effect-parameter control (see `/effect/param` below) - drag it onto
any track alongside (or instead of) the analyzer. Each knob row has: a Stage
number box (which `effectChain` stage of the *current* preset to target, 0-based),
a Param name field (the ISF input name - check the preset's shader header for
valid names, e.g. `blockCount`, `jitterAmount`), Min/Max number boxes (the
real-world range the knob's 0-1 travel maps to), and the knob itself.
**Not yet verified working** - see the in-patch comment for why (Max's trial
state on this machine has, as of the last check, prevented newly-added patch
objects from actually executing, even though they load and display correctly).

## Keyboard shortcuts (engine window)

| Key | Action |
|---|---|
| `F` / `F11` | Toggle real OS fullscreen on the current monitor |
| `[` / `]` | Move the window (and fullscreen state, if active) to the previous/next monitor |
| `Left` / `Right` | Previous / next preset |
| `1`-`9` | Toggle effect-chain stage N on/off (only affects `effectChain` presets; no-op otherwise) |
| `C` | Open the default webcam as the video input |
| Drag a video file onto the window | Load it as the `inputImage` source for `effectChain` presets |

## OSC contract (UDP, port 9000, 127.0.0.1)

| Address | Args | Effect |
|---|---|---|
| `/audio/level` | float 0-1 | Overall level |
| `/audio/bass` | float 0-1 | Bass band |
| `/audio/mid` | float 0-1 | Mid band |
| `/audio/high` | float 0-1 | High band |
| `/audio/beatphase` | float 0-1 | Position within the current beat (from Live's song time) |
| `/audio/onset` | none (bang) | Rising-edge bass transient (bass crossing above 0.3). Drives a decaying `onset` pulse (peaks at 1.0, linear decay to 0.0 over 150ms) available to every ISF shader/preset exactly like the signals above - e.g. the "Video Glitch Chain" preset's `ScanlineJitter` stage spikes its jitter on each onset. |
| `/preset/select` | float/int index | Jump to preset N (0-based, wraps) |
| `/preset/next` / `/preset/previous` | none | Step through presets |
| `/preset/transitionduration` | float/int milliseconds | Set the crossfade length used by future preset switches (default 600ms; 0 = hard cut). An in-progress crossfade keeps whatever duration it started with. |
| `/effect/toggle` | float/int stage index | Toggle an effect-chain stage on/off |
| `/effect/param` | int stageIndex, string paramName, float value | Live-set an ISF input on a preset (a specific `effectChain` stage, or the lone shader of a single-shader preset, which ignores stageIndex). If the target preset's JSON also has an `audioMappings` entry for that param name, the audio-reactive calc overwrites this value again on the very next frame - only unmapped params stay "set". |
| `/display/select` | float/int index | Move the window to monitor N |
| `/display/next` / `/display/previous` | none | Move to the previous/next monitor |
| `/fullscreen` | float/int (0/1) or none | Set, or (with no arg) toggle, fullscreen |
| `/camera/open` | float/int device index (default 0) | Open a webcam as video input |
| `/debug/snapshot` | none | Write the current frame to `VJEngine_snapshot.png` next to the .exe, overwriting any previous one. Lets a preset/effect be checked without a live screenshot - e.g. from a script, or by an assistant with no screen access. |

## Performance

Every 5 seconds the engine appends an FPS line (with the current preset name)
to `VJEngine.log` next to the .exe - the same file/format `VideoPlayer`'s
camera/decode diagnostics use, since there's no debugger attached in normal
use. Useful for judging whether a preset/effect chain or video source is
actually taxing the GPU.

## Preset transitions

Switching presets (keyboard, `/preset/select`, `/preset/next`/`previous`, or a
future scene-link) crossfades rather than cutting - the outgoing preset keeps
rendering (and reacting to audio) for 600ms (adjustable, see
`/preset/transitionduration` above) while the incoming one fades in.
Switching again mid-fade just retargets the fade to the new preset instead of
queuing or snapping - no special handling needed on the controlling end.

## Presets

Scanned from `engine/Presets/*.json` at startup, sorted by filename (hence
the `00-06` prefixes controlling order). Currently:

- `00 - MultiPass Test` - single ISF shader, multi-pass sanity check
- `01 - Star Bass Pulse` / `02 - Noise Level Reactive` / `03 - Star Static Blue` - single-shader, audio-reactive test shaders
- `04 - Video Glitch Chain` - RGBShift -> BlockGlitch -> ScanlineJitter, needs a video/camera input
- `05 - Video Datamosh` - persistent-buffer datamosh effect, needs a video/camera input
- `06 - Kaleido Mosaic` - Pixelate -> Kaleidoscope, needs a video/camera input (compile-verified via `VJEngine.log`, not yet visually checked)

Add a preset by dropping a new `*.json` file in `engine/Presets/` (see the
existing files for the `shader` vs `effectChain` shape, `params`, and
`audioMappings` - source can be `level`/`bass`/`mid`/`high`/`beatphase`/`onset`).

## Known scope limits (intentionally out for now)

Projection mapping, VST3, MadMapper integration - not installed, not planned
for the current milestone.

Scene-linked preset switching (name a Session View scene with a leading
number, e.g. "3 Drop", to auto-select that preset) is implemented in
`VJ Audio Analyzer.maxpat` but likewise unverified for the same Max-trial
reason as the effect knobs above.
