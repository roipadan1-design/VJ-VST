# Handoff prompt for Claude Design

**How to use this.**
1. Attach these files:
   - `wireframe-basic.svg`, `wireframe-expanded.svg`, `wireframe-source-and-states.svg`
   - `current-ui.png` (as the "before")
   - one or two art stills for mood: `../before-it-disappears/stills/audit/show-variants.jpg` and `../images/negative-storm-blood.png`
2. Paste everything between the lines.
3. The full reasoning is in `DESIGN-DOSSIER.md` if the tool can read more.

**Owner decisions, 2026-09-26 (already applied to the prompt below; they override the wireframes where the two differ):**
- PLAY shows **all 8 knobs**, not 4.
- Scenes are **cued, then fired with GO** (theatre style). A click only cues; there is no automatic switch and no "cut now" button.
- This is a **pilot of a product that will be sold**, so first-run and empty states matter.
- **No product name yet.** Use a neutral placeholder wordmark and do no logo or naming work.
- The owner's laptop is **1920 x 1080 at 125 % = 1536 x 864 logical px**.

---

You are designing the user interface of an audio-visual instrument whose final name is not decided yet: show it as a neutral placeholder wordmark "[NAME]" and do no logo work. Today it is called "VJ Analyzer". It is a VST3 audio plug-in for Ableton Live 10 on a Windows laptop. This first user is the pilot, and the product will then be sold, so a new user must also find their way without a manual.

**What the product does.**
- It listens to the music on a track and drives a separate full-screen visuals window, the "engine". The engine renders 16 abstract generative scenes: film grain, red monochrome, fine plexus lines, dust, noise.
- The user is one artist-performer. He plays music and visuals alone, live, often in the dark, sometimes for a contemporary dance show.
- He is not an engineer. His complaint about the current UI: *"I can't understand what does what, there are many knobs that aren't always needed, I feel lost."*
- Your job: a finished, professional product UI with its own visual language that makes him never feel lost.

## Structure: three views of one plug-in window

1. **PLAY (760 x 460 px at 100 %)**, the performance view and the default. It holds only what is touched during a show:
   - **Header:** placeholder wordmark "[NAME]" + one small signal-red square; role chip "LEAD · MIX · Master"; engine status pill (dot, "60 fps · 124.0 BPM · 17.3"); an EDIT ▸ toggle; **SAFE** (tungsten outline: fades to a designated safe scene, calms reactions) and **BLACKOUT** (red outline), grouped in the top-right corner, far from HIT.
   - **NOW card:**
     - a 200 x 112 still of the live scene with hairline crop marks at the corners
     - the scene name (20 px), a one-line description and a small LIVE tag
     - a **cue line**: "NEXT ▸ HALO RING · press GO". After GO it reads "switching on the next beat" with a 4-segment beat countdown ("next bar" for scenes that wait for the bar).
     - a large **GO** button (tungsten when something is cued, dim otherwise). GO is the performer's main scene action and is MIDI-mappable. There is no "cut now" button.
   - **Scene grid:** 4 x 4 tiles (107 x 30), each a small thumbnail + name. There is an "IMG" badge on scenes that show the user's own image or video. The live tile has a signal-red left bar; the cued tile has a tungsten dashed outline and a "CUED" badge; a tile that was fired and is waiting for the beat is filled tungsten. Include ◂ ▸ (they move the cue, not the live scene) and the caption "click = cue · GO = switch · double-click = cue + GO".
     - The 16 scenes: Hot Blobs, Dot Relief, One Bit, Corridor, Fibers, Terminal, Ink, Signal Fog, Mesh Body, Morphogen, Halo Ring, Emergence, Negative, Media Negative, Media Lines, Membrane.
   - **Eight knobs**, the 8 macros, in one row (about 56 px each): INTENSITY, SPEED (centre shows "x1.00" or "FROZEN"), FORM, SCALE, IMPACT, ERODE, GLIDE, DETAIL. Each has a sub-line saying what it does in the current scene (e.g. "here: body → dust"). Give them visual grouping (what it looks like: Intensity, Form, Scale, Erode, Detail · how it moves: Speed, Glide · how hard hits land: Impact) without splitting them into separate panels. They map 1:1 to the 8 knobs of a MIDI controller, in this order.
   - **LISTEN TO:** five lamp-toggles (KICK, SNARE, HAT, BASS, LEVEL). The lamp flashes on each hit; an off toggle has a dashed outline and a hollow lamp. Caption: "what the picture follows".
   - **Actions:** HIT (the biggest button, 130 x 92), CALM ("reactions fade out · 1 bar"), FREEZE ("motion stops, grain lives").
   - **MOMENTS:** four stored states A-D (letter + optional name, empty = dashed), "recall = morph over [1 BAR ▾]", and SAVE….
   - **SOUND strip:** one thin level bar + LIVE / SILENT.
2. **EDIT (1180 x 460)**: the PLAY view stays *exactly* where it is, and a 420 px drawer opens on the right.
   - The drawer has a vertical tab rail with hairline icons + labels: MOTION, LISTEN, LOOK, COLOUR, MEDIA, SETUP.
   - It has a persistent **info bar** at the bottom that explains whatever the mouse is over (name, value, one sentence, and the parameter's name in Live).
   - Tab contents:
     - **MOTION:** Camera drift, Music push, plus Freeze / Reverse / Tempo-lock toggles. (Speed and Glide are on PLAY with the other macros, so the drawer needs no SHAPE tab.)
     - **LISTEN:** Impact, Decay, Listen amount, Calm, plus the five listen toggles with activity meters.
     - **LOOK:** FILM (Grain, Halation, Gate weave, Dust, Film base), DIGITAL (Crush, Trails, Glitch, Smear, Glyphs) and CUTS (Auto-cut as a segmented control OFF / 16 / 8 / 4 / 2 / 1 / KICK; Flash; Shots; HUD). A value of zero shows "off".
     - **COLOUR:** a grid of 14 palette cards, each with 3 stripes + a name. The palettes are Blood, Ember, Bone, Ice, Acid, Violet, Rust, Nitrate, Cyanotype, Tungsten, Ash, Custom, Scene Colors and Split. Add three custom colour chips (Shadow / Mid / Light).
     - **MEDIA:** 8 slot cards (thumbnail, file name, size, loading bar), Load / Clear, "Use in all IMG scenes", Loop / Ping-pong, Clip length (Free, 1 beat … 8 bars), and a line "Shown by: …".
     - **SETUP:** Role, Lead toggle, Detect, Trim, Level Auto / Fixed, Visual lookahead (with a warning), full signal diagnostics, engine location, UI size, Show tips, and a list of every parameter with its name in Live.
3. **SOURCE (360 x 220)**: shown by every non-lead instance (e.g. the one on the kick track). It contains:
   - a role segmented control
   - a big lamp for its role's hits, with "last hit 0.4 s"
   - a band meter with a visible threshold line
   - Detect and Trim knobs, and a MIDI-in lamp
   - the footer "Feeding the lead instance on 'Master'. Scenes, knobs and look are played there."
   - a MAKE LEAD button

The wireframes attached show all three views and the key states. Treat their **layout and content as the spec** and their styling as a starting point to refine.

## Visual language: "darkroom instrument"

The UI should feel like a precise measuring instrument lit by a darkroom safelight: warm near-black, paper-bone type, hairlines, crop marks, crosshairs, and exactly one red for "live". The picture is loud, so the UI is quiet. The identity comes from the art (attached stills), not from plug-in clichés.

**Colour tokens (dark only):**

| Token | Hex | Use |
|---|---|---|
| ink-0 | #0C0A09 | window background (warm, never pure black) |
| ink-1 | #151210 | panels, drawer, tiles |
| ink-2 | #1E1A17 | hover, toggled-on fill |
| line | #3A322C | hairlines, knob tracks, idle outlines |
| bone | #ECE4D6 | primary text, value arcs, active outlines |
| ash | #A89E90 | labels, secondary text |
| dust | #8A7F73 | hints, disabled |
| signal | #F0503F | **LIVE / OUTPUT only**: live tile, LIVE tag, engine dot, hit lamps, BLACKOUT |
| blood | #B3160E | large red fills behind bone text only |
| halation | #F2937F | modulation rings, meters, morph progress |
| tungsten | #E9A450 | **PENDING only**: cued scene, save armed, loading, fading |
| remote | #6FC3D6 | **moved by MIDI / automation** tick, nothing else |

**Colour rules:**
- Red is rationed to three or fewer elements in normal play.
- Colour never carries state alone: always add a word, a dashed outline or a hollow shape.
- Body text contrast is 4.5:1 or more.
- Scene thumbnails are greyscale stills tinted by the current palette, so the palette is the only colourful thing on screen.

**Type:**
- **IBM Plex Sans Condensed** for labels and names. Caps labels are SemiBold, 9-10 px, tracked +1.2 px. Sentences are Regular, 10-11 px. The scene name is 20 px SemiBold.
- **IBM Plex Mono** for every number (BPM, fps, x1.00, dB, bars).
- Minimum 9 px at 100 %.

**Shapes:**
- 2 px corner radius (film-frame corners), 1 px hairlines. No drop shadows, glows, gradients, pills or nested rounded cards.
- **Knobs:** a 270° track in `line`, a 3 px value arc in `bone` from 7 o'clock, and an outer 2 px `halation` arc showing how much the sound is moving it right now. The centre disc shows the real unit. No pointer, no glow. A small cyan tick appears for 1 s when MIDI or automation moved the knob.
- **Icons:** 1 px hairline on a 16 px grid, built from the HUD vocabulary (crosshair, small square, circle, a line joining them), always with a text label.
- **Texture:** optional static grain at 3-4 % on the background only.

**Motion:**
- Nothing moves when idle except meters and lamps.
- State changes last 150 ms or less.
- Hit lamps decay over 200 ms; meters release over 300 ms.
- The cue countdown steps per beat.
- BLACKOUT = a steady 2 px red frame around the window + "OUTPUT DARK". **Never blink**, because it is used in dark rooms.

**Do:**
- words and real units
- CUED / LIVE / OUTPUT DARK / SILENT spelled out
- BLACKOUT far from HIT
- one fixed info bar for help

**Don't:**
- neon mint / magenta / violet
- navy
- faces, eyes or masks in any icon or illustration (the artist dislikes them; even two round holes read as eyes)
- light theme
- bare 0-100 numbers

## Screens to deliver

1. PLAY, connected: Mesh Body LIVE, Halo Ring CUED (waiting for GO).
1b. PLAY, GO pressed: Halo Ring switching, 2 of 4 beats elapsed.
2. PLAY, engine not running: the NOW card becomes a 3-step checklist (Start engine [red button] / Play something in Live / Pick a scene); tiles are dimmed.
3. PLAY, starting engine (tungsten progress hairline; "Locate VJ Engine.exe" link after 8 s).
4. PLAY, "ENGINE LOST" (the engine crashed or was closed mid-show: red engine dot, "Relaunch" button; the controls keep working).
5. PLAY, connected but silent ("SILENT · play something in Live", DEMO button).
6. PLAY, BLACKOUT active.
7. PLAY, SAVE armed (the Moments row outlined in tungsten: "Click a moment to save · Esc cancels").
8. PLAY, moment B recalling (morph progress bar in the tile).
9. PLAY, a media scene live (NOW card: "slot 3 · field_02.mp4 · 1280x720 · shown by IMG scenes").
10. EDIT, each tab: MOTION, LISTEN, LOOK, COLOUR, MEDIA, SETUP.
11. SOURCE: normal, no audio, and a MIDI note arriving.
12. PLAY at 75 %, 100 % and 150 %.

## Components to deliver (with all states)

| Component | States |
|---|---|
| Header | lead, source |
| Role chip | n/a |
| Engine pill | off, starting, connected, lost (relaunch), low-fps "76 %" in tungsten, blackout |
| Button | idle, hover, pressed, on, disabled, primary, destructive idle / active |
| Macro knob 56 px and small knob 36 px | idle, hover, dragging with value bubble, modulated, remote tick, off, disabled |
| Segmented control | n/a |
| Lamp-toggle | on idle, on hit, off |
| Scene tile | idle, hover, LIVE, CUED, switching (after GO), IMG, no engine, loading |
| GO button | nothing cued (dim), cued (tungsten), pressed / switching |
| NOW card | all states above |
| Moment tile | empty, stored, active, recalling, armed target |
| Tab rail item | n/a |
| Info bar | n/a |
| Palette card | n/a |
| Media slot card | empty, loading, image, clip, active, error |
| Meters | level, band + threshold, spectrum |
| Inline message | e.g. "Switched to Media Negative so your image shows" |
| Parameter list row | n/a |

## Constraints (this will be built in JUCE 8 C++, not on the web)

- **Cheap:** solid fills, 1 px lines, arcs, embedded-font text, cached bitmaps (thumbnails, grain tile, icons).
- **Avoid:** blur, shadows, glows, animated gradients, anything that needs full-window repaints at 30 fps. The UI shares an integrated Intel GPU with the visuals engine.
- **Scaling:** fixed steps of 75 / 100 / 125 / 150 %. Design on a 4 px grid at 100 % and deliver 1x and 2x assets. Hairlines must stay crisp at every step.
- The plug-in opens as a **floating window** in Live (it cannot dock in Live's device strip). The owner's laptop is 1920 x 1080 at 125 % Windows scaling = **1536 x 864 logical px**, and with the taskbar and title bar about 800 px of height are usable. PLAY must fit comfortably beside Live there, and still work on a 1280 x 720-logical screen.
- **Hit targets:** at least 24 x 24 px at 100 %.

## Deliverables

- High-fidelity frames for every screen above.
- A component sheet with states.
- The token table (colours, type, spacing, radii, stroke widths) as a design-token list.
- Icon set (6 tab icons + small glyphs: arrows, crop mark, lamp).
- Knob and tile redlines (sizes, stroke widths, spacing) precise enough for a developer to redraw them with paths.
- One short paragraph per screen explaining the design choice.
