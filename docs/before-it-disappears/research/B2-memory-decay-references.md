# B2: Memory and decay in film, video, sound and visual art

*Research strand B2 for "Before It Disappears". Reference library, 44 entries in 6 groups, a top 8
and "What this means for our show". Written 2026-09-24.*

## How to read this

- **Source status.** WebFetch was refused for every domain I tried (en.wikipedia.org, e-flux.com,
  getty.edu), so I stopped after three tries. **Every source here is marked [search]**: the claim
  comes from the text the search engine returned for that URL, not from a page I opened. Under
  brief section 8 ("only include links you actually opened"), that means **every link below needs a
  human click before it goes into CONCEPT.md.** I used only URLs that appeared in my results. Where a
  fact did not appear in any result, I say "unknown" or "not confirmed".
- **No duplication.** Rainer Kohlberger, Ryoichi Kurokawa, Peter Kubelka (*Arnulf Rainer*) and Pierce
  Warnecke (*Textures*, *Data Decay*) are already in `docs/RESEARCH-REPORT.md` §C1–C2. Film-chain
  craft (grain, halation, weave, lifted blacks) is in §B1, and noise as material in §B2. I refer to
  those sections instead of repeating them. None of the seeds in this strand appear in
  `REFERENCE-LIBRARY.md` or `RESEARCH-REPORT.md`.
- **Movements.** "M1–M10" follows the arc in brief §3: M1 waiting · M2 the moment leaves · M3 we reach
  (slow, replay, name) · M4 the frame (outline, softening, colour drift, rewriting) · M5 the pivot
  ("it creates") · M6 the cloud · M7 the shadow · M8 the inversion · M9 transformation · M10 the mark.
- **Stage fact that changes several entries.** The owner confirmed **rear projection**. The projector
  beam is behind the screen, so the audience never sees it. The dancers do not throw the projector's
  shadow onto the image; only the front stage light can put their shadows on the screen, and that
  greys it. Shadow, beam and haze ideas (Boltanski, McCall, Eliasson) therefore have to be
  **generated inside the image**. They cannot come from the room.

### Engine vocabulary used below (defined once)

These are proposals. Costs assume the Iris Xe running at half internal resolution (960×540).

| Tag | What it is | Cost |
|---|---|---|
| **ACC**: accumulation buffer | Running exposure: `acc = mix(acc, frame, a)`, `a = 1 − exp(−dt/T)`. **RGBA8 cannot do long exposures.** A step smaller than 1/255 rounds to zero, so `a` must stay at or above about 0.004, which caps T at about 4 s at 60 fps. For T = 10–120 s you need **RGBA16F** (960×540×8 B ≈ 4.1 MB per texture, 2 for ping-pong) | S, 1 pass, 2 textures |
| **RECALL**: lossy re-encode | A held snapshot that is re-encoded each time it is recalled: downsample ×0.5, quantise to N levels (32→8), blur 1–3 px, domain warp 0.5–2 px, hue shift a few degrees toward the palette's warm end, then write back over the snapshot. The encoding runs **only on a recall event**, not every frame | S (event), 3 passes, 1 texture |
| **RESIDUE**: mark buffer | A buffer that holds on and only fades slowly: `res = max(res * d, edge(frame) * k)`. With `d = 0.9995` per frame the half-life is about 1,386 frames, about 23 s at 60 fps; `d = 0.99995` gives about 3.9 min. The film chain composites it at low gain | S, 1–2 passes, 1 R16F texture |
| **ERODE**: decay mask | 3D value noise thresholded by a rising level `t`. Where `noise < t`, the image turns into emulsion bloom and dust (Nitrate palette) or into black | S, 1 pass |
| **PHASE**: structure scramble | A cheap spatial stand-in for Paulstretch: `low = blur(src)`, `detail = src − low`, `out = low + |detail| * signedNoise(t)`. It keeps the light and local contrast and throws away the arrangement | S, 2 passes |
| **REPLAY**: control-stream loop | Record the OSC, macro and trigger stream for N bars and play it back, adding drift (Δ) on every pass. This costs almost nothing and is deterministic, which suits "repeatable and recoverable". A pixel ring buffer is the expensive alternative: 60 frames at 480×270 RGBA8 ≈ 31 MB | ≈0 (stream) / M (pixels) |

---

## Group 1: Time made visible (exposure, stills, the frame)

### 1. Hiroshi Sugimoto, *Theaters* (series begun 1978)
- **Link:** https://fraenkelgallery.com/portfolios/hiroshi-sugimoto-theaters [search] ·
  https://publicdelivery.org/hiroshi-sugimoto-theaters/ [search]
- **Looks like:** a black-and-white photograph of an empty cinema or drive-in. The screen is a
  glowing pure-white rectangle. The seats and ornament are lit only by the screen's own spill.
- **Principle:** the shutter stays open for the whole feature film, and the projector is the only
  light source. Thousands of images add up to white; the story vanishes and only the light it gave
  off remains. Sugimoto had "a vision of a shining screen" in 1976.
- **Take:** accumulation as a form. A whole section, or the whole night, can sum into one image.
  The **architecture lit by the spill** is the part that survives. That is "light at the edges"
  exactly.
- **Avoid:** a full-white screen. At that level it blinds the dancers' silhouettes and breaks the
  luminance budget. Also avoid a literal proscenium or cinema frame drawn on screen.
- **Serves:** M4, "A memory lives inside a frame. / It keeps the outline, / never the whole story",
  and M10, "Only a mark."
- **Engine:** **ACC** with T = 30–120 s fed by the scene before the film chain, then tone-mapped so
  the sum is **edge-weighted**: `acc_display = acc * (1 − centreMask)`. The screen's white becomes a
  soft rim. Use it as the finale's "accumulated night" layer. S.

### 2. Hiroshi Sugimoto, *Seascapes* (1980→)
- **Link:** https://fraenkelgallery.com/portfolios/hiroshi-sugimoto-seascapes [search] ·
  https://www.artic.edu/articles/1078/a-voyage-on-hiroshi-sugimotos-seascapes [search]
- **Looks like:** sea and sky split exactly at the centre horizon. Long exposure flattens the waves
  and clouds into grey tone. The series is monochrome.
- **Principle:** the one view "we still share with the ancients". Sugimoto calls photography the
  "fossilization of time". Nothing happens, and that is the content.
- **Take:** a single **horizon line** as the calmest possible state of the screen. There are two
  tones and one line, and long exposure removes every event. It suits M1 (waiting) and is a
  candidate "rest" preset.
- **Avoid:** a literal sea or waves, and pictorial landscape.
- **Serves:** M1, "They wait. / Quietly. / Like silence."
- **Engine:** Signal Fog (scene 08) or a new "Horizon" scene squeezed to a 2–4 px band at y = 0.5,
  run through **ACC** (T ≈ 20 s) so any motion smears into tone. Palette Ash or Bone. S.

### 3. Chris Marker, *La Jetée* (1962)
- **Link:** https://sf-encyclopedia.com/entry/jetee_la [search] ·
  https://aspectfilmjournal.web.unc.edu/2023/09/mah-an-examination-of-time-medium-and-the-moving-image-in-la-jetee/ [search]
- **Looks like:** a "photo-roman" of about 200 black-and-white stills with dissolves and a narrator.
  Only one shot moves: a woman slowly opening and closing an eye.
- **Principle:** memory as stills. Motion is so rare that when it arrives it feels like a miracle.
  They could afford the movie camera for one afternoon, according to the SF Encyclopedia summary in
  my results.
- **Take:** a **grammar of held frames**. Freeze the generative image, cross-dissolve between
  freezes, and let real motion return for a few seconds only once. The rarity is the event, which
  fits the owner's "rare correlated events".
- **Avoid:** **the eye.** The film's key moment is exactly the form the owner forbids. Also avoid
  faces and post-apocalyptic imagery.
- **Serves:** M3, "We slow it down. / We replay it.", and M2, "Movement disappears before we can hold
  it."
- **Engine:** a "Hold" control: freeze the scene's time uniform, keep the film chain (grain at
  24 fps, weave) running over the still, and move to the next hold with a snapshot morph of 1–4
  bars. One "release" per section lets time run at 1.0 for 4–8 s. Cost ≈0.

### 4. Chris Marker, *Sans Soleil* (1983, year not confirmed in my results): "the Zone"
- **Link:** https://rhizomes.net/issue8/tryon.htm [search] ·
  https://mubi.com/en/notebook/posts/chris-marker-s-imaginary-japan [search] ·
  https://www.modwiggler.com/forum/viewtopic.php?t=69702 [search]
- **Looks like:** travel-diary footage. In "the Zone" sequences, archival images (1960s protests,
  Cabral's soldiers) are pushed through a video synthesizer into flat, posterised, solarised colour
  fields.
- **Principle:** the synthesizer belongs to Hayao Yamaneko, a fictional character, and the Zone is
  named in homage to Tarkovsky's *Stalker*. It "remembers" the images and transforms them. The
  narration says the synthesized images "proclaim themselves to be what they are: images, not the
  portable and compact form of an already inaccessible reality". The results describe these images
  as "affected by the moss of time".
- **Take:** **this is our manifesto's precedent.** A machine that remembers by *transforming* is
  more honest than one that pretends to preserve. It justifies a purely generative instrument for a
  piece about memory.
- **Avoid:** the 1980s video-synth rainbow look (full-spectrum posterisation) and any archival
  political footage.
- **Serves:** M5, "it no longer recalls. / It creates."
- **Engine:** quantise luma into 3–5 bands and gradient-map them through the section palette (Crush
  plus palette, already present). The Zone should be a *state* the image enters, not a filter. S.

### 5. Hollis Frampton, *(nostalgia)* (1971)
- **Link:** https://bombmagazine.org/articles/2006/10/01/hollis-framptons-nostalgia/ [search] ·
  https://www.screeningthepast.com/issue-22-reviews/hollis-frampton-nostalgia/ [search]
- **Looks like:** twelve black-and-white photographs, each burning slowly on a hot plate. They curl,
  blacken and glow at the edge. Michael Snow reads the commentary. About 36–38 min.
- **Principle:** **sound and image are out of step.** While one photo burns, the voice describes
  the *next* one, so the viewer always remembers ahead of, or behind, what they see.
- **Take:** (a) destruction that starts **from the edge inward** as a slow, beautiful event, and
  (b) **offset recall**: the image answers the music one phrase late, so each section's image is the
  memory of the previous musical phrase.
- **Avoid:** literal photographs and literal fire. Avoid illustrating burning.
- **Serves:** M8, "Perhaps we remember / the last time / we remembered it."
- **Engine:** (a) **ERODE** with the threshold field centred on the frame edge
  (`noise + edgeDist*k < t`), plus a hot rim through Halation. (b) **REPLAY** with a one-phrase
  delay: the macros follow the audio features of 4–8 bars ago. S / ≈0.

### 6. Andrei Tarkovsky, *Mirror* (1975)
- **Link:** https://www.bfi.org.uk/film/97341aa7-c937-57bf-9011-99bb084472b9/mirror [search] ·
  https://www.scenebygreen.com/2023/12/04/mirror-1975/ [search]
- **Looks like:** colour and black-and-white alternate across three eras. There are dream rooms
  where water pours down the walls and the ceiling dissolves. A barn burns in the rain, first seen
  through a dirty mirror.
- **Principle:** memory is non-linear and mixes up its materials (fire *and* rain). The film trusts
  the texture of one remembered room more than a story.
- **Take:** **impossible weather** as memory: two contradictory elements in the same frame (rising
  particles *and* falling streaks). Monochrome switching as a structural cue. A slow camera
  movement that "discovers" rather than cuts.
- **Avoid:** the Tarkovsky pastiche (dripping water, levitation, wind in grass) and any figure.
- **Serves:** M6, "Memory is a cloud. / Always changing. / Always drifting."
- **Engine:** two particle layers with opposite gravity (the Gravity macro split ±) inside Signal
  Fog, and a palette toggle between Ash (monochrome) and one warm palette on a 16-bar snapshot
  morph. M.

### 7. Jonas Mekas, *Reminiscences of a Journey to Lithuania* (1972)
- **Link:** https://mubi.com/en/us/films/reminiscences-of-a-journey-to-lithuania [search] ·
  https://www.slantmagazine.com/film/reminiscences-of-a-journey-to-lithuania/ [search]
- **Looks like:** a diary film of handheld Bolex fragments, single-frame bursts, overexposed light
  and flares. Mekas and his brother return to their village after 27 years. 88 min.
- **Principle:** memory arrives as flickering fragments of *light* rather than scenes. Home movies
  "raw and unedited".
- **Take:** **short bursts** (2–12 frames) of overexposure as the rhythm of involuntary memory,
  landing where you don't expect them ("settling in places / we never expect").
- **Avoid:** the handheld-nostalgia look and "home movie" sepia. Keep the bursts within the 3/s
  flash cap.
- **Serves:** M6, "A voice. / A scent. / A familiar light."
- **Engine:** a "Glimpse" event: 3–8 frames of lifted exposure plus Halation at the frame edge,
  fired by a probabilistic gate (p ≈ 0.02 per bar, only when presence is high). Uses the existing
  Flash limiter. S.

### 8. Tacita Dean, *Disappearance at Sea* (1996)
- **Link:** https://artlyst.com/features/tacita-dean-disappearance-at-sea-1996-significant-works-sue-hubbard/ [search] ·
  https://www.mariangoodman.com/artists/39-tacita-dean/works/39446/ [search]
- **Looks like:** 16 mm anamorphic colour, 14 min, seven long static shots. The lighthouse lamps
  turn in close-up, alternating with the sea. Dusk falls to night; the sky moves through yellow,
  red and purple.
- **Principle:** the light that is meant to guide becomes the subject, while the thing it looks for
  (the lost sailor, Donald Crowhurst) is absent.
- **Take:** a **rotating beam** that sweeps a dark frame and lights only what it passes: a
  "familiar light" that returns on a period. One real dusk-to-night colour shift over a whole
  section.
- **Avoid:** lighthouse imagery and the sea.
- **Serves:** M6, "A familiar light.", and M1, waiting.
- **Engine:** a radial mask rotating with a phrase LFO (one turn per 2–4 bars) multiplying any
  scene, with ACC at T ≈ 3 s so the beam leaves a fading wake. Palette morph Tungsten → Violet over
  the section. S.

---

## Group 2: The material decays (film as a body)

### 9. Bill Morrison, *Decasia* (2002)
- **Link:** https://www.moma.org/calendar/events/512 [search] ·
  https://bombmagazine.org/articles/bill-morrisons-decasia-the-state-of-decay/ [search] ·
  https://www.loc.gov/static/programs/national-film-preservation-board/documents/decasia.eagan.pdf [search]
- **Looks like:** 67 min of silent-era footage whose nitrate is decomposing. Blooms, bubbles and
  blotches eat the figures. A whirling dervish opens it. Michael Gordon's symphonic score was
  premiered live by the Basel Sinfonietta in 2001.
- **Principle:** the decay *is* the image. The emulsion's own death animates the film; mortality is
  shown through the medium.
- **Take:** **decay as an active, spreading, beautiful process**: organic blooms that grow from
  seeds and swallow form. Our **Nitrate palette** is named for exactly this. Also a live score with
  image: the image as the music's second body.
- **Avoid:** found footage (the brief is generative only), "old film" nostalgia, and sepia kitsch.
  Morrison's archive figures are out. So is uniform all-over scratching; per §B1, real defects are
  rare and correlated.
- **Serves:** M4, "The edges begin to soften. / The colours slowly change."
- **Engine:** **ERODE** driven by Morphogen (scene 10) as the mask. Reaction-diffusion spots are a
  convincing stand-in for nitrate blooms. Seeds come from rare kicks, the bloom's rim runs hot
  through Halation, and the palette is Nitrate. M (the RD already runs).

### 10. Bill Morrison, *Light Is Calling* (2004)
- **Link:** https://michaelgordonmusic.com/music/light-is-calling/ [search] ·
  https://366weirdmovies.com/bill-morrisons-light-is-calling-2004-and-just-ancient-loops-2012/ [search]
- **Looks like:** one scene from *The Bells* (1926), optically reprinted and cut to Gordon's 7-min
  piece. Bodies dissolve "in swirling waves of golden light".
- **Principle:** a *single* remembered scene, stretched and reprinted until the decay turns ecstatic
  rather than tragic.
- **Take:** proof that decay can be **radiant, not sad**. It is a model for the "paradise" side of
  M7, and a length model: 7 minutes is close to our 5-min finale.
- **Avoid:** golden-sepia as a default grade. Keep it earned, once.
- **Serves:** M7, "Sometimes that shadow / feels like paradise."
- **Engine:** ERODE with the mask's rim added (not subtracted) to the image, in Tungsten or Ember,
  Halation at 0.3–0.4. S.

### 11. Bill Morrison, *Dawson City: Frozen Time* (2016)
- **Link:** https://wexarts.org/blog/bill-morrison-dawson-city-frozen-time [search] ·
  https://www.tcm.com/articles/1434281/dawson-city-frozen-time [search]
- **Looks like:** 533 reels of nitrate found in 1978 in the permafrost under a demolished swimming
  pool in the Yukon, damaged along their edges by water. The score is by Alex Somers.
- **Principle:** memory **preserved by being buried and forgotten**. The cold kept it; the damage
  sits at the edges while the centre often survives.
- **Take:** "They don't [fade]. / They wait." This is the most literal real-world case of M1. Also
  damage that **frames** the image (edge rot) instead of covering it.
- **Avoid:** historical-documentary content, and treating the archive as an illustration.
- **Serves:** M1, "We think memories fade. / They don't. / They wait."
- **Engine:** ERODE constrained to a 5–15 % border band of the frame (vignette-shaped mask), with a
  clean centre. Dormant state: the whole scene lives at 5–10 % luma under the mask and waits for
  a "wake" cue. S.

### 12. Jürgen Reble, *Instabile Materie* (1995) / *Materia Obscura* (2009)
- **Link:** https://filmalchemist.de/films/instabile_materie.html [search] ·
  https://www.sensesofcinema.com/2004/cteq/jurgen_reble/ [search] ·
  https://expcinema.org/site/en/blu-ray/j%C3%BCrgen-reble-materia-obscura [search]
- **Looks like:** hand-processed 16 mm strips covered with chemicals. Salts crystallise into
  "molding shapes"; microscopic particles float in the emulsion. In 2009 it was re-scanned frame by
  frame and **slowed down** for HD.
- **Principle:** the emulsion itself is the landscape. There is no depicted subject, only matter
  reacting. Slowing it down turns chemical noise into morphology.
- **Take:** the closest analogue precedent to our **generative** stance. Abstract matter,
  crystallisation fronts, particles. Slowing a chaotic process until it reads as form.
- **Avoid:** saturated chemical rainbow colour; stay monochrome.
- **Serves:** M5, "It creates.", and M9, transformation.
- **Engine:** Morphogen at low feed/kill, with the time scale slowed to 0.1–0.25 in the section.
  Ink (scene 07) for the soak fronts. S–M.

### 13. Stan Brakhage, *Mothlight* (1963)
- **Link:** https://www.artforum.com/features/j-hoberman-on-stan-brakhages-mothlight-200839/ [search] ·
  https://filmsinreview.lib.byu.edu/film_review/mothlight/ [search]
- **Looks like:** moth wings, petals, seeds and grass pressed between two strips of 16 mm splicing
  tape and contact-printed. A flickering, frame-by-frame rush of translucent organic shapes. No
  camera.
- **Principle:** cinema made of the *remains* of living things. The material chosen had to be thin
  and translucent enough to let light through.
- **Take:** **translucency**: forms that exist only because light passes through them. Fine veined
  structures (our Fibers and Mesh Body lines) caught one frame at a time.
- **Avoid:** its frame-to-frame flicker rate as a constant; it would break our 3/s cap if it read
  as flashes. Use it on texture, not luminance. Avoid literal insects and wing shapes (these can
  read as eyes and "wings" symbolism).
- **Serves:** M10, "The quiet mark / left / by everything / that ever / moved us."
- **Engine:** Fibers (scene 05) with backlight (luminance = thickness × light), and texture
  re-seeded at 12 fps while overall luma is held constant. S.

### 14. Nam June Paik, *Zen for Film* (1964)
- **Link:** https://www.eai.org/titles/zen-for-film [search] ·
  https://www.moma.org/collection/works/128108 [search] ·
  http://www.medienkunstnetz.de/works/zen-for-film/ [search]
- **Looks like:** clear 16 mm leader projected: a white rectangle. Over time it collects dust and
  scratches, and viewers' shadows fall on it. Paik recommended kicking the film around a dirty
  floor first.
- **Principle:** "clear film, accumulating in time dust and scratches." The work's content is the
  history of its own projection.
- **Take:** **the dust layer as memory**. Our Dust should *accumulate* over the night, not be
  re-randomised: particles that land stay (RESIDUE). By the finale the "empty" frame carries
  everything that happened to it.
- **Avoid:** a bright white rectangle as the base (luminance); invert it to black leader with
  light dust.
- **Serves:** M10, "Only a mark.", and M8, "every time we remember, / we leave a part of ourselves
  behind."
- **Engine:** make Dust persistent: new specks write into a **RESIDUE** buffer with d = 0.99995
  (half-life ≈ 3.9 min), so each section leaves dust that the next inherits. S.

### 15. Peter Tscherkassky, *Outer Space* (1999)
- **Link:** https://expcinema.org/site/en/wiki/work/outer-space [search] ·
  https://www.filmcomment.com/interview-peter-tscherkassky/ [search]
- **Looks like:** cameraless black-and-white found footage (*The Entity*, 1982), contact-printed
  by hand in the darkroom. Each frame is 5–7 layered exposures; sprocket holes and soundtrack
  intrude into the picture, and the image tears.
- **Principle:** **palimpsest by re-exposure**: the same strip of stock is exposed to several strips
  of source, so every frame is an optical collage of different moments.
- **Take:** multiple time-offset exposures of **our own scene** composited additively (5–7 layers,
  each a different moment); frame edges and sprockets as intruding structure (with restraint).
- **Avoid:** its horror narrative and violence, the figure, and aggressive shock cutting.
- **Serves:** M4, "The story is rewritten / every time it is remembered."
- **Engine:** composite 3–5 **REPLAY**-offset instances of the scene uniforms (t, t−0.5 s, t−2 s…)
  as additive layers. Each is a full scene render, so this is **L**. Cheaper: 3 **ACC** buffers at
  different T (0.2 s, 2 s, 20 s) mixed at 0.5/0.3/0.2. M.

### 16. Tony Conrad, *Yellow Movies* (1972–73)
- **Link:** https://www.moma.org/collection/works/108075 [search] ·
  https://buffaloakg.org/blog/exhibition-spotlight-tony-conrads-yellow-movies [search] ·
  https://greenenaftaligallery.com/exhibitions/tony-conrad2 [search]
- **Looks like:** large sheets of paper with a black border in cinema proportions, filled with cheap
  house paint that Conrad knew would yellow. Twenty-three of them.
- **Principle:** "films that could run for a lifetime": the only event is the slow colour shift of
  the material.
- **Take:** **the palette itself should age across the night**, too slowly to notice in any one
  section but plain by the finale. "The colours slowly change" as a global, show-long process, not
  an effect.
- **Avoid:** literal yellow-tinting and a sepia signature.
- **Serves:** M4, "The colours slowly change."
- **Engine:** a show-long "Age" parameter (0 → 1 across the Arrangement) that lerps each palette's
  3 stops by a small, fixed offset (e.g. white-point from #F2F2F0 toward #EDE6D6; black-point lift
  +1–2 %). Cost ≈0 (uniform).

### 17. Paul Sharits, *Shutter Interface* (1975)
- **Link:** https://www.metmuseum.org/art/collection/search/623288 [search] ·
  https://www.artic.edu/artworks/214310/shutter-interface [search]
- **Looks like:** four 16 mm projectors with overlapping loops of solid-colour frames broken by
  single black frames, forming seven pulsing colour strips on the wall. High-frequency tones.
- **Principle:** anti-illusion: the apparatus (frames, shutter, overlap) is the content.
  Overlapping loops of different lengths never realign.
- **Take:** **loops of unequal length** (phasing), so combinations never repeat exactly. This is a
  structural idea for REPLAY: loop lengths 7, 11 and 13 bars.
- **Avoid:** **the flicker itself.** Solid-colour frame flicker is a photosensitivity hazard and is
  incompatible with our 3/s cap. See RESEARCH-REPORT §C1 (Kubelka) for the same caution.
- **Serves:** M3, "We replay it."
- **Engine:** REPLAY with co-prime loop lengths per macro group. ≈0.

### 18. Gustav Metzger, *Liquid Crystal Environment* (1965–66; with Cream, The Who, The Move at the Roundhouse, 1966)
- **Link:** https://www.lifa-research.org/en/artworks/liquid-crystal-environment/ [search] ·
  https://apollo-magazine.com/gustav-metzger-1926-2017/ [search]
- **Looks like:** heat-sensitive liquid crystals between glass slides in several projectors. As the
  slides cool, fields of colour creep, crack and bloom across the walls.
- **Principle:** **auto-destructive and its twin, auto-creative art**: work driven by processes that
  degrade or grow on their own. It is also one of the earliest UK projections for live music.
- **Take:** the explicit pairing of **auto-destructive and auto-creative** is exactly the text's
  pivot (M4 → M5). And a precedent for slow, material image processes behind live music.
- **Avoid:** psychedelic full-spectrum colour.
- **Serves:** M5, "It no longer recalls. / It creates."
- **Engine:** one process with a sign switch: the ERODE threshold rises (destroys) in M4, then the
  same field drives Morphogen growth (creates) in M5. S–M.

---

## Group 3: Slowness and stretched time

### 19. Bill Viola, *The Reflecting Pool* (1977–79)
- **Link:** https://www.moma.org/collection/works/120413 [search] ·
  https://www.guggenheim-bilbao.eus/en/learn/schools/teachers-guides/reflecting-pool-1977-79 [search] ·
  https://www.eai.org/titles/the-reflecting-pool-collected-work-1977-80 [search]
- **Looks like:** 7 min colour video. A man leaps toward a pool and freezes in mid-air. The scene
  is still; all change happens only in the reflections on the water.
- **Principle:** still-framing and keying **join different layers of time into one image**. One
  layer is frozen while another keeps living.
- **Take:** **split time**: one element of the frame holds (the "outline") while a second layer
  (the reflection or residue) keeps moving. It is a direct way to show "the moment leaves while we
  are inside it".
- **Avoid:** the figure, the leap and water symbolism.
- **Serves:** M2, "Every moment is already leaving / while we are still living inside it."
- **Engine:** freeze the scene uniforms for the main layer (Hold) while the **ACC**/Trails layer
  and grain keep running and are fed from a live copy. Two scene instances cost L; a cheaper route
  is to freeze geometry and animate only the domain-warp of the film chain. S.

### 20. Bill Viola, *The Passing* (1991)
- **Link:** https://www.moma.org/collection/works/118326 [search] ·
  https://www.eai.org/titles/the-passing [search] ·
  https://www.mplus.org.hk/en/collection/objects/the-passing-202164/ [search]
- **Looks like:** 54 min black and white, shot with night-vision, infra-red and ultra-low-light
  cameras. Desert at night, Viola underwater, his mother's death and his son's birth.
- **Principle:** the **edge of visibility**. Low-light sensors make the dark grainy, soft and
  alive, so darkness has texture rather than being empty.
- **Take:** **"black" as a living low-light field**: very low luma with coarse sensor noise, never
  flat black. Birth and death in one arc, like our end ("where the moment ends, and where we
  begin").
- **Avoid:** personal-grief imagery and the figure.
- **Serves:** M1, "Quietly. / Like silence.", and M9, "where the moment ends, / and where we begin."
- **Engine:** a "Low-light" look: Blacks lift 3–5 %, grain amplitude raised in the shadows (the
  floor of the §B1 grain curve at about 0.5k instead of 0.25k), chroma off. S.

### 21. Bill Viola, *Emergence* (2002)
- **Link:** https://www.getty.edu/art/collection/object/108VM9 [search] ·
  https://www.artway.eu/posts/bill-viola-emergence [search]
- **Looks like:** shot on 35 mm at **210 fps** (the source says "seven times slower", so relative to
  30 fps) and played for **11 min 49 s**. A pale figure rises out of a marble cistern, water
  pouring over the rim, and two women catch him. Modelled on a 1400s Masolino fresco.
- **Principle:** extreme slow motion as attention: slowing sensory input makes the viewer aware of
  their own perceiving.
- **Take:** a **rise** held for minutes, with one slow overflow. The slow form of our Emergence
  scene (12) already exists.
- **Avoid:** Pietà and religious iconography, the nude figure (duplicating dancers), and
  melodrama.
- **Serves:** M3, "We slow it down.", and the "Because" finale.
- **Engine:** Emergence (scene 12) at time scale 0.12–0.15 (≈7× slower) with the "overflow" as
  particles spilling over a horizontal rim at y ≈ 0.6. S.

### 22. Douglas Gordon, *24 Hour Psycho* (1993)
- **Link:** https://gagosian.com/news/museum-exhibitions/douglas-gordon-24-hour-psycho/ [search] ·
  https://www.presenhuber.com/exhibitions/douglas-gordon3 [search]
- **Looks like:** Hitchcock's *Psycho* slowed from 109 min to exactly 24 h, "approximately two
  frames per second". My arithmetic: 1,440 / 109 ≈ 13.2× slower, so about 1.8 fps. Silent, on a
  free-standing screen. The 2008 version shows it forward and in reverse side by side.
- **Principle:** at extreme slowness, a familiar image stops being narrative and becomes
  sculpture. Viewers remember the story, but the screen never gets there.
- **Take:** **recognition without narrative**. Also the forward/reverse pairing, relevant to the
  reversal lore around "Because" (strand E; I have not verified it here).
- **Avoid:** appropriation of footage and any horror reference.
- **Serves:** M3, "We slow it down.", and "But the moment itself never returns."
- **Engine:** step-time: advance the scene clock in discrete steps at 1.5–2 Hz with cross-dissolves
  (ACC T ≈ 0.4 s) instead of continuous flow. ≈0.

### 23. Paul Nasca, *Paulstretch* (algorithm, 2006) and Shamantis, *U Smile 800% Slower* (2010)
- **Link:** https://www.npr.org/sections/therecord/2010/08/18/129283985/the-art-of-a-time-stretch [search] ·
  https://www.musicradar.com/news/tech/justin-bieber-slowed-down-800-is-ambient-masterpiece-272104 [search] ·
  https://polarity.me/posts/articles/2026-07-07-paulxstretch-paulstretch-explained/ [search] ·
  https://ui.adsabs.harvard.edu/abs/2022ASAJ..151A.158M/abstract [search]
- **Looks like (sounds like):** a 3:17 pop song turned into a 35+ minute cathedral-like drone
  (Nick Pittsinger / Shamantis, posted to Reddit 16 Aug 2010). A literal 8× would give about 26
  min, so treat "800%" as approximate. It was compared to the *Blade Runner* soundtrack and the
  Cocteau Twins.
- **Principle:** FFT windows, **phase randomised per bin**, overlap-add. It keeps the spectral
  *magnitude* (the colour of the sound) and discards the transients and exact timing. In images,
  Oppenheim & Lim (1981) showed that **phase carries the structure** (edges and arrangement) and
  magnitude carries a content-agnostic energy profile
  (https://ui.adsabs.harvard.edu/abs/1981IEEEP..69..529O/abstract [search]).
- **Take:** **the key model for the finale**, a slowed "Because". Time-stretch keeps the *light* of
  a memory and loses its *outline*, the reverse of M4 ("keeps the outline, never the whole story").
  So the finale can be the moment the image stops keeping outlines and keeps only light.
- **Avoid:** the meme register (a novelty "slowed + reverb" aesthetic), and pairing a stretched song
  with obvious dreamy clichés (lens flares, bokeh hearts).
- **Serves:** M3, "We slow it down.", then M9, "Until we can no longer tell / where the moment ends."
- **Engine:** **PHASE**: blur(src) keeps the light field; the detail band is re-signed by slow
  noise (z-scroll 0.03/s), so structure dissolves while colour and energy remain. Over the finale,
  ramp the phase-scramble amount from 0 to about 0.8. S, 2 passes. Note: the owner's finale is
  "the original recording slowed down"; the stretch ratio and whether it is resampled or
  phase-vocoded are **unknown**. Ask, because a resampled slowdown also drops the pitch while a
  Paulstretch-type stretch does not, and each suggests a different visual.

---

## Group 4: Sound: recall as degradation

### 24. William Basinski, *The Disintegration Loops* (2002–03)
- **Link:** https://www.soundonsound.com/techniques/classic-tracks-william-basinski-disintegration-loops [search] ·
  https://median.newmediacaucus.org/the_aesthetics_of_erasure/the-disintegration-loops/ [search] ·
  https://www.texasmonthly.com/arts-entertainment/william-basinski-september-11-disintegration-loops/ [search]
- **Looks like (sounds like):** 1980s quarter-inch tape loops being digitised in summer 2001. Each
  pass past the head sheds ferrite, so the loop gets more gaps, more muffling and more silence.
  *dlp 1.1* is 63 min. The accompanying video is a single static shot of the last daylight hour
  over lower Manhattan on 11 September, from Basinski's roof.
- **Principle:** **replay is destruction.** The act of listening wears the memory away, and what
  remains is the loop's skeleton plus silence.
- **Take:** the central mechanism of M3–M4 in one sentence: *every replay costs something*. A
  visual loop that loses high frequencies and gains holes each time it comes round. And the static
  single shot: the image hardly moves while the sound decays.
- **Avoid:** 9/11 imagery or any reference, smoke, and treating the process as a filter preset
  rather than a structure.
- **Serves:** M3, "We replay it.", and M4, "The story is rewritten / every time it is remembered."
- **Engine:** **REPLAY** a 4- or 8-bar control stream; on each pass `n`, raise the ERODE threshold
  `t_n = t_0 + 0.06·n`, raise the blur radius by 0.5 px, and drop Detail by 8 %. After about 12
  passes, only the loop's strongest shape remains. S. Deterministic, so it is recoverable from a
  snapshot plus pass count.

### 25. Alvin Lucier, *I Am Sitting in a Room* (1969)
- **Link:** https://www.moma.org/explore/inside_out/2015/01/20/collecting-alvin-luciers-i-am-sitting-in-a-room/ [search] ·
  https://rohandrape.net/ut/rttcc-text/Lucier1969a.pdf [search] ·
  http://www.lovely.com/albumnotes/notes1013cd.html [search]
- **Looks like (sounds like):** Lucier reads a text, records it, plays it back into the room and
  re-records it, 32 times (about 40 min). Speech dissolves into the room's resonant tones. The text
  ends by relating this to smoothing out his stutter.
- **Principle:** **generation loss converges on the medium's own character.** Iterated copying
  through a fixed filter leaves only the filter's resonances: the room remembers itself, not the
  voice.
- **Take:** the most precise model of M8: "Perhaps we remember / the last time / we remembered it."
  After enough recalls, what remains is the *shape of the one who remembers*, which is M9–M10:
  "the memory carries us", "a shape inside us". The engine's own "room" is its resonant form.
- **Avoid:** audible or visible speech, text-on-screen as a copy of the piece, and making the
  process visible as a technical demo.
- **Serves:** M8 and M10, "can still leave a shape / inside us."
- **Engine:** a feedback loop through a fixed kernel K: `f_{n+1} = normalize(K * f_n + ε·input)`.
  Choose K as the show's signature: a ring (Halo Ring's radius) or a 2–3 cycle/frame
  band-pass. Anything fed in converges over roughly 20–40 iterations (one per beat, or continuous
  with a small ε) toward the kernel's pattern. The show's "mark" is literally the engine's room
  tone. S–M, 2 passes (blur-difference band-pass) plus Trails.

### 26. The Caretaker (Leyland James Kirby), *Everywhere at the End of Time* (2016–19), with Ivan Seal's covers
- **Link:** https://www.thewire.co.uk/news/54374/final-release-for-the-caretaker-project-after-20-years [search] ·
  https://www.cambridge.org/core/journals/the-british-journal-of-psychiatry/article/an-empty-bliss-beyond-this-world-the-music-of-the-caretaker-as-a-representation-of-dementia-psychiatry-in-music/7CABE1610A4F5D45CD9B699E3126354A [search] ·
  https://www.rewirefestival.nl/feature/everywhere-at-the-end-of-time-20-years-of-the-caretaker [search]
- **Looks like (sounds like):** six albums of 1920s–30s ballroom 78s, looped and degraded stage by
  stage as a portrait of Alzheimer's progression. Clear, pleasant loops become confused, then pure
  noise. Ivan Seal's oil-painting covers show indistinct objects that can't quite be named.
- **Principle:** staged, **irreversible** degradation mapped to a clinical arc. Each stage echoes
  the previous.
- **Take:** **staging**: decay as a sequence of distinct states (not a smooth fade), each
  containing the ghost of the last. Seal's "unnameable objects" are an excellent model for our
  forms: almost recognisable, never nameable (which also keeps us away from faces).
- **Avoid:** dementia as spectacle. The piece's "horror" late stages became an internet meme; do
  not reference that register. Avoid ballroom nostalgia.
- **Serves:** M4 through M5, and M7, "Sometimes it feels / like hell."
- **Engine:** six snapshot states per decay section (A → D plus two extra), each morphing over 16
  bars, each carrying 15–25 % of the previous state's image via ACC at T ≈ 8 s. S.

### 27. Leyland Kirby, *Sadly, the Future Is No Longer What It Was* (2009)
- **Link:** https://ra.co/reviews/6970 [search] ·
  https://www.thelineofbestfit.com/reviews/albums/leyland-kirby-sadly-the-future-is-no-longer-what-it-was-21094 [search]
- **Looks like (sounds like):** three discs of long, blurred piano and drone ("When We Parted My
  Heart Wanted to Die", "Memories Live Longer Than Dreams"). Recorded in Berlin, 2008–09. A
  hauntological lament for a lost future.
- **Principle:** long-form, quiet, entirely self-made. Memory mourns *futures* as well as pasts.
- **Take:** duration and patience: 10+ minute states that change almost imperceptibly. It suits
  "mixed music" sections with sparse harmony (see RESEARCH-REPORT §B3).
- **Avoid:** hauntology's retro-kitsch (VHS logos, 1970s TV).
- **Serves:** M6, "Suspended between presence / and absence."
- **Engine:** minimum snapshot morph of 16 bars; Reactivity ≤ 0.3; CALM engaged. ≈0.

### 28. Gavin Bryars, *The Sinking of the Titanic* (1969→)
- **Link:** https://gavinbryars.com/post/the-sinking-of-the-titanic-compositions [search] ·
  https://forma.org.uk/projects/the-sinking-of-the-titanic [search] ·
  https://theartsdesk.com/new-music/gavin-bryars-sinking-titanic [search]
- **Looks like (sounds like):** the hymn "Autumn", reportedly played by the ship's band, heard as if
  it continued underwater. Bryars described "a slow descent to the ocean bed", with echo and
  deflection and "considerable high frequency reduction". Instrumentation is indeterminate. Obscure
  Records, 1975. Later versions were made with Philip Jeck and **Bill Morrison** (Forma project).
- **Principle:** a remembered melody filtered by an imagined medium: depth equals loss of high
  frequencies. The work is open and re-performed, never fixed.
- **Take:** **depth as low-pass**: the further back a memory, the fewer high frequencies (edges,
  detail, grain). This gives a concrete axis for "the edges begin to soften", and suggests a
  finale in which a known song sinks.
- **Avoid:** the Titanic, water and shipwreck imagery.
- **Serves:** M4, "The edges begin to soften.", and the finale.
- **Engine:** a "Depth" macro mapping to blur radius 0 → 6 px, Detail 1 → 0.3, grain cell size 1.2
  → 2.0 px, and halation spread +50 %. S.

### 29. Stephan Mathieu, *A Static Place* (12k, 2011)
- **Link:** https://12kmusic.bandcamp.com/album/a-static-place [search] ·
  https://brainwashed.com/index.php?option=com_content&view=article&id=25226:stephan-mathieu-qa-static-placeq-qremainq&catid=101&Itemid=855 [search]
- **Looks like (sounds like):** 78 rpm records from 1928–32 of early music (clavichord, viols,
  lute, hurdy-gurdy), played on two acoustic HMV 102 gramophones with cactus needles, re-miked and
  transformed by **spectral analysis and convolution** into sustained glowing fields.
- **Principle:** the whole chain of media (instrument → groove → needle → horn → room → mic →
  spectrum) is audible. Each step both preserves and transforms.
- **Take:** memory as a **chain of transfers**, each leaving its tint. Our film chain can be framed
  that way: grain, halation, weave and dust as the "transfers" a memory passes through, added one
  at a time over the show.
- **Avoid:** antique-object fetish (gramophone horns and so on).
- **Serves:** M8, "the last time / we remembered it."
- **Engine:** show-long staging of the film chain: section 1 clean, then add Weave, then Grain,
  then Halation, then Dust (persistent). Each "transfer" is added at a section boundary. ≈0.

### 30. Tim Hecker, *Ravedeath, 1972* (Kranky, 2011)
- **Link:** https://timhecker.bandcamp.com/album/ravedeath-1972 [search] ·
  https://ra.co/reviews/8510 [search]
- **Looks like (sounds like):** a pipe organ recorded live at Fríkirkjan church, Reykjavík, on 21
  July 2010, then saturated and crushed into distorted ambient. The cover is a re-photographed,
  deteriorated archive photo of the first MIT piano drop (1972).
- **Principle:** beauty via **saturation and distortion** rather than clarity. The degraded copy of
  an archive image as a sleeve.
- **Take:** loudness and fullness can come from *crushed* texture rather than brightness. A way to
  make the "hell" side of M7 intense without raising APL.
- **Avoid:** harsh digital clipping on screen; a bright image.
- **Serves:** M7, "Sometimes it feels / like hell."
- **Engine:** Crush (threshold) plus Smear at high values, with luma capped at 0.35. S.

### 31. Philip Jeck, *Vinyl Requiem* (1993, with Lol Sargent)
- **Link:** https://www.factmag.com/2013/10/23/waxwork-philip-jeck-on-his-landmark-turntable-piece-vinyl-requiem/ [search] ·
  http://www.thedoublenegative.co.uk/2014/10/surreal-and-optimistic-philip-jecks-vinyl-requiem-replayed/ [search]
- **Looks like:** an 8 × 6 m wall of 180 white-painted Dansette record players that doubles as a
  contoured projection screen, with 12 slide projectors and 2–3 film projectors. Sources disagree
  on the film-projector count. Performed live.
- **Principle:** a requiem for a medium, performed with the medium. The screen is made of the
  things that are dying.
- **Take:** a precedent for **live AV where the image surface and the sound source are the same
  body**, and for a single operator-orchestrator at the front.
- **Avoid:** turntable nostalgia imagery.
- **Serves:** M8, "The memory carries us."
- **Engine:** conceptual only (staging). ≈0.

---

## Group 5: Layering, averaging, copying (palimpsest)

### 32. Idris Khan, *every… William Turner postcard from Tate Britain* (2004) and *Struggling to Hear… After Ludwig van Beethoven Sonatas* (2005)
- **Link:** https://www.victoria-miro.com/artists/14-idris-khan/works/artworks4643/ [search] ·
  https://www.victoria-miro.com/artists/14-idris-khan/works/artworks6264 [search] ·
  https://fraenkelgallery.com/exhibitions/idris-khan [search]
- **Looks like:** every Turner postcard sold at Tate, digitally stacked into one misty image,
  "as though … conjuring up a William Turner painting from memory". The Beethoven work layers
  every page of the piano sonatas into one dark, vibrating field of staves (258 × 192 cm).
- **Principle:** **stacking collapses time into a single moment**. The composite is both more and
  less than any source. Lines reinforce where they agree and turn to fog where they differ.
- **Take:** **stacking our own frames**, so recurring structure darkens and firms up while the
  variable part fogs. The **Beethoven link** is worth flagging to strand E: "Because" is widely
  said to derive from a reversed "Moonlight" Sonata (lore until verified). A stacked-score image
  could be the finale's hidden rhyme. Use abstract lines only.
- **Avoid:** showing musical notation literally (on-screen staves are illustration) and
  Turner-orange sunsets.
- **Serves:** M4, "The story is rewritten / every time it is remembered.", and M9.
- **Engine:** **ACC** (T = 10–30 s) over a line-based scene (Fibers, Mesh Body), with
  multiply-darken instead of mean on the line channel so repeated positions deepen. S.

### 33. Jason Salavon, amalgamations: *The Class of 1988* (year not confirmed) and *Every Playboy Centerfold* series
- **Link:** http://salavon.com/work/Figure1EveryPlayboyCenterfold/ [search] ·
  https://aphelis.net/uniqueness-averageness-jason-salavons-every-playboy-centerfold-decades-2002/ [search]
- **Looks like:** shroud-like, blurred mean images built by per-pixel averaging of many photographs
  (a yearbook class, or a decade of centrefolds). The *Class of 1988* year is not confirmed in my
  results.
- **Principle:** the pixel mean reveals the *typical* and erases the individual.
- **Take:** only the **math**. Mean versus median versus max over a set of generated frames gives
  three different "memories" of the same section.
- **Avoid:** **the subject matter entirely**: faces and bodies, the objectified figure. This entry
  is included for the operation only.
- **Serves:** M6, "Suspended between presence / and absence."
- **Engine:** ACC gives the mean. For a "max memory", keep `max(acc*0.999, frame)`, which is the
  brightest thing that happened, fading slowly. S.

### 34. William Kentridge, charcoal "stone-age" animation (1989→; series names and dates not confirmed in my results)
- **Link:** https://harvardfilmarchive.org/programs/the-animated-films-of-william-kentridge [search] ·
  https://www.theartstory.org/artist/kentridge-william/ [search] ·
  https://www.royalacademy.org.uk/article/ra-magazine-william-kentridge [search]
- **Looks like:** grey charcoal drawings photographed, partly erased, redrawn and photographed
  again. Every erased state leaves a smudged ghost. Twenty to sixty drawings per film, and the
  final sheet is a palimpsest.
- **Principle:** "Erasure becomes a kind of pentimento… more ghostly in drawing." The **trace of
  the previous state is kept alive as much as the figure.**
- **Take:** **the single most usable mechanism for the ending.** Every movement leaves grey
  residue; the final frame is the accumulated sheet. That is "the quiet mark / left / by everything
  / that ever / moved us" made literal.
- **Avoid:** figurative drawing, political satire and a charcoal pastiche (paper texture overlay).
- **Serves:** M10, and M8, "every time we remember, / we leave a part of ourselves behind."
- **Engine:** **RESIDUE** buffer: `res = max(res * 0.9995, 0.25 * lum(frame))`, composited under
  the live scene in a grey 20–30 % below the palette's mid stop. Over a full section the residue
  becomes the ground. S.

### 35. Hito Steyerl, "In Defense of the Poor Image" (e-flux journal #10, November 2009)
- **Link:** https://www.e-flux.com/journal/10/61362/in-defense-of-the-poor-image [search]
  (WebFetch blocked)
- **Looks like (essay):** "The poor image is a copy in motion. Its quality is bad, its resolution
  substandard. As it accelerates, it deteriorates. It is a ghost of an image…" (quoted from the
  search result, not from the opened page).
- **Principle:** every circulation compresses. The copy's poverty records its history of use and
  love, and "perfect" images are suspect.
- **Take:** a **theory of generation loss as value**. Recall = re-encode: blockiness, banding and
  lost resolution are the image's biography.
- **Avoid:** glitch-art clichés (datamosh rainbows) and literal JPEG blocks as a style.
  RESEARCH-REPORT §C3 already covers datamosh.
- **Serves:** M4, "The story is rewritten / every time it is remembered."
- **Engine:** **RECALL**: on each recall event, downsample ×0.5, quantise to fewer levels (32 → 16
  → 8 over successive recalls), apply 8×8 block-mean at a 10–20 % mix, and write back. S, event
  only.

### 36. Gerhard Richter, blurred photo-paintings (e.g. *Onkel Rudi*, 1965)
- **Link:** https://engelsbergideas.com/reviews/gerhard-richter-between-past-and-present/ [search] ·
  https://unitlondon.com/2018-05-08/process-as-painting-gerhard-richter/ [search]
- **Looks like:** grey oil paintings copied from black-and-white snapshots, then dragged
  horizontally while wet. They look like newsprint, film grain or TV static. *Onkel Rudi* shows a
  smiling uncle in soldier's uniform (the source notes he was a Nazi); the companion *Tante Marianne* shows an aunt murdered under
  Aktion T4.
- **Principle:** the blur declines to depict directly. It is a *moral* distance from a painful past,
  not an aesthetic softening.
- **Take:** **directional (horizontal) blur as a sign of memory**. Our **Smear** control is already
  this; use it horizontally, as a slow drag rather than a glitch.
- **Avoid:** portraits and figures, and blur used as prettiness.
- **Serves:** M4, "It keeps the outline, / never the whole story."
- **Engine:** Smear in horizontal-only mode at 2–12 px, animated by a phrase LFO. S.

### 37. Roman Opałka, *1965/1 – ∞* (1965–2011)
- **Link:** https://www.dailyartmagazine.com/roman-opalka/ [search] ·
  https://lesoeuvres.pinaultcollection.com/en/artwork/opalka-19651 [search] ·
  http://www.opalka1965.com/en/index_en_v3.php?lang=en [search]
- **Looks like:** 233 canvases ("Details") of white numerals counted from 1 up; the last number
  was 5,607,249. From 1972 each canvas's grey ground got **1 % more white**, until from 2008 he
  painted white on white ("blanc mérité").
- **Principle:** a life measured by a monotonic process that makes the marks progressively less
  visible. The work *disappears into its own ground* as it nears the end.
- **Take:** a **show-long monotonic drift of the ground toward the marks**: in the finale, figure
  and ground converge. That is "where the moment ends, / and where we begin". It also mirrors the
  text's shrinking lines.
- **Avoid:** numerals or counters on screen, and white-on-white at high luminance (dancers). Do it
  at the dark end instead: black on black.
- **Serves:** M9, "Until we can no longer tell / where the moment ends, / and where we begin."
- **Engine:** "Convergence" uniform: `ground = mix(black, markColour, c)`, with `c` rising by 1 %
  per bar across the finale, so figure and ground meet at about 60–80 % luma difference reduction.
  ≈0.

---

## Group 6: Absence, shadow, light as material

### 38. Christian Boltanski, *Théâtre d'ombres* (1984–97) and *Monument* (1986)
- **Link:** https://www.jupiterartland.org/art/christian-boltanski-theatre-dombres/ [search] ·
  https://www.sfmoma.org/artwork/88.52.A-KK/ [search] ·
  https://rubellmuseum.org/2019-christian-boltanski [search]
- **Looks like:** *Théâtre d'ombres*: small cut-metal and cardboard puppets on wires, lit by a lamp
  or candle and blown by fans. Huge wavering shadows move over the walls, drifting in and out of
  focus. *Monument*: framed photos of children's faces in a pyramid, with bare bulbs and hanging
  wires, in dim light.
- **Principle:** the ephemeral as memorial. Cheap, fragile materials (tin, wire, bulbs) stand for
  fragile memory, "which only exist as long as one remembers them".
- **Take:** (a) **the shadow is larger and softer than its source, and it breathes**; the focus
  drifts. (b) **Bare bulbs with visible wires**: small points of warm light in a dark field, the
  "familiar light" as a constellation of single sources.
- **Avoid:** the skeleton and devil figures (the puppets are macabre; the owner hates skulls), the
  faces in *Monument* (forbidden), and funerary kitsch (candles).
- **Serves:** M7, "every memory becomes / the shadow of itself."
- **Engine:** rear projection gives us no real dancer shadows, so **generate the shadow**: render a
  scene's silhouette at quarter res, blur it 8–24 px with a slow focus LFO, scale it 1.5–3× from a
  low anchor point, and multiply it over a warm Tungsten field. The bulbs are 5–12 point lights
  with Halation, flickering at < 1 Hz. S.

### 39. Rachel Whiteread, *House* (1993)
- **Link:** https://www.tate.org.uk/art/artists/rachel-whiteread-2319/five-things-know-rachel-whiteread [search] ·
  https://www.artchive.com/artwork/house-rachel-whiteread-1993/ [search]
- **Looks like:** a full-size concrete cast of the interior of a Victorian terraced house at 193
  Grove Road, London. The walls were stripped away to leave the solid void, with wallpaper and
  fireplace impressions on its skin. Unveiled 25 October 1993, demolished 11 January 1994.
- **Principle:** **casting absence**. The empty space we lived in becomes the solid object, and the
  surface carries imprints of what touched it.
- **Take:** **the text's own final image**: "even what is gone / can still leave a shape / inside
  us". It suggests a figure/ground **inversion** at M8: what was empty becomes the lit form.
- **Avoid:** architecture and rooms on screen.
- **Serves:** M8 and M10, "can still leave a shape / inside us."
- **Engine:** invert the RESIDUE: display `(1 − live) * residue`, which lights where things *were*
  and are no longer. One pass. S. It pairs with scene 13 Negative's split.

### 40. Felix Gonzalez-Torres, *"Untitled" (Portrait of Ross in L.A.)* (1991)
- **Link:** https://en.wikipedia.org/wiki/%22Untitled%22_(Portrait_of_Ross_in_L.A.) [search] ·
  https://artincontext.org/untitled-portrait-of-ross-in-l-a/ [search]
- **Looks like:** a corner pile of wrapped candies, ideally 175 lb (Ross Laycock's healthy body
  weight). Visitors take pieces; the pile dwindles and is replenished.
- **Principle:** a portrait that is **shared out and diminished by the audience** and endlessly
  renewed. Loss and continuity in one mechanism.
- **Take:** **quantity as memory**. A finite particle count that the show "spends" (each event
  removes particles) and that is replenished only at the end. "We leave a part of ourselves behind"
  becomes a visible budget.
- **Avoid:** literalism (candy, a body count).
- **Serves:** M8, "every time we remember, / we leave a part of ourselves behind."
- **Engine:** a global particle budget (e.g. 200k → 20k across the show). Each triggered event
  subtracts 0.5–2 %; the finale restores it over 60 s. S (it saves GPU as the show goes on).

### 41. Chiharu Shiota, *The Key in the Hand* (Venice Biennale, Japan Pavilion, 2015)
- **Link:** https://www.designboom.com/art/chiharu-shiota-venice-art-biennale-the-key-in-the-hand-05-06-2015/ [search] ·
  https://2015.veneziabiennale-japanpavilion.jp/en/project/ [search]
- **Looks like:** a ceiling-to-floor web of red yarn holding about 180,000 keys over two wooden
  boats. The red is the colour of the blood in our bodies, "therefore life itself".
- **Principle:** memories as **threads connecting objects**, dense enough to become a space.
- **Take:** the **dense line web as a volume**, close to The Noise Diary's plexus and our Mesh Body.
  A strong single-colour choice: red thread on dark (Blood palette).
- **Avoid:** keys, boats, literal thread, and red everywhere. Use Blood at most in one section.
- **Serves:** M6, "It follows us quietly", and M8.
- **Engine:** Mesh Body (scene 09) with the connection distance raised so the plexus fills 60–80 %
  of the frame at low alpha (0.05–0.12 per line). Palette Blood. M.

### 42. Uta Barth, *Ground* (1992–97) and *Field* series
- **Link:** https://www.getty.edu/art/exhibitions/barth/explore.html [search] ·
  https://aperture.org/editorial/what-uta-barths-images-tell-us-about-the-limits-and-possibilities-of-sight/ [search]
- **Looks like:** photographs focused on the empty space in front of the camera, so the interiors
  and backgrounds are soft fields of colour. No subject.
- **Principle:** the subject has been removed and the out-of-focus *backdrop* is what remains. The
  eye searches for a focus that never comes.
- **Take:** **the backdrop without the portrait**. Our screen is literally the backdrop to the
  dancers, and it should often be an unfocused field that the eye searches, leaving the focus to
  the bodies.
- **Avoid:** nothing major; just avoid "stock photo bokeh".
- **Serves:** M6, "Suspended between presence / and absence."
- **Engine:** a scene at Space macro = max blur (8–20 px), Detail 0.2. It is a readable "rest"
  state for dance-heavy passages. S.

### 43. Anthony McCall, *Line Describing a Cone* (1973)
- **Link:** https://www.tate.org.uk/research/tate-papers/08/anthony-mccall-line-describing-a-cone [search] ·
  https://whitney.org/collection/works/15286 [search]
- **Looks like:** 16 mm, black and white, silent, 30 min, in a hazed room. A white line slowly
  draws a circle; in the haze the beam becomes a solid hollow cone that viewers walk through.
- **Principle:** "solid light". The projection *beam*, not the screen, is the work.
- **Take:** the circle drawn slowly over 30 min is a perfect **long arc** for one section, and
  close to our Halo Ring. **Rear projection means the audience never sees our beam**, so the cone
  cannot be physical. Take the **slow drawing of one circle** and a haze *in* the image instead.
- **Avoid:** proposing stage haze to "see beams". It won't work with rear projection, and haze in
  front of the screen only lowers the image's contrast. That is a lighting-designer conversation
  (strand C).
- **Serves:** M3, "We give it names" (the line describes), and M10, one line only.
- **Engine:** Halo Ring (scene 11) with arc length = section progress 0 → 360°, line width 1–2 px,
  Trails on. S.

### 44. Ann Veronica Janssens, *yellowbluepink* (Wellcome Collection, 15 Oct 2015 – 3 Jan 2016); Olafur Eliasson, *The weather project* (Tate Modern, 2003); James Turrell, *Aten Reign* (Guggenheim, 2013)
- **Link:** https://wellcomecollection.org/exhibitions/ann-veronica-janssens--yellowbluepink [search] ·
  https://olafureliasson.net/artwork/the-weather-project-2003/ [search] ·
  https://www.tate.org.uk/press/press-releases/unilever-series-olafur-eliasson-weather-project [search] ·
  https://www.guggenheim.org/press-release/turrellrelease [search]
- **Looks like:** Janssens: a gallery filled with dense coloured mist, visible for a few inches,
  moving from pink through yellow to pale blue as you walk, with shadows appearing and vanishing.
  Eliasson: a semicircle of about 200 mono-frequency lamps doubled by a mirror ceiling into a sun,
  in mist; the lamps reduce everything to yellow and black. Turrell: *Aten Reign* fills the
  rotunda with shifting LED rings around daylight. His Ganzfeld pieces flood the whole visual field
  with one colour.
- **Principle:** **light as material, not as depiction**. Perception becomes the subject ("seeing
  yourself seeing"), and colour hangs in the air.
- **Take:** the **cloud** without an image: a whole section can be a field of slightly varying
  colour and density, with no objects. Eliasson's two-tone restriction (yellow/black) proves a
  monochrome palette can be overwhelming.
- **Avoid:** full-field bright colour. Ganzfeld at stage scale would drown the dancers' silhouettes
  and could disorient. Avoid rainbow gradients, the "Instagram fog room", and a sun disc.
- **Serves:** M6, "Memory is a cloud. / Always changing. / Always drifting."
- **Engine:** Signal Fog (scene 08) at Detail 0.1 and Space max, luma capped at 0.25, with a very
  slow palette gradient (one 3-stop palette sweep per 32 bars). S.

---

## Top 8 for this show

Ranked by how directly each one gives the engine a **mechanism** (not a mood) for a movement of
the text, and by fit with the owner's taste.

1. **Kentridge, erasure animation (#34)** → **RESIDUE**. Each movement leaves grey residue, and
   the final frame is the sheet everything left behind. This *is* M10 ("The quiet mark / left / by
   everything / that ever / moved us") and M8. Cheap (S), fully generative, and suits a mostly-black
   frame.
2. **Lucier, *I Am Sitting in a Room* (#25)** → an iterated fixed-kernel loop that converges on the
   engine's own "room tone". It is the literal model of "we remember / the last time / we remembered
   it", and it ends on "a shape inside us". It gives the show a signature form.
3. **Basinski, *Disintegration Loops* (#24)** → **REPLAY + ERODE per pass**: every replay costs
   something. It covers M3–M4 and is deterministic, so it is recoverable from snapshot plus pass
   count.
4. **Sugimoto, *Theaters* (#1)** → **ACC** long exposure: a whole section summed into one light,
   seen only at the edges. It gives M4's frame and the "accumulated night" layer for the finale.
5. **Paulstretch (#23)** → **PHASE** scramble: keep the light, lose the outline. It is the visual
   grammar of the slowed "Because" finale and the bridge to M9.
6. **Marker, *Sans Soleil*'s Zone (#4)** → the artistic justification for M5 ("It creates") and for
   a generative-only instrument: a machine that remembers by transforming.
7. **Morrison, *Decasia* / *Light Is Calling* (#9–10)** → **ERODE** through Morphogen blooms in
   the Nitrate palette. Decay is active and can turn radiant (paradise) or consuming (hell), which
   covers M4 and M7. Keep its principle and drop its footage.
8. **Viola, *The Reflecting Pool* / *Emergence* (#19, #21)** → **split time** (one layer frozen,
   one alive) and 7× slow rises. It covers M2–M3 and gives the finale's tempo.

Just outside: Opałka (#37, figure and ground converge for M9), Conrad (#16, show-long palette
ageing), Whiteread (#39, inverted residue at M8) and Paik (#14, persistent dust). All are nearly
free and should be adopted as show-wide rules rather than features.

---

## What this means for our show

1. **Build three buffers, not ten effects.** ACC (RGBA16F, half res, 2 × 4.1 MB), RESIDUE (R16F,
   1 texture) and REPLAY (control stream, ≈0) cover almost every entry above. Total added cost is
   S–M: about 3–4 extra passes at half res. Keep RGBA16F for ACC, because RGBA8 cannot hold
   exposures longer than about 4 s at 60 fps.
2. **Replay the control stream, not pixels.** "We replay it" should re-run the last 4–8 bars of
   OSC and macro data with drift per pass (Basinski). This is deterministic, needs no pixel ring
   buffer (about 31 MB even at quarter res), and a known snapshot plus pass count restores the
   exact state.
3. **Every recall must cost something, and the cost must be visible.** Per pass: ERODE threshold
   +0.06, blur +0.5 px, Detail −8 %. After about 12 passes only the strongest shape remains. That
   is M4 as a process rather than a filter.
4. **Make Dust and residue cumulative across the whole night** (Paik, Kentridge). Specks and edge
   traces write to RESIDUE with d ≈ 0.99995 (half-life about 3.9 min), so each section inherits the
   last one's marks. The final image is then the night's trace, not a fade to black, which confirms
   brief §3's "residue, not emptiness" hypothesis.
5. **Age the palette show-long** (Conrad, Stephan Mathieu). One "Age" uniform, 0 → 1 across the
   Arrangement, shifts the white point from about #F2F2F0 toward #EDE6D6 and lifts blacks by 1–2 %.
   Add the film-chain "transfers" one per section (Weave → Grain → Halation → persistent Dust), so
   the chain itself tells the story of copying.
6. **Use Nitrate once, as an event.** Nitrate decay (Morrison) belongs to M4 and M7 as blooms
   grown by Morphogen, seeded by rare kicks. Never use it as a constant "old film" overlay. That is
   the sepia kitsch we must avoid (see RESEARCH-REPORT's anti-patterns).
7. **Treat the pivot (M5) as the same process with its sign flipped** (Metzger's
   auto-destructive/auto-creative pairing). The field that eroded now grows. The audience should
   feel the *same* material turn from loss to invention.
8. **Generate the shadows.** Rear projection gives no dancer shadows and no visible beam. For M7,
   render silhouettes blurred 8–24 px and scaled 1.5–3× from a low anchor (Boltanski), with a slow
   focus LFO. Tell the lighting designer (strand C) that front haze will only grey the image.
9. **Finale grammar ("Because", slowed):** PHASE scramble from 0 to about 0.8 across the 5 min, so
   the image keeps its light and loses its outlines (Paulstretch). ACC at T = 60–120 s, edge-weighted
   (Sugimoto). Figure and ground converge by 1 % per bar (Opałka), at the dark end, never at full
   white. Before designing further, find out whether the slowdown is a resample (lower pitch) or a
   stretch (same pitch).
10. **Use one "familiar light".** A small warm point source with Halation (Boltanski bulbs, Dean's
    lighthouse) returns rarely and unexpectedly (p ≈ 0.02 per bar, only at high presence) through
    the Glimpse event. Let it be the recurring motif that M6 names ("A familiar light.").
11. **Freeze more than you move** (La Jetée, Viola, Gordon). Add a "Hold" control that stops the
    scene clock while grain (24 fps) and weave keep running, plus step-time at 1.5–2 Hz. Real
    continuous motion becomes the rare event, once per section.
12. **Keep the fixed-kernel signature** (Lucier). Pick one "room" (for example Halo Ring's radius
    or a 2–3 cycle/frame band-pass) and let every section's feedback converge toward it. By the end,
    all the night's material looks like it passed through the same space: "a shape inside us".
13. **Hard exclusions drawn from this research.** No eye moments (La Jetée), no faces (Boltanski's
    *Monument*, Salavon), no skeletal shadow puppets, no flicker fields (Sharits: keep the 3/s cap),
    no found footage (Morrison, Tscherkassky: principles only), no staves or numerals on screen
    (Khan, Opałka), and no sun discs or full-field Ganzfeld brightness (Eliasson, Turrell).
14. **Pass these to other strands:** the Beethoven stacking (Khan) and the forward/reverse pairing
    (Gordon, 2008) go to strand E, with the "reversed Moonlight Sonata" story still unverified
    here. Rear projection plus haze goes to strand C. The one-phrase-late image (Frampton) goes to
    strand D as a mapping option: the image answers the music of 4–8 bars ago.

---

## Caveats

- **Nothing was opened.** WebFetch was blocked on en.wikipedia.org, e-flux.com and getty.edu; I
  stopped there as instructed. All facts come from search-engine summaries of the listed URLs
  **[search]**, and every link must be clicked by a human before it is cited as "opened" in
  CONCEPT.md.
- **Not confirmed in my results:** the release year of *Sans Soleil* (given as 1983 from general
  knowledge; verify), the year of Salavon's *The Class of 1988*, the length of *La Jetée*, and the
  exact number of film projectors in *Vinyl Requiem* (sources say 2 or 3). The *Emergence* slowdown
  ("seven times slower") is taken from the Getty summary; 210/30 = 7, so it assumes 30 fps
  playback. The "U Smile" numbers (800 % vs 3:17 → 35+ min) don't match a literal 8×.
- **Inferences** (marked in the text as recipes) include every engine translation, all numeric
  parameters, and the Oppenheim–Lim reading of Paulstretch for images. They are my proposals, not
  measured.
- My web-search budget ran out before I could check more sources (such as a Basinski loop-length
  technical source).

---

## Sources

All [search]: seen in WebSearch results, not opened. Three WebFetch attempts were blocked
(en.wikipedia.org/wiki/The_Disintegration_Loops, e-flux.com Steyerl essay, getty.edu *Emergence*).

**Film and video**
- https://rhizomes.net/issue8/tryon.htm
- https://mubi.com/en/notebook/posts/chris-marker-s-imaginary-japan
- https://www.modwiggler.com/forum/viewtopic.php?t=69702
- https://en.wikipedia.org/wiki/Sans_Soleil
- https://sf-encyclopedia.com/entry/jetee_la
- https://aspectfilmjournal.web.unc.edu/2023/09/mah-an-examination-of-time-medium-and-the-moving-image-in-la-jetee/
- https://www.moma.org/calendar/events/512
- https://bombmagazine.org/articles/bill-morrisons-decasia-the-state-of-decay/
- https://www.loc.gov/static/programs/national-film-preservation-board/documents/decasia.eagan.pdf
- https://en.wikipedia.org/wiki/Decasia
- https://michaelgordonmusic.com/music/light-is-calling/
- https://366weirdmovies.com/bill-morrisons-light-is-calling-2004-and-just-ancient-loops-2012/
- https://www.sensesofcinema.com/2006/the-films-of-bill-morrison/bill-morrison-interview/
- https://wexarts.org/blog/bill-morrison-dawson-city-frozen-time
- https://www.tcm.com/articles/1434281/dawson-city-frozen-time
- https://en.wikipedia.org/wiki/Dawson_City:_Frozen_Time
- https://fraenkelgallery.com/portfolios/hiroshi-sugimoto-theaters
- https://publicdelivery.org/hiroshi-sugimoto-theaters/
- https://hyperallergic.com/hiroshi-sugimotos-otherworldly-photographs-of-movie-theaters/
- https://fraenkelgallery.com/portfolios/hiroshi-sugimoto-seascapes
- https://www.artic.edu/articles/1078/a-voyage-on-hiroshi-sugimotos-seascapes
- https://www.moma.org/collection/works/120413
- https://www.guggenheim-bilbao.eus/en/learn/schools/teachers-guides/reflecting-pool-1977-79
- https://www.eai.org/titles/the-reflecting-pool-collected-work-1977-80
- https://www.moma.org/collection/works/118326
- https://www.eai.org/titles/the-passing
- https://www.mplus.org.hk/en/collection/objects/the-passing-202164/
- https://www.getty.edu/art/collection/object/108VM9
- https://www.artway.eu/posts/bill-viola-emergence
- https://www.bfi.org.uk/film/97341aa7-c937-57bf-9011-99bb084472b9/mirror
- https://www.scenebygreen.com/2023/12/04/mirror-1975/
- https://www.artforum.com/features/j-hoberman-on-stan-brakhages-mothlight-200839/
- https://filmsinreview.lib.byu.edu/film_review/mothlight/
- https://filmalchemist.de/films/instabile_materie.html
- https://www.sensesofcinema.com/2004/cteq/jurgen_reble/
- https://expcinema.org/site/en/blu-ray/j%C3%BCrgen-reble-materia-obscura
- https://expcinema.org/site/en/wiki/work/outer-space
- https://www.filmcomment.com/interview-peter-tscherkassky/
- https://www.metmuseum.org/art/collection/search/623288
- https://www.artic.edu/artworks/214310/shutter-interface
- https://artlyst.com/features/tacita-dean-disappearance-at-sea-1996-significant-works-sue-hubbard/
- https://www.mariangoodman.com/artists/39-tacita-dean/works/39446/
- https://mubi.com/en/us/films/reminiscences-of-a-journey-to-lithuania
- https://www.slantmagazine.com/film/reminiscences-of-a-journey-to-lithuania/
- https://gagosian.com/news/museum-exhibitions/douglas-gordon-24-hour-psycho/
- https://www.presenhuber.com/exhibitions/douglas-gordon3
- https://bombmagazine.org/articles/2006/10/01/hollis-framptons-nostalgia/
- https://www.screeningthepast.com/issue-22-reviews/hollis-frampton-nostalgia/
- https://www.eai.org/titles/zen-for-film
- https://www.moma.org/collection/works/128108
- http://www.medienkunstnetz.de/works/zen-for-film/
- https://www.moma.org/collection/works/108075
- https://buffaloakg.org/blog/exhibition-spotlight-tony-conrads-yellow-movies
- https://greenenaftaligallery.com/exhibitions/tony-conrad2
- https://www.lifa-research.org/en/artworks/liquid-crystal-environment/
- https://apollo-magazine.com/gustav-metzger-1926-2017/
- https://harvardfilmarchive.org/programs/the-animated-films-of-william-kentridge
- https://www.theartstory.org/artist/kentridge-william/
- https://www.royalacademy.org.uk/article/ra-magazine-william-kentridge
- https://www.e-flux.com/journal/10/61362/in-defense-of-the-poor-image

**Sound**
- https://www.soundonsound.com/techniques/classic-tracks-william-basinski-disintegration-loops
- https://median.newmediacaucus.org/the_aesthetics_of_erasure/the-disintegration-loops/
- https://www.texasmonthly.com/arts-entertainment/william-basinski-september-11-disintegration-loops/
- https://www.moma.org/explore/inside_out/2015/01/20/collecting-alvin-luciers-i-am-sitting-in-a-room/
- https://rohandrape.net/ut/rttcc-text/Lucier1969a.pdf
- http://www.lovely.com/albumnotes/notes1013cd.html
- https://www.thewire.co.uk/news/54374/final-release-for-the-caretaker-project-after-20-years
- https://www.cambridge.org/core/journals/the-british-journal-of-psychiatry/article/an-empty-bliss-beyond-this-world-the-music-of-the-caretaker-as-a-representation-of-dementia-psychiatry-in-music/7CABE1610A4F5D45CD9B699E3126354A
- https://www.rewirefestival.nl/feature/everywhere-at-the-end-of-time-20-years-of-the-caretaker
- https://ra.co/reviews/6970
- https://www.thelineofbestfit.com/reviews/albums/leyland-kirby-sadly-the-future-is-no-longer-what-it-was-21094
- https://gavinbryars.com/post/the-sinking-of-the-titanic-compositions
- https://forma.org.uk/projects/the-sinking-of-the-titanic
- https://theartsdesk.com/new-music/gavin-bryars-sinking-titanic
- https://12kmusic.bandcamp.com/album/a-static-place
- https://brainwashed.com/index.php?option=com_content&view=article&id=25226:stephan-mathieu-qa-static-placeq-qremainq&catid=101&Itemid=855
- https://timhecker.bandcamp.com/album/ravedeath-1972
- https://ra.co/reviews/8510
- https://www.factmag.com/2013/10/23/waxwork-philip-jeck-on-his-landmark-turntable-piece-vinyl-requiem/
- http://www.thedoublenegative.co.uk/2014/10/surreal-and-optimistic-philip-jecks-vinyl-requiem-replayed/
- https://www.npr.org/sections/therecord/2010/08/18/129283985/the-art-of-a-time-stretch
- https://www.musicradar.com/news/tech/justin-bieber-slowed-down-800-is-ambient-masterpiece-272104
- https://polarity.me/posts/articles/2026-07-07-paulxstretch-paulstretch-explained/
- https://ui.adsabs.harvard.edu/abs/2022ASAJ..151A.158M/abstract
- https://ui.adsabs.harvard.edu/abs/1981IEEEP..69..529O/abstract

**Visual art**
- https://engelsbergideas.com/reviews/gerhard-richter-between-past-and-present/
- https://unitlondon.com/2018-05-08/process-as-painting-gerhard-richter/
- https://www.jupiterartland.org/art/christian-boltanski-theatre-dombres/
- https://www.sfmoma.org/artwork/88.52.A-KK/
- https://rubellmuseum.org/2019-christian-boltanski
- https://www.victoria-miro.com/artists/14-idris-khan/works/artworks4643/
- https://www.victoria-miro.com/artists/14-idris-khan/works/artworks6264
- https://fraenkelgallery.com/exhibitions/idris-khan
- http://salavon.com/work/Figure1EveryPlayboyCenterfold/
- https://aphelis.net/uniqueness-averageness-jason-salavons-every-playboy-centerfold-decades-2002/
- https://www.dailyartmagazine.com/roman-opalka/
- https://lesoeuvres.pinaultcollection.com/en/artwork/opalka-19651
- http://www.opalka1965.com/en/index_en_v3.php?lang=en
- https://www.tate.org.uk/art/artists/rachel-whiteread-2319/five-things-know-rachel-whiteread
- https://www.artchive.com/artwork/house-rachel-whiteread-1993/
- https://en.wikipedia.org/wiki/%22Untitled%22_(Portrait_of_Ross_in_L.A.)
- https://artincontext.org/untitled-portrait-of-ross-in-l-a/
- https://www.designboom.com/art/chiharu-shiota-venice-art-biennale-the-key-in-the-hand-05-06-2015/
- https://2015.veneziabiennale-japanpavilion.jp/en/project/
- https://www.getty.edu/art/exhibitions/barth/explore.html
- https://aperture.org/editorial/what-uta-barths-images-tell-us-about-the-limits-and-possibilities-of-sight/
- https://www.tate.org.uk/research/tate-papers/08/anthony-mccall-line-describing-a-cone
- https://whitney.org/collection/works/15286
- https://wellcomecollection.org/exhibitions/ann-veronica-janssens--yellowbluepink
- https://olafureliasson.net/artwork/the-weather-project-2003/
- https://www.tate.org.uk/press/press-releases/unilever-series-olafur-eliasson-weather-project
- https://www.guggenheim.org/press-release/turrellrelease
