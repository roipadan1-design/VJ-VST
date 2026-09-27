# Reels recording: record engine output + master audio, up to 20s, vertical option

Researched 2026-09-28. Not built yet. No files touched by this research pass.

## Verdict

**Yes — achievable at good quality, without a major rearchitecture.** For a solo
hobbyist project this is roughly a session and a half of focused work:
- ~1 session: engine-side video capture (async GL readback) + encode via ffmpeg.
- ~0.5 session: plugin-side WAV capture + RECORD button + OSC wiring.
- ~0.5 session: mux, vertical crop, polish, test in Live.

The one thing explicitly **out of scope for v1**: a truly independent portrait
*composition* (re-rendering the generative scene itself at 9:16 instead of
cropping). See §2 for why, and the upgrade path if it's ever wanted later.

## 1. Video capture: async PBO readback, not naive glReadPixels

`MainComponent::render()` already has the right hook: it leaves the finished
frame in framebuffer 0 and calls `spoutSender.sendFrame(0, physicalWidth,
physicalHeight)` right there (`MainComponent.cpp:342-345`). A recorder taps
the same spot.

The engine already does **synchronous** `glReadPixels` today, but only for
one-off single-frame captures (`MainComponent::captureSnapshot`, `.cpp:351-370`,
and `forensics::dumpFrame`). Doing that every frame for up to 20 seconds would
stall the GL pipeline each frame and likely visibly drop frame rate on the
Iris Xe integrated GPU this project already treats as its performance ceiling.

**Fix: a small (~60-80 line) new `PixelReadback` helper using 2-3 ring-buffered
PBOs** (`GL_PIXEL_PACK_BUFFER`): each frame, `glReadPixels` into the oldest PBO
(returns immediately - it's writing into a buffer object, not client memory),
then map the PBO from N-2 frames ago (already finished) and hand those bytes
to the encoder. Adds 2-3 frames of latency to the recording (irrelevant -
nothing interactive depends on it) and keeps the render thread non-blocking.
`GLHelpers.h/.cpp` has `GLRenderTarget`/`FullscreenQuad` but no PBO helper yet
- this is genuinely new code, but small, and only active while recording.

## 2. Vertical (9:16): crop, don't re-render

Traced the actual data flow: `PresetManager::render()` (`.cpp:324-331`) takes
`frame.width/height` from the real window size and is architecturally
resolution-agnostic (nothing hardcoded to one aspect). But `compositeTarget`,
`currentTarget`, `outgoingTarget`, `frozenTarget`, and every `V2Instance`'s
per-preset `stageTargets`/`FinishPass` are single GL resource sets sized by
`GLRenderTarget::ensure()`, which reallocates (`glDeleteTextures`/`glGenTextures`
+ fresh storage) whenever the requested size changes. Calling
`presetManager.render()` a second time per frame at a different (portrait)
size would thrash every one of these every single frame - a serious,
possibly show-breaking hit.

Worse: state-advance lives inside `render()` itself, not a separate step -
`V2Instance::render()` drives `Modulation::process()` (integrates by `dt`) and
`advanceSources()` (steps on beat/bar crossings, independent of `dt`). A second
call per real frame would double-step beat-driven media/text cycling even if
you zero `dt` for the envelope math.

**Recommendation: center/native crop from the single existing landscape render**,
taken from the same PBO readback in §1 (as a GL blit into a portrait-sized FBO
with a "cover crop" UV transform, so you're still encoding native pixels, not
an upscaled screenshot crop). This fits the project's current visual direction
(generative/abstract, not framed shots with subjects that need true portrait
recomposition) and has zero risk to the live, audience-facing render path.
Optional no-risk quality bump while recording: push `renderScale` toward 1.0
and size the crop against `physicalHeight` so the 1080-wide portrait slice
isn't upscaled from a narrower source.

*Upgrade path if ever wanted later:* duplicate a `V2Instance`'s GL resources
into a second parallel instance, and on its call skip `advanceSources()`/
`Modulation::process()` (reuse only draw calls with values already advanced by
the first call). Real, separate-project-sized work - not part of this feature.

## 3. Audio/video sync: two files, muxed after, not a new realtime channel

Checked `analysis/include/vj/Protocol.h`: there is no shared wall-clock between
engine and plugin today - only host-transport-relative fields. Given that, and
given short ambient/generative clips with no dialogue/lip-sync to protect:

- Engine writes a **silent** .mp4 from its own PBO/encode loop.
- Plugin writes a **.wav** from the exact samples already in `processBlock`
  (see §4 - no WASAPI loopback needed).
- Both start on the same OSC-triggered instant (`/record/start`); localhost UDP
  latency is sub-ms, so the real skew is just per-process thread-scheduling
  jitter - on the order of one video frame (~16-33ms). Imperceptible for this
  content.
- After stop, mux once with `ffmpeg -i silent.mp4 -i audio.wav -c copy -shortest
  final.mp4` (instant, no re-encode).

Streaming raw audio engine-side for tighter single-process sync was considered
and rejected for v1: it needs a new socket/protocol extension and jitter
buffer for a sync improvement this content doesn't need.

## 4. Concrete pieces to build

**Engine (`engine/Source/`):**
- New `VideoRecorder.h/.cpp`: owns the PBO readback ring (§1), the
  crop-to-portrait GL blit (§2, when vertical requested), and an `ffmpeg.exe`
  child process fed raw frames on a **writable stdin pipe** (JUCE's
  `juce::ChildProcess` only exposes read access to a child's stdout, so this
  needs a small Win32 anonymous-pipe + `CreateProcess` with `STARTUPINFO.hStdInput`
  redirected - same tier of Win32 code already in `plugin/Source/EngineLauncher.cpp`,
  just for a pipe instead of window-focus calls). ffmpeg command for the video
  leg: `ffmpeg -f rawvideo -pix_fmt bgra -s WxH -r <fps> -i - -c:v libx264
  -preset veryfast -crf 18 -pix_fmt yuv420p -y silent.mp4`. Internal auto-stop
  timer for the requested duration (don't rely on `MainComponent` for the
  timeout).
- `MainComponent.h/.cpp`: add `VideoRecorder videoRecorder;`; call
  `videoRecorder.pushFrame(...)` right after the existing
  `spoutSender.sendFrame(0, ...)` call (`.cpp:344`) whenever recording is
  active. Two new OSC addresses in `oscMessageReceived`, next to the existing
  `/camera/open`/`/camera/close` block (`.cpp:980-996`):
  - `/record/start` `f durationSeconds` `i vertical(0|1)`
  - `/record/stop`
  - `/record/audiopath` `s path` (see mux step below)
- Output convention: `Documents\VJ VST\Recordings\<timestamp>.mp4`, matching
  the existing `Documents\VJ VST\Looks` convention (`LookLibrary.cpp:13`).

**Plugin (`plugin/Source/`):**
- `AnalysisWorker.h/.cpp`: same pending-flag pattern already used for the
  webcam button (`.h:94-95`, `.cpp:218-229`):
  ```cpp
  void startRecording (double seconds, bool vertical) { recordSeconds = seconds; recordVertical = vertical; recordStartPending = true; }
  void stopRecording() { recordStopPending = true; }
  ```
  drained in the same worker block, sending `/record/start` (float seconds,
  int vertical) / `/record/stop`.
- **Master audio - correction to the original assumption:** `isLead()`
  (`LeadRegistry.h`) is about which instance sends *control* OSC when several
  instances run at once, and its scoring favors role MIX over track name - it
  does **not** mean "this instance is on the Master track." What actually
  guarantees "this `processBlock` buffer is the true master mix" is simply
  being inserted on Live's Master track, independent of role/lead status.
  Recommendation: gate recording on a new `isOnMasterTrack()` check
  (`trackName.toLowerCase().contains("master")`, `trackName` already comes
  from the host's `TrackProperties` in `updateTrackProperties()`,
  `PluginProcessor.cpp:599-606`) - don't reuse `isLead()` for this.
- `PluginProcessor.cpp::processBlock` (around line 733, right where
  `worker.pushAudio(left, right, numSamples)` already runs): when a
  `std::atomic<bool> recording` flag is set, also feed the same `left`/`right`
  pointers into a `juce::AudioFormatWriter::ThreadedWriter` (standard JUCE
  idiom - writes WAV asynchronously off the audio thread) opened at
  `startRecording()`, closed at `stopRecording()`/timeout.
- `PluginEditor.h/.cpp`: new `recordButton` (toggle) + an orientation toggle,
  built exactly like the existing `webcamButton` (`.cpp:1285-1296`); place in
  the SETUP tab near the role/lead controls, since this is a "which instance"
  concern; `setEnabled` gated on `processor.isOnMasterTrack()` the same way
  `webcamButton` gates on `status.connected` (`.cpp:2309`).

**Muxing:** the plugin, on stop/timeout, sends the WAV's absolute path back to
the engine over the existing reply-port `OSCReceiver` (`AnalysisWorker` already
has `replyPort`, used for `/v2/hello`) as `/record/audiopath <path>`.
`VideoRecorder` waits for that (short timeout/fallback to keeping the silent
video if it never arrives - e.g. wrong instance recording) before running the
final `ffmpeg -c copy` mux into `Recordings\<timestamp>.mp4`.

## Open items / things to confirm before or during build
- Confirm the RECORD button's home (SETUP tab was suggested; could also live
  next to WEBCAM in MEDIA, or get its own small area) once it's on screen.
- Decide the default clip length control: fixed 20s vs a countable dial.
- ffmpeg.exe is confirmed installed on this laptop (the same machine used for
  both dev and the show, per project docs) - bundling a copy next to the
  engine .exe is optional extra insurance for the backup laptop, not required
  for v1.
