# Deep research brief: audio-visual instruments, craft knowledge and a visual reference library

## Your role

You are a **senior creative technologist and real-time graphics engineer** with 15+ years in audio-visual performance. You have:
- built audio-reactive instruments in Max/MSP/Jitter and Max for Live, VST/AU plug-ins (JUCE), TouchDesigner systems and custom GLSL engines
- toured as a VJ and live visualist
- curated visual art for festivals such as MUTEK, CTM, Sónar+D and Berlin Atonal

You know the developer communities from the inside. You can tell marketing copy from real engineering. You judge visuals with a trained eye: composition, texture, motion, colour and how image and sound lock together.

You are advising a small team (one artist-owner and one engineer) that is building its own instrument. The engineer will turn your findings directly into code, so be concrete: techniques, parameters, numbers, pitfalls and links to working implementations. Vague advice is useless to us.

**Standards:**
- Cite a URL for every non-obvious claim.
- Include only links you actually opened. Never invent a URL.
- Write "unknown" rather than guess, and label inferences as inferences.
- Say when a finding comes only from marketing copy and not from manuals, code or user reports.
- Flag licences (GPL, CC-NC, and so on) for any code or asset you recommend.

**Depth:** be exhaustive. This research is meant to be very deep. If you hit an output limit, split the report into numbered parts and continue when asked "continue".

**Language:** write the report in English, and open it with a short executive summary in Hebrew for the owner.

---

## 1. Our product (context, Sept 2026)

A live audio-visual instrument for **Ableton Live 10.1 on Windows 11**. The machine has an **Intel Iris Xe integrated GPU**, which is a hard performance budget. It has three parts:

- **VJ Analyzer** (VST3 audio effect, JUCE): one instance per track, with a role of MIX / KICK / SNARE / HAT / BASS / TEXTURE.
  - **Analysis:** level, bass/mid/high, 32-band log spectrum, per-band onsets, spectral centroid, flatness, flux and energy trend, MIDI notes, and Live's transport (tempo, beat, bar). Everything goes out over OSC.
  - **Macros (8):** Intensity, Motion, Color, Space, Impact, Gravity, Viscosity, Detail.
  - **Other controls:** scene list, HIT, BLACKOUT, and hit sensitivity with adaptive/locked normalisation.
  - **VISUALS REACT TO:** global toggles for kick / snare / hat / bass / level. A closed channel's hits are dropped and its continuous signals fade to neutral.
  - **LOOK panel:** Grain, Crush (soft to two-tone), Flash (a kick may drop a black or colour frame), Glitch, Trails, Symbols (braille strips), Cut Rate (automatic scene cuts on the beat grid) and a 3-colour palette (shadow/mid/light gradient map, with presets or a free pick).
- **VJ Engine** (standalone OpenGL 2.1-compatible app, window/fullscreen/Spout).
  - **Scenes:** JSON presets made of ISF GLSL stages, including multi-pass and persistent float buffers.
  - **Modulation graph:** macros, AD envelopes fired by hits, tempo LFOs, and routes with curves and smoothing.
  - **Output chain:** beat-quantised cuts and crossfades, bloom, tone mapping, then a global look pass.
- **Scenes today (7, all generative, red-monochrome):**
  - a liquid mass (kicks throw it, with gravity, viscosity and ripple rings)
  - a dot relief of abstract 3D forms
  - a 1-bit threshold
  - an endless corridor fly-through
  - a fibre/nerve web
  - a scrolling invented terminal script
  - ink blots

**Constraints:** generative content only (no stock footage in the product). No figurative clichés; the owner dislikes skulls, masks and faces. It must run smoothly on an integrated GPU.

## 2. The owner's taste (the most important input)

- **Core style:** ambient, downtempo, **grain, noise, 35mm film artefacts**, meaning:
  - film grain and gate weave
  - halation and bloom on highlights
  - dust, hairs and scratches, light leaks, film burn and flicker
  - soft chromatic fringing and lifted blacks
  - slow, breathing motion
- **A favourite artist:** known as **"The Noise Diary"**. Identify them first (likely Instagram / YouTube / Vimeo / Bandcamp), confirm the identification with links, and analyse their work in depth. If you cannot identify them with confidence, say so and list the candidates.
- **Current reference clips:** short Instagram reels, apparently by the account **phs.wrk**. They show:
  - black to red to salmon monochrome
  - hard threshold and grain
  - about one cut per second, with black and full-red flash frames
  - braille and glyph grids over the image
  - terminal text, horizontal smear and pixel blocks
  - fibrous root/nerve textures
  - a continuous forward dolly through a corridor

  Analyse this account too if you can reach it.

---

## 3. Part A: tools and competitors (features we lack)

**Primary (the owner's references, go deep):**
1. Showsync: Videosync, Beam for Live, Sync Tools. https://www.showsync.com/
2. Zwobot: https://www.zwobotmax.com/ (and /manual/)
3. VS – Visual Synthesizer (Imaginando): https://www.imaginando.pt/products/vs-visual-synthesizer
4. Arkestra: https://www.arkestra.app/
5. Photism: https://photism.app/ (manual, plugin, scenes)
6. T3X2R: https://www.t3x2r.com/ (MAYAS, R3NDER)

**Secondary:**
- Synesthesia
- Resolume Arena (audio FFT, envelopes, parameter animation, dashboard)
- VDMX
- TouchDesigner (Audio Analysis component and community audio-reactive toolkits)
- Magic Music Visuals
- Spettro VST
- EboSuite
- Visibox
- MilkDrop / projectM / NestDrop
- Plane9
- Notch
- Hydra
- cables.gl
- Vuo
- any 2025–2026 newcomers

**For each product answer:**
1. **Audio routing:** master only, or several tracks/stems? How does the user choose what reacts to what (global, per layer, per parameter)? How are hits separated per instrument (bands, stems, MIDI)? How are thresholds, sensitivity and hold exposed?
2. **Settings scope:** do settings live per source, per scene or globally? What works on stage, and what confuses users (with evidence from forums and reviews)?
3. **Controls:** every performance control, for example global FX, macros, strobe, blackout, freeze, tap tempo, speed multiplier, beat divisions, randomise, master intensity and opacity.
4. **Post-FX:** the global FX chain and the colour system (palettes, gradient maps, LUTs, palette changes on the beat).
5. **Live flow:** scene switching, transitions, snapshots and morphing, auto-pilot, following Live's clips and scenes. Also what happens to trails and feedback on a scene change.
6. **MIDI:** MIDI learn, templates, LED feedback, soft takeover.
7. **UI:** how complexity is hidden, and how preview and monitoring work inside the DAW.

## 4. Part B: craft knowledge harvest (so we write better code and better animation)

Read the developer and artist communities, not only product pages. At minimum:
- the Cycling '74 forum and maxforlive.com
- the Ableton forum
- the KVR developer forum and the JUCE forum
- the Derivative (TouchDesigner) forum and community posts, and r/TouchDesigner
- r/vjing, r/generative, r/creativecoding, r/shaders
- Shadertoy (and the comments on top shaders)
- the ISF community and VJ Union
- the Resolume and Notch forums
- the openFrameworks / Processing / Cinder forums
- Interactive & Immersive HQ
- well-known tutorial authors and educators (for example Inigo Quilez, The Book of Shaders, Acerola, Bileam Tschepe / elekktronaut, Paketa12 — verify and extend this list)
- GitHub repositories of audio-reactive and film-emulation projects

**Topics.** For each, give concrete techniques, parameter ranges, pitfalls, and links to open-source implementations with their licences:
1. **Film emulation in real time.** Physically motivated grain (luminance-dependent, per-channel, grain size vs resolution, temporal behaviour), halation, gate weave, flicker, dust/scratch/hair generation (procedural vs scanned plates), light leaks and film burn, chromatic softness, lifted blacks and film curves, LUT pipelines, dithering against banding. What separates a convincing 35mm look from a cheap "Instagram filter".
2. **Noise as material.** Blue vs white noise, animated noise that does not shimmer, static and analog-video noise, VHS/tape artefacts, and domain warping for organic textures.
3. **Ambient and downtempo motion design.** How good visualists make slow music feel alive without twitching:
   - smoothing strategies and envelope shapes
   - slow LFOs locked to phrases
   - reacting to build-ups rather than to every hit
   - "breathing" camera moves
   - when **not** to react
4. **Audio-to-visual mapping craft.** Which features drive which visual parameters well. Anti-jitter. Hierarchy (rhythm vs texture). How pros avoid "everything reacts to everything".
5. **Feedback systems.** Trails, reaction-diffusion, fluid simulation and particle systems that are cheap enough for an integrated GPU (resolution tricks, half-res buffers, temporal accumulation).
6. **Architecture lessons from people who built these tools.** Renderer in-process vs a separate process, plug-in GUI pitfalls in Ableton, frame pacing, OSC vs shared memory, Spout, parameter smoothing, preset and transition design, shader hot-reload.
7. **Performance on integrated GPUs.** What actually costs time (fbm octaves, raymarch steps, full-screen passes, fp16 vs fp32 targets) and the best-practice budgets.

## 5. Part C: a visual reference library (with analysis)

Scan the web for **outstanding work** and build us a curated reference library. Where to look:
- artist sites and Instagram/Vimeo/YouTube channels
- Vimeo Staff Picks, Behance, Are.na channels
- Motionographer and Stash
- Shadertoy (top shaders)
- the TouchDesigner community showcase and the Notch showcase
- festival programmes: MUTEK, CTM, Berlin Atonal, Unsound, Sónar+D, Ars Electronica / Prix Ars, Lumen Prize, SIGGRAPH Art Gallery, Mapping Festival
- VJ contests and battles, and projection-mapping competitions (for example Genius Loci Weimar and the 1 Minute Projection Mapping competition)
- VJ-loop communities

**Seed artists to check** (verify, and add many more): Rainer Kohlberger, Sabrina Ratté, Ryoichi Kurokawa, Paul Prudence, Tarik Barri, Pierce Warnecke, Robert Henke, Ryoji Ikeda, Kangding Ray, Alba G. Corral, Rosa Menkman, Takeshi Murata.

**Style sections** (target at least 120 entries in total):
1. **Ambient / grain / noise / 35mm / analog texture.** This is the core of the library and needs at least 40 entries.
2. Dark monochrome and red-industrial (like our current references).
3. Glitch, datamosh and digital decay.
4. Minimal generative / geometric / data.
5. Organic, fluid, cellular.
6. Typography, glyphs, code and braille.
7. Abstract 3D forms and sculptural light.
8. Analog video synthesis and feedback.

**For each entry:**
- artist and work title, year, and the link to the work
- **stills:**
  - If your environment can download or capture images, save 1–3 stills per entry (at least 1280 px wide where possible), filed in a folder per style section, and deliver everything as a ZIP with an index file.
  - If it cannot, give **direct image URLs** (not only page URLs) and, for videos, **timestamps (mm:ss)** of the exact moments worth capturing. We will capture them ourselves.
- a precise visual description: palette (approximate hex values), contrast, grain and texture, composition, motion (speed, rhythm, camera), and how it relates to the sound
- **technique inference:** how it is probably made (layers, passes, noise types, feedback, simulation, film emulation) and how hard it would be on an integrated GPU
- **reproduction recipe:** the outline of a GLSL/ISF implementation (passes and key functions) that would get us 80% of the look
- which of our scenes or LOOK controls it informs, or the new scene it suggests

## 6. Output format

1. **Hebrew executive summary** (10 bullets).
2. **Part A:** a teardown per product, a **REACT TO recommendation** (how the best tools solve "what reacts to what", with evidence, including whether per-channel settings are a good idea), and a **feature gap table**. The table columns are: *Feature | Who has it | Do we have it (yes/no/partial) | Value for a live VJ (1–5) | Effort (S/M/L) | Notes*. It needs at least 40 rows.
3. **Part B:** a craft knowledge handbook, organised by the topics above, with techniques, numbers, code pointers and licences.
4. **Part C:** the reference library, organised by style section, plus an analysis of "The Noise Diary" and of phs.wrk.
5. **A style bible for the owner's look:** grain spec (size, intensity and luminance response in numbers), 35mm artefact list with parameter ranges, palette families, motion tempos for ambient and downtempo, composition rules, and do/don't lists.
6. **15 new scene concepts** derived from the references. Each gets a description, audio mapping, the knobs it exposes, a technique outline and an estimated integrated-GPU cost.
7. **Top 15 prioritised additions to our instrument**: what, why, and how it should behave.
8. **Anti-patterns:** what users complain about in these tools, and what makes audio-reactive visuals look cheap.
9. **Further reading:** a categorised list of links (forums, threads, tutorials, papers, repositories, artist pages), each with one line on why it is worth reading.
10. **Sources:** every URL used.
