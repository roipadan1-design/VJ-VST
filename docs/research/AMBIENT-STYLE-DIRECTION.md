# Ambient style direction: "lying on your back"

Status: research done, first two scenes built (**17 Lumia**, **18 Liquid Light**). They compile and render in the real engine: ~55-58 fps on the Iris Xe with `--demo`, and not black. They have **not** been checked in Ableton Live yet.

The brief, in the owner's words: visuals for ambient music. No kicks, just the feeling of lying on your back and relaxing. Cool references, nothing boring.

---

## 1. The mindset in one paragraph

Ambient visuals are a **room**, not a show. Eno's rule for the music applies to the picture too: it should be ["as ignorable as it is interesting"](http://music.hyperreal.org/artists/brian_eno/MFA-txt.html) (Music for Airports liner notes). You can glance away for a minute, look back, and it has changed without you seeing it change. In the good references nothing happens *to* the image. Light moves **through a medium**: fog, oil, water, a curtain of charged air. The motion is on the scale of breathing and weather (20-120 s cycles), not beats. The ambient style is a slower sibling of the existing one. It keeps the shared DNA: mostly dark frame, thin light, generative only, film grain, 3-colour palette. It trades the cuts, thresholds and hits for **dissolves, translucency and swells**.

## 2. References (what each one teaches)

### Light art and expanded cinema

| Reference | What it is | What we take |
|---|---|---|
| **Thomas Wilfred, *Lumia Suite, Op. 158*** (MoMA, 1964-81). [MoMA](https://www.moma.org/collection/works/80014), [Yale "Lumia" show](https://artgallery.yale.edu/exhibitions/exhibition/lumia-thomas-wilfred-and-art-light), [Hyperallergic](https://hyperallergic.com/thomas-wilfred-lumia/), [Apollo](https://apollo-magazine.com/thomas-wilfred-lumia-light-machines-artist-inventor/) | Veils and skeins of coloured light drifting across a screen in a dark room. The full cycle runs about 9 years. At times most of the picture falls into darkness. | **Scene 17 Lumia.** Veils, not objects. Darkness is part of the composition. Change you can't catch happening. |
| **Brian Eno, *77 Million Paintings*** ([Wikipedia](https://en.wikipedia.org/wiki/77_Million_Paintings), [Wired interview via moredarkthanshark](https://www.moredarkthanshark.org/eno_int_wired-jul07.html)) | Hand-made images layered and cross-faded by software. Eno says visitors arrive hectic and slowly settle to the work's very slow pace. | Layers **add up** (Lumia's veils add, never occlude). Long crossfades between scenes (4-5 s, on the bar). |
| **Eno, *Mistaken Memories of Mediaeval Manhattan*** (1980-81, vertical "video paintings"). [DesignObserver](https://designobserver.com/from-the-archive-brian-eno-artist-of-light/) | A camera barely moves; clouds pass. Eno described them as paintings you can walk away from. | The picture should hold as a still. Motion only adds a second layer of reading. |
| **Anthony McCall, *Line Describing a Cone*** (1973). [Tate Papers](https://www.tate.org.uk/research/tate-papers/08/anthony-mccall-line-describing-a-cone), [Wikipedia](https://en.wikipedia.org/wiki/Line_Describing_a_Cone) | A single projected line takes 30 min to become a cone of light in haze. | Extreme slowness is legitimate. Light becomes solid when something (haze) scatters it. |
| **Fujiko Nakaya fog sculptures**, and **Sakamoto + Shiro Takatani + Nakaya, *LIFE-WELL*** ([MOT Tokyo](https://www.mot-art-museum.jp/en/exhibitions/RS/), [Nakaya, Wikipedia](https://en.wikipedia.org/wiki/Fujiko_Nakaya), [LIFE-WELL](https://www.sitesakamoto.com/artworks/12/)) | Fog as a medium that carries light and shadow. Nakaya's interest is in decay and dispersal. | "Transmitted, scattered light", not emitted lines. The haze term in Lumia. |
| **James Turrell, Ganzfelds** ([PopSci on the perceptual science](https://www.popsci.com/science/article/2013-07/james-turrell-psychology/)) | Colour changes so slowly that there is nothing to fixate on. | A floor for how slow a palette or colour drift can go (route smoothing of 5-10 s on the colour routes). |
| **Tim Hecker live, with MFO (Marcel Weber)**, *EPHEMERA* ([MFO](https://mfoptik.de/ephemera_live/), [MIRA](https://mirafestival.com/en/artista/tim-hecker-live-lightshow-by-mfo/)) | A dark, fogged room. Fields of colour in the haze driven by harmonic waves and Perlin noise. Hecker calls the concert "a break from the despotism of the eye". | For drone sets the visual should almost disappear. That's why REST exists and the scenes go nearly dark in silence. |
| **Paul Clipson with Grouper, *Hypnosis Display*** ([KQED](https://www.kqed.org/arts/10540951/interview-grouper-and-paul-clipson-discuss-hypnosis-display), [Wikipedia](https://en.wikipedia.org/wiki/Paul_Clipson)) | Live-layered 16 mm and Super 8 superimpositions, macro against vast. Image and music stay independent, and loose sync happens by accident. | Don't over-sync. Sustained signals lean on the picture; they don't drive it. |
| **Luke Savisky with Stars of the Lid** ([Luke Savisky](https://lukesavisky.com/), [Hicks.design](https://hicks.design/journal/stars-of-the-lid-live)) | Kaleidoscopic projection described as "glowing, serene and volcanic", morphing along with drones in churches. | Glow and serenity can still have mass. |

### Liquid light (1960s-70s)

| Reference | What we take |
|---|---|
| **Joshua Light Show**, Fillmore East ([Wikipedia](https://en.wikipedia.org/wiki/The_Joshua_Light_Show), [Vice](https://www.vice.com/en/article/experimental-imagery-meet-the-grandfathers-of-vjing-joshua-light-show/)). **Mark Boyle & Joan Hills**, UFO Club / Soft Machine ([Four Corners](https://www.fourcornersbooks.co.uk/articles/boyle-family-light-shows/)). **Bill Ham, Glenn McKay** ([Liquid light show, Wikipedia](https://en.wikipedia.org/wiki/Liquid_light_show)) | The actual mechanism was **transmitted light**: dyes and oil between clock glasses on an overhead projector. The lamp's heat made convection and boiled the dye. Operators rocked the dish and dripped in more liquid. The immiscible boundary (the meniscus) draws a line, and oil blobs act as lenses. **This is scene 18.** |
| **Kim Keever**, pigment in a 200-gallon tank ([Hasselblad](https://www.hasselblad.com/inspiration/stories/kim-keever-painting-or-abstract-photography/), [It's Nice That](https://www.itsnicethat.com/articles/kim-keever-abstracts)) | Diffusing clouds of colour. The "let physics do it" attitude, which is why scene 18 simulates the liquid instead of faking it with noise. |
| **Jordan Belson, Vortex Concerts / *Samadhi*** ([Center for Visual Music](https://centerforvisualmusic.org/Belson/)) | Planetarium visual music, and the direct ancestor of the light shows. A meditative, not psychedelic, reading of the same tools. |

### Long exposure and oscilloscope

| Reference | What we take |
|---|---|
| **Hiroshi Sugimoto, *Seascapes*** (exposures up to 3 h) and ***Theaters*** (a whole film in one exposure). [Art Institute of Chicago](https://www.artic.edu/articles/1078/a-voyage-on-hiroshi-sugimotos-seascapes), [Public Delivery](https://publicdelivery.org/hiroshi-sugimoto-theaters/) | Long exposure turns motion into a calm field and time into light. A horizon is the most restful composition there is. Lumia's veils settle toward a horizon line in silence. |
| **Ben F. Laposky, *Oscillons*** (1950s oscilloscope photographs). [Wikipedia](https://en.wikipedia.org/wiki/Ben_F._Laposky), [Vasulka archive PDF](https://vasulka.org/archive/Artists3/Laposky,BenF/ElectronicAbstractions.pdf). **Mary Ellen Bute, *Abstronic*** ([Center for Visual Music](https://www.centerforvisualmusic.org/ButeRetrospective.htm)) | Slow Lissajous and waveform light, photographed with long exposures: thin glowing curves with phosphor persistence. **Candidate for the next scene** (see section 6). |

### The aurora (physics used directly)

[Sky & Telescope](https://skyandtelescope.org/stargazing-and-observing/celestial-objects-to-watch/an-aurora-watchers-guide/), [EarthSky, forms of aurora](https://earthsky.org/astronomy-essentials/forms-of-aurora-arcs-curtains-corona/). A curtain is made of parallel rays along the magnetic field. It has a sharp, frilled lower border and folds. **Where a curtain is seen edge-on it narrows to a bright streak.** Lumia renders exactly that: the fold's compression `1/|du/dx|` is its brightness. Shader reference for later volumetric work: nimitz, ["Auroras"](https://www.shadertoy.com/view/XtGGRt) (CC BY-NC-SA, so for study only, not for copying).

### Anti-references (the "chillout screensaver" we are *not* making)

Space nebula stock loops, starfields, lens flares, lava-lamp blobs in saturated rainbow, Milkdrop and Winamp-style kaleidoscopes, glossy "AI fluid data sculpture" renders, beat-synced pulsing, and anything centred and symmetric (mandalas). The test: if it would work as a Windows screensaver or a spa waiting-room TV, it's out.

## 3. Design decisions

### Palette: not red-mono

- The engine colours every scene with the performer's **global** 3-colour palette: a gradient map on luminance in `LookPass`, default **Blood**. A preset cannot pick its palette. So the scenes are designed in **luminance**: dark field, a mid body, and light only at hems, meniscus lines and caustics.
- Recommended existing palettes: **Cyanotype** (Lumia's home), **Ash**, **Nitrate** and **Tungsten** (Liquid Light's home). **Scene Colors** shows each scene's own muted duotone. That duotone follows the music's brightness (`descriptor.centroid` -> `tint`): cold sea-glass and violet against warm sodium and dusty rose (Lumia), indigo against amber dye (Liquid Light). Both are desaturated to 60-80%.
- **Proposed new palette presets** for the plug-in. Not added: that's a plug-in change, and new entries must be appended after "Split" to keep saved-set indices.
  - **Dusk** `#06060e / #4a4f7a / #e9dccf`: indigo night, slate, warm paper.
  - **Tide** `#030a0b / #3c6e68 / #dfe8dc`: deep teal, sea-glass, bone.
  - **Sodium** `#080604 / #7a5a2e / #f1dfb8`: sodium-lamp amber, dimmer than Tungsten.

### Motion: continuous, no events

- Every moving part runs on the scene clock (`integrate` rates, `vj_dt`), so Speed, Freeze and Glide behave as in every other scene. The designed speeds are slow: Lumia's drift is 0.05/s, and Liquid Light's convection is about 5-8% of the frame height per second.
- No thresholds, cuts or hard edges are created by the scene. Lumia has none at all. Liquid Light's only lines are the physical meniscus, 1-2 px and anti-aliased.
- The phrase LFO (16 bars on the scene clock) slowly re-folds Lumia and **rocks the dish** in Liquid Light: back and forth, like the operators did.
- Scene changes into these scenes are **crossfades of 4 s and 5 s, quantised to the bar** (every other scene cuts on the beat).

### Audio: sustained signals, slow smoothing

Every audio route has seconds of attack and a longer release. The existing scenes use 0-60 ms. The mapping follows the generator's rules: one audio source per parameter, REST, HIT, BODY and BUILD routes required, and no route into a speed.

| Signal (continuous unless noted) | Lumia | Liquid Light |
|---|---|---|
| `react.energy` (REST, loudness around 0.5) | horizon: veils rise with the level and settle in silence (2.5 s / 6 s) | density: louder thins the dye so more light leaks (2.5 s / 6 s) |
| `descriptor.presence` (~2.5 s "how much is happening") | ray articulation (3 s / 6 s) | lamp (3 s / 6 s) |
| `react.body` (auto-ranged bass) | hem undulation (1.5 s / 5 s) | oil lens magnification (1.5 s / 5 s) |
| `react.tension` (BUILD) | a third veil unfurls (3 s / 10 s) | the glass heats: dye thins, caustics brighten (3 s / 10 s) |
| `react.air` (highs) | slow shimmer along the rays (2 s / 5 s) | caustic rim brightness (1.5 s / 5 s) |
| `descriptor.centroid` (timbre) | own-colour temperature (5 s / 10 s) | dye balance (5 s / 10 s) |
| `env.hit` (HIT, event, required by the rules) | a **slow swell**: rays lengthen and glow (0.7 s / 5 s) | clear spirit runs in along the current and thins a ragged streak of dye (0.3 s / 2.5 s) |
| `env.drop` (event) | haze blooms like a dawn (2 s / 9 s) | the oil clears (2 s / 9 s) |
| kick / snare / accent pulses | **not used** | **not used** |

### Taste guards

- **No eyes.** Lumia is all horizontal bands and vertical rays, with nothing round. In Liquid Light, the oil reservoir is kept to one broad noise octave so lenses have no pinholes or specks. The HIT originally opened a round lit window, which the engine frame dump showed reading like an eye; it was replaced by a ragged streak aligned to the flow.
- **60-80% dark.** Measured through the Cyanotype palette on the engine's frame dump, with film defaults. Lumia: 60-64% of pixels under 8% luminance. Liquid Light: 59-67%.
- **Cheap.** Lumia uses about 25 value-noise taps per pixel, fewer than Membrane. Liquid Light runs one half-resolution float state pass (9 taps plus 5 fetches) and a full-resolution pass with 10 fetches. The first clean run measured 55-58 fps at the engine's auto quality on the Iris Xe. A later run of Membrane, Lumia and Liquid Light together measured ~18 fps for all three, the existing Membrane included, so the machine was throttled.

## 4. How to play them (recommended settings)

- **Character: BREATHE.** Only big moments land, with long hit decays.
- **REACT TO: turn kick off** for true ambient material. The analyser can read swelling low drones as kicks. Kicks are the only source of accents, and accents also trigger engine-wide effects that no scene can switch off: the exposure punch and the time kick.
- LOOK: grain 0.3, halation 0.35, blacks 0.35 (the defaults) suit both scenes. Keep Flash, Glitch, Symbols, Smear, Cut Rate and Shots at 0.
- MOVE: Drift 0.2-0.4 helps Lumia. Speed below 0.5 is "fine control for slow ambient" (quadratic curve).

## 5. Engine follow-ups (recommended, not done)

These are engine-wide and outside a preset's reach. They are listed because they matter most for ambient:

1. **Leaving REST is abrupt.** In BREATHE, `restOutSeconds` is 0.08, so when sound returns after silence the exposure jumps from the rest floor (0.22) to 1 in 80 ms. That's a hard cut in brightness. An "ambient" character, or a per-preset reaction hint, should use restOut of 2-4 s and a restFloor of about 0.45.
2. **Per-preset reaction hints.** A preset field such as `"reaction": { "preferredStyle": "breathe", "kickBeats": 0, "accentStops": 0 }` would let ambient scenes turn off the time kick and accent exposure punch, which are global today.
3. **Scene-suggested palette.** An optional preset field (e.g. `"suggestedPalette": "Cyanotype"`) that the plug-in offers on scene change, so ambient scenes don't open in Blood.

## 6. Next ambient scenes (ranked)

1. **Oscillon.** After Laposky and Bute: 2-3 slow Lissajous and harmonograph curves with phosphor persistence, driven by the pitch/centroid and level of a drone. It needs a persistent buffer for the decay, like scene 18's.
2. **Seascape.** After Sugimoto: a horizon, long-exposure water below and an even sky above. Presence moves the tonal balance. Almost nothing happens, on purpose.
3. **Solid Light.** After McCall: one slowly sweeping plane of light made visible by drifting haze, "a line becoming a cone" over a whole song.

## 7. Files

- `engine/Shaders/Instrument/lumia.fs`, `engine/Presets/17 - Lumia.json`
- `engine/Shaders/Instrument/liquid_light.fs`, `engine/Presets/18 - Liquid Light.json`
- Both presets are defined in `engine/tools/build_instrument_presets.py`, the source of truth. Its `__main__` deletes any preset JSON not in its list. The 16 existing presets regenerate byte-identically, and all rule checks pass.
- Two engine gotchas found while building, worth knowing for any new shader:
  - A shader input must not reuse an engine uniform name (`level`, `bass`, `mid`, `high`, `beatphase`, `onset`, `TIME`), or it won't compile. Lumia's `level` became `horizon`.
  - The engine clears persistent targets to **opaque black (alpha 1)**, so an "initialised" flag must not be alpha = 1. Liquid Light writes a = 0.5.
