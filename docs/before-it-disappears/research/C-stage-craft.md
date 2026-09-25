# Strand C: Stage craft for projecting behind dancers

*"Before It Disappears": rear projection, light budget, safety and show control, with numbers.*

**Status of sources.** WebFetch was blocked for every domain tried (ofcom.org.uk, w3c.github.io, la.rosco.com), so every web claim below comes from search-result text only and is marked **[search]**. Nothing here was verified against the full page. Where a number matters for money or safety (screen gain, Ofcom thresholds), check the primary document before relying on it. Repo files I read are marked **[opened]**. My own reasoning is marked **Inference**.

**Already covered elsewhere (not repeated here):**
- `docs/RESEARCH-REPORT.md`, *Style bible*: the lifted-black floor of 0.02–0.06 and the "never pure #000 except Flash" rule. §2 below changes that rule for the stage.
- *Anti-patterns*: flash > 3/s.
- *Top 15* #10: a photosensitivity limiter with an optional luminance-delta clamp.
- B7: performance on integrated GPUs.
- USAGE.md [opened]: the Spout output, BLACKOUT, CALM, and the 3/s cap on Flash and kick-cuts.

---

## 1. Rear projection: materials, depth, lens

### Screen materials (published numbers)

| Material | Gain | Viewing cone / angle | Note | Source |
|---|---|---|---|---|
| Rosco **Grey** RP | **0.90** | **120° cone**, 170° viewing angle (85° off axis) | "double the viewing cone of the Black screen". Front or rear. PVC, ~1 lb/yd², flame-retardant, 55" seamless rolls | Rosco product page, AV-iQ listing [search] |
| Rosco **Black** RP | unknown | **60° cone** | best contrast, narrowest audience | Rosco guide [search] |
| Rosco **Twin White** | unknown | "almost 180°" | "equally bright… front or rear". Rosco calls it "the color of choice for theatre cyclorama" | Rosco Twin White page [search] |
| Gerriets **OPERA creamy-white 2.2** | **0.32** | very even across the whole angle | front and rear | Gerriets US [search] |
| Gerriets **OPERA grey-blue 2.2** | **0.26** | very even | slightly cooler white point | Gerriets US [search] |
| Gerriets **TRANSMISSION** RP | **1.13** | even | the high-luminance RP option | Gerriets US [search] |

**Caveat.** Manufacturers do not measure gain the same way. Gerriets notes that half-gain angle is "rarely specified" [search]. Compare gains within one brand only, or measure with a luminance meter on site.

### Hot spot and viewing cone

- On a rear screen, "a bright spot ('hot spot') is often visible at the center… because the viewer is usually looking almost directly at the source through the screen" (Broadway Educators, Part III [search]).
- Image quality "falls off sharply" outside a 60–110° viewing cone, and darker, denser surfaces fall off closer to the centre axis (Broadway Educators / Rosco [search]).
- **Trade-off:** a grey or black screen gives better blacks but narrows the cone. White or Twin White gives a wide cone but lifts the blacks.

### What rear projection solves

- No dancer shadows on the image, and no projector light on the dancers. The only beam is backstage.
- Ambient stage light that passes through the screen "is transmitted… into the (matte black) rear projection room and gets mostly absorbed". Contrast is therefore less affected by ambient light than with a front screen (av-info.eu [search]).
- Nobody walks through the beam in front of the screen (Stage Depot FAQ [search]).

### What rear projection costs

1. **Depth.**
   - Required depth ≈ throw ratio × image width + projector body + about 0.5–1 m for access. **Inference**, from the throw-ratio definition.
   - Worked example for an 8 m wide image:
     - standard lens (1.5–2.0) → 12–16 m (impossible in most black boxes)
     - short lens (≈0.8) → ~6.4 m
     - ultra-short-throw lens → ~3 m
   - Real UST lenses:
     - Epson **ELPLX01**: 0.35, zero offset, "ideal for… rear-projection". Fits Epson Pro L/G projectors up to 8,500 lm [search].
     - Panasonic **ET-D75LE90/95**: 0.36 (WUXGA), about **1.5 m for a 200" image**, for Panasonic 3-chip DLP projectors [search].
   - A **mirror** folds the path, but it has to be a large front-surface mirror, and those are hard to keep clean in a theatre (ControlBooth [search]).
2. **The zone behind the screen is dead space.** Anyone or anything crossing the beam backstage throws a shadow onto the screen. **Inference.** Plan dancer crossovers around the beam.
3. **The stage still hits the screen from the front.** Any front light, or light bounced off pale costumes, lifts the screen's black exactly as it would on a front screen. That is why RP screens are grey (see §2).
4. **Mirroring.** Set the projector to **"Rear"** projection mode (horizontal flip), or "Rear/Ceiling" if it is rigged upside down (Epson projection modes [search]). Do not flip in the engine: the operator monitor would read backwards.
5. **Lens shift, not keystone.**
   - Lens shift is lossless.
   - Digital keystone "reduces resolution" and "lowers image brightness", because only part of the chip is used (ProjectorCentral / Projector Reviews / Epson guide [search]).
   - Put the projector on the screen's centre axis where possible, use lens shift to finish, and use keystone only for the last few pixels.
6. **The hot spot at eye level.** **Inference:** when the lens sits on or near the audience's eye line through the screen, the hot spot is at its worst. Rig the projector above or below the sightline and correct with lens shift, so the axis through the screen does not point into the seats. Test from the front row, the centre and an extreme side seat.

**Two rules for our taste (mostly black, light at the edges). Inference:**
- A dark centre hides the RP hot spot, because a hot spot only shows where there is image.
- The frame edges are off-axis for most seats, so they will read *dimmer* than on the operator monitor. Budget edge light about 20–30 % hotter than it looks right on the monitor, and confirm by eye from the side seats.

---

## 2. Light budget: lumens, black level, contrast

### Formulas (search-verified)

- Luminance: **L (cd/m², nits) = Φ × G / (π × A)**, with Φ in lumens reaching the screen, G the screen gain and A the area in m² (ProjectorCentral/Valerion/Calculator Academy [search]).
- Solved for lumens: **Φ = L × π × A / G**.
- 1 fL = 3.426 cd/m² [search].
- Cinema reference: SMPTE 196M, **16 fL open gate ≈ 55 nits**, **14 fL ≈ 48 nits** with film. Home cinema runs 12–30 fL (Acoustic Frontiers / Elite Screens [search]).

**Worked example.** Assume an 8 × 4.5 m screen (36 m²). This is an assumption; the real size is unknown.

| Target peak (full white) | Gain 0.9 (Rosco Grey) | Gain 0.3 (Gerriets Opera) |
|---|---|---|
| 30 nits | 3,770 lm | 11,300 lm |
| 48 nits (cinema) | **6,030 lm** | **18,100 lm** |
| 80 nits | 10,050 lm | 30,200 lm |

In the other direction:
- 7,000 lm on gain 0.9 → 56 nits
- 10,000 lm on gain 0.9 → 80 nits
- 10,000 lm on gain 0.3 → 27 nits

**Derate spec-sheet lumens by roughly 20–40 %** for colour calibration, eco mode, zoom and ageing. **Inference**, general practice; the exact figure is unknown per model.

### Lumen classes

- Broadway Educators [search]:
  - a 3,000–4,000 lm classroom projector "can work just great on stage"
  - 6,000–10,000 lm+ makes it "definitely… easier"
- A university dance theatre's inventory lists a **Sony VPL-FHZ75 laser, 8,000 lm** (BU Dance Theater inventory [search]).
- Projectors of 20,000 lm and more exist for large stages (rebeam / ProjectorCentral [search]).
- **Inference:** a mid-size dance stage with a 6–10 m RP screen wants a **WUXGA laser in the 7k–12k lm class**. Only a large proscenium needs 15–20k+.
- For our deliberately dark aesthetic, a bright projector is *headroom*, not brightness we will use. It buys the option of a white-hot edge in the finale.

### "Black" is never black

- **Projector black.**
  - Contrast is full-on/off (optimistic) or ANSI checkerboard, typically "hundreds:1".
  - Light scattered inside the lens and light engine lifts the blacks next to the whites (ProjectorCentral / Christie whitepaper [search]).
  - "When digital projectors fade out, they still project light, called 'video black'… easily visible during a stage blackout" (City Theatrical Projector Dowser [search]).
- **Ambient black. Inference, worked numbers.**
  - Suppose stage spill leaves 10 lux on the screen's audience side and the screen face reflects about 20 %. Then L = E·ρ/π ≈ **0.64 nits**.
  - A 10k lm projector with 2,000:1 on/off contrast has a video black of only about 0.04 nits.
  - So in a lit scene **the room, not the projector, sets the black**: roughly 16× more here. In a stage blackout it is the other way round: the projector's video black is the only light left.
- **Consequences for the engine. Inference:**
  1. On stage, set LOOK → **Blacks to 0**. The room will lift the black for us; the Style-bible floor of 0.02–0.06 is meant for dark rooms and monitors.
  2. For true blackouts, ask for a **DMX dowser / shutter**:
     - City Theatrical 4160: a flag that travels 90° in under 1 s
     - Wahlberg DMX Projector Shutter
     - Blacky [search]
     
     Alternatively, ask for the projector's own mechanical shutter under control. Engine BLACKOUT alone leaves a glowing grey rectangle.
  3. Mask the output to the exact screen outline. A black-but-lit rectangle that overshoots or undershoots the screen frame reads as a TV.

---

## 3. The dancers: legibility and luminance budget

### Published practice

- Keep stage light off the screen. Theatre Ave suggests focusing lamps "about 5 feet off the screen" and using barn doors [search].
- Pale costumes near the screen wash it out with bounce. With front projection you need either distance or "mostly side light" (Theatre Ave [search]).
- End the acting area **6–8 ft (1.8–2.4 m) short of the cyc** in a no-actor zone. A low ground row lifts the image off the floor (Broadway Educators, *Lighting the Stage When Using Projections* [search]).
- Back-light or side-light performers to reduce shadows on the screen (Stage Depot [search]).
- Lighting and projection designers should "work out well ahead of time what the relationship between their two media will be". Sometimes the lighting "going dimmer is the best choice", handing energy to the projection (Broadway Educators, Part I [search]).
- Typical stage illuminance: **300–750 lux on actors** for theatre, 200–800 lux on acting areas for live-only audiences. The older rule is 50–100 fc (≈540–1,080 lux), with backgrounds kept lower (SHEHDS / Fine Design Associates [search]; secondary sources).

### A maximum APL for dance

No published standard was found. **Unknown.** This is our proposed budget. **Inference**, derived as follows:
- A dancer lit at 500 lux with skin or costume reflectance of about 0.4 reads at L ≈ 500×0.4/π ≈ **64 nits**.
- For the bodies to read as figure, the screen area directly behind them should sit at **≤ 1/4 to 1/10** of that, so **≤ 6–16 nits**.
- With the 80-nit peak of the worked example, that means:

| State | Mean picture level (linear, of peak) | Where the light sits |
|---|---|---|
| Dancers moving, full focus on bodies | **≤ 8 %** (≈ 6 nits) | outer 15–20 % of frame width, top third. Centre-lower 50 % near black |
| Duet, solo, text-carrying image | ≤ 12 % | edges plus one soft field above head height |
| Transition, no dancer on stage | ≤ 25 % | anywhere |
| **Silhouette moment** (deliberate) | 30–60 % behind the bodies | dancers lit at ≤ 50 lux side light only, or none. The screen becomes the light source |
| Finale peak (a few seconds) | ≤ 40 % | a wide low horizon band, never full-field white |

**Measure it.** Record APL as the engine's mean linear output per section. Verify with a phone lux or luminance app at the screen in tech. **Inference.**

---

## 4. Haze with rear projection

- Haze acts as "a kind of constant virtual gauze", a "looking-through-a-veil effect… like blurry memory scenes in movies". Density grows with distance, so rear seats see it softer (American Theatre, *A Hazy Shade of Theatre* [search]).
- Projectors fire through haze to good effect, but text becomes unreadable and contrast drops (ControlBooth, *Projecting through haze* [search]).
- **Rear-projection specifics. Inference:**
  1. The beam is visible only backstage, so the audience never sees "memory as a cloud" from projector beams. If the choreographer wants beams in the air, that needs a separate front or side source, such as a small projector or a light.
  2. Audience-side haze scatters the *stage light* into a veil over the screen and raises its black. Expect 2–5× lifted blacks in heavy haze; the figure is **unverified** and has to be measured.
  3. Treat haze as a scene-level decision in cues, not a constant. It is part of the dramaturgy of forgetting, and it costs black.
- **Rules.**
  - Haze can trip optical smoke detectors. Venues isolate detectors and often require a fire watch (4Wall, Entertaining Safety [search]).
  - Actors' Equity and university protocols require warnings "at the front of the house, at entrance doors… and in the program" [search].
  - Glycol can dry throats; mineral oil can irritate eyes and contact lenses [search]. Ask the dancers.

---

## 5. Output chain on Windows

### Tools that receive Spout and can mask or warp

| Tool | Windows? | Mask/warp | Note |
|---|---|---|---|
| **Resolume Arena** | yes | polygon slices and slice masks are **Arena-only**; corner/point warping; edge blend | Spout input "always enabled", shows under Sources (Resolume support [search]). Also plays backup video files |
| **MadMapper 6** | yes (Win 10+, macOS 11+) | mesh warp, Bézier masks, soft-edge | from **€479 perpetual / €44 per month**; free version watermarks the output (madmapper.com FAQ [search]) |
| **TouchDesigner** | yes | Kantan Mapper, Stoner (corner-pin + mesh) | Non-Commercial is capped at **1280×1280** per TOP (Derivative [search]). A paid show likely needs a commercial licence (**verify terms**) |
| HoloMapper | Windows/Linux | mesh warp, Spout/NDI | newer, unverified [search] |
| **Millumin** | **Mac only** | — | [search] (limeartgroup) |
| **QLab** | **Mac only** | — | ETC community / Wikipedia [search]. Windows cue players: StageCUE, MultiPlay (free) [search] |

**Recommendation. Inference.**
- A flat RP screen with lens shift needs **no warping**, only a **mask** and possibly a 4-corner pin of a few pixels.
- A second GPU application on the Iris Xe costs frames. The engine already holds only about 42 fps at 76 % render scale on Hot Blobs (USAGE [opened]).
- So: **build a mask/corner-pin into the engine's final pass** (small cost: 1 pass, no buffers), and keep **Resolume Arena** as the fallback.
- Resolume doubles as the backup player (§7).

### Resolution and aspect

- Output exactly the projector's **native** resolution. Install lasers are typically 1920×1200 (16:10) or 1920×1080. Scaling in the projector costs sharpness and latency. **Inference.**
- The Iris Xe HDMI 2.0 port can reach 4K60 in principle, but users report failing to hold it (Intel community [search]). 1080p/1200p60 is the safe target.
- On an 8 m screen, 1920 px gives 4.2 mm pixels, which is fine at ≥ 5 m viewing distance.
- If the screen is wider than 16:9 (a cyc is often 2:1 to 3:1), render 16:9 and mask to a letterbox, or zoom so the width fills the screen and mask top and bottom. Either way the masking lives in the engine. **Inference.**

### Latency

- Spout: "max 1 frame", GPU-only, no encode (Daydream blog / Resolume [search]).
- Projector input lag depends on the model. DLP generally needs less processing. At 60 Hz one frame is 16.7 ms. In IMAG work 4–5 frames is typical, and ½ s is "a deal breaker" (ProjectorCentral, Blackmagic forum [search]).
- Our chain:
  - Live audio buffer
  - OSC on loopback (≈0)
  - 1–2 engine frames at 42–60 fps (17–48 ms)
  - +1 frame if routed through Resolume
  - projector (unknown, likely 16–50 ms)
  
  That totals **~50–120 ms. Inference.**
- This is invisible for cue-driven dramaturgical change. It matters only for hit-synchronous accents: pre-shift those automation points 1–3 frames earlier in the Arrangement.
- **Measure it** with a clap-and-flash test filmed at 240 fps. **Inference.**

---

## 6. Working with the lighting designer

- **Colour temperature.**
  - Projectors are calibrated to about **6500 K (D65)**. Tungsten stage light is about **3200 K** (Jaspertronics / Valerion [search]).
  - On stage, a D65 "white" next to tungsten-lit skin reads blue-cold, and the dancers read orange. LED fixtures can go either way.
  - Options, most practical first. **Inference:**
    1. Set the projector's white point lower (5,400 K or a user setting), where the menu allows.
    2. Bias the engine palette: the "light" colour moves warmer, for example #FFE6C8 instead of #FFFFFF.
    3. Ask the LD for a lightly corrected (¼ CTB) wash where screen and body meet. LEE's colour-temperature calculator gives the gel conversions [search].
- **The spill colour becomes our black.** **Inference:** on an RP screen the lifted black takes the colour of the stage spill. Measure it in tech and set the engine's shadow palette colour to match. The lifted black then looks intended rather than dirty.
- **Who owns the cyc.** Settle in writing:
  - Is the screen a projection surface only? Or will the LD also light it from the front (cyc floods) or from behind (bounce)?
  - With RP, any front cyc light fights the image.
  - Agree section by section: "screen = video" or "screen = light". Never both at full strength.
- **Focus and tech-week practice.**
  - Paper tech with SM/LD/choreographer before tech, then cue-to-cue, then full runs (SMNetwork / Wikipedia *Technical rehearsal* [search]).
  - Content changes continue during tech (AACT, *Projection Designer* [search]).
  - Ask for a projector **focus call** in darkness, *before* lighting focus, and a shared "projection check" state in the LX desk: all stage light off except worklight-off states.

---

## 7. Show control, cue discipline, redundancy

### Ableton as the timecode spine

- **Live cannot send MTC natively** (Ableton SMPTE FAQ / forum [search]).
- Routes, in order of preference:
  1. Put an **LTC audio track** in the Arrangement (a generated LTC .wav) on a spare interface output, cabled to the LX desk.
     - ETC Eos accepts SMPTE and MTC, 32 sources each, through Event Lists (ETC docs [search]).
     - This makes lighting chase our timeline, which is the right answer for a one-operator show.
  2. Leolabs **LTC-to-MTC Converter** (Max for Live) [search]. It needs M4L, so **check that it runs on Live 10**.
  3. Plain **MIDI notes** from a Live MIDI track to Eos. Eos has no MSC from Ableton, but notes work (ETC community [search]).
- The engine itself needs no timecode, because Live's automation drives it over OSC (USAGE [opened]).

### Cue discipline (practice; inference from theatre norms)

- One **locator per cue**, named with section number, bar and meaning (e.g. `S3.2 – weather lifts`).
- Every section begins with an automation **reset**: preset, macros, LOOK, palette, REACT TO, CALM state, Blacks = 0. A mid-section start or a restart after a crash then lands in a known state.
- The live layer on the Korg adds on top of automation and never replaces it. Live's "Re-Enable Automation" must be pressed (or automated back) after each manual override. **Verify on Live 10.**
- Map exactly 3 panic controls on the Korg: **BLACKOUT**, **CALM**, **Reactivity to 0**. All three are Live parameters (USAGE [opened]).

### Redundancy

- Media-server practice is **main + backup** servers with a switcher and synchronised content. Checklist: matching software versions, the same project on both, output config verified (Doremi Event / PLSN *Redundancy* [search]).
- **Our scaled version. Inference:**
  1. **Pre-render every section.** Capture a clean run of each section from the engine output at native resolution, plus a 10-minute **"hold" loop** per section (calm state, no hits). The hold loops matter more than frame-accurate copies: our images are slow, so a hold loop is dramaturgically safe.
  2. **Backup player.** A second laptop, or Resolume on a spare machine, with the files in cue order.
  3. **HDMI switcher** between laptop and projector, e.g. Blackmagic **ATEM Mini** with 4 HDMI inputs and an HDMI out to the projector [search].
     - Input 1 = main, input 2 = backup, input 3 = black.
     - **Inference:** the switcher also gives the laptop a constant display handshake. Windows will not rearrange the monitors if the projector is power-cycled.
     - It adds some latency. **Unknown**, measure it.
  4. **On a crash**, in this order:
     - cut the switcher to backup (or black, or close the dowser)
     - the music keeps running, because the engine is a separate process from Live
     - relaunch the engine (`Start VJ Session.bat` or the exe), press `]`/`F` to put it fullscreen on the projector
     - jump the Live automation state (next locator, or re-trigger the section reset)
     - cut back at a section boundary
     - **Verify in code** that a relaunched engine receives the full state (macros, LOOK, palette) from the analyzer without a parameter change.
  5. **Audio is the bigger single point of failure.** If the laptop dies, the music dies too. A stereo bounce of the full show on a phone or backup player is the minimum (outside this strand).

---

## 8. Safety: photosensitivity

### The standards (search text only; primary documents were blocked)

- **WCAG 2.3.1.**
  - Nothing may flash "**more than three times in any one second** period" unless below the general and red flash thresholds.
  - General flash: opposing changes of **≥ 10 % of max relative luminance** where the darker image is below 0.80.
  - Red flash: "any pair of opposing transitions involving a saturated red".
  - Area exemption: ≤ 0.006 sr, about **25 % of any 10° visual field** (W3C Understanding / NYU [search]).
- **Ofcom/ITC guidance, adopted by ITU-R BT.1702 (2005).**
  - A harmful flash is an opposing change of **≥ 20 cd/m²** when the darker image is **below 160 cd/m²**.
  - A sequence is not permitted at more than **3 flashes/s** when the flashing area exceeds **¼ of the screen**.
  - "Isolated single, double, or triple flashes are acceptable".
  - "**Irrespective of luminance, a transition to or from a saturated red is also potentially harmful**" (Ofcom gn_flash, ITU BT.1702 [search]).
  - Sequences over **5 s** may be problematic even under threshold (PMC gap analysis [search]).
- **Patterns.**
  - Most provocative: high-contrast bars at **1–4 bars per degree**, especially oscillating or reversing at 15–20 Hz.
  - Consensus limits: **≤ 5 pairs of stripes if moving, ≤ 8 pairs if static**.
  - Red–green bars are "non-provocative". Central vision is more provocative than peripheral (PMC gap analysis [search]).
- **Live strobes (HSE, via Epilepsy Society):** "a maximum rate of **four** Hertz" in clubs and public performances. The 3–30 Hz band triggers about 60 % of photosensitive people [search].
- **Large screens.** Screens "much larger in their vision than the guidelines had anticipated" raise the risk. Yet seizures in cinemas are "rare" because of the "low intensity of the projection" (Epilepsy Society factsheet [search]).
- **Testing tools.**
  - Broadcast uses the **Harding FPA**, which is commercial (Wikipedia, hardingfpa.com [search]).
  - **PEAT** is free, but its licence prohibits film, broadcast and gaming use (Trace Center [search]).
  - EA **IRIS** is open-source, BSD-3, implements WCAG and ISO 9241-391, and has a Python port (github.com/electronicarts/IRIS, pypi iris-pse-detection [search]).

### Is our 3/s cap enough?

**No, not on its own. Inference, with evidence from the code [opened]:**

1. **The area exemption never applies on a stage.** An 8 m screen subtends **77° from 5 m, 44° from 10 m and 30° from 15 m**, while a 10° field at 10 m is only 1.75 m wide. Any full-frame flash covers many 10° fields. Frequency is the only safeguard we have.
2. **Our flashes count as flashes.** The screen peak is 27–80 nits, but a black → colour frame easily exceeds the 20 cd/m² Ofcom delta. Our frames also always sit under the 160 cd/m² floor, so the absolute threshold always applies.
3. **The cap is exactly at the boundary.** `LookPass.cpp:455` enforces a 0.34 s gap, so at most **2.94 flashes/s**. WCAG allows "no more than three"; Ofcom allows only isolated triples, not *sequences*. A 128-BPM kick drives a continuous sequence at about 2.1/s. That passes the 3/s count, but it is exactly the kind of sustained **5 s+ sequence** the guidance flags.
4. **Flash and kick-cut have separate timers.** Flash uses `lastFlashTime`; kick-cut uses `lastKickCut` (`PresetManager.cpp:376`). Combined, they can produce **~6 luminance transitions per second**.
5. **Red.** Flash can drop a colour frame, and the palettes include *Blood* and *Split* (black/red/cyan). **Saturated red transitions are flagged at any luminance.**
6. **In-shader flicker is outside the cap.** For example, `corridor.fs:83` steps lights at `floor(vj_time * 20.0)`, a 20 Hz blink driven by hats. It sits in the most provocative band.
7. **Patterns.** Symbols (braille strips), Glitch row-tears and stripe-like scenes can build gratings. On an 8 m / 1920 px screen:
   - 1° is 8.7 cm at 5 m (about 21 px) and 35 cm at 20 m (about 84 px).
   - So **high-contrast stripes 5–85 px wide** fall in the 1–4 bars/degree band for some seat.

**Proposed rules for this show:**
- a single global limiter over *all* luminance transitions (flash + cut + scene flicker): **≤ 2 per second and ≤ 6 in any 5 s**
- no saturated-red full-frame transitions: red enters by fade of ≥ 250 ms
- no oscillating stripe patterns with more than 5 pairs
- the 20 Hz shader flicker disabled in the dance show-file
- run **IRIS** on a recorded capture of every section before the premiere
- **print a warning** on tickets, programme and front-of-house notice if any flash remains

UK venues (Traverse, the Royal Opera House's Linbury Theatre) routinely warn for "flashing lights" and haze. Arcola cites official guidance to print warnings on tickets and premises [search]. **Israeli practice: unknown**; nothing was found. Ask the venue.

---

## 9. Rehearsals with live, generative visuals

- **Theatre norm:** concept meetings → paper tech → cue-to-cue (dancers "give cue lines or movements only") → runs. Content may still be made in tech (SMNetwork, AACT, Brodie *Process & Craft* [search]).
- **For a generative engine. Inference, practice recommendations:**
  1. **Studio mock-up from week 1.** A 3–5k lm projector, or a 55"+ TV turned to portrait/landscape at the screen's aspect ratio, placed upstage of the dancers at the right height, so the choreographer sees bodies against image.
  2. **Record every run.** One fixed wide camera plus a simultaneous Spout capture of the engine. Review the two side by side and note cues as `bar:beat`, not "when she falls".
  3. **Freeze the spine early.** Section order and locators first. Looks change later without moving cues.
  4. **One variable per session:** a luminance session, a timing session, a "no video" control run. The choreographer should see the piece without video at least once to judge what the screen adds.
  5. **In the venue, demand:** a dark projection focus call; at least one cue-to-cue with the LD; two full dress runs with haze if haze is used; and 30 min of dark time for luminance metering from three seats.

---

## What this means for our show

1. **Choose rear projection onto a grey RP screen** (Rosco Grey, gain 0.9, 120° cone) unless the venue's seating is very wide. Then consider Twin White and accept lifted blacks.
2. **Ask for backstage depth now.** For an 8 m image we need ~3 m with a UST lens (0.35–0.36), ~6.5 m with a 0.8 lens, or 12 m+ with a standard lens. This single number may decide front vs rear.
3. **Projector target:** a WUXGA laser of **≥ 7,000 lm (10k preferred)** with lens shift, a Rear mode and a mechanical shutter. Budget it with Φ = L·π·A/G.
4. **Peak luminance:** 50–80 nits of available headroom. Show-average luminance ≤ 8 % of peak while dancers move.
5. **Set Blacks = 0 on stage.** The room lifts the black; the Style-bible floor is for monitors.
6. **Match the shadow palette colour to the measured stage spill** so the lifted black looks designed.
7. **Our light-at-the-edges taste fits dance:** it keeps the zone behind the bodies dark and hides the RP hot spot. Give the edges +20–30 % over the monitor look, for the off-axis seats.
8. **Silhouette moments are a separate, declared state:** screen 30–60 % behind the dancers, LD at ≤ 50 lux side light. Use them rarely (1–3 in the show).
9. **Add a mask/corner-pin to the engine's final pass** (S cost, 1 pass). Keep Resolume Arena as the fallback and as the backup player. Millumin and QLab are Mac-only.
10. **Output native resolution at 60 Hz** from HDMI 2.0. Do not attempt 4K on the Iris Xe.
11. **A DMX dowser or the projector shutter for every true blackout.** Engine BLACKOUT is not black.
12. **Unify the flash limiter:** one counter for flash + cut + in-shader flicker, ≤ 2/s and ≤ 6 per 5 s. Ban saturated-red flash frames and remove the 20 Hz corridor flicker from the dance show-file.
13. **Run IRIS on recorded captures of each section.** Print a flashing-light warning if any flash remains.
14. **Put an LTC track in the Arrangement** so the LX desk chases our timeline. One operator cannot also call lighting cues.
15. **Every section starts with a full automation reset** (preset, macros, LOOK, palette, CALM). Every section has a pre-rendered **hold loop**.
16. **ATEM-class HDMI switcher:** main / backup / black on three buttons. It also stabilises the display handshake.
17. **Three panic controls on the Korg:** BLACKOUT, CALM, Reactivity → 0. Nothing else is "live-critical".
18. **Haze is a per-scene choice** in the cue list, not a constant. With RP, the audience will not see projector beams in it.
19. **From week 1, mock up in the studio** with a small projector or TV behind the dancers, and record every run with a synced engine capture.
20. **Colour:** move the engine's "light" colour warmer, or set the projector to about 5,400 K, to sit with tungsten or warm LED skin light.

## Draft pre-show checklist (T = doors)

**T−120 min**
- Laptop on mains power, Windows power mode "Best performance", sleep Never.
- Windows Update paused, notifications off (Focus Assist), Wi-Fi off unless needed.
- Reboot.
- Launch `Start VJ Session.bat`. The engine is on the projector output (`]`, `F`) and the analyzer pill shows connected, engine fps ≥ 50 and render scale as expected.
- Check `VJEngine.log` for shader errors.

**T−90**
- Projector warmed up ≥ 30 min, Rear mode on, lens shift and focus checked on the grid pattern.
- Mask aligned to the screen edges.
- Dowser/shutter opens and closes from DMX.
- Switcher: inputs 1, 2 and 3 tested and cut to 1.

**T−60**
- Backup laptop or player on, section files and hold loops cued.
- Audio backup (stereo bounce) loaded.
- LTC reaching the LX desk: the desk clock moves when Live plays.

**T−45: run the "top of show" check**
- Play the first 30 s of each section from its locator. Every reset lands correctly.
- BLACKOUT, CALM and Reactivity respond from the Korg.
- The flash limiter setting is confirmed.

**T−30**
- Luminance spot-check with house lights at show level.
- Haze state agreed with the LX op / SM.
- Stage clear of the backstage beam path.

**T−15**
- Live at the top locator, automation re-enabled (no orange override buttons).
- Engine in BLACKOUT, dowser closed, switcher on input 1.

**T−5**
- Phone on flight mode. Water, torch, printed cue sheet with the crash procedure.

**Post-show**
- Save the Live set under a dated name, copy `VJEngine.log`, note any issue per cue.

## Draft tech rider (video)

- Rear-projection screen: grey RP (Rosco Grey or equivalent), seamless or welded, at the stated size. Tensioned frame or bottom pipe, **no creases**. Black masking legs and border.
- Backstage clear depth: **≥ X m** (per the lens, see §1), blacked out, with no work lights in or near the beam.
- Projector: laser, **≥ 7,000 ANSI lm** (10k preferred), native 1920×1200 or 1920×1080, lens shift, Rear mode, suitable lens (UST ≤ 0.4 if depth < 6 m).
- Projector blackout: mechanical shutter under DMX, or a DMX dowser (City Theatrical 4160 / Wahlberg / Blacky), with **one DMX channel** on the LX desk.
- Signal: HDMI from FOH/wing to the projector. Over 15 m, HDBaseT or fibre extenders. Two runs (main + spare).
- Operator position with a sightline to the screen and the stage. Table ≥ 1.2 m, 2 × mains, a small monitor or stage feed.
- Audio: interface outputs 1–2 to PA, **output 3 = LTC to the LX desk** (balanced line, 1 × XLR).
- Time: a dark projection focus call (≥ 1 h) before LX focus; a cue-to-cue with the LD; 2 dress runs; 30 min dark time for metering.
- Haze policy and smoke-detector isolation procedure confirmed in writing, if haze is used.
- Front-of-house warnings for flashing light and haze, as needed.

## Questions for the lighting designer

1. Does the lighting also light the screen/cyc from the front or behind? Where is it "video only"?
2. Planned illuminance on the dancers (lux) and main angles: how much side light versus front? Can the downstage-of-screen zone stay 1.8–2.4 m clear of light?
3. Fixture types and colour temperature (tungsten ~3200 K, or LED with adjustable CCT)? Which white should the projection match?
4. Will you chase LTC from Ableton, or should cues be triggered by MIDI notes? Which desk (Eos family or another)?
5. Can the projector shutter/dowser be patched on your desk? Who fires it?
6. Haze: which states, what density, which fluid?
7. What does the stage look like at "video only": what residual light remains on the screen?
8. Silhouette moments: will you give us dark bodies against a bright screen (≤ 50 lux side light) at 1–3 agreed points?

## Questions for the venue

1. Stage dimensions: width, depth, height to grid, and **clear depth behind the screen position**. Is there an upstage crossover?
2. Is there an in-house RP screen or cyc? Material, size and seams? Can we bring our own?
3. In-house projector: model, lumens, lens(es), shutter? Rigging points for the projector behind the screen?
4. Seating layout: extreme side angles and front-row distance (this sets the viewing cone and the stripe limits)?
5. Cable path and distance from the operator position to the projector. Does HDBaseT or fibre exist?
6. Haze policy, detector isolation and fire-watch requirements?
7. What are the venue's rules or practice for flashing-light warnings (Israel), and where are they printed?
8. Access schedule: focus call in darkness, cue-to-cue, dress runs; available dark time; get-in and get-out hours?
9. House light levels during the show: exit signs, aisle lights. These set our floor black.

---

## Sources

**Repo [opened]:**
- `/home/user/VJ-VST/USAGE.md`
- `/home/user/VJ-VST/docs/before-it-disappears/ART-DIRECTOR-BRIEF.md` (§2, 5C, 6, 8)
- `/home/user/VJ-VST/docs/RESEARCH-REPORT.md` (headings, Style bible, Anti-patterns, Top 15)
- `/home/user/VJ-VST/engine/Source/LookPass.cpp` (lines 434–490)
- `engine/Source/PresetManager.cpp:376` (grep)
- `engine/Shaders/Instrument/corridor.fs:83` (grep)

**Web (all [search]; WebFetch blocked on ofcom.org.uk, w3c.github.io, la.rosco.com):**

Screens and rear projection:
- https://us.rosco.com/en/product/grey-rosco-rp-screen
- https://www.av-iq.com/avcat/ctl1642/index.cfm?manufacturer=rosco-laboratories&product=grey-rosco-rp-screen
- https://la.rosco.com/sites/default/files/content/resource/2018-07/Rosco_Guide_to_Projection_Screens.pdf
- https://us.rosco.com/en/node/492
- https://us.rosco.com/en/product/twin-white-rosco-screen
- https://www.gerriets.us/us/operar-grey-blue-front-and-rear-projection-screen
- https://www.gerriets.us/us/products/projection-screens/projection-screens/rear-projection?mode=content
- https://www.rosebrand.com/Downloads/Rose-Brand-Projection-Material-Information.pdf
- https://broadwayeducators.com/projections-on-stage-part-iii-choices-about-screens/
- http://www.av-info.eu/video/rearprojection.html
- https://stagedepot.co.uk/scenic-effects/video/projection-screen/faq
- https://www.controlbooth.com/threads/rear-projection-small-throw-distance-available.48228/
- https://epson.com/Accessories/Projector-Accessories/ELPLX01-Ultra-Short-throw-Lens/p/V12H004X01
- https://www.bhphotovideo.com/c/product/1616719-REG/epson_v12h004x01_elplx01_ultra_short_throw_lens.html
- https://www.audiovideonation.com/panasonic-ultra-short-throw-lens-for-panasonic-3-chip-dlp-projectors/
- https://files.support.epson.com/docid/cpd4/cpd40467/source/basic_use/concepts/projection_modes_upside_down.html
- https://www.projectorcentral.com/Understanding-Lens-Offset-and-Lens-Shift.htm
- https://www.projectorreviews.com/terms/digital-keystone-correction/
- https://epson.com/projector-guide-how-to-buy-a-projector-image-position-adjustment

Light budget, lumens and black level:
- https://acousticfrontiers.com/blogs/articles/image-brightness-targets-and-calculating-foot-lamberts-from-projector-lumens
- https://elitescreens.com/2016/04/ansi-lumens-foot-lamberts-image-luminance/
- https://www.valerion.com/blog/understanding-projector-brightness-lumens-ansi-lumens-iso-lumens-lux-foot-lamberts-and-nits
- https://calculator.academy/lumens-to-nits-calculator/
- https://www.projectorcentral.com/projector-contrast-ratio.htm
- https://www.christiedigital.com/globalassets/help-center/whitepapers/documents/christie-contrast-matters-whitepaper.pdf
- https://www.citytheatrical.com/resources/videos/projector-dowser
- https://wahlberg.dk/products/dmx-projector-shutter
- https://www.theblacky.com/en/features.html
- https://broadwayeducators.com/projections-on-stage-part-i-how-do-i-make-them-brighter/
- https://broadwayeducators.com/lighting-the-stage-when-using-projections/
- https://www.bu.edu/fitrec/files/2020/07/Dance-Theater-Inventory-7.2020.pdf
- https://www.rebeam-shop.com/en/products/projectors/from-20.000-ansi-lumens/
- https://www.projectorcentral.com/best-bright-projectors.htm
- https://theatreave.com/blogs/news/projection-tip-5-simple-screens-mindful-lighting
- https://theatreave.com/blogs/news/5-projection-tips
- https://shehds.com/blogs/news/how-bright-should-stage-lights-be-for-performances-and-events
- https://finedesignassociates.com/resources/theatrical-lighting-mechanics/

Haze:
- https://www.americantheatre.org/2018/06/19/a-hazy-shade-of-theatre-the-case-for-clearer-design/
- https://www.controlbooth.com/threads/projecting-through-haze.4450/
- https://www.4wall.com/help/will-haze-set-off-fire-alarms-or-smoke-detectors--n-185
- https://entertainingsafety.com/knowledge-base/theatrical-fog-and-smoke-detectors-how-to-avoid-setting-off-fire-alarms/
- https://actorsequity.org/resources/producers/safe-and-sanitary/smoke-and-haze
- https://myusf.usfca.edu/usf-stages/special-effects

Output chain and latency:
- https://resolume.com/support/en/syphonspout
- https://resolume.com/support/en/output-transformation
- https://madmapper.com/madmapper/faq
- https://docs.derivative.ca/Palette:kantanMapper
- https://interactiveimmersive.io/blog/touchdesigner-resources/tips-for-working-with-projectors-and-touchdesigner/
- https://limeartgroup.com/top-5-video-mapping-software/
- https://alternativeto.net/software/qlab/?platform=windows
- https://blog.daydream.live/spout-and-syphon-in-scope-zero-latency-on-your-machine/
- https://www.projectorcentral.com/projector-input-lag.htm
- https://forum.blackmagicdesign.com/viewtopic.php?f=4&t=50687
- https://community.intel.com/t5/Graphics/Unable-to-achieve-4K-60Hz-using-the-latest-Iris-Xe-i7-1165G7/m-p/1222475

Colour and working with the lighting designer:
- https://www.jaspertronics.com/blogs/tech-updates/the-role-of-color-temperature-in-projector-performance
- https://www.valerion.com/uk/blog/best-color-temperature-projector-calibration
- https://leefilters.com/lighting/colour-temperature-calculator/

Show control and redundancy:
- https://help.ableton.com/hc/en-us/articles/360010120320-SMPTE-Timecode-FAQ
- https://isotonikstudios.com/product/ltc-to-mtc-converter-by-leolabs/
- https://www.showsync.com/tools/
- https://www.etcconnect.com/WebDocs/Controls/EosFamilyOnlineHelp/en/Content/23_Show_Control/02_Timecode/TIMECODE.htm
- https://community.etcconnect.com/control_consoles/eos-family-consoles/f/eos-family/25312/ableton
- https://www.doremievent.com/dual-video-server-redundancy/
- https://plsn.com/articles/video-digerati/redundancy/
- https://www.blackmagicdesign.com/products/atemmini
- https://support.microsoft.com/en-us/windows/change-the-power-mode-for-your-windows-pc-c2aff038-22c9-f46d-5ca0-78696fdf2de8

Safety:
- https://w3c.github.io/wcag21/understanding/three-flashes-or-below-threshold.html
- https://digitalaccessibility.nyu.edu/testing/sc231.html
- https://www.ofcom.org.uk/__data/assets/pdf_file/0021/16248/gn_flash.pdf
- https://www.itu.int/rec/R-REC-BT.1702/
- https://pmc.ncbi.nlm.nih.gov/articles/PMC11872230/
- https://epilepsysociety.org.uk/news/minimising-risk-flashing-lights
- https://epilepsysociety.org.uk/sites/default/files/2023-07/PhotosensitiveepilepsyJuly2023.pdf
- https://en.wikipedia.org/wiki/Harding_test
- https://www.hardingfpa.com/technical-support/how-to-interpret-hardingfpa-results/
- https://trace.umd.edu/peat/
- https://github.com/tokoroten/iris-pse-detection
- https://www.traverse.co.uk/content-warnings
- https://www.hse.gov.uk/event-safety/special-effect.htm

Rehearsals:
- https://smnetwork.org/forum/stage-management-plays-musicals/best-way-to-run-tech-week-cue-to-cue-rehearsal/
- https://aact.org/projection-designer
- https://brodiegraphics.com/projection-designer/
- https://en.wikipedia.org/wiki/Technical_rehearsal

**Not found / unknown:**
- a published maximum APL for dance
- Israeli strobe-warning practice
- Rosco Black and Twin White gain figures
- the input latency of the unknown projector and of the ATEM Mini
- whether the Leolabs M4L device runs on Live 10
- the terms of the TouchDesigner licence for paid shows
