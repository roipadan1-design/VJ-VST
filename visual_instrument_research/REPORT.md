# From prototype to an audio-reactive visual instrument
## Research report — Windows 11 · Ableton Live 10 · Max 8

**Research date:** 22 September 2026  
**Decision:** retain the external renderer; turn the control and modulation layer into an instrument before changing plugin format or graphics API.

### Reading this report

This report follows the supplied brief’s sections A–I. The implementation contract is in **ENGINEERING_SPEC.md**; the twelve visual designs are in **STARTER_PRESETS.md**. The bundle also contains a strict preset schema, a worked preset, an original starter shader, a feature-frame schema, and a validator. Source IDs link to **sources.md**, which records URLs, dates and evidence limitations.

**[Verified]** means a feature, requirement, license statement or documented behavior was found in the identified primary source. It does not mean the software was installed here. **[Vendor claim]** means a supplier’s performance, quality or operational claim, including advertised compatibility. **[Inference]** means an interpretation, proposed implementation or engineering judgment. All proposed budgets and default parameters are design targets, not benchmark results.

The existing implementation is described by the supplied brief, not inspected source code. Official documentation, release histories, repositories, developer posts and several official stills were examined. Embedded demonstration videos were located but not played or frame-reviewed. Consequently, this report does **not** present guessed motion, FPS, latency, process boundaries or shader algorithms as observations. Current purchasing pages sometimes failed to expose prices. Those prices remain unknown rather than being filled from old reviews.

## Executive summary — עברית

**ההמלצה המרכזית: לא להחליף עכשיו את הארכיטקטורה.** כבר יש לכם חלוקה נכונה בין Live, מכשירי שליטה וניתוח, ומנוע גרפי חיצוני. השלב הבא צריך לשפר את השפה המוזיקלית של המערכת: נרמול עוצמה אדפטיבי, אירועי מכה נפרדים מעוצמה רציפה, מעטפות, ארבעה מאקרו עקביים, מעברים מסונכרנים וסט קטן של פריסטים מוקפדים. מעבר ל־VST3 אינו פותר לבדו אף אחת מהבעיות האלה.

**Max 8 אינו סיבה לוותר על M4L.** מגבלת חלונות Jitter ברישוי דרך Live חלה בזמן עריכה; היא אינה מגבלה כללית על מכשיר שמור שפועל בתוך Live. קיימות גם עדויות מפתח ל־GL3 ו־Spout ב־Max 8 על Windows. למנוע חיצוני ממילא אין תלות בחלון Jitter. [[H02]](sources.md#h02) [[H03]](sources.md#h03)

מפת התאימות שונה מכפי שאפשר להסיק מדפי השיווק: Videosync כבר תומך ב־Windows, אבל דורש בו Live 12; Beam דורש לפחות Live 11.3.20. Zwobot 3 הפסיק תמיכה רשמית ב־Live 10 בנובמבר 2024. מדריך Photism מציין Live 11/12 למכשיר M4L; גרסת VST3 שלו היא מועמדת לבדיקה, לא תאימות מוכחת ל־Live 10. Arkestra הוא מוצר Mac בלבד. דווקא T3X2R מציין בדיקה עם Live 10 ו־Max 8.6.5. VS 2 תומך ב־Windows/VST3, אך לא נמצאה התחייבות מפורשת ל־Live 10. [[S02]](sources.md#s02) [[S04]](sources.md#s04) [[Z03]](sources.md#z03) [[P03]](sources.md#p03) [[A01]](sources.md#a01) [[T02]](sources.md#t02) [[V01]](sources.md#v01)

הרפרנסים החשובים ביותר לפיתוח הם **VS** למבנה מודולציה סינתיסייזרי, **Photism** לפריסטים ולשפה חזותית מוגבלת ומגובשת, **Arkestra** למאקרו ולחלוקת תפקידים, **Showsync** לאינטגרציה עם Live, ו־**MilkDrop/NestDrop/Synesthesia** לפריסטים, זיכרון חזותי ושכבת audio→shader. יש כאן לקחים משלימים, לא מוצר יחיד שחייבים לשכפל.

**המסלול המוצע:** תחילה שלושה פריסטים גנרטיביים טובים על המנוע הנוכחי, עם ניתוח משופר ושליטה אמיתית מ־MIDI; לאחר מכן צינור צבע ליניארי עם מטרות float, bloom מדוד, דפדפן פריסטים ומעברים; ורק אחרי מבחני עומס ושמירת סט — מעטפת VST3 דקה, אם היא חוסכת התקנה או מאפשרת ניתוח C++ משותף. את הרינדור משאירים בתהליך חיצוני.

לגרסה הראשונה לא נדרשים הפרדת מקורות באמצעות ML, דיפוזיה בזמן אמת, Vulkan או מנוע תלת־ממד מלא. ל־Ableton כבר יש יתרון טוב יותר: ערוצים מופרדים ו־MIDI. מפרט המימוש מציע חוזה אירועים, אלגוריתמים וערכי פתיחה, סכמת JSON, מיפוי קונטרולר, תקציבי ביצועים ומבחני קבלה; כל מספר שלא נמדד מסומן כיעד לבדיקה.

# A. Deep teardown of the six references

## A1. Showsync: Videosync, Beam for Live and Sync Tools

**Position and architecture.** [Verified] These are different products, not interchangeable visualizers. Videosync extends Live’s tracks, clips and racks into video; Beam addresses lighting fixtures; LiveGrabber and the other Sync Tools exchange control and time information. Videosync documents a companion application and networked rendering, including running Live and the renderer on different computers. That is strong evidence for separation beyond “another plugin window.” Its public shader/plugin interface includes GLSL and M4L; the exact native graphics backend and interprocess implementation are not established by the examined pages. [[S01]](sources.md#s01) [[S03]](sources.md#s03) [[S04]](sources.md#s04) [[S05]](sources.md#s05)

**Compatibility and licensing.** [Vendor claim] Current Videosync requirements are Windows 10 build 1909+, **Live 12.0.0+ and Max 8.6.0+ on Windows**. The Mac minimum is Live 11.3.20/Max 8.5.6. Intro does not require M4L, but that does not establish a Live 10 Windows exception. Current Beam requires Live 11.3.20/Max 8.5.6. Neither current full product meets the fixed host target. Commercial licenses and an unrestricted-duration, fully functional Videosync trial are documented; an authoritative current checkout price was not recovered. Record it as unknown. [[S02]](sources.md#s02) [[S03]](sources.md#s03) [[S04]](sources.md#s04)

**Analysis, modulation and MIDI.** [Verified] The strength is native musical structure: clip playback, MIDI-triggered Video Simpler, polyphony, warping, racks, track faders, sends and automation. LiveGrabber explicitly exports envelopes, onsets, MIDI notes and track/scene/plugin changes through OSC, including a reverse control path. A published FFT size, onset algorithm, universal modulation matrix or per-band classifier for Videosync was not found. Do not convert “integrated with Live” into an invented MIR feature list. [[S03]](sources.md#s03) [[S05]](sources.md#s05)

**Content, look and UI.** [Verified] The official Squares still is a useful miniature design study: a compact device exposes geometry, note assignments, velocity response and attack/release rather than a huge generic parameter list. It establishes note-controlled geometry, not the internal rendering algorithm. [Inference] Reproduce that discipline with a few stable semantic controls; clean shape edges, carefully spaced repetitions and controlled event envelopes can carry a performance without expensive simulation. Videosync’s wider catalog is a toolkit rather than one compulsory aesthetic. [[S06]](sources.md#s06)

**Output and reliability.** [Verified] The feature list specifies main/return Spout or Syphon output, external camera input, an independently chosen rendering resolution, and an ISF device with a substantial shader library. HAP support is explicitly Mac-only. The product’s 2.1 page adds video recording and monitoring. [Vendor claim] Stage reliability and frame accuracy are advertised, but the examined material does not provide a reproducible GPU/FPS benchmark on the target machine. [[S01]](sources.md#s01) [[S03]](sources.md#s03)

**What to borrow.** [Inference] Borrow the musical object model and “the Live device is the instrument panel.” Keep video timing, preview and output resolution separate. Borrow LiveGrabber’s breadth of control messages, not an assumption that every latest download retains Live 10 support. Avoid building fixture control, full video warping or a complete rack-equivalent compositor into v1. Those would compete with the narrow preset instrument rather than improve it.

## A2. Zwobot

**Position and architecture.** [Verified] Zwobot is a modular M4L video-processing ecosystem: decks, clip support, effect modules, monitoring and recording. Its documentation describes Max video/GL infrastructure and Windows Spout installation. [Inference] Treat the normal implementation as Max-hosted graphics unless a particular version demonstrates an independent renderer process; a monitor or output window alone proves no such isolation. No reliable public evidence identifies one universal GL2/GL3 backend for all versions. [[Z01]](sources.md#z01) [[Z02]](sources.md#z02)

**Critical compatibility correction.** [Verified] The developer’s **9 November 2024** v3 announcement withdraws official Live 10 support while allowing that it might still run. The preceding v2.95 release, dated 21 March 2024, is a legacy investigation candidate, not a supported recommendation. A stale minimum such as Max 7.2.5 on a multilingual page cannot override the versioned changelog. The current homepage also displays a maintenance warning asking customers not to order. Current checkout price and the availability/support terms of a legacy installer remain unknown. [[Z03]](sources.md#z03) [[Z01]](sources.md#z01)

**Audio, modulation and MIDI.** [Verified] Zwobot combines tempo-linked motion, sequencing and effect parameters exposed through M4L. The developer changelog reports a low-end double-trigger fix; that establishes the existence of beat-reactive behavior, not a verified kick classifier. FFT length, normalization strategy, exact band boundaries, external sidechain architecture, controller templates and LED feedback are unknown in the examined documentation. Existing Live MIDI mapping remains a relevant integration route. [[Z02]](sources.md#z02) [[Z03]](sources.md#z03)

**Content and appearance.** [Verified] The catalog contains both source/geometry tools and recognizable processing families: RGB manipulation, MOSH, blur/bloom, pulse, displacement/height-like treatments and blending. The inspected official effects still uses a dense grey Live-style panel, colored knob arcs, crossfade and overlay controls. [Inference] Its characteristic design opportunity is staged transformation of a source: distort coordinates, alter channels, accumulate feedback, then finish the color. This resembles the current prototype’s strongest area. Adding more glitch stages is therefore less strategically valuable than using Zwobot to learn controllable effect ranges and reliable transitions. [[Z01]](sources.md#z01) [[Z04]](sources.md#z04)

**Output, cost and field evidence.** [Verified] The manual discusses recording, output monitoring and Spout/Syphon workflows. It warns about video decoding/looping behavior and recommends an intraframe-friendly workflow where appropriate. Versioned developer notes document undo-history issues, antialiasing/default-resolution changes and dependency adjustments around newer Max/VIDDLL versions. These are real maintenance signals, not evidence that all Zwobot installations are unstable. No comparable modern 1080p/4K GPU benchmark was found. [[Z02]](sources.md#z02) [[Z03]](sources.md#z03)

**What to borrow.** [Inference] Borrow composable effects, usable parameter bounds and the separation of input/processing/output modules. Do better on dependency diagnostics and explicit host-version qualification. Avoid copying a large module catalog before the browser, calibration and event semantics are solved. Do not ask a Live 10 user to purchase the current package on the strength of an old minimum-version statement.

## A3. VS 2 — Imaginando

**Verified product facts.** VS 2 is a commercial visual synthesizer offered as standalone and plugin variants, including Windows VST3 instrument/effect workflows. The product page lists **€129**, **€9.90/month rent-to-own**, five activations and Windows 10/11. The payment-count/contract details were not verified. [Vendor claim] Its FAQ suggests an RTX 2060-class dedicated GPU as a sensible starting point, not a guarantee of every scene at 4K60. Live 10 certification was not found: test its current VST3 on Live 10.1.43 rather than declaring compatibility from the format alone. [[V01]](sources.md#v01)

**Verified instrument model.** Its documented structure has eight visual layers, four voices per layer, four LFOs, four audio modulators and two envelopes. Audio modulation distinguishes gate/peak-style behavior from selectable spectrum regions. The signed modulation matrix includes envelopes, LFOs, audio, keyboard and velocity sources. MIDI learn, tempo synchronization and preset playback are documented. Critically, the manual says VS’s material/shader format is **not directly ISF-compatible**; importing arbitrary ISF is not a drop-in interchange promise. [[V01]](sources.md#v01) [[V02]](sources.md#v02)

**Output and unknown internals.** Spout/Syphon/NDI and high-resolution recording are advertised. The claimed recording ceiling is not proof of sustained 4K60 under arbitrary layers. Exact FFT/hop, normalization, graphics API, render-process ownership and Live 10 editor-close behavior remain unknown. A mature commercial implementation is evidence that a plugin visualizer can be shipped, not evidence that GPU-in-editor risks disappear. [[V01]](sources.md#v01)

**Design interpretation.** [Inference] This is the most directly useful reference for making visuals feel like a synthesizer. The important copy is not “eight layers,” but a small number of clearly typed modulators with predictable destinations. Note pitch can choose structure; velocity can scale a short envelope; a slow LFO can keep motion alive between hits. This gives temporal hierarchy that raw level mapping lacks.

For your v1, one scene, two trigger envelopes, two tempo LFOs and four visible macros are preferable to reproducing all VS polyphony. Add polyphony only where the visual idea needs independent note objects. A ring pulse might instantiate four concurrent note shapes; a fullscreen feedback shader should usually remain one shared scene. Expose audio calibration separately from preset settings so switching a look does not undo the performer’s input trim.

The visual implementation lesson is also restraint: layered procedural materials can produce density without requiring particle physics. A clean background, a primary geometric form, a secondary texture and one finishing stage are sufficient. No specific SDF, fluid or raymarching implementation is attributed to VS without shader-level evidence.

## A4. Arkestra 3

**Position, format and price.** [Verified] Arkestra is a standalone Mac visual-performance environment, with an **Echo AU/VST3** companion that sends audio analysis from a DAW. This is a particularly relevant commercial example of a thin DAW component plus a separate visual application. The current product requires macOS 13+, not Windows. Pro is listed at **$199** for two computers and 3.x updates, or **$19 × 12 months** rent-to-own; a free tier exists. The latter totals $228 before tax, so it is not the same total as the one-time price. None of those options satisfies the fixed Windows requirement. [[A01]](sources.md#a01)

**Analysis and modulation.** [Verified] The audio documentation distinguishes broad continuous features from configurable kick/snare/hat-style onset regions, with threshold and temporal controls. MIDI mapping supports more than CCs, including note-driven envelopes and program changes; device-specific feedback is documented. Chain macros can control multiple destinations. These are useful contracts to imitate independently of platform. [[A02]](sources.md#a02) [[A03]](sources.md#a03) [[A04]](sources.md#a04)

The audio page’s wording about approximately sixty “third-octave” bands should not be imported as a verified filterbank specification. [Inference] Twenty hertz to twenty kilohertz spans roughly ten octaves, hence roughly thirty third-octave intervals. The page may mean another range or band spacing; the examined text does not reconcile it. The lesson is to publish your own exact frequency edges rather than imitate ambiguous marketing terminology.

**Scenes, graphics and UX.** [Verified] Scenes select combinations across tracks/chains; the interface puts preview, tracks, effects and an inspector into distinct areas. The code editor exposes GLSL/ISF-style shader authoring. That proves a shader-authoring interface, not whether every current Mac render pass executes through OpenGL rather than a translated backend. The documented GPU effects include feedback, datamosh, fluid, stutter, pixel sorting and color/LUT operations. [[A05]](sources.md#a05) [[A06]](sources.md#a06) [[A07]](sources.md#a07) [[A08]](sources.md#a08)

**Look and reproducibility.** [Inference] A scene with frame persistence, controlled chromatic structure and one tempo-anchored motion can reproduce much of the “designed instrument” feel without copying Arkestra’s entire editor. Its named effect families suggest implementations worth prototyping—advected history, scanline sampling, threshold sorting, coordinate deformation—but do not reveal exact shader code. For a stage performer, a track macro is valuable because it changes a perceptual idea, not because it exposes more mathematics.

**Output, performance and limitations.** [Verified] External displays, recording and texture-sharing capabilities are documented, with tier restrictions; the current interface documentation also mentions NDI. Hardware-normalized benchmark data and Windows portability details were not found. “Zero latency,” where used in promotion, should be read as qualitative: analysis, scheduling and display still take time. [[A01]](sources.md#a01) [[A08]](sources.md#a08)

**What to borrow.** [Inference] Borrow Echo’s separation of responsibilities, audio-role selection, many-to-one macros and scenes as performance states. Avoid rebuilding a multi-track compositor, code studio and media library simultaneously. Arkestra is a design reference for this project, not a machine-compatible alternative to install.

## A5. Photism

**Position and commercial model.** [Verified] Photism focuses on immediately usable audiovisual looks rather than a general-purpose VJ graph. The current site lists **$19**, twenty scenes and twenty-four palettes, with browser, M4L, plugin and standalone entry points. The site’s update wording is not fully consistent about lifetime versus major-version entitlement; verify the purchase terms rather than promising perpetual updates. [[P01]](sources.md#p01) [[P02]](sources.md#p02)

**Important architecture and compatibility findings.** [Verified] The manual describes M4L analysis → local Node server → browser/**three.js**, **Edge WebView2** for the Windows plugin, and **Live 11/12** for M4L. It also says closing the plugin editor ends an active recording, and simultaneous drawing instances reduce frame rate. Thus Photism is not evidence that an editor-hosted visualizer is lifecycle-independent. The Windows VST3 remains a plausible Live 10.1.43 trial candidate, not a verified target-host solution. [[P03]](sources.md#p03)

**Control model.** [Verified] The manual documents sixteen automatable macros, selectors, CC learn and companion sidechain devices with distinct musical roles. It describes Windows Spout as beta and dependent on Windows graphics capture support. These are practical features with explicit limitations, not proof of an undisclosed low-latency native texture path. [[P03]](sources.md#p03)

**Concrete visual evidence.** [Verified] The inspected Morphogen still has pale, coral-like labyrinths on a black background, arranged in mirrored rounded regions with restrained colored fringes. This is a still-image observation only. The scene’s own page identifies a reaction–diffusion system with feed/kill-style control and symmetry. The Fluid page describes a GPU fluid/particle construction and differentiated musical responses; its particle-count and behavior descriptions are vendor claims, not measured capacity. [[P07]](sources.md#p07) [[P04]](sources.md#p04) [[P05]](sources.md#p05)

**Engineering interpretation.** [Inference] Photism is especially relevant to the new product direction because the scenes share a visual vocabulary. A performer chooses an identity and then varies reaction, motion, trails and color; they do not assemble a rendering engine during the set. Reproduce the design constraint, not the exact art: five semantic palette roles, bounded contrast, one principal silhouette, consistent macro meanings and an attractive zero-input state.

Reaction–diffusion can be recreated with two ping-pong fields at a modest simulation resolution. The expensive-looking result comes from how concentrations become edges, light and negative space, not from simulating at the projector’s full resolution. Likewise, a layered noise field with coherent lighting can be more polished than a technically correct but poorly composed fluid solver.

**What to borrow and avoid.** [Inference] Borrow immediate preset identity, curated color, separated musical roles and clear user-facing diagnostics. Do better on editor-independent operation, target-host qualification and predictable output recovery. Do not infer that WebView2 itself is unsuitable; the actual issue is which lifecycle owns rendering and recording. Browser-style rendering can work, but replacing the working C++/ISF host with that stack would add migration risk without solving the current mapping problem.

## A6. T3X2R

**Position and architecture.** [Verified] T3X2R offers M4L visual modules organized around a required Render component and generators such as Mayas. It is the closest of the six to a positively stated version match. The Render and Mayas product pages list tests with **Live 10/11 and Max 8.6.5**, and a separate Live 12/Max 9.0.5 configuration. The Render price selector spans **$14.99–$34.99**; Mayas is listed at **$11.99** and requires Render. The exact bundle/variant cost needs checkout confirmation. [[T02]](sources.md#t02) [[T03]](sources.md#t03)

**Do not overread the match.** [Vendor claim] A tested-version line is stronger than “M4L compatible,” but it is still not a test on your GPU, OS build and Live set. The page’s “GLCore” wording does not by itself resolve how every download maps to Max 8’s GL2/gl3 engines. An external rendering window is documented; a separate OS rendering process is not. Treat it as Max-based until a process inspection establishes otherwise. [[T02]](sources.md#t02)

**Analysis and control.** [Verified] Mayas exposes spectrum-related mesh deformation, color/position/dimension controls and BPM/envelope-driven behavior. The inspected UI still shows coordinate and rotation controls, representation choices and audio-spectrum influence. Exact analysis bands, adaptive normalization, onset algorithms, sidechain policy, MIDI learn and controller feedback are not established in the public material examined. Live parameters are the natural integration surface; do not advertise unverified extra MIDI features. [[T03]](sources.md#t03) [[T05]](sources.md#t05)

**Look and reproduction.** [Verified] The official Mayas still shows a cyan wire-grid toroidal form against black, with sparse contrasting points. [Inference] The economical route to that kind of image is a parametric mesh with controlled vertex displacement and point/line rendering—not a raymarcher. A simple torus or sphere, smooth band-driven displacement, consistent camera orbit and restrained glow can produce a highly legible stage image. This is one of the best starter directions for your engine because it offers a new generative identity without requiring a full scene graph. [[T05]](sources.md#t05)

**Output and maintenance.** [Verified] Render describes preview/output windows, sharing via Spout/Syphon packages and finishing controls such as antialiasing/gamma/tone treatment. Recording is described through the product’s modular workflow. Numerical FPS and CPU/GPU data are unknown. A CC BY-NC footer on the learning site creates an asset/code licensing question; it is not sufficient evidence to conclude that purchased-device commercial performance is prohibited. Obtain explicit terms before redistributing patches, examples or artwork. [[T02]](sources.md#t02) [[T04]](sources.md#t04)

**What to borrow.** [Inference] Borrow the lightweight parametric-geometry vocabulary and compact Live panel. Use it as the first target-machine comparison trial, with an exact installer/version recorded. Avoid treating the version table as universal certification or assuming the current renderer has your existing process isolation.

## A7. Comparison matrix

**Legend:** D = documented; V = vendor claim; U = unknown in examined evidence. “No” means the current documented requirements do not meet the fixed target, not that hacking or historical releases can never run. Product facts are sourced in A1–A6; comparison judgments are inferences.

| Capability | Showsync / Videosync | Zwobot 3 | VS 2 | Arkestra 3 | Photism | T3X2R |
|---|---|---|---|---|---|---|
| Primary role | Live-native video workflow | Modular M4L video/FX | Visual synthesizer | Standalone visual workstation | Curated reactive scenes | Modular M4L generative graphics |
| Current Windows support | D | D | D | No | D | V |
| Live 10 + Max 8 Windows | **No: Live 12 on Windows** | **Official support withdrawn** | **VST3 trial required** | **No: macOS only** | **M4L no; VST3 trial required** | **Explicit Live 10/Max 8.6.5 test claim** |
| Main form | Companion renderer + Live devices | M4L modules | Plugins + standalone | Standalone + Echo plugin | Browser/M4L/plugins/standalone | M4L + Render module |
| Separate render process | D companion/network architecture | U | U | D standalone architecture | WebView2/editor lifecycle documented; full ownership U | U; separate window only |
| Graphics evidence | GLSL/ISF interface | Max/Jitter ecosystem | Material shader editor; backend U | GLSL/ISF authoring; backend U | three.js / WebView2; GPU scene descriptions | Max/GL terminology; exact version backend U |
| Audio extraction detail | Limited public DSP detail | Limited public DSP detail | Audio gate/spectrum modulators | Configurable audio/onset roles | Role-oriented response; algorithm U | Spectrum/envelope behavior |
| Exact FFT/hop published | U | U | U | U | U | U |
| MIDI emphasis | Notes/clips/Live mapping | Live mapping | Synth-style note/CC modulation | Broad learn and feedback | CC learn + host macros | Live controls; extra protocol U |
| Modulation model | Live racks/automation | Modules and tempo controls | Signed source→destination matrix | Audio/MIDI/modulators/macros | Curated scene controls and macros | Generator-specific controls |
| Explicit synth envelopes/LFOs | Device-dependent | Device-dependent | D | D note-envelope/modulation | Hit decay; exact general LFO model U | BPM/envelope controls |
| Content organization | Tracks, clips, racks, instruments | Decks/modules/effects | Layers, voices, patches | Tracks, chains, scenes | Whole looks/scenes/palettes | Generators + required renderer |
| Main aesthetic lesson | Musical timing and structural clarity | Transformations and compositing | Temporal hierarchy/polyphony | Performance-level scene organization | Coherent art direction | Economical parametric geometry |
| Compact Live strip | D | D | Host plugin panel rather than native M4L | Echo only | M4L macros/selectors | D |
| Spout on Windows | D | D | D | Not applicable | **Beta** | V/package-dependent |
| Syphon on Mac | D | D | D | D | D | D |
| NDI | U in examined core pages | U | D | D in current UI docs | U | U |
| Fullscreen/second display | D | D | D | D | D | D |
| Recording | D in current product | D | D | Tier-dependent | D; editor lifecycle limitation | Modular/version-dependent |
| Price captured | U | U; ordering warning | €129; monthly option | $199 or $19×12 | $19 | Render range + module purchases |
| Controlled FPS benchmark | None found | None found | None found | None found | None found | None found |
| Key explicit risk | Host minimum | Legacy/support/dependencies | Target-host and editor tests | Platform mismatch | Editor lifetime; beta Spout | Exact package/backend/rights |

### Best reference for each capability

[Inference] **Showsync** for making Live’s musical structure the user interface; **VS** for typed modulation; **Photism** for a small preset product with coherent visual identity; **Arkestra** for macro/scene organization and a companion-plugin architecture; **T3X2R** for a stated Live 10/Max 8 target and lightweight geometry; **Zwobot** for modular video treatment and the maintenance lessons of a broad M4L ecosystem. Outside the six, **Synesthesia** is the strongest audio→shader vocabulary reference and **NestDrop/MilkDrop** the strongest preset-performance reference. These are design judgments, not an overall quality ranking.

# B. Extended landscape: only the parts that help this product

## B1. MilkDrop, projectM, Butterchurn and NestDrop

[Verified] MilkDrop’s preset model combines persistent state, per-frame equations, custom shapes/waves and spatial warp evaluation. The historical “per-pixel” terminology can describe a warp mesh’s vertices rather than an arbitrary full-resolution fragment program. MilkDrop 2 adds HLSL warp and composite stages. Its bass/mid/treble values are relative quantities, with attenuated variants such as `bass_att`; do not assume their nominal range is the same as this engine’s 0–1 contract. A universal semantic kick/snare classifier is not implied. [[L01]](sources.md#l01)

[Inference] The enduring insight is that a preset is a **small stateful program**, not just static slider values. It owns visual memory, a few slowly changing quantities and a mapping from music into their evolution. Adopt that idea while avoiding unrestricted preset scripting in v1. Named integrators, envelopes, seeded random sources and a bounded stage graph cover much of its expressive value without arbitrary loops or filesystem access.

[Verified] projectM is a cross-platform OpenGL implementation of the MilkDrop ecosystem; its core is LGPL-2.1. Butterchurn provides a MIT-licensed WebGL 2 implementation. Both are useful for studying warp/composite separation and preset transitions, but neither makes `.milk` shaders directly interchangeable with ISF. LGPL can permit a closed-source application under its conditions; it is not a blanket “cannot use commercially,” nor permission to ignore relinking/source obligations. Preset assets require their own license audit. [[L02]](sources.md#l02) [[L03]](sources.md#l03)

[Verified] NestDrop is a Windows MilkDrop/Spout performance application with multiple decks, a curated preset collection, previews, favorites and queues. Its high-resolution/60-fps advertising is a vendor claim without a standardized hardware workload here. It does not need Live or Max to render; synchronization/control into your Live 10 system would be external and must be configured. [[L04]](sources.md#l04)

**What to take into JSON.** [Inference] Separate `parameters`, `modulators`, `routes`, `events`, `renderGraph` and `transition`. Explicitly declare a feedback buffer’s lifetime. Seed randomness per preset session. Define motion in seconds or beats, not “increment once each frame.” Keep previews/favorites/setlists first-class; a technically excellent preset that cannot be found on stage is not useful content.

## B2. Synesthesia’s audio→shader vocabulary

[Verified] SSF pairs shader and metadata structures, with optional scripting, rather than providing only a raw FFT texture. Its audio interface distinguishes level, hits, presence and time-like accumulators; it also provides beat/BPM functions and spectrum/history textures. Exact proprietary DSP implementations behind those names are not published in the inspected reference. [[L06]](sources.md#l06) [[L05]](sources.md#l05)

The following is an **identifier inventory**, not a claim that this project should clone the algorithms. For the first five families, the documented naming expands over overall, Bass, Mid, MidHigh and High where listed in the source. [[L05]](sources.md#l05)

| Family / identifiers | Intended distinction | Proposed use here |
|---|---|---|
| `syn_Level`, `syn_BassLevel`, `syn_MidLevel`, `syn_MidHighLevel`, `syn_HighLevel` | Smoothed activity | Size, density, bounded deformation |
| `syn_Hits`, corresponding band `Hits` | Brief event-like activity | Trigger an owned decay envelope, not arbitrary frame jitter |
| `syn_Presence`, corresponding band `Presence` | Slower sustained activity | Composition density and secondary texture |
| `syn_Time`, corresponding band `Time` | Integrated audio-driven time | Continuous rotation/advection |
| `syn_CurvedTime`, corresponding band `CurvedTime` | Nonlinear accumulated motion | Occasional acceleration rather than brightness |
| `syn_OnBeat`, `syn_ToggleOnBeat`, `syn_RandomOnBeat`, `syn_BeatTime` | Discrete beat-related behavior | Beat gates, alternating states, held random, counting |
| `syn_BPM`, `syn_BPMTwitcher` | Tempo and beat-shaped progression | Prefer Live transport in this project |
| `syn_BPMSin`, `syn_BPMSin2`, `syn_BPMSin4`, `syn_BPMSin8` | Related tempo oscillators | Slow coherent movement |
| `syn_BPMTri`, `syn_BPMTri2`, `syn_BPMTri4`, `syn_BPMTri8` | Related triangular oscillators | Sweeps and symmetric motion |
| `syn_FadeInOut`, `syn_Intensity` | Longer musical activity | Section-level composition changes |
| `syn_Spectrum` | FFT/processed spectral/waveform channels | Optional texture interface |
| `syn_LevelTrail` | Activity-history texture | Trails, ribbons and retrospective marks |

**Design conclusion.** [Inference] Your current six flat sources mix fundamentally different things. `onset` is an event, `beatphase` is cyclic phase, `level` is a measurement and an integrated bass clock is accumulated state. Those need different types, lifetime rules and interpolation. A bigger list of frequency bands alone will not repair that mismatch. Publish a compact, owned vocabulary such as `audio.mix.level`, `audio.kick.hit`, `transport.phase`, `clock.bass`, `env.impact` and `lfo.orbit` rather than adopting `syn_*` names with subtly incompatible semantics.

## B3. Focused additional references

| Product/reference | Verified relevant mechanism | Target compatibility and limits | What to borrow |
|---|---|---|---|
| Resolume | FFT can drive a parameter from selected spectral regions; gain/fall controls; level or movement-speed modes; reusable envelope shapes | Windows standalone; independent of Max. Exact installed version/driver must be tested | Parameter-level mapping and editable response shapes. [[L07]](sources.md#l07) [[L08]](sources.md#l08) |
| TouchDesigner `audioAnalysis` | Low/mid/high, kick/snare/rhythm indicators, centroid and slow/fast spectral-density outputs; disable unused analysis | Windows standalone analysis reference, not a Live 10 device | Feature gating and modular analysis. Do not claim kick accuracy without testing. [[L09]](sources.md#l09) |
| Magic Music Visuals | Ordered modifiers convert audio/MIDI/OSC features into useful parameter values; operations include smoothing, expressions, gating and holding | Windows standalone; MIDI/OSC features depend on edition | Mapping order must be explicit; values and modulation are different concepts. [[L10]](sources.md#l10) |
| Plane9 | Large predefined scene collection, scene combinations and transitions, standalone/reactive behavior | Legacy Windows application; Windows 11/current-driver support not verified; no Live requirement | Browse by visual identity and mix simple scene ingredients. Not a recommended code dependency. [[L11]](sources.md#l11) |
| Spettro | Developer listing describes Windows VST3/standalone shader visualization | Exact Live 10 certification unknown; inherited claim about an independent renderer not re-established here | Trial candidate and architecture questions; do not rely on editor-close behavior without testing. [[L13]](sources.md#l13) |
| px-stream | Open-source compact M4L/ISF experiments, transitions and feedback | **Live 12.2/Max 9.0.7–9.0.9**, explicitly WIP; not target-compatible | Read source and metadata patterns, not a ready Live 10 replacement. [[L12]](sources.md#l12) |
| Visibox | Performer-oriented triggering of visual/audio material as an instrument | Mac/Windows standalone; no native Live/Max dependency; present control/output details need trial | Setlist clarity, keyboard/pedal operation and fast first performance. It is not an MIR research engine. [[L14]](sources.md#l14) [[L15]](sources.md#l15) |

[Inference] Kaleidoscope-style applications provide a useful category lesson, not a requirement for another dependency: one source, a polar fold and a tightly constrained palette can generate many coherent looks. The relevant implementation fits your existing fragment-shader engine. No additional current Live 10-native commercial device was sufficiently verified to justify another “definitely compatible” recommendation. That is a boundary of this investigation, not a claim that none exists.

# C. Audio-reactive analysis: it is not just frequencies

## C1. Organize analysis by musical purpose

[Inference / recommended design] There are four different questions:

1. **How much activity is present?** Levels, spectral distribution and sustained presence.
2. **Did something happen?** A MIDI note, transient, clip launch or threshold crossing.
3. **Where are we in musical time?** Transport position, beat phase, meter, bar and launch quantization.
4. **What larger change is developing?** A buildup, loss of bass, section transition or return after silence.

Those questions operate at different time scales. A 5 ms envelope, a 20 ms onset detector, a quarter-note phase and an eight-second activity reference cannot be represented as interchangeable numbers. The analysis API must include units, validity, time support and reset behavior. A feature name without those semantics becomes a future compatibility problem for every preset.

For this project the most valuable hierarchy is **MIDI/known stem identity → per-track detection → full-mix heuristics → optional ML**. On a kick track, the track label supplies information that no full-mix classifier needs to rediscover. The classifier can still detect noise or miss ghost notes; an actual MIDI note provides a stronger event source when it exists.

## C2. Spectral front end, windows and latency

[Inference / proposed defaults] Analyze at the host sample rate; do not resample merely to make an FFT size look familiar. Use a periodic Hann window and precomputed plans/filter weights. At 48 kHz:

| Analysis path | FFT / hop | Window duration | Update interval | Purpose |
|---|---:|---:|---:|---|
| Fast time-domain envelope | No FFT; sample/block updates | Adjustable 3–10 ms attack | Audio/block rate | Immediate impact and stem activity |
| Short transient spectrum | 1024 / 128 | 21.33 ms | 2.67 ms | Mid/high spectral change |
| Main descriptive spectrum | 2048 / 256 | 42.67 ms | 5.33 ms | Bands, centroid, flatness, visual spectrum |
| Optional high-resolution diagnostic | 4096 / 512 | 85.33 ms | 10.67 ms | Pitch-oriented or slow display analysis, not fast hits |

The numbers follow `window_ms = 1000*N/fs` and `hop_ms = 1000*H/fs`. **A 42.67 ms window does not mean every detected change has exactly 42.67 ms latency.** A causal window contains the recent past; its taper, signal location and decision rule determine when a change becomes observable. Conversely, labeling a trailing window by its center does not make its output available earlier. Record the ending sample index and window length, and measure response to impulses and real drums.

Use stereo power, not a mono sum, for general activity: `(L²+R²)/2` remains active for antiphase stereo. For FFT bands, average left/right spectral powers. A mono `(L+R)/2` analysis can be a selectable option when phase cancellation is intentional, not the default.

A one-sided energy spectrum needs the DC/Nyquist exception: double interior-bin power, not the endpoints. Normalize by FFT/window conventions once, and test that a steady sine’s measured level does not change when the analysis FFT size changes. Keep waveform rendering separate from power measurement.

**Band choices.** Use six coarse power regions with proposed edges **20, 60, 150, 400, 2,000, 6,000 and 16,000 Hz**, clamped to Nyquist. Add thirty-two log-spaced display/control bands from 30 Hz to the lesser of 16 kHz and Nyquist. Do not claim thirty-two independently resolved low-frequency measurements: a 2048-point FFT at 48 kHz has approximately 23.44 Hz bin spacing. Narrow low-end controls can use filters or a longer window when resolution matters more than onset speed.

Log-frequency bands are convenient for visual structure because equal ratios receive similar visual space. Mel and Bark groupings are useful perceptual models, not automatic improvements in visual quality. Third-octave grouping is interpretable but comparatively coarse. The inspected products range from a few named bands to selectable spectrum regions; exact proprietary FFT configurations are generally not disclosed. Synesthesia’s four named regions, VS’s selectable audio regions, and TouchDesigner’s mixed feature set are different design choices, not evidence for one mandatory professional band count. [[L05]](sources.md#l05) [[V02]](sources.md#v02) [[L09]](sources.md#l09)

**Cost.** FFT work grows approximately with `N log N`, but CPU percentages cannot be inferred from that scaling without a build, processor and scheduling measurement. Run the richer spectrum on the mix or selected sources, not automatically on every stem. A kick-only source can often use a band envelope and event detector. Proposed budget: analysis worker P99 below 1 ms per main hop for one rich stereo source on the test machine; this is a release gate to measure, not a forecast of current performance.

## C3. Envelope following, dB and loudness

[Inference / proposed implementation] Keep both a fast peak-like envelope and an RMS/power envelope. For a target measurement `x`, use `a = exp(-dt/tau)` and `y = a*y + (1-a)*x`, choosing `tau_attack` when `x > y` and `tau_release` otherwise. For RMS behavior smooth power and take its square root; smoothing absolute amplitude is not the same statistic. Start with 5 ms attack / 120 ms release for activity, then let mappings add their own artistic smoothing.

Define dB consistently: `20*log10(max(rms, eps))` for amplitude and `10*log10(max(power, eps))` for power. Use a finite floor, such as −120 dBFS, and reject NaN/infinite input before it reaches normalization. Preserve raw dBFS in diagnostics so users can distinguish quiet audio from a low modulation amount.

[Verified] EBU-mode momentary loudness uses a 400 ms window; short-term uses three seconds. These are useful slow energy references, not kick triggers. A custom RMS envelope must not be labeled LUFS unless the required loudness weighting/channel treatment is implemented. [[D15]](sources.md#d15)

[Inference] Add a proper loudness meter later only if it solves calibration or content-reporting needs. For v1, raw RMS dBFS plus well-behaved adaptive scaling is more useful than a nominal “LUFS” control with nonstandard ballistics. A slow loudness-derived feature may drive density, but should not delay the fast impact path.

## C4. Adaptive normalization — solve responsiveness without erasing the music

The requirement is not literally to make silence and a drop equally bright. It is to preserve usable control range across different tracks while preserving **contrast inside a track**. Any automatic gain rule can fail when it interprets intentional quietness as a calibration error. The solution is multiple measurements and bounded adaptation, not one ever-changing divisor.

### Separate three signals

[Inference / specification] Publish:

- **Absolute activity:** raw dBFS and a fixed calibrated 0–1 mapping. This preserves the difference between a breakdown and a drop.
- **Relative activity:** normalized against recent non-silent material. This keeps a quiet recording usable.
- **Event evidence:** independently normalized spectral/envelope change. It must not fire because the visual normalization gain moved.

Presets should use relative activity for texture and motion, absolute activity/trend for section contrast, and events for impacts. Provide an `Audio Response` trim and a calibration lock outside preset state. The calibration belongs to the input/session; loading a different visual look should not silently retune the detector.

### Compare normalization strategies

| Strategy | Benefit | Failure mode | Verdict |
|---|---|---|---|
| Fixed gain | Predictable; preserves dynamics | Different masters react differently | Keep as manual/locked mode |
| Running min/max | Very cheap and apparently adaptive | One outlier dominates; minimum follows silence/noise | Do not use naïvely |
| Slow-release peak reference | Responsive upper reference; easy to implement | A drop depresses the following quiet section until recovery | Good first incremental improvement with a floor |
| Sliding percentiles | Robust against isolated peaks; exposes usable range | Needs history and startup policy; adaptation can still erase contrast | Recommended balanced mode |
| Per-track calibrated reference | Stable musical roles | Requires setup/track identity | Best stage mode after soundcheck |
| Learned global “energy” model | Potential semantic context | Extra latency, licensing and failure modes | Not needed for v1 |

### Concrete bounded percentile design

[Inference / proposed defaults] Sample each band’s pre-normalization log energy into an **8-second, 100 Hz** history. A fixed histogram over −100 to +12 dBFS, with a ring of histogram-bin indices, gives bounded memory and avoids sorting or allocation in the audio callback. Update statistics at 10 Hz on the analysis worker; do not sort 800 samples on every audio sample. Exclude gate-closed observations rather than treating silence as a legitimate lower percentile.

Estimate `Q20` and `Q95`. Let the desired upper reference be `U* = Q95 + 3 dB`. Let desired width be `W* = max(24 dB, U* - Q20)`, optionally capped at 60 dB. Follow rising `U*` quickly (150 ms time constant) to avoid prolonged saturation. Let `U` fall no faster than **2 dB/s**, so a quiet gap does not cause immediate visual gain pumping. Smooth width on a multi-second time scale and enforce `W >= 24 dB` after every update. Compute `z = clamp((dB - (U-W))/W, 0, 1)`; apply a gentle response curve only after this normalization.

This asymmetry is deliberate: a higher reference reduces gain quickly; a falling reference increases it cautiously. The corresponding implementation must name that direction clearly—“attack” and “release” labels on AGC gain are otherwise easy to reverse.

Proposed default gate: open above **−60 dBFS**, close below **−66 dBFS** with 200 ms hold, configurable for the real noise floor. These are calibration defaults, not claims that all music below −60 dBFS is irrelevant. A quiet microphone, ambient stem or upstream attenuated track needs a different floor or manual trim. When the gate is closed, freeze the normalizer’s references and decay visual activity to zero; do not learn the room noise upward. Retain the most recent valid reference across short silence.

At startup, show `Calibrating` for two seconds. Initialize from a conservative fixed reference, then permit one bounded transition to the measured range. A user-initiated `Recalibrate` may deliberately reset history; a preset change may not. For severe level shifts, recovery can take many seconds because history aging and gain-rise limits both apply. Offer `Locked`, `Balanced` and `Ambient` modes instead of pretending one time constant is optimal for every performance.

### Prevent loss of musical contrast

[Inference] Derive `energyTrend` from pre-normalization log energy, for example a difference between 0.5-second and 4-second smoothers, mapped over a stated dB range. Derive `presence` from a slower gated activity measure. A quiet section can therefore remain physically darker while its local detail still moves expressively. Use a macro to mix absolute and relative influence, rather than compressing the analysis into a single undocumented “intensity.”

Per-band normalization also needs limits. Independently maximizing an almost absent high band can make tape hiss look as important as the bass. Keep a minimum band activity threshold relative to total power and publish a band-validity/relative-energy measure. Optionally blend band-specific normalization with a shared overall reference. A high-frequency band with 0.1% of total energy should not gain arbitrary authority merely because it is locally variable.

### Test cases that distinguish a real solution from a demo

[Inference / acceptance tests] Feed silence; very quiet noise; a steady tone; the same drum loop at several gains; quiet material after a loud drop; bass-heavy material with negligible highs; antiphase stereo; and sudden source switching. Independently calibrating two gain-scaled copies should yield similar relative envelopes, while their absolute dBFS remains different. During silence, normalized levels must settle to zero and event counts stay zero. After a source switch, references must either transfer by an explicit policy or recalibrate visibly—never produce a full-screen flash because an old divisor was small.

## C5. Onsets and transient detection

[Verified] FluCoMa offers several spectral onset metrics; its documentation recommends a normalized metric as a practical starting point and also describes spectral flux, energy and high-frequency-content alternatives. SuperFlux’s reference work specifically addresses false detections from spectral movement such as vibrato. These are algorithm references, not evidence that every kick/snare role can be reliably separated from a full mix. [[D03]](sources.md#d03) [[D06]](sources.md#d06)

[Inference / engineering comparison]

| Method | Core evidence | Strength | Main failure / latency consideration |
|---|---|---|---|
| Envelope slope / fast–slow difference | Positive rise in band activity | Cheap, excellent on known drum stems | Bass notes, compressors and tremolo can look like hits |
| Spectral flux | Sum of positive spectral changes | Broad general-purpose onset evidence | Vibrato and moving harmonics can false-trigger |
| SuperFlux-style comparison | Compare with a frequency-neighborhood maximum from the prior spectrum | Reduces small spectral-shift false alarms | More tuning; a wide neighborhood can hide real changes |
| High-frequency content | Frequency-weighted energy/change | Sharp attacks and noisy percussion | Cymbal beds and bright sustained noise dominate |
| Complex-domain detection | Difference from predicted magnitude/phase | Can detect attacks not dominated by energy rise | More state/phase handling; low-energy phase is unstable |
| Neural onset/beat model | Learned temporal pattern | Potentially stronger musical discrimination | Context, model rights and streaming delay must be established |

### A causal v1 detector

[Inference / proposed algorithm] Compute a short-window magnitude spectrum and a log-frequency analysis representation. Apply bounded per-bin whitening or slow spectral normalization. Let `L_t[b] = log(1 + k*M_t[b])`. Form `D_t = sum_b max(0, L_t[b] - max(L_(t-1)[b-1:b+1]))` for the chosen region, with edge handling and normalization by contributing bins. This is a **SuperFlux-inspired implementation proposal**, not a claim of bit-identical replication of the paper or a license to copy unlicensed reference code.

Threshold against recent history: `T = median(D_history) + kMAD * MAD(D_history) + floor`. Begin with one second of history, `kMAD = 3`, and a calibrated nonzero floor. Exclude the newest decision frames from the reference statistic so a transient does not raise its own threshold before it can be detected. Require a rising crossing and local prominence. An optional one-hop delayed peak decision costs one additional hop; an immediate crossing mode is faster but less selective.

Use separate refractory windows: proposed **90 ms bass, 70 ms mid, 45 ms high**, adjustable. Do not enforce a global 150 ms lockout that erases fast hi-hats. Use hysteresis/re-arm criteria so a long transient tail cannot trigger repeatedly. Trigger strength can be based on threshold exceedance, then compressed into 0–1; it is not the raw FFT magnitude.

For bass impacts on a known kick source, combine this with a fast/slow low-band envelope or use MIDI directly. The 1024-point spectrum has limited bass resolution. Label full-mix outputs `bassTransient`, `midTransient`, `highTransient`; only label a source `kick` when routing or a qualified classifier supplies that meaning.

**Event semantics are as important as detection.** Send an event ID, source, sample timestamp, strength and clock epoch. A one-frame float set to one can be lost between a 120 Hz sender and a 60 Hz renderer. An event queue can preserve it and create an envelope at the correct age. Deduplicate retransmissions by ID. Do not replay a half-second backlog as a burst after a GPU stall.

## C6. Beat, tempo and the Ableton advantage

[Verified] Max 8’s `plugphasor~` provides an audio-rate ramp synchronized to the host beat; `plugsync~` reports transport information with validity flags. This is a stronger synchronization primitive than assuming that UI-level `live.object` polling is sample-accurate. The brief’s current transport implementation is reported to work well; replacing its polling is an accuracy/robustness refinement, not a claim that the existing result is unusable. [[H04]](sources.md#h04) [[H05]](sources.md#h05) [[H07]](sources.md#h07)

[Inference] Use host transport for tempo, beat/bar phase and preset quantization. Use audio onsets for expressive accents, syncopation and transient strength. Do not let a beat detector fight the DAW clock. Only enable automatic tempo tracking when the engine is driven by external unsynchronized audio; expose its confidence and a manual tap/tempo fallback.

**Libraries and why they are optional.** [Verified] BTrack supplies a C++/Max route but is GPL-3.0. aubio is also GPL-3.0. madmom separates BSD code from non-commercial model assets. Essentia offers an AGPL/commercial route and separately licensed models/dependencies. Beat This! publishes MIT code and weights, but its file-oriented inference is not by itself evidence of bounded causal streaming. These distinctions matter more than choosing the most fashionable beat estimator for a host that already supplies the beat. [[D10]](sources.md#d10) [[D09]](sources.md#d09) [[D11]](sources.md#d11) [[D12]](sources.md#d12) [[D13]](sources.md#d13)

### MIDI and Live API are complementary, not equivalent

[Inference] A MIDI note intercepted before the instrument avoids audio onset-estimation delay. It does **not** make photons appear with zero latency: Live scheduling, IPC, frame boundaries and scanout remain. A plugin can receive note events with within-block sample offsets when the host supplies them. A Max MIDI device’s scheduler path should be measured rather than assumed to preserve the same timestamp precision automatically.

Reading stored clip notes through the Live API is useful for planning, thumbnails and scene logic. It is not the same as observing actual playback: launch quantization, loops, clip edits, mute, follow actions and transport jumps change what is heard. Prefer actual played MIDI for per-hit triggers. Use Live API observers for slower semantic state such as active clip, track identity and scene intent; do not poll every track name on every frame.

A sidechain audio bus also does not provide arbitrary access to every Live track. Each additional source must be routed or provided by a companion analyzer. Your existing thin per-track M4L pattern is therefore an advantage, not an architectural embarrassment.

## C7. Descriptors that are useful visually

[Verified] Spectral shape toolkits such as FluCoMa expose descriptors beyond band energy. The exact selected descriptors should be justified by a visual role, not by completeness of an MIR catalog. [[D05]](sources.md#d05)

[Inference / recommended mapping table]

| Feature | Definition/interpretation | Useful destination | Suggested timescale and caution |
|---|---|---|---|
| Centroid | Power- or magnitude-weighted frequency mean; choose and document one | Edge brightness, palette position, line fineness | 80–250 ms; log-map frequency; gate silence |
| Flatness | Geometric/arithmetic spectral mean ratio | Roughness, grain/noise contribution | 150–400 ms; denominator floor; not “musical quality” |
| Rolloff | Frequency below which a stated share, e.g. 85%, of energy lies | Detail bandwidth, spread of particles | 150–300 ms; can jump when spectrum changes |
| Spread | Weighted dispersion around centroid | Spatial width or turbulence | 150–500 ms; normalize by defined frequency range |
| Flux | Positive spectral change | Event evidence or restrained agitation | Separate short impact and smoothed activity paths |
| Zero-crossing rate | Sign changes per sample interval | Secondary noisiness cue | Poor standalone classifier; DC/noise sensitive |
| Chroma | Energy folded by pitch class | Twelve controlled color/shape choices | Prefer MIDI pitch when available; hysteresis and confidence |
| MFCCs | Compact cepstral timbre coordinates | Learned/curated timbre-to-look morph space | Do not map coefficient 7 to hue blindly; fit and validate a small manifold |
| Stereo balance/width | Channel energy and correlation-derived measures | Position, asymmetry | Optional; avoid antiphase cancellation and excessive camera sway |

Store units and weighting choices. For example, a centroid normalized linearly over 0–24 kHz spends little control travel in the musically occupied region. A log-scaled, clamped 200 Hz–8 kHz range is a more useful proposed artistic control, but it must not replace the raw measured centroid field.

## C8. Buildups, drops and section awareness

[Inference / v1.5 proposal] A useful lightweight section detector can combine pre-AGC energy slope, spectral brightening, increasing transient density and loss/return of bass. Compute each at its appropriate time scale. Use a novelty score over a small normalized feature vector to suggest a change, not to force a random preset every time a cymbal arrives.

A conservative state machine might have `steady → rising → candidateDrop → steady`, with minimum dwell times measured in bars. Confirm a candidate drop using a substantial energy/bass return after a preceding low-activity or rising section. Add hysteresis and a cooldown of several bars. Save evidence and confidence in the debug log. These heuristics will misclassify some genres; the fallback is a visually harmless intensity change, not an irreversible scene jump.

Automatic preset changes should be opt-in and restricted to a curated setlist. The performer can arm the next scene; the detector supplies timing suggestions. This is a better default than asking uncertain inference to choose both artistic content and transition time.

## C9. ML and real-time image generation: what the evidence actually supports

**Causal separation.** [Verified, paper measurement] HS-TasNet reports 23 ms algorithmic latency. Its small model has 16 million parameters; the paper’s table reports approximately **3.98 ms on one CPU core, 1.83 ms on four cores, and 2.10 ms on the tested GPU** for its processing measurement. The hardware is an Intel i7-12850HX and NVIDIA RTX 3080 Ti. The larger model reports different costs. These are not Windows end-to-end figures, and model inference time must not be confused with the analysis window or output-device delay. [[M01]](sources.md#m01) [[M02]](sources.md#m02)

[Inference] This establishes that low-latency separation is more than a conceptual demo. It does not establish that a legally distributable, trained, stateful Windows build is ready to drop into your product. The availability/license of exact trained weights, warm-up, runtime, worst-case scheduling, GPU contention and failure behavior remain adoption gates. On known Live stems it also solves a problem you do not have. Keep a separation adapter behind an optional external-source mode; do not make startup or basic triggering depend on it.

**Demucs and neural beat tools.** [Inference grounded in the referenced implementations] Offline separation quality and “faster than real time” throughput do not imply low streaming latency. A model can process a minute of audio quickly while still needing seconds of context. Inspect causal structure and lookahead explicitly before calling it live-compatible. Beat This! is useful as an offline comparison/annotation tool; BTrack/aubio are simpler streaming references but have copyleft licensing concerns for embedding. [[M06]](sources.md#m06) [[D13]](sources.md#d13) [[D10]](sources.md#d10) [[D09]](sources.md#d09)

**Diffusion.** [Verified, project benchmark] StreamDiffusion’s published table uses an RTX 4090, i9-13900K and Ubuntu 22.04.3. It reports roughly 106 fps text-to-image and 94 fps image-to-image for the stated one-step SD-turbo configuration, versus approximately 38/37 fps for the stated four-step configuration. That is throughput under particular conditions—not proof of low command-to-photon latency, temporal consistency, 1080p rendering or equivalent Windows performance. The pipeline is Apache-2.0; selected model licenses still require separate review. [[M03]](sources.md#m03)

[Verified] Current StreamDiffusionTD documentation describes a 0.4.1 maintenance release over 0.4.0, TensorRT/CUDA integration, combined conditioning and GPU-to-GPU transport; a dated 0.3.1 update is visible in April 2026. This is a real maintained integration path, not merely a 2023 proof of concept. Its performance and exact operator/model licensing were not qualified on this project’s machine. [[M04]](sources.md#m04) [[M05]](sources.md#m05)

**Verdict.** [Inference] Production-usable for this v1: deterministic shaders, track-aware envelopes, onsets, host tempo and MIDI. Production-possible but optional: carefully profiled causal separation for external mixed audio. Useful exploratory/secondary-output technology: diffusion with a deterministic fallback and generous GPU headroom. Not an acceptable default: requiring generative inference to hit every kick, guaranteeing semantic prompt changes within one frame, or claiming a throughput headline is a latency guarantee. The safest near-term ML use is offline creation of licensed textures or assisting preset curation, not occupying the same GPU budget as the stage-critical renderer.

# D. Mapping and modulation: make the result musical

## D1. Typed sources and a deliberately small matrix

[Inference / recommended design] Implement a bounded directed modulation graph. Source types are continuous unipolar values, bipolar values, phase, accumulated clocks, and discrete events. Destinations are continuous parameters, enums, trigger actions and macro bases. Type errors must fail validation: an event cannot be treated as an indefinitely high level; a hue/phase cannot be linearly smoothed across wrap without an explicit cyclic mode.

A route should specify source, destination, amount, input interval, curve, polarity, attack/release smoothing and optional quantization. Keep routing in normalized parameter space, then convert to physical units at the final boundary. The detailed contract in the engineering spec deliberately restricts v1 to additive contributions and clamps once after summation. This avoids ambiguous route-order behavior; a later multiplicative stage must be explicit rather than silently changing existing presets.

The useful lesson from VS is typed synth modulation; from Magic it is explicit operation order; from Arkestra it is semantic macros. These products differ in implementation, so the proposal below is not represented as their undocumented shared algorithm. [[V02]](sources.md#v02) [[L10]](sources.md#l10) [[A04]](sources.md#a04)

## D2. Continuous motion versus events

[Inference] Continuous values describe sustained behavior: bass opens a shape, centroid increases fine detail, a slow clock rotates the composition. An event starts an owned visual action: a kick launches a ring, a snare advances a palette choice, a note seeds a particle burst. The event’s envelope determines duration even after the detector returns to zero.

Use an attack/decay envelope with a proposed 3–5 ms attack and 120–250 ms decay for impact. For a sustained note use ADSR with actual note-off semantics. An onset cannot directly supply a sustain duration; faking note-off after a fixed period is an explicit policy, not ADSR information extracted from audio.

Keep one visually dominant rhythmic response. A useful default hierarchy is kick→large form/brief energy, snare→small discrete accent, hats→fine texture, transport→continuous movement. Not every preset needs all four. Silence should still leave an attractive low-motion image, or fade to black according to an explicit performance setting.

## D3. Four visible macros, eight stable host slots

[Inference / proposed UX] Show four macros consistently: **Intensity, Motion, Color, Space**. Reserve stable host IDs `macro.1`–`macro.8`; use the first four in the minimum UI and reveal the rest only when necessary. A display caption can change, but the automation ID cannot change with a preset.

One macro may alter several normalized destinations with different signs/curves. For example, increasing Space can enlarge radius while reducing object count and increasing empty margins. Intensity should not merely multiply output RGB until everything clips. Let it change impact depth, emissive gain and density within a curated range. Color should move along a palette or transition between named palette roles, not necessarily sweep the entire hue wheel.

Manual control, host automation and MIDI must set the same **base macro value**. Modulation remains a separate layer. Otherwise a returning MIDI value overwrites a running LFO or automation records jittering modulated values instead of the performer’s intention.

## D4. Switching, morphing and transitions

[Inference] A preset switch has two decisions: **when** to switch and **how** to show it. Queue a request for the next beat/bar while preloading shaders and assets. Quantization uses host PPQ/meter, including tempo changes and transport jumps. For live hits, do not quantize by default; imposing a beat grid on a human snare destroys the performance.

Begin with a hard cut, a linear-light crossfade, a dip-to-background and a luma wipe. A linear-light crossfade uses `(1-t)A+tB`; audio-style equal-power curves are not automatically appropriate for images because overlapping bright fields can gain luminance. Morph parameters only when the source/destination graphs and semantics are compatible. Crossfade unrelated graphs rather than pretending a fluid state can morph meaningfully into a torus.

Retargeting a transition must not accumulate unbounded renderers. Your existing mid-fade behavior should be preserved with a strict policy: snapshot the current composite, retire the old pair, and transition from that snapshot to the newest destination. This trades a brief frozen transition source for bounded GPU/memory cost. Prewarm persistent simulations off-screen within a budget; a new reaction–diffusion field should not start as an uninteresting blank.

## D5. Anti-patterns to exclude

[Inference] Raw FFT noise on position; all bands mapped to every destination; high-rate random hue; extreme bloom covering weak composition; frame-dependent feedback; automatic normalization of silence; unbounded camera motion; accidental MIDI jumps after preset loading; and transitions that briefly require three expensive scenes. These are not stylistic sins in all art, but they are poor defaults for a dependable musician-facing instrument.

The supplied schema separates render stages, parameter metadata, modulators, macro mappings, routes, triggers and transitions. The worked Pulse Halo preset demonstrates those structures without requiring a large material editor. It is a design contract for the next version; it will not load in the current prototype without the specified migration layer.

# E. MIDI control design and Live 10 routing reality

## E1. Host facts that change the decision

[Verified] Live 10 gained VST3 support in **10.1**. Its **10.1.30** notes explicitly fix MIDI CC reception and MIDI Learn in certain VST3 audio effects. The **10.1.25** notes add VST3 MIDI-output message support for the stated SDK generation. Therefore “Live 10 supports VST3” is insufficiently precise: qualify **10.1.43** as the intended test baseline, while retaining a record of the user’s actual installation. [[H01]](sources.md#h01)

[Inference] A VST3 audio effect can be a good eventual carrier for audio analysis, parameters and event input, provided its event buses and controller mapping are implemented correctly. VST3 does not expose every MIDI concept as an arbitrary raw byte stream in the same way as an external MIDI port. Test notes, CC, pitch bend, channel pressure, automation and sidechain independently; one successful note does not establish them all.

## E2. Routing matrix

| Vehicle | How audio arrives | How notes/CC arrive | Best use here / limitation |
|---|---|---|---|
| M4L audio effect on an audio track | The track’s audio through `plugin~` | Live-mapped parameters work; do not assume the audio track supplies a general note stream to the device | Keep the analyzer; add a companion M4L MIDI device for note events |
| M4L MIDI effect before an instrument | No audio at that position | Played notes and MIDI control through the MIDI device chain | Clean event bridge; preserve pass-through so the instrument still plays |
| VST3 audio effect with an event input | Audio main bus and declared/routed sidechain | A separate MIDI track can target the effect when Live exposes its input; verify actual routing | Event-aware analyzer without moving audio; use 10.1.30+ fixes |
| VST3 instrument | MIDI track/instrument context; audio-input capabilities depend on supported buses/host routing | Normal note/event path | Natural for visual polyphony; less natural as a drop-on-any-audio-track analyzer |
| “VST3 MIDI-only effect” | Not a general substitute for native M4L MIDI effects in Live | Host-specific behavior | Do not promise a standalone VST3 MIDI-effect category; test a declared instrument/effect wrapper |
| Standalone engine with virtual MIDI port | Separate audio/feature connection | External MIDI routing through a virtual port or direct controller ownership | Useful fallback; adds setup and possible duplicate/feedback routing |

The Max MIDI/audio distinction is grounded in the device model and historical Cycling ’74 discussion; plugin output routing is documented by Ableton. Exact current-plugin input menus still require a Live 10 test. [[H08]](sources.md#h08) [[H06]](sources.md#h06)

[Inference / lowest-friction v1] Hardware knobs use Live MIDI Map to the analyzer/control device’s macros. A small **VJ MIDI Bridge.amxd** on a MIDI track forwards performed notes and selected CCs to the engine, while passing MIDI through to an instrument. One companion device is less work than migrating the entire product merely to obtain direct MIDI. Later, a VST3 wrapper can remove Max as a distribution requirement without replacing the renderer.

Do not assume channel identity survives every Live routing combination unchanged. Ableton documents channel handling limitations in its internal MIDI paths. Use explicit source instances/roles rather than packing sixteen unrelated stems into MIDI channels and expecting universal preservation. [[H06]](sources.md#h06)

## E3. Learn, pickup and high-resolution controls

[Inference / specification] Learn mode highlights a target, listens only to allowed message types and shows the captured device/port/channel/controller. Require confirmation when a source is already assigned elsewhere. Support one control driving a macro with many destinations; do not duplicate hidden per-parameter learn rules unnecessarily. Save mappings in a controller profile and base values in Live-set state.

For absolute CC, implement pickup: after a preset recall, ignore movement until the hardware crosses the stored value or enters a small deadband. Display a direction arrow. For endless encoders support explicitly selected relative encodings (two’s complement, sign/magnitude and offset/binary styles); automatic detection must be confirmed because low values can be ambiguous. Encoders should change a normalized value by a configurable increment, with optional acceleration that can be disabled.

Seven-bit CC is adequate for most stage macros after smoothing. Optional 14-bit CC pairs and NRPN require proper message assembly, timeouts and channel state; they are not a reason to postpone the usable seven-bit path. Do not assume every VST3 wrapper/host forwards all raw NRPN sequences to plugin learn. The M4L MIDI bridge or standalone port can be the more predictable route for advanced controller protocols.

LED feedback must originate from the authoritative post-recall **base state**, not echo arbitrary incoming MIDI. Rate-limit feedback, suppress loops, and use an explicit controller profile. Incoming messages caused by feedback should not re-trigger actions. A controller with a non-motorized fader cannot physically follow a recalled value; LEDs/display and pickup must resolve that mismatch.

## E4. Concrete controller layout

[Inference / design choice] Use a **Launch Control XL MK1/MK2-style generic MIDI template** as the reference layout, not a dependency on a current DAW integration script. Novation still provides official manuals and Windows 10/11 driver downloads for those models. The exact custom CC/note numbers below are **our proposed user template**, not claimed factory assignments. A newer XL generation must be treated as a separate profile. [[E09]](sources.md#e09)

The engineering spec assigns eight top-row knobs to stable macros, additional knobs to response/transition controls, faders to source amounts and master level, and buttons to eight preset slots plus transport-independent performance actions. Use MIDI channel 16 only as a profile convention; it is configurable and not relied upon to survive every Live internal route.

[Inference] APC-style grids and Launchpad-style pads suit preset banks; a MIDI Fighter Twister-style encoder bank suits continuous macro performance; Push can exploit Live’s device mapping. No current model’s bundled integration script is asserted to support Live 10 without a version check. These are form-factor comparisons, not a retail buying list.

**Current Windows risk.** [Verified] Resolume’s support material identifies MIDI trouble associated with Windows MIDI 2.0 updates and gives driver-level troubleshooting. This does not prove your system is affected, but it is a concrete reason to record OS build, MIDI driver and controller firmware in the qualification manifest and avoid untested pre-show updates. Do not disable security updates as a blanket fix; qualify changes on a non-show image first. [[E10]](sources.md#e10)

# F. Reaching a premium look in code

## F1. A visual hierarchy, not a shopping list of effects

[Inference] An attractive preset should still compose a good frame with the audio disconnected. Establish a dominant shape, a secondary texture and a restrained finishing layer. Give rhythm to the dominant shape, timbre to texture, and longer musical phrases to color or camera. Do not map the same raw bass envelope to radius, brightness, rotation, noise scale and bloom simultaneously. That produces a visibly correlated meter rather than a visual instrument.

The inspected T3X2R and Photism stills support two economical directions: legible wire geometry with negative space, and dense organic contours constrained by symmetry. They do not establish frame rates or the exact original shader implementation. [[T05]](sources.md#t05) [[P07]](sources.md#p07)

All techniques below can run in the **external Windows renderer independently of the Live/Max version**. Availability inside Max 8 depends on the selected GL engine and packages. The proposed baseline is desktop OpenGL 3.3-class hardware with a legacy-shader compatibility path; compute shaders are an optional later capability, not a v1 requirement. GPU costs in this section are **engineering allocation targets**, never measurements on an unspecified “NVIDIA-class” GPU.

## F2. Geometry and procedural fields

| Technique | Aesthetic contribution | Implementation and first useful limits | Main cost / failure mode |
|---|---|---|---|
| 2D signed-distance shapes | Clean rings, bands, masks, repeated symbols | Evaluate distance in aspect-correct coordinates; derive antialias width from derivatives; animate a few shape parameters | One full-screen pass; hard thresholds alias and oversized glow erases geometry |
| Parametric mesh | Wire spheres, toroids, ribbons, orderly spectrum forms | Grid of angles → positions/normals; vertex displacement from a smoothed spectrum texture; line/point/solid variants | Vertex count and translucent overdraw; thin lines flicker without a suitable AA strategy |
| 3D SDF raymarching | Sculptural glowing objects, soft unions, tunnels | Conservative sphere tracing, bounded scene, 32–64 starting steps, finite max distance; normals from distance gradients | Cost scales with pixels × steps × distance evaluations; shadow/AO marches multiply work |
| fBm/domain warping | Marble, silk, liquid-looking folds | Sum 3–5 noise octaves; use two low-frequency noise outputs to deform the coordinates of another field | Nested noise grows rapidly; evaluate at reduced resolution before adding unnecessary octaves |
| Polar folding / kaleidoscope | Coherent radial symmetry and musical repetition | Convert to radius/angle, fold angle into a wedge, transform back; soft seam treatment | Generally cheap; rapid segment-count changes pop unless deliberately triggered |
| Fractals / repeated domains | Tunnels, recursive ornament, crystalline detail | Start with 2D Julia or limited repeated SDFs; keep explicit iteration/escape limits | Unbounded iteration, singularities and temporal shimmer; not every complex image needs a fractal |

[Inference / original snippets] A clean ring needs no 3D renderer:

```glsl
vec2 p = (2.0 * gl_FragCoord.xy - resolution.xy) / resolution.y;
float d = abs(length(p) - radius) - thickness;
float aa = max(fwidth(d), 1.0 / resolution.y);
float coverage = 1.0 - smoothstep(-aa, aa, d);
float halo = exp(-max(d, 0.0) * haloFalloff);
vec3 linearColor = background + accent * (coverage * emission + halo * glow);
```

Domain warping means evaluating a field in a deformed coordinate system, for example `q = p + warp * vec2(noise(p+s1), noise(p+s2)); value = fbm(q)`. Two layers of carefully bounded warping often give enough depth. A curl-like 2D flow uses a scalar potential `psi`: `v = (dpsi/dy, -dpsi/dx)`. Analytic derivatives are preferable when available; finite differences require more field evaluations. The permissively licensed Gustavson/Ashima sources provide a practical noise starting point and explicitly cover legacy GLSL compatibility. Retain their notices when copying source. [[G15]](sources.md#g15)

For a smooth SDF union, a polynomial blend is easy to implement, but it is not a guarantee that a deformed field remains a true distance bound. A raymarcher must use conservative steps, an iteration cap and a miss color. Treat domain warping plus aggressive sphere-tracing steps as a correctness problem, not just an artistic control.

## F3. Feedback, state and simulation

[Inference] Feedback is the most valuable near-term extension because persistent buffers already exist in your engine. Read yesterday’s image through a slightly transformed coordinate, decay it, then inject today’s geometry. Write to a different texture; reading and writing the same attached texture is not a valid substitute for ping-ponging. Express decay as a time constant: `retain = exp(-dt / trailSeconds)`, not an unexplained constant per frame. At 30 and 60 fps, a nominal one-second trail should last roughly the same time.

A zoom/rotate feedback stage can use `uvPrev = inverseTransform(uv, dt)`, followed by a controlled blend or additive injection. Prevent runaway brightness in linear HDR with explicit dissipation, finite-value checks and bounded exposure. Do not clamp all history to 0–1 before bloom if luminous trails are part of the look. Reset persistent state on a declared seed/reset event, resolution change or incompatible graph load—not on every macro change.

**Reaction–diffusion.** [Inference] A Gray–Scott-style two-field solver evolves concentrations U and V with diffusion, reaction `U*V*V`, feed and removal. Use explicit ping-pong RG16F or RG32F state, a stable discretization and deliberately small simulation steps. The eye sees the contour extraction and palette, so simulate at 256² or 512² first, upscale and shade the concentration field. Feed/kill are artistically sensitive; expose a bounded “morphology” macro along a curated parameter path rather than allowing arbitrary unstable combinations. Photism explicitly names this family for Morphogen, which supports the technique comparison without revealing its exact implementation. [[P04]](sources.md#p04)

**Particles.** [Inference] OpenGL 3.3 can update positions and velocities through texture ping-pong or transform feedback; a compute rewrite is not necessary for an initial 16k–64k-particle look. Separate simulation count from draw size. Billboard size, additive overlap and fill rate can matter more than particle count. Use seeded spawn rules, bounded lifetime and an audio-triggered injection budget so a dense drum roll cannot allocate an unbounded number of particles.

**Fluid.** [Verified] GPU Gems describes a GPU implementation of advection, force application, divergence, pressure solution and projection; the WebGL Fluid Simulation repository provides MIT-licensed code worth studying. These are algorithm/code references, not a promise that their host framework belongs in your JUCE app. [[G04]](sources.md#g04) [[G14]](sources.md#g14)

[Inference] Begin with a 256² velocity grid, pressure iterations bounded to 12–20, and a separate dye grid at 512². Audio hits inject force/dye at curated locations. A pressure solver doing twenty full-grid iterations is fundamentally more work than one procedural color pass. Reuse the fluid velocity for particles only after the solver is stable. Treat cellular automata similarly: fixed-resolution state, explicit boundary rules, deterministic seed and bounded steps. Neither a fluid nor an automaton must advance once per rendered frame.

## F4. Color and the finishing pipeline

[Inference / proposed order]

```
input decode -> linear source/generator -> scene/effect chain in float
            -> linear-light preset transition -> bloom -> exposure/tone map
            -> display-referred grade -> restrained grain -> sRGB output
            -> master blackout gate -> Spout / projector / preview
```

Work in **linear RGBA16F** for ordinary internal color. An ISF pass declaring `FLOAT:true` has a stronger documented precision requirement: the format specifies **32-bit floating point per channel**. Allocate RGBA32F for those passes or explicitly report a compatibility limitation; do not advertise RGBA16F as exact compliance. ISF also defines audio-input textures and pass-size expressions that the prototype currently lacks. [[G01]](sources.md#g01)

Bloom should enhance an already well-composed source. A half/quarter/eighth-resolution downsample-and-upsample chain gives a soft glow economically. Use a soft threshold or bounded energy extraction, then add the result before tone mapping. A bright isolated ring can bloom beautifully; raising every pixel’s luminance merely lowers contrast. First measure the complete chain, including transitions and output copies, rather than timing a single shader in isolation.

A simple, explicitly named Reinhard-style operator is acceptable for v1. A popular small “ACES-like” rational curve is not the complete ACES color-management system, and using an AgX name without its input/output transforms is similarly misleading. Filament is a useful production source for studying real-time rendering and tone/color decisions, but its implementation and included assets must be inspected at the chosen revision. [[G06]](sources.md#g06)

[Inference] Adopt curated palettes with roles: background, shadow, body, accent and highlight. Interpolate palette stops in OKLab on the CPU, gamut-map or clamp deliberately, then upload a small lookup texture. OKLCH hue interpolation needs an explicit shortest-arc convention and handling for nearly neutral colors; blindly interpolating hue at negligible chroma can jump. The original OKLab publication defines the transform and explains its perceptual motivation. [[G05]](sources.md#g05)

A cosine palette is an inexpensive alternative: `a + b*cos(2*pi*(c*t+d))`. Curate the coefficients; arbitrary random coefficients do not guarantee good contrast or in-gamut color. Changes of palette are discrete performance events with 250–1000 ms color interpolation. Fast hue rotation on every hat usually reads as instability, not sophistication.

Use chromatic aberration sparingly near edges, vignette at low strength and grain that scales with output resolution. Distinguish intentional feedback trails from true motion blur. Temporal accumulation requires history invalidation on cuts, resizes and camera discontinuities; TAA additionally needs motion/jitter policy and is not a free antialiasing checkbox. Begin with derivative AA and restrained spatial supersampling for thin geometry before building a complete temporal reconstruction system.

## F5. Quantitative allocation, without invented GPU benchmarks

At 1920×1080, one RGBA16F texture contains **16.59 MB** of pixel data; at 3840×2160 it contains **66.36 MB**, excluding driver alignment/metadata. An RGBA32F target doubles those figures. Ping-pong pairs, two transition graphs, bloom and video textures multiply memory. 4K has four times as many pixels as 1080p, but actual runtime does not scale by a universal factor because fixed costs, bandwidth, occupancy and overdraw differ.

[Inference / qualification targets] Allocate **<8 ms GPU time per steady-state 1080p frame**, **<12 ms during a two-graph transition**, and leave the remainder of a 16.67 ms period for scheduling/output variance. These are goals for a recorded test GPU, not minimum requirements already established. An initial allocation could be 0.5–1.5 ms for simple geometry, 1–3 ms for procedural fields, 2–5 ms for expensive simulation/raymarching, and 1–2 ms for the finishing/output chain. Those categories overlap and cannot all spend their maximum simultaneously.

Measure 4K separately. The adaptive-quality ladder should reduce simulation resolution, bloom resolution and optional detail before reducing the frame rate or corrupting the intended composition. Never silently change the rate of beat events as a quality adjustment.

## F6. Twelve concrete starter designs

The companion **STARTER_PRESETS.md** specifies twelve designs in implementation order, with parameter ranges, four macros, continuous and event mappings, MIDI behavior, shader/state needs, quality budgets and tests. The first three—**Pulse Halo, Horizon Lines and Mosaic Tiles**—are the minimum lovable content set. Feedback Ribbons and Spectral Torus are the next visual identities. Fluid and raymarching are later options, not dependencies for calling the instrument useful.

# G. Architecture decision and migration

## G1. Four choices against the actual constraints

| Choice | Target compatibility | Strength | Main risk | Decision |
|---|---|---|---|---|
| Pure M4L + Max 8 Jitter | Viable in principle; package/GL/driver tests required | Direct Live controls, fast patch iteration | Replacing a working engine; shared host/resource behavior; package portability | A valid alternative, not the best migration |
| VST3 with editor-owned renderer | Live 10.1+ format support; plugin-specific tests required | Single install and familiar plugin UI | Editor lifetime, host windows/DPI, multiple instances, audio-host failure blast radius | Do not choose as the production default |
| Thin M4L/VST3 + external renderer | Matches working user-reported design | Isolation, fullscreen independent of editor, reusable renderer | IPC/state/lifecycle need explicit design | **Recommended** |
| Engine receives raw loopback audio only | Windows APIs available; host-independent | Standalone audio capture | Loses stems/host state, extra routing and latency, attribution ambiguity | Optional standalone mode, not primary Live path |

[Verified] Ableton’s Jitter authorization note restricts display **while editing** a device with the Live-only authorization; it does not describe a blanket ban on saved M4L visual devices. Max 8 developer reports establish a Windows gl3/Spout path for particular versions, although a forum success is not a compatibility matrix for every package. Max 8 therefore does not force VST3. [[H02]](sources.md#h02) [[H03]](sources.md#h03)

[Inference] A plugin-created top-level window can outlive an editor object if engineered that way, but it still normally belongs to the plugin/host process. Window separation and process separation are different properties. Do not claim VS has solved every JUCE lifecycle issue without its source or measurements. A hybrid design removes renderer-window lifetime from the audio host instead of demanding a perfect host-window abstraction.

## G2. Keep the renderer; make interfaces explicit

[Inference / recommended migration]

```
LIVE 10 / MAX 8                          EXTERNAL WINDOWS PROCESS
Audio track -> Audio Analyzer ----------> source registry + clock estimator
MIDI track  -> MIDI Bridge -------------> event scheduler + modulation graph
Control     -> Visual Instrument -------> preset/state coordinator
          feature/event/control IPC      |
                                         v
                                     render graph -> linear finish -> Spout
                                                  -> fullscreen / preview

Later: thin JUCE VST3 wrapper replaces or complements the three Live devices.
       AnalysisCore and the protocol remain the same; renderer does not move.
```

A source analyzer owns **observations**, the control device owns **performance state**, and the engine owns **GPU resources and presentation**. Multiple audio tracks must not each become competing authorities for preset selection. Give one conductor permission to change the output state; source devices publish named roles and features.

Use a stable protocol major version, sender/source identifiers, connection tokens and clock epochs. Preserve your existing OSC addresses in a legacy adapter so the prototype can continue to run while the new route graph is implemented. An adapter is cheaper than changing every M4L patch and preset in one commit.

## G3. IPC: reliability is semantic, not a transport label

| Mechanism | Recommended role | Benefits | Caveat |
|---|---|---|---|
| OSC over loopback UDP | Current and v1 feature snapshots; events with sequence IDs | Already working; easy Max interoperability | Datagram loss/reordering; no built-in state transactions |
| Acknowledged control messages | Preset load, state recall, output changes | Idempotent commands with explicit result | Implement retry, deduplication, timeout and transaction boundaries |
| Windows named pipe | Optional later C++ control/assets/state channel | Local IPC and clear connection lifetime | Blocking operations belong off the audio thread; framing still required |
| Shared-memory SPSC ring | Optional raw audio or high-rate bulk data | Avoids copies/socket overhead for suitable workloads | Synchronization, schema, ownership and crash recovery become your responsibility |
| Spout texture sharing | Completed GPU image | Keeps image exchange on the graphics path | Not a substitute for a documented, reliable control protocol |

OSC defines messages and bundles, while Windows documents named pipes, file mapping and other IPC primitives. Neither source automatically supplies application-level event idempotence or real-time safety. Those are design responsibilities. [[E07]](sources.md#e07) [[E08]](sources.md#e08)

[Inference] Send continuous features as coherent **latest-wins snapshots**. Do not replay a half-second backlog of obsolete envelope values after a stall. Send onsets/notes as **timestamped events** with IDs: duplicate packets must not generate duplicate flashes. Reliable state transactions can be acknowledged and retried; percussive events older than a musical expiry should be dropped with a diagnostic rather than arriving as delayed bursts.

Keep loopback binding as the default. A UDP “load file” command exposed to a network is not harmless just because the intended user is a musician. Validate packet sizes, finite numeric values, asset paths and command permissions. Use a per-session token and explicit remote-control opt-in. Tokens are not encryption, and local malware is outside the promise of this interface.

## G4. Where analysis runs

[Inference] For the next milestone, improve analysis inside Max 8 using built-ins or a pinned, tested FluCoMa path. Reuse stem audio already flowing through Live; do not route the whole mix through a Windows loopback just to avoid writing a Max object. When a common C++ AnalysisCore is justified, run it in a worker behind a preallocated audio FIFO within a thin plugin/external, not in the graphics process by default.

A worker introduces scheduling latency but protects the audio callback from FFT/percentile/serialization spikes. The callback can still compute a cheap envelope and copy bounded audio blocks. Make backlog observable and drop visual-analysis work before threatening audio. For four drum stems, note events or simple envelopes may be sufficient; reserve expensive spectral descriptors for the mix and selected textured sources.

The engine may offer WASAPI capture for standalone use later, with its own routing/latency qualification. That mode must not be marketed as equivalent to sample-timed Live events or per-track semantic roles.

## G5. Graphics API recommendation

| Option | License / Windows path | Relevance to this codebase | Decision |
|---|---|---|---|
| OpenGL 3.3 baseline, optional 4.x | Driver API; no host-version dependency in external process | Preserves GLSL/ISF work and existing Spout integration | **Keep; modernize incrementally** |
| Direct3D 11 | Windows API; SDK terms separate | Natural Windows texture-sharing path; mature tooling | Revisit only if GL interoperability proves a measured problem |
| Direct3D 12 / Vulkan | Explicit modern APIs | More control, much more synchronization/descriptor/lifetime work | Not warranted by twelve modest presets |
| bgfx | BSD-2-Clause | Broad renderer abstraction and shader tooling | Study; adopting it requires an ISF/shader integration plan |
| Diligent Engine | Apache-2.0 core; some backend offerings have separate terms | Broad API abstraction | Larger migration; verify exact backend/dependency license |
| sokol_gfx | zlib | Compact C abstraction | Attractive for new small projects, but not an automatic JUCE/ISF adapter |
| wgpu / Dawn | MIT or Apache-2.0 / BSD-style project terms | Modern WebGPU approach | Shader translation, API semantics and tooling add work |

License evidence: [[G07]](sources.md#g07) [[G08]](sources.md#g08) [[G09]](sources.md#g09) [[G10]](sources.md#g10) [[G11]](sources.md#g11). Spout documents OpenGL and DirectX support; it does not require that the whole engine be rewritten in DirectX to share a texture. [[G03]](sources.md#g03)

[Inference] Do **not** switch the context to core profile and expect `#version 120`, `gl_FragColor`, old texture calls and legacy host geometry to survive unchanged. First isolate shader parsing, generated preambles, fullscreen geometry and framebuffer management. Retain a compatibility context for current presets while adding a versioned modern path. Build a corpus of representative ISF shaders and compare outputs before retiring the legacy path.

`glslang` and SPIRV-Cross are useful compiler tools with permissive but component-specific licenses. They do not magically translate every legacy shader’s host conventions, sampler behavior, feedback schedule, color assumptions or undefined behavior into another API. A future backend rewrite needs a shader dialect contract in addition to a compiler. [[G12]](sources.md#g12) [[G13]](sources.md#g13)

## G6. State, lifecycle and latency

[Inference] Live-set state stores semantic data: preset ID and content hash, eight stable macro slots, source-role bindings, controller profile, output policy and normalization configuration. It does not serialize live GL handles, FBOs or a running fluid field. A persistent texture snapshot could be an optional later asset, not a prerequisite for opening a set.

On recall, the device sends an atomic state transaction. The engine validates assets and compiles a candidate graph while retaining the current picture, then acknowledges successful activation. A missing preset must show a clear fallback and a recoverable error, not silently substitute a different numbered preset. Stable IDs are more reliable than list indices when the library grows.

Latency comprises audio buffering, analysis evidence accumulation, worker/IPC scheduling, render-frame waiting, GPU work and display scanout/processing. An FFT window’s full duration is not automatically the algorithm’s effective detection delay, and the bin spacing alone does not determine it. Measure an impulse or performed MIDI note against a filmed/photodiode display event. A direct MIDI path removes audio onset estimation; it does not create zero-latency light.

The engineering specification sets per-stage budgets, clock/reset semantics and timeout behavior. No end-to-end latency or crash-isolation result has been measured on your machine in this research.

# H. Engineering practice and code worth studying

## H1. Real-time-safe boundaries

[Verified] JUCE supplies `AudioProcessorValueTreeState` for parameter/state management and `AbstractFifo` as an indexing mechanism for FIFO storage. The latter does not allocate or own the sample storage for you. Its producer/consumer assumptions matter; it is not a generic multi-producer lock-free queue. [[E01]](sources.md#e01) [[E02]](sources.md#e02)

[Inference] The audio callback performs bounded audio pass-through, cheap metering, timestamp capture and writes to preallocated buffers. It must not compile shaders, allocate JSON, send a possibly blocking socket message, launch processes, access the filesystem or wait for the renderer. Host parameter values should be accessed through the documented real-time-facing mechanism, not by walking a mutable ValueTree from the audio thread.

Use a separate worker for FFT/onset/history work and another bounded handoff for IPC. Reserve memory at prepare-time, handle block-size changes, and publish overload counters. If a queue fills, drop analysis data with an explicit discontinuity marker; do not block the host. Use actual atomics and well-defined memory ordering. A hand-written “seqlock” over ordinary C++ fields can still contain undefined data races even when it appears to work on one CPU.

Host automation edits, plugin UI gestures and external controller changes require an authority policy. Host-visible base parameters and internal modulation must not continuously overwrite one another. A macro can display a base value plus a modulated indication; recording automation should record the deliberate base gesture, not every internally generated LFO sample.

## H2. Render-loop contract

[Inference] Own the GL context and resource destruction on a designated render thread. Parse assets and validate JSON off-thread; stage GPU compilation/uploads at a safe point. Keep the last good program until the new candidate has linked, reflected the expected inputs and passed a warm-up frame. Never replace an active program with a null handle on compile failure.

Use variable wall-clock presentation but fixed-step simulation where stateful solvers need it. Cap catch-up steps after a stall and declare the policy; attempting hundreds of missed fluid steps can prolong the very stall you are recovering from. Stateless procedural motion can evaluate directly from a monotonic time or host beat position.

Issue GPU timestamp queries and read their results later without stalling for completion. Record CPU frame time separately. A 60 fps average can hide repeated 80 ms hitches. Track P50/P95/P99 frame intervals, missed deadlines, allocation spikes, shader compile times, texture memory and receiver stalls.

On multi-monitor systems, qualify DPI changes, monitor hotplug, mixed refresh rates, dragging the window and fullscreen transitions. Avoid binding the renderer’s existence to whether a plugin editor is visible. Shader hot reload is a development feature; precompile/prewarm the show bank before performance and make live compilation an explicit risk indicator.

## H3. Curated repository map and license gate

**Date policy:** “Unknown” means the browsed repository did not expose a reliable last-commit date. It is not replaced with the research access date. Pin a commit/tag and regenerate an SBOM before integration. Licenses below describe the inspected top-level source; dependencies, model weights, example media and shader collections can differ. Closed-source distribution requires a dependency-level review, not a blanket judgment based on a repository badge.

| Subsystem / repository | License found | Activity evidence as of 2026-09-22 | What to study; target suitability |
|---|---|---|---|
| FluCoMa core and Max adapters | BSD-3-Clause | Exact last activity unknown | Feature/onset implementations and Max wrapper structure; C++17/Windows build path, pin/test Max 8 binary [[D01]](sources.md#d01) [[D02]](sources.md#d02) [[D14]](sources.md#d14) |
| Kiss FFT | BSD-3-Clause | Exact last activity unknown | Small FFT integration; suitable C/C++ Windows candidate, plans/buffers prepared off audio thread [[D07]](sources.md#d07) |
| Signalsmith DSP | MIT | Organization lists DSP update **2026-08-23**, not verified final commit | Lightweight DSP abstractions; compile exact revision in MSVC [[D08]](sources.md#d08) [[E12]](sources.md#e12) |
| aubio | GPL-3.0 | Exact last activity unknown | Compare onset/tempo methods; **do not statically absorb into a proprietary product without a compliant licensing route** [[D09]](sources.md#d09) |
| BTrack | GPL-3.0 | Release **1.0.7, 2025-12-31** visible | Beat-tracker study; unnecessary for transport clock, copyleft integration gate [[D10]](sources.md#d10) |
| madmom | BSD code; supplied model terms differ, including CC BY-NC-SA | Exact last activity unknown | Offline research/comparison; model rights and runtime make it a poor default shipping dependency [[D11]](sources.md#d11) |
| Essentia | AGPL-3.0 / commercial options; models separately licensed | Exact last activity unknown | Rich descriptors/algorithm comparisons; commercial and dependency rights need explicit clearance [[D12]](sources.md#d12) |
| Beat This! | MIT code/weights stated by project | Exact last activity unknown | Neural beat research; causal streaming and Windows live budget still unproven here [[D13]](sources.md#d13) |
| projectM | LGPL-2.1 family; inspect exact component/revision | Exact last activity unknown | Preset scheduling, feedback and MilkDrop compatibility; linking/redistribution obligations and preset rights separate [[L02]](sources.md#l02) |
| Butterchurn | MIT library; assets/presets separately reviewed | Exact last activity unknown | Browser MilkDrop implementation and equation pipeline; study rather than replace native renderer [[L03]](sources.md#l03) |
| VVISF-GL | BSD-style project license | Exact last activity unknown | ISF parsing, shader generation, pass scheduling and buffer lifetime; Windows port/build verification needed [[G02]](sources.md#g02) |
| Spout2 | BSD-2-Clause | Versioned update log; exact latest commit unknown | Texture lifetime, GL/DX interop and sender/receiver discovery; native Windows fit [[G03]](sources.md#g03) [[E11]](sources.md#e11) |
| Gustavson/Ashima webgl-noise | MIT | README references a newer **2022** implementation; last commit unknown | Portable noise functions and derivatives; GLSL 1.20 path useful now [[G15]](sources.md#g15) |
| WebGL Fluid Simulation | MIT | Exact last activity unknown | Advect/project/dye pass organization and input injection; port bounded shader logic, not assumed turnkey native integration [[G14]](sources.md#g14) |
| Filament | Apache-2.0 main project | Exact last activity unknown | Color, exposure, material/post-processing implementation; not a recommendation to adopt its entire engine [[G06]](sources.md#g06) |
| bgfx / Diligent / sokol | BSD-2 / Apache-2 / zlib respectively; see G5 caveats | sokol README contains **2026-09-14** change note; other exact latest commits unknown | Backend/resource design and examples; external Windows application, independent of Live/Max [[G07]](sources.md#g07) [[G08]](sources.md#g08) [[G09]](sources.md#g09) |
| JUCE | Commercial or applicable copyleft terms; version-specific | Current repository license differs from a project pinned to JUCE 8 | Plugin parameter/state and OpenGL context examples; license and MSVC/Live behavior pinned to actual release [[E01]](sources.md#e01) [[E05]](sources.md#e05) [[E06]](sources.md#e06) |
| Steinberg VST3 SDK | **MIT in SDK 3.8** | Licensing announcement **2025-10-28** | Event/audio bus declarations, host state and SDK examples; older host compatibility still a test [[E03]](sources.md#e03) [[E04]](sources.md#e04) |

[Verified / important update] Repeating the older statement “VST3 SDK means GPL or a commercial Steinberg agreement” is outdated for the inspected 3.8 generation. This does **not** remove JUCE’s independent license obligations or grant rights to copied third-party shaders. [[E03]](sources.md#e03) [[E04]](sources.md#e04) [[E05]](sources.md#e05)

[Inference] There is no need to adopt a large modulation framework merely to implement the specified acyclic additive routes. Own that small deterministic layer. Study audio-synth modulation concepts, but avoid copying an attractive GPL synth’s implementation into a closed-source product without a licensing plan. Likewise, do not ship a downloaded Shadertoy/MilkDrop collection without checking each shader/preset’s rights.

## H4. Tests that should exist before the plugin rewrite

[Inference / proposed suite] DSP tests use impulses, sine bursts, steady tones, noise, silence, stereo anti-phase material, six-band sweeps and the same music at different gain offsets. Assert finite outputs, correct band ordering, no normalization of silence into activity, bounded onset duplication and predictable response under sample-rate changes. Compare the normalizer against both fixed-reference and naive running-maximum baselines; the new method must preserve the perceptual difference between a breakdown and a drop.

Visual regression should fix preset, seed, feature timeline, resolution and elapsed beat/time. Save representative frames rather than one arbitrary screenshot. For pure arithmetic shaders on the same machine, use tight tolerances; across GPUs/drivers, use bounded image differences and perceptual metrics rather than universal bit equality. Feedback simulations need a known starting state and replayed timestep sequence.

Run at least a two-hour show soak with repeated preset changes, source dropouts, controller floods, output resize, receiver disconnect and shader-load failure. Collect audio-underrun indicators separately from visual frame statistics. A renderer crash test must prove that Live’s audio continues and that a clean process can reconnect without invalid host state. A single successful fullscreen demo does not establish this.

# I. Product and UX

## I1. Minimum lovable version

[Inference] The first releasable experience should consist of **three excellent generative presets, four macros, explicit kick/texture/MIDI roles, preset pads, quantized transitions, a preview, independent fullscreen and working Spout**. Add calibrated response controls and a visible connection state. The rest of the twelve designs are an expansion roadmap. Neither a node editor nor a full media library is required to make that first experience valuable.

The compact Live device should show a preset name/previous/next, four large macro controls, small source/activity indicators, a response selector, a transition control, an output button and a clearly distinguishable blackout status. Reserve an expandable inspector for route curves and thresholds. Do not force all DSP knobs into Live’s device-strip height.

Use consistent macro semantics: **Intensity** changes visual energy, **Motion** changes pace/flow, **Color** changes palette location/accent, **Space** changes composition/depth/scale. Their underlying destinations vary by preset, but a performer should be able to predict the gesture. Preset names and thumbnails should describe the look, not the algorithm alone.

## I2. First-run path

[Inference] Open the device, launch/find the engine, choose an output and see a beautiful idle preset before configuring audio. A simple meter confirms the selected source; a test note and test hit confirm event routing. Offer “master only” as the quick path and “kick + texture + MIDI” as the more expressive path. Explain that source roles are explicit and optional; do not pretend the master analyzer always knows which drum produced an onset.

Run a two-second calibration when appropriate, display its progress and allow a locked-response mode. A user should never have to turn up the sound merely to make a visual visible. However, silence must not be expanded into false movement; idle animation belongs to the preset, not to imaginary detected energy.

Before starting a show, prewarm the chosen bank, check assets, show output resolution and frame budget, confirm the receiver, and save the set. Keep advanced diagnostics exportable as a small JSON manifest/log rather than requiring screenshots of many settings dialogs.

## I3. Failure behavior is part of the instrument

[Inference] Blackout is an immediate final-output gate outside the preset graph, so a bad shader cannot bypass it while the engine is still executing. A fully hung GPU/process cannot guarantee a newly rendered black frame; an external receiving application or hardware switch must own the ultimate fail-safe. State this honestly in the manual.

On lost analysis, release activity smoothly to zero and preserve transport-independent idle motion. On lost clock, show the clock source as stale and follow the preset’s declared hold/free-run policy; do not invent a tempo change. On output disconnect, preserve the performance state and offer a safe local preview rather than repeatedly stealing focus. After a renderer restart, restore state without automatically enabling a previously blacked-out output.

Rapid high-contrast flashing is opt-in, off by default and not assigned to a normal preset-selection control. This is a product-risk policy, not a medical guarantee that any flash rate is universally safe. Avoid adding a global strobe simply because other VJ programs have one.

# Open questions that require your machine or the developers

The following are unresolved by public documentation or cannot be established without your actual code/hardware:

| Question | Concrete way to settle it |
|---|---|
| Exact Live 10 / Max 8 / JUCE / GPU baseline | Export Live, Max, Windows build, driver, GPU, display refresh/DPI, ASIO interface and CMake dependency manifest |
| Max 8.6.5 + your Live 10 set | Test a copy of the set using the external Max path; keep the working installation recoverable |
| GL3 and Spout packages inside M4L | Test saved device versus editing, package versions and integrated/discrete GPU selection; this is not needed for the existing external renderer |
| VS and Photism VST3 on Live 10.1.43 | Trial install: notes/CC/automation/sidechain/state, editor close, fullscreen, two instances and monitor changes; request vendor confirmation |
| T3X2R’s exact renderer/version/terms | Ask for the currently supplied Max-8-compatible build, GL-engine setting and commercial performance/redistribution distinction |
| Actual onset-to-photon latency | Record known MIDI/audio events and output light with a high-speed camera or photodiode; report distribution, not just best case |
| Driver behavior at 1080p/4K and two displays | GPU timestamp capture plus external frame pacing; test Spout receiver on the same adapter |
| Current prototype’s threading and buffer correctness | Review source for GL ownership, FBO hazards, UDP thread safety, audio callback allocation and persistent-state lifetime |
| Live duplicate-instance identity and state recall | Duplicate tracks/devices, save/reopen, undo/redo and open two sets/instances; verify source/conductor arbitration |
| Scene-name linking | Test renamed scenes, duplicate names, launch quantization and empty slots; move to IDs/event state rather than name-only coupling |
| Third-party preset rights and exact repo activity | Pin revisions, inspect all licenses/dependencies, request missing permissions and record commit hashes/dates in the SBOM |
| StreamDiffusion/ML on the show GPU | Benchmark only after the conventional renderer meets its budget; include VRAM, sustained throughput, warm-up and interference with Live |

# Final decision

[Inference] Build a **musically structured preset instrument on the current hybrid foundation**. The first major improvement is not more shader complexity; it is a stable distinction between measured sound, adaptive activity, rhythmic events, transport time and performer intent. Once that contract is reliable, a ring, a wire torus or a feedback ribbon can feel authored and playable. The supplied engineering spec turns this recommendation into implementation order, schemas and acceptance tests; it is not a claim that those changes have already been compiled or tested in your codebase.
