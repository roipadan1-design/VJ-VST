# Art-direction brief: "Before It Disappears"

**Scope:** creative and technical research, visual concept, and show presets for a contemporary dance piece.
**Written for:** a Claude Code session opened at the root of this repository (`VJ VST`).
**Output:** `docs/before-it-disappears/CONCEPT.md`, written in Hebrew, plus a folder of stills.

To run it, open a new session in this repo and say: *"Read docs/before-it-disappears/ART-DIRECTOR-BRIEF.md
and carry it out."*

---

## 0. How to work

Work in five phases, in this order.

1. **Absorb (repo only, no web yet).** Read every file listed in section 4. Render the existing scenes
   (section 4.4) and look at the stills yourself.
2. **Interview the owner.** Ask only what blocks you, taken from section 2.3. Ask at most 8 questions,
   using AskUserQuestion (2 calls of up to 4 questions). For anything left unanswered, write down the
   assumption you are making and carry on. Do not ask about anything already stated in this brief.
3. **Research** (web and repo). Cover the strands in section 5. They are independent, so you may run them
   as parallel sub-agents, one per strand. Give each sub-agent the relevant part of this brief and the
   research standards (section 8), and have it return findings with URLs. You write the synthesis
   yourself.
4. **Write** `CONCEPT.md` (section 7) and save the stills.
5. **Present.** In chat, give a short Hebrew summary: the 3 directions, your recommendation, and the
   decisions you need from the owner and the choreographer.

**Hard limits in this phase:**
- Do not change anything in `engine/`, `plugin/`, `analysis/`, `m4l-device/` or the shipped presets.
  Write only under `docs/before-it-disappears/`.
- You **may** run the engine and the existing tools to render stills and analyse audio. You **may**
  write small throwaway scripts in the session scratchpad.
- Building comes later, once the owner picks a direction. The document ends with the build plan for
  that later phase.

---

## 1. Your role

You are a **senior art director and creative director in audiovisual performance and video art**, with
20+ years of work across:
- stage projection for contemporary dance and theatre
- gallery video art
- live AV concerts (MUTEK, CTM, Berlin Atonal, Sónar+D)

You have worked side by side with choreographers and lighting designers, in the rehearsal room and in
tech week. You think like a **dramaturg of the image**: every visual decision has to answer "what does
this do to the dancers, and to the meaning?".

You also carry a **copywriter's ear.** Naming, the one-sentence idea, a program note and titles are part
of the concept, not decoration. A preset called "Scene 07" is a failed idea; a preset whose name tells
the operator what it *means* is a working tool.

You are technically literate, which keeps your ideas buildable:
- real-time GLSL and ISF, feedback buffers, TouchDesigner
- projector physics: lumens, black level, contrast, throw
- show control, and what an integrated GPU can and cannot do

You have strong taste, and you say no. Give recommendations, not surveys. When you list options,
rank them and commit to one.

---

## 2. The show

### 2.1 What we know

- **Title:** *Before It Disappears.* Confirm the exact spelling and whether there is a Hebrew title.
- **Form:** a contemporary dance piece. The owner (Roi) composes the **original music** and creates the
  **video art**. The choreographer wrote the conceptual text, saved in
  `docs/before-it-disappears/choreographer-text.md`. That text is the heart of this brief (section 3).
- **Themes, in the owner's words:** memory and forgetting; the body, time and transience; and the
  choreographer's reframing of memory as transformation, not loss.
- **Stage:** video is projected on a **screen or cyclorama behind the dancers.** That is the only
  projection surface. It is not yet known whether the projection comes from the front or the rear.
- **Operation:** a hybrid of two layers.
  - **Spine:** a cue-locked timeline in Ableton Live's Arrangement. Automation of the VJ Analyzer's
    parameters (scene, macros, snapshots, palette, LOOK) makes every night play the same.
  - **Live layer:** the owner plays on top of the spine with a MIDI controller (an old Korg, model
    unknown).
- **Finale:** a scene of about **5 minutes set to The Beatles' "Because"** (*Abbey Road*, 1969).
  - The owner has the lyrics. **Do not reproduce the lyrics, even partly, anywhere you write:
    they are copyrighted.** Refer to their images by paraphrase only: an elemental song about the
    earth's roundness, the wind and the blue sky, with a bridge about love.
  - The original recording is under 3 minutes. The 5-minute length suggests a cover, an extended
    arrangement, or a reworking by the owner. Ask which.

### 2.2 What this changes compared with the instrument's previous brief

The instrument was built for club and concert AV:
- red-mono, industrial and film references
- kick-driven hits
- cuts about once per second

A dance piece turns several of those assumptions around:
- **The dancers are the figure.** The screen is the field, the memory and the weather around them.
  The screen must never compete with the bodies, or copy them.
- **Legibility of the bodies is sacred.** Every visual state has a luminance budget.
- **Tempo is dramaturgical.** Change happens by section and by cue, not by kick. The music may have
  little or no steady percussion (to be confirmed).
- **Repeatability.** The show must look the same on night 1 and night 10, and survive a crash.

### 2.3 Open questions (the pool for the interview in phase 2)

Pick the ones that actually block you.

1. Choreographer's name, number of dancers, total duration, venue, premiere date, and rehearsal access
   to the venue.
2. Section list and running order: which music goes where, and whether it is finished. Tempo(s), and
   how percussive the music is. Where the audio files are.
3. "Because": is it the original recording, a cover, or the owner's arrangement? Why 5 minutes? What
   happens in the choreography during it?
4. Projection:
   - front or rear projection
   - projector model, lumens and resolution
   - screen or cyc size and aspect ratio, and whether the image fills it
   - distance from the dancers
   - whether the lighting designer also lights the cyc
5. Lighting designer, costumes (colours, materials), and whether haze is allowed.
6. The text: spoken as voice-over, printed in the programme, projected, or only a source for the team?
7. The choreographer's own visual references, no-go's, and how much the screen should lead versus
   support.
8. Does the owner also play the music live while running the visuals? Which computer outputs video?
   This machine has an Intel Iris Xe integrated GPU. What is the controller model?

---

## 3. The choreographer's text

Read `choreographer-text.md` many times. Then write a real dramaturgical analysis of it: the
structure, the turning points, the key verbs and images, what is said and what is avoided, and the
text's form on the page.

Below is our first reading. Treat it as **hypotheses to test, deepen or overturn**, not as the answer.

**The arc, in movements**

1. **Waiting, not fading.** Memories don't fade; they wait, like silence. The dormant image: present
   but not yet lit.
2. **The moment leaves while we are inside it.** This is dance itself: movement disappears before it can
   be held.
3. **We reach for it anyway: we slow it down, replay it, name it.** These are, literally, the tools of
   video: time-stretch, loop and replay, captions and labels. They also fail: "the moment itself never
   returns".
4. **The frame.** Memory lives inside a frame and keeps the outline but not the whole story. Edges
   soften, colours drift, and the story is rewritten on every recall. The line about remembering "the
   last time we remembered it" is, in memory science, reconsolidation. In media it is generation loss:
   a copy of a copy.
5. **The pivot.** At the edge of itself, memory "no longer recalls. It creates." This is the hinge of
   the piece. For us it is the move from a *traced* image (reproducing something) to a *generated*
   one. Our instrument is purely generative, so this line is practically our manifesto.
6. **The cloud.** Memory drifts, "suspended between presence and absence", and settles in unexpected
   places: a voice, a scent, a familiar light. Note the *familiar light* as a possible recurring motif.
7. **The shadow.** Every memory becomes the shadow of itself, sometimes paradise and sometimes hell.
   Shadows are physically present on a stage. If the projection comes from the front, the dancers
   cast shadows onto the image.
8. **The inversion.** "We no longer carry the memory. The memory carries us." The ground carries the
   figure; the screen could literally take the dancers over.
9. **Transformation, not preservation.** The boundary dissolves: "where the moment ends, and where we
   begin". Figure and ground merge.
10. **The mark.** The ending is "only a mark… left by everything that ever moved us". *Moved* is a pun
    on being moved emotionally and being set in motion, which is dance. The lines shrink to one or
    two words: the text's typography enacts disappearance.

**Implication to test.** The text rejects the easy reading of the title. The arc is not *present → fade
to black*. It is closer to *waiting → slipping → grasping → rewriting → creating → drifting → shadow →
inversion → mark*. The last image should be a **residue or a trace**, not emptiness. If you disagree,
argue it.

---

## 4. What already exists (read, look, and audit)

### 4.1 Read these files

| File | Why |
|---|---|
| `USAGE.md` | The full technical reference: plug-in controls, OSC, preset format, tools |
| `docs/QUICKSTART-HE.md` | The owner's view of the instrument, in Hebrew |
| `docs/REFERENCE-LIBRARY.md` | References the owner loves, and what was taken from each |
| `docs/RESEARCH-REPORT.md` | Previous deep research: craft handbook, style bible, 15 scene concepts. **Do not redo it.** Build on it and cite it by section |
| `docs/RESEARCH-BRIEF-COMPETITORS.md` | The previous brief, including the owner's taste profile |
| `docs/REVIEW-2026-09-24-HE.md` | The latest review round: the owner's feedback after testing, and planned changes to the controls (macros may be renamed) |
| `engine/tools/build_instrument_presets.py` | How the scenes are defined: parameters, routes, envelopes, LFOs |
| `engine/Presets/*.json` | The generated presets |
| `engine/Shaders/Instrument/*.fs` | The scene shaders (read enough to judge what each one can become) |
| `engine/Source/LookPass.*`, `FinishPass.*`, `Signals.*`, `Modulation.*`, `SourceLibrary.*`, `PresetV2.*` | The global look, film chain, section signals, modulation and source material |
| `plugin/Source/PluginProcessor.*` | Which parameters exist, and so what Arrangement automation can drive |
| `references/` (local only) and `video ref/frames/` | Reference stills and clips: look at them |

### 4.2 The instrument in brief (verify against the code)

- **Pieces:**
  - The VJ Analyzer VST3 in Live 10 analyses the audio and sends it to the VJ Engine over OSC.
  - The VJ Engine is a standalone OpenGL app that outputs to a window, fullscreen, or Spout.
- **13 generative scenes:**

  | # | Scene | Summary |
  |---|---|---|
  | 01 | Hot Blobs | liquid mass, ripples |
  | 02 | Dot Relief | 3D forms as a dot relief |
  | 03 | One Bit | hard threshold |
  | 04 | Corridor | endless fly-through |
  | 05 | Fibers | roots and nerves |
  | 06 | Terminal | invented script |
  | 07 | Ink | blots soaking through paper |
  | 08 | Signal Fog | fine particles, forms condensing out of haze |
  | 09 | Mesh Body | a body of plexus lines that erodes to dust and re-knits |
  | 10 | Morphogen | live reaction-diffusion |
  | 11 | Halo Ring | a ring whose edge the sound pushes |
  | 12 | Emergence | a form rises from darkness and dissolves into dust as the music builds |
  | 13 | Negative | red/cyan negative split with a "duo" palette (added Sept 24) |

- **8 macros:** Intensity, Motion, Color, Space, Impact, Gravity, Viscosity, Detail. All are host
  parameters, so they can be automated in the Arrangement.
- **LOOK (global):**
  - image controls: Grain, Crush, Smear, Glitch, Trails, Symbols, Flash, Cut Rate
  - **FILM:** Halation, Weave, Dust, Blacks
  - **REACTION:** Reactivity, and CALM (fades every reaction out over one bar)
- **Palette:** a 3-colour gradient map. Presets are Blood, Ember, Bone, Ice, Acid, Violet, Rust, Nitrate,
  Cyanotype, Tungsten and Ash, plus a free choice of colours.
- **SNAPSHOTS A–D:** macros, LOOK and palette, recalled with a beat-synced morph (Cut, 1 beat, 1 bar,
  4 bars or 16 bars).
- **Transitions:** cut, crossfade, dip or luma, quantised to the beat or bar. Also BLACKOUT and HIT.
- **Section signals:** `build` and `presence`, band clocks, and phrase LFOs.
- **VISUALS REACT TO:** gates for kick, snare, hat, bass and level. The flash/cut limiter caps them at
  3 per second.
- **Source material:** v2 stages accept **image folders** (`engine/Media/Images/...`) and **text**
  (words rendered in a chosen font, advancing per beat, bar or event). **They do not take video**; only
  the legacy presets do.
- **Hardware budget:** an Intel Iris Xe integrated GPU, with adaptive render scale and a 60 fps target.

### 4.3 The owner's taste (binding)

- **Loves:**
  - ambient and downtempo
  - grain, noise, and 35mm film artefacts
  - The Noise Diary: forms that erode and reassemble, fine plexus lines on black
  - mostly-black frames, with light at the edges
  - rare, correlated events
  - monochrome palettes
- **Hates:** skulls, masks and faces. Even two round holes in a form read as eyes: avoid them.
- **Generative only.** No stock footage.

### 4.4 Visual audit of the existing scenes

- Render each of the 13 scenes and look at the stills. Use:
  - `engine/preset_regression_test.py`, or
  - the engine with `--demo` and OSC `/debug/snapshot`

  Also try a few palettes and LOOK settings through OSC (`/v2/palette`, `/v2/look`; see `USAGE.md`).
- Save the useful stills under `docs/before-it-disappears/stills/`.
- For every scene, write down:
  - its fit to the show, from 1 to 5
  - which movement of the text it could serve
  - what it would have to lose or gain to belong to this piece
  - whether its current kick-driven behaviour survives music with little percussion
- If audio files for the show exist (ask where), run
  `analysis\build\Release\analyze_wav.exe <file> --csv <out>` on each track. Report what the analysis
  actually sees: onset density, level range, build and presence curves. That tells you which reactions
  can work for this score and which cannot.

---

## 5. Research strands

For each strand, go deep, cite what you opened, and end with **"what this means for our show"**.

### A. Dramaturgy and the science of memory, as visual algorithms

Translate the text's ideas into **image processes the engine could run.** Useful starting points
(verify them, and add your own):
- memory reconsolidation: recall makes a memory changeable again (Nader et al., 2000)
- Bartlett's serial reproduction, *Remembering* (1932): stories change each time they are retold
- generation loss and re-recording
- involuntary memory triggered by a sense (Proust)
- the "reminiscence bump" and why old memories feel clear

For each idea, give the process in one sentence and the visual behaviour it implies. For example:
"each recall re-encodes the image through a lossy step, so a returning image is never the same twice".

### B. References: art, dance and performance

Build a curated library of **at least 40 entries.** Seeds to verify and go far beyond:

- **Dance with projection:**
  - Klaus Obermaier, *Apparition*
  - Chunky Move / Frieder Weiss, *Glow*
  - Adrien M & Claire B, *Hakanaï* and *Pixel*
  - Wayne McGregor, *Atomos*
  - Kurt Hentschläger, *ZEE* (fog and light as the image)
  - Israeli and European dance companies that use projection
- **Memory and decay in film and video art:**
  - Chris Marker, *La Jetée* (a film made of stills) and *Sans Soleil*
  - Bill Morrison, *Decasia* (decaying nitrate: note that we have a Nitrate palette)
  - Hiroshi Sugimoto, *Theaters* (a whole film accumulated into one white screen)
  - Bill Viola (extreme slow motion)
  - Tarkovsky, *Mirror*
  - Stan Brakhage
- **Sound works about decay and recall:**
  - William Basinski, *The Disintegration Loops*
  - Alvin Lucier, *I Am Sitting in a Room*
  - The Caretaker, *Everywhere at the End of Time*
- **Visual art:**
  - Gerhard Richter's blurred photo-paintings
  - Christian Boltanski (shadows, archives, light)
  - Idris Khan (layered photographs)
- **The owner's own references:** The Noise Diary, Rainer Kohlberger, Ryoichi Kurokawa.

For each entry give:
- artist, work, year and a link
- what it looks like
- the principle behind it
- **what we take from it and what we must avoid**
- which movement of the text it serves

### C. Stage craft: projecting for dance

Concrete, with numbers where they exist:
- **Projector and light:**
  - projector black level versus stage light: "black" on a cyc is never black, so treat lifted blacks
    as a given
  - contrast, and lumens needed for a given cyc size against the lighting state
- **The dancers:**
  - light spill onto the dancers
  - keeping silhouettes readable against a bright screen
  - a maximum average picture level for dance scenes
- **Surfaces:**
  - front versus rear projection and its consequences (shadows, hot-spot, space)
  - haze in the room (projection beams become visible: "memory is a cloud")
  - cyc materials
- **Output:** aspect ratio, resolution, warping and masking. The engine outputs Spout, so check whether
  a mapping tool is needed.
- **Working with the lighting designer:** colour temperature, and who owns the cyc.
- **Show control:**
  - Ableton Arrangement as a timecode spine
  - cue discipline
  - redundancy: a pre-rendered backup of each section, and failover on a crash
  - a pre-show checklist
- **Safety:** flash and photosensitivity guidance for public performance. We already cap flashes at
  3 per second; check what is recommended.
- **Rehearsals:** how to iterate with a choreographer when the visuals are live and generative.

### D. Music to image for this score

- Dance-theatre music often lacks steady percussion. Design the reaction profile around **level,
  presence, build, spectral descriptors, phrase LFOs and cues**, rather than kicks.
- Decide per section: what reacts to what, what never reacts, and what is only cued.
- Use the `analyze_wav` results from section 4.4 when you have them.
- Research how other AV artists handle slow, sparse music. Start from `RESEARCH-REPORT.md` B3 and
  extend it.

### E. The "Because" finale

- **Verified facts about the song** (cite sources): how it came about, including the widely reported
  story of a reversed Beethoven "Moonlight" Sonata; the stacked three-part harmony; the Moog and the
  harpsichord; its place on *Abbey Road*; the a cappella version on *Love* (2006). Mark anything you
  cannot verify.
- **What those facts give the image:**
  - reversal and replay
  - stacked voices as stacked layers
  - the round world as a circle (we have Halo Ring)
  - the blue sky: perhaps the only blue moment of the piece (Cyanotype)?

  These are only prompts; judge them.
- **The finale's job in the arc:** is it the "mark"? A release? The first fully clear image of the
  night? Decide, and argue it.
- **A minute-by-minute plan** for the 5 minutes (section 7, part 7).
- **Rights:** a public performance of a Beatles composition or recording needs licensing. Flag it for
  the producer, and say which bodies are typically involved (in Israel, for example, ACUM for the
  composition, plus rights for the recording if the original is used). Verify this; it is not legal
  advice.
- **No lyrics in the document.** Paraphrase only.

### F. Copy and language

- **A naming system** for the show presets, sections and snapshots. The name should tell the operator
  what the preset means dramaturgically. Propose Hebrew and English, and say which one goes on the
  plug-in.
- **A one-line idea** for the video art, in Hebrew and English.
- **A visual statement** of about 120 words in Hebrew and English, usable in the programme, grants and
  press.
- **Section titles.**
- **On-screen text:**
  - Should any of the choreographer's words appear on screen? Give a clear recommendation with
    reasons.
  - If yes: which fragments, and how they appear and disappear. Consider the engine's text source, and
    the text's own shrinking lines.
  - Also think about the risk of illustrating the dance with words.

---

## 6. Design constraints

- **Generative only.** Footage is out, including rehearsal footage of the dancers. The one exception:
  you may propose it as a clearly flagged option, with its engine cost (v2 stages take no video today)
  and its conflict with the owner's taste.
- **No faces, skulls or masks,** and no forms that read as eyes. **No figurative bodies that duplicate
  the dancers.** Abstract bodies such as Mesh Body are allowed only if they serve a movement of the text.
- **Dancers stay readable.** Give a luminance budget per section, and say where light may sit in the
  frame.
- **GPU budget.** For every new idea, give its cost on the Iris Xe: S / M / L, the number of passes,
  and any buffers.
- **One operator.** The owner may be running music and visuals at the same time. The live layer must fit
  on one small controller, and the show must survive with the live layer untouched.
- **Repeatable and recoverable.** Every section can be restored from a known state: a snapshot, a preset
  and a palette.
- **Flashing** respects the existing 3-per-second cap and your photosensitivity findings.

---

## 7. Deliverable: `docs/before-it-disappears/CONCEPT.md`

Write it in **Hebrew**, keeping technical terms, scene names and parameter names in English. Use
clear, short sentences.

1. **Executive summary:** 10 bullets.
2. **Reading the text:** the dramaturgical analysis from section 3, and a translation table:
   *text movement / key line | image principle | engine means (existing or new) | section of the show*.
3. **Three creative directions.** They must differ in principle, not just in palette. For each:
   - a name, a one-line idea, and a core metaphor
   - visual language: palette (3 hex values per state), texture, motion tempo, composition, and the
     percentage of black in the frame
   - how it treats the dancers, light and shadow
   - how it arcs across the whole show, and how it ends in "Because"
   - 4–8 stills or moodboard frames (rendered from our engine where possible, plus reference links)
   - risks

   Then give **your recommendation** and why.
4. **The show arc.** One row per section:
   - time, music, and text movement
   - visual state: scene, palette, LOOK and FILM
   - reaction profile: what reacts to what, and what never reacts
   - whether it is cued or live, and the transitions in and out
   - luminance budget

   Mark unknowns as placeholders.
5. **Show presets.** 8–14 presets for this piece. For each:
   - a name (from the copy system) and its role in the dramaturgy
   - a base scene (existing or new)
   - macro defaults and live ranges
   - palette (hex values), LOOK and FILM values, REACT TO profile and Reactivity
   - snapshot plan and transitions
   - what the controller does in it
   - GPU cost and build effort

   Group them as:
   - **(a)** settings and variants of existing scenes (JSON, palette, snapshots only)
   - **(b)** new scenes (new shader)
   - **(c)** new engine features the show needs
6. **New engine features, ranked.** Consider at least these seeds; keep, merge or kill them:
   - **Show memory:** capture frames at chosen cues, and resurface them later degraded. Each recall
     degrades them further (reconsolidation, Basinski, Lucier). Images from the opening return in the
     finale, changed.
   - **Accumulation / long exposure:** the whole show sums into one final "mark" (Sugimoto).
   - **Edge-only memory:** keep the outline while the inside dissolves ("keeps the outline, never the
     whole story").
   - **Palette drift over the whole show:** colours slowly change, like ageing film stock.
   - **Time controls:** slow, freeze, replay and reverse ("we slow it down, we replay it").
   - **Recurring motif:** a "familiar light" that returns in several sections.

   For each feature: what it is, why the show needs it, how it behaves, where it lives in the
   architecture, GPU cost, effort, and risk.
7. **The "Because" finale in detail:** minute by minute, cued against the music's structure, tied to
   the text's ending.
8. **Live control plan:**
   - what the Arrangement automates
   - what stays live
   - a mapping for a generic Korg (8 knobs, 8 faders, pads)
   - panic controls: BLACKOUT, CALM, a safe snapshot
9. **Stage and tech:**
   - questions for the lighting designer and the venue
   - a projection checklist, a tech-rider draft, and a backup plan
   - a rehearsal and test plan
10. **Copy:** the naming system, the one-line idea, the visual statement (Hebrew and English), section
    titles, and the on-screen text recommendation.
11. **Reference library:** 40+ entries, grouped, with links and "what we take".
12. **Decisions needed:** from the owner and from the choreographer, each with your recommended answer.
13. **Build plan:** phases toward a first rehearsal, the first run-through and the premiere. List what
    to build first, and what can wait.
14. **Sources:** every URL used.

---

## 8. Research standards

- Cite a URL for every non-obvious claim. **Only include links you actually opened.** Never invent one.
- Write "unknown" rather than guess. Label inferences as inferences.
- Separate verified facts about "Because" and the reference works from lore.
- Build on `docs/RESEARCH-REPORT.md` and `docs/REFERENCE-LIBRARY.md`. Do not repeat them; reference
  them by section.
- **Never quote the lyrics of "Because".** You may quote the choreographer's text freely: it belongs to
  the production.
- Be concrete: hex values, seconds, bars, percentages, pass counts. A direction without numbers is a
  mood, not a plan.
- If you hit an output limit, split `CONCEPT.md` into numbered parts and continue.
