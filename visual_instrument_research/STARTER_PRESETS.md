# Twelve starter presets — authored instrument designs

**Status:** original design proposals, not recreated vendor presets or measured GPU demonstrations. All twelve target the **external Windows OpenGL renderer**, so they do not require Live 11/12 or Max 9. A 3.3-capable rendering path is sufficient for the proposed first implementations; compute is optional. Only **Pulse Halo** has a complete JSON/shader example in this bundle. The remaining entries are implementation specifications, not finished shader files.

**Common performance language:** four knobs are always **Intensity, Motion, Color, Space**. In the custom controller profile these are CC20–23. The pad bank uses notes36–43; Bank A holds presets1–8 and Bank B holds9–12. Bank selection is initially in the UI. Note46 is the separate manual user-hit action, note47 palette advance, note48 freeze, note49 blackout toggle and note51 panic blackout. These are controller-source messages, not MIDI notes on the musical kick source. Played musical notes arrive through the separate role-aware bridge.

**Common mapping defaults:** continuous energy is normalized and gated; raw dB remains available for diagnosis. Use roughly 30–80 ms attack and 150–300 ms release unless an entry specifies otherwise. Hits trigger envelopes, never random one-frame parameter spikes. Hue/palette transitions take 250–1000 ms. Limit dominant audio responses to one or two perceptual gestures. Idle motion remains attractive at zero input.

**Budget convention:** scene-only 1080p GPU allocations below are **targets**, excluding a shared 1–2 ms finishing/output allocation. They are not measured performance claims. Two expensive scenes may need reduced quality during a crossfade. 4K is qualified separately; reduce field/simulation resolution before sacrificing stable event timing.

## 1. Pulse Halo — luminous geometry with negative space

**Look:** one crisp, slightly deformable luminous ring on a near-black blue field. Bass gives a brief expansion/brightening rather than shaking the entire image. Slow breathing continues between hits. An understated halo occupies much less of the frame than the dark background.

**Technique:** aspect-correct 2D SDF, derivative antialiasing, bounded sinusoidal radial deformation, linear emission plus a small local glow; shared bloom/tone map. No video, history texture or mesh is required. The supplied shader is original and has not been GPU-compiled here.

| Parameter | Range; authored default |
|---|---|
| Radius | 0.10–0.90; 0.45 in normalized half-height coordinates |
| Thickness | 0.002–0.050; 0.012 |
| Emission | 0.1–8.0; 1.4 |
| Local glow | 0–1; 0.30 |
| Palette position | 0–1 cyclic; 0.55 |
| Drift extent | 0–1; 0.12 |
| Beat pulse | 0–1; 0, primarily envelope-controlled |
| Shape warp | 0–0.20; 0.025 |

**Four macros:** Intensity → emission + glow; Motion → drift extent + warp; Color → palette position; Space → ring radius. Center macro contributions on their authored defaults, as in the JSON, to preserve the initial appearance.

**Audio:** bass activity contributes only 6% of normalized radius range using a 0.75 power curve and 35/180 ms smoothing. A kick-role hit drives a 4 ms attack / 180 ms decay pulse. High activity adds up to 10% of local glow with 30/250 ms smoothing. An eight-beat bipolar LFO adds a small radius breath independently of audio.

**MIDI:** Bank A pad1/note36 selects it; CC20–23 are the macros. Manual user-hit starts the same pulse; played kick-role MIDI is preferred over inferred bass hits when assigned. Palette advance is a separate slow action, not every drum hit.

**Budget/test:** ≤1.5 ms scene target. Check 16:9 and 4:3 geometry, macro extremes, black/silent input and a dense kick roll. A failed bloom pass must still leave a legible ring. Worked files: `presets/01_pulse_halo.json`, `shaders/pulse_halo.fs`.

## 2. Horizon Lines — a restrained perspective field

**Look:** evenly spaced horizontal contour lines converge toward a distant horizon. One broad wave travels through them; a kick gently lifts the foreground and a snare briefly accents a subset of lines. Use two related cool colors with one warm highlight, not a full-spectrum rainbow.

**Technique:** a single fragment pass using periodic distance-to-line in a perspective-warped coordinate system, with a smoothly integrated scroll phase. Optional small noise displacement is secondary. This can precede any true 3D camera implementation.

| Parameter | Range; default |
|---|---|
| Line count | 8–80; 28, rounded at a controlled boundary |
| Line width | 0.001–0.02; 0.004 |
| Perspective strength | 0–0.9; 0.55 |
| Wave amplitude | 0–0.25; 0.04 |
| Scroll speed | −0.5–0.5 normalized units/s; 0.06 |
| Horizon height | −0.5–0.5; 0.05 |
| Highlight level | 0–4; 0.7 |
| Palette position | 0–1; 0.30 |

**Four macros:** Intensity → highlight + wave amplitude; Motion → integrated scroll speed; Color → curated palette blend; Space → perspective + horizon position. Speed changes integrate a phase; do not multiply an ever-growing TIME by a changing speed and create jumps.

**Audio:** low activity bends the foreground with 50/220 ms smoothing. Kick → 5/180 ms wave impulse. Snare-role hits accent alternate lines with a 3/120 ms envelope. Centroid only nudges the highlight color over 200 ms. Keep line count manual or quantized to avoid rapid topology popping.

**MIDI:** Bank A pad2/note37. User-hit injects one wave. A sustained played MIDI note may hold a highlighted band; release it through a short fade. CC20–23 retain common macro meanings.

**Budget/test:** ≤1.5 ms scene. Thin lines must not shimmer badly when nearly horizontal, at low motion or during a resolution change. Silence leaves slow travel and a stable horizon.

## 3. Mosaic Tiles — discrete rhythm with controlled color

**Look:** a sparse grid of rounded luminous tiles on dark negative space. A few tiles respond to each hit, with different rhythmic roles occupying different regions. The frame reads as an intentional layout rather than a random flashing checkerboard.

**Technique:** repeated 2D rounded-box SDFs; a small CPU event-state array or low-resolution state texture stores tile envelopes/seed. Deterministic selection ensures reproducible replay. No full-resolution feedback buffer is needed.

| Parameter | Range; default |
|---|---|
| Columns | 3–16; 8 |
| Rows | 2–10; 5 |
| Fill ratio | 0.20–0.85; 0.50 |
| Corner radius | 0–0.45 tile units; 0.16 |
| Active density | 0.05–0.60; 0.18 |
| Decay | 80–800 ms; 220 |
| Emission | 0.2–5; 1.3 |
| Palette position | 0–1; 0.10 |

**Four macros:** Intensity → emission + active density; Motion → stepping/decay balance; Color → limited palette; Space → fill ratio + grid density through curated discrete states. Changes of grid dimensions reset or remap event state deliberately.

**Audio:** kick activates one central/low tile group; snare picks a contrasting side group; hats supply very small edge accents with probability bounded by intensity. Continuous level controls only the base luminance at 80/250 ms. Envelope decay carries the rhythm after each event.

**MIDI:** Bank A pad3/note38. Played notes can select tile index by pitch class or a configured range; velocity controls accent strength. Cap simultaneous active tiles, and distinguish note-on velocity0 from a new hit. User-hit advances a deterministic tile pattern.

**Budget/test:** ≤1.5 ms scene. Replay the same seed/events twice and compare identical selected tiles. A controller flood cannot light every tile without a configured density limit. Default output does not include rapid full-frame white flashes.

## 4. Polar Petals — low-cost kaleidoscopic structure

**Look:** six to twelve curved petals radiate from the center, with a clear dark core and soft highlights. A slow rotation provides continuity; a kick opens the structure briefly. Palette changes alter material identity over a phrase rather than spinning hue continuously.

**Technique:** polar coordinate folding, a small SDF/noise field inside one wedge and smooth radial masks. A derivative-aware seam treatment prevents bright cracks. Symmetry is a design constraint, not a substitute for composition.

| Parameter | Range; default |
|---|---|
| Petal count | 3–16; 8 |
| Inner radius | 0.05–0.5; 0.18 |
| Outer radius | 0.3–1.1; 0.75 |
| Fold curvature | 0–1; 0.35 |
| Rotation speed | −0.5–0.5 rad/s; 0.05 |
| Edge softness | 0.001–0.08; 0.012 |
| Emission | 0.2–5; 1.4 |
| Palette position | 0–1; 0.45 |

**Four macros:** Intensity → emission/opening; Motion → integrated rotation + modest curvature motion; Color → palette blend; Space → inner/outer radii. Petal count changes only on a requested beat/bar boundary or a deliberate hard gesture.

**Audio:** kick → radial opening envelope, 4/220 ms. Mid activity → curvature, 60/240 ms. Flatness → a small texture roughness blend, 200/400 ms, not shape destabilization. Hats may produce a weak rim highlight.

**MIDI:** Bank A pad4/note39; user-hit opens the petals. Selected played notes may choose among preapproved symmetry counts at the next beat. The ordinary macros never create unbounded segmentation.

**Budget/test:** ≤2 ms scene. Test angular seams, zero/near-zero radius, extreme aspect ratios and symmetry changes while crossfading. No NaNs from undefined angle handling at the center.

## 5. Domain Silk — flowing color without a fluid solver

**Look:** broad translucent-looking folds of ink or fabric, with shallow apparent lighting and a slow directional flow. Keep one smooth color family and use the accent in narrow creases. The design should read as a material surface, not undifferentiated noise.

**Technique:** 3–5 octave fBm, two-component domain warp and gradient-derived fake lighting. Run at half resolution first and reconstruct smoothly; detail is band-limited rather than simply sharpened. Use a licensed noise implementation or original code. [[G15]](sources.md#g15)

| Parameter | Range; default |
|---|---|
| Field scale | 0.5–8; 2.2 |
| Warp amount | 0–2.5; 0.8 |
| Flow speed | 0–0.4 units/s; 0.05 |
| Octaves | 2–5; 4 |
| Ridge contrast | 0.2–3; 1.1 |
| Light depth | 0–1; 0.35 |
| Accent fraction | 0.02–0.4; 0.12 |
| Palette position | 0–1; 0.65 |

**Four macros:** Intensity → contrast/accent fraction; Motion → phase velocity + bounded warp; Color → palette; Space → field scale/light depth. Octaves belong to a quality/detail control, not a fast audio destination.

**Audio:** bass activity gently expands field scale, 80/300 ms. Centroid moves lighting/accent emphasis over 200 ms. A snare adds a 5/250 ms ridge accent. Avoid coordinate reseeding on every hit; it destroys the perceived material continuity.

**MIDI:** Bank A pad5/note40. User-hit adds a ridge accent. Reseed is an explicit modifier/action with a crossfade, never a default pad side effect. CC20–23 are common macros.

**Budget/test:** ≤3 ms scene allocation at its selected internal scale. Compare aliasing and motion at 30/60 fps; clamp derivative lighting. It must remain composed at zero warp and maximum permitted warp.

## 6. Feedback Ribbons — memory as musical phrasing

**Look:** a few luminous ribbons spiral outward, leaving slowly fading traces. New kick events add clean lines; old marks turn into a coherent tunnel-like history. Low-frequency motion is stable, with only occasional palette accents.

**Technique:** persistent ping-pong color target, inverse zoom/rotation sample and new 2D ribbon injection. Express decay in seconds. Separate simulation/history from the final bloom so each frame does not recursively bloom its own already bloomed output.

| Parameter | Range; default |
|---|---|
| Trail lifetime | 0.15–4 s; 1.2 |
| Zoom rate | −0.10–0.15 /s; 0.035 |
| Rotation rate | −0.6–0.6 rad/s; 0.08 |
| Injection width | 0.002–0.05; 0.01 |
| Injection gain | 0–4; 1.2 |
| Ribbon count | 1–8; 3 |
| History distortion | 0–0.10; 0.012 |
| Palette position | 0–1; 0.8 |

**Four macros:** Intensity → injection gain + width; Motion → zoom/rotation; Color → injection palette; Space → trail lifetime + distortion. Keep gain and decay combinations within a tested stable range.

**Audio:** kick injects a new ribbon with velocity/strength weighting. Bass activity subtly changes zoom with 60/250 ms smoothing. Hats add faint edge marks only above a gate. Long-term energy trend can shorten trails during dense drops so the image does not become a solid mass.

**MIDI:** Bank A pad6/note41. User-hit injects a ribbon; freeze holds history without blocking input queues; reseed/reset explicitly clears it. Preset entry resets history unless compatible retention was requested.

**Budget/test:** ≤2.5 ms scene. One-minute replay at 30 versus60 fps should have similar trail duration. Verify bounded luminance, different read/write textures and correct reset after resize. A silent tail must dissipate rather than become permanent glare.

## 7. Spectral Torus — geometric depth with modest resources

**Look:** a wire torus or rounded lattice floats on black. Low-frequency motion gently deforms its body; higher spectral detail creates sparse surface accents. Use a controlled camera orbit and thin, well-spaced lines. This is inspired by the economy of the inspected T3X2R geometry, not a copy of its mesh or shader. [[T05]](sources.md#t05)

**Technique:** parametric torus mesh, smoothed spectrum texture sampled in vertex displacement, line/point rendering and a depth-aware fade. Add a companion vertex-shader path to the host or a small native mesh stage; the prototype’s fragment-only ISF subset is insufficient for that exact implementation.

| Parameter | Range; default |
|---|---|
| Major radius | 0.3–2.0; 1.0 |
| Minor radius | 0.05–0.8; 0.28 |
| Displacement | 0–0.35; 0.06 |
| Grid density | 16–128 samples per main direction; 64 |
| Orbit speed | −0.3–0.3 rad/s; 0.04 |
| Line emphasis | 0–1; 0.7 |
| Point fraction | 0–0.3; 0.04 |
| Palette position | 0–1; 0.52 |

**Four macros:** Intensity → displacement/emission; Motion → camera/object orbit; Color → lattice/accent palette; Space → torus proportions/camera distance. Grid density is a quality setting, not an audio-controlled random topology change.

**Audio:** spectrum32 after 40/180 ms smoothing deforms vertices. Kick adds a 5/160 ms uniform expansion. Centroid influences accent position slowly. Cap total displacement to prevent self-intersection or inversion at aggressive inputs.

**MIDI:** Bank A pad7/note42. Played pitch can choose an accent sector or object orientation; user-hit expands the form. MIDI velocity controls pulse, not the camera’s absolute position.

**Budget/test:** ≤2.5 ms scene. Test line aliasing, camera clipping, zero spectrum, singular torus proportions and all-points-on overdraw. It should still look dimensional without heavy bloom.

## 8. Orbit Particles — bounded, purposeful motion

**Look:** a sparse cloud of particles follows a few curved orbital bands, with short luminous accents on hits. Most particles remain dim; a smaller subset carries bright rhythmic motion. Avoid the default “screensaver snow” aesthetic through constrained spawn geometry.

**Technique:** texture-state or transform-feedback particles on a 3.3-class path, deterministic respawn and additive billboards. Start around16k particles and scale only after profiling. Separate simulation state, draw size and trail rendering.

| Parameter | Range; default |
|---|---|
| Particle count | 2k–64k; 16k |
| Orbit radius | 0.1–2; 0.7 |
| Flow speed | 0–2; 0.25 |
| Drag | 0.1–4 /s; 0.8 |
| Particle size | 0.5–6 pixels at1080p; 1.5 |
| Lifetime | 0.3–6 s; 2.5 |
| Burst fraction | 0–0.15; 0.03 |
| Palette position | 0–1; 0.2 |

**Four macros:** Intensity → burst fraction/brightness; Motion → flow speed/drag; Color → palette; Space → radius/size balance. Count is a quality parameter and never allocated in response to audio.

**Audio:** kick injects momentum into a bounded fraction of existing particles. Sustained bass alters orbital radius with 80/300 ms smoothing. High activity brightens a small subset, 30/150 ms. One source should not modulate both every particle’s size and count on every frame.

**MIDI:** Bank A pad8/note43. Played notes select one of a few emitters; velocity sets impulse. User-hit creates a bounded burst. Note-offs do not erase the entire cloud.

**Budget/test:** ≤3 ms scene. Stress simultaneous note-ons and a60-second dense drum roll; count/memory remain bounded. Measure large-billboard overdraw separately from simulation time. Rendering a black scene must not leave stale particles after reset.

## 9. Curl Filaments — coherent strands rather than noise

**Look:** fine colored strands drift through a soft flow, gathering and separating without obvious random jitter. Keep the bright density near the center and allow the edges to dissolve. Individual lines should be readable at the intended viewing distance.

**Technique:** particles or short line segments advected through a curl field, with bounded trail history. Analytic noise derivatives reduce field-evaluation cost when available; an inexpensive 2D field can be more effective than premature full3D turbulence. [[G15]](sources.md#g15)

| Parameter | Range; default |
|---|---|
| Filament count | 1k–16k; 4k |
| Curl scale | 0.3–6; 1.5 |
| Flow speed | 0–1.5; 0.18 |
| Segment history | 2–24; 8 |
| Width | 0.5–3 pixels; 1 |
| Center attraction | 0–1; 0.15 |
| Turbulence | 0–1; 0.25 |
| Palette position | 0–1; 0.7 |

**Four macros:** Intensity → visible strand fraction/emission; Motion → flow/turbulence; Color → gradient; Space → field scale/center attraction. History length is not changed every hit because reallocating/reindexing trails can create hitches.

**Audio:** mid activity slowly changes turbulence, 100/350 ms. Kick injects a small radial impulse with a 5/200 ms envelope. Flatness mixes fine roughness over300 ms. A bass-only passage should not suddenly produce high-frequency visual chatter.

**MIDI:** Bank B pad1/note36. User-hit creates a radial disturbance. Sustained notes can attract a selected strand group; on release the attraction fades, not snaps.

**Budget/test:** ≤4 ms scene with bounded count/history. Test NaNs, escaped particles, line count explosion and frame-rate-independent advection. Confirm legibility without increasing every strand to additive white.

## 10. SDF Beacon — one sculptural hero object

**Look:** a single softly joined geometric object hovers in a dark space. Its silhouette is more important than surface noise. Bass changes its proportions; a brief rim accent marks the kick. The camera moves slowly and predictably.

**Technique:** bounded SDF sphere tracing with a few primitives, smooth union, one light and inexpensive gradient normal. Start at half resolution,32–64 steps, no recursive reflection and no expensive soft-shadow march. A scene can look sculptural without a path tracer.

| Parameter | Range; default |
|---|---|
| Primitive separation | 0–1; 0.25 |
| Smooth-union width | 0.02–0.5; 0.15 |
| Twist | −1–1 rad/unit; 0.12 |
| Object scale | 0.3–1.5; 0.8 |
| Orbit speed | −0.2–0.2 rad/s; 0.03 |
| Roughness-style shading mix | 0–1; 0.45 |
| Rim emission | 0–4; 0.6 |
| Palette position | 0–1; 0.1 |

**Four macros:** Intensity → rim/deformation; Motion → orbit/twist speed; Color → material/rim palette; Space → scale/separation. Warp parameters are bounded to preserve conservative marching behavior.

**Audio:** bass moves separation with 60/250 ms smoothing; kick gives a 4/180 ms rim pulse. Centroid changes light/material emphasis over250 ms. Never make max ray steps a rhythmic parameter.

**MIDI:** Bank B pad2/note37. Played notes select one of a few curated primitive arrangements with a short morph only where correspondence is defined. User-hit accents the rim.

**Budget/test:** ≤5 ms scene at qualified internal resolution. Test camera inside/near geometry, grazing rays, misses, maximum twist, iteration cap and finite output. Quality reduction must preserve silhouette before adding surface detail.

## 11. Morphogen Garden — slow organic change, fast musical accents

**Look:** organic islands and labyrinthine contours evolve within a composed silhouette. Colors are restrained and edges receive most of the light. The simulation changes slowly; drum hits add local disturbances rather than attempting to make the entire chemical field follow each kick.

**Technique:** two-channel reaction–diffusion state at256² or512², fixed solver steps, contour shading and optional polar symmetry. The technique is documented by Photism’s Morphogen page, but this design does not copy its field, parameter presets or artwork. [[P04]](sources.md#p04)

| Parameter | Range; default |
|---|---|
| Morphology path | 0–1; 0.35, mapped to curated stable feed/kill pairs |
| Simulation rate | 0.25–2 relative; 0.8 |
| Seed radius | 0.005–0.08 domain units; 0.025 |
| Injection strength | 0–1; 0.2 |
| Contour width | 0.005–0.15 concentration units; 0.035 |
| Symmetry | 1–8; 4, controlled discrete choice |
| Edge emission | 0–4; 1 |
| Palette position | 0–1; 0.6 |

**Four macros:** Intensity → edge emission/injection; Motion → bounded solver rate; Color → palette; Space → morphology/symmetry through curated safe paths. Raw feed/kill remain in an advanced inspector, not an unrestricted front-panel pair.

**Audio:** level/energy trend shifts morphology very slowly,0.5–3 s. Snare/user hits inject a small seeded disturbance, with event rate limited. Kick primarily accents contour light via a 4/200 ms envelope; it does not require destabilizing the solver.

**MIDI:** Bank B pad3/note38. Selected notes place seeds in predefined regions; velocity controls injection. Freeze pauses solver advancement but keeps output/controls responsive. Reset is explicit and preserves blackout state.

**Budget/test:** ≤4 ms scene at256² initial state. Verify solver stability, concentration bounds, reproducible seeds, resize policy and recovery from a long stall. Validate each curated morphology path, including interpolation, rather than assuming stable endpoints imply a stable journey.

## 12. Ink Flow — ambitious final expansion

**Look:** two or three coherent colored ink streams move through a dark field, forming broad curls and occasional fine filaments. New musical events inject color at predictable positions. Use a restrained dye palette and avoid turning the image into uniformly saturated smoke.

**Technique:** low-resolution2D velocity/pressure solver, dye advection and bounded force injection. Start with256² velocity,512² dye and12–20 pressure iterations. A particle overlay is optional and must earn its GPU budget. GPU Gems and the MIT WebGL fluid code are suitable implementation studies. [[G04]](sources.md#g04) [[G14]](sources.md#g14)

| Parameter | Range; default |
|---|---|
| Flow force | 0–2; 0.35 |
| Vorticity emphasis | 0–1; 0.25 |
| Velocity dissipation | 0.1–3 /s; 0.7 |
| Dye lifetime | 0.3–5 s; 1.8 |
| Injection radius | 0.01–0.15 domain units; 0.045 |
| Injection amount | 0–1; 0.25 |
| Emitter separation | 0–0.8; 0.35 |
| Palette position | 0–1; 0.4 |

**Four macros:** Intensity → injection/contrast; Motion → force/dissipation; Color → emitter palette; Space → emitter separation/radius. Pressure iterations and grid resolution are quality controls, not user gestures.

**Audio:** kick injects force from the bottom emitter; snare adds dye from a lateral emitter; hats add occasional small highlights above a gate. Continuous low activity changes flow force with 100/350 ms smoothing. Source separation comes from the DAW roles, not an assumed perfect master-bus drum classifier.

**MIDI:** Bank B pad4/note39. Played notes address a small emitter set; velocity scales bounded injection. User-hit injects a balanced two-emitter gesture. A note flood cannot increase iteration count or allocate new emitters.

**Budget/test:** ≤5 ms scene at the initial grids. Test pressure-solver convergence, divergence after force injection, boundaries, long silence, dense events, half-speed rendering and receiver disconnect. Do not ship until it remains stable in the same soak test as the simple presets.

## Recommended content order

Ship1–3 first. Add4 and6 for variety, then7 when mesh/vertex support is ready. Add5/8/9 as performance capacity permits. Treat10–12 as separately gated advanced content. The product’s core identity should not depend on the most expensive preset working on an unspecified GPU.
