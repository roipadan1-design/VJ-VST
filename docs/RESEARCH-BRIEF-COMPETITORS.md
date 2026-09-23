# Research brief: how competing audio-visual tools are built, and what we are missing

You are a research agent. You have **no access to our code**; everything you need to know about our product is in this brief. Use web search and page fetching extensively (official sites, manuals, changelogs, YouTube/Vimeo tutorials and their descriptions, forums: Resolume forum, r/vjing, r/ableton, Cycling '74, KVR, Gearspace, CDM). Cite a URL for every non-obvious claim. Write "unknown" instead of guessing, and label inferences as inferences.

## 1. Our product (current state, Sept 2026)

A live audio-visual instrument for **Ableton Live 10.1 on Windows**, with an **Intel Iris Xe integrated GPU**. It is built as three parts:

- **VJ Analyzer**: a VST3 audio effect. You put one instance on a track and give it a *role*: MIX, KICK, SNARE, HAT, BASS or TEXTURE. It analyses the audio into levels, bass/mid/high, 32-band spectrum, onsets, centroid/flatness/flux and energy trend. It also reads MIDI notes and Live's transport (tempo, beat, bar), and sends everything over OSC to the engine. Its UI has:
  - 8 macros: Intensity, Motion, Color, Space, Impact, Gravity, Viscosity, Detail
  - a scene list, HIT and BLACKOUT buttons
  - Hit Sensitivity, Trim, and Adaptive/Locked normalisation
  - a **LOOK** panel. It holds Grain, Crush (soft to two-tone), Flash (chance a kick drops a black or colour frame), Glitch, Trails, Symbols (braille strips), Cut Rate (automatic cuts between scenes, from every 16 beats up to every kick), and a 3-colour palette (shadow/mid/light gradient map, with 7 presets or a free colour pick).
- **VJ Engine**: a standalone OpenGL app (window/fullscreen/Spout). Its scenes are JSON presets made of ISF GLSL stages, plus a modulation graph: macros, AD envelopes fired by kick/snare/hat events, tempo LFOs, and routes with curves/smoothing into shader parameters. It has beat/bar-quantised cuts and crossfades, and a global look pass over everything.
- **Scenes (7, all generative, red-monochrome aesthetic):**
  - liquid blobs (kick throws the mass, gravity and viscosity, ripple rings)
  - dot relief of abstract 3D forms
  - 1-bit threshold
  - endless corridor fly-through
  - fibre/nerve web
  - scrolling invented "terminal" script
  - ink blots

**Just built, first version:** a **"VISUALS REACT TO"** control, meaning global toggles for which signals drive the visuals (kick / snare / hat / bass / level). With only KICK on, the whole picture moves only with the kick; opening HAT adds the hats, and so on. A closed channel's hits are dropped and its continuous signals fade to neutral. The user also asked whether each channel should have *its own* effect settings. We believe that is a UX trap, but we want evidence either way, and we want to know how the best tools present this.

## 2. Products to research

**Primary (the user's six references, go deep):**
1. Showsync: Videosync, Beam for Live, Sync Tools. https://www.showsync.com/
2. Zwobot: https://www.zwobotmax.com/ (and /manual/)
3. VS – Visual Synthesizer (Imaginando): https://www.imaginando.pt/products/vs-visual-synthesizer
4. Arkestra: https://www.arkestra.app/
5. Photism: https://photism.app/ (manual, plugin, scenes)
6. T3X2R: https://www.t3x2r.com/ (MAYAS, R3NDER)

**Secondary (only what is relevant to the questions below):**
- Synesthesia (https://synesthesia.live/)
- Resolume Arena/Avenue (audio FFT, envelopes, parameter animation, dashboard)
- VDMX
- TouchDesigner (Audio Analysis component)
- Magic Music Visuals
- Spettro VST
- EboSuite
- Visibox
- MilkDrop / projectM / NestDrop
- Plane9
- Notch
- Any 2025–2026 newcomers

## 3. Questions (answer each, per product where it applies)

**A. Audio routing and "what reacts to what" (highest priority)**
1. Does the product analyse the master only, or several tracks/stems? How does the user choose the source (per layer, per parameter, global)?
2. Is there anything like our REACT TO: global enable/solo/mute of analysis channels, or per-layer source selection? How is it presented (toggles, matrix, dropdown per parameter)?
3. Do settings live per source/channel, per layer/scene, or globally? What do users and reviewers say works on stage, and what confuses them?
4. How are hits (onsets) separated per instrument: stems, frequency bands, MIDI notes? How are thresholds, sensitivity and "gate/hold" exposed?

**B. Controls and parameters we lack**
5. List every performance control each product exposes: global FX, per-scene parameters, macros, strobe, blackout, freeze/hold, tap tempo, speed multiplier, beat-division selectors, randomise, "intensity" master, brightness/opacity master, and so on. Build a table and **mark what we do not have**.
6. Global post-FX chains: which effects do they ship (e.g. feedback, kaleido, mirror, RGB split, bloom, film grain, posterise, colour grading/LUT, strobe) and how are they ordered or controlled?
7. Colour systems: palettes, gradient maps, LUTs, hue shift, per-scene vs global colour, palette changes on beat.

**C. Scenes, transitions and live flow**
8. How scenes are switched: quantised to beat/bar, transition types and durations, auto-pilot/sequencer, follow Live's scenes/clips.
9. Snapshots / cue lists / A-B banks / morphing between parameter states.
10. What happens to effects (trails, feedback) on a scene change: reset, carry over, crossfade?

**D. MIDI and hardware**
11. MIDI learn UX, controller templates, LED feedback, soft takeover, pads-as-scene-grid.

**E. Presentation and UX**
12. How many controls are on the main performance screen? How is complexity hidden (pages, "advanced" drawers, per-scene labels on generic knobs)?
13. Preview and monitoring inside the DAW (thumbnail, mini-preview, FPS/GPU meter).

## 4. Output format (strict)

Deliver one Markdown report with these sections:
1. **Executive summary:** 10 bullet points, the most important findings for us.
2. **Per-product teardown:** one section per primary product, answering A–E concisely, with URLs.
3. **REACT TO recommendation:** how the best tools solve "what reacts to what", and a concrete recommendation for our design (global toggles vs per-layer source vs per-parameter source), including the per-channel-settings question. Give the evidence.
4. **Feature gap table:** columns are *Feature | Who has it | Do we have it (yes/no/partial) | Value for a live VJ (1–5) | Effort guess (S/M/L) | Notes*. Include at least 30 rows.
5. **Top 10 prioritised additions for us**, each with what, why, how it should behave, and which product does it best.
6. **Anti-patterns:** what users complain about in these tools that we must avoid.
7. **Sources:** every URL used.

Keep claims verifiable. Mention when a finding comes only from marketing copy and not from manuals or user reports.
