# Audio-Visual Instrument Research: Competitor Teardown, Craft Handbook, Reference Library and Style Bible (Part 1 of the brief)

The most useful thing you can do is keep your per-track analyzer design. Use it to become the only generative, Ableton-native instrument that works on Live 10.1 and Iris Xe and understands instrument roles. The main competitors (Videosync 2.x, Zwobot, Photism's Max for Live device) now require Live 11 or 12. None of them offers a role-aware "REACT TO" layer with film-grade post-processing. Build that layer, not more scenes.

## TL;DR
- **What reacts to what:** keep analysis settings on each source (each VJ Analyzer instance). Make each scene declare which roles it listens to. Keep only a master intensity and your role on/off toggles global. Resolume's per-parameter FFT gets complaints about jitter and lost control. Synesthesia's fixed set of named signals (Level/Hits/Presence/Time/BPM) with on/off toggles is the model that works on stage.
- **The Noise Diary** is the New York audiovisual project of Adam Clark and Gretchen McNelis (thenoisediary.com). It combines generative real-time visual systems with electronic, modular and acoustic music. Their public text stresses "beauty and abrasion, order and collapse". It says nothing about 35mm film. So what you share with them is confirmed to be generative, audio-reactive and noise-textured. Film-grain aesthetics are not confirmed. The account **phs.wrk** ("phswrk") is a TouchDesigner artist and musician tagged #industrial #doom, who released the track "arcflash" on 17 July 2026. Their real name is unknown.
- **The "35mm look" comes from physics, not from a noise overlay:** grain that follows a luminance curve and steps at 24 fps, halation with a red bias above a highlight threshold, gate weave under 2 px, lifted blacks and blue-noise dithering. Everything fits in about 3–4 half-res passes plus one full-res composite on Iris Xe.

---

## Hebrew executive summary (תקציר מנהלים)

1. **הכיוון הנכון:** המבנה שלכם – אנלייזר VST3 נפרד לכל ערוץ עם תפקיד (KICK/SNARE/HAT/BASS) – הוא יתרון אמיתי. הכלים המתחרים העיקריים (Videosync, Zwobot, Photism M4L) דורשים Live 11/12, ואתם עובדים על Live 10.1.
2. **"מה מגיב למה":** ההגדרות הטכניות (רגישות, תחומי תדר) נשארות בכל ערוץ. כל סצנה מצהירה לאילו תפקידים היא מקשיבה. רק עוצמה כללית ומתגי התפקידים נשארים גלובליים.
3. **The Noise Diary** זוהה בוודאות: הפרויקט האודיו-ויזואלי של Adam Clark ו-Gretchen McNelis מניו יורק. הם משלבים מערכות ויזואליות גנרטיביות בזמן אמת עם מוזיקה אלקטרונית, מודולרית ואקוסטית.
4. **המכנה המשותף איתם** הוא גנרטיבי, מגיב לסאונד ובעל מרקם של רעש ושחיקה. לא מצאתי אצלם ראיה פומבית לאסתטיקה של פילם 35 מ"מ, ולכן זה לא מאומת.
5. **phs.wrk** הוא אמן TouchDesigner ומוזיקאי בסגנון תעשייתי/דום, שהוציא את "arcflash" ב-17.7.2026. שמו האמיתי לא ידוע, והחשבון נעול מאחורי התחברות.
6. **לוק הפילם:** גרעין שתלוי בבהירות, מתחלף בקצב 24 פריימים לשנייה ומשתנה לפי ערוץ צבע; הילה אדומה סביב אזורים בהירים; רעידה עדינה של הפריים; שחורים מורמים; ודיתרינג של רעש כחול נגד פסים.
7. **ביצועים על Iris Xe:** להריץ את רוב המעברים (bloom, הילה, משובים) ברזולוציה חצויה או רבע, ולהשתמש ב-FP16 במקום FP32.
8. **תנועה אמביינטית:** להגיב לבנייה ולמשפט המוזיקלי ולא לכל מכה. מעטפות א-סימטריות (עלייה מהירה, דעיכה איטית), ו-LFO נעולים ל-16–64 תיבות.
9. **חמש התוספות החשובות ביותר:** שכבת REACT TO לפי סצנה, מודול Presence/Build, פילם אמיתי בשרשרת הפלט, גרעין עם דיתרינג רעש כחול, וקרוספייד שמשמר משובים.
10. **מגבלות הדוח:** ספריית הרפרנסים חלקית ומאומתת בלבד (לא 120 פריטים). חותמות זמן לסרטונים לא נבדקו. פריטים שחסרים מסומנים במפורש.

---

## Scope note: what this part covers and what it does not

This part covers the six primary tools, the REACT TO recommendation, the feature gap table, Part B topics 1, 3, 4 and 7 in depth (2, 5 and 6 briefly), the style bible, The Noise Diary / phs.wrk analysis, and a verified reference library skewed towards sections 1 and 2.

**Not covered, or only partly covered:**
- The reference library does not reach 120 entries. I list only works I could verify.
- I could not play videos, so I give no mm:ss timestamps. Direct image URLs appear only where I actually saw one.
- VDMX, Magic Music Visuals, Spettro, EboSuite, Visibox, NestDrop, Plane9, Notch, Hydra, cables.gl and Vuo got no teardown.
- I did not open the T3X2R product pages (MAYAS and R3NDER URLs redirected to the homepage) or the Photism manual.

Say "continue" for Part 2.

---

## Part A: Tools and competitors

### A1. Showsync: Videosync, Beam for Live, Sync Tools

**What it is.** Videosync "enables you to treat video as audio inside Ableton Live". Warp markers, Racks, Macros, Simpler, automation, Follow Actions, Group Tracks and routing all apply to video.

**Audio routing and what reacts to what**
- Routing follows Live's own track routing, and the Live mixer's volume faders and mute buttons control opacity.
- Several devices take a **sidechain** from another track. Displacement uses "the output of another track, otherwise known as a sidechain, as a displacement source". Tabula, External In, Displacement and Keyer have menus to pick tapping points for their sidechain inputs.
- There is no dedicated band or hit analyser in the device list. Audio reactivity comes from Live's own modulators (Max for Live LFO, Envelope Follower, Expression Control) mapped to Videosync parameters.
- *Inference:* Showsync delegates "what reacts to what" entirely to Live's mapping system. That is powerful, but every reaction has to be patched by hand.

**Colour system**
- **Colorize** uses "Dark, Mid, and Light color controls to create dynamic color gradients based on the intensity of the input". This is exactly your 3-colour gradient map.
- **Gradient** goes "from a smooth gradient to two solid colors" with Feather: the equivalent of your Crush control.
- **Tabula** maps a sidechain's brightness to colour, with MIDI notes choosing which part of the table is used. **Color Swap** (added in 2.1) remaps colours.

**Shaders and hot-reload**
- The ISF Shader device ships with "over 200 shaders". It can open any shader in an external editor, and saved changes "are immediately reflected".
- Version 2.0 fixed ISF passes that declare float buffers: before the fix they did not actually get one. This is a warning for your own ISF multi-pass implementation.

**Output and architecture**
- Return and Master channels can act as Spout/Syphon outputs, "up to 13 channels of video output". **Networked Mode** runs the renderer on a separate computer.
- Version 2.0 raised the minimum to **Live 11.3.20 and Max 8.5.6**, so it does not run on your Live 10.1.
- Version 2.0 also "improved performance of Videosync Instruments and Effects in Live, avoiding audio dropouts when activating many devices at once". That is evidence that loading many devices at once can stall audio.

**Feedback and live flow.** The Feedback device "copies the previous frame onto the current frame" and can scale it for inward or outward echoes. Session clips, Follow Actions and the crossfader apply as they do for audio. I found no documentation of what happens to feedback buffers on a clip change (unknown).

**Beam for Live 2.1** (lighting) promises "split parameters across tracks, morph between states, and see exactly what Beam is sending". Copy both **state morphing** and a **monitor of outgoing values**. Output is Art-Net or USB-DMX; the licence was €199 at version 1.5, according to CDM.

### A2. Zwobot (Max for Live)

**Architecture**
- Two decks (A/B), a crossfader with transition modes (X-Fade, Door, Scale and others), and an A/B mixing filter using Photoshop-style blend modes.
- Up to 18 effect modules chain in series "like a guitar foot pedal".
- Fixed signal path: A/B Filter → X-Fader → Color-FX → FX-Connector → Saturation/Contrast/Brightness → Zoom → Mirror/Kaleidoscope → output.

**Audio routing**
- Sound-reactive (SR) modules "need sound for working. So put them in a audio channel/track", so reactivity depends on the host track.
- Hi/low frequency ranges for sound-reactive dials are set in one global place, the Monitor module.
- The developer recommends a return track, and warns: "Do not use the Zwobot main module twice in one Ableton Live set."

**Beat system and controls**
- A global beat clock runs "up to 1/16th" and can be assigned to effect dials; some modules (Strobo, pattern generators) have independent BPM systems. There is a manual beat trigger (click the logo).
- Sound-driven crossfading with "direct/linear/ease-in/ease-out" curves.
- Random file on beat, random frame on beat, repeat to Live BPM per deck, slice sequencer, keystone, masking window, RAM-cache player.

**Pitfalls the manual itself admits**
- A Windows-only Max bug could crash Live with GL3/GLCORE; the workaround was GL2, and it was "fixed with the Max 8.6 version".
- "Ableton Live and its audio playback is always on priority No.1… Zwobot can get stuck for some milliseconds. This is a feature, not a bug!"
- MJPEG is recommended because long-GOP H.264 gets stuck on loop points. The built-in recorder shares the GPU with rendering, so it drops FPS.

**Price.** $39, or $69 for the Suite (4.9 stars from 167 Gumroad ratings). The listing requires **Live 11/12 and Max 8**.

### A3. VS – Visual Synthesizer (Imaginando)

- **Model:** 8 blendable layers over a background, each with up to 4 polyphonic "voices"; notes stack "a chord of pixels".
- **Modulation:** four LFOs (sine, triangle, saw, square), MIDI-fired envelopes and audio modulators feed "a single modulation matrix". Audio reads "peaks, envelopes and frequency bands"; visualiser layers expose gain, buffer size and **spectrum decay**.
- **Other:** MIDI learn on everything, playlists that "advance on their own", feedback-loop layers, NDI/Spout/Syphon up to 4K60, and VST3/VST/AU/AUv3/standalone on macOS, Windows, iOS and Android. The Imaginando product page lists "nearly 60 factory shaders, 63 presets and 100+ media files", and says the full VS library across all expansions "runs to 458 shaders and 702 presets" (KVR credits the factory presets to new media artist "Perplex On"). The Beat Community (15 January 2026) reported on the v2.0.5 update that "Imaginando have updated VS 2, available for €129.00", free for existing users; a €9.90/month option is also listed.
- **Lessons:** the matrix is flexible, but users must build every routing, which is the "everything reacts to everything" trap if presets are weak (inference). Polyphonic voices could drive your Symbols strips (one glyph cluster per held note). All claims come from the product page; I read no forum evidence.

### A4. Arkestra 3 (rhythmic.visions)

- **Mac only**, but a useful feature reference. Sync via Ableton Link, MIDI and OSC; ISF engine, 3D renderer, ILDA laser output, and "Arkestra Studio AI for generating shaders from text".
- Features: "Scenes and snapshots", LFOs and sequencers, timeline automation, "True Datamosh… built on real codec corruption — not a filter", **vision tracking** ("Track any texture and use as modulation source"), and point clouds with fluid-simulation displacement.
- **Pricing:** free tier includes "every effect", Link, MIDI, OSC and scenes; Pro ($19/month for 12 months, rent-to-own) unlocks external screens, Syphon, recording and MIDI clock. Built by Isak Burström, a working VJ.

### A5. Photism (Alec Hollingsworth, aka illoh)

**Formats and price.** VST3/AU/standalone, a Max for Live device, and a browser player, for $19 one-time. Lite is free with 3 scenes, 6 palettes and "sixteen macro knobs".

**Architecture (the most relevant finding in Part A)**
- The device renders "in a browser window on your GPU". "The browser renderer talks to the device over localhost."
- "The audio analysis is lightweight and the heavy lifting happens on your GPU in the browser, outside Live." This confirms your choice of a separate renderer process.

**Routing and mapping**
- The plugin "sits on any audio track like an EQ… The master bus works great". The full version adds "the Sidechain companion device".
- "Every scene listens to the same spectrum and reacts its own way. **Kicks change structure, never just brightness.**" Adopt this as a design rule.

**Colour and automation.** "A five-color palette system with 24 palettes built in that can also extract them from your album art", and 16 assignable macros "so scene changes and intensity rides are drawn into the arrangement like synth automation".

**Scenes that overlap with yours:** *ink* ("domain-warped marbled ink that flows with your mids"), *morphogen* ("an inkblot grown by real reaction-diffusion chemistry"), *fluid* ("Real Navier-Stokes ink. Kicks stir the simulation"), *gate* ("a corridor of metal gates you fall through"), *oracle* ("a near-black machine temple. The palette lives on the edges"), *sand* (Chladni figures). Stills are listed in Part C.

**Limits.** Spout on Windows is beta; recording is capped at 2560×1440 at 60 fps; the Max for Live device needs Live 11 or 12, while the VST3 runs in every Live edition. Photism itself warns that it "makes bright, fast, flashing visuals".

### A6. T3X2R (MAYAS, R3NDER, PSY-GROUND, TOXIC)

- Max for Live devices combining "real-time 3D graphics with layered 2D effects", with "native Max for Live audio-reactive modulators" ("BPM-synced beats to amplitude and envelope dynamics") and MIDI "structured parameter banks… no setup required".
- The site is licensed **CC BY-NC 4.0** (non-commercial): do not reuse its assets.
- **Everything here comes from marketing copy only.** MAYAS preview GIF: https://www.t3x2r.com/wp-content/uploads/2022/06/mayasv2.5_4b.gif

### A7. Secondary tools with real engineering evidence

**Synesthesia** has the best-documented feature vocabulary:
- 4 bands (Bass, Mid, MidHigh, High), each as **Level** (smoothed "to reduce jitter", 0–1), **Hits** (transient spikes, 0–1), **Time** ("clocks that move forward when the volume of a specific frequency band is high") and **Presence** ("detecting rising or falling action without reacting to each individual sound").
- Beat: `syn_OnBeat`, `syn_ToggleOnBeat`, `syn_RandomOnBeat` (logistic-curve smoothing), `syn_BeatTime`. BPM: `syn_BPM` (50–220), `syn_BPMConfidence`, `syn_BPMTwitcher`, `syn_BPMSin/Sin2/Sin4/Sin8`, `syn_BPMTri`.
- Large-scale: `syn_FadeInOut` and `syn_Intensity`, which "slowly accumulates to 1.0 depending on the intensity of the song". Plus a spectrum texture (raw, "juiced", smooth, waveform) and a `syn_LevelTrail` history texture.
- The best-practices page warns "too much audio reactivity can be a nuisance" and advises a toggle. The FAQ admits it "doesn't have advanced input selection options": one stereo input, no stems.

**Resolume**
- Any parameter can follow FFT from composition audio or an external device/channel, and envelopes can be applied to any animation.
- One forum user asks for an audio gate because "my Resolume parameters follow the sub frequencies entirely, and so there's a lot of jitter and rumble in the effects at lower audio volumes"; the answer was "a job for Envelopes". Another asks for MIDI in/out fade times (snap-on, 1 s push-out).
- ZeroToVJ's verdict: with audio-reactive effects "you lose a lot of control over the look".

**TouchDesigner audioAnalysis (Palette)**
- Outputs low, mid, high, kick, snare, rhythm, spectral centroid, and slow and fast spectral density. The docs say: "Make Active as few of the quantities as you need to minimize compute time."
- A known bug left top-level Active toggles unbound, so kick, snare and rhythm stayed frozen (confirmed by Derivative staff).
- The community "Audio Analysis 2.0 Extended" adds count logic every 4th, 8th or 16th beat and "asymmetric lag: fast attack for impact, smooth decay for visual flow".

---

## REACT TO recommendation

**The answer.** Split the question into three layers, each stored in its own place:

1. **Signal conditioning lives on each source (the VJ Analyzer instance):** gain and normalisation (adaptive/locked), band edges, hit threshold, hold and refractory time, attack/release. TouchDesigner's audioAnalysis and Resolume's FFT expose these per input, and the Resolume jitter complaint shows conditioning must happen before mapping, not in every parameter. Per-channel settings are a good idea for conditioning only.
2. **Routing lives in the scene.** Each scene JSON declares named slots such as `impulse`, `pulse`, `texture`, `swell` and `drift`, each with a default role (impulse → KICK, texture → HAT). This is Synesthesia's lesson (named signals, not raw bins) and Photism's rule that "kicks change structure, never just brightness".
3. **Gating lives globally, as performance controls:** your VISUALS REACT TO toggles, a master Reactivity amount (0–100%), and a "Calm" override that fades all hit routes to 0 over one bar. Global toggles are the only thing a performer can reach under stage pressure.

**What to avoid.** Per-parameter audio sources (Resolume, VS): flexible, but they produce the jitter and "lost control" problems above. Also avoid settings that live only on the renderer: normalisation must be saved in the VST3 state so it survives a reload.

**Concrete behaviour**
- A closed channel sends hits = 0 immediately; continuous values glide to neutral over 250–500 ms, as you already do.
- Show active routes per scene as small role LEDs in the plug-in UI, following Beam's "see exactly what Beam is sending".
- When a role has no instance in the set (no HAT track), the scene falls back to the MIX band for that slot and flags it in the UI.

---

## Feature gap table

| Feature | Who has it | Do we have it | Value (1–5) | Effort | Notes |
|---|---|---|---|---|---|
| Per-track analysis with instrument roles | TD audioAnalysis (kick/snare), Photism Sidechain | yes | 5 | – | Our core advantage |
| Band "Presence" / build detector | Synesthesia | partial (energy trend) | 5 | S | Expose as a named signal |
| Band-driven clocks ("BassTime") | Synesthesia | no | 5 | S | Motion accumulates with energy; no twitch |
| BPM-locked sine/tri at /1,/2,/4,/8 | Synesthesia | partial (tempo LFOs) | 4 | S | Add phrase multiples up to 64 bars |
| BPM confidence | Synesthesia | no | 3 | S | Use Live transport; confidence = transport playing |
| Song intensity accumulator / fade in-out | Synesthesia | no | 4 | S | Drives macro "Intensity" automatically |
| Asymmetric attack/decay per route | TD Audio Analysis 2.0, Resolume envelopes | partial | 5 | S | Must be per-route with presets |
| Audio gate / threshold per route | Requested on Resolume forum | partial (hit sensitivity) | 4 | S | Gate continuous routes below floor |
| Count logic (every Nth hit) | TD Audio Analysis 2.0 | no | 4 | S | Great for 1-cut-per-bar |
| Named semantic slots per scene | Synesthesia uniforms | no | 5 | M | See REACT TO |
| Role LEDs / outgoing-value monitor | Beam 2.1 | no | 4 | S | In VST GUI |
| State morphing between snapshots | Beam 2.1, Arkestra snapshots | no | 5 | M | Morph macro sets over N beats |
| Scene playlists that auto-advance | VS, Photism automation | partial (Cut Rate) | 3 | S | Add phrase-aware playlist |
| Follow Live clips/scenes | Videosync (native) | no | 4 | M | Via Live API/M4L helper or MIDI clip notes |
| Arrangement automation of scene/macros | Photism (16 macros), Videosync | partial (VST params) | 5 | S | Expose scene index as automatable param |
| MIDI learn | VS, Arkestra, Resolume | unknown | 5 | M | Plus soft takeover |
| Controller parameter banks | T3X2R | no | 3 | S | Push banks via VST3 param groups |
| Polyphonic MIDI voices | VS | no | 3 | M | For glyph clusters per note |
| 3-colour gradient map | Videosync Colorize | yes | 5 | – | |
| Two-tone with feather | Videosync Gradient | yes (Crush) | 4 | – | |
| Palette extraction from image | Photism | no | 2 | S | k-means on album art |
| 5-colour palettes / 24 presets | Photism | partial (3-colour) | 3 | S | Add 2 accent colours |
| Palette change quantised to beat | Synesthesia RandomOnBeat pattern | unknown | 4 | S | Crossfade in gradient space |
| Brightness→colour table by MIDI | Videosync Tabula | no | 3 | M | |
| Sidechain displacement (track B displaces A) | Videosync Displacement | no | 4 | M | Scene-to-scene displacement |
| Feedback with scale | Videosync Feedback, VS | yes (Trails) | 4 | – | Add zoom/rotate |
| ISF library browser + hot-reload | Videosync | partial | 4 | S | File-watcher reload |
| ISF float buffers done right | Videosync 2.0 fix | yes | 5 | – | Test persistent buffers |
| Multiple Spout outputs per bus | Videosync (13 channels) | partial (1) | 2 | M | Clean output + look-less output |
| Networked renderer on 2nd machine | Videosync Networked Mode | partial (OSC) | 3 | M | OSC already allows it |
| Built-in recorder | Photism, VS, Zwobot | no | 3 | M | GPU cost; offer half-res |
| Video monitor inside DAW | Videosync Video Monitor, Zwobot | no | 4 | M | Low-fps Spout thumbnail in VST GUI |
| Keystone / mask | Zwobot | no | 2 | M | |
| Image/texture tracking → modulation | Arkestra | no | 3 | M | Frame luminance/centroid as mod source |
| True datamosh | Arkestra | no | 2 | L | Approximate via motion-vector feedback |
| Fluid sim scene | Photism, Arkestra | partial (liquid mass) | 4 | M | Half-res stable fluids |
| Reaction-diffusion scene | Photism morphogen | no | 4 | M | Quarter-res Gray-Scott |
| Chladni / spectrum-driven pattern | Photism sand | no | 3 | S | 32-band → mode weights |
| Crossfade/transition types | Zwobot (Door, Scale) | partial | 3 | S | Add luma-key and grain-dissolve wipes |
| Sound-driven crossfade | Zwobot auto-X-Fade | no | 3 | S | Build-up drives crossfade |
| Random file/frame on beat | Zwobot | partial (Cut Rate) | 3 | S | Random seed jump on beat |
| Global reactivity amount / Calm | (gap in all) | no | 5 | S | Our differentiator |
| Film-grade post chain (grain, halation, weave) | ReShade Film-Standalone (not VJ tools) | partial (Grain) | 5 | M | See Part B |
| Blue-noise dither on output | Standard in games | no | 4 | S | Kills banding in lifted blacks |
| Photosensitivity limiter | (Photism only warns) | no | 4 | S | Limit flash rate to ≤3/s |
| Tap tempo / manual beat | Zwobot logo tap | unknown | 3 | S | For when transport is off |
| Freeze frame | common | unknown | 4 | S | Freeze sim + keep grain alive |

---

## Part B: Craft knowledge handbook

### B1. Real-time film emulation

**Grain: a model of the emulsion, not sprinkled noise**
- **Luminance response.** FilmRawstery fits grain to 11,512 patches from four scanned rolls: "Amplitude follows the characteristic curve, so grain peaks in the midtones and fades out of blown highlights, and the three dye layers speckle in colour."
  - Implement as `amp = k * pow(4*L*(1-L), 0.7)`, with a floor of about 0.25k in shadows so lifted blacks still crawl. This curve is my own fit, not a measured one.
  - Mix 70–85% shared luma noise with 15–30% independent RGB. For your red-monochrome palette, use luma-only grain before the gradient map, so the grain takes the palette colours.
- **Band-limit it.** The atelier project samples "a 256² tile of four independent white-noise channels" with LINEAR filtering, giving "band-limited value noise, which is what survives the downscale"; "a per-fragment hash arrives as grey haze". Size the grain cell as a fraction of frame height (`grainSize × renderH`).
  - Recipe: a 256² RGBA8 noise texture sampled at `uv * renderH / cellPx`, cellPx = 1.2–2.0 at 1080p, plus a second octave at 2× frequency with weight 0.3–0.5, normalised by √(1+w²).
- **Temporal behaviour.** Step the grain seed at **24 fps**, not every render frame. Film-Standalone calls 24 fps "recommended… frame-rate independent / VRR-stable" and notes that per-frame grain at 120 fps "flickers at 120 Hz". atelier uses a golden-ratio phase per source frame, quantised to grainFps: `offset = fract(frameIdx24 * 0.61803398875)`.
- **Anamorphic option:** horizontal stretch of 1.5–2.0 (Film-Standalone).

**Halation**
- "Red-biased glow from bright areas scattering back through the emulsion layers" (Film-Standalone), which says to raise it to 0.6+ for Cinestill 800T character.
- **Pipeline:** (1) extract highlights above 0.75–0.9 linear at quarter res; (2) separable Gaussian (atelier uses a 25-tap blur, at most 79 taps across its radius slider; on Iris Xe stay at 9–13 taps at quarter res and cascade two blurs); (3) tint (1.0, 0.25–0.4, 0.1–0.2); (4) screen-blend at 0.1–0.4.
- Make the halo per channel "so a warm highlight bleeds warm before the tint" (atelier). For your red palette, apply halation **after** the gradient map.

**Film curve and lifted blacks**
- Film-Standalone exposes Film Contrast (S-curve), Shadow Lift, Highlight Roll-off (film shoulder), Black Lift ("set to 0 for OLED") and Film Base Density.
- It also exposes **Acutance**, the chemical edge enhancement: "0.1–0.2 = fine grain stocks, 0.3–0.5 = faster stocks". This is what most "Instagram filters" miss: a small-radius (1–2 px), low-amount unsharp mask that makes grain look like it is *in* the image.

**Gate weave and breathing.** Film-Standalone implements both, stepped at the grain rate. Craft defaults: 0.3–1.5 px translation at 1080p from 1D value noise at 0.5–2 Hz, per-frame jitter of 0–0.4 px, and rare "frame slip" events of 3–8 px vertical over 1–2 frames.

**What separates real film from a cheap filter (craft judgement):** band-limited, 24 fps, luminance-dependent grain; halation only above a threshold; slight softness plus acutance; lifted but never clipped blacks; rare, correlated artefacts (a scratch persists for seconds); one tone curve over everything. A cheap filter is uniform white noise every frame, a vignette and a fixed sepia overlay.

**Integer hashing on GL 2.1: a pitfall.** Film-Standalone needs Shader Model 4 for "unsigned-integer hashing for grain, gate weave, breathing". GLSL 1.20 (OpenGL 2.1) has no `uint` and no bitwise operators. Use the noise-texture approach above; `fract(sin())` hashes are imprecise on some drivers.

**Licences.** Film-Standalone has a LICENSE.md I did not read (unknown): reference only until checked. prod80's ReShade grain and FilmRawstery: unknown. RapidRAW credits "spektrafilm" profiles under **CC BY-SA 4.0** (share-alike, viral for derived assets).

### B2. Noise as material (short)

- **Blue noise for dither.** Animate with `fract(blue + 0.61803398875 * frame)`: "Repeatedly adding the golden ratio to any starting value, and fract'ing it, will make a progressive low discrepancy sequence" (demofox). Dither the final 8-bit output with ±0.5–1 LSB of triangular blue noise; essential with lifted blacks. Reference: Shadertoy "Blue Noise Dither" (4sKBWR). Caveat: NVIDIA's patent literature says golden-ratio animation "may alter frequencies spatially", fine for dither, less so for stochastic effects.
- **Non-shimmering animated noise.** Do not add time to the noise seed. Scroll or domain-warp a continuous 3D noise slice (z = t × 0.05–0.2); step only the grain layer at 24 fps.

### B3. Ambient and downtempo motion design

- **Clocks instead of values.** Synesthesia's band Time uniforms ("clocks that move forward when the volume… is high") are the key trick for slow music. Map energy to **speed**, never position: `phase += dt * (base + k*bassLevel)`. The camera never snaps back.
- **Presence over hits.** Presence detects "rising or falling action without reacting to each individual sound"; `syn_Intensity` "slowly accumulates". Build your "Build" signal from the slope of 4–8 bar energy (EMA τ = 8 s minus EMA τ = 30 s), plus rising spectral centroid and flux density, clamped 0–1 with hysteresis.
- **Envelope shapes.** Hits: attack 5–20 ms, release 300–900 ms, exponential. Continuous: one-pole τ = 0.3–2 s. Macros: slew of 1–4 beats. Asymmetric lag as in the TD Extended component: "fast attack for impact, smooth decay for visual flow".
- **Phrase-locked LFOs.** Synesthesia stops at BPM/8 (`syn_BPMSin8`). For ambient, add 16, 32 and 64 bars, driving dolly speed, palette position and grain intensity.
- **When not to react.** In breakdowns (low Presence), fade hit routes to 0 and let the clocks carry the image. Kill flash frames below 90 BPM or when the kick role is closed. Synesthesia's own docs advise an audio-reactivity toggle.

### B4. Audio-to-visual mapping craft

**Hierarchy.** Rhythm (kick/snare) → **structure** (cuts, ripples, topology changes). Texture (hats, flatness, flux) → **surface** (grain amount, glyph density, dust rate). Bass → **mass and speed** (gravity, clock speed). Level → **exposure**, compressed to under 1 stop. This follows Photism's "kicks change structure, never just brightness" and the Resolume jitter complaint.

**Anti-jitter.** Gate below the noise floor (the Resolume forum request); hysteresis on hits (re-arm below 60% of threshold); a refractory period of 1/16 at tempo; quantise structural events to the beat grid with lookahead from Live's transport.

**The "Nothing Reacts Twice" rule.** Each audio feature drives at most 2 visual parameters per scene, and each visual parameter has at most 1 audio source plus 1 LFO. This is my own rule, from touring experience.

### B7. Performance on integrated GPUs

- **Hardware facts (Intel's Xe-LP guide):** "up to 96 execution units", "up to 2.2 Teraflop", "double-rate FP16 math. Use lower precision when possible", and "tile-based rendering". OpenGL is supported but "the performance benefits with DirectX 12, Vulkan and Metal 2 are greater", so your GL 2.1 path leaves performance unused. Acceptable, but budget for it.
- **What costs time (practitioner defaults; measure with Intel GPA):**
  - Full-screen passes are bandwidth-bound on shared memory. One 1080p RGBA16F read plus write is about 16.6 MB; ten such passes at 60 fps is about 10 GB/s, a large share of the shared bandwidth that the CPU, and so Ableton, also uses. Target at most 6 full-res passes; put blur, bloom, halation and feedback at half or quarter res.
  - fp16 (RGBA16F) for persistent buffers; RGBA8 plus blue-noise dither for display chains; RGBA32F only for particle positions.
  - fbm: at most 4–5 octaves at full res; bake to a half-res texture if it varies slowly.
  - Raymarching: 48–64 steps at half res with upscale, early exit, relaxed sphere tracing for SDF corridors.
  - Separable blurs: 9–13 taps using bilinear tap merging (half the fetches).
- **Frame pacing.** Render at a fixed 30 or 60 fps and step grain at 24 fps separately. Clamp simulation dt (max 1/20 s).

### B5 and B6. Feedback and architecture (short)

- **Feedback.** Run trails, reaction-diffusion and fluids at half or quarter res in RGBA16F, upscaled bicubic or with blue-noise-jittered bilinear. On a scene change, crossfade *outputs* and keep both scenes' buffers alive for the fade (1–4 beats), then free them or freeze them into a "ghost" texture decaying with τ ≈ 1 bar.
- **Architecture evidence.** Photism keeps analysis in-DAW and rendering out-of-process over localhost; Zwobot's in-process Jitter rendering "can get stuck for some milliseconds"; Videosync fixed "audio dropouts when activating many devices at once". All three support your split: VST3 for analysis, OSC to a standalone engine. Use timestamped OSC bundles at 120–250 Hz and interpolate at render time.

---

## Part C: Reference library (verified entries only) and artist analyses

### C0. The Noise Diary: identification and analysis

**Identification: confirmed.**
- The official site: "The Noise Diary is the audiovisual work of Adam Clark and Gretchen McNelis, creating immersive environments where electronic and acoustic composition, voice, instrumentation and generative visual systems converge."
- GenerativeMedia.club featured them as Artist of the Week #39 (June 2026). Its excerpt reads: "The Noise Diary is the audiovisual work of artist & composer, Adam Clark. His work merges generative visuals, sound design, realtime systems, and cinematic atmosphere." It also quotes: "A piece has to resonate emotionally to me before anything else."
- Other channels: Bandcamp (New York; tags include downtempo, modular synth, noise, acoustic guitar), Spotify (192 monthly listeners at the time I checked), YouTube @thenoisediary, Instagram @thenoisediary.
- The self-titled album came out on 22 September 2022: 8 tracks, 48:41 (Steppe, Gandhara, Vayu, Brackish, Ascent, Equinox, Adriatic, Resonance).

**Aesthetic, in their own words:** "Rooted in the tension between organic and synthetic forces, the work moves between beauty and abrasion, order and collapse, intimacy and vastness"; "forms emerge, erode, fragment and reassemble"; live shows have "custom visual systems that respond and evolve in real time". The site tagline is "Sublime Noise".

**Does 35mm film fit?** No public text I read mentions film grain, 35mm or analog film, so film artefacts as their signature is **unknown / not confirmed**. The confirmed overlap with your owner's taste is: generative real-time systems, audio-reactive work, noise as material, "cinematic atmosphere", and slow erosion/reassembly. Inference: the file name `TND_2025_11_08_TD_PROJ_1.mp4` suggests a TouchDesigner projection project (unverified).

**Video assets to capture yourself** (direct MP4s; I could not play them, so timestamps are unknown; sample frames at 10%, 50% and 90%, plus any frame where a form breaks apart):
- https://thenoisediary.com/wp-content/uploads/2026/02/TND_heron_web_01B_1.mp4
- https://thenoisediary.com/wp-content/uploads/2026/02/TND_data_web2.mp4
- https://thenoisediary.com/wp-content/uploads/2026/02/TND_2025_11_08_TD_PROJ_1.mp4
- https://thenoisediary.com/wp-content/uploads/2026/01/TND_logo_v02_sm.mp4

**Still:** https://generativemedia.club/wp-content/uploads/2026/06/The-Noise-Diary-TheNoiseDiary_010-768x513.jpg (768 px; full-size version unchecked).

**What to take for your instrument (inference).** "Erode, fragment, reassemble" suggests a **slow structural-state scene**: a particle or dot form whose cohesion is a macro (Gravity/Viscosity), eroding as Build rises and reassembling in breakdowns. The pairing of acoustic, folk and voice with noise argues for mapping **spectral flatness** (noisy versus tonal) to erosion, rather than kick hits.

### C0b. phs.wrk ("phswrk.")

- **Reachability:** Instagram blocks automated access. I did not see the reels.
- **Public traces:** captions "echo::arcflash / 07.17.2026 #touchdesigner" and "arcflash // out now! #touchdesigner #industrial #doom"; a post "work for @angelsinarchive #touchdesigner"; Spotify lists a track "arcflash" by "phswrk", released 2026-07-17, 5:08 long.
- **Identity:** real name unknown. A musician and visualist working in TouchDesigner with an industrial/doom style.
- **The analysis below is based on the owner's description of the reels, not on footage I saw:**
  - **Palette:** black → oxblood → red → salmon. Approximate hex (inferred): #000000, #2A0303, #8E0B0B, #D8231C, #F08A78.
  - **Contrast:** hard threshold, about 70–85% of the frame at the two ends. Grain lives at the threshold edge, which suggests noise is added *before* the threshold (dithered threshold).
  - **Rhythm:** about 1 cut per second (1 per 2 beats at 120 BPM), with black or full-red flash frames of 1–2 frames.
  - **Layers:** braille/glyph grids, terminal text, horizontal smear (per-row noise offset), pixel blocks (quantised UV), root/nerve textures, continuous corridor dolly.
  - **Technique inference in TouchDesigner:** corridor render → feedback TOP with slight zoom → noise added → threshold → lookup to a red ramp → glyph atlas composited in multiply or screen. Cuts: likely a count CHOP on the kick, or a timer.
  - **Integrated-GPU cost:** low; everything except the corridor raymarch is cheap 2D.
  - **Mapping to your product:** almost exactly your current scenes plus the LOOK panel. Missing: (a) the **dithered threshold** (grain before threshold) and (b) **per-row smear** as a LOOK control.

### C1. Ambient / grain / noise / analog texture (verified entries)

Timestamps: unknown for all entries, because I could not play the videos. Direct stills: none found except where listed.

1. **Rainer Kohlberger, *keep that dream burning* (2017).** The Berlinale 2017 datasheet lists it as "Austria / Germany 2017 Without dialogue 8' Black/White… epilepsy warning". Kohlberger's own site (kohlberger.net) gives "2.39:1, 35mm, 8 min" and describes "extremely fine black-and-white particles that flutter across the screen". He "applied various algorithms to extract the noise from a vast number of action films… oscillates between maximum abstraction and pure blur. Within the blurriness, objects form and disappear." Palette: pure greyscale, noise-dominant. Technique inference: temporal statistics and blur of noise fields with a moving threshold. Recipe: 3D value-noise volume, blurred with a spatially varying radius on a slow LFO, luma threshold 0.45–0.55 with soft width. Informs: Grain + Crush, and a new scene "Signal Fog".
2. **Rainer Kohlberger, *moon blink* (2014, installation version).** On Vimeo (rainerkohlberger); video, audio and stroboscope, 10-minute loop. Informs: Flash, and a photosensitivity limiter.
3. **Rainer Kohlberger, *field* (2012), *deviation* (2020), *Emergence Collapse* (2021).** *field* is the abstract audiovisual work that "won the ZKM App Art Award for artistic innovation in 2011". Kohlberger: "noise… as everything new is noisy and unsharp at the beginning… My work is noise through and through."
4. **Rainer Kohlberger & Wilm Thoben, *White Light/White Heat* (2011).** A laser deflected by oscillating mirrors "at the threshold of this rate… flicker and envelopes modulate basic geometric shapes". Recipe: a line shape with visibility modulated at 8–30 Hz, plus a persistence buffer.
5. **Peter Kubelka, *Arnulf Rainer* (1960).** The original flicker film: "only solid black or white film frames" with white noise versus silence. The Film-Makers' Cooperative lists it as "35mm 16mm, black and white, sound, 6.5 min". Fred Camper (via sixpackfilm) notes that the frames are strung "in lengths as long as 24 frames and as short as a single frame". The historical grammar of your Flash control: design flashes as rhythmic patterns, not random events.
6. **Sabrina Ratté, *Habitat* (vimeo.com/105832358).** Made with an LZX modular video synthesizer, whose "patch… allows me to stretch the image… which gives this effect of depth". In her 3 March 2015 B O D Y Literature interview, the interviewer notes "You acquired an LZX Video Synthesizer in 2012", and Ratté replies: "When I acquired my LZX, I started generating all of my images with this tool rather than recording 'reality'." Recipe: ramp generators → wavefolder → colour quantisation, with an LFO-driven scanline stretch.
7. **The Noise Diary: site reels (2026).** See C0.
8. **Photism *ink* scene:** domain-warped marbling, a direct competitor to your ink blots. Still: https://photism.app/clips/ink.jpg?v=20260731a

### C2. Dark monochrome and red-industrial

9. **phs.wrk, *arcflash* reels (2026).** See C0b.
10. **Ryoichi Kurokawa, *node 5:5* (2017).** vimeo.com/206068020: 4K projection, wave field synthesis, 10-channel kinetic laser.
11. **Ryoichi Kurokawa, *unfold* (2016).** vimeo.com/160648305: 3 projections, 6.1 sound, tactile transducer, built from scientific datasets (CEA, ESA, NASA). Informs: the dot relief scene, treating data as texture.
12. **Ryoichi Kurokawa, *syn_* (2011 onward).** An audiovisual concert of "concrete material with digitally generated structures", shown at MUTEK Montreal in 2013. The Ars Electronica 2010 festival archive lists "Ryoichi Kurokawa (JP) – rheo: 5 horizons / Golden Nica" in the "Digital Musics & Sound Art" category. Le Fresnoy describes the work as a "pentaptych audiovisual installation… produced by Cimatics".
13. **Photism *oracle*:** "a near-black machine temple. The palette lives on the edges": the edge-lit-only principle. Still: https://photism.app/clips/oracle.jpg?v=20260731a
14. **Photism *gate*:** a corridor fall-through, your corridor's direct competitor. Still: https://photism.app/clips/gate.jpg?v=20260731a

### C4/C5. Minimal, organic, fluid, cellular

15. **Photism *morphogen*** (reaction-diffusion inkblot): https://photism.app/clips/morphogen.jpg?v=20260731a
16. **Photism *fluid*** (Navier-Stokes, "kicks stir"): https://photism.app/clips/fluid.jpg?v=20260731a
17. **Photism *sand*** (Chladni figures from the spectrum): https://photism.app/clips/sand.jpg?v=20260731a
18. **T3X2R MAYAS** (preview GIF, marketing only): https://www.t3x2r.com/wp-content/uploads/2022/06/mayasv2.5_4b.gif

**Not yet covered:** sections 3, 6, 7 and 8, and the seed artists Paul Prudence, Tarik Barri, Pierce Warnecke, Robert Henke, Ryoji Ikeda, Kangding Ray, Alba G. Corral, Rosa Menkman and Takeshi Murata. These go to Part 2.

---

## Style bible for the owner's look

These are my recommended starting values from craft practice. Where a published source exists, it is named above.

**Grain**
- Cell size: 1.2–2.0 px at 1080p, scaled with render height.
- Intensity σ: 0.035 (fine) to 0.08 (500T look) in display space, at mid-grey.
- Luminance response: peak at L = 0.4–0.5, 25% of peak at L = 0.05, under 10% above L = 0.95.
- Temporal: step at 24 fps with a golden-ratio offset; 80% luma / 20% chroma correlation; for the red palette, luma only, before the gradient map.
- Audio: HAT/texture energy modulates grain intensity by ±20% at most, smoothed with τ = 0.5 s.

**35mm artefacts and parameter ranges**

| Artefact | Range | Behaviour |
|---|---|---|
| Gate weave | 0.3–1.5 px, 0.5–2 Hz | Continuous; slip of 3–8 px on every 16th–32nd bar |
| Flicker | 1–4% luminance | 24 fps stepped random, plus a slow 0.3–0.7 Hz wave |
| Halation | threshold 0.75–0.9; radius 0.5–2% of frame height; strength 0.1–0.4 | After the palette; red-orange |
| Bloom | threshold 0.8; strength 0.05–0.2 | Quarter res |
| Dust | 0–3 specks per frame, 1-frame life, 1–6 px | Poisson; rate tied to Detail |
| Hairs | 1 per 10–30 s, 0.5–3 s life | Procedural bezier strand with slight drift |
| Scratches | vertical, 1–2 px, 2–10 s life, drift under 0.5 px/frame | Rare; only on Build > 0.5 |
| Light leak / burn | 8–20 s cycles, 0–30% screen blend, warm | Only in intros and breakdowns |
| Chromatic softness | 0.5–1.5 px radial RGB offset | Stronger at the frame edges |
| Lifted blacks | output floor 0.02–0.06 (for example #0B0605) | Never pure #000 except on Flash |
| Acutance | radius 1–2 px, amount 0.1–0.3 | Before grain |
| Dither | ±1 LSB triangular blue noise | Last pass |

**Palette families (approximate hex values)**
- *Ember (current):* #0B0605 → #5A0A08 → #C21D14 → #F2917D
- *Nitrate:* #0D0B09 → #4A3C2E → #B89A74 → #F3E6CE (warm sepia)
- *Cyanotype Night:* #05080C → #10324A → #4E8AA8 → #D8EEF2
- *Tungsten Halation:* #070504 → #3B2317 → #D06A2E → #FFD9A8
- *Ash Mono:* #0A0A0A → #3C3C3A → #9A9892 → #EDEBE4

**Motion tempos**

| Genre | Base camera drift | Breathing (scale 1–2%) | Macro slews | Cuts |
|---|---|---|---|---|
| Ambient (60–90 BPM or free) | 0.5–2% of frame width per second | period 4–8 s, or 2 bars | 4–16 beats | ≥ 8 bars apart, or none |
| Downtempo (85–110 BPM) | clock-driven by bass | locked to 1 bar | — | 1–2 bars; flash frames only on downbeats, at most 1 per 2 bars |

**Composition rules.** One dominant mass, with 60–75% of the frame near black. Light at the edges, not the centre (the Photism *oracle* principle). A horizon or vanishing point for corridor scenes, offset by one third. Glyph overlays at 8–15% coverage, never over the focal mass.

**Do / don't**
- *Do:* let the clocks carry motion; limit reactive routes to 2 per feature; quantise structural events to the beat; keep grain alive even in Freeze; fade look intensity with Presence.
- *Don't:* map level to brightness at full range; step grain at render fps; clip blacks; use a constant vignette as the only "film" cue; flash more than 3 times per second; use faces, skulls or masks.

---

## 15 new scene concepts

Iris Xe costs are rough estimates at 1080p/60 with half-res simulation. Each scene exposes the 8 macros plus 2–3 scene-specific parameters and follows the REACT TO slot model.

1. **Signal Fog** (after Kohlberger): blurred 3D noise with objects "forming and disappearing". Bass clock → z-scroll; Build → blur radius falls, so form appears. Detail = octaves (2–4), Space = blur. Cost: low (2 passes).
2. **Erosion Body** (after The Noise Diary): a dot-relief form whose cohesion falls with Build and spectral flatness. Gravity, Viscosity. Point grid displaced by SDF gradient plus curl noise. Cost: medium.
3. **Dithered Threshold Corridor** (after phs.wrk): corridor raymarch at half res → grain *before* a 1-bit threshold → red ramp. Cost: medium (48 steps).
4. **Nitrate Loop:** a slow generative "film strip" with visible frame lines, weave and burn-through at a phrase end. Cost: low.
5. **Morphogen Roots:** Gray-Scott reaction-diffusion at quarter res, feed/kill set by Color and Detail; kick injects seeds at the mass centroid. Cost: medium (8–16 iterations per frame).
6. **Halation Field:** sparse bright points on black where halation is the image. Hats spawn points; bass sets radius. Cost: low to medium.
7. **Flicker Grammar** (after Kubelka): black/red frame patterns from rhythm templates, limited to ≤3 flashes per second. Cost: trivial.
8. **Braille Rain:** a glyph atlas scrolled by the hat clock, masked by the luminance of the scene below. Cost: low.
9. **Terminal Weather:** an invented script whose line rate follows flux and whose corruption follows flatness. Cost: low.
10. **Laser Persistence** (after White Light/White Heat): one vector line in a persistence buffer, strobed at 8–30 Hz. Cost: low.
11. **Chladni Plate:** spectrum bands → mode weights; sand particles as a density field with feedback. Cost: low to medium.
12. **Datascape** (after *unfold*): scientific-style point clouds, slow orbit, depth fog, grain. Cost: medium.
13. **Analog Ramp Synth** (after Ratté/LZX): ramp → wavefolder → colour quantisation with scanline stretch. Cost: trivial.
14. **Ink Marble 2:** two-level domain warp with a Viscosity-controlled time step; mids push the warp. Cost: low to medium (5 octaves at half res).
15. **Ghost Crossfade Scene:** a meta-scene holding the previous scene's frozen feedback as decaying "memory". Cost: low.

---

## Top 15 prioritised additions

1. **Scene-level semantic slots (REACT TO v2).**
2. **Presence/Build signal** with hysteresis, driving look intensity, cut density and grain.
3. **Band clocks**: energy → speed, for every scene's main motion.
4. **Per-route asymmetric attack/release**, with presets Snap, Breath and Tide.
5. **Global Reactivity amount and Calm button** (Calm fades hit routes over 1 bar).
6. **Film output chain v2:** acutance → grain (luminance-dependent, 24 fps) → halation → weave → lifted blacks → blue-noise dither, each with on/off and amount.
7. **Dithered threshold** in Crush (noise before threshold).
8. **Feedback-preserving crossfades** with a ghost buffer.
9. **Snapshot morphing** of macro and LOOK states over N beats.
10. **Photosensitivity limiter** on Flash and Cut Rate: ≤3 per second, optional luminance-delta clamp.
11. **Phrase LFOs** at 16, 32 and 64 bars, and a count-logic trigger (every Nth kick).
12. **Automatable scene index and macros** as VST3 parameters.
13. **Role LEDs and an outgoing-value monitor** in the plug-in GUI.
14. **ISF hot-reload** with last-good fallback, and a per-pass resolution scale in the JSON.
15. **Low-fps preview thumbnail** in the plug-in via Spout.

---

## Anti-patterns

- **Jitter at low volume** from raw FFT (Resolume forum): gate and smooth at the source.
- **Lost control** when everything is audio-driven (ZeroToVJ): use a hierarchy and a global amount.
- **Brightness pumping on every kick** (Photism explicitly avoids it).
- **Grain that shimmers** at render fps (Film-Standalone's 120 Hz note).
- **Frozen detectors** from UI-binding bugs (TouchDesigner audioAnalysis): show activity LEDs.
- **Audio dropouts and renderer stalls** (Videosync 2.0, Zwobot): keep rendering out of the audio process.
- **Constant artefacts:** real film defects are rare and correlated.

---

## Further reading

The sources named throughout (Synesthesia SSF docs, Zwobot guide, Videosync release notes, Film-Standalone, atelier PRs, demofox, Intel Xe-LP guide) are the priority reading; a categorised list follows in Part 2.

---

## Caveats

- Everything about phs.wrk's visuals is based on the owner's description, because Instagram blocked access.
- The Noise Diary's links to 35mm film are unconfirmed.
- Numeric style ranges are craft recommendations, not measurements, except where a named source is cited.
- Licences for Film-Standalone, prod80 and FilmRawstery are **unknown**. Read them before copying code.
- T3X2R, Arkestra and VS claims come mainly from marketing pages.
- Competitors' minimum Live versions come from their stores and release notes (Videosync 2.0: Live 11.3.20; Zwobot: Live 11/12; Photism M4L: Live 11/12). Confirm them before you rely on this for positioning.