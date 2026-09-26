# Control map v2: fewer controls, same power

## תקציר (לבעלים)

- **המטרה:** פחות דברים על המסך, בלי לאבד שום יכולת. מסך ההופעה יורד מ-77 פקדים ל-34, ומ-29 כפתורים מסתובבים ל-8.
- **מסך ההופעה (PLAY) בגודל 760x480**, כך שהוא נכנס ליד Live בלפטופ שלך. יש בו רק:
  - רשת הסצנות, עם NEXT ו-GO כמו היום
  - 8 הכפתורים, עם המילה של הסצנה מתחת לכל אחד
  - HIT ו-BLACKOUT
  - 4 "רגעים" (A עד D)
  - מצב המנוע
- **14 כפתורי ה-LOOK הפכו לבחירה אחת:** Clean / Film / Worn / Broken / Print / Data, ועוד פס אחד של "כמה חזק". הכפתורים הבודדים עדיין קיימים, במגירת EDIT.
- **שבעה פקדי תגובה הפכו לשורה אחת:** STILL / BREATHE / PULSE / PUNCH.
  - STILL הוא ה-CALM הישן.
  - חמש נורות מתחת לשורה מראות למה התמונה מקשיבה עכשיו.
- **IMPACT הוא עכשיו "כמה התמונה מגיבה למוזיקה" בכלל:** באפס היא מתעלמת מהמוזיקה, ב-40% היא כמו שתוכננה.
- **צבע ו-Look יושבים יחד בכרטיס אחד, STYLE.**
- **סנאפשוטים נקראים עכשיו "רגעים":** לחיצה על רגע ריק שומרת אותו, ולחיצה על רגע שמור מחזירה אותו. כפתור STORE נעלם.
- **תמונה משלך:** גוררים קובץ לחלון, או לוחצים LOAD. אם הסצנה לא יודעת להציג תמונות, הפלאגין עובר לבד לסצנה שיודעת, ואומר את זה.
- **EDIT פותח מגירה מימין.** החלון מתרחב ל-1180 ולא נהיה גבוה יותר. יש במגירה 6 לשוניות: LOOK, COLOUR, REACT, MOTION, MEDIA, SETUP.
- **בערוצים האחרים (קיק, סנר וכו')** מופיע חלון קטן, SOURCE.
  - התפקיד נקבע לבד לפי שם הערוץ.
  - רק עותק אחד "מוביל", והבחירה אוטומטית. זה סוף ה"מלחמה" של Send macros.
- **שום דבר לא נשבר:** כל 57 הפרמטרים נשארים, כך שמיפויי MIDI ואוטומציה ממשיכים לעבוד. נוספים רק 3 פרמטרים: Look, Look Amount, React Style.
- **החלטות שלך:** השמות של ה-Looks ושל שורת התגובה, והאם הכפתור החמישי יקרא IMPACT או REACT.

---

*26 September 2026. Design and documentation only: no source code was changed and nothing was committed.*

*Wireframes: [wireframe-play-v2.svg](wireframe-play-v2.svg) (PLAY, with component IDs in cyan) and [wireframe-edit-v2.svg](wireframe-edit-v2.svg) (EDIT with every tab, plus the SOURCE view). Both were generated from the same coordinates as the tables below. This document supersedes the layout parts of [DESIGN-DOSSIER.md](DESIGN-DOSSIER.md) §4.3-4.5 and the wireframe-basic / expanded / source SVGs. It keeps the dossier's visual language (§5).*

*Inputs:*
- *the owner's feedback of 26 Sept*
- *DESIGN-DOSSIER.md, CLAUDE-DESIGN-PROMPT.md, COMPETITOR-WORKFLOW-RESEARCH.md, AUDIO-SOURCE-ROUTING-RESEARCH.md and REFERENCE-ZWOBOT-SHOWCASE.md (recommendation 10)*
- *`plugin/Source/PluginEditor.cpp/.h` and `PluginProcessor.cpp/.h`*

*REACTION-DESIGN.md did not exist while this was written. §8 lists the assumptions to check against it.*

## Contents

1. [Merge analysis](#1-merge-analysis)
2. [PLAY view](#2-play-view-760-x-480)
3. [EDIT view](#3-edit-view-1180-x-480)
4. [SOURCE view and the automatic lead](#4-source-view-and-the-automatic-lead)
5. [Host parameters](#5-host-parameters)
6. [Cognitive walkthrough](#6-cognitive-walkthrough-before-vs-after)
7. [Visual language and JUCE notes](#7-visual-language-and-juce-notes)
8. [Alignment with REACTION-DESIGN.md, and open questions](#8-alignment-with-reaction-designmd-and-open-questions)

---

## 0. The idea in five lines

1. **Three depths, one home per control.**
   - **PLAY** holds what a hand touches during a song.
   - **EDIT** holds what you set per song or per show.
   - **SOURCE** is what a drum-track instance needs.
2. **Merge by intent, not by widget.** Many controls express one intent: "how much does the music move it", "what does it look like", "keep this". Each intent gets one front control, and the detail controls stay behind it in EDIT.
3. **Merged controls write the old parameters.** Every existing ID keeps working for automation and MIDI maps. New front controls either write the old values (look preset, react style) or scale them on the way to the engine (look amount, impact).
4. **Things the software can know, it decides:**
   - the role, from the track name
   - the lead, by election
   - the input calibration
   - the switch to a media scene when you load an image

   The owner overrides them in one place.
5. **Say what happens.** A single info line under PLAY explains whatever the mouse is over, and says what the plug-in is waiting for.

**Numbers:**

| | Today (one screen) | PLAY v2 |
|---|---|---|
| Clickable controls | 77 (scene list counted once) | 34 |
| Rotary knobs | 29 | 8 (+1 slider) |
| EDIT (one tab at a time) | n/a | at most 21 controls per tab |

---

## 1. Merge analysis

### 1.1 Fate of every current control

Fates:
- **PLAY:** on the performance view.
- **EDIT › tab:** in the drawer.
- **MERGED:** driven by a new front control.
- **AUTO:** decided by the software, with an override.
- **REMOVED:** gone.

All host IDs stay.

| # | Current control (host ID) | Fate | Details |
|---|---|---|---|
| 1 | Role buttons MIX…TEXTURE (`role`) | **AUTO** + EDIT › SETUP, SOURCE view | Set from the Live track name (§4.3). Manual override in SETUP / SOURCE. No longer on PLAY, which removes the clash with the REACT TO chips (dossier confusion 2) |
| 2 | OPEN / SHOW ENGINE | **MERGED** into the engine pill (H2) | The pill *is* the button. Off = red START ENGINE; on = status, and a click shows the window |
| 3 | FULLSCREEN | PLAY (H3), icon beside the pill | |
| 4 | Status pill | PLAY (H2) | Adds bar.beat. The LIVE / SILENT words move to the info line |
| 5 | Spectrum (32 bands) | EDIT › SETUP (diagnostics) | |
| 6 | Level bar + dBFS | **MERGED** into the LEVEL lamp (R2) | The number is in SETUP |
| 7 | BASS / MID / HIGH bars | EDIT › SETUP | |
| 8 | Lamps KICK/LOW, MID, HIGH, MIDI | **MERGED** into the PLAY lamp strip (R2) | MIDI lamp: SOURCE view + SETUP |
| 9 | Status line (LIVE / CALIBRATING / SILENT, BPM, bar) | **MERGED** into the pill (BPM, bar) + info line (SILENT, CALIBRATING) | |
| 10 | Hit Sens (`sensitivity`) | **AUTO** (calibration) + EDIT › SETUP "Detect" + SOURCE view | The knob centre reads "auto" until touched |
| 11 | Trim (`trim`) | **AUTO** + EDIT › SETUP + SOURCE view | |
| 12 | Lookahead (`lookahead`) | EDIT › SETUP | With a red warning line. Never automatic (the plug-in cannot know whether instruments are played live) |
| 13 | Adaptive / Locked (`normalizer`) | **AUTO** (Adaptive) + EDIT › SETUP "Level: Auto / Fixed" | |
| 14 | Send macros (`sendControls`) | **AUTO** (lead election, §4.2) + EDIT › SETUP "Can lead" | Kills the once-a-second fight. The parameter now means "may be elected" |
| 15 | Intensity (`macro1`) | PLAY (K1) | Scene sub-label |
| 16 | Form (`macro3`) | PLAY (K3) | Scene sub-label |
| 17 | Scale (`macro4`) | PLAY (K4) | Scene sub-label |
| 18 | Erode (`macro6`) | PLAY (K6) | Scene sub-label |
| 19 | Detail (`macro8`) | PLAY (K8) | Scene sub-label |
| 20 | Speed (`macro2`) | PLAY (K2) | Centre "x1.00" / "FROZEN"; sub-label = live speed with push |
| 21 | Glide (`macro7`) | PLAY (K7) | Centre = bars |
| 22 | Drift (`drift`) | EDIT › MOTION "Camera drift" | |
| 23 | Push (`push`) | **MERGED** into React Style (the character sets it) + EDIT › REACT "Music push" | |
| 24 | FREEZE (`freeze`) | PLAY (K9), a small toggle in the MOTION group | Also in EDIT › MOTION |
| 25 | REVERSE (`reverse`) | EDIT › MOTION | |
| 26 | SYNC (`sync`) | EDIT › MOTION "Tempo-lock" | |
| 27 | Impact (`macro5`) | PLAY (K5); **it becomes the one reaction amount** | Below 25 % it also fades the continuous following, so 0 = the picture ignores the music (§5.3) |
| 28 | Softness (`softness`) | **MERGED** into React Style + EDIT › REACT "Decay" | |
| 29 | Reactivity (`reactivity`) | **MERGED** into Impact (amount) + EDIT › REACT "Follow" (trim) | |
| 30 | CALM (`calm`) | **MERGED** into the React switch as **STILL** (R1) | |
| 31-35 | React to KICK / SNARE / HAT / BASS / LEVEL (`reactKick`…`reactLevel`) | **MERGED** into React Style + the PLAY lamps show them + EDIT › REACT toggles (override) | |
| 36 | Scene name + description | PLAY (S2, S3) | |
| 37 | Scene list (`preset`) | PLAY scene grid (S8) | 4 x 4 tiles. Same cue / GO semantics |
| 38 | < > (`scenePrev`, `sceneNext`) | PLAY (S4, S6) | They move the cue |
| 39 | GO (`sceneGo`) | PLAY (S7), bigger | |
| 40 | HIT (`hit`) | PLAY (K10), the biggest button, next to IMPACT | |
| 41 | BLACKOUT (`blackout`) | PLAY (H6), top-right, isolated | |
| 42-46 | Grain, Halation, Weave, Dust, Blacks | **MERGED** into Look + Look Amount (C2, C3) + EDIT › LOOK › FILM | Shown as "Gate weave" and "Film base" |
| 47-51 | Crush, Trails, Glitch, Smear, Symbols | **MERGED** into Look + Look Amount + EDIT › LOOK › DIGITAL | Shown as "Glyphs" |
| 52 | Flash (`flash`) | **MERGED** into Look + EDIT › LOOK › ON HITS | |
| 53 | Cut Rate (`cutRate`) | EDIT › MOTION "Auto scene cuts", a segmented control | Not part of any look, and not scaled by Look Amount: it switches scenes |
| 54 | Shots (`shots`) | **MERGED** into Look + EDIT › LOOK › ON HITS | |
| 55 | HUD (`hud`) | **MERGED** into Look + EDIT › LOOK › OVERLAY | |
| 56 | Palette (`palette`) | PLAY (C1), colour picker with stripes; EDIT › COLOUR (cards) | |
| 57-59 | Shadow / Mid / Light swatches | EDIT › COLOUR | |
| 60-63 | Snapshots A-D (`snap1`…`snap4`) | PLAY **MOMENTS** (N1-N4) | Named by recipe, e.g. "Film · Blood" |
| 64 | STORE | **REMOVED** | Click an empty moment = save. Right-click = save here / rename / clear |
| 65 | Morph time (`morphTime`) | PLAY (N5), caption "morph · 1 bar ▾" | |
| 66 | Media slots 1-8 (`mediaSlot`) | EDIT › MEDIA (cards with thumbnails) | PLAY shows the active slot (M1) |
| 67 | LOAD | PLAY (M2) + drag-and-drop anywhere + EDIT › MEDIA | |
| 68 | USE MEDIA (`useMedia`) | EDIT › MEDIA "Use in all image scenes" | |
| 69 | CLEAR | EDIT › MEDIA | |
| 70 | LOOP / PING-PONG (`clipMode`) | EDIT › MEDIA | |
| 71 | Clip Sync (`clipSync`) | EDIT › MEDIA "Length" | |
| 72 | Media info line | **MERGED** into M1 + the info line (I1) + EDIT › MEDIA status | |
| 73 | Tooltips (only help today) | **MERGED** into the info line (I1) | The same text is still the tooltip after 1.2 s |
| 74 | The 35 % alpha dimming of non-sending instances | **REMOVED** | Replaced by the SOURCE view |

### 1.2 Out-of-the-box merges, evaluated

| Idea | Verdict | Why |
|---|---|---|
| 14 LOOK knobs → one **Look** preset + one **Look Amount** | **Adopt** | "Make it more worn" is one choice, not five knob hunts. Amount acts as dry / wet (100 % = the look as set), so it never changes old sets. The knobs stay in EDIT for fine work. Editing one shows **Custom** |
| Palette + look fused into one **Style** preset (e.g. "Blood Film") | **Reject the fusion; adopt the card** | 14 palettes x 6 looks = 84 presets, and the owner changes colour independently of treatment. So: one **STYLE** card with two pickers side by side (COLOUR, LOOK) and the amount under them |
| **Recipe as a name** (Zwobot rec. 10) | **Adopt** | Moments are named automatically by their recipe ("Worn · Ash · Pulse" shortened to "Worn · Ash"), and renamable. The STYLE card and the React switch spell the live recipe in words |
| Snapshots → **Moments**, click-empty-to-save | **Adopt** | Removes the invisible STORE mode (dossier confusion 10). An empty slot has nothing to recall, so the obvious click is "save" |
| Moments also carry the scene (a Cue Bank) | **Later** | This would bypass GO, the owner's chosen switching model. Revisit with the Cue Bank (competitor research M1) |
| Roles automatic from the track name | **Adopt** (§4.3) | JUCE forwards Live 10.1.2's track name (`updateTrackProperties`). An unknown name keeps MIX. A manual pick switches AUTO off |
| Lead automatic (process-wide election) | **Adopt** (§4.2) | All instances live in Live's process. A shared registry elects exactly one sender with no engine change |
| Media panel → EDIT, with only an indicator + LOAD on PLAY, and drop-anywhere | **Adopt** | The owner's failed click path (memory "show results immediately") becomes: drop the file, see it. The auto-switch to Media Negative stays and is announced in the info line |
| SIGNAL panel → one lamp strip | **Adopt** | Five lamps (KICK SNARE HAT BASS LEVEL) in the REACT block. They show hits *and* what the current character ignores (hollow, struck-through). The spectrum and bands go to SETUP |
| CALM → the **STILL** cell of the React switch | **Adopt** | Calm is "the character: none". One row answers "how does the picture follow the music", including "not at all" |
| Reactivity + Impact → one knob (IMPACT) | **Adopt** | Removes the "I turned Impact to 0 and it still moves" trap. Keeps the owner's 8 knobs. Reactivity survives as the EDIT trim "Follow" |
| REACT TO toggles + Push + Softness → **React Style** (BREATHE / PULSE / PUNCH) | **Adopt** | Direction of REACTION-DESIGN.md. The toggles stay in EDIT as overrides. The PLAY lamps make the character's choice visible |
| Hit Sens / Trim / normaliser → automatic calibration | **Adopt** (REACTION-DESIGN) | Per-instance input setup belongs to SETUP / SOURCE, never PLAY |
| FREEZE merged into Speed ("Speed 0 = frozen") | **Reject** | Freeze is instant and returns to the same speed. A knob can't do both. Kept as a small toggle (K9) on the MOTION group caption |
| A bipolar Speed knob (left = reverse, centre = frozen) that absorbs Reverse and Freeze | **Reject** | It changes `macro2` semantics and breaks existing automation. It halves the resolution of normal speeds |
| Reverse, Sync, Drift on PLAY | **Reject** | Rare or set-per-song. EDIT › MOTION |
| Sync automatic when Live plays | **Reject** | It would change scene speeds between songs by itself |
| Cut Rate inside the Look presets | **Reject** | It switches scenes by itself, which fights cue + GO. It lives alone in EDIT › MOTION |
| Look Amount as a 9th knob | **Reject** | A 9th knob beside the 8 macros reads as "another macro". A horizontal slider inside the STYLE card reads as "amount of this choice" |
| Engine pill = engine button | **Adopt** | One object for "is it running / start it / show it" |
| Tooltips → a fixed info line | **Adopt** | Pop-ups cover the knobs. A fixed line under PLAY always answers "what is this" in the same place, and doubles as the state-hint line |
| Lookahead automatic | **Reject** | The plug-in cannot know if a live instrument is monitored through Live |
| A **SAFE** button (competitor research) | **Not now** | BLACKOUT + STILL + FREEZE cover it with visible, separate meanings. Revisit for the dance show |
| Per-group colours on the macros | **Reject** | Colour carries state only (dossier §5.4). Grouping comes from captions and spacing |
| Macro names change per scene | **Reject** | The owner learns 8 fixed names. The scene's word is the sub-label (kept) |

---

## 2. PLAY view (760 x 480)

The default view of the lead instance. At 100 %, 760 x 480 logical px. On the owner's 1536 x 864 logical screen that leaves about 776 px for Live beside it, and about 300 px of height free under the taskbar and title bar.

**Wireframe:** [wireframe-play-v2.svg](wireframe-play-v2.svg). The cyan IDs in it are the IDs below.

```
 0 ┌ VJ ANALYZER  [● ENGINE 60 fps · 124.0 BPM · 17.3][⤢] [LEAD · Master] [EDIT ▸]  [BLACKOUT] ┐
40 ├──────────────────────────────────────────────────────────────────────────────────────────┤
   │ SCENE              click = cue · GO = switch …  │ REACT       how the picture follows… │
   │ [LIVE] Mesh Body                                │ [STILL|BREATHE|PULSE|PUNCH]          │
   │ A breathing body of fine plexus lines; …        │ ● KICK ◌ SNARE ◌ HAT ● BASS ▬ LEVEL  │
   │ [◂][ NEXT ▸ HALO RING · press GO     ][▸][ GO ] │ STYLE                                │
   │ [Hot Blobs ][Dot Relief][One Bit   ][Corridor ] │ [COLOUR ▮▮▮ Blood ▾][LOOK Film ▾]    │
   │ [Fibers    ][Terminal  ][Ink       ][Sig. Fog ] │ LOOK AMOUNT ━━━━━━━━━━━━━●  100 %    │
   │ [Mesh Body▌][Morphogen ][Halo Ring…][Emergence] │ [▭ IMAGE · SLOT 1 field_02.jpg ▸][LOAD…] │
   │ [Negative  ][Media Neg.][Media Lin.][Membrane ] │ MOMENTS           morph · 1 bar ▾    │
   │                                                 │ [A Film·Blood][B …][C …][D + save]   │
306├──────────────────────────────────────────────────────────────────────────────────────────┤
   │ SHAPE · THIS SCENE                        MOTION  [‖ FREEZE]  REACT                     │
   │ (INTENSITY)(FORM)(SCALE)(ERODE)(DETAIL)   (SPEED)(GLIDE)     (IMPACT)   [   HIT   ]     │
   │  glow     tension body size body→dust …   live x1.12 bars    hits+flow                  │
442├──────────────────────────────────────────────────────────────────────────────────────────┤
   │ IMPACT 40 % — How much the music moves the picture. 0 = ignores it …   (Live: Impact)   │
480└──────────────────────────────────────────────────────────────────────────────────────────┘
```

### 2.1 Components (x, y, w, h at 100 %)

Column meanings:
- **Label:** the text drawn on the component.
- **Help:** shown in the info line (I1) on hover, and as the tooltip after 1.2 s.
- **Binds:** the host parameter or the processor call.

#### Header (0, 0, 760, 40)

Fill `ink-1`, hairline at y 40.

| ID | x, y, w, h | Label | Help | States | Binds |
|---|---|---|---|---|---|
| H1 | 16, 12, 100, 16 | VJ ANALYZER | (none) | Placeholder text, no branding work | n/a |
| H2 | 124, 8, 300, 24 | ● ENGINE 60 fps · 124.0 BPM · 17.3 | Connected: "The picture engine. Click to bring its window to the front." Off: "The picture engine is not running. Click to start it." | **Off:** `signal`-filled button, ink text "▶ START ENGINE" (the only filled red thing on screen). **Starting:** `tungsten` dot, "STARTING…", a hairline progress along the bottom. After 8 s: "NOT FOUND · LOCATE…". **On:** `signal` dot + fps · BPM · bar.beat (mono). **Slow** (render scale < 100 %): "· 76 %" appended in `tungsten`. **Lost:** `signal` outline, "ENGINE LOST · RELAUNCH". **Blackout:** "OUTPUT DARK" instead of fps | `openEngine()` (existing) |
| H3 | 428, 8, 24, 24 | ⤢ (four corner marks) | "Engine full screen on / off. F in the engine window does the same." | Disabled (dust) when there is no engine | `toggleEngineFullscreen()` |
| H4 | 460, 8, 100, 24 | LEAD · Master | "This window leads the picture: its knobs, look and scene go to the engine. Click for setup." | LEAD + track name. If the election found no lead: "NO LEAD" in `tungsten` | Opens EDIT › SETUP |
| H5 | 572, 8, 76, 24 | EDIT ▸ / EDIT ◂ | "Open the detail drawer: look, colour, reactions, motion, media, setup." | Off / on (on = `ink-2` fill, bone outline) | Window width 760 ↔ 1180 |
| H6 | 660, 6, 84, 28 | BLACKOUT | "Fades the output to black. Click again to bring it back." | **Idle:** 1.5 px `signal` outline, `signal` text. **On:** `signal` fill, ink text, plus a steady 2 px `signal` frame around the whole window. Never blinks | `blackout` |

#### Scenes (left column, x 16-464)

| ID | x, y, w, h | Label | Help | States | Binds |
|---|---|---|---|---|---|
| S0 | 16, 48, 60, 12 (+ hint 200, 48, 264, 12, right-aligned) | SCENE · "click = cue · GO = switch · double-click = both" | (none) | n/a | n/a |
| S1 | 16, 70, 32, 14 | LIVE | (none) | `signal` tag. In blackout it widens to 72 px and reads OUTPUT DARK. Hidden with no engine | Engine status |
| S2 | 56, 64, 408, 26 | *scene name*, 20 px SemiBold | (none) | No engine: "No engine" in `dust` | `status.presetName` |
| S3 | 16, 94, 448, 14 | *scene description*, 10.5 px `dust`, one line, ellipsised | (none) | This line is replaced by media hints when relevant: "Mesh Body doesn't show images — pick a scene marked IMG", "Switched to Media Negative so your image shows" | `status.sceneDescription` |
| S4 | 16, 116, 32, 36 | ◂ | "Cue the previous scene. GO switches. (Live: Previous Scene)" | Idle / hover / pressed | `scenePrev` |
| S5 | 52, 116, 276, 36 | Cue field | Cued: "Click to cancel the cue." | **Empty:** `dust` "Click a scene to cue it". **Cued:** dashed `tungsten` outline, "NEXT ▸ HALO RING" (11 px caps, `tungsten`) + "press GO to switch". **Switching:** 15 % `tungsten` fill, solid outline, "HALO RING ▸ on the next beat", plus a 4-segment countdown at the right (from the engine's `beatsToGo` when available, otherwise a steady bar). **Transport stopped:** "press GO — switches at once" | `getCuedScene()`, `getFiredScene()`; click = `cueScene (-1)` |
| S6 | 332, 116, 32, 36 | ▸ | "Cue the next scene. GO switches. (Live: Next Scene)" | Idle / hover / pressed | `sceneNext` |
| S7 | 372, 116, 92, 36 | GO (14 px bold) | "Switch to the cued scene. While Live plays it lands on the scene's beat. (Live: Go)" | **Nothing cued:** `ink-1` / `line` / `dust` text, and a click does nothing (the info line says "Cue a scene first"). **Cued:** `tungsten` fill, ink text. **Pressed:** 60 % `tungsten`. **Switching:** `tungsten` outline only | `sceneGo` |
| S8 | 16, 162, 448, 132 | Scene grid: 4 columns, tile 109 x 30 at (16 + 113·col, 162 + 34·row) | Per tile: "*Name*: *description*. Click = cue, double-click = switch." | **Idle:** `ink-1` / `line`. **Hover:** `ink-2` / `ash`. **LIVE:** `ink-2`, `signal` outline, 3 px `signal` left bar, "LIVE" micro. **CUED:** dashed `tungsten` outline + a `tungsten` "NEXT" badge. **Switching:** 20 % `tungsten` fill + "BEAT" badge. **IMG** badge (22 x 12, `ash` outline) on scenes that show the user's media. **No engine:** names from the cached list, `dust`, not clickable. **More than 16 scenes:** the wheel scrolls by row, with a 2 px scroll hairline at x 466 | Click = `cueScene (i)`; double-click = `fireScene (i)` (existing) |

#### REACT, STYLE, MEDIA, MOMENTS (right column, x 480-744)

| ID | x, y, w, h | Label | Help | States | Binds |
|---|---|---|---|---|---|
| R0 | 480, 48, 80, 12 (+ hint 560, 48, 184, 12) | REACT · "how the picture follows the music" | (none) | n/a | n/a |
| R1 | 480, 64, 264, 28 (4 cells of 66: STILL 480, BREATHE 546, PULSE 612, PUNCH 678) | STILL · BREATHE · PULSE · PUNCH | See the list after this table | See the list after this table | `calm`, `reactStyle` (new) |
| R2 | 480, 96, 264, 16 (5 cells of 53) | ● KICK ● SNARE ● HAT ● BASS ▬ LEVEL | "What the picture follows. A lamp flashes on each hit. A crossed-out lamp is ignored by the current character." | **Followed:** `ash` dot, which flashes `signal` for 200 ms on a hit. **Ignored:** hollow dashed dot, struck-through `dust` text. **LEVEL:** a 12 x 6 `halation` bar. **STILL:** all dim. **Silent:** all `dust`, and the info line says "SILENT — play something in Live" | Display: lead onsets today, engine `/v2/sources` echo later (routing research, Option A) |
| C0 | 480, 126, 80, 12 | STYLE | (none) | n/a | n/a |
| C1 | 480, 142, 130, 24 | Micro "COLOUR" + a 30 x 8 three-stripe chip + palette name + ▾ | "The picture's three colours. Click to choose. (Live: Palette)" | Closed / open. The popup has 14 rows (stripe chip + name), then "Edit colours…" (opens EDIT › COLOUR). The chip shows the actual colours, including Custom | `palette` |
| C2 | 614, 142, 130, 24 | Micro "LOOK" + look name + ▾ | "The film and digital treatment over every scene. Click to choose. (Live: Look)" | The name, or *Custom* in `ash` italic after an edit. The popup lists Clean, Film, Worn, Broken, Print, Data, then a separator, "Custom (edited)" (disabled, ticked when active) and "Edit look…" (opens EDIT › LOOK) | `lookPreset` (new) |
| C3 | Label 480, 172, 64, 20 · slider 548, 172, 156, 20 · value 708, 172, 36, 20 | LOOK AMOUNT ━━━● 100 % | "How strong the look is. 0 = clean picture, 100 % = the look as set. Double-click = 100 %. (Live: Look Amount)" | Remote tick when MIDI moves it | `lookAmount` (new) |
| M1 | 480, 206, 196, 24 | 32 x 18 thumbnail + micro "IMAGE · SLOT 1" + file name + ▸ | "Your image or clip in the active slot. Click to manage the slots. Drop a file anywhere on this window to load it." | **Empty:** "IMAGE · none — drop a file or LOAD" in `dust`. **Loading:** `tungsten` hairline progress. **Loaded:** name. **Error:** `signal` "can't read this file" | Click opens EDIT › MEDIA |
| M2 | 680, 206, 64, 24 | LOAD… | "Load an image (PNG, JPEG) or a short clip into the active slot. If the live scene can't show it, the plug-in switches to Media Negative." | Idle / pressed | `loadMedia()` + `ensureMediaScene()` (existing) |
| N0 | 480, 246, 80, 12 | MOMENTS | (none) | n/a | n/a |
| N5 | 636, 239, 108, 22 | morph · 1 bar ▾ (right-aligned text button) | "How long a recalled moment takes to blend in. (Live: Snapshot Morph)" | Popup: Cut, 1 beat, 1 bar, 4 bars, 16 bars | `morphTime` |
| N1-N4 | (480 + 67·i, 264, 63, 30), i = 0-3 | A B C D + the auto name ("Film · Blood") | Stored: "Moment A (Film · Blood). Click to blend to it. Right-click: save here, rename, clear. (Live: Snapshot A)". Empty: "Empty. Click to save the knobs, look, colour and reactions here." | **Empty:** dashed `dust`, "+ save". **Stored:** `ink-1` / `line`. **Active:** bone outline (last recalled, not changed since). **Recalling:** a 2 px `halation` progress bar along the bottom over the morph time. **Just saved:** 600 ms `tungsten` outline + "SAVED" | Click stored = `snap1..4`. Click empty = `storeSnapshot (i)`. Right-click menu = store / rename / clear |

**R1 help, per cell:**
- STILL: "Reactions fade out over one bar; the picture keeps moving on its own. Click a character to come back. (Live: Calm)"
- BREATHE: "The picture swells with loudness and bass. No hits."
- PULSE: "The kick leads, the rest flows. Snare and hat are ignored."
- PUNCH: "Every hit lands: kick, snare, hat. Short and sharp."

**R1 states:**
- The selected character cell has a `bone` fill with ink text.
- **STILL** has a `tungsten` outline while it fades (one bar), then a `bone` fill.
- Under STILL, the character that will return keeps a 1.5 px bone underline.
- **Edited in EDIT › REACT:** no fill. The last character is outlined with an asterisk ("PULSE*"), and its help reads "Edited. Click to reset to Pulse."

#### Bottom band: the 8 macros and HIT

A hairline at y 306 runs from x 16 to 744.

**Knob cells** are 72 x 94:
- a Ø56 knob at the top centre, with the value inside (mono 10 px)
- the label at y + 62 (10 px SemiBold, `bone`)
- the sub-label at y + 76 (9.5 px, `ash`)

The screen order is grouped (the owner liked the grouping). The host order (`macro1…8`) is unchanged, and Live's MIDI Map maps by parameter, not by screen position.

| ID | x, y, w, h | Label (sub-label) | Help | Centre value | Binds |
|---|---|---|---|---|---|
| K0a | 16, 318, 200, 12 | SHAPE · THIS SCENE | "What the scene looks like. The word under each knob says what it does in this scene." | n/a | n/a |
| K1 | 16, 342, 72, 94 | INTENSITY (*scene word*, e.g. "glow") | "Light and energy of the scene." | 50% | `macro1` |
| K3 | 88, 342, 72, 94 | FORM (*scene word*) | "The scene's character. The word underneath says what it is here." | % | `macro3` |
| K4 | 160, 342, 72, 94 | SCALE (*scene word*) | "Bigger as you turn it up." | % | `macro4` |
| K6 | 232, 342, 72, 94 | ERODE (*scene word*, e.g. "body → dust") | "From pristine to worn, torn and dissolved." | % | `macro6` |
| K8 | 304, 342, 72, 94 | DETAIL (*scene word*) | "Density and fineness." | % | `macro8` |
| K0b | 392, 318, 60, 12 | MOTION | (none) | n/a | n/a |
| K9 | 456, 314, 80, 24 | ‖ FREEZE | "Stops all motion at once; film grain keeps running. Click again to go on at the same speed. (Live: Freeze)" | On: `ink-2` fill, bone outline; the Speed centre reads FROZEN | `freeze` |
| K2 | 392, 342, 72, 94 | SPEED (live readout: "live x1.12", "reverse", "FROZEN") | "How fast everything moves. 0 = frozen, middle = the designed speed, full = x4." | x1.00 / FROZEN | `macro2` |
| K7 | 464, 342, 72, 94 | GLIDE ("bars") | "How long speed changes take to arrive: 0 to 4 bars." | 0.25 | `macro7` |
| K0c | 552, 318, 72, 12 | REACT | (none) | n/a | n/a |
| K5 | 552, 342, 72, 94 | IMPACT (the scene's word if it names it, else "hits + flow"; "still" during STILL) | "How much the music moves the picture. 0 = ignores it, 40 % = as designed, 100 % = hits land double." | 40% | `macro5` (+ derived reactivity, §5.3) |
| K10 | 640, 342, 104, 86 | HIT (20 px) / "a manual kick" | "A manual hit, like a kick. Works in STILL too. (Live: Hit)" | Idle: `ink-2`, bone outline. On press: a 150 ms 20 % `signal` fill | `hit` |
| I1 | 16, 446, 728, 22 | Info line, 10 px `dust` | Hover: "NAME value — sentence. (Live: *parameter name*)". Idle, in priority order: "Start the engine (top left) to see the picture." → "SILENT — play something in Live." → "Halo Ring is cued — press GO." → the media hint → empty | n/a | Component under the mouse |

**States common to all knobs:**
- idle, and hover (the arc brightens and the info line shows the help)
- drag: vertical, Shift = fine, double-click = default
- **modulated:** an outer 2 px `halation` arc from the value, whose length = `status.macroActivity`
- **remote:** a 4 px `remote` cyan dot for 1 s when MIDI or automation moved it without a UI gesture
- **no engine:** rings off, and the sub-labels fall back to generic words

### 2.2 Global PLAY states

| State | Header | Scene area | REACT / STYLE | Knobs | Info line |
|---|---|---|---|---|---|
| Engine off (first run) | H2 = red START ENGINE; H3 disabled | S2 "No engine". S3 "1 Start the engine · 2 Play something in Live · 3 Pick a scene, press GO". Grid from the cache, dimmed | Normal (values are stored in the set) | Usable, no rings | "Start the engine (top left) to see the picture." |
| Starting | H2 tungsten STARTING… | As above | Normal | n/a | "Starting the engine…" |
| Engine lost | H2 ENGINE LOST · RELAUNCH | Names kept; the last scene is re-sent on reconnect (existing 1 s re-send) | Normal | Usable | "The engine closed. Click RELAUNCH." |
| Connected, silent | Normal | Normal | Lamps `dust` | No rings | "SILENT — play something in Live." |
| Scene cued | Normal | S5 cued, tile NEXT, S7 tungsten | n/a | n/a | "Halo Ring is cued — press GO." |
| Switching (after GO) | Normal | S5 switching + countdown, tile BEAT | n/a | n/a | "Switching on the next beat…" |
| BLACKOUT | H6 filled; 2 px signal frame around the window | S1 OUTPUT DARK | n/a | n/a | "Output is dark. Click BLACKOUT to bring it back." |
| STILL | n/a | n/a | R1 STILL (tungsten while fading); lamps dim | K5 sub-label "still", rings shrink | "Reactions fading out over one bar." |
| Frozen | n/a | n/a | n/a | K9 on; K2 centre FROZEN | n/a |
| Media not shown by the live scene | n/a | S3 hint; IMG tiles get a brighter IMG badge | M1 normal | n/a | Same hint |
| Save of a moment | n/a | n/a | Tile flashes SAVED | n/a | "Saved as moment D (Worn · Ash)." |
| Two instances claim lead (only possible across processes) | H4 "2 LEADS" in tungsten | n/a | n/a | n/a | "Another instance also leads. Only this one is used." |

### 2.3 Behaviour notes

- **Cue and GO** are exactly today's processor logic (`cueScene`, `fireScene`, `stepCue`, `goCue`). Only the drawing changes. Double-click = cue + GO.
- **Scene cache:** the editor stores the engine's last scene list and IMG flags in the plug-in state (property `sceneCache`). The grid is therefore never an empty "-" on first open.
- **Drag and drop:** the editor is a `FileDragAndDropTarget` for png / jpg / jpeg / mp4 / mov / m4v / avi / wmv / mkv / webm.
  - A drop loads into the active slot and calls `ensureMediaScene()`.
  - A drop on a slot card in EDIT › MEDIA loads into that slot.
- **The info line** replaces pop-up help as the primary explanation. The same text is set as the tooltip, with a 1200 ms delay.
- **No keyboard shortcuts on PLAY.** Live owns the keyboard (Space = transport), and the Korg or MIDI Map is the fast path.

---

## 3. EDIT view (1180 x 480)

### 3.1 How it opens

- **EDIT ▸ (H5)** widens the window from 760 to **1180 x 480**. The height never changes, well under the 560 limit.
- The PLAY panel stays exactly where it was. The drawer occupies x 760-1180, with a hairline at x 760 (the FabFilter / Arturia "expand" pattern: spatial memory survives).
- **Close:** EDIT ◂, or the ✕ at (1140, 8, 24, 24).
- **Shortcuts** open the drawer on a given tab:

  | From | Opens |
  |---|---|
  | C1 "Edit colours…" | COLOUR |
  | C2 "Edit look…" | LOOK |
  | M1 | MEDIA |
  | H4 | SETUP |

- The open / closed state and the last tab are remembered per instance (state properties `drawerOpen`, `drawerTab`).
- **The info line (I1)** stretches to (16, 446, 1148, 22) and serves the drawer too.
- **The tab strip** sits in the header band: 6 tabs at (776 + 60·i, 8, 56, 24), i = 0-5.
  - **LOOK · COLOUR · REACT · MOTION · MEDIA · SETUP.**
  - Active tab: `ink-2` fill + bone outline. Idle: `line` outline, `ash` text.
- **The content area** is (776, 48, 388, 388).
- **Drawer knobs** are Ø36 in 74-93 px cells. The label is 9 px caps `ash`, the value is mono 8.5 px inside the knob, and a value of zero reads "off".

**Wireframe:** [wireframe-edit-v2.svg](wireframe-edit-v2.svg) (the full window with LOOK, then every other tab and the SOURCE view).

### 3.2 LOOK tab

| ID | x, y, w, h | Label | Binds | Help |
|---|---|---|---|---|
| L1 | (776 + 65·i, 48, 61, 24), i = 0-5 | CLEAN · FILM · WORN · BROKEN · PRINT · DATA | `lookPreset` = i + 1 | See the list after this table. Selected = bone fill. None is filled when Custom |
| L2 | Label 776, 80, 60, 20 · slider 840, 80, 280, 20 · value 1124, 80, 40, 20 | AMOUNT | `lookAmount` | Same as C3 |
| L3 | Caption 776, 112 · knobs (776 + 78·i, 128, 74, 66) | **FILM** ("35 mm physics"): GRAIN, HALATION, GATE WEAVE, DUST, FILM BASE | `grain`, `halation`, `weave`, `dust`, `blacks` | "Film grain at 24 fps, following the image's brightness." / "Red glow bleeding out of the highlights." / "Gate weave, rare frame slips, flicker, soft colour fringes." / "Dust specks, hairs and scratches — rare and correlated." / "Lifted, tinted blacks: the film base." |
| L4 | Caption 776, 204 · knobs (776 + 78·i, 220, 74, 66) | **DIGITAL** ("disturbances"): CRUSH, TRAILS, GLITCH, SMEAR, GLYPHS | `crush`, `trails`, `glitch`, `smear`, `symbols` | "Soft S-curve → posterised → hard two-tone." / "Frame memory: motion leaves a trail." / "Row tears, pixel blocks, RGB split, pulsed by hits." / "VHS tracking drift on rows, in bursts." / "Braille glyph strips, re-dealt on snares." |
| L5 | Caption 776, 296 · knobs (776 + 78·i, 312, 74, 66), i = 0-2 | **ON HITS · OVERLAY** ("what strong hits do"): FLASH, SHOTS, HUD | `flash`, `shots`, `hud` | "Chance a kick drops a black or colour frame (max 3 a second)." / "Strong hits become short cuts: new framing, negative, lens punch, black frame." / "Measuring-instrument overlay: crosshairs, linked squares, faint circles." |
| L6 | Note 776, 396, 270, 28 · button 1060, 400, 104, 24 | "Turning a knob makes the look CUSTOM. Moments remember custom looks." · RESET TO *FILM* | Rewrites the selected preset vector | "Put every knob back to the *Film* look." |

**L1 help, per look:**
- CLEAN: "Almost no treatment: a clean digital picture."
- FILM: "35 mm: grain, halation, weave, dust, lifted blacks. The default."
- WORN: "An old print: heavier grain, dust and weave."
- BROKEN: "Digital damage: glitch, smear, flashes and hit cuts."
- PRINT: "Photocopy: hard two-tone, grey paper blacks, grain."
- DATA: "Instrument: glyph strips, HUD overlay, hit cuts."

### 3.3 COLOUR tab

| ID | x, y, w, h | Label | Binds | Help |
|---|---|---|---|---|
| P1 | Cards (776 + 98·col, 48 + 52·row, 94, 48), 4 per row | Stripes (86 x 22) + name. Order: Blood, Ember, Bone, Ice / Acid, Violet, Rust, Nitrate / Cyanotype, Tungsten, Ash, Split / Custom, Scene Colors | `palette` indices 0-10, 13, 11, 12 (the parameter order is unchanged) | "*Name* palette." Scene Colors: "Every scene keeps its own colours (palette off)." Split: "Black / red / cyan, made for Negative." Selected = bone outline + bone name |
| P2 | Caption 776, 268 | CUSTOM COLOURS | n/a | n/a |
| P3 | Swatches (776 + 130·i, 284, 124, 40), labels at y 330 | SHADOW · MID · LIGHT | `setCustomColour (i)` (state, not a parameter) via the existing ColourSelector call-out | "Click to change this colour. The palette becomes Custom." |
| P4 | 776, 352, 388, 28 | Note: "Click a colour to change it; the palette becomes Custom. Scene Colors = every scene keeps its own colours." | n/a | n/a |

### 3.4 REACT tab

| ID | x, y, w, h | Label | Binds | Help |
|---|---|---|---|---|
| Q1 | 776, 48, 388, 28 (4 x 97) | STILL · BREATHE · PULSE · PUNCH | `calm`, `reactStyle` | Same as R1 |
| Q2 | 776, 82, 388, 26 | Two lines: the selected character's sentence + "Picking a character sets the four knobs and the FOLLOWS lamps below." | n/a | n/a |
| Q3 | Caption 776, 116 · knobs (776 + 97·i, 132, 93, 66) | **AMOUNT AND FEEL:** IMPACT, FOLLOW, DECAY, MUSIC PUSH | `macro5`, `reactivity`, `softness`, `push` | See the list after this table |
| Q4 | Caption 776, 208 (+ hint "click to override the character") · toggles (776 + 78·i, 224, 74, 28) · activity bars (776 + 78·i, 256, 74, 3) | **FOLLOWS:** KICK · SNARE · HAT · BASS · LEVEL | `reactKick`, `reactSnare`, `reactHat`, `reactBass`, `reactLevel` | "The picture follows *kick* hits. Off = ignored." On = `ink-2` + bone outline + lamp. Off = dashed `dust`. The bar shows that channel's activity |
| Q5 | Caption 776, 272 · rows (776, 288 + 22·k, 388, 20), up to 5, then a scroll | **SOURCES** ("instances feeding the picture"): lamp · track name · role · "last hit 0.3 s" / "lead · this window" | Display. Needs the engine's `/v2/sources` echo (routing research, Option A.6). Until then: this instance only + "other sources appear when the engine reports them" | "An instance of the plug-in on another track. Its hits drive the picture." |
| Q6 | 776, 400, 388, 24 (dashed, reserved) | LISTEN FOCUS · MIX ──●── VISUALS | Future `focus` (routing research, Option B) | Hidden until a VISUALS / FOCUS source exists |

**Q3 help, per knob:**
- IMPACT: same as K5.
- FOLLOW: "How much the steady flow (loudness, bass, brightness) moves the picture, on top of IMPACT. 100 % = as designed."
- DECAY: "How long each hit reaction lingers: x0.5 to x4."
- MUSIC PUSH: "How much loud passages speed motion up, up to x2."

Changing DECAY, MUSIC PUSH or a FOLLOWS toggle makes the character Custom. IMPACT and FOLLOW are amounts, not character, so they never do.

Per-instance detection (Detect, Trim, Level) is *not* here: it is input setup, in SETUP and in the SOURCE view.

### 3.5 MOTION tab

| ID | x, y, w, h | Label | Binds | Help |
|---|---|---|---|---|
| V1 | Caption 776, 48 · knob 776, 64, 93, 66 | CAMERA · CAMERA DRIFT | `drift` | "A slow camera over the whole picture: push-in, turn, pan. Stops with Speed 0 or Freeze." |
| V2 | Caption 880, 48 · toggles 880, 64, 90, 28 / 976, 64, 90, 28 / 1072, 64, 92, 28 · note 880, 98, 284, 26 | DIRECTION AND TEMPO: FREEZE · REVERSE · TEMPO-LOCK | `freeze`, `reverse`, `sync` | "Stops all motion." / "Runs all motion backwards." / "Speeds follow Live's tempo: the designed speed is at 120 BPM." |
| V3 | Caption 776, 148 (+ hint "switches scenes by itself; GO still works") · segmented 776, 164, 388, 28 (7 cells) | AUTO SCENE CUTS: OFF · 16 · 8 · 4 · 2 · 1 · KICK | `cutRate` | See the notes after this table |
| V4 | 776, 198, 388, 14 | Note: "Counted in beats. KICK = every kick (max 3 a second). Paused while STILL." | n/a | n/a |
| V5 | Caption 776, 236 · box 776, 252, 388, 44 | LIVE READOUT: "speed x1.12 (Speed x1.00 · music push +12 %)", "clock following Live · 124.0 BPM · bar 17" | Display (`status.speed`, transport) | "What the scene clock is doing right now." |
| V6 | 776, 308, 388, 14 | "Speed and Glide are on the main panel (MOTION)." | n/a | n/a |

**V3 values.** Cells write 0.0, 0.1, 0.3, 0.5, 0.7, 0.8 and 0.95. These match the engine's thresholds in `PresetManager::updateAutoCut`:

| Stored value | Engine behaviour |
|---|---|
| < 0.04 | off |
| < 0.2 | every 16 beats |
| < 0.4 | every 8 beats |
| < 0.6 | every 4 beats |
| < 0.75 | every 2 beats |
| < 0.9 | every beat |
| otherwise | every kick |

Reading back maps each value range to its cell.

**V3 help:** "Automatic cuts between scenes, every N beats or on every kick. Off by default."

### 3.6 MEDIA tab

| ID | x, y, w, h | Label | Binds | Help |
|---|---|---|---|---|
| D1 | Cards (776 + 98·col, 48 + 78·row, 94, 72), 2 rows of 4 | 94 x 53 thumbnail (the first frame for clips, with the duration) + "1 · field_02.jpg" | Click = `mediaSlot` = i + 1. Drop a file = load into that slot. Right-click = Load… / Clear / Show in Explorer | "Slot *n*. Click to show it; drop a file here to load it. (Live: Media Slot)" |
| D2 | LOAD… 776, 208, 90, 24 · CLEAR 872, 208, 70, 24 · toggle 948, 208, 216, 24 | LOAD… · CLEAR · ☐ USE IN ALL IMAGE SCENES | `loadMedia()`, `setMediaPath (slot, {})`, `useMedia` | See the notes after this table |
| D3 | Caption 776, 244 · segmented 776, 260, 150, 24 · caption 940, 244 · segmented 940, 260, 224, 24 | CLIP: LOOP · PING-PONG. LENGTH: FREE · 1b · 1 · 2 · 4 · 8 | `clipMode`, `clipSync` | "Clips loop from the end to the start, or play forward and back." / "Free = the clip's own speed x Speed; or stretch it over 1 beat to 8 bars, locked to the tempo." |
| D4 | Caption 776, 300 · text 776, 312, 388, 30 | SHOWN BY: "Media Negative, Media Lines. With USE IN ALL also: Dot Relief, One Bit, Emergence …" | `status.presetUsesMedia` | n/a |
| D5 | 776, 352, 388, 14 | "slot 1 · field_02.jpg · 1280x720 · loaded" (mono) | `status.media[slot]` | n/a |

**D1 states:**
- **Active:** bone outline + a "SHOWING" micro tag.
- **Empty:** dashed `dust` + "drop a file".
- **Loading:** a `tungsten` bar.
- **Error:** `signal` text.

**D2 help:**
- LOAD…: "Load into the selected slot."
- CLEAR: "Empty the selected slot; scenes go back to their own forms."
- USE IN ALL IMAGE SCENES: "Put your image into every scene that can use one (marked IMG)."

### 3.7 SETUP tab

| ID | x, y, w, h | Label | Binds | Help |
|---|---|---|---|---|
| U1 | Caption 776, 48 (+ track name at the right) | THIS INSTANCE · "track 'Master'" | `updateTrackProperties` | n/a |
| U2 | Role cells (776 + 53·i, 64, 52, 24) · AUTO chip 1096, 64, 68, 24 | MIX · KICK · SNARE · HAT · BASS · TEXT. · AUTO ✓ | `role`; state `roleAuto` | "What is on this track. AUTO follows the track name; picking a role by hand turns AUTO off." |
| U3 | Row 776, 96, 300, 24 · toggle 1076, 96, 88, 24 | "LEAD — this window sends knobs, look and scene" / "SOURCE — the lead is on 'Master'" · CAN LEAD ✓ | Display of the election; toggle = `sendControls` | "Off = this instance never leads (for example a test copy)." |
| U4 | Caption 776, 132 (+ "automatic unless you turn a knob") · knobs 776, 148 / 873, 148 / 970, 148 (93 x 66) · LEVEL caption 1068, 148 + segmented 1068, 164, 96, 24 · warning 970, 218, 194, 22 in `signal` | INPUT: DETECT · TRIM · LOOKAHEAD · LEVEL AUTO / FIXED | `sensitivity`, `trim`, `lookahead`, `normalizer` | See the notes after this table |
| U5 | Caption 776, 248 · spectrum 776, 264, 240, 64 · bands 1024, 264, 140, 40 · lamps 1024, 310, 140, 16 · status 776, 334, 388, 12 | SIGNAL: 32-band spectrum; LOW / MID / HIGH bars; LOW MID HIGH MIDI lamps; "LIVE 124.0 BPM 17.3 calibrated" | Meters (existing `paintSignalPanel` content, re-laid) | n/a |
| U6 | Caption 776, 356 · path 776, 372, 220, 24 · LOCATE… 1000, 372, 70, 24 · UI SIZE segmented 776, 404, 200, 24 · PARAMETERS… 1068, 404, 96, 24 | ENGINE AND WINDOW | `enginePath` state; UI scale (state `uiScale`); a call-out listing every host parameter: Live name → UI name → where it lives | "Where VJ Engine.exe is." / "Size of this window: 75-150 %." / "Every parameter as Live names it, for MIDI mapping and Configure." |

**U4 help:**
- DETECT: "How easily hits are found on this track. 'auto' = calibrated for you; turning it switches to manual, and AUTO brings it back."
- TRIM: "Input level for the analysis only; the sound is never changed."
- LOOKAHEAD: "Delays the sound (Live compensates) so the picture lands early. Only for playback / DJ sets."
- LEVEL: "Auto follows each song's loudness; Fixed stays put."

---

## 4. SOURCE view and the automatic lead

### 4.1 SOURCE view (360 x 240)

Every instance that is **not** the elected lead shows this view instead of PLAY. The editor calls `setSize (360, 240)` when the election result changes. There is no dimmed full UI any more.

**Wireframe:** the bottom-right of wireframe-edit-v2.svg.

| ID | x, y, w, h | Label | Binds | Help / states |
|---|---|---|---|---|
| W1 | Header 0, 0, 360, 36: wordmark 12, 10, 84, 16 · tag 98, 11, 50, 14 · engine 288, 10, 60, 16 | VJ ANALYZER · SOURCE · ● engine | Engine status | Engine dot: `signal` = connected, `dust` = off |
| W2 | Caption 12, 44 · dropdown 12, 60, 150, 24 · note 170, 60, 178, 24 | THIS TRACK · "KICK ▾" · "auto from 'Kick 808'" | `role`, `roleAuto` | "What is on this track. Picking one by hand turns AUTO off." Menu: Auto, Mix, Kick, Snare, Hat, Bass, Texture |
| W3 | Lamp 12, 96, 64, 64 · text 12, 166, 64, 12 | Big hit lamp · "last hit 0.4 s" | `meters.lastOnset[role band]` | Flashes `signal` for 200 ms per hit |
| W4 | Caption 88, 98 · meter 88, 110, 260, 10 · threshold line | LOW BAND (MID for SNARE, HIGH for HAT) · a `signal` threshold line at the Detect position | Meters + `sensitivity` | "The level of the band this role listens to. Hits fire above the red line." |
| W5 | DETECT 88, 136, 64, 60 · TRIM 156, 136, 64, 60 · MIDI lamp 232, 140, 40, 44 | DETECT ("auto") · TRIM · MIDI | `sensitivity`, `trim`, `meters.lastMidi` | As in U4. MIDI: "Lights on MIDI notes; each note is a hit for this role." |
| W6 | Hairline y 200 · text 12, 206, 250, 28 · button 268, 208, 80, 24 | "Feeds the picture. Knobs, look and scenes are played in the lead on 'Master'." · MAKE LEAD | Pins the lead to this instance | "Make this window the one that plays the picture." |

### 4.2 Lead election (kills the "Send macros" fight)

Live runs every VST3 instance in one process, so a process-wide registry can elect the lead with no engine change: a `juce::SharedResourcePointer<LeadRegistry>`, with a message-thread timer at 4 Hz.

**Each instance registers:**
- its creation serial
- its role
- `sendControls` (now: "can lead")
- its track name
- `leadPinned` + a pin time (state properties, saved with the set)

**Election rule** (deterministic, re-run when any of these change or an instance is added or removed):
1. If any candidate is **pinned**, the one pinned most recently leads. MAKE LEAD pins and unpins all others.
2. Otherwise, the candidates are the instances with `sendControls` on. Prefer role **MIX**; among MIX instances, prefer a track name of "Master" or "Main" (best effort; Live reports the Master track's name); then the **oldest** (lowest serial).
3. If no instance may lead, no one sends controls. Every window shows "NO LEAD" in `tungsten`, with the fix in the info line.

**Wiring:**
- `timerCallback` sends `c.sendControls = registry.isLead (this)` instead of the raw parameter. Exactly one instance sends knobs, look, scene, react and blackout.
- The first time a set has two or more candidates, the winner is auto-pinned. The choice then survives reloads even if the load order changes.
- Duplicated tracks copy `leadPinned`. The later pin time wins, and ties go to the older serial.
- **Handover:** when the lead is deleted, the next candidate leads with its *own* stored values. The info line in the new lead says "This window now leads the picture."
- **Engine backstop (later, routing research A.1):** the engine accepts controls only from one `leadId` and reports it in `/v2/status`. This covers two Live processes or two computers. The UI's "2 LEADS" state uses it.

### 4.3 Role from the track name

- `updateTrackProperties` gives the name. Name and colour can arrive separately in Live 10, so debounce by 500 ms.
- **Matching:** case-insensitive, whole words, first match wins:

  | Pattern | Role |
  |---|---|
  | kick, bd, kik, "bass drum" | KICK |
  | snare, sd, clap, rim | SNARE |
  | hat, hh, hihat, hi-hat, cymbal, ride, shaker | HAT |
  | bass, sub, 808 (not "bass drum") | BASS |
  | master, main, mix, bus | MIX |
  | anything else | keep the current role (default MIX) |

- The rule applies only while `roleAuto` = true (the default).
  - A manual pick sets `roleAuto` = false.
  - AUTO sets it back and re-applies.
  - `role` stays the host parameter, so automation still works (and turns AUTO off).
- A second MIX instance on "Pads" is harmless once the engine uses the lead's MIX as the stable mix (routing research A.2). Until then the SOURCE view shows a `tungsten` hint: "Two MIX sources: pick what's on this track."

---

## 5. Host parameters

### 5.1 Existing IDs: all 57 stay, unchanged

`role`, `sensitivity`, `trim`, `normalizer`, `sendControls`, `macro1`…`macro8`, `preset`, `blackout`, `grain`, `crush`, `flash`, `glitch`, `trails`, `symbols`, `cutRate`, `smear`, `halation`, `weave`, `dust`, `blacks`, `reactivity`, `calm`, `morphTime`, `palette`, `reactKick`, `reactSnare`, `reactHat`, `reactBass`, `reactLevel`, `drift`, `push`, `softness`, `sync`, `reverse`, `freeze`, `useMedia`, `mediaSlot`, `clipMode`, `clipSync`, `shots`, `hud`, `lookahead`, `hit`, `snap1`…`snap4`, `scenePrev`, `sceneNext`, `sceneGo`.

- Names, ranges, defaults and creation order are unchanged.
- New parameters are **appended**. JUCE's VST3 IDs are hashed from the string IDs (no `JUCE_FORCE_USE_LEGACY_PARAM_IDS` in `plugin/CMakeLists.txt`), but appending is the zero-risk choice for Live's automation, MIDI maps and Configure lists.
- Two meanings widen, compatibly:
  - `sendControls` now means "may be elected lead". On is still the default, and an instance that had it off still never sends.
  - `macro5` also scales the continuous following below 25 % (§5.3). Sets with Impact at 25 % or more behave exactly as before.

### 5.2 New parameters

| ID | Name in Live | Type and range | Default | What it does |
|---|---|---|---|---|
| `lookPreset` | Look | Choice: Custom, Clean, Film, Worn, Broken, Print, Data | **Film** (index 2) | On change to a named look, writes the look vector (§5.4) into the 13 look parameters. Shows Custom after any edit |
| `lookAmount` | Look Amount | Float 0-1, shown as % | **1.0** | Multiplies 13 look slots on the way to the engine. The stored knob values are untouched |
| `reactStyle` | React Style | Choice: Custom, Breathe, Pulse, Punch | **Pulse** (index 2) | On change to a named character, writes `reactKick…reactLevel`, `softness`, `push` (§5.4). Shows Custom after any edit of those |

**Budget:**
- 57 + 3 = **60**.
- Two slots are reserved: `focus` "Listen Focus" (routing research, Option B) and one for REACTION-DESIGN's calibration mode if it must be automatable. That gives 62.
- Live's limit before a plug-in opens "with an empty panel" is 64.

**Not host parameters** (plug-in state, saved with the set):

| State | Type / default | Purpose |
|---|---|---|
| `roleAuto` | bool, true | §4.3 |
| `leadPinned`, `leadPinTime` | n/a | §4.2 |
| `momentName0..3` | strings; default = auto recipe | Moment names |
| `sceneCache` | n/a | §2.3 |
| `drawerOpen`, `drawerTab`, `uiScale` | n/a | Window and drawer memory |

### 5.3 How merged controls drive the old ones

All of this runs on the message thread in `VJAnalyzerProcessor::timerCallback` (30 Hz). It **polls** rather than adding listeners, so it is thread-safe and simple.

```cpp
// 1) Presets that WRITE old parameters (look, react style)
if (auto lp = choice ("lookPreset"); lp != lastLookPreset && ! morph.active)
{
    if (lp != customIndex) writeVector (lookVectorIds, lookVectors[lp], /*morphBeats*/ 1.0); // via the Morph helper
    lastLookPreset = lp;
}
else if (lp != customIndex && ! morph.active && ! writing && ! matches (lookVectorIds, lookVectors[lp], 0.005f))
    setChoiceSilently ("lookPreset", customIndex);    // a knob was edited -> Custom (also updates lastLookPreset)
// identical block for reactStyle over { reactKick..reactLevel, softness, push }, written instantly.

// 2) Controls that SCALE on send (stored values never change)
const float amount = param ("lookAmount");
for (auto id : { grain, crush, flash, glitch, trails, symbols, smear, halation, weave, dust, blacks })
    c.look[slot (id)] = param (id) * amount;
c.look[14] = param ("shots") * amount;
c.look[15] = param ("hud")   * amount;
c.look[cutRate] = param ("cutRate");                       // never scaled: it switches scenes

// 3) IMPACT is the one reaction amount
const float impact = param ("macro5");                     // hits: engine scales them by 2 x impact (unchanged)
c.look[12] = param ("reactivity") * juce::jlimit (0.0f, 1.0f, impact / 0.25f); // flow fades out below 25 %
c.look[13] = param ("calm") > 0.5f ? 1.0f : 0.0f;          // STILL

// 4) Lead
c.sendControls = leadRegistry->isLead (*this);
```

**Rules for the writers:**
- **Look writes** go through the existing `Morph` machinery, with a fixed one-beat morph, so a look switch from the Korg never pops.
- **React writes** are instant. The engine already glides closed routes to neutral (`isClosedByReact`).
- **Suppression:**
  - While a write or a moment morph is running, the Custom check is suppressed.
  - A moment recall that sets `lookPreset` or `reactStyle` also sets `lastLookPreset` / `lastReactStyle`, so it never triggers a second write.
- **Fresh instance:** the constructor applies the default `reactStyle` vector (Pulse) once. `lookPreset` Film equals today's look defaults, so it needs no write.
- **Loading a set** (`setStateInformation`, then reconcile):
  - Old sets have no `lookPreset` or `reactStyle`. The plug-in shows the preset whose vector matches the loaded values within 0.005, or else **Custom**, and writes nothing.
  - `lookAmount` defaults to 1.0, so old sets look identical.
- **STILL** (R1 cell 0): toggles `calm` on. A character cell sets `calm` off and `reactStyle` to that character, in one gesture (two parameter changes inside one begin / end gesture).
- **Auto scene cuts** (V3): writes the discrete `cutRate` values listed in §3.5.
- **Role AUTO:** writes `role` when the track name matches (§4.3).
- **Calibration AUTO:** owned by REACTION-DESIGN.md. The UI shows "auto" in the DETECT knob centre and switches to manual when the knob is turned.

### 5.4 Preset vectors (starting values; one tuning session with the owner)

**Look vectors.** Each named look writes these 13 parameters. It never touches `cutRate` or `reactivity`.

| Look | grain | crush | flash | glitch | trails | symbols | smear | halation | weave | dust | blacks | shots | hud |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Clean | .05 | .25 | 0 | 0 | 0 | 0 | 0 | .10 | 0 | 0 | .20 | 0 | 0 |
| **Film** (= today's defaults) | .30 | .35 | 0 | 0 | .10 | 0 | 0 | .35 | .30 | .30 | .35 | 0 | 0 |
| Worn | .50 | .40 | 0 | .05 | .20 | 0 | .15 | .45 | .55 | .65 | .50 | 0 | 0 |
| Broken | .40 | .55 | .30 | .50 | .30 | 0 | .45 | .30 | .40 | .30 | .30 | .35 | 0 |
| Print | .55 | .85 | 0 | 0 | 0 | 0 | 0 | .15 | .20 | .50 | .60 | .15 | 0 |
| Data | .20 | .50 | .15 | .15 | .10 | .50 | .10 | .20 | .10 | .10 | .30 | .40 | .60 |

- "Print" today is hard two-tone with grey paper blacks.
- The Zwobot "Print" pass (invert on grey, REFERENCE-ZWOBOT-SHOWCASE rec. 6c) would later make it truer.

**React Style vectors.** The names and values are owned by REACTION-DESIGN.md; these are placeholders consistent with its direction.

| Character | reactKick | reactSnare | reactHat | reactBass | reactLevel | softness (decay) | push |
|---|---|---|---|---|---|---|---|
| Breathe | off | off | off | on | on | .80 (x2.6) | .15 |
| **Pulse** | on | off | off | on | on | .50 (x1.4) | .30 |
| Punch | on | on | on | on | on | .20 (x0.8) | .50 |

- Today's defaults (all on, .5, .3) load as Custom in existing sets. That is correct and harmless.
- A fresh instance starts on Pulse.

### 5.5 Moments (snapshots) store more

Extend `snapshotParamIds()`:

- **Continuous** (morphed): `lookAmount` and `reactKick…reactLevel`. The bools switch when the morph passes half-way.
- **Discrete** (switched half-way, like `palette`): `lookPreset` and `reactStyle`. They must **not** be interpolated.

Old stored snapshots lack these keys and keep the current values (the existing `getProperty (id, current)` fallback). `calm`, `freeze` and `blackout` stay out of moments: they are performance toggles, not a state of the picture.

---

## 6. Cognitive walkthrough (before vs after)

- **Clicks** = discrete mouse actions (a drag counts as 1; picking a file in a dialog counts as 1).
- **"What is this?"** = a moment where the owner must stop to work out which control to use, or what just happened.
- **Before** = the current build (current-ui.png, commit 633c551).

| # | Task | Before: path | Clicks | ? | After: path | Clicks | ? |
|---|---|---|---|---|---|---|---|
| 1 | Start and see visuals | OPEN ENGINE → FULLSCREEN | 2 | 3 | Red START ENGINE pill → ⤢ | 2 | 0 |
| 2 | Pick and switch a scene | Click a name in the list → GO (or double-click) | 2 (1) | 1 | Click a tile → GO (or double-click) | 2 (1) | 1 (0 with thumbnails) |
| 3 | Make it react more or less | Drag Impact… | 1 | 3 | Drag IMPACT; optionally one click on BREATHE / PULSE / PUNCH | 1 (+1) | 0 |
| 4 | Calm it before a quiet section, then back | CALM → CALM | 2 | 1 | STILL → the underlined character | 2 | 0 |
| 5a | Change the colour | Palette dropdown → a name | 2 | 1 | COLOUR ▾ → a row with stripes | 2 | 0 |
| 5b | Change the look ("more worn") | Drag some of Grain, Dust, Weave, Blacks, Erode | 3-5 | 3 | LOOK ▾ → Worn (+ AMOUNT drag) | 2 (+1) | 0 |
| 6 | Load my own image and see it | LOAD → pick the file | 2 | 2 | Drop the file on the window (or LOAD… → pick) | 1 (2) | 0 |
| | **Total (core paths)** | | **14-16** | **14** | | **12** | **1** |

**The "what is this?" moments, before:**
1. **Start and see visuals:**
   - The first thing next to OPEN ENGINE is a row MIX KICK SNARE HAT BASS TEXTURE: "do I pick one?"
   - The pill "NO ENGINE - start VJ Engine" and the OPEN ENGINE button say the same thing twice.
   - The scene list is an empty "-" until the engine connects.
2. **Pick and switch a scene:** names only, no pictures.
3. **React more or less:**
   - Seven candidates: Hit Sens, Trim, Impact, Softness, Reactivity, Push and five chips.
   - Impact at 0 still leaves the flow moving: "is it broken?"
   - Reactivity and Impact sound the same.
4. **Calm it:** CALM vs FREEZE vs Reactivity: "does CALM stop the motion?"
5. **Colour and look:**
   - The palette dropdown shows names without colours.
   - Which of 14 knobs is "worn"? Blacks and Weave are unclear names, and Erode (a macro) competes with them.
6. **Load an image:**
   - Slots 1-8 are numbers without content.
   - USE MEDIA: what is its scope?
   - (The scene changed by itself: already explained since 8e6a735, only in a 10.5 px line.)

**Why "after" is lower:**
1. Only one red button exists when the engine is off, and the grid shows cached names.
2. Unchanged; the switching model was already right (owner). Scene thumbnails (competitor research R2) remove the last moment.
3. One knob, whose 0 truly means "ignores the music". The character row and the lamps show *what* it follows.
4. STILL sits inside the row that already says how the picture follows the music. The lamps go dim, and the info line says "fading out over one bar".
5. Colours are shown as colours. Looks are named by result, not by mechanism.
6. Dropping the file is the obvious gesture. The plug-in switches to a scene that shows it and says so (memory "show results immediately").

**Remaining risk:** the first time, the owner may not expect IMPACT to calm the flow below 25 %. The info line says it on hover. If it still surprises him, relabel K5 as REACT (open question 2).

---

## 7. Visual language and JUCE notes

### 7.1 Tokens (from DESIGN-DOSSIER.md §5.4, unchanged)

`vjui` colours are replaced one for one. mint, magenta and violet are **removed**.

| Token | Hex | Use here |
|---|---|---|
| `ink-0` | #0C0A09 | Window background |
| `ink-1` | #151210 | Header, tiles, buttons, drawer header |
| `ink-2` | #1E1A17 | Hover, toggled-on, LIVE tile |
| `line` | #3A322C | Hairlines, knob tracks, idle outlines |
| `bone` | #ECE4D6 | Text, value arcs, selected segments (fill), active outlines |
| `ash` | #A89E90 | Labels, sub-labels |
| `dust` | #8A7F73 | Hints, info line, disabled, ignored lamps |
| `signal` | #F0503F | **Live / output only:** LIVE tag and tile, engine dot, hit lamp flash, BLACKOUT, START ENGINE, lookahead warning |
| `tungsten` | #E9A450 | **Pending only:** cue field, NEXT badge, GO when cued, STILL fading, loading, SAVED, "NO LEAD" |
| `halation` | #F2937F | Knob modulation rings, LEVEL bar, morph progress, activity bars |
| `remote` | #6FC3D6 | The "moved by MIDI / automation" dot (and the ID tags in the wireframes, which are not UI) |

**Rules:**
- In normal play at most three `signal` elements are visible: the engine dot, the LIVE tag and the LIVE tile. The hit lamp is a 200 ms flash.
- State is never colour alone. It always has a word (LIVE, NEXT, BEAT, SAVED, OUTPUT DARK), a dashed outline or a hollow lamp.

**Type:**
- IBM Plex Sans Condensed, 9-20 px; captions are 9 px SemiBold caps, tracked +1.1.
- IBM Plex Mono for every number.
- Both OFL, embedded via BinaryData.
- Minimum 9 px at 100 %. (Bahnschrift SemiCondensed ships with Windows 10 / 11 and is an acceptable stop-gap for a first build.)

**Shapes:**
- 2 px radius, 1 px hairlines.
- No shadows, glows, gradients or blur.
- Knob: 270° track in `line` + value arc in `bone` (3 px, or 2.5 px small), with no pointer. Centre disc `ink-1` with the mono value. Outer 2 px `halation` modulation arc.

### 7.2 Components to build (all cheap: fills, 1 px paths, cached text)

| Component | Notes |
|---|---|
| `vjui::LookAndFeel` | Override: `drawRotarySlider`, `drawLinearSlider` (C3, L2), `drawButtonBackground` / `drawButtonText`, `drawToggleButton`, `drawPopupMenuBackground` / `drawPopupMenuItem` (palette rows with stripe chips), `drawTooltip`, `drawScrollbar` |
| `MacroKnob` (existing) | Resize to Ø56 (PLAY) / Ø36 (drawer). Draw the value in real units via `getCurrentValueAsText()` (the parameters already have string functions); "off" at 0. Add the remote dot: compare against the editor's own drag gestures |
| `SceneGrid` | **One** component that paints all tiles and hit-tests the mouse (not 16 buttons). It repaints only when the list, cue, fired or live state changes, plus the cued tile at 10 Hz |
| `CueBar` | S5 display + click to cancel |
| `SegmentedControl` | Generic; used by R1, Q1, V3, U2, U4, D3, U6 |
| `LampStrip` | R2; repaints its own 264 x 16 only |
| `PickerButton` | C1 and C2; opens a `PopupMenu` with custom-drawn items |
| `MomentTile` x 4 | Right-click menu; inline rename via a `TextEditor` on double-click of the name |
| `InfoLine` | The 30 Hz timer reads the component under the mouse (`Desktop::getMainMouseSource().getComponentUnderMouse()`), walks up the parents to the first non-empty help text, and repaints only on change |
| `Drawer` | A custom tab strip + 6 tab components; static parts use `setBufferedToImage (true)` |
| `SourceView` | Its own component. The editor swaps PLAY / SOURCE and calls `setSize` |

**Remove:**
- the per-tick `setAlpha` loop over about 60 components (dossier §6.5)
- the always-visible spectrum repaint (it now paints only while the SETUP tab is visible)

**Window:**
- `setSize (760, 480)` (PLAY), `(1180, 480)` (EDIT) or `(360, 240)` (SOURCE); not user-resizable.
- UI size (75-150 %) uses a transform on a content component + `setSize` scaled. Live's own HiDPI auto-scale stays in charge at 100 %.
- **Test once in Live 10:** the plug-in-initiated resize (VST3 `resizeView`) when EDIT opens.

**Verification:** `plugin/build/PluginHarness_artefacts/Release/PluginHarness.exe <out.png> 3` screenshots the UI with no engine. Add harness modes for the EDIT tabs and for SOURCE (the existing `--cuetest` finds `sceneList` by component ID; keep that ID on `SceneGrid`).

### 7.3 Suggested build order (about 6 engineer-days)

1. **Processor (1 day):**
   - the 3 new parameters, the writers, the reconcile on load, the Impact → reactivity derivation and the look-amount scaling
   - the lead registry + `sendControls` wiring
   - `roleAuto`
   - the moments extension
2. **LookAndFeel, tokens and the knob restyle (0.5 day).**
3. **PLAY (1.5 days):** SceneGrid, CueBar, the REACT row + lamps, STYLE, the media row + drag and drop, MOMENTS, the info line, the scene cache.
4. **EDIT drawer, six tabs (1.5 days).**
5. **SOURCE view + track-name role + MAKE LEAD (1 day).**
6. **Harness screenshots of every state in §2.2, then one session in Live on the owner's laptop (0.5 day).** Follow the six tasks of §6 with his own files.

---

## 8. Alignment with REACTION-DESIGN.md, and open questions

REACTION-DESIGN.md had not appeared by the time this was finished (checked at the start and end). This map assumes:

1. **One REACT amount = the IMPACT knob (`macro5`).** The owner requires the 8 macros on PLAY, and Impact is already the reaction macro, so the amount lives in that slot. Two amount knobs on PLAY would recreate dossier confusion 1.
   - If REACTION-DESIGN defines a new amount parameter, it should drive K5's slot (and the processor derivation in §5.3) rather than add a ninth knob.
2. **Character switch = BREATHE / PULSE / PUNCH**, stored in the new `reactStyle`. Its vectors are owned by REACTION-DESIGN (§5.4 has placeholders).
   - **STILL** (= `calm`) is added as the first cell, because "calm it" is a performance gesture that belongs in the same row.
3. **Automatic calibration** replaces Hit Sens, Trim and the normaliser as front controls. The parameters remain as manual overrides in SETUP and in the SOURCE view, with "auto" shown in the DETECT knob centre.
4. **The REACT TO toggles** become part of the character, with overrides in EDIT › REACT (FOLLOWS). The PLAY lamps show which channels the character follows.

**Open questions for the owner:**
1. **Look names:** Clean / Film / Worn / Broken / Print / Data. Are these the right words? The vectors need one 20-minute tuning session with him.
2. **Knob 5 label:** keep **IMPACT** (his current word), or rename it **REACT** now that it is the one reaction amount? It is a display change only; the Live name stays "Impact".
3. **Pulse without snare and hat:** is that right as the default feel for ambient / downtempo, or should Pulse include every hit?
4. **Moments and scenes:** should a moment also remember the scene (cueing it for GO)? This is the first step towards a Cue Bank for the dance show.
