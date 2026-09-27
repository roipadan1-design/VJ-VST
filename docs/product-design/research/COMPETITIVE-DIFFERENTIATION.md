# Competitive differentiation: what nobody else is doing, and what we could own

*27 September 2026. Research only: no code, preset or existing doc was changed.*

*Scope: new findings only. It deliberately does not repeat what is already in `docs/RESEARCH-REPORT.md` (23 Sept: Videosync, Zwobot, VS, Arkestra, Photism, T3X2R, Synesthesia, Resolume, TD audioAnalysis, film craft, 24-entry reference library), `docs/product-design/COMPETITOR-WORKFLOW-RESEARCH.md` (26 Sept: 30-tool landscape table, workflow, cue lists, safety, Ableton linking, Mix Map, co-pilot), `DESIGN-DOSSIER.md`, `CONTROL-MAP.md`, `REACTION-DESIGN.md` (REST / accents / drop / scars / time kick) and `WORK-PLAN-2026-09-26.md`. Where this doc touches an idea already there, it says so and adds only the new angle.*

**Evidence tags** (same convention as the workflow report): **[D]** docs/manual, **[M]** marketing, **[P]** press/editorial, **[U]** user report (forum/review), **[A]** academic/standards, **[S]** search snippet only, **[I]** my inference.

---

## TL;DR

1. **Our real edge is narrower than we have been saying.** Magic Music Visuals and VDMX already take several audio inputs, one per instrument. What nobody has is *zero-routing role semantics inside Live on Windows*, together with a generative, film-grade look. Say that.
2. **The unclaimed big idea: one gesture plays both sound and image.** Only hardware does it (Korg Kaoss Entrancer "Combi", Teenage Engineering OP-Z). The plug-in already sits in the audio path, so coupled AV gestures are feasible (§4 idea 3, §5.1).
3. **Pros judge a tool by its failures.** The most feared is a frozen or desktop output. An output guardian process that holds the last frame is cheap for us, and nobody advertises one (idea 1).
4. **Timing: the performer sees the worst sync in the room.** Sound flies about 2.9 ms per metre, and the eye flags sound arriving more than 45 ms early. At 20 m our estimated 75-140 ms chain is mostly in sync for the audience, but not at the laptop. Measure it, compensate grid events, and tune for a listening distance (idea 2).
5. **The missing "no ceiling on virtuosity".** The PLAY view is the low entry fee. Motion loops, per-scene articulations (a visual drum rack), a fixed physical vocabulary with LED feedback and foot control make the instrument something you practise (ideas 7-9, 13).
6. **Memory is a feature nobody has.** A looper / granulator of the instrument's own output, degrading with each replay, is both product and the subject of the dance show (idea 4).
7. **Content and community.** Perform at 720p, then render the same performance at 4K offline; share looks as patch codes; get one "set print" still per night (ideas 5-6, §5.3).

## Contents

1. Competitive landscape: new findings only
2. What "play it like an instrument" requires (UX evidence)
3. Wider reference pool (artists and studios)
4. Ranked feature / direction ideas (value vs cost)
5. Think outside the box
6. Sources

---

# 1. Competitive landscape: new findings only

## 1.1 Three corrections to what we currently believe

These matter because they change *which* claims we can make when we sell the product.

| Current belief (where) | What I found | So what |
|---|---|---|
| "No tool offers instrument roles; Synesthesia takes one stereo mix" (COMPETITOR-WORKFLOW §5, B3) | True **inside a DAW**, false outside it. Magic Music Visuals accepts "multiple audio/MIDI/OSC inputs simultaneously, including … multichannel audio devices", so visuals "can react differently to every individual instrument" [S] https://magicmusicvisuals.com/. VDMX lets each Audio Analysis plug-in pick "an audio source and channel", with named filters and per-filter centre, range, gain and smoothing [S] https://docs.vidvox.net/vdmx/vdmx_plugins. | Our edge is not "multi-input". It is **zero-routing role semantics inside Live**: drop it on the kick track and it *is* the kick, with no audio interface loopback, no channel numbers, no filter tuning. Say that, not "multi-track analysis". |
| EboSuite: platform and date "unknown" (COMPETITOR-WORKFLOW table) | EboSuite 2.0 shipped **April 2022, macOS only**; the developer says cross-platform video "is no minor task" [P] https://cdm.link/ebosuite-2-0-hands-on-live-video-recording-and-more-makes-vjing-feel-native-in-ableton-live/. The reviewer called it "a full realization of an audiovisual instrument" inside Live. | On **Windows + Live** the only clip-as-video tool is still Videosync 2 (Live 12 only). The Windows/Live gap is real and still open. |
| The **Scope** idea (real waveform as a line; REACTION-DESIGN idea 11) reads as novel | It is table stakes elsewhere. VDMX publishes "FFT and raw audio waveform … as grayscale video streams that are 1 pixel tall" [S] (same VDMX page). MadMapper's GLSL materials get 1D audio FFT textures with ATTACK / DECAY / RELEASE per input [S] https://docs.madmapper.com/madmapper/6/3.-media/writing-custom-glsl-materials. EYESY's whole "scope" mode family is exactly this [S] https://www.perfectcircuit.com/critter-guitari-eyesy.html. Notch has a Sound Texture node [D] https://manual.notch.one/2026.1/en/docs/learning/working-with-audio/. | Still worth building, but the differentiated version is **role-separated scope**: the kick track's waveform and the pad track's waveform as two different lines, which only a per-track instrument can do. |

## 1.2 Tools the earlier reports did not examine

| Tool | What it is | Audio-reactivity model | What is new for us |
|---|---|---|---|
| **MadMapper 6** (garagecube) | Mapping + generative "Materials" (GLSL), Mac/Win | Bands (amplitude, bass, medium, treble), `bpm`, `beatCount`, fractional beat divisions (1, 4, 16 beats); Ableton Link [D] https://docs.madmapper.com/madmapper/6/11.-live-performance-and-control | **One Learn button for every protocol**, with mappable items tinted by protocol (keyboard blue, MIDI green, OSC yellow, DMX purple). **OSCQuery** so other apps discover parameters automatically. Wildcard OSC ("/surfaces/*/opacity" to 0 = black out everything). 14-bit pitch-bend for precise control. [D] same page |
| **Notch** (2026.1) | Real-time 3D/VFX, blocks run inside Resolume / media servers | Sound Capture / Loader nodes; Sound FFT, Sound FFT Region and Sound modifiers; Sound Deformer; Sound Texture [D] (Notch page above) | The block host warns that **"loading a Block will pause output until loading completes"** [S] https://manual.notch.one/2026.1/en/docs/workflows/working-with-media-servers/blocks/using-notch-in-resolume/. Our engine prebuilds the next scene, so "no output stall on scene load" is a claim we can make and test. |
| **Magic Music Visuals** | Modular node visualiser, Win/Mac, Studio / Performer editions | Per-node audio features with thresholds and smoothing; multichannel inputs (above) | A competitor-authored 2026 ranking calls it the "best modular power-user visualizer" [P] https://vvavy.io/blog/2026-best-music-visualizers/, and a snippet adds that you operate "a desktop creation tool rather than … picking a scene" [S]. Confirms the market split: modular power vs. pick-and-play. Nobody owns "pick-and-play with depth". |
| **VDMX6** (audio side) | Mac modular VJ | Named per-filter bands with their own smoothing; analysis can drive a movie's *rate* so it "only plays forward when there is an audio input" [D] https://vdmx.vidvox.net/tutorials/more-fun-audio-analysis-techniques | Same idea as our band clocks; VDMX leaves it as a recipe for the user to patch. We ship it as a default. |
| **Smode** | Windows real-time compositor / media server, free Community tier | Audio-reactive nodes; MIDI / OSC / DMX / timecode [S] https://www.smode.io/en/solutions/live-visual-music-events | The heavyweight Windows option (tour and LED-wall scale). Not our market, but it proves Windows pros exist and pay. |
| **VJ Loop Studio 3D** (new, 31 July 2026) | Windows 10/11, €99 lifetime | 9 sources incl. "Low Blend" / "High Blend" because "raw bands are twitchy"; source + range + amount per parameter; 7 tempo-synced modulators [M] https://vjloopstudio.com/posts/audio-reactive-guide/ | A brand-new **Windows, one-time-price** competitor, but loop-generation first. Its per-parameter routing is the Resolume/VS model we already reject. Notable: live preview reacts to the input; **export re-analyses the audio file offline, frame by frame**, for reproducible renders. |
| **CoGe VJ** | Mac, Quartz Composer based | Audio parameter modulation, MIDI clock | Last release 1.8 (June 2017); 2.0 announced in 2021 and not shipped [S] https://imimot.com/blog/cogevj-development-update/. Effectively stalled. |
| **Cathodemer** | Windows (Steam) video synth + CRT simulator | "All parameters can be controlled via MIDI and audio input" [S] https://www.hypertonal.net/cathodemer/ | A Windows *instrument*, not a VJ mixer: "playable live … just like with an expressive musical instrument". Its differentiator is a **display simulator** (the screen itself is part of the look). Same move as our film chain, applied to CRT. |
| **Lumen** (Paracosm) | Mac semi-modular software video synth | MIDI-driven (audio-reactive via MIDI) [S] https://www.synthtopia.com/content/2016/11/28/lumen-video-synthesizer-for-mac-gets-midi-control/ | 150+ patches. Proves the "patch" vocabulary works for visuals when the synth structure is fixed and learnable. |
| **Screen Sampler** (2026) | Browser, free / open source | Samples regions of your own screen onto 3D shapes; MIDI [P] https://limeartgroup.com/the-mega-list-of-vj-software-and-tools/ | Novel input idea: the performer's own screen (DAW, mixer) as texture. Cheap trick for us too: Live's own UI or waveform as a media source. |

**Not worth more time:** AI tools (Kaiber Superstudio is offline generation with "Beat Sync", not live [S]), VVavy (browser, author ranks itself #1), GrandVJ, Modul8, MixEmergency. The earlier verdict on real-time AI stands.

## 1.3 The "visual instrument" lineage (hardware and instrument-like apps)

These are the products that already behave like instruments rather than mixers. None of them sits in a DAW, and none has a film-grade look. Their design choices are the most transferable part of this whole report.

| Instrument | The design move | Evidence |
|---|---|---|
| **Korg Kaoss Pad Entrancer (KPE1)** | **Combi mode: one X-Y gesture drives a matched audio effect *and* video effect.** "Choosing a particular video effect also calls up a complementary audio effect" (auto-rotation + tape echo, emboss + phaser). Tempo from knob, auto-detect or tap; a Mute/Freeze toggle; two 6-second video sample slots. | [P] https://www.soundonsound.com/reviews/korg-kpe1-kaoss-pad-entrancer |
| **Teenage Engineering OP-Z** | Visuals are **tracks in the same sequencer as the music**: Photomatic (up to 10 sequences of 24 pictures, now video too), and VideoLab, which exposes "the inner control data" of the OP-Z to Unity so every note and knob can drive a scene. TE worked with Keijiro Takahashi and Unity Tokyo. | [M] https://teenage.engineering/products/op-z/videolab ; Photomatic numbers [S] https://teenage.engineering/products/op-z , https://cdm.link/2018/10/op-z-dmx-unity-3d/ |
| **Critter & Guitari EYESY** | **5 knobs, same meaning in every mode**: size, X, Y, **knob 4 = amount of reaction**, knob 5 = background. Modes split into **scope** (draws the audio) and **trigger** (a transient above threshold fires an animation). Modes are open Python; **209 community modes** on Patchstorage (top download 412). | Knobs and modes [S] https://www.perfectcircuit.com/critter-guitari-eyesy.html ; counts [D] https://patchstorage.com/platform/eyesy/ |
| **Sleepy Circuits Hypno** | Two shape oscillators; **"turning past the midpoint causes the shapes to begin to feedback on themselves"** (five feedback modes). One knob crosses a qualitative boundary instead of adding a switch. 7 CV + 2 trigger inputs at audio-Eurorack levels. | [S] https://sleepycircuits.com/hypno |
| **Lumen / Cathodemer** | Fixed signal-flow synth you can reroute; patches as the unit of sharing (above). | above |

**Lessons (inference [I]):**
1. **A constant control vocabulary across patches** (EYESY knob 4 is always "reaction") is what lets people play without looking. We already have fixed macro *directions*; the missing part is a *physical* constant: the same Korg knob is always REACT, in every scene, forever.
2. **The instrument pairs a picture gesture with a sound gesture** (Kaoss Combi, OP-Z). Every VJ tool in the earlier reports treats sound as *input only*. None lets the visual gesture also play the sound. For a solo musician-visualist this is the biggest unclaimed idea in the field (see §5).
3. **Knob positions can cross thresholds** (Hypno's midpoint feedback). That is a way to hide a mode switch inside a macro without adding a control.

## 1.4 Systems that already fuse music and image as one work

| Work / system | The fusion principle | Evidence |
|---|---|---|
| **Tarik Barri, Versum** | A 3D world seen and heard from a flying camera with virtual microphones: "every visual object creates sounds, and every sound is visual"; the **flight path is the composition**. Used for Thom Yorke, Nicolas Jaar, Monolake, Paul Jebanasam. On AV: "Sound provides a context for the visuals and vice versa." | Quote and working method [P] https://electronicgroove.com/interview-tarik-barri/ ; Versum description [S] https://tarikbarri.nl/projects/versum |
| **INFRATONAL (Louk Amidou)** | Hand gestures perform the music *and* the visuals as "intangible instruments", built in TouchDesigner. | [S] https://derivative.ca/community-post/exploring-humanized-algorithmic-art-infratonal/65499 |
| **Brian Eno, 77 Million Paintings** (2006) | 296 hand-made slides; software superimposes **four at a time** in slowly changing permutations, with the music randomised the same way, so a combination never repeats. The title is the combination count. | [S] https://en.wikipedia.org/wiki/77_Million_Paintings |
| **Endel** | Endless generative soundscapes that adapt to **time of day, weather, heart rate, light level**; "each soundscape has their own generative visual". Beat intensity follows the day's "rises, peaks and rests". | [S] https://endel.io/technology ; https://endel.zendesk.com/hc/en-us/articles/360012517639-How-Endel-Works |
| **Robert Henke, CBM 8032 AV** | Five 1980 Commodore machines, 1 MHz, 32 KB; sound and image from the same CPU. "It's minimalism by design … everything has to be highly structured and reduced." | [S] https://roberthenke.com/concerts/cbm8032av.html ; https://clotmag.com/news/performance-robert-henkes-cbm-8032-av-premiere-at-unsound |
| **Pauric Freeman** | TouchDesigner + Eurorack via Expert Sleepers ES-9 (DC-coupled CV and audio as data). "Because the sound and visual are actually separate entities, you have to work carefully to merge the two." Chooses *waveforms vs gates* per the psychological effect wanted. | [P] https://derivative.ca/community-post/pauric-freeman-working-sound-image-input-devices-and-human-perception/65814 |

**Common thread [I]:** the strongest AV works share **one control signal or one generative decision between sound and image**, rather than analysing finished audio after the fact. Our analyser listens after the fact; our MIDI-as-hits path and the planned MIDI cue track are the only places we share the *cause*. §4 and §5 push this further.

## 1.5 User voice (what hurts under real conditions)

Reddit could not be fetched (blocked for this agent, as it was for the earlier report). These come from vendor forums, community sites and editorial pieces.

| Pain | Evidence | Our position |
|---|---|---|
| **The output freezes mid-show.** A first gig: output froze after 20 minutes, "a big ugly blue screen for two minutes", then again 5 minutes later. Other threads: crashes "2-3 times during a full night" (7.23), and 30-60 s whole-app freezes 4-5 times in a 4-hour show. Advice: keep a hardware mixer to switch to. | [S] Resolume forum https://resolume.com/forum/viewtopic.php?t=10254 , https://resolume.com/forum/viewtopic.php?t=11091 (403 on open; search snippets) | Separate engine process + state re-send is structurally better. The missing piece is a **last-good-frame hold** and a **watchdog** (COMPETITOR-WORKFLOW R6 covers relaunch; §4 adds the frame hold). |
| **Login and licence friction.** Arkestra's mandatory login "doesn't feel right" to a user even though no data is kept. | [U] https://www.elektronauts.com/t/arkestra-audio-reactive-visuals-with-ableton-link-for-macos/216978 | Sell it offline-activated. A stage machine is often offline. |
| **Users want per-track sources and visible filters.** The same thread asks for "per-track audio filters or inputs", visible frequency scales and Q, and "smoothed random". | [U] same thread | Per-track is what we already are. Show it (Mix Map, B3 in the workflow report). |
| **Raw bands are twitchy.** A 2026 vendor builds smoothed "Blend" sources for exactly this reason. | [M] https://vjloopstudio.com/posts/audio-reactive-guide/ | Already solved in our model (conditioning at the source + REACTION-DESIGN stretch). |
| **Controller reality on a dark stage.** "On a dark stage with strobes firing, you cannot read your laptop screen reliably"; endless encoders with LED rings are the favourite because "the same knob can serve dozens of parameters without a single jump"; "Save your mapping file to the cloud and to a USB stick." | [P] https://limeartgroup.com/best-midi-controllers-of-vjs-visual-artists/ | We have no LED feedback out and depend on Live's MIDI Map. See §2 and §4. |
| **Overuse kills impact.** "A hard strobe is a gasp. Applied for forty minutes, it is wallpaper." Dim in breaks: "darkness … will amplify the effect of light". | [S] https://limeartgroup.com/vj-mistakes-beginners/ ; https://www.stvinmotion.com/three-beginner-vj-mistakes/ | Independent confirmation of REACTION-DESIGN's REST and accent selection. Nothing new to add. |

## 1.6 Where the white space is now

Updated map of what is taken and what is open, after this pass:

| Claim | Who already owns it | Open for us? |
|---|---|---|
| Clip-as-video inside Live | Videosync (Live 12 on Windows), EboSuite (Mac) | No; do not chase |
| Modular power patching | TouchDesigner, Magic, VDMX, Notch, Smode | No |
| Pick-a-scene simplicity | Synesthesia, Photism, NestDrop | Crowded |
| **Generative, film-grade look + instrument semantics + inside Live on Windows** | nobody | **Yes, core** |
| **One gesture plays sound and image** (Kaoss Combi, OP-Z) | Only hardware, only outside a DAW | **Yes, big** |
| **Instrument-grade timing honesty** (constant latency, venue-distance compensation, no-stall scene loads) | nobody advertises it | **Yes, cheap** |
| **Shareable visual patches with a constant control vocabulary** (EYESY + Patchstorage model) | EYESY (hardware), Synesthesia marketplace (scenes, not patches) | **Yes, later** |
| **Context-adaptive generative behaviour** (Endel-style: time in set, room, phrase) | Endel (audio only), nobody in VJ | **Yes, speculative** |

---

# 2. What "play it like an instrument" requires

The earlier docs settled the *layout* (PLAY / EDIT / SOURCE, 8 knobs, cue + GO) and the *reaction model*. This section adds what they did not cover: hard numbers for timing, the instrument-design literature, and the failure modes that decide whether a pro trusts a tool on stage.

## 2.1 Timing: the numbers, and a finding about who sees the lag

**Reference numbers**

| Quantity | Value | Source |
|---|---|---|
| Acceptable delay from gesture to *sound* in a digital instrument | "10 milliseconds", with variation that "should not exceed 1 ms" | Wessel & Wright, NIME 2002 [A] https://arxiv.org/pdf/2010.01570 |
| How to beat jitter | "Time tags allow one to implement a scheduling discipline that reduces jitter by trading it for latency" | same [A] |
| When viewers *notice* sound and picture out of sync | Sound early by more than **45 ms**, or late by more than **125 ms** (ITU-R BT.1359 detectability) | [P] https://www.tvtechnology.com/opinions/av-synchronization-how-bad-is-bad |
| When viewers *object* | Sound early by more than **90 ms**, late by more than **185 ms** (acceptability) | same |
| Stricter budgets | ATSC: +15 / −45 ms at the encoder; film (BR.265) about ±22 ms at 24 fps | same |
| Projector input lag | Cheap DLP "about 33 ms", cheap LCD "under 50 ms", home-cinema models 24-50 ms; **"Frame interpolation can single-handedly add over 100 milliseconds"** | [P] https://www.projectorcentral.com/projector-input-lag.htm |
| Sound in air | 343 m/s, so about **2.9 ms per metre** | physics |

**Our chain, estimated [I]** (to be measured, see idea 2 in §4):

| Stage | Estimate |
|---|---|
| Live audio buffer (256 samples at 48 kHz) | 5 ms |
| Onset detection window | 5-20 ms |
| OSC to the engine (localhost UDP) | < 1 ms |
| Waiting for the next render frame at 60 fps | 0-17 ms (mean 8) |
| Render | ≤ 17 ms |
| Present / compositor (1-2 frames with v-sync) | 17-33 ms |
| Projector | 24-50 ms (150 ms or more with interpolation left on) |
| **Picture after the sound leaves Live** | **about 75-140 ms**, before PA processing |

**The finding.** Visual lag here means "sound early", and the eye tolerates only 45 ms of that. But an audience member stands away from the PA, and the sound reaches them late while the light reaches them at once:

| Listener position | Sound's flight time | Perceived "sound early" (75-140 ms chain) | Verdict |
|---|---|---|---|
| The performer, 1-2 m from a monitor | 3-6 ms | about 70-135 ms | **Noticeable** |
| Audience at 10 m | 29 ms | about 45-110 ms | Borderline |
| Audience at 20 m | 58 ms | about 15-80 ms | Mostly fine |
| Audience at 30 m | 87 ms | about −10 to +55 ms | In sync |

- **The performer always sees the worst sync in the room.** If the owner tunes by eye from the laptop, he will over-correct and make it *worse* for the audience. The tool should say so, and tune for a chosen listening distance.
- **Predictable events can be early on purpose.** Anything on the beat grid (cuts, flashes on downbeats, breath, bar-locked changes) is known in advance from Live's transport. The engine can fire it at `beatTime − measuredLatency + audienceDistance/343`. Only unpredictable onsets remain late. RESEARCH-REPORT B4 mentions "lookahead from Live's transport"; the new parts are the measured latency and the audience-distance term.
- **Jitter matters more than delay.** A constant 80 ms is learnable; ±1 frame of wander is not (Wessel & Wright's 1 ms point, scaled to frames). Timestamped OSC is already planned (RESEARCH-REPORT B6). Keep it.
- **Venue projectors ship with interpolation on.** A 100 ms+ mode switch the VJ never sees. A pre-show "sync check" is a real feature, not a nicety.

## 2.2 What the instrument-design literature says (and what it means here)

| Principle | Source | For this product [I] |
|---|---|---|
| **"Low entry fee with no ceiling on virtuosity"** | Wessel & Wright [A] (above) | The PLAY view is the entry fee. The ceiling is missing: nothing yet rewards practice (no articulations, no motion recording, no gesture vocabulary). See §4 ideas 7-9 and §5. |
| **"Programmability is a curse"** | Perry Cook, NIME principles [A] https://www.idmil.org/publication/expert-commentary-perry-cooks-principles-still-going-strong/ | Confirms "no mod matrix for the performer". |
| **"Some players have spare bandwidth, some do not"** | same | The owner plays the music *and* the picture. He has almost no spare bandwidth, so the default must be good with zero attention, and every live action must be one motion (or a foot). |
| **"Instant music, subtlety later"** | same | First touch must produce a big, right-looking change. REACT at 50 = "as designed" already does this. |
| **"Make a piece, not an instrument or controller"** | same | *Before It Disappears* is the piece. Build for it, then generalise. This supports shipping per-piece "scores" (§5). |
| **Instrument efficiency** = output complexity / input complexity; **diversity** at micro, mid and macro scale | Sergi Jordà, *Digital Lutherie* [S] https://www.researchgate.net/publication/228715881_Digital_Instruments_and_Players_Part_II-Diversity_Freedom_and_Control | Check each macro for efficiency: one turn should move a whole, coherent picture. Check diversity at three scales: within a scene (knobs), between scenes (contrast), between sets (characters, looks). |
| An **"inexhaustible, infinitely variable … substance"**, a **"non-diagrammatic"** space, **"perceptually-motivated mappings"** | Golan Levin, *Painterly Interfaces* (MIT, 2000) [A] http://www.flong.com/archive/texts/publications/thesis/index.html | A scene should never loop visibly and never run out. Its controls should read as touching the picture (an XY on the image), not as editing a diagram. |
| "Because the sound and visual are actually separate entities, you have to work carefully to merge the two in the viewer's perception" | Pauric Freeman [P] (§1.4) | Every mapping is a perception claim. Choose the signal type (gate vs waveform) by the feeling wanted. |
| Improvisation matters: real time "adds a type of unpredictability and excitement" | Tarik Barri [P] (§1.4) | Keep a controlled amount of chance (seeds, accents) inside every scene. |

## 2.3 Stage conventions and fears (evidence, beyond what the workflow report covered)

**Controls**
- **Endless encoders with LED rings** are the VJ favourite, because they never jump when the target changes, and because on a dark stage "you cannot read your laptop screen reliably" [P] (lime guide, §1.5). We send nothing back to the controller today.
- **Constant meaning per physical control** (EYESY's knob 4 is always reaction). This is what allows eyes-up playing. Our macros keep their *direction* per scene, but their physical position depends on the user's Live MIDI Map, which lives per set.
- **One Learn for everything, coloured by protocol** (MadMapper) and **parameter auto-discovery** via OSCQuery [D] (§1.2). This is how a phone or tablet remote configures itself.
- **Perform Mode** (TouchDesigner): the editor is closed, one output window runs, and fullscreen-exclusive gives "stutter-free playback" [D] https://docs.derivative.ca/Perform_Mode. Our engine window is already editor-free; fullscreen-exclusive is worth testing on Iris Xe.

**Transitions**
- "Cut on action, fade on pause" [S] (search summary of VJ editing guides). This matches our cue + GO with bump vs crossfade.
- The feared bad transition is the **stall**: Notch blocks "pause output until loading completes" [S] (§1.2). Our prebuild avoids it; make it a tested guarantee (shader warm-up of every scene at engine start).
- Darkness hides a change. Theatre changes scenes in a blackout, and REACTION-DESIGN's REST makes dark moments. A GO that waits for the next REST or DROP makes the change invisible or turns it into the event (§4 idea 11).

**Failure modes, ranked by how often they appear in the evidence**
1. **Frozen output or desktop / blue screen on the projector** (Resolume threads [S]). The audience sees the operating system.
2. **Output stall** on scene or content load (Notch [S]).
3. **Output placement lost** after a restart (workflow report R6).
4. **Mapping lost** ("save your mapping file to the cloud and to a USB stick" [P]).
5. **Sync drift** or a venue projector adding 100 ms (above).
6. **Photosensitivity** (covered: workflow R5 and the show's C-stage-craft).
7. **Mis-trigger** (covered: BLACKOUT isolated).

Items 1, 2 and 5 are not covered by any existing doc, and they are what a professional judges a tool by.

---

# 3. Wider reference pool (artists and studios)

**Excluded as already documented:** Kohlberger, Kubelka, Ikeda, Ratté, Kurokawa, Prudence, Warnecke, Murata (RESEARCH-REPORT); Umeda, Hentschläger (*ZEE*, *FEED*), Langheinrich (*Waveform B*), Nonotak, Henke *Lumière*, Tim Hecker, Murcof / AntiVJ, Dumb Type, McCall, Janssens (before-it-disappears B1); Brakhage, Morrison, Reble, Sharits, Conrad, Basinski and the other memory-decay references (B2).

| # | Artist / work | Why it belongs next to The Noise Diary and phs.wrk | What to take (for scenes or product) | Evidence |
|---|---|---|---|---|
| 1 | **404.zero** (Kristina Karpysheva, Sasha Letcius; St Petersburg, since 2016) | Monochrome code-art; "structural metamorphosis"; TouchDesigner visuals + modular sound; mini-album *Black Sunday* (2020) as a monochrome AV track | Their "Machines": systems that "continue to live and breathe on its own"; drones that "only last for once". Scenes should be non-repeating machines, not loops. Sound and image are made separately and synchronised live, like ours. | [P] https://clotmag.com/interviews/404-zero-the-architecture-of-nothingness-and-metaphysics-of-time-and-space-in-mind-bending-audiovisual-code-art ; [S] https://www.factmag.com/2020/03/13/404-zero-black-sunday-av-album/ |
| 2 | **MFO (Marcel Weber)**: Berlin Atonal director for lights and visuals; Ben Frost *A U R O R A* (Unsound 2014); Emptyset *Dissever* (Atonal 2025) | The industrial-dark lineage behind phs.wrk: "blinding strobes, sudden blackouts"; mixes analogue projectors, film, lenses, mirrors and chemicals with generative software | Visuals as **light architecture**, not a screen: beams through haze, strobes, blackout as material. Argues for a "light" output mode (a DMX / strobe channel driven by the same accents; Beam for Live proves the market). | [S] https://mfoptik.de/about/ ; https://www.berlin-ism.com/en/news/ben-frost-mfo ; https://clotmag.com/oped/atonal-2025-the-house-that-jack-built-five-days-of-noise-darkness-in-the-looming-corridors-of-kraftwerk-berlin |
| 3 | **GRIEND** (Puce Mary & Rainy Miller), Atonal 2025 | "Near-total darkness with only occasional strobes" | Extreme form of REST: darkness is the default state, the light is the rare event. A "Dark" character below BREATHE is a coherent option. | [S] same Atonal 2025 review |
| 4 | **Tarik Barri, *Versum*** (Thom Yorke, Nicolas Jaar, Monolake, Paul Jebanasam *Continuum*) | Dark 3D space, points of light, slow flight | **The camera path is the performance.** Corridor, Membrane and Mesh Body could expose a "fly" gesture (XY = heading, a fader = speed) whose path can be recorded and replayed. | §1.4 |
| 5 | **Pauric Freeman** (Dublin; TouchDesigner + Eurorack) | Hyper-digital, minimal primitives on black, strictly synced to modular patches | Choose gate vs waveform per desired feeling; the ES-9 approach (CV as data) = our MIDI-as-hits path, extended to CC and pitch. | §1.4 |
| 6 | **INFRATONAL** (Louk Amidou; Paris) | Monochrome algorithmic forms played by hand | Gesture plays both media; demoscene lineage. | §1.4 |
| 7 | **Lis Rhodes, *Light Music*** (1975; Tate collection) | Black-and-white line drawings printed onto the optical soundtrack; "What you hear is equivalent to what you see" | **The image is the sound.** Grounds the "hear the picture" idea (§5). Line-work in monochrome = our plexus / fibre scenes. | [S] https://www.tate.org.uk/art/artworks/rhodes-light-music-t13857 ; https://www.acmi.net.au/stories-and-ideas/seeing-sound-lis-rhodes-light-music/ |
| 8 | **Bruce McClure**, projector performances | Modified 16 mm projectors, loops of black emulsion with clear frames (flicker), **driven through guitar effect pedals**, optical sound, complete darkness | Pedals as the interface for a performer whose hands are busy; flicker as rhythm (links to the Kubelka "Flicker Grammar" idea already in the library); the projector itself as instrument. | [S] https://www.artforum.com/features/powers-of-projection-the-art-of-bruce-mcclure-193078/ ; https://expcinema.org/site/en/tags/bruce-mcclure?page=1 |
| 9 | **Robert Henke, *CBM 8032 AV*** (2019→) | Five 1980 Commodores, 1 MHz, 32 KB; sound and image from the same CPU; green-mono text-mode graphics | "Minimalism by design": the hardware limit becomes the style. Our Iris Xe budget can be sold the same way (§5). | §1.4 |
| 10 | **Robert Seidel**, *_grau* (2004), *Vellum* (2009-10), *vitreous* (2015) | Abstract cinema from the Bauhaus-Weimar school: sediment-like layers, slit-scan, lyrical erosion; *_grau* processes the memory of a car accident | **Slit-scan** (time displaced across the frame) is a cheap GPU technique and a strong "memory" image for the dance show. Candidate scene or LOOK control. | [S] https://robertseidel.com/grau/ ; https://www.creativeapplications.net/project/tearing-shadows-robert-seidel-breathes-light-into-a-plastic-nebulae/ |
| 11 | **Alba G. Corral** (Catalunya; with Jon Hopkins, Arbol) | Organic line drawing via code, improvised live with musicians | Drawing as input: the performer's pen or mouse deposits seeds or lines into a scene (Levin's "substance" idea). Was an unverified lead in RESEARCH-REPORT; now verified. | [S] https://mutek.org/en/speakers/alba-g-corral |
| 12 | **Granular Synthesis** (Hentschläger + Langheinrich, 1991-2003) | The method, not the look: "granular synthesis" applied to **both** video and sound | A **visual granulator / looper** over the instrument's own recent output (§4 idea 4). | [S] https://v2.nl/organizations/granularsynthesis/ |
| 13 | **Brian Eno, *77 Million Paintings*** | Slow superimposition of four hand-made layers; never repeats | An ambient / installation mode: slow permutations of a curated pool (§4 idea 14). | §1.4 |
| 14 | **Konx-om-Pax** (Tom Scholefield; with Kode9, Hudson Mohawke, Lone; curated "Contemporary Hardcore", Atonal 2018) | The rave-graphic end of the Atonal world; less close to the owner's taste | Useful only as the contrast: saturated 3D graphic design vs our film-dark restraint. | [S] https://planet.mu/artists/konx-om-pax/ |

**What the new pool adds, in three moves [I]:**
1. **Darkness is the default; light is the event** (GRIEND, MFO, McClure, and already Hentschläger). This pushes REST further than REACTION-DESIGN's 22 % floor, as an optional extreme character.
2. **One technique applied to both media** (Granular Synthesis, Lis Rhodes, Henke's single CPU, Versum). This is the same conclusion as §1.4, reached from the art side.
3. **The device is part of the work** (McClure's projectors and pedals, Henke's Commodores, Cathodemer's simulated CRT). The film chain already treats film as the device. The next step is the projector (§4 idea 10).

---

# 4. Ranked feature / direction ideas

**Scale.** *Differentiation value* 1-5: how much it separates us from the §1 field *and* matters to a working performer. *Build cost*: **S** ≤ 2 dev-days, **M** 3-8 days, **L** 2+ weeks (one engineer or agent session, current code base). Ranking = value first, then cheapness. **Overlap** names the existing doc that touches the idea; only the new angle is described.

| Rank | Idea | What it is | Why it differentiates (evidence) | Value | Cost | Overlap |
|---|---|---|---|---|---|---|
| 1 | **Output guardian: the audience never sees your desktop** | A tiny, dumb fullscreen process owns the projector window and shows frames from the engine via Spout. If the engine crashes, hangs or restarts, the guardian holds the last good frame (or plays a slow grain-only idle loop from its own shader) until the engine is back, then crossfades in. Plus: all scene shaders warmed up at engine start, so no scene load can stall output. | Frozen / blue output is the #1 feared failure in the evidence (§2.3). Notch documents output pauses on load. No competitor advertises this. Our split-process design makes it natural. | 5 | M | Workflow R6 (relaunch banner) covers recovery, not what the audience sees meanwhile |
| 2 | **Timing honesty: measured latency + audience-distance sync** | (a) A **Sync Check**: the engine flashes a square on a click (from the plug-in), the owner films projector + speaker with a phone at 240 fps; one number (ms) is entered, or read automatically from the laptop mic + webcam later. (b) Grid-locked events fire early by that latency. (c) A **"sync for listeners at N m"** setting (default 10 m) adds distance/343. (d) The status pill shows "sync +12 ms @ 10 m". (e) A pre-show warning: "projector adds > 60 ms, turn off frame interpolation". | ITU numbers + physics (§2.1): the performer sees the worst sync. Nobody in the VJ field frames latency this way. Positions us as the *instrument-grade* tool. | 4 | S-M | RESEARCH-REPORT B4/B6 (lookahead, time tags); the plug-in's Lookahead param (live-audio risk) |
| 3 | **Coupled AV gestures ("the instrument plays both")** | The plug-in already sits in the audio path. Add an optional, off-by-default **sound partner** per macro/action, on the lead instance: FREEZE also freezes the audio (spectral/granular hold); ERODE adds tape wear / bitcrush / dust crackle; DROP opens a filter sweep; REACT 0 ducks transients. One knob moves picture and sound together. | Only hardware does this (Kaoss Entrancer Combi, OP-Z). The strongest AV works share one control signal (§1.4). Nothing inside a DAW does it. For a solo musician-visualist it removes a hand. | 5 | M-L | REACTION-DESIGN principle 7 ("same control signal"), but only in the MIDI → image direction |
| 4 | **Visual memory: a looper / granulator of our own output** | Keep the last 8-16 bars of the final image at quarter res, luma only (≈130 KB/frame; 16 s × 12 fps ≈ 25 MB). Gestures: REPLAY (loop the last bar), SLICE (stutter on the beat grid), REVERSE, SMEAR (granular time spread), and **each replay degrades** (generation loss). Palette and grain are applied after, so replays stay in the current look. | Kaoss Entrancer has 6-second video sampling; VJ tools loop clips, but no generative tool can loop *itself*. Fits the dance show's memory research directly ("Replay: compressed, in slices, and in reverse"; "Generation loss" in A-memory-science). | 4 | M | none (REACTION-DESIGN idea 13 "Afterimage" is a one-frame version) |
| 5 | **Perform at 720p, render the same performance at 4K** | Log every control message with timestamps during a set (small text file), bounce the audio from Live, then re-run the engine offline, frame by frame, deterministic (seeded randomness), at any resolution. | VJ Loop Studio 3D separates live vs offline analysis; nobody lets you *replay a live performance* at higher quality. Solves the Iris Xe recording problem (workflow table: recorders drop fps). Musicians need content for release and social. | 4 | M | Workflow gap-table "built-in recorder" (live capture only) |
| 6 | **Patch codes: share a look as a string** | Serialise scene + 8 macros + look vector + palette + character + seed into a short code (base64 JSON, ~150-300 chars) and a QR. Paste = load (cued, not live). Later: a community page like Patchstorage, then a marketplace. | Hydra shares whole sketches as a URL; EYESY has 209 community modes; Synesthesia sells scenes. Nobody shares *performance states* of a fixed instrument, like synth presets. Viral and cheap. | 4 | S (codes) → L (site) | Moments (CONTROL-MAP) store states locally only |
| 7 | **A constant physical vocabulary + controller profiles with LED feedback** | Promise: "knob 5 is REACT on every scene, forever". Ship profiles (the owner's Korg once known, MIDI Fighter Twister, Launch Control XL) inside the plug-in, independent of Live's per-set MIDI Map, sending LED / ring values back. | EYESY's knob 4 = reaction is why it can be played eyes-up; LED rings are the VJ norm on dark stages (§2.3). | 4 | S-M | Workflow M4 (in-plug-in MIDI learn); the new part is the *fixed-vocabulary* promise and feedback out |
| 8 | **Motion loops (record a gesture, it repeats)** | Hold REC on a macro, turn it for 1-4 bars; it loops quantised to the grid until cleared. Elektron-style, without the step feel: store the continuous curve. | Elektron "live recording" turns knob moves into per-step locks [S] (§ sources). No VJ tool offers this; it frees the musician's hands (Cook's "spare bandwidth"). | 4 | S-M | none |
| 9 | **Scene articulations (a visual drum rack)** | Each scene declares up to 8 named **gestures** (Fibers: tear, knot, bloom…; Morphogen: seed, flood…), played by pads or MIDI notes, velocity = size. A MIDI clip in Live then *is* a visual score. | VS has polyphonic voices, NestDrop/Spettro map notes to presets; nobody gives each generative scene its own playable vocabulary. This is the "no ceiling on virtuosity" half the product lacks (§2.2). | 4 | M | Workflow R8 (notes select cues); REACTION-DESIGN idea 12 (velocity accents) |
| 10 | **Projection look** (the device as part of the image) | A look layer that simulates the *projector*: 48 Hz shutter flicker, lamp hot-spot fall-off, focus breathing, a faint gate frame, dust in the lens. Pairs with the existing film chain. | Cathodemer sells a CRT simulator; McClure performs the projector. For a film-grain product this is the natural next layer, and cheap. | 3 | S | LOOK chain (film) |
| 11 | **GO in the dark** | A quantise option "on the next REST or DROP": the scene change hides in a dark moment, or becomes the drop. | Theatre changes in blackouts; darkness as the default in the new references (§3). Tiny on top of existing cue + GO. | 3 | S | REACTION-DESIGN idea 9 (GO on DROP) |
| 12 | **Role-separated Scope** | The Scope scene / overlay with one line per role: the kick track's waveform, the pad's, the bass's. | Scope itself is common (§1.1); per-role scope is impossible without per-track instances. Makes the architecture visible. | 3 | M | REACTION-DESIGN idea 11, workflow B3 (Mix Map) |
| 13 | **Foot control profile** | A 4-6 switch MIDI foot controller: GO / DROP / HIT / FREEZE, expression pedal = REACT or ERODE. Documented profile + defaults. | McClure plays projectors through guitar pedals; a musician's hands are busy. Nobody targets the playing musician's feet. | 3 | S | none |
| 14 | **OSCQuery on the engine** | Publish every parameter with ranges and names, so TouchOSC / Open Stage Control / TouchDesigner discover them. | MadMapper does it [D]; makes the phone remote (workflow M6) near-zero work. | 3 | S-M | Workflow M6 |
| 15 | **Set arc** | Optional slow drift across the whole set: the film "print" ages (grain, dust, scar floor rise) from the first minute to the last, locked by a toggle. | Endel adapts to time of day; nobody in VJ has a set-scale time layer. Our REACTION-DESIGN scales stop at the section. | 3 | S | REACTION-DESIGN scars (section scale) |
| 16 | **Drift mode (installation / long ambient)** | Slow permutations of 2 layers from a curated pool (Iris Xe: two half-res scenes at most), never repeating; hands-off for hours. | Eno's 77 Million Paintings; opens galleries and long ambient sets. | 3 | M-L | Workflow B2 co-pilot (suggests, human confirms) |
| 17 | **Offline licence, no login** | Machine-bound key that works without internet. | Arkestra users dislike the mandatory login (§1.5); stage machines are often offline. | 3 | S-M | Workflow B4 (productise) |
| 18 | **Light output (DMX strobe / wash from the same accents)** | One DMX universe via Art-Net: accents fire a strobe, REST dims a wash, DROP blooms. | MFO / Atonal: the picture is light architecture; Showsync sells Beam for lighting separately. | 3 | M | none |
| — | *Not recommended now:* MPE control | Live 10 has no MPE; it arrived in Live 11 [S] https://help.ableton.com/hc/en-us/articles/360019144999-MPE-in-Live-11-and-later-FAQ. Use aftertouch / mod wheel as a single "breath" source instead. | | 2 | M | |
| — | *Not recommended now:* screen or DAW window as a texture | Screen Sampler shows it can look good, but it is a novelty for this taste. | | 2 | S | |

**If only three things get built next:** 1 (guardian), 2 (timing honesty) and 7 (fixed vocabulary + LED feedback). Together they turn "a nice visualiser" into "a tool a pro trusts on stage", at S-M cost. **If one big bet:** 3 (coupled AV gestures), because it is the claim nobody in a DAW can make.

---

# 5. Think outside the box

These go past the ranked list. Each says what it is, why it is not obvious, and the smallest test that would tell us whether it is worth it. All are inferences [I] built on the evidence above.

### 5.1 From visualiser to AV instrument: the scene has a voice

The ranked idea 3 couples a few macros to audio effects. The larger version: **every scene ships with a sound partner**, a small DSP "timbre" in the plug-in that shares the scene's control signals. Fibers ↔ a granular shimmer that tears when the fibres tear; One Bit ↔ a gate / bitcrush that closes with the letterbox; Corridor ↔ a tape-delay feedback that lengthens with the travel; Membrane ↔ a resonator tuned by the folds. The partner is either an effect on the music (dry/wet, default 0) or a quiet layer on a return track.

- **Why not obvious:** every VJ product treats audio as input only. The only precedents are hardware (Kaoss Entrancer Combi) and custom art systems (Versum, *Lumière*, Henke's CBM).
- **What it changes:** a scene becomes a *patch* in the synth sense: image + sound + controls. That is a genuinely new product category inside a DAW: "an audiovisual instrument plug-in".
- **Smallest test:** one scene (One Bit) with one coupled effect (gate + bitcrush on ERODE and the letterbox), played by the owner on one of his tracks. If it makes him play differently, it is real.

### 5.2 The instrument remembers (and forgets)

Idea 4 (visual looper) framed as a concept: the instrument keeps a **memory** of what it has shown. The performer can recall it (replay, slice, reverse, smear), and every recall **degrades** it, as the dance show's memory research describes (reconsolidation, generation loss). The memory could also be seeded from media slots, so a photograph enters memory and wears away over a piece.

- **Why not obvious:** generative tools are memoryless by design; clip tools loop fixed media. A generative instrument that loops and ages *its own past* has no competitor, and it is the literal subject of *Before It Disappears*.
- **Smallest test:** 8 bars of luma history + REPLAY and REVERSE on two buttons, degrading by one grain/blur step per replay.

### 5.3 The set leaves a print

Across a whole set, the engine accumulates one image: a slit-scan column per second, or a long exposure of the output, through the film chain. At the end of the night the performer gets **one still that is the set**: unique, shareable, a poster or an album cover. Combined with 5.4, the print can carry the set's wear (the scars from ranked idea 15).

- **Why not obvious:** VJ tools produce no artefact; musicians need content. A souvenir from each performance is marketing that users make for us.
- **Smallest test:** a 1-px column every second into a 3600-px wide PNG over a one-hour set. A day of work.

### 5.4 Shareable patches and scores, not just presets

- **Patch** (ranked idea 6): a look you can post as text or a QR code under a video. "Made with X, patch in the comments" is how synth presets spread.
- **Score:** a MIDI clip of articulations + cue notes + macro CCs (ranked ideas 8-9) that plays a piece's visuals in any copy of the instrument. Copy the clip into another set and the visual choreography comes with it. Perry Cook's "make a piece" principle, turned into a format: the dance show becomes the first published score.
- **Community:** a Patchstorage-like page (EYESY has 209 modes with one open format) and later a paid pack store (Synesthesia's marketplace model).

### 5.5 Tune to the room, not to the laptop

Beyond ranked idea 2: the instrument knows the *room*. Enter screen size, listener distance and the projector's measured lag once per venue, and the engine adjusts sync, flash limits (a larger screen fills more of the visual field, which matters for photosensitivity) and the Projection look's brightness fall-off. Save it as a **venue profile**, like a front-of-house engineer's system preset.

- **Why not obvious:** VJ tools treat the output as a rectangle of pixels; audio people have treated the room as part of the instrument for decades.

### 5.6 Hear the picture

The reverse channel: the engine sends a scanline (or the luminance profile) of the current image back to the plug-in, which can output it as audio on its own track, Lis Rhodes' optical soundtrack as a live instrument. Or use it as modulation: the picture's brightness side-chains a filter in the music.

- **Why not obvious:** it closes the AV loop in both directions, a claim no VJ tool makes. It is an art-market and installation feature more than a club feature.
- **Smallest test:** a 512-sample row read back once per frame and played as a wavetable. Two days.

### 5.7 The constraint is the style

Henke's CBM 8032: "minimalism by design". Our Iris Xe budget forces half-res simulation, grain, dither and restraint, which *is* the film-dark look. Position the product as **designed for the laptop you already have**, and make the low-power path a look rather than a fallback (e.g. a deliberate "print" mode at quarter res with heavy grain and dither). Competitors ask for a GTX 1060 (Synesthesia) or 24 GB (AI tools).

### 5.8 Light, not only picture

MFO and the Atonal lineage treat visuals as light in a room. One Art-Net universe (ranked idea 18) driven by the same accents, REST and DROP turns the instrument into a small light show for venues without an LD, and puts the red-mono aesthetic into haze and strobes, where phs.wrk-style flashes belong.

### 5.9 Two small ideas worth keeping in mind

- **Knob thresholds as hidden modes** (Hypno): past 75 % a macro may cross into a qualitatively new state (feedback, inversion) instead of only "more". It adds depth without a new control; it needs a visible tick on the knob so it is never a surprise.
- **Drawing as input** (Alba G. Corral, Levin): an XY pad or pen on the NOW thumbnail deposits seeds, tears or light where you touch. Direct, non-diagrammatic control of the picture.

---

# 6. Sources

**Opened (full page read)**
- MadMapper live performance docs: https://docs.madmapper.com/madmapper/6/11.-live-performance-and-control
- Patchstorage, EYESY: https://patchstorage.com/platform/eyesy/
- Sound on Sound, Korg KPE1 review: https://www.soundonsound.com/reviews/korg-kpe1-kaoss-pad-entrancer
- Teenage Engineering VideoLab: https://teenage.engineering/products/op-z/videolab
- Notch manual, Working with audio: https://manual.notch.one/2026.1/en/docs/learning/working-with-audio/
- VJ Loop Studio audio-reactive guide: https://vjloopstudio.com/posts/audio-reactive-guide/
- VVavy 2026 visualiser ranking (competitor-authored): https://vvavy.io/blog/2026-best-music-visualizers/
- CDM, EboSuite 2.0: https://cdm.link/ebosuite-2-0-hands-on-live-video-recording-and-more-makes-vjing-feel-native-in-ableton-live/
- Lime Art Group mega list 2026: https://limeartgroup.com/the-mega-list-of-vj-software-and-tools/
- Lime Art Group MIDI controllers for VJs: https://limeartgroup.com/best-midi-controllers-of-vjs-visual-artists/
- Ableton blog, three artists and visuals: https://www.ableton.com/en/blog/extending-live-how-three-different-artists-approach-visuals-live-performance/
- Elektronauts, Arkestra thread: https://www.elektronauts.com/t/arkestra-audio-reactive-visuals-with-ableton-link-for-macos/216978
- Golan Levin thesis index: http://www.flong.com/archive/texts/publications/thesis/index.html
- IDMIL on Perry Cook's principles: https://www.idmil.org/publication/expert-commentary-perry-cooks-principles-still-going-strong/
- Wessel & Wright 2002 (text extracted from the PDF): https://arxiv.org/pdf/2010.01570
- TV Tech, AV sync thresholds: https://www.tvtechnology.com/opinions/av-synchronization-how-bad-is-bad
- ProjectorCentral, input lag: https://www.projectorcentral.com/projector-input-lag.htm
- Derivative, Perform Mode: https://docs.derivative.ca/Perform_Mode
- VDMX, More fun audio analysis techniques: https://vdmx.vidvox.net/tutorials/more-fun-audio-analysis-techniques
- Derivative, Pauric Freeman interview: https://derivative.ca/community-post/pauric-freeman-working-sound-image-input-devices-and-human-perception/65814
- Electronic Groove, Tarik Barri interview: https://electronicgroove.com/interview-tarik-barri/
- CLOT, 404.zero interview: https://clotmag.com/interviews/404-zero-the-architecture-of-nothingness-and-metaphysics-of-time-and-space-in-mind-bending-audiovisual-code-art
- Synthtopia, CoGe 1.7 (content thin): https://www.synthtopia.com/content/2016/04/29/coge-vj-semi-modular-video-software-gets-bulletproof-upgrade/

**Search snippets only [S] (leads; not opened)**
- Magic Music Visuals: https://magicmusicvisuals.com/ · VDMX plugins: https://docs.vidvox.net/vdmx/vdmx_plugins · MadMapper GLSL materials: https://docs.madmapper.com/madmapper/6/3.-media/writing-custom-glsl-materials · Notch in Resolume: https://manual.notch.one/2026.1/en/docs/workflows/working-with-media-servers/blocks/using-notch-in-resolume/ · Smode: https://www.smode.io/en/solutions/live-visual-music-events · CoGe status: https://imimot.com/blog/cogevj-development-update/ · Cathodemer: https://www.hypertonal.net/cathodemer/ · Lumen: https://www.synthtopia.com/content/2016/11/28/lumen-video-synthesizer-for-mac-gets-midi-control/ · EYESY: https://www.perfectcircuit.com/critter-guitari-eyesy.html · Hypno: https://sleepycircuits.com/hypno · OP-Z: https://teenage.engineering/products/op-z , https://cdm.link/2018/10/op-z-dmx-unity-3d/
- Resolume forum crash threads (403 on open): https://resolume.com/forum/viewtopic.php?t=10254 , https://resolume.com/forum/viewtopic.php?t=11091
- VJ mistakes: https://limeartgroup.com/vj-mistakes-beginners/ , https://www.stvinmotion.com/three-beginner-vj-mistakes/
- Versum: https://tarikbarri.nl/projects/versum · INFRATONAL: https://derivative.ca/community-post/exploring-humanized-algorithmic-art-infratonal/65499 · 77 Million Paintings: https://en.wikipedia.org/wiki/77_Million_Paintings · Endel: https://endel.io/technology , https://endel.zendesk.com/hc/en-us/articles/360012517639-How-Endel-Works · Henke CBM 8032: https://roberthenke.com/concerts/cbm8032av.html , https://clotmag.com/news/performance-robert-henkes-cbm-8032-av-premiere-at-unsound
- Jordà: https://www.researchgate.net/publication/228715881_Digital_Instruments_and_Players_Part_II-Diversity_Freedom_and_Control
- Artists: https://www.factmag.com/2020/03/13/404-zero-black-sunday-av-album/ · https://mfoptik.de/about/ · https://www.berlin-ism.com/en/news/ben-frost-mfo · https://clotmag.com/oped/atonal-2025-the-house-that-jack-built-five-days-of-noise-darkness-in-the-looming-corridors-of-kraftwerk-berlin · https://www.tate.org.uk/art/artworks/rhodes-light-music-t13857 · https://www.acmi.net.au/stories-and-ideas/seeing-sound-lis-rhodes-light-music/ · https://www.artforum.com/features/powers-of-projection-the-art-of-bruce-mcclure-193078/ · https://expcinema.org/site/en/tags/bruce-mcclure?page=1 · https://robertseidel.com/grau/ · https://www.creativeapplications.net/project/tearing-shadows-robert-seidel-breathes-light-into-a-plastic-nebulae/ · https://mutek.org/en/speakers/alba-g-corral · https://v2.nl/organizations/granularsynthesis/ · https://planet.mu/artists/konx-om-pax/
- Elektron live recording / parameter locks: https://www.manualslib.com/manual/1613256/Elektron-Digitone-Keys.html?page=18
- MPE arrived in Live 11: https://help.ableton.com/hc/en-us/articles/360019144999-MPE-in-Live-11-and-later-FAQ
- Kaiber Superstudio: https://www.producthunt.com/products/kaiber/reviews

**Could not reach:** reddit.com (blocked for this agent), Resolume forum threads (403), CNMAT copy of Wessel & Wright (404; the arXiv copy was used).

**Caveats**
- The latency chain in §2.1 is an estimate; measure before quoting it anywhere.
- Coupled AV gestures (idea 3, §5.1) change the plug-in from an analyser into an audio effect with a sound of its own. That is a product decision for the owner, not an engineering default.
- Nothing here was tested in Live.
