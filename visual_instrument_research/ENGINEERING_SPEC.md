# Engineering specification — Visual Instrument v2

**Status:** proposed implementation contract, 22 September 2026. This is a design for the next version, **not** a claim that the current C++/Max code already implements it. Normative “must” statements are proposed acceptance requirements. DSP constants, GPU allocations and latency limits are starting targets to qualify locally.

**Target:** Windows 11 Pro, Ableton Live 10 Suite, Max 8, discrete NVIDIA-class GPU. Record exact versions; use Live 10.1.43 and a tested Max 8 build as the qualification baseline. A later VST3 wrapper needs Live 10.1+ format support and the relevant MIDI fixes; it does not require Max 9. [[H01]](sources.md#h01)

**Related files:** REPORT.md contains evidence and alternatives; STARTER_PRESETS.md contains twelve designs; preset.schema.json and audio-feature-contract.schema.json define structural contracts; validate_presets.py checks examples and semantic relationships. The new preset format is **not backward-loadable by the prototype without an adapter**.

## 1. Goals and non-goals

The first delivery makes three attractive generative presets playable with four macros, reliable audio/MIDI events, musical transitions and stage-safe output. It preserves the existing external renderer and Spout path. It keeps analysis/control failures from blocking the audio callback and makes timing/state observable.

Do not include projection mapping, a general node editor, live diffusion, source-separation ML, an unrestricted shader marketplace or a graphics-API rewrite in the critical path. Do not require every stem to run the richest analyzer. Retain existing video/glitch content through a legacy adapter, but do not make the new generative set depend on a video file.

## 2. Components and ownership

```text
ABLETON LIVE 10                                      EXTERNAL ENGINE
┌────────────────────────────────────────┐          ┌───────────────────────────┐
│ Audio track: VJ Audio Analyzer         │ features │ Source Registry           │
│ audio callback → bounded audio FIFO   ├─────────►│ Clock / Health Estimator  │
│ analysis worker → publication queue   │ events   │                           │
│                                       │          │ Event Scheduler           │
│ MIDI track: VJ MIDI Bridge             ├─────────►│ Role Resolver / Dedup     │
│ played notes / selected CC, pass-through│          │                           │
│                                       │ control  │ State Coordinator         │
│ VJ Instrument control device          ├─────────►│ Preset Loader / Cache     │
│ macros, preset, sources, output policy │◄─────────┤ ACK / telemetry           │
└────────────────────────────────────────┘          │                           │
                                                   │ Modulation Runtime        │
Later: thin VST3 AudioProcessor wrapper              │ Render Graph              │
uses the same AnalysisCore / contracts.              │ Linear Finish + Blackout  │
                                                   │ Spout / Window / Preview  │
                                                   └───────────────────────────┘
```

**Audio Analyzer** owns input observations, not preset selection. **MIDI Bridge** owns performed MIDI events and optional controller forwarding; it preserves note pass-through. **Instrument control device** is the single conductor for one output bus. **Engine** owns GPU resources, preset execution and presentation. **AnalysisCore**, when implemented in C++, is a host-independent library with no JUCE GUI, GL or socket dependency.

Create stable `sessionId`, `sourceId` and `outputBusId` values. A host-duplicated device can duplicate saved IDs, so the engine must also assign a fresh `connectionId` and detect collisions. Do not silently merge two live sources merely because a copied device contains the same UUID. Show a collision and offer a deterministic new source identity. One conductor lease per bus prevents two tracks fighting over the current preset.

## 3. Threading and failure boundaries

| Execution context | Allowed work | Forbidden work |
|---|---|---|
| Host audio callback | Bit-preserving audio pass-through; cheap envelope; timestamp capture; bounded FIFO write | Allocation, file/network I/O, JSON, locks/waits, process launch, GL, percentile scans |
| Analysis worker | FFT, spectral bands, descriptors, robust thresholds, history update | Blocking the audio thread; waiting for rendering |
| IPC worker / Max message path | Bounded serialization, sockets, ACK/retry, diagnostics | Running GPU work or unbounded backlog replay |
| Engine command thread | Packet validation, permission checks, asset parsing, state staging | Mutating active GL objects concurrently |
| Render thread | Context ownership, GPU resources, frame-boundary swaps, modulation evaluation, output | Waiting for host audio or synchronous disk access |
| UI/message thread | UI, file dialogs, launch/discovery, editing base state | Direct unsynchronized writes to render/audio state |

Use a preallocated **single-producer/single-consumer** ring at each actual SPSC boundary. Multiple producers need separate queues or a deliberately chosen MPSC mechanism. An `AbstractFifo` only manages indices; allocate the storage separately. [[E02]](sources.md#e02)

The worker’s audio ring should cover at least 8192 samples at 48 kHz, scaled with sample rate. This is capacity, **not a desired latency**. Target normal backlog below 6 ms. If the worker falls more than 50 ms behind, skip stale analysis, emit a discontinuity and rebuild the windows from recent audio. Never delay audio to preserve visual-analysis completeness. Reset onset comparisons after a discontinuity; do not interpret missing frames as a transient.

## 4. Audio analysis contract

### 4.1 Supported rates and frame plans

Initial qualification rates: 44.1 and 48 kHz. Also define plans for 88.2/96 and 176.4/192 kHz; advertise them only after tests. For 44.1/48 kHz use the values below. Double FFT lengths and hops for the 88.2/96 family, quadruple for the 176.4/192 family to preserve approximate time/frequency scales. Unexpected sample rates require an explicit supported plan or a visible analysis-disabled state, not hidden resampling.

| Path | Window/hop at 48 kHz | Output and purpose |
|---|---|---|
| Fast time domain | Sample-based; publish with feature frame | Peak/RMS envelope; 5 ms attack, 120 ms release |
| Short spectral | 1024 / 128, periodic Hann | Transient novelty; 21.33 ms observation window, 2.67 ms hop |
| Main spectral | 2048 / 256, periodic Hann | Six bands, 32 display/control bands, descriptors; 42.67 ms window, 5.33 ms hop |
| Optional slow | 4096 / 512 | Low-frequency detail or offline diagnostics; not the default hit trigger |
| Normalizer statistics | 100 Hz history, 10 Hz quantile updates | Adaptive activity and confidence |
| IPC publication | Target 120 Hz latest snapshot | Render interpolation, not 120 Hz recomputation of every analysis feature |

Actual M4L scheduler publication may jitter. A 120 Hz request is not an audio-thread timing guarantee. Keep sample-based measurement timestamps so the engine can distinguish new evidence from late delivery.

### 4.2 Channel handling and calibrated energy

Do not average L and R waveforms before all analysis: anti-phase content could disappear. Compute channel powers separately and average powers. For an unnormalized DFT, an energy-calibrated one-sided bin estimate is:

```text
P[k] = s[k] * (|XL[k]|² + |XR[k]|²) / (2 * N * sum(w[n]²))
s[k] = 1 at DC and Nyquist, otherwise 2
```

For mono, omit the channel average. Confirm the scaling with unit-amplitude bin-centered sinusoids and noise. A full-scale sine has approximately −3.01 dBFS mean-square level under this amplitude convention; this is not an integrated LUFS meter. Exclude DC for musical descriptors unless explicitly needed.

Six band edges are **20, 60, 150, 400, 2000, 6000, 16000 Hz**. Integrate energy using fractional edge-bin weights rather than simply dropping a boundary bin. Publish each band’s actual edges. At the supported rates, the highest edge remains at or below Nyquist. Define aggregate bass as bands 0–1, mid as 2–3, high as 4–5. Raw aggregate energy is computed before logarithms; do not average dB values to sum band energy.

Use **32 logarithmically spaced control bands** from 30 Hz to min(16 kHz, Nyquist), with nonnegative triangular weights. Normalize overlapping weights deliberately; retain a separate six-band energy path so a display filterbank does not silently change the meaning of dBFS. This is a useful artistic resolution, not an instrument classifier.

### 4.3 Envelopes

For peak-style input use `abs(x)`; for RMS use an energy smoother then square root. Apply attack/release coefficients according to whether the target rises or falls:

```text
a = exp(-dt / tau)
y = a*y + (1-a)*x
```

A zero time constant means immediate assignment. Clamp nonfinite input before publication and count the error. Use a floor of −160 dB for logging zeros. Parameter attack/release values describe this exponential time constant, not a promise of complete settling within that interval.

Keep raw peak/RMS, absolute activity and relative activity distinct. Momentary loudness uses a much longer integration scale than a drum transient; EBU mode specifies 400 ms momentary and 3 s short-term windows. An optional LUFS meter may guide structure/intensity, but it must not replace the fast event path. [[D15]](sources.md#d15)

### 4.4 Adaptive normalizer — mandatory v1 behavior

Maintain an independent normalizer for broadband level and each useful band/aggregate. Default mode is **Balanced**:

1. Keep 8 seconds of **100 Hz** gated dB measurements: 800 entries in preallocated storage. Use a fixed histogram over −100 to +12 dB in 0.5 dB bins; update counts as entries expire. Compute Q20 and Q95 at 10 Hz on the worker. Values outside that range saturate the histogram but retain their true raw measurement.
2. Desired upper anchor `U* = Q95 + 3 dB`. Desired span `W* = clamp(U* − Q20, 24, 60) dB`. A narrow dynamic range must not cause unlimited amplification of tiny fluctuations.
3. If U must rise, follow it with a 150 ms time constant. If it must fall, limit the fall to **2 dB/second**. Raising U reduces sensitivity promptly on a loud arrival; lowering it slowly prevents the response from immediately expanding every breakdown back to maximum.
4. Smooth W with a 3 second time constant, retain its 24–60 dB bounds. Compute `relative = clamp((dB − (U − W)) / W, 0, 1)`.
5. Default signal gate: open above **−60 dBFS**, close below **−66 dBFS**, 200 ms hold. These are configurable starting values, not universal noise-floor estimates. While closed, output zero activity, decay envelopes and stop admitting measurements to normalization history.
6. On reopening, preserve prior anchors briefly. If history is empty or stale for more than 30 seconds, use a conservative fixed reference and collect a 2 second startup calibration, then blend to the learned anchors over 1 second. Switching presets does not reset source calibration.
7. Publish `calibrated`, gate state and raw dB so the UI explains an unexpected response. Missing or nearly silent bands do not become high activity because their local normalization range collapses.

Define absolute activity as `clamp((rmsDbfs + 60) / 48, 0, 1)` initially, gated separately; expose a sensitivity trim rather than claiming a psychoacoustic absolute truth. Normalizer input trim affects analysis only, never the audio passed through to Live.

**Modes:** Locked freezes anchors after calibration; Balanced uses the policy above; Ambient permits a user-confirmed lower gate and a slower, wider response, but does not auto-amplify numerical noise. The first implementation may ship Locked/Balanced only.

**Preserve structure:** `energyTrend` must use pre-normalization log energy. Compare a 0.5 second smoother with a 4 second smoother; scale their difference by a documented 12 dB reference and clamp to −1…1. It is a build/drop hint, not a semantic section classifier. Default automatic preset changes remain off.

### 4.5 Onsets and role events

Start with log-magnitude, per-frequency-whitened **positive spectral flux** on the short FFT. Use a local neighboring-bin maximum in the previous spectrum to suppress small pitch-shift/vibrato differences; label this implementation “SuperFlux-inspired” until validated against that algorithm, not an exact reimplementation. The primary paper explains why vibrato suppression helps. [[D06]](sources.md#d06)

Whitening must have an amplitude floor and bounded gain. Over-whitening silence creates false novelty. For each bass/mid/high novelty stream maintain a one-second robust history. Threshold is median + **3 × MAD + floor**, where MAD is median absolute deviation. Treat 3 as an empirical coefficient, not a Gaussian false-alarm probability. Exclude the candidate and a short recent decision guard from its own threshold estimate. Define the novelty scale during implementation and tune the floor against silence/noise fixtures; **do not invent a universal numeric floor independent of FFT/whitening normalization**.

Peak picking allows one short-hop lookahead for confirmation; provide an immediate mode only after its false-trigger cost is understood. Initial refractory periods: **bass 90 ms, mid 70 ms, high 45 ms**. A strong new event during refractory is counted diagnostically, not emitted twice. Optional event strength derives from excess above threshold with a bounded normalization, not from absolute audio level alone.

An event generated from the full mix is named `bassTransient`, not “kick.” The **Role Resolver** can map a dedicated kick source’s transient to `event.kick`. If a kick MIDI role is assigned, MIDI wins for that role and corresponding audio hits within an 80 ms matching window are suppressed. Keep this matching window configurable; two legitimate kicks closer than the window must not be collapsed when they come from the authoritative MIDI stream.

### 4.6 Transport and MIDI timing

Use `plugphasor~` as an audio-rate beat-phase source where applicable and `plugsync~`/Live state for transport metadata. A polled Live API position is useful but must not be described as inherently sample-accurate. [[H04]](sources.md#h04) [[H05]](sources.md#h05)

The clock tuple contains playing/valid, BPM, beat position, beat/bar phase, meter and an epoch. Increment epoch on a discontinuous seek/reset; distinguish an expected loop wrap from a network reorder. Beat position is in quarter-note units. In 6/8, a bar is three quarter notes, not six; calculate `barBeats = numerator * 4 / denominator`. Store the bar origin so loop/pickup positions do not accidentally redefine the downbeat.

Use **performed** MIDI notes from a M4L MIDI bridge for hits. Reading clip note data is useful for previews or known future scheduling but is not equivalent to observing actual playback under mute, launch, loop, groove and editing. MIDI removes onset-detection uncertainty, not display latency. Scene/clip observers belong to the message/control path; validate the existing scene-name linkage before relying on it.

## 5. Feature and event formats

The JSON examples are human-readable capture/debug representations governed by `audio-feature-contract.schema.json`. **Do not send pretty-printed JSON at 120 Hz as the required wire format.** Preserve their semantics in a compact OSC bundle. IDs are UUID strings; sample indices and monotonic timestamps are 64-bit values in the logical model.

Feature frames contain source identity, sequence, newest analyzed sample index, rate, analysis age, transport, broadband level, six bands, spectrum32, useful descriptors and health. `analysisAgeSamples` describes the representative age of a continuous estimate; it is not an asserted measurement of the algorithm’s event latency. Event timestamps identify estimated/performed event time separately.

### 5.1 Initial OSC wire layout

Continue listening on port 9000 by default, with a configurable return port. Bind loopback unless the user deliberately enables remote operation. Keep individual UDP datagrams **≤1200 bytes**; split optional spectrum/diagnostics into a separately sequenced message if the encoded bundle exceeds that budget. Required header, transport and activity values must be coherent. Never concatenate unrelated frames merely to fill a packet.

| Address | Payload semantics |
|---|---|
| `/v2/hello` | protocol version, session/source IDs, return port, role, capabilities; reply assigns connection token |
| `/v2/frame/header` | token, source ID, sequence, sample-index hi/lo, sample rate, analysis age, monotonic-us hi/lo |
| `/v2/frame/transport` | epoch, valid/playing, BPM, whole beat + fractional beat, beat/bar phases, meter |
| `/v2/frame/level` | raw RMS/peak dB, relative/absolute activity, gate/calibration flags |
| `/v2/frame/bands` | six ordered raw dB, relative and absolute values; band edges supplied on hello/config change |
| `/v2/frame/shape` | centroid01, flatness, rolloff01, flux01, energyTrend |
| `/v2/spectrum32` | token, source ID, frame sequence, 32 bounded values; optional separate packet |
| `/v2/event` | token, source/event IDs, sequence, sample timestamp, epoch, type/role, strength, optional note/channel, expiry |
| `/v2/control` | token, command UUID, expected state revision, command type, small bounded payload |
| `/v2/ack` | command UUID, accepted/pending/applied/rejected, state revision, reason code |
| `/v2/health` | heartbeat, current preset/state revision, render timing, dropped packets/events, source age |

Use OSC 1.0 int32/float32/string types on the conservative Max path. Represent logical uint64 fields as two signed int32 bit-pattern words and reassemble unsigned in the receiver; test boundary/wrap cases. Split long beat positions into integer/fraction to avoid losing phase precision in float32. Implement this encoding explicitly rather than relying on an external’s undocumented support for 64-bit OSC extensions. Bundle semantics and encoding come from the OSC specification. [[E07]](sources.md#e07)

Do not pack full presets into routine control datagrams. Presets are validated local assets addressed by stable ID/content hash. A bounded state transaction includes preset, macros, source assignments and output policy; a larger future asset-transfer service belongs on a framed reliable channel, not ad hoc oversized UDP.

### 5.2 Delivery and expiry

Features: latest valid sequence wins; interpolate slowly varying values from recent timestamps and never replay stale backlog. Events: deduplicate by connection/event ID, preserve order within the same source where possible, expire after **150 ms** by default. A delayed hit is normally worse than a missed hit. Count both distinctly.

Controls: acknowledge receipt, validate atomically, then report applied/rejected. Retry receipt at 100/200/400 ms, then show a connection error; do not repeatedly apply the action. Keep a bounded deduplication cache for at least 30 seconds. An accepted command waiting for a quantized boundary remains pending; it is not resent as a new command. Cap the pending queue and keep only the latest superseding preset request.

After 250 ms without features, mark the source stale and release activity toward zero over 300 ms. After 2 seconds, show disconnected. These are visual policies, not network-reliability guarantees. Stop emitting synthetic onsets from stale state. Unknown protocol majors or nonfinite numbers are rejected with bounded diagnostics.

## 6. Modulation runtime

### 6.1 Typed sources and evaluation order

Continuous sources have documented ranges: audio activity 0–1, descriptors generally 0–1, energy trend −1…1, phases 0–1, unipolar/bipolar LFOs as declared. Events never masquerade as a one-frame float pulse. They trigger owned envelopes or discrete actions.

At each render/simulation evaluation:

1. Resolve source roles and valid timestamped events.
2. Advance transport/free-running LFOs and envelopes using elapsed time, not frame count.
3. Read host/UI/controller **base** macro values. Apply pickup/smoothing to the control gesture, not to a rewritten automation stream.
4. Evaluate routes in a stable order into a separate accumulator per destination.
5. Add accumulated contributions to the normalized base parameter, then clamp **once**. Explicit cyclic parameters wrap instead of clamp.
6. Convert to physical range and publish a coherent parameter block for the render frame.

A route implements:

```text
u = clamp((source - inputMin) / (inputMax - inputMin), 0, 1)
q = curve(u)     # linear, u^exponent, or u*u*(3-2*u)
q_smooth = attack_release(q, route.attackMs, route.releaseMs, dt)
contribution = amount * (q_smooth - center)
result01 = clamp(base01 + sum(contribution), 0, 1)
physical = min + result01 * (max - min)
```

Validate `inputMin < inputMax`. `amount` is measured in **normalized destination range**, not native units. For bipolar behavior map −1…1 to 0…1 and use center 0.5; choose amount accordingly. Reset route smoothers to their first valid value on graph activation, not always zero. This avoids an unintended startup sweep. Document whether a held invalid source fades or holds; default audio sources fade to zero, macro bases hold.

The first schema supports additive routes only. Multiplication, arbitrary expression evaluation, cycles and parameter-to-parameter feedback are intentionally excluded. Add them only with explicit versioned semantics. A compiler can pre-resolve strings to indices so the frame loop performs no hash lookup or allocation per route.

### 6.2 Envelopes, LFOs and trigger actions

AD envelopes use the preset’s attack/decay, clamp peak to 0–1, and support restart or max retrigger. For `max`, the new target can raise the current envelope without causing a downward discontinuity; begin the decay from the resulting peak after the attack. Define the decay curve consistently—v1 exponential reaching a small cutoff then zero—and test at different frame rates.

Tempo LFOs derive phase from transport beat position/period. When transport is invalid, follow a declared global policy: hold by default, optional last-tempo free-run. Sample-and-hold uses a seeded PRNG on phase boundary events; it must not draw a random number each video frame.

Trigger actions may start an envelope, advance a palette or reseed a simulation. Trigger-level refractory gates are distinct from detector gates and should not accidentally double the intended limit. Preset switches are control operations, not ordinary route destinations. Quantize scene/palette actions only when requested; leave kick pulses immediate.

### 6.3 Macros and host parameters

Expose **four visible macros** but reserve eight stable automation slots from the first release. Use permanent IDs such as `macro.1`–`macro.8`, not a preset name in the automation identifier. Changing a display label must not break an existing Live set. The four starter labels are Intensity, Motion, Color, Space.

Host automation, local UI and learned CC update the base value through one authority path. Internal modulation does not write the host base every frame. APVTS state and realtime parameter access must follow JUCE’s documented boundaries in the later VST3 wrapper. [[E01]](sources.md#e01)

## 7. Preset schema and worked example

`preset.schema.json` is a Draft 2020-12 structural schema with closed objects and bounded collections. It includes engine requirements, source policy, ordered stages, physical parameter ranges/defaults, macros, modulators, routes, triggers, transitions, finishing controls and performance allocations. `presets/01_pulse_halo.json` is a complete example referencing the original `shaders/pulse_halo.fs`.

The example has one generator and eight parameters. Four macro routes are centered on their default positions so opening the preset preserves the authored base appearance. A kick event drives a 4/180 ms envelope; bass adds modest radius breathing; high activity adds a small glow accent; an eight-beat LFO contributes slow motion. The shader outputs linear HDR color; the engine owns bloom and tone mapping. This division avoids each preset implementing its own inconsistent output conversion.

The validator also checks unique IDs/slots, defaults within bounds, route source/destination references, shader path containment, ISF input names/ranges and trigger envelope targets. JSON Schema alone cannot establish all graph relationships or prove a shader compiles. `VALIDATION.md` records exactly which local checks ran.

**Legacy migration:** recognize the existing unversioned preset form, translate static params and `{source, scale, offset}` mappings to an internal legacy route adapter, preserve the old numerical behavior and warn only about unsupported features. Do not reinterpret an existing native-unit scale as a normalized v2 amount. Save upgraded presets under a new file/ID until the user confirms migration. Missing video inputs must degrade clearly, not silently turn a video preset into a different generator.

## 8. MIDI profile and routing

Use `controller-profile.json` as a **proposed custom user template** for Launch Control XL MK1/MK2, not a factory mapping. Novation documents user templates and separate LED message semantics; consult the programmer guide when implementing feedback. [[E09]](sources.md#e09) [[E13]](sources.md#e13)

| Physical controls | Proposed messages, channel 16 | Assignment |
|---|---|---|
| Top eight knobs | CC20–27 | Stable macros 1–8; first four visible by default |
| Middle eight knobs | CC28–35 | Trim, response, onset sensitivity, attack, release, transition, palette fade, idle motion |
| Bottom eight knobs | CC36–43 | Band gates, refractory controls, bloom, exposure; advanced page |
| Eight faders | CC44–51 | Mix/kick/snare/hat/bass/texture amounts, master intensity, output gain |
| First eight main buttons | Notes36–43 | Preset slots1–8 in the current bank |
| Next eight main buttons | Notes44–51 | Previous, next, user hit, palette advance, freeze, blackout toggle, hold-to-recalibrate, panic blackout |

The remaining hardware buttons are left unassigned by this minimal profile. Controller notes must be tagged as controls, not interpreted as musical kick notes. Separate the controller source from the played-note role source even if both pass through Live. Avoid assigning the same CC via Live MIDI Map and the engine bridge simultaneously.

Default absolute CC pickup deadband is **2/127**, smoothing **30 ms**. After recall, wait for crossing/nearby pickup before applying a physical knob. Relative encoders use an explicit selected encoding; they do not use absolute pickup. Optional high-resolution CC/NRPN parsing is a later tested capability, not silently inferred from 7-bit learn.

Learn records source port/profile, channel, message type and number, with conflict confirmation. Device reconnect matches stable identity where available and asks before binding a different port. LED feedback reflects authoritative base/preset/blackout state, is limited to 30 Hz per control and suppresses echoed input. The XL protocol is not generic RGB; implement its actual color/flag message format. The supplied JSON is not a downloadable vendor SysEx template.

M4L audio tracks remain the analyzer path; a M4L MIDI-effect companion is the straightforward performed-note path. A later VST3 audio effect must expose and qualify its event bus in Live 10, including the separate MIDI-track routing. Do not advertise VST3 “MIDI-only effect” behavior as identical to a native Live MIDI device. [[H01]](sources.md#h01) [[H06]](sources.md#h06) [[H08]](sources.md#h08)

## 9. Rendering pipeline

### 9.1 Context and resources

Keep the current JUCE OpenGL host initially. Isolate resource ownership behind renderer interfaces before changing GLSL dialect. Qualify a 3.3-capable baseline while retaining the legacy compatibility path; do not force a core profile until all old shaders and draw calls have an adapter. Optional 4.3 compute kernels are a later capability.

Ordinary scene/effect targets: **RGBA16F linear**. ISF `FLOAT:true`: **RGBA32F** for the format’s stated precision. Simulation state: RG16F/RG32F as needed. SDR Spout/output target: a receiver-tested RGBA8/BGRA8 format with explicit sRGB transfer handling. ISF input textures, persistent buffers, dynamic pass-size expressions and vertex companions need individual conformance tests. [[G01]](sources.md#g01)

Implement dynamic pass sizes with a small bounded expression evaluator supporting only documented arithmetic/variables, not `eval` or arbitrary scripting. Clamp dimensions to resource limits and reject allocations beyond the per-graph budget. Validate division by zero, negative sizes, NaN and extremely large sizes before touching GL.

Persistent passes use distinct read/write textures and a declared reset policy. Rebuild on resolution changes. The output preview does not force the expensive scene to render twice at two resolutions; normally downsample the main completed output.

### 9.2 Color and post-processing

Decode video/source color consistently. Treat metadata/range/matrix conversion as a tested input step; do not assume every Media Foundation frame is already correct full-range linear RGB. Ordinary sRGB artwork is decoded once. Generate scene colors in linear space, composite/transition there, apply bloom, exposure and a named tone mapper, then display grade/grain and one output transfer conversion.

Do not apply both shader-side sRGB encoding and an enabled sRGB framebuffer conversion to the same output. Test grey ramps and known color patches through both the own-window and Spout receiver. Define straight versus premultiplied alpha at each boundary; the first generative set can use opaque alpha1 to avoid hidden ambiguity.

Blackout/output gain is a final explicit stage. A panic command cancels pending unblackout actions and remains latched across engine restart until the performer releases it. A crashed/hung renderer cannot guarantee delivery of a new black texture; an external receiver/hardware fail-safe is required for that stronger guarantee.

### 9.3 Performance targets

At 1080p60: steady scene plus finish/output GPU P95 **<8 ms** and transition P95 **<12 ms**, measured on the recorded qualification GPU. Record P99 frame interval and missed deadlines as well; do not report only FPS averages. CPU analysis-worker P99 target is **<1 ms per main hop** for the nominated rich source at 48 kHz. These allocations require profiling and can be revised before setting advertised requirements.

Use asynchronous GPU timestamps. No synchronous readback in ordinary output; snapshot capture uses an asynchronous staging/PBO path or is explicitly marked as a diagnostic hitch risk until implemented. Limit two active live graphs during transitions. On mid-fade retarget, capture the current linear composite as a texture, retire the old pair and fade from that snapshot to the new graph. This avoids triple-graph spikes at the cost of freezing the outgoing image during the new transition.

Simulation uses fixed 1/60-second presentation steps with solver substeps selected for numerical stability. Cap catch-up at four steps; after a long stall, resynchronize without an unbounded catch-up loop. Stateless shaders evaluate their own monotonic/beat time directly. The first fallback ladder lowers bloom and simulation resolution, then optional detail, while preserving event timing and macro meaning.

## 10. State recall, process lifecycle and security

Persist a versioned state object with preset ID/content hash, eight macro bases, source-role assignments, response settings, controller profile, output policy and transition preference. Do not save OS process IDs, transient connection tokens or GPU handles as durable identity. Calibrated running statistics are transient by default; persist the normalization policy, not accidentally a room-specific noise floor.

Discover an existing engine by local endpoint/handshake. Launch only from the UI/message path after user intent; never from `processBlock`. Use a bounded startup timeout and an actionable error. Engine close must not stop audio. Plugin editor close must not stop rendering. Bypass or host suspension may stop new audio analysis; release source activity and indicate staleness rather than freezing a high activity value forever.

A restored state is staged, validated and activated atomically. Missing assets retain the current picture or use a named safe fallback with an error; do not partially activate a graph. A content hash mismatch requires confirmation or an explicit version policy. Use a per-session connection token, local binding and asset-root path containment. Network enablement is an advanced option with clearly stated lack of encryption in the initial OSC path.

Offline audio export is not a real-time visual capture guarantee. In v1, keep visual output in real-time mode or explicitly pause it during non-real-time host processing. A later deterministic offline video renderer must consume a recorded feature/event timeline; it should not try to equate audio callback speed with display frame rate.

## 11. Phased roadmap with acceptance gates

| Phase | Small implementation step | Acceptance gate |
|---|---|---|
| 0 — Freeze a trustworthy baseline | Manifest, legacy fixtures, logs, current nine-preset snapshots, source-role test set | Existing behavior reproduced; scene-name/control claims marked tested or untested; one-command diagnostic export |
| 1 — Musical signal contract | Fast envelope, six bands, normalization, transient events, transport epoch; old OSC adapter | Silence never pumps to full activity; repeated gain-offset tracks remain usable; no duplicate event from retransmission; raw dB remains available |
| 2 — Instrument controls | Four/eight stable macros, route runtime, AD/LFO, MIDI bridge and pickup | Example schema loads; 30/60/120 fps parameter trajectories match within tolerance; no double control via MIDI Map + bridge; note pass-through preserved |
| 3 — First lovable visuals | Pulse Halo, Horizon Lines, Mosaic Tiles; calibrated palette/idle behavior | Each works without video and without audio; first four controls remain meaningful at all extremes; 1080p target measured |
| 4 — Finish and transitions | Float linear path, bloom, output conversion, two-graph switching, thumbnail bank | Color ramp passes; 1000 rapid switches without unbounded memory growth; retarget does not exceed two live graphs; compile failure retains last good frame |
| 5 — Show reliability | State transactions, conductor arbitration, hotplug, recovery, prewarm | Two-hour soak; renderer kill/restart leaves audio running; saved set returns exact preset/macros; blackout remains latched; loss/reorder tests pass |
| 6 — VST3 spike, not full rewrite | Thin analyzer/control wrapper, sidechain/event buses, APVTS state | Live 10.1.43: notes, CC, automation, recall, bypass, editor close, two instances, DPI/monitor tests; no renderer dependency on editor |
| 7 — Expand deliberately | Feedback/torus then particles/raymarch/RD/fluid | Each new design has its own performance/numerical stability gate; no regression of first three; ML remains optional research |

Phase 1 calibration fixtures include silence, −60/−40/−20 dBFS noise, sine bursts, per-band transients, a long quiet passage followed by a loud drop, and identical music at 0/−12/−24 dB gain. Evaluate both useful sensitivity and preserved phrase contrast; “all versions look identical” is not the right acceptance criterion for every mode.

Phase 5 fault injection includes 1% packet loss, reordering/duplication, a 500 ms feature outage, blocked output receiver, invalid shader, missing video, monitor disconnect, controller flood and duplicate Live devices. Reliable commands must either apply once or fail visibly. Expired hit events may be dropped, never replayed as a late burst. Measure audio continuity through a recorded null/passthrough test; inspect allocations/locks with appropriate target tools.

## 12. Main risks and de-risking

| Risk | Mitigation and evidence to collect |
|---|---|
| Normalizer erases dynamics or amplifies silence | Raw/absolute/relative separation; minimum range; gate; fast gain reduction/slow gain increase; quiet/drop fixtures |
| Trigger latency feels disconnected | Direct performed MIDI for known roles; short FFT for audio hits; timestamp each stage; measure photon latency distribution |
| CPU/GPU contention affects Live | Bounded off-audio analysis; renderer process isolation; lower quality before catching up; soak with the real set |
| Legacy GLSL fails in a new context | Keep compatibility path; corpus tests and shader adapter before core-profile switch |
| State corruption or competing devices | Single conductor lease, versioned transactions, stable IDs plus collision detection |
| “Spout works” hides receiver/color issues | Same-adapter tests, negotiated texture format, grey/color ramps, receiver reconnect/hotplug tests |
| Copyleft/model/preset licensing surprises | Pin exact dependencies; SBOM with source/weights/assets separated; obtain commercial rights where required |
| Too much scope for a solo developer | Three-preset gate before simulations/plugin migration; reject features without a concrete performance gesture |

## 13. Definition of done for this research package

The included JSON/schema/shader-header relationships can be checked locally with the supplied validator. The report and specification are source-backed design work. **No Windows build, Live/Max device execution, VST3 host test, GPU shader compile, visual benchmark or end-to-end latency measurement was performed in this environment.** Those remain explicit implementation acceptance gates, not implied results.
