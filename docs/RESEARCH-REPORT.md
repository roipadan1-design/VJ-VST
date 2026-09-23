# Audio-Visual Instrument Research
## Competitor teardown, craft handbook, reference library and style bible

*Prepared 23 September 2026 for a two-person team building a generative VJ instrument for Ableton Live 10.1 on Windows 11 / Intel Iris Xe.*

**Standards used.** URLs are given only for pages actually opened. "Unknown" means not found. Inferences are labelled. Claims taken from marketing copy only are flagged. Licences are flagged where known. Video timestamps could not be verified (videos could not be played), so none are given; capture stills at 10 / 50 / 90 % and at visible structural breaks.

---

## תקציר מנהלים (Hebrew executive summary)

1. **הכיוון הנכון:** המבנה שלכם – אנלייזר VST3 נפרד לכל ערוץ עם תפקיד (KICK/SNARE/HAT/BASS) – הוא יתרון אמיתי. הכלים המתחרים העיקריים (Videosync 2.x, Zwobot, Photism M4L) דורשים Live 11/12, ואתם עובדים על Live 10.1.
2. **"מה מגיב למה":** ההגדרות הטכניות (רגישות, תחומי תדר) נשארות בכל ערוץ. כל סצנה מצהירה לאילו תפקידים היא מקשיבה. רק עוצמה כללית ומתגי התפקידים נשארים גלובליים.
3. **The Noise Diary** זוהה בוודאות: הפרויקט האודיו-ויזואלי של Adam Clark ו-Gretchen McNelis מניו יורק. הם משלבים מערכות ויזואליות גנרטיביות בזמן אמת עם מוזיקה אלקטרונית, מודולרית ואקוסטית.
4. **המכנה המשותף איתם** הוא גנרטיבי, מגיב לסאונד ובעל מרקם של רעש ושחיקה. לא מצאתי אצלם ראיה פומבית לאסתטיקה של פילם 35 מ"מ, ולכן זה לא מאומת.
5. **phs.wrk** הוא אמן TouchDesigner ומוזיקאי בסגנון תעשייתי/דום, שהוציא את "arcflash" ב-17.7.2026. שמו האמיתי לא ידוע, והחשבון נעול מאחורי התחברות.
6. **לוק הפילם:** גרעין שתלוי בבהירות, מתחלף בקצב 24 פריימים לשנייה ומשתנה לפי ערוץ צבע; הילה אדומה סביב אזורים בהירים; רעידה עדינה של הפריים; שחורים מורמים; ודיתרינג של רעש כחול נגד פסים.
7. **ביצועים על Iris Xe:** להריץ את רוב המעברים (bloom, הילה, משובים) ברזולוציה חצויה או רבע, ולהשתמש ב-FP16 במקום FP32.
8. **תנועה אמביינטית:** להגיב לבנייה ולמשפט המוזיקלי ולא לכל מכה. מעטפות א-סימטריות (עלייה מהירה, דעיכה איטית), ו-LFO נעולים ל-16–64 תיבות.
9. **חמש התוספות החשובות ביותר:** שכבת REACT TO לפי סצנה, מודול Presence/Build, פילם אמיתי בשרשרת הפלט, גרעין עם דיתרינג רעש כחול, וקרוספייד שמשמר משובים.
10. **מגבלות הדוח:** ספריית הרפרנסים חלקית ומאומתת בלבד (24 פריטים, לא 120). חותמות זמן לסרטונים לא נבדקו. פריטים שחסרים מסומנים במפורש.

---

## TL;DR

- Keep the per-track analyzer design. Build a role-aware **REACT TO** layer on top of it, not more scenes. Videosync 2.x, Zwobot and Photism's M4L device now require Live 11/12; nothing else in the field runs generative, Ableton-native visuals on Live 10.1 + Iris Xe with instrument roles.
- **What reacts to what:** conditioning per source (VJ Analyzer), routing per scene (named slots), gating global (toggles, master Reactivity, "Calm"). Per-parameter audio sources (Resolume, VS) are what generate the jitter and "lost control" complaints.
- **The Noise Diary** = Adam Clark and Gretchen McNelis, New York (thenoisediary.com). Confirmed overlap: generative, real-time, noise-as-material, "erode, fragment, reassemble". 35mm/film aesthetic: unconfirmed.
- **phs.wrk** = TouchDesigner artist, industrial/doom, track "arcflash" (17 July 2026). Name unknown; reels not viewable. Take from your description: dithered threshold (grain *before* the 1-bit cut) and per-row smear as a LOOK control.
- **The 35mm look comes from physics, not a noise overlay:** luminance-dependent, band-limited grain stepped at 24 fps; halation with red bias above a highlight threshold; gate weave under 2 px; lifted blacks; blue-noise dither. It fits in 3–4 half-res passes plus one full-res composite on Iris Xe.

---

## Scope note

**Covered:** the six primary tools; Synesthesia, Resolume and TouchDesigner; REACT TO recommendation; 50-row feature gap table; Part B topics 1–7; style bible; 15 scene concepts; 15 prioritised additions; anti-patterns; The Noise Diary and phs.wrk analyses; a verified reference library of 24 entries.

**Not covered or partial:** VDMX, Magic Music Visuals, Spettro, EboSuite, Visibox, NestDrop, Plane9, Notch, Hydra, cables.gl and Vuo (short unverified notes only). T3X2R product sub-pages and the Photism manual were not opened. Reference library does not reach 120 entries; seed artists Tarik Barri, Robert Henke, Kangding Ray, Alba G. Corral and Rosa Menkman are unverified leads. No video timestamps.

---

# Part A: Tools and competitors

## A1. Showsync: Videosync, Beam for Live, Sync Tools

*Sources opened: https://www.showsync.com/videosync/ , https://videosync.showsync.com/ , https://videosync.showsync.com/compare-editions , https://support.showsync.com/release-notes/videosync/2.0 , https://support.showsync.com/release-notes/videosync/1.1 , https://beam.showsync.com/ , https://cdm.link/showsyncs-beam-1-5-lights-in-ableton-live/*

**What it is.** Videosync "enables you to treat video as audio inside Ableton Live": warp markers, Racks, Macros, Simpler, automation, Follow Actions, Group Tracks and routing all apply to video.

**Audio routing / what reacts to what**
- Routing follows Live's own track routing; the Live mixer's faders and mutes control opacity.
- Several devices take a **sidechain** from another track (Displacement, Tabula, External In, Keyer) with menus to pick tapping points.
- No dedicated band/hit analyser in the device list; reactivity comes from Live's own modulators (M4L LFO, Envelope Follower, Expression Control) mapped to Videosync parameters. *Inference:* "what reacts to what" is delegated entirely to Live's mapping system: powerful, but every reaction is patched by hand.

**Colour system**
- **Colorize**: Dark / Mid / Light colour controls producing gradients from input intensity (your 3-colour gradient map).
- **Gradient**: from smooth gradient to two solid colours with Feather (your Crush).
- **Tabula**: maps a sidechain's brightness to colour; MIDI notes select the table region. **Color Swap** (2.1) remaps colours.

**Shaders / hot-reload.** ISF Shader device ships "over 200 shaders"; any shader can be opened in an external editor and saved changes are reflected immediately. Version 2.0 fixed ISF passes declaring float buffers that previously did not get one — a warning for your own ISF multi-pass code.

**Output / architecture.** Return and Master channels can be Spout/Syphon outputs ("up to 13 channels"). **Networked Mode** runs the renderer on a second computer. Version 2.0 raised the minimum to **Live 11.3.20 and Max 8.5.6** (does not run on Live 10.1). 2.0 also "improved performance … avoiding audio dropouts when activating many devices at once" — evidence that in-process loading can stall audio.

**Feedback / live flow.** Feedback device "copies the previous frame onto the current frame" with scaling for inward/outward echoes. Session clips, Follow Actions and the crossfader behave as for audio. Behaviour of feedback buffers on clip change: **unknown**.

**Beam for Live 2.1** (lighting): "split parameters across tracks, morph between states, and see exactly what Beam is sending". Copy **state morphing** and an **outgoing-value monitor**. Art-Net/USB-DMX output; €199 at v1.5 per CDM.

## A2. Zwobot (Max for Live)

*Sources: https://www.zwobotmax.com/manual/ , https://zwobot.gumroad.com/l/nqkxh , https://zwobot.gumroad.com/l/zwobotsuite*

- **Architecture:** two decks (A/B), crossfader with transition modes (X-Fade, Door, Scale …), Photoshop-style A/B blend filter, up to 18 serial effect modules "like a guitar foot pedal". Fixed path: A/B Filter → X-Fader → Color-FX → FX-Connector → Sat/Contrast/Brightness → Zoom → Mirror/Kaleidoscope → out.
- **Audio routing:** sound-reactive modules "need sound for working. So put them in a audio channel/track" — reactivity depends on the host track. Hi/low ranges for SR dials are set globally in the Monitor module. Recommended on a return track; "Do not use the Zwobot main module twice in one Ableton Live set."
- **Beat system / controls:** global beat clock "up to 1/16th" assignable to dials; some modules (Strobo, pattern generators) have independent BPM; manual beat trigger (click logo); sound-driven crossfade with direct/linear/ease-in/ease-out; random file/frame on beat; repeat-to-Live-BPM per deck; slice sequencer; keystone; mask; RAM-cache player.
- **Admitted pitfalls:** a Windows-only Max bug could crash Live with GL3/GLCORE (workaround GL2, "fixed with the Max 8.6 version"); "Ableton Live and its audio playback is always on priority No.1 … Zwobot can get stuck for some milliseconds. This is a feature, not a bug!"; MJPEG recommended because long-GOP H.264 sticks on loop points; built-in recorder shares the GPU and drops FPS.
- **Price:** $39 / $69 Suite (4.9★, 167 Gumroad ratings). Requires **Live 11/12 and Max 8**.

## A3. VS – Visual Synthesizer (Imaginando)

*Source: https://www.imaginando.pt/products/vs-visual-synthesizer (product page only — no forum evidence read)*

- 8 blendable layers over a background, each with up to 4 polyphonic "voices" (notes stack "a chord of pixels").
- Four LFOs (sine/tri/saw/square), MIDI-fired envelopes and audio modulators feed "a single modulation matrix". Audio reads "peaks, envelopes and frequency bands"; visualiser layers expose gain, buffer size and **spectrum decay**.
- MIDI learn on everything; playlists that auto-advance; feedback-loop layers; NDI/Spout/Syphon to 4K60; VST3/VST/AU/AUv3/standalone on macOS, Windows, iOS, Android. ~60 factory shaders, 63 presets, 100+ media; full library across expansions 458 shaders / 702 presets. v2.0.5 (Jan 2026) €129 or €9.90/month.
- **Lesson (inference):** the matrix is flexible but every routing is user-built — the "everything reacts to everything" trap if presets are weak. Polyphonic voices could drive your Symbols strips (one glyph cluster per held note).

## A4. Arkestra 3 (rhythmic.visions)

*Source: https://www.arkestra.app/ (marketing copy)*

- **Mac only**, but a feature reference. Ableton Link, MIDI, OSC; ISF engine; 3D renderer; ILDA laser; "Arkestra Studio AI for generating shaders from text".
- "Scenes and snapshots", LFOs and sequencers, timeline automation, "True Datamosh … built on real codec corruption", **vision tracking** ("Track any texture and use as modulation source"), point clouds with fluid-sim displacement.
- Free tier: every effect, Link, MIDI, OSC, scenes. Pro $19/month × 12 rent-to-own: external screens, Syphon, recording, MIDI clock. Built by Isak Burström, a working VJ.

## A5. Photism (Alec Hollingsworth / illoh)

*Sources: https://photism.app/ , https://photism.app/scenes/ , https://photism.app/buy/ , https://photism.app/lite/ , https://photism.app/learn/best-ableton-visualizers/*

- **Formats/price:** VST3/AU/standalone, M4L device, browser player; $19 one-time. Lite free (3 scenes, 6 palettes, 16 macro knobs).
- **Architecture (most relevant finding in Part A):** the device renders "in a browser window on your GPU"; "the browser renderer talks to the device over localhost"; "audio analysis is lightweight and the heavy lifting happens on your GPU in the browser, outside Live." Confirms your separate-renderer choice.
- **Routing/mapping:** plugin "sits on any audio track like an EQ … The master bus works great"; full version adds a Sidechain companion device. Design rule to adopt: "Every scene listens to the same spectrum and reacts its own way. **Kicks change structure, never just brightness.**"
- **Colour/automation:** five-colour palettes, 24 built in, extractable from album art; 16 macros automatable in the arrangement "like synth automation".
- **Overlapping scenes:** *ink* (domain-warped marbling), *morphogen* (real reaction-diffusion inkblot), *fluid* (Navier-Stokes; "kicks stir"), *gate* (corridor of gates you fall through), *oracle* ("near-black machine temple. The palette lives on the edges"), *sand* (Chladni). Stills: https://photism.app/clips/ink.jpg?v=20260731a , morphogen.jpg , fluid.jpg , gate.jpg , oracle.jpg , sand.jpg (same path pattern).
- **Limits:** Spout on Windows beta; recording capped at 2560×1440@60; M4L device needs Live 11/12 (VST3 runs in every edition). Photism warns it "makes bright, fast, flashing visuals".

## A6. T3X2R (MAYAS, R3NDER, PSY-GROUND, TOXIC)

*Source: https://www.t3x2r.com/ — homepage only; product sub-pages redirected. **Marketing copy only.***

- M4L devices combining "real-time 3D graphics with layered 2D effects", "native Max for Live audio-reactive modulators" (BPM-synced beats, amplitude, envelope) and MIDI "structured parameter banks … no setup required".
- Site licensed **CC BY-NC 4.0** — do not reuse assets. MAYAS preview GIF: https://www.t3x2r.com/wp-content/uploads/2022/06/mayasv2.5_4b.gif

## A7. Secondary tools with real engineering evidence

**Synesthesia** — *https://app.synesthesia.live/docs/ssf/audio_uniforms.html , https://app.synesthesia.live/docs/ssf/best_practices.html , https://production.synesthesia.live/faq/*
- 4 bands (Bass, Mid, MidHigh, High), each as **Level** (smoothed, 0–1), **Hits** (transient spikes), **Time** ("clocks that move forward when the volume of a specific frequency band is high") and **Presence** ("detecting rising or falling action without reacting to each individual sound").
- Beat: `syn_OnBeat`, `syn_ToggleOnBeat`, `syn_RandomOnBeat`, `syn_BeatTime`. BPM: `syn_BPM` (50–220), `syn_BPMConfidence`, `syn_BPMTwitcher`, `syn_BPMSin/Sin2/Sin4/Sin8`, `syn_BPMTri`.
- Large-scale: `syn_FadeInOut`, `syn_Intensity` ("slowly accumulates to 1.0"), spectrum textures, `syn_LevelTrail` history texture.
- Best-practices: "too much audio reactivity can be a nuisance" — add a toggle. FAQ: no advanced input selection; one stereo input, no stems.

**Resolume** — *https://resolume.com/support/en/7/parameter-animation , https://resolume.com/forum/viewtopic.php?t=21657 , https://resolume.com/forum/viewtopic.php?t=17270 , https://zerotovj.com/should-you-use-audio-reactive-visuals/*
- Any parameter can follow FFT from composition audio or external device/channel; envelopes apply to any animation.
- Forum: "my Resolume parameters follow the sub frequencies entirely, and so there's a lot of jitter and rumble in the effects at lower audio volumes" (answer: envelopes). Another thread requests MIDI fade in/out times. ZeroToVJ: with audio-reactive effects "you lose a lot of control over the look".

**TouchDesigner audioAnalysis** — *https://derivative.ca/UserGuide/Palette:audioAnalysis , https://forum.derivative.ca/t/audio-analysis-not-detecting-kick-snare-and-rhythm/293931 , https://derivative.ca/community-post/asset/audio-analysis-20-extended-audio-reactive-components/73946 , https://interactiveimmersive.io/blog/touchdesigner-lessons/how-to-make-anything-audio-reactive-in-touchdesigner/*
- Outputs low/mid/high, kick, snare, rhythm, spectral centroid, slow and fast spectral density; docs: "Make Active as few of the quantities as you need to minimize compute time."
- A known bug left top-level Active toggles unbound so kick/snare/rhythm stayed frozen (confirmed by Derivative staff).
- Community "Audio Analysis 2.0 Extended" adds count logic every 4th/8th/16th beat and "asymmetric lag: fast attack for impact, smooth decay for visual flow".

**Unverified notes on the rest (no pages opened; treat as leads):** VDMX (Mac; per-receiver attack/release, closest to "conditioning at the receiver"); Notch (heavy GPU; reference for its Film Grain/Halation nodes); Hydra and cables.gl (browser; per-op analyser smoothing, same trap as Resolume); Vuo (Mac); MilkDrop/projectM (adaptive-threshold beat detect, LGPL — read `beatDetect.cpp`); NestDrop (Windows MilkDrop + Spout; sensitivity and preset-switch-on-beat ≈ your Cut Rate); Magic Music Visuals (per-node smoothing); EboSuite (Mac, ISF); Plane9 (Windows; playlist/auto-pilot); Spettro, Visibox (simple visualisers).

---

## REACT TO recommendation

Split "what reacts to what" into three layers, each stored in its own place:

1. **Signal conditioning lives on each source (VJ Analyzer instance):** gain and normalisation (adaptive/locked), band edges, hit threshold, hold, refractory time, attack/release. TD audioAnalysis and Resolume FFT expose these per input; the Resolume jitter complaint shows conditioning must happen *before* mapping. **Per-channel settings are a good idea for conditioning only.**
2. **Routing lives in the scene.** Each scene JSON declares named slots — `impulse`, `pulse`, `texture`, `swell`, `drift` — each with a default role (impulse → KICK, texture → HAT). Synesthesia's lesson (named signals, not raw bins); Photism's rule (kicks change structure, never just brightness).
3. **Gating lives globally as performance controls:** your VISUALS REACT TO toggles, a master Reactivity amount (0–100 %), and a **Calm** override that fades all hit routes to 0 over one bar. Global toggles are the only thing reachable under stage pressure.

**Avoid:** per-parameter audio sources (Resolume, VS); settings that live only on the renderer (normalisation must be in VST3 state).

**Behaviour:** closed channel → hits = 0 immediately, continuous values glide to neutral over 250–500 ms. Show active routes per scene as role LEDs in the plug-in (Beam's "see exactly what Beam is sending"). If a role has no instance in the set, fall back to MIX for that slot and flag it.

---

## Feature gap table

| Feature | Who has it | Do we have it | Value (1–5) | Effort | Notes |
|---|---|---|---|---|---|
| Per-track analysis with instrument roles | TD audioAnalysis, Photism Sidechain | yes | 5 | – | Core advantage |
| Band "Presence" / build detector | Synesthesia | partial (energy trend) | 5 | S | Expose as named signal |
| Band-driven clocks ("BassTime") | Synesthesia | no | 5 | S | Energy → speed, no twitch |
| BPM-locked sine/tri at /1,/2,/4,/8 | Synesthesia | partial (tempo LFOs) | 4 | S | Add phrase multiples to 64 bars |
| BPM confidence | Synesthesia | no | 3 | S | Use Live transport |
| Song intensity accumulator / fade in-out | Synesthesia | no | 4 | S | Drives Intensity macro |
| Asymmetric attack/decay per route | TD AA 2.0, Resolume envelopes | partial | 5 | S | Per-route with presets |
| Audio gate / threshold per route | Requested on Resolume forum | partial | 4 | S | Gate continuous routes |
| Count logic (every Nth hit) | TD AA 2.0 | no | 4 | S | 1-cut-per-bar |
| Named semantic slots per scene | Synesthesia uniforms | no | 5 | M | See REACT TO |
| Role LEDs / outgoing-value monitor | Beam 2.1 | no | 4 | S | In VST GUI |
| State morphing between snapshots | Beam 2.1, Arkestra | no | 5 | M | Morph macro sets over N beats |
| Scene playlists that auto-advance | VS, Photism | partial (Cut Rate) | 3 | S | Phrase-aware playlist |
| Follow Live clips/scenes | Videosync | no | 4 | M | Live API/M4L helper or MIDI clip notes |
| Arrangement automation of scene/macros | Photism, Videosync | partial | 5 | S | Scene index as VST param |
| MIDI learn | VS, Arkestra, Resolume | unknown | 5 | M | Plus soft takeover |
| Controller parameter banks | T3X2R | no | 3 | S | VST3 param groups |
| Polyphonic MIDI voices | VS | no | 3 | M | Glyph clusters per note |
| 3-colour gradient map | Videosync Colorize | yes | 5 | – | |
| Two-tone with feather | Videosync Gradient | yes (Crush) | 4 | – | |
| Palette extraction from image | Photism | no | 2 | S | k-means |
| 5-colour palettes / 24 presets | Photism | partial | 3 | S | Add 2 accents |
| Palette change quantised to beat | Synesthesia pattern | unknown | 4 | S | Crossfade in gradient space |
| Brightness→colour table by MIDI | Videosync Tabula | no | 3 | M | |
| Sidechain displacement | Videosync Displacement | no | 4 | M | Scene-to-scene displacement |
| Feedback with scale | Videosync Feedback, VS | yes (Trails) | 4 | – | Add zoom/rotate |
| ISF library browser + hot-reload | Videosync | partial | 4 | S | File-watcher |
| ISF float buffers done right | Videosync 2.0 fix | yes | 5 | – | Test persistent buffers |
| Multiple Spout outputs | Videosync (13) | partial (1) | 2 | M | Clean + look-less output |
| Networked renderer | Videosync | partial (OSC) | 3 | M | |
| Built-in recorder | Photism, VS, Zwobot | no | 3 | M | Offer half-res |
| Video monitor inside DAW | Videosync, Zwobot | no | 4 | M | Low-fps Spout thumbnail |
| Keystone / mask | Zwobot | no | 2 | M | |
| Texture tracking → modulation | Arkestra | no | 3 | M | Frame luma/centroid as mod source |
| True datamosh | Arkestra | no | 2 | L | Motion-vector feedback approximation |
| Fluid sim scene | Photism, Arkestra | partial | 4 | M | Half-res stable fluids |
| Reaction-diffusion scene | Photism morphogen | no | 4 | M | Quarter-res Gray-Scott |
| Chladni / spectrum pattern | Photism sand | no | 3 | S | 32-band → mode weights |
| Crossfade/transition types | Zwobot | partial | 3 | S | Luma-key, grain-dissolve |
| Sound-driven crossfade | Zwobot | no | 3 | S | Build drives crossfade |
| Random seed/frame on beat | Zwobot | partial | 3 | S | Seed jump on beat |
| Global reactivity amount / Calm | (gap in all) | no | 5 | S | Differentiator |
| Film-grade post chain | Film-Standalone (not a VJ tool) | partial (Grain) | 5 | M | See Part B |
| Blue-noise dither on output | Standard in games | no | 4 | S | Kills banding |
| Photosensitivity limiter | (Photism only warns) | no | 4 | S | ≤3 flashes/s |
| Tap tempo / manual beat | Zwobot | unknown | 3 | S | When transport is off |
| Freeze frame | common | unknown | 4 | S | Freeze sim, keep grain alive |

---

# Part B: Craft knowledge handbook

## B1. Real-time film emulation

*Sources: https://github.com/artzox/Film-Standalone , https://github.com/CostardRouge/atelier/pull/122 , https://github.com/CostardRouge/atelier/pull/144 , https://github.com/lim8701/FilmRawstery , https://vmoldo.com/film-grain-emulation-2025-guide/ , https://github.com/CyberTimon/RapidRAW/releases/tag/v1.6.2*

**Grain: a model of the emulsion, not sprinkled noise**
- **Luminance response.** FilmRawstery fits grain to 11,512 patches from four scanned rolls: "Amplitude follows the characteristic curve, so grain peaks in the midtones and fades out of blown highlights, and the three dye layers speckle in colour." Implement as `amp = k * pow(4*L*(1-L), 0.7)` with a floor of ~0.25k in shadows (my fit, not measured). Mix 70–85 % shared luma noise with 15–30 % independent RGB. For red-monochrome, use luma-only grain *before* the gradient map.
- **Band-limit it.** atelier samples "a 256² tile of four independent white-noise channels" with LINEAR filtering → "band-limited value noise, which is what survives the downscale"; a per-fragment hash "arrives as grey haze". Recipe: 256² RGBA8 noise texture sampled at `uv * renderH / cellPx`, cellPx 1.2–2.0 at 1080p, plus a second octave at 2× frequency, weight 0.3–0.5, normalised by √(1+w²).
- **Temporal.** Step the grain seed at **24 fps**. Film-Standalone: 24 fps "recommended … frame-rate independent / VRR-stable"; per-frame grain at 120 fps "flickers at 120 Hz". atelier: golden-ratio phase per source frame, `offset = fract(frameIdx24 * 0.61803398875)`.
- **Anamorphic option:** horizontal stretch 1.5–2.0 (Film-Standalone).

**Halation** — "Red-biased glow from bright areas scattering back through the emulsion layers" (Film-Standalone; 0.6+ for Cinestill 800T). Pipeline: extract highlights > 0.75–0.9 linear at quarter res → separable Gaussian (9–13 taps at quarter res, cascade two) → tint (1.0, 0.25–0.4, 0.1–0.2) → screen at 0.1–0.4. Per-channel halo "so a warm highlight bleeds warm before the tint" (atelier). For the red palette, apply halation **after** the gradient map.

**Film curve and lifted blacks** — Film-Standalone exposes Film Contrast (S-curve), Shadow Lift, Highlight Roll-off, Black Lift ("set to 0 for OLED"), Film Base Density, and **Acutance** ("0.1–0.2 = fine grain stocks, 0.3–0.5 = faster stocks"): a 1–2 px, low-amount unsharp mask that makes grain look *in* the image. This is what cheap filters miss.

**Gate weave and breathing** — both in Film-Standalone, stepped at grain rate. Defaults: 0.3–1.5 px translation at 1080p from 1D value noise at 0.5–2 Hz; per-frame jitter 0–0.4 px; rare "frame slip" 3–8 px vertical over 1–2 frames.

**Real film vs cheap filter (craft judgement):** band-limited, 24 fps, luminance-dependent grain; halation only above threshold; slight softness plus acutance; lifted but never clipped blacks; rare, correlated artefacts; one tone curve over everything. A cheap filter is uniform white noise every frame, a vignette and a fixed sepia overlay.

**GL 2.1 pitfall.** Film-Standalone needs SM4 for unsigned-integer hashing. GLSL 1.20 has no `uint` or bitwise ops — use the noise-texture approach; `fract(sin())` hashes are imprecise on some drivers.

**Licences:** Film-Standalone LICENSE.md not read (unknown). prod80 ReShade grain and FilmRawstery: unknown. RapidRAW credits "spektrafilm" profiles under **CC BY-SA 4.0** (share-alike, viral for derived assets). Reference only until checked.

## B2. Noise as material

*Sources: https://blog.demofox.org/2020/05/10/ray-marching-fog-with-blue-noise/ , https://www.shadertoy.com/view/4sKBWR , https://image-ppubs.uspto.gov/dirsearch-public/print/downloadPdf/12283028*

- **White vs blue.** White noise (per-pixel hash) has energy at all frequencies, reads as "digital sand" and aliases when downscaled. Blue noise has no low-frequency clumps: use it for dithering, stochastic sampling and dust placement. Grain wants *band-limited* value noise (B1). Ship three textures: 256² white RGBA8 (grain), 64²/128² blue (dither/jitter), 256² 3-octave simplex (domain warp). Generate offline in Python; nothing needs `uint`.
- **Blue noise animation.** `fract(blue + 0.61803398875 * frame)`: "Repeatedly adding the golden ratio to any starting value, and fract'ing it, will make a progressive low discrepancy sequence" (demofox). Dither the final 8-bit output with ±0.5–1 LSB triangular blue noise — essential with lifted blacks. Caveat (NVIDIA patent literature): golden-ratio animation "may alter frequencies spatially" — fine for dither, less so for stochastic effects.
- **Animated noise that doesn't shimmer.** Shimmer = frame-to-frame decorrelation. Cures, cheapest first: (1) scroll the tile slowly (`uv + t*vec2(0.013,0.007)`); (2) blend two tiles with a triangle-wave crossfade over 2–4 s; (3) sample a 3D volume with `z = t*0.05–0.2` (4–5× the cost of 2D). Rule: only the *grain* layer may decorrelate every frame, and it does so at 24 fps.
- **Static and analog-video noise.** Snow = white noise convolved horizontally: `hash(y,t)` row noise plus `hash(x*0.25,y,t)` streak. VHS: chroma at quarter horizontal res delayed 1–2 px right, luma sharp; head-switch band 8–16 rows at bottom with offset noise; tracking errors as per-row horizontal offset from 1D noise at 0.5–3 Hz, amplitude 0–12 px, gated into bursts; dropouts as sparse white dashes 20–120 px, 1 row, 1-frame life. This is the phs.wrk "horizontal smear" control: amplitude by Glitch, burst rate by flux. Cost trivial.
- **Domain warping.** `p' = p + k1*fbm(p + k2*fbm(p))`; k1 0.3–1.2 UV, k2 1–4; 3 inner / 3–4 outer octaves. Time-offset inner and outer at different rates (inner z = t·0.03, outer z = t·0.11) for slow structure with faster surface detail; drive the outer offset with the bass *clock*, not level. Run at half res (arithmetic-bound, ~4× back).

## B3. Ambient and downtempo motion design

- **Clocks instead of values.** Synesthesia band Time ("clocks that move forward when the volume … is high"). Map energy to **speed**, never position: `phase += dt*(base + k*bassLevel)`. The camera never snaps back.
- **Presence over hits.** Presence detects "rising or falling action without reacting to each individual sound"; `syn_Intensity` "slowly accumulates". Build signal = slope of 4–8-bar energy (EMA τ=8 s minus EMA τ=30 s) + rising centroid + flux density, clamped 0–1 with hysteresis.
- **Envelopes.** Hits: attack 5–20 ms, release 300–900 ms, exponential. Continuous: one-pole τ 0.3–2 s. Macros: slew 1–4 beats. Asymmetric lag as in TD Extended: "fast attack for impact, smooth decay for visual flow".
- **Phrase LFOs.** Synesthesia stops at BPM/8; add 16, 32, 64 bars for dolly speed, palette position, grain intensity.
- **When not to react.** In breakdowns (low Presence) fade hit routes to 0 and let clocks carry the image. Kill flash frames below 90 BPM or when KICK is closed. Synesthesia's docs advise a reactivity toggle.

## B4. Audio-to-visual mapping craft

- **Hierarchy.** Rhythm (kick/snare) → **structure** (cuts, ripples, topology). Texture (hats, flatness, flux) → **surface** (grain, glyph density, dust rate). Bass → **mass and speed** (gravity, clock speed). Level → **exposure**, compressed to < 1 stop.
- **Anti-jitter.** Gate below the noise floor; hysteresis on hits (re-arm below 60 % of threshold); refractory period of 1/16 at tempo; quantise structural events to the beat grid with lookahead from Live's transport.
- **"Nothing Reacts Twice" (my rule):** each audio feature drives ≤ 2 visual parameters per scene; each visual parameter has ≤ 1 audio source + 1 LFO.

## B5. Feedback systems on an integrated GPU

- **Trails.** Ping-pong RGBA16F at half res. `max(scene, prev*decay)` = phosphor; `mix(scene, prev, decay)` = smear. Decay 0.85–0.97/frame at 60 fps (τ 0.1–0.5 s). Zoom/rotate the resample (scale 0.995–1.01, ±0.1°/frame). Subtract 0.002 before clamping or fp16 never returns to black. Blue-noise-jitter the resample UV ±0.5 px.
- **Reaction-diffusion (Gray-Scott).** Quarter res, RG16F, 8–16 iterations/frame. Laplacian kernel centre −1, orthogonal 0.2, diagonal 0.05. Families: coral f=0.055 k=0.062; mitosis f=0.0367 k=0.0649; worms f=0.078 k=0.061; fingerprint f=0.037 k=0.06. One 2D macro (Color→f, Detail→k) constrained to the stable band. Kick injects B at the mass centroid, 6–12 px Gaussian.
- **Stable fluids.** Stam-style at quarter res: advect velocity → force (kick impulse, curl noise) → divergence → 20–30 Jacobi → gradient subtract → advect dye. Vorticity confinement ε 0.2–0.5. Single-channel dye, gradient-mapped afterward.
- **Particles on GL 2.1.** Positions in an RGBA32F texture (the one legitimate fp32 target), updated in a fragment pass, drawn with vertex texture fetch. 65k GL_POINTS additive is comfortable; keep sprites < 8 px and let bloom do the size.
- **Scene change.** Keep both scenes alive for the fade, then freeze the outgoing feedback into a "ghost" texture decaying with τ ≈ 1 bar, screened under the incoming scene. Reset sim buffers only on explicit Reset or after a full-black Flash.

## B6. Architecture lessons

- **Process split.** Zwobot and Videosync (in-process) admit stalls/dropouts; Photism renders out of process over localhost. Your VST3 + standalone is right. Send OSC as timestamped bundles at 120 Hz from a lock-free FIFO to a sender thread — never from `processBlock`. Engine keeps a 2-frame ring and interpolates by timestamp.
- **Plug-in GUI in Live (Windows/JUCE).** Editor is destroyed when the device view collapses — all UI state in the processor. Don't open an OpenGL context in the editor; blit a 15 fps Spout thumbnail into a `juce::Image` with the software renderer.
- **Smoothing** in the engine, not the plug-in. One-pole `y += (x-y)*(1-exp(-dt/τ))`. Topology-changing macros (Detail→octaves, Space→raymarch bounds) must be quantised and crossfaded, not smoothed.
- **Presets.** Scene JSON, LOOK state and macro set as three separate objects with independent transition times.
- **Hot-reload.** 150 ms debounce, compile on a worker thread, swap only on success, keep last-good.
- **Frame pacing.** V-sync on, dt clamped to 1/20 s, grain clock quantised to 24 Hz. Check the panel's actual refresh (48/60/90).

## B7. Performance on integrated GPUs

*Source: https://www.intel.com/content/www/us/en/developer/articles/guide/lp-api-developer-optimization-guide.html*

- Xe-LP: "up to 96 execution units", "up to 2.2 Teraflop", "double-rate FP16 math. Use lower precision when possible", tile-based rendering. OpenGL supported but "the performance benefits with DirectX 12, Vulkan and Metal 2 are greater".
- **Budgets (practitioner defaults; measure with Intel GPA):** full-screen passes are bandwidth-bound on shared memory (1080p RGBA16F read+write ≈ 16.6 MB; 10 passes at 60 fps ≈ 10 GB/s shared with Ableton). Target ≤ 6 full-res passes; blur/bloom/halation/feedback at half or quarter res. fp16 for persistent buffers; RGBA8 + blue-noise dither for display; RGBA32F only for particle positions. fbm ≤ 4–5 octaves at full res. Raymarch 48–64 steps at half res with upscale, early exit, relaxed sphere tracing. Separable blurs 9–13 taps with bilinear tap merging.

---

# Part C: Reference library and artist analyses

## C0. The Noise Diary

*Sources: https://thenoisediary.com/ , https://thenoisediary.com/about/ , https://generativemedia.club/artist/ , https://thenoisediary.bandcamp.com/album/the-noise-diary , https://open.spotify.com/artist/0gT2EgT72VXknZmDUjJ3Ze , https://www.facebook.com/thenoisediary/*

**Identification: confirmed.** "The Noise Diary is the audiovisual work of Adam Clark and Gretchen McNelis, creating immersive environments where electronic and acoustic composition, voice, instrumentation and generative visual systems converge." GenerativeMedia.club Artist of the Week #39 (June 2026): "merges generative visuals, sound design, realtime systems, and cinematic atmosphere"; quote: "A piece has to resonate emotionally to me before anything else." Bandcamp (New York; tags downtempo, modular synth, noise, acoustic guitar); self-titled album 22 Sep 2022, 8 tracks, 48:41. Tagline "Sublime Noise". Aesthetic in their words: "the tension between organic and synthetic forces … beauty and abrasion, order and collapse, intimacy and vastness"; "forms emerge, erode, fragment and reassemble"; live shows use "custom visual systems that respond and evolve in real time".

**35mm film?** No public text mentions film grain, 35mm or analog film — **not confirmed**. Confirmed overlap: generative real-time systems, audio-reactive, noise as material, cinematic atmosphere, slow erosion/reassembly. *Inference:* file name `TND_2025_11_08_TD_PROJ_1.mp4` suggests TouchDesigner (unverified).

**Assets to capture:**
- https://thenoisediary.com/wp-content/uploads/2026/02/TND_heron_web_01B_1.mp4
- https://thenoisediary.com/wp-content/uploads/2026/02/TND_data_web2.mp4
- https://thenoisediary.com/wp-content/uploads/2026/02/TND_2025_11_08_TD_PROJ_1.mp4
- https://thenoisediary.com/wp-content/uploads/2026/01/TND_logo_v02_sm.mp4
- Still: https://generativemedia.club/wp-content/uploads/2026/06/The-Noise-Diary-TheNoiseDiary_010-768x513.jpg

**Take for the instrument (inference):** a slow structural-state scene — a dot/particle form whose cohesion is a macro (Gravity/Viscosity), eroding as Build rises, reassembling in breakdowns. Map **spectral flatness** (noisy vs tonal) to erosion rather than kicks.

## C0b. phs.wrk ("phswrk.")

*Sources: https://www.instagram.com/phs.wrk/ (login-walled), https://open.spotify.com/track/4DbDYHzEkClbb3OjQG0Ai6*

- Reels not viewable. Public traces: captions "echo::arcflash / 07.17.2026 #touchdesigner", "arcflash // out now! #touchdesigner #industrial #doom", "work for @angelsinarchive #touchdesigner"; Spotify track "arcflash" by "phswrk", 2026-07-17, 5:08. Real name unknown.
- **Analysis based on the owner's description, not footage:** palette black → oxblood → red → salmon (inferred hex #000000, #2A0303, #8E0B0B, #D8231C, #F08A78); hard threshold with 70–85 % of the frame at the extremes; grain at the threshold edge → noise added *before* threshold; ~1 cut/s with 1–2-frame black/red flashes; braille/glyph grids, terminal text, per-row smear, quantised pixel blocks, root/nerve textures, continuous corridor dolly.
- **TD technique inference:** corridor → feedback TOP with slight zoom → noise → threshold → red ramp lookup → glyph atlas multiply/screen. Cuts via count CHOP on kick or timer. Cost low except the corridor.
- **Missing from your LOOK panel:** dithered threshold; per-row smear.

## C1. Ambient / grain / noise / analog texture

*Sources: https://www.cerclemagazine.com/en/magazine/articles-magazine/rainer-kohlbergers-visual-music-between-noise-and-light/ , https://vimeo.com/rainerkohlberger , https://iffr.com/en/person/rainer-kohlberger , https://mubi.com/en/cast/rainer-kohlberger , https://vimeo.com/31265234 , https://en.wikipedia.org/wiki/Arnulf_Rainer_(film) , https://www.harvestworks.org/?p=1951 , https://2019.grayareafestival.io/?p=50*

1. **Rainer Kohlberger, *keep that dream burning* (2017).** 35mm, 2.39:1, 8 min, B/W, epilepsy warning. "Extremely fine black-and-white particles that flutter across the screen"; algorithms extract noise from action films; "oscillates between maximum abstraction and pure blur. Within the blurriness, objects form and disappear." Recipe: 3D value-noise volume, blurred with a spatially varying radius on a slow LFO, luma threshold 0.45–0.55 soft. Informs Grain + Crush; new scene "Signal Fog".
2. **Kohlberger, *moon blink* (2014).** Video, audio, stroboscope, 10-min loop. Informs Flash and a photosensitivity limiter.
3. **Kohlberger, *field* (2012), *deviation* (2020), *Emergence Collapse* (2021).** *field* won the ZKM App Art Award 2011. "My work is noise through and through."
4. **Kohlberger & Wilm Thoben, *White Light/White Heat* (2011).** Laser via oscillating mirrors; "flicker and envelopes modulate basic geometric shapes". Recipe: one line shape strobed 8–30 Hz into a persistence buffer.
5. **Peter Kubelka, *Arnulf Rainer* (1960).** Only solid black/white frames, white noise vs silence, 6.5 min; runs "as long as 24 frames and as short as a single frame". The grammar of your Flash: rhythmic patterns, not random events.
6. **Sabrina Ratté, *Habitat* (vimeo.com/105832358).** LZX modular video synth; "When I acquired my LZX, I started generating all of my images with this tool rather than recording 'reality'." Recipe: ramps → wavefolder → colour quantisation, LFO scanline stretch.
7. **The Noise Diary, site reels (2026).** See C0.
8. **Photism *ink*.** Domain-warped marbling. Still: https://photism.app/clips/ink.jpg?v=20260731a
9. **Pierce Warnecke, *Textures* (2012).** Live AV with found objects, field recordings and video samples: "granular surfaces, aged objects and deteriorated materials … the effect of time on matter: modification, deterioration and finally disappearance." Later *Endless Growth (Data Decay v3)* (2019, ISM Hexadome) does the same from datasets. Strongest conceptual match for "matter decaying over the set" driven by Build → Erosion Body scene.

## C2. Dark monochrome / red-industrial

*Sources: https://vimeo.com/206068020 , https://vimeo.com/160648305*

10. **phs.wrk, *arcflash* reels (2026).** See C0b.
11. **Ryoichi Kurokawa, *node 5:5* (2017).** 4K projection, wave-field synthesis, 10-channel kinetic laser.
12. **Kurokawa, *unfold* (2016).** 3 projections, 6.1 sound, tactile transducer, built from CEA/ESA/NASA datasets. Informs dot relief.
13. **Kurokawa, *syn_* (2011→), *rheo: 5 horizons* (Golden Nica 2010).** Concrete material with digital structures.
14. **Photism *oracle*.** "Near-black machine temple. The palette lives on the edges." Still: https://photism.app/clips/oracle.jpg?v=20260731a
15. **Photism *gate*.** Corridor fall-through. Still: https://photism.app/clips/gate.jpg?v=20260731a

## C3. Glitch / datamosh

*Sources: https://americanart.si.edu/node/1351 , https://www.eai.org/titles/monster-movie , https://rhizome.org/editorial/2380 , https://hyperallergic.com/takeshi-murata-monster-movie*

16. **Takeshi Murata, *Monster Movie* (2005).** 4:19, sound Plate Tectonics, Smithsonian collection. Footage from *Caveman* (1981); datamoshing mixed "the video's compression into a kind of digital liquid". Rhizome places it with Paper Rad/Paul B. Davis (2007) as pre-2009 pixel-bleed art. Full-spectrum palette (opposite of yours); take the *rhythm*: every distortion timed to the soundtrack. Recipe without codec corruption: keep a motion-vector field (16-px block matching or curl noise × flux); on "mosh" trigger stop reading new frames and advect the previous frame for N frames. Cost low at half res.

## C4. Minimal generative / geometric / data

*Sources: https://special.ycam.jp/datamatics/en/testpattern.html , https://www.creativeapplications.net/project/ryoji-ikeda-profile/ , https://www.e-flux.com/announcements/39634/ryoji-ikeda-datamatics*

17. **Ryoji Ikeda, *test pattern* (2008→).** Converts any data into barcode and 0/1 patterns; first edition 8 monitors + 16 speakers, patterns generated from waveforms in real time at "some hundreds of frames per second". Third concert in *datamatics*; *data.tron* calculates every pixel by mathematical principle. Recipe: 32-band spectrum → bar widths quantised to 1/2/4/8 px, tiled with per-row phase; binary grid from bit-planes. Strictly 1-bit; bypass the film chain. Informs Symbols; new "Barcode" scene.

## C5. Organic / fluid / cellular

18. **Photism *morphogen***: https://photism.app/clips/morphogen.jpg?v=20260731a
19. **Photism *fluid***: https://photism.app/clips/fluid.jpg?v=20260731a
20. **Photism *sand*** (Chladni): https://photism.app/clips/sand.jpg?v=20260731a
21. **Paul Prudence, *The Mylar Topology*.** Liquid forms with binaural beats, "vertebral columns and congealing oil slicks". (https://www.creativeapplications.net/people/paul-prudence/)

## C7. Abstract 3D / sculptural light

*Sources: https://www.creativeapplications.net/project/cyclotone-2012-by-paul-prudence/ , https://vimeo.com/129664516 , https://visualmusicarchive.org/works/w/cyclotone.html*

22. **Paul Prudence, *Cyclotone* (2012) / *Cyclotone II* (2015, vvvv).** Uses OSC/MIDI triggers embedded in Ableton Live's timeline plus real-time audio analysis; primitives deformed and extended in 4D, textures modulated by sound. Stills: Flickr set linked from vimeo.com/129664516. That hybrid of composed DAW triggers + live analysis is what your Live-transport integration offers. Recipe: SDF torus/ring stack rotated in a 4D plane, edge-distance shading, hats modulate texture. Cost medium.
23. **Prudence, *Parhelia*.** Concentric forms as an imaginary machine.
24. **T3X2R MAYAS** (marketing GIF): https://www.t3x2r.com/wp-content/uploads/2022/06/mayasv2.5_4b.gif

## C6 / C8 and unverified leads

Section 6 (typography/glyphs): Ikeda's *test pattern* (above) and phs.wrk. Section 8 (analog video synth): Ratté/LZX (above). **Unverified leads (no pages opened):** Tarik Barri (*Versum*, navigable AV space — camera-as-instrument for the corridor), Robert Henke (*Lumière*, laser persistence), Kangding Ray (red/black industrial live visuals), Alba G. Corral (organic Processing work), Rosa Menkman (glitch theory).

---

# Style bible for the owner's look

*Recommended starting values from craft practice; published sources named in Part B where they exist.*

**Grain**
- Cell size 1.2–2.0 px at 1080p, scaled with render height.
- Intensity σ 0.035 (fine) – 0.08 (500T) in display space at mid-grey.
- Luminance response: peak at L 0.4–0.5; 25 % of peak at L 0.05; < 10 % above L 0.95.
- Temporal: 24 fps with golden-ratio offset; 80 % luma / 20 % chroma; red palette → luma only, before gradient map.
- Audio: HAT/texture energy modulates grain ±20 % max, τ 0.5 s.

**35mm artefacts**

| Artefact | Range | Behaviour |
|---|---|---|
| Gate weave | 0.3–1.5 px, 0.5–2 Hz | Continuous; 3–8 px slip every 16th–32nd bar |
| Flicker | 1–4 % luminance | 24 fps random + 0.3–0.7 Hz wave |
| Halation | thr 0.75–0.9; radius 0.5–2 % frame height; 0.1–0.4 | After palette; red-orange |
| Bloom | thr 0.8; 0.05–0.2 | Quarter res |
| Dust | 0–3 specks/frame, 1-frame life, 1–6 px | Poisson; rate by Detail |
| Hairs | 1 per 10–30 s, 0.5–3 s life | Bezier strand, slight drift |
| Scratches | vertical 1–2 px, 2–10 s, drift < 0.5 px/frame | Only on Build > 0.5 |
| Light leak / burn | 8–20 s cycles, 0–30 % screen, warm | Intros and breakdowns only |
| Chromatic softness | 0.5–1.5 px radial RGB offset | Stronger at edges |
| Lifted blacks | floor 0.02–0.06 (e.g. #0B0605) | Never pure #000 except Flash |
| Acutance | radius 1–2 px, 0.1–0.3 | Before grain |
| Dither | ±1 LSB triangular blue noise | Last pass |

**Palette families**
- *Ember (current):* #0B0605 → #5A0A08 → #C21D14 → #F2917D
- *Nitrate:* #0D0B09 → #4A3C2E → #B89A74 → #F3E6CE
- *Cyanotype Night:* #05080C → #10324A → #4E8AA8 → #D8EEF2
- *Tungsten Halation:* #070504 → #3B2317 → #D06A2E → #FFD9A8
- *Ash Mono:* #0A0A0A → #3C3C3A → #9A9892 → #EDEBE4

**Motion tempos**

| Genre | Camera drift | Breathing (scale 1–2 %) | Macro slews | Cuts |
|---|---|---|---|---|
| Ambient (60–90 BPM / free) | 0.5–2 % frame width/s | 4–8 s or 2 bars | 4–16 beats | ≥ 8 bars apart or none |
| Downtempo (85–110 BPM) | bass-clock driven | 1 bar | — | 1–2 bars; flashes on downbeats only, ≤ 1 per 2 bars |

**Composition.** One dominant mass; 60–75 % of frame near black; light at the edges (Photism *oracle*); vanishing point offset by a third; glyph overlays 8–15 % coverage, never over the focal mass.

**Do:** clocks carry motion; ≤ 2 reactive routes per feature; quantise structural events to the beat; grain stays alive in Freeze; look intensity fades with Presence.
**Don't:** map level to brightness at full range; step grain at render fps; clip blacks; rely on a constant vignette; flash > 3/s; faces, skulls, masks.

---

# 15 new scene concepts

*Iris Xe cost estimates at 1080p/60 with half-res simulation. Each exposes the 8 macros plus 2–3 scene-specific knobs and follows the REACT TO slot model.*

1. **Signal Fog** (Kohlberger): blurred 3D noise, objects forming and dissolving. Bass clock → z-scroll; Build → blur radius falls. Detail = octaves 2–4, Space = blur. Low (2 passes).
2. **Erosion Body** (Noise Diary / Warnecke): dot-relief form whose cohesion falls with Build and spectral flatness. Gravity, Viscosity. Point grid displaced by SDF gradient + curl noise. Medium.
3. **Dithered Threshold Corridor** (phs.wrk): half-res corridor raymarch → grain before 1-bit threshold → red ramp. Medium (48 steps).
4. **Nitrate Loop:** generative film strip with frame lines, weave, burn-through at phrase ends. Low.
5. **Morphogen Roots:** quarter-res Gray-Scott, feed/kill by Color/Detail; kick seeds at centroid. Medium (8–16 iters).
6. **Halation Field:** sparse bright points on black where halation is the image. Hats spawn; bass sets radius. Low–medium.
7. **Flicker Grammar** (Kubelka): black/red frame patterns from rhythm templates, ≤ 3/s. Trivial.
8. **Braille Rain:** glyph atlas scrolled by the hat clock, masked by scene luminance. Low.
9. **Terminal Weather:** invented script; line rate by flux, corruption by flatness. Low.
10. **Laser Persistence** (White Light/White Heat): one vector line in a persistence buffer, strobed 8–30 Hz. Low.
11. **Chladni Plate:** spectrum bands → mode weights; sand as density field with feedback. Low–medium.
12. **Datascape** (*unfold*): point clouds, slow orbit, depth fog, grain. Medium.
13. **Analog Ramp Synth** (Ratté/LZX): ramp → wavefolder → colour quantisation, scanline stretch. Trivial.
14. **Ink Marble 2:** two-level domain warp, Viscosity-controlled step; mids push the warp. Low–medium.
15. **Ghost Crossfade Scene:** meta-scene holding the previous scene's frozen feedback as decaying memory. Low.
16. *(bonus)* **Barcode** (Ikeda): 1-bit spectrum barcodes and bit-plane grids, film chain bypassed. Trivial.

---

# Top 15 prioritised additions

1. Scene-level semantic slots (REACT TO v2).
2. Presence/Build signal with hysteresis → look intensity, cut density, grain.
3. Band clocks: energy → speed for every scene's main motion.
4. Per-route asymmetric attack/release with presets Snap / Breath / Tide.
5. Global Reactivity amount + Calm button (fades hit routes over 1 bar).
6. Film output chain v2: acutance → grain (luminance-dependent, 24 fps) → halation → weave → lifted blacks → blue-noise dither, each with on/off and amount.
7. Dithered threshold in Crush (noise before threshold).
8. Feedback-preserving crossfades with a ghost buffer.
9. Snapshot morphing of macro + LOOK states over N beats.
10. Photosensitivity limiter on Flash and Cut Rate (≤ 3/s, optional luminance-delta clamp).
11. Phrase LFOs at 16/32/64 bars + count-logic trigger (every Nth kick).
12. Automatable scene index and macros as VST3 parameters.
13. Role LEDs + outgoing-value monitor in the plug-in GUI.
14. ISF hot-reload with last-good fallback; per-pass resolution scale in JSON.
15. Low-fps preview thumbnail in the plug-in via Spout.

---

# Anti-patterns

- **Jitter at low volume** from raw FFT (Resolume forum) → gate and smooth at the source.
- **Lost control** when everything is audio-driven (ZeroToVJ) → hierarchy + global amount.
- **Brightness pumping on every kick** (Photism explicitly avoids it).
- **Grain that shimmers** at render fps (Film-Standalone's 120 Hz note).
- **Frozen detectors** from UI-binding bugs (TD audioAnalysis) → activity LEDs.
- **Audio dropouts / renderer stalls** (Videosync 2.0, Zwobot) → keep rendering out of the audio process.
- **Constant artefacts** — real film defects are rare and correlated.
- **Uniform white-noise "film" overlays**, fixed vignette, fixed sepia — the cheap-filter signature.

---

# Further reading (opened links only)

**Film emulation**
- https://github.com/artzox/Film-Standalone — parameter vocabulary; the 24 fps argument; acutance.
- https://github.com/CostardRouge/atelier/pull/122 and /pull/144 — band-limited grain, per-channel halation.
- https://github.com/lim8701/FilmRawstery — measured grain-vs-density curves.
- https://vmoldo.com/film-grain-emulation-2025-guide/ — why grain is denser in highlights but visible in shadows.
- https://github.com/CyberTimon/RapidRAW/releases/tag/v1.6.2 — CC BY-SA film profiles (licence warning).

**Noise and dither**
- https://blog.demofox.org/2020/05/10/ray-marching-fog-with-blue-noise/ — golden-ratio animated blue noise.
- https://www.shadertoy.com/view/4sKBWR — blue-noise dither reference.

**Audio-to-visual mapping**
- https://app.synesthesia.live/docs/ssf/audio_uniforms.html — the best signal vocabulary in the field.
- https://app.synesthesia.live/docs/ssf/best_practices.html — the reactivity-toggle advice.
- https://derivative.ca/UserGuide/Palette:audioAnalysis and https://derivative.ca/community-post/asset/audio-analysis-20-extended-audio-reactive-components/73946 — count logic, asymmetric lag.
- https://resolume.com/forum/viewtopic.php?t=21657 and ?t=17270 — the complaints your design answers.
- https://zerotovj.com/should-you-use-audio-reactive-visuals/ — the "lost control" argument.

**Architecture**
- https://support.showsync.com/release-notes/videosync/2.0 — float-buffer fix, audio-dropout fix, Live 11 minimum.
- https://www.zwobotmax.com/manual/ — in-process stall admission, GL2/GL3 bug.
- https://photism.app/learn/best-ableton-visualizers/ — out-of-process renderer over localhost.

**Performance**
- https://www.intel.com/content/www/us/en/developer/articles/guide/lp-api-developer-optimization-guide.html — Xe-LP facts.

**Artists**
- https://thenoisediary.com/ , https://generativemedia.club/artist/
- https://vimeo.com/rainerkohlberger , https://www.cerclemagazine.com/en/magazine/articles-magazine/rainer-kohlbergers-visual-music-between-noise-and-light/
- https://www.eai.org/titles/monster-movie , https://americanart.si.edu/node/1351 , https://rhizome.org/editorial/2380
- https://special.ycam.jp/datamatics/en/testpattern.html
- https://www.creativeapplications.net/project/cyclotone-2012-by-paul-prudence/ , https://vimeo.com/129664516
- https://www.harvestworks.org/?p=1951 , https://2019.grayareafestival.io/?p=50
- https://vimeo.com/206068020 , https://vimeo.com/160648305 , https://vimeo.com/31265234

---

# Sources (every URL opened)

showsync.com/videosync/ · videosync.showsync.com · videosync.showsync.com/compare-editions · support.showsync.com/release-notes/videosync/2.0 · support.showsync.com/release-notes/videosync/1.1 · beam.showsync.com · cdm.link/showsyncs-beam-1-5-lights-in-ableton-live/ · zwobotmax.com/manual/ · zwobot.gumroad.com/l/nqkxh · zwobot.gumroad.com/l/zwobotsuite · imaginando.pt/products/vs-visual-synthesizer · arkestra.app · photism.app · photism.app/scenes/ · photism.app/buy/ · photism.app/lite/ · photism.app/learn/best-ableton-visualizers/ · t3x2r.com · app.synesthesia.live/docs/ssf/audio_uniforms.html · app.synesthesia.live/docs/ssf/best_practices.html · production.synesthesia.live/faq/ · resolume.com/support/en/7/parameter-animation · resolume.com/forum/viewtopic.php?t=21657 · resolume.com/forum/viewtopic.php?t=17270 · zerotovj.com/should-you-use-audio-reactive-visuals/ · derivative.ca/UserGuide/Palette:audioAnalysis · docs.derivative.ca/Palette:audioAnalysis · forum.derivative.ca/t/audio-analysis-not-detecting-kick-snare-and-rhythm/293931 · derivative.ca/community-post/asset/audio-analysis-20-extended-audio-reactive-components/73946 · interactiveimmersive.io (audio-reactive lesson) · github.com/artzox/Film-Standalone · github.com/CostardRouge/atelier/pull/122 · github.com/CostardRouge/atelier/pull/144 · github.com/lim8701/FilmRawstery · vmoldo.com/film-grain-emulation-2025-guide/ · github.com/CyberTimon/RapidRAW/releases/tag/v1.6.2 · blog.demofox.org/2020/05/10/ray-marching-fog-with-blue-noise/ · shadertoy.com/view/4sKBWR · image-ppubs.uspto.gov (12283028) · intel.com Xe-LP guide · thenoisediary.com · thenoisediary.com/about/ · thenoisediary.bandcamp.com · open.spotify.com/artist/0gT2EgT72VXknZmDUjJ3Ze · open.spotify.com/track/4DbDYHzEkClbb3OjQG0Ai6 · generativemedia.club · generativemedia.club/artist/ · last.fm (The Noise Diary) · facebook.com/thenoisediary · instagram.com/thenoisediary · instagram.com/phs.wrk · cerclemagazine.com (Kohlberger) · vimeo.com/rainerkohlberger · iffr.com/en/person/rainer-kohlberger · mubi.com/en/cast/rainer-kohlberger · vimeo.com/31265234 · en.wikipedia.org/wiki/Arnulf_Rainer_(film) · vimeo.com/206068020 · vimeo.com/160648305 · americanart.si.edu/node/1351 · eai.org/titles/monster-movie · rhizome.org/editorial/2380 · hyperallergic.com/takeshi-murata-monster-movie · special.ycam.jp/datamatics/en/testpattern.html · creativeapplications.net/project/ryoji-ikeda-profile/ · e-flux.com/announcements/39634 · creativeapplications.net/project/cyclotone-2012-by-paul-prudence/ · creativeapplications.net/people/paul-prudence/ · vimeo.com/129664516 · visualmusicarchive.org/works/w/cyclotone.html · harvestworks.org/?p=1951 · 2019.grayareafestival.io/?p=50 · werkleitz.de (Biederman/Warnecke)

---

# Caveats

- phs.wrk's visuals are analysed from the owner's description only.
- The Noise Diary's link to 35mm film is unconfirmed.
- Numeric style ranges are craft recommendations, not measurements, except where a source is cited.
- Licences for Film-Standalone, prod80 and FilmRawstery are unknown — read before copying code.
- T3X2R, Arkestra and VS claims come from marketing pages.
- Competitors' minimum Live versions come from stores and release notes; confirm before relying on them for positioning.
