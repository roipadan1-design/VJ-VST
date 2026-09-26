# Reaction design: making the sound move the picture

*26 September 2026. Design only: no source, preset or shader was changed. Written after the owner's first test inside Ableton Live. The parallel forensic measurement of "old vs new" is `docs/product-design/REACTIVITY-FORENSICS.md` (in progress when this was written). This document does not repeat those measurements. It treats the code reading in section 1 as design input only.*

---

## בקצרה (בשבילך)

1. **למה זה הרגיש חלש.** בגרסה הקודמת כל קיק שינה משהו שנשאר: הסצנה "התערבבה" מחדש, והבס דחף את התנועה קדימה. את זה הורדנו כדי להיפטר מהקפיצות. מה שנשאר הוא הבזק שנעלם תוך 2-3 פריימים, ותזוזות קטנות שנראות כמעט קבועות.
2. **העיקרון החדש, מתוך 57 הקליפים של Zwobot:** בשקט המסך נח ומחשיך, ועל המכה קורה אירוע. לא כל מכה מקבלת אירוע, רק המכות החשובות.
3. **שלוש שכבות זמן.** **מכה:** אירוע גדול וברור. **ביט ומשפט:** נשימה, מסה ובס. **קטע:** מנוחה, בנייה ודרופ.
4. **לכל סצנה "דקדוק" משלה:** איך היא נראית במנוחה, מה קורה במכה, מה הבס עושה לה, ומה קורה לה בבנייה ובדרופ.
5. **במקום 7 כפתורי תגובה יש כפתור אחד, REACT.** 0 = לא מגיב, 50 = כמו שתוכנן, 100 = פראי. הוא נכנס למקום של Impact, בתוך 8 הכפתורים.
6. **לידו מתג אופי עם שלוש אפשרויות:** **BREATHE** (נושם, לאמביינט ולמחול), **PULSE** (כל ביט מורגש, ברירת המחדל) ו-**PUNCH** (מכות חדות).
7. **כיול אוטומטי:** המערכת לומדת את הטווח של כל שיר. כך התמונה אף פעם לא מתה, ואף פעם לא נתקעת למעלה.
8. **מה עובר ללשונית מתקדמת:** Hit Sens, Trim, Adaptive, Reactivity, Softness ו-Push. **CALM ו-LISTEN TO נשארים.**
9. **ארבעה דברים חדשים:**
   - **דרופ:** אירוע גדול אחד.
   - **מנוחה בשקט:** המסך מחשיך, אבל הגרעין ממשיך.
   - **"דחיפת זמן" על מכה:** הסצנה מתקדמת קדימה בתנועה חלקה ולא בקפיצה.
   - **"צלקות":** סימנים שמצטברים לאורך קטע ונמחקים בדרופ.
10. **ההמלצה:** חבילה של 3-4 ימי עבודה שנותנת את רוב השיפור. אחריה: Scope (הגל האמיתי כקו), "אדום רק על המכה", ו-GO שמחכה לדרופ.
11. **שתי שאלות אליך:** באיזה אופי להתחיל? אני ממליץ על PULSE, ועל BREATHE למופע המחול. והאם מותר לנו שהמסך יחשיך בשקט עד בערך רבע מהבהירות?

---

## Contents

1. [Why the reaction lost significance (design reading)](#1-why-the-reaction-lost-significance-design-reading)
2. [What makes a reaction feel significant](#2-what-makes-a-reaction-feel-significant)
3. [The model: three time scales and a Reaction Bus](#3-the-model-three-time-scales-and-a-reaction-bus)
4. [Control: one REACT knob and a CHARACTER switch](#4-control-one-react-knob-and-a-character-switch)
5. [Per-scene reaction grammar (16 scenes)](#5-per-scene-reaction-grammar-16-scenes)
6. [Out-of-the-box ideas, rated](#6-out-of-the-box-ideas-rated)
7. [Recommended package (about 3.5 days)](#7-recommended-package-about-35-days)
8. [Risks and open questions](#8-risks-and-open-questions)
9. [Sources](#9-sources)

---

## 1. Why the reaction lost significance (design reading)

This compares the preset generator before the control redesign (`git show 09279ba:engine/tools/build_instrument_presets.py`) with today's version, plus `Modulation.cpp`, `Motion.h` and the analyser. The forensic document measures the numbers. The design conclusion is qualitative, and it is what matters here.

| Mechanism | Before (09279ba) | Now | What it did to the feel |
|---|---|---|---|
| **Reseed on every kick** | Every scene: `KICK_ACTIONS` included `reseed` | Only Negative (`reseed_on_kick=True`) | Before, each kick made a **lasting change**: the layout moved to a new state and stayed there. It read as a jump, but also as "the music did something". |
| **Band clocks** (bass → rate) | Audio routed into the integrated rates of 10 scenes: bass in 9, mid in Terminal (`hot_blobs.flow 0.35`, `fibers.drift 0.4`, `signal_fog.clock 0.4`, `mesh.breathe 0.4`, `corridor.travel 0.2`, `negative cam.orbit 0.12`…) | None (rule: nothing routes into a rate). Global Push 0.3 instead: at most x1.3, driven by `0.6·bassRel + 0.4·levelRel` | The music no longer visibly **drives** the motion. |
| **Hit envelopes** | Kick decays to 1 % in 180 ms | 250 ms x Softness 1.41 ≈ 354 ms | The envelope is exponential (`exp(-4.6 t / decay)`), so the half-life is 27 ms before and 53 ms now. At 60 fps a hit is at half strength after 2-3 frames. It is a **spike**, not an event. |
| **Hit size** | Impact default 0.5 (gain x1.0) | Impact 0.4 (x0.8), and every trigger is scaled by `0.4 + 0.6 · strength` | Example: Mesh Body `env.kick → surge 0.6` gives surge ≈ 0.6 x 0.8 x 0.64 ≈ 0.31. In the shader that is `radius · (1 + 0.12 · 0.31)`, **+3.7 %**. Invisible. |
| **Stacked reactions** | Several audio sources per parameter (Corridor surge = kick + swell; Dot Relief depth = bass + swell + build) | One audio source per parameter | Fewer "double hits". This is correct, but combined with the rows above it removed the last strong moments. |
| **Continuous values** | Same analyser | Same | `AdaptiveNormalizer` holds a span of **at least 24 dB**. A comment in `Analyzer.cpp` notes that on a mastered track kicks rise only 3-5 dB over the bass line, so `bassRel` swings by roughly 0.1-0.3. The routes use `center 0`, so the result is mostly a **constant offset plus a small wiggle**. |
| **Rest state** | None | None | Silence and music look almost the same. There is no contrast to react against. |

**Conclusion.** The old version felt more significant for two reasons, not because its hits were bigger:
- each kick **changed something that stayed changed** (reseed), and
- the **bass drove the motion** (band clocks).

The redesign rightly removed the jumps and the runaway speed, but it removed persistence and drive along with them. The new model brings persistence and drive back **without jumps**:
- **selected accents** that have a lasting consequence
- **time kicks**: an eased lurch of the scene clock that never snaps back and never accumulates speed
- **contrast-stretched body and energy** that feed Push
- a **rest state**, so there is something to react against

---

## 2. What makes a reaction feel significant

These eight principles are the design rules for everything that follows. The evidence comes from the owner's Zwobot showcase analysis (`docs/REFERENCE-ZWOBOT-SHOWCASE.md`) and from pages opened for this document (section 9).

1. **Contrast before amplitude.** A hit is only as visible as the rest it interrupts.
   - In the showcase, 6 sound-reactive clips (44, 49, 50, 55, 56, 57) are near-black **70-94 % of the time** and wake up on hits.
   - Clip 55 has the clearest grammar: silence = a quiet grid, hit = an explosion.
   - Clip 31 shows the failure mode: an uncalibrated SR module, so the screen is dead.
   - **Rule:** every scene has a designed REST look, darker and sparser, with a floor.
2. **Selection: not every hit.**
   - The showcase has three motion regimes: flow (no cuts), hits (1-2 events a second) and chaos (more than 4 a second).
   - The owner's taste matches "flow with single events on hits".
   - **Rule:** an accent selector promotes only the meaningful hits to full events. The others become small ticks, or nothing.
3. **Persistence.** An event must be seen at full strength for at least 2-3 frames, and ideally leave a trace.
   - The old reseed left a trace, and so do Morphogen's seeds.
   - A time kick leaves the scene further along.
   - **Rule:** hit envelopes get a **hold**, and each scene gets at least one consequence that outlives the flash.
4. **Anticipation, action, follow-through** (the animation principle).
   - A build should *inhale*: compress, tighten, fray, close a letterbox.
   - The drop *exhales*: release, bloom, lurch.
   - **Rule:** tension drives "inward" parameters, and the drop releases them.
5. **Three time scales, each with its own job.**
   - Synesthesia's audio uniforms are the cleanest public model of this:
     - **Hits** spike on isolated transients.
     - **Presence** follows the overall rise and fall of activity, not the individual sounds.
     - **Time** clocks move forward faster when the music is louder.
     - **Intensity** slowly accumulates.
   - Our model keeps that separation and adds the section scale (rest, build, drop).
6. **Asymmetric lag.** Derivative's Audio Analysis 2.0 puts it as "Fast 'attack' for impact, smooth 'decay' for visual flow".
   - **Rule:** attack happens on the frame of the hit, release is shaped by the CHARACTER.
7. **The same control signal for sound and image.**
   - Robert Henke describes Lumière's sound and lasers as fed by the same control signals.
   - For us that means MIDI accents, a Scope made from the real waveform, and a manual DROP. Section 6 covers all three.
8. **Never dead, never clipped.**
   - Each signal is auto-ranged against its own recent history.
   - Hits are ranked against recent hits, not against a fixed threshold.
   - There is a breathing floor when nothing is detected.
   - The rest state has a minimum brightness, so the screen never dies.

---

## 3. The model: three time scales and a Reaction Bus

### 3.1 The three time scales

| Scale | Duration | Signals (new names) | Drives visually | Rules |
|---|---|---|---|---|
| **HIT** | 10 ms to 1.5 s | `event.accent`, `event.tick`, `env.hit`, `env.snare`, `env.pulse`, **time kick** | **Structure**: tear, surge, flare, rip, storm, seeds, stones; a lurch of the scene clock | Sparse and big. Attack within one frame, then a hold of at least 2 frames. Selected, not every onset. At most 3 per second (photosensitivity guard, as for Flash and Shots today). |
| **BEAT / PHRASE** | 1 beat to 8 bars | `react.body`, `react.energy`, `react.air`, `react.breath` | **Mass and surface**: size, density, depth, glow (< 1 stop), fine texture; speed through Push | Continuous, contrast-stretched, smoothed per CHARACTER. Never a step. Never routed into a rate. |
| **SECTION** | 8 bars to minutes | `react.rest`, `react.tension`, `event.drop`, `env.drop`, `react.scar` | **State**: rest look, anticipation, the drop release, accumulated memory | Slow, with hysteresis. One-off events at most every 8 bars. |

Section state machine: `REST ⇄ PLAY`, `PLAY → BUILD` (tension rising), `BUILD → DROP` (a one-off), then `DROP → PLAY`. `PLAY → REST` happens after a quiet hold. The first hit after REST is always an accent.

### 3.2 Where it lives: `ReactionShaper` (new, engine)

It is a new class in `engine/Source/Reaction.h/.cpp`. It runs once per frame in `MainComponent::renderOpenGL`, **after** `applyReactMask` and `clock.update`, so the LISTEN TO gates and the beat clock are already resolved. It writes a `ReactionState` into `Signals` and appends three new event types to `signals.events`.

```cpp
// Signals.h - appended at the end of the enum so existing indices stay valid
enum class EventType { kick, snare, hat, bassTransient, midTransient, highTransient,
                       midiNote, userTrigger, accent, tick, drop, count };

struct ReactionState            // 0..1 unless noted, written every frame
{
    float body = 0, energy = 0.5f, air = 0, breath = 0.5f;
    float tension = 0, rest = 0, scar = 0;
    float dropEnv = 0;          // the global drop envelope (look pass, clock)
    float contAmount = 1, hitAmount = 1;   // REACT curves x React trim x CALM fade
    float exposure = 1;         // rest floor x drop bloom (look pass)
    int style = 1;              // 0 BREATHE, 1 PULSE, 2 PUNCH
};
```

### 3.3 Continuous signals: auto-ranged ("stretched")

Every continuous signal goes through the same auto-range. It keeps its own floor and ceiling, so a mastered track uses the whole 0-1 range and a quiet track does too.

```
Stretch(x, dt, floorUpTau, ceilDownTau, minSpan, gamma):
  floor = x < floor ? follow(floor -> x, 0.3 s)  : follow(floor -> x, floorUpTau)   // tracks the troughs
  ceil  = x > ceil  ? follow(ceil  -> x, 0.02 s) : follow(ceil  -> x, ceilDownTau)  // tracks the peaks
  span  = max(ceil - floor, minSpan)             // minSpan stops noise being blown up
  y     = clamp((x - floor) / span, 0, 1) ^ gamma
  gate: if !anyLive or levelAbs < 0.15 (about -53 dBFS), the target is 0 (glide 300 ms)
```

| Signal | Input | floorUp / ceilDown / minSpan | Then |
|---|---|---|---|
| `body` | `bassRel` (or the kick role's level when a KICK source is live, as today) | 2 s / 6 s / 0.15 | gamma and attack/release per CHARACTER (4.3). Steady bass reads low; bass **pulses** read high. |
| `energy` | `levelRel` | 20 s / 30 s / 0.20 | `energy = 0.5 · levelRel + 0.5 · stretched`, so a steady drone sits near the neutral 0.5 and does not sink to 0. Attack/release per CHARACTER. |
| `air` | `max(highRel, flux)` | 3 s / 8 s / 0.15 | Attack/release per CHARACTER. Hats and texture feed the surface only; they never make accents. |
| `breath` | Clock `barPhase` | none | `0.5 + 0.5 · cos(2π · phase / periodBars)`, which peaks on the downbeat, times `(0.25 + 0.75 · energy)`. Period per CHARACTER. |

`isClosedByReact` in `Modulation.cpp`:
- `body` is closed by the BASS toggle
- `air` is closed by HAT
- `energy`, `breath`, `tension`, `rest` and `scar` are closed by LEVEL

### 3.4 Accent selection ("not every hit")

Candidate hits are the events `kick`, `snare`, `midiNote` and `userTrigger` (after the REACT TO mask). Hats are never candidates.

```
on candidate h (strength s, time t):
  if userTrigger, or (midiNote and velocity >= 100):   emit accent(1.0); continue
  ring.push(s)                               // last 16 candidate strengths
  rank   = fraction of ring below s
  forced = (t - lastCandidate) > max(1 bar, 2 s)                      // "the return" after a gap
        or (style != BREATHE and clock following and |barPhase| < 70 ms and h is kick)   // downbeats
  q      = clamp(q0[style] - 0.5 · (REACT - 0.5), 0.05, 0.95)
  gapOk  = t - lastAccent >= max(minGapBeats[style] · 60/bpm, 0.34 s)
  if (forced or rank >= q) and gapOk:
       emit accent(strength = forced ? max(s, 0.7) : 0.7 + 0.3 · rank); lastAccent = t
  else if t - lastHitEnvTrigger >= 0.34 s:
       emit tick(strength = s)
  (the original kick / snare events stay in the frame, so existing triggers still work)
```

Governor, so the result is never clipped: if candidates arrive faster than 3 per second for 4 s (a busy bass line mistaken for kicks, for example), then `q += 0.15` and the tick level is halved until the rate falls.

### 3.5 REST (silence as an instrument)

```
quiet  = !gateOpen or levelAbs < 0.25 (about -48 dBFS)
      or (levelRel < 0.25 and presence < 0.2 and no candidate for restHold[style])
rest  -> 1 with tau restIn[style] while quiet; -> 0 with tau restOut[style] otherwise
first candidate after rest > 0.5 is a forced accent (the wake-up is an event)
```

What rest does:
- **Globally,** `exposure = mix(1, restFloor[style], rest · restDepth(REACT))`. This is applied in `LookPass` **right after the fetch, before acutance and grain**, so grain, dust and the lifted blacks keep living. The screen is dark, not dead.
- **Per scene,** the `react.energy` route carries the scene into its rest look (section 5).
- `react.rest` is also available to presets.

### 3.6 TENSION and the DROP

- **Tension** is today's `SectionTracker.build` with two additions:
  - An optional roll detector: `max(build, rise of candidate rate over 2 s versus 16 s)`. Snare rolls count as tension.
  - A **reset on drop**: tension goes to 0 with tau = 1 bar, so the release is visible as motion rather than a snap.

```
fastDb = follow(levelDb, 0.3 s); slowDb = follow(levelDb, 8 s)      // levelDb = levelAbs · 48 - 60
primed = tensionPeakLast4Bars >= 0.35 or restWithinLast2Bars
fire drop when:
     fastDb - slowDb >= dropJumpDb[style] - 4 · (REACT - 0.5)
 and primed
 and a kick / bassTransient in the last 250 ms (or a >= 8 dB rise in bassAbs)
 and t - lastDrop >= max(8 bars, 12 s)
 and REACT >= 0.15
emit event.drop(strength = clamp(0.6 + excessDb / 6, 0.6, 1))
on drop: tension -> 0 (tau 1 bar), scar -> 0 (tau 2 bars), rest -> 0 now,
         env.drop fires (shape per CHARACTER), time kick x3, look bloom
manual DROP: plug-in action "Drop" and /v2/trigger "drop" fire the same event
```

### 3.7 SCAR (memory)

```
on accent: scar = min(1, scar + scarPerAccent[style] · hitAmount · strength)
each frame: scar *= exp(-dt / (32 bars)); in rest: tau 4 s (it heals); on drop: -> 0 over 2 bars
```

Globally, `LookPass` uses `dust_effective = dust · (1 + 0.8 · scar)`: the film gets dirtier through a section. The Dust knob still decides whether there is any dust at all. Three scenes also route scar (section 5).

### 3.8 The time kick (hit = motion, not a jump)

This is added to `SceneClock` (`Motion.h`). On an accent, the scene clock advances by a fixed amount of *designed motion*, delivered with an ease. Every integrated rate and every `vj_time` shader moves a little further, smoothly, and never snaps back.

```cpp
void kick (double beats, double tauSeconds, double bpm)
{
    pending = std::min (pending + beats * 60.0 / bpm, 60.0 / bpm);   // cap: 1 beat pending
    kickTau = tauSeconds;
}
// in update(), after frame.speed is known:
const double step = pending * (1.0 - std::exp (-dt / kickTau));
pending -= step;
frame.sceneDt = frame.speed * dt + step * base;   // base = the knob speed (signed; 0 when frozen)
frame.sceneTime += frame.sceneDt;                 // sceneBeat unchanged: LFOs keep their phase
```

- **No runaway.** The displacement per hit is fixed, and accents are at most 3 per second with a minimum gap of half a beat or more. Over any 2 s the extra motion is at most about 0.9 times the designed speed (PUNCH at REACT 100).
  - Push is at most x1.84 in that corner (`0.6 x 1.4`), so the combined worst case is about x2.7.
  - A combined governor in `SceneClock` scales the kicks down whenever the 4 s average speed exceeds x2.5 of the knob speed.
- **"0 = still" holds.** Speed 0 or Freeze gives `base = 0`, so there is no lurch.
- **Hits still never touch a rate parameter.** The generator rule stays.

### 3.9 The global look layer (every scene)

These are added to `LookPass` and driven by the bus:
- **Exposure** (`exposure` uniform, applied right after the fetch): the rest floor, times `2^(bloomStops · dropEnv · min(1, hitAmount))` for the drop bloom.
- **Breath:** in the MOVE Drift camera transform, `z *= 1 + breathZoom[style] · (breath - 0.5) · 2 · contAmount`. This applies even when Drift is 0, so the guard must move. The whole picture inhales toward the downbeat.
- **Accent-driven cut grammar.** The Flash, SHOTS and auto-cut "kick" mode (`PresetManager::updateAutoCut`) listen to `event.accent` instead of `event.kick`. Their probability scales with `min(1, hitAmount)`, replacing `reactAmount`.
- **Light leak** (the optional stretch in section 7).

---

## 4. Control: one REACT knob and a CHARACTER switch

### 4.1 What the owner sees

- **REACT**: knob 5 of the 8 (it replaces Impact; the same host parameter `macro5`, so saved MIDI maps survive).
  - It shows a word, not a number: `still` (0) · `breathing` (1-15) · `gentle` (16-40) · `as designed` (41-60) · `strong` (61-85) · `wild` (86-100).
  - Its outer ring pulses with every accent and turns grey during REST.
- **CHARACTER**: a 3-way switch under REACT:
  - **BREATHE:** the music breathes the image, and only big moments land.
  - **PULSE:** every beat is felt, and accents land. This is the default.
  - **PUNCH:** tight hits and cut-ready.
- **CALM** (kept): REACT glides to 0 over one bar, and back.
- **LISTEN TO** (kept): *what* the picture follows. REACT and CHARACTER set *how much* and *how*.
- **DROP** (new, momentary): fires the drop by hand. It can be mapped to MIDI and automated. This matters for the dance show.
- **State lamps** in the LISTEN strip: `REST`, a thin `TENSION` bar and a `DROP` flash. They answer the dossier's question "what is the sound doing right now?" (DESIGN-DOSSIER 4.7).

### 4.2 The REACT curve (0-100 → internal amounts)

The formulas are exact, so an engineer can implement from them. The table gives values at the listed points; interpolate linearly between rows or use the formulas.

- `contGain(r)` = `r <= 0.5 ? 1 - (1 - 2r)^2 : 1 + (r - 0.5)`. It scales continuous routes (body, energy, air, breath) and the Push drive.
- `hitGain(r)` = `r < 0.1 ? 0 : r <= 0.5 ? smoothstep((r - 0.1) / 0.4) : 1 + 1.6 (r - 0.5)`. It scales the hit, snare, pulse and drop envelopes, time kicks, and look events.
- `accentShift(r)` = `-0.5 (r - 0.5)`. It is added to the CHARACTER's accent quantile, so more REACT means more hits become accents.
- `pushFactor(r)` = `r < 0.5 ? 2r : 1 + 0.8 (r - 0.5)`.
- `restDepth(r)` = `min(1, r / 0.3)`.
- `dropShiftDb(r)` = `-4 (r - 0.5)`. The drop is off below REACT 15.

| REACT | Word | contGain | hitGain | accent shift | pushFactor | restDepth | drop threshold shift |
|---|---|---|---|---|---|---|---|
| 0 | still | 0 | 0 | +0.25 | 0 | 0 | off |
| 10 | breathing | 0.36 | 0 | +0.20 | 0.20 | 0.33 | off |
| 25 | gentle | 0.75 | 0.32 | +0.125 | 0.50 | 0.83 | +1.0 dB |
| 40 | gentle | 0.96 | 0.84 | +0.05 | 0.80 | 1.0 | +0.4 dB |
| **50** | **as designed** | **1.00** | **1.00** | **0** | **1.00** | **1.0** | **0** |
| 60 | strong | 1.10 | 1.16 | -0.05 | 1.08 | 1.0 | -0.4 dB |
| 75 | strong | 1.25 | 1.40 | -0.125 | 1.20 | 1.0 | -1.0 dB |
| 90 | wild | 1.40 | 1.64 | -0.20 | 1.32 | 1.0 | -1.6 dB |
| 100 | wild | 1.50 | 1.80 | -0.25 | 1.40 | 1.0 | -2.0 dB |

The design intent of the curve:
- **0 to 10:** only breathing and body come in, and hits stay off.
- **10 to 50:** hits fade in, and REST deepens up to 30.
- **50:** the designed look.
- **Above 50:** more hits are promoted to accents, events get bigger (up to x1.8), and Push leans harder.

### 4.3 The CHARACTER table

Beats are the host's; times are in ms unless noted. Two conversions:
- **Half-life to preset `decayMs`** (the time to 1 % in today's envelope): `decayMs = halfLife x 6.64`.
- **Softness to decay trim:** Softness (expert) multiplies every hold and half-life by `trim = 0.5 · 8^softness / 1.414`. That gives x0.35 at 0, **x1.0 at the default 0.5**, and x2.83 at 1.

| Parameter | BREATHE | PULSE (default) | PUNCH |
|---|---|---|---|
| Intent | the image breathes; only big moments land | every beat is felt; accents land | tight hits, cut-ready |
| Accent quantile `q0` (candidates ranked below it become ticks) | 0.85 | 0.60 | 0.25 |
| Forced accents | return after a gap, MIDI vel ≥ 100, HIT | + kick on the bar's downbeat | + kick on the bar's downbeat |
| Minimum gap between accents | 2 beats | 1 beat | 0.5 beat (never under 0.34 s) |
| `env.hit` attack / hold / half-life | 80 / 0 / 650 (decayMs 4316) | 0 / 33 / 200 (1328) | 0 / 50 / 90 (598) |
| Tick level (ticks retrigger `env.hit` at this amount) | 0 (no ticks) | 0.30 | 0.50 |
| `env.snare` level / half-life | 0.25 / 300 | 0.50 / 120 | 0.80 / 70 |
| `env.drop` attack / hold / half-life | 400 ms / 1 bar / 2 bars | 0 / 1 beat / 1 bar | 0 / 2 beats / 2 beats |
| `body` attack / release, gamma | 150 / 900, 0.8 | 30 / 350, 1.2 | 10 / 180, 1.6 |
| `energy` attack / release | 600 / 2500 | 250 / 1200 | 120 / 700 |
| `air` attack / release | 60 / 600 | 30 / 350 | 15 / 200 |
| Breath period / zoom depth | 2 bars / 2.5 % | 1 bar / 1.5 % | 2 beats / 0.8 % |
| Push maximum (`pushC`) | 0.25 | 0.45 | 0.60 |
| Time kick per accent (beats of designed motion / ease tau) | 0.5 / 400 ms | 0.35 / 120 ms | 0.25 / 50 ms |
| Time kick per tick | none | none | 0.08 / 40 ms |
| Rest floor (exposure) / enter after / fade in / fade out | 0.22 (-2.2 stops) / 2.5 s / 2.0 s / 80 ms | 0.32 (-1.6) / 1.5 s / 1.2 s / 60 ms | 0.45 (-1.15) / 1.0 s / 0.6 s / 30 ms |
| Drop threshold (fast - slow level) | +7 dB | +6 dB | +5 dB |
| Drop bloom | +0.5 stop | +0.8 stop | +1.0 stop |
| Light leak on accent (x Halation) / half-life (stretch) | 0.8 / 1200 | 0.5 / 800 | 0.3 / 400 |
| Scar per accent | 0.04 | 0.06 | 0.08 |
| Suggested for | ambient, pads, the dance show | downtempo with drums | peaks, rhythmic sections |

Envelope change needed in `Modulation.cpp`:
- Add `holdSeconds` to `Envelope`. The sequence becomes attack, then hold at the target, then exponential decay.
- Envelopes whose preset entry has `"style": "hit" | "snare" | "drop"` read attack, hold and decay from the current CHARACTER **every frame**, times the Softness trim. A style switch therefore applies at once.
- A trigger action may carry `"scale": "tick" | "snare"`. The engine then multiplies its amount by that style's tick or snare level.

### 4.4 How today's engine parameters are computed

With `r` = REACT (macro slot 4), `c` = CHARACTER, and the expert trims:

| Engine quantity today | New computation |
|---|---|
| `ReactMask.amount` (one number for everything) | Split into `contAmount = contGain(r) x reactTrim x calmFade` and `hitAmount = hitGain(r) x reactTrim x calmFade`. `calmFade` is today's CALM ramp (1 bar). `reactTrim` is the old Reactivity parameter, now an expert trim with default 1. |
| Route gain in `ModulationRuntime::process` | Routes from `audio.*`, `descriptor.*` and `react.*` are multiplied by `contAmount`. `env.*` routes are multiplied by `hitAmount`. **This replaces** `scaleBy: macro.impact` (`2 x Impact`); the generator stops emitting it. Macro and LFO routes are unchanged. |
| `macro.impact` routes (Hot Blobs throw, Negative cuts) | Unchanged. They read macro5 (now REACT) raw, so more REACT means bigger throws and more cuts. |
| `MotionFrame.decayScale` (Softness) | Only affects un-styled envelopes (`pulse`), as today. Styled envelopes use the CHARACTER times x the Softness trim. |
| SceneClock `drive` | `(0.6 · body + 0.4 · energy) x contAmount`, from the bus instead of raw `bassRel` and `levelRel` |
| SceneClock `push` | `clamp(pushC[c] x pushFactor(r) x (push / 0.3), 0, 1)`. The Push parameter becomes an expert trim, and its default 0.3 means x1. Speed never goes above x2 from Push, as today. |
| Time kick | On `event.accent`: `sceneClock.kick(kickBeats[c] x hitAmount x strength, kickTau[c], bpm)`. PUNCH also kicks on ticks. |
| LookPass `reactAmount` (Flash, SHOTS, glitch pulses) | `min(1, hitAmount)`, fired on `event.accent` |
| Auto-cut "every kick" | On `event.accent`, only when `hitAmount > 0.3` |
| Onset sensitivity (Hit Sens) | Unchanged per instance (detection). Accent ranking makes it matter far less. |

### 4.5 What happens to each existing control

| Control (host ID) | New status | Why |
|---|---|---|
| Impact (`macro5`) | **Becomes REACT.** Same ID; the display label is "React". | One "how much" knob, already one of the 8 |
| *(new)* Character (`character`, choice) | **PLAY**, next to REACT; default PULSE; stored in snapshots | The "how" |
| *(new)* Drop (`drop`, momentary action) | PLAY (small) and MIDI-mappable | Manual drop for choreography |
| CALM (`calm`) | Stays in PLAY | Stage emergency; equals REACT → 0 over 1 bar |
| React to Kick/Snare/Hat/Bass/Level | Stay in PLAY as LISTEN TO lamps | "What", not "how much" |
| Reactivity (`reactivity`, look 12) | **Expert** ("React trim", default 1) | Redundant with REACT; kept so automation does not break |
| Softness (`softness`) | **Expert** ("Decay trim", default 0.5 = x1) | CHARACTER sets the decays |
| Push (`push`) | **Expert** ("Push trim", default 0.3 = x1) | CHARACTER and REACT set Push |
| Hit Sens (`sensitivity`) | **Expert / SETUP** ("Detect") | Per-instance detection only; accent ranking absorbs level differences |
| Trim (`trim`) | **SETUP** | The auto-range makes it rarely needed. Show a hint "input very quiet" when levelAbs stays under -50 dBFS instead. |
| Adaptive / Locked (`normalizer`) | **SETUP**, default Adaptive | The stretch sits on top of it |
| Flash, SHOTS, Cut Rate, HUD | Stay in LOOK > CUTS | Now fire on accents and scale with REACT. Rule: **REACT never switches on something the performer turned off.** |

The result is **2 controls** for reaction in PLAY (REACT and CHARACTER), plus CALM and the LISTEN lamps, where there were about 12 before.

### 4.6 Automatic calibration: "never dead, never clipped"

1. **Auto-range** on body, energy and air (3.3). A mastered track and a quiet sketch both use the full range.
2. **Rank-based accents** (3.4). The loudness of the hits no longer decides whether they count; their rank against the last 16 does.
3. **Rate governor** (3.4). A flood of candidates raises the accent quantile. No strobe.
4. **Breathing floor.** When REACT > 0, `breath` gives tempo-locked motion even when no hit is detected (pads-only ambient).
5. **Rest floor.** Exposure never goes below 0.22. Grain, dust, the film base and each scene's rest element stay visible (the Zwobot 31 lesson).
6. **Photosensitivity.** The hit envelope retriggers at most 3 times a second. The drop bloom is at most +1 stop, and the leak is soft and low-frequency.

### 4.7 Protocol and host (backward compatible)

- **OSC (new):**
  - `/v2/style i` (0 BREATHE, 1 PULSE, 2 PUNCH)
  - `/v2/trigger s "drop"`
  - `/v2/status` gains `rest`, `tension`, `accentCount` and `dropCount` for the lamps
  - `/v2/macroActivity[4]` carries the accent envelope, for the REACT ring
- **Plug-in:**
  - Append `character` (AudioParameterChoice) and `drop` (to `actionIds`). Existing IDs never move.
  - Add `character` to `snapshotParamIds()`.
- **Old sets.** macro5 keeps its saved value; Impact 0.4 reads as REACT 40, "gentle". Old Softness and Push defaults now mean x1.

---

## 5. Per-scene reaction grammar (16 scenes)

### 5.1 Notation and shared rules

- `energy → threshold −0.5 @.5` means: route `react.energy` to that parameter with amount -0.5 (in 0-1 parameter range) and centre 0.5. With centre 0.5, **REACT 0 shows today's tuned look** (the neutral pose). Silence carries the scene to its REST look, and full energy carries it to the full look.
- `body`, `air`, `tension` and `scar` use centre 0 (they only add) unless noted. `hit` routes read `env.hit`, which accents fire at 1 and ticks at the tick level.
- **Attack and release** of every `react.*` and `env.*` route are set in the engine by CHARACTER (4.3), so route `attackMs` and `releaseMs` are 0. Exceptions are noted (Terminal fill, One Bit frame).
- The **global drop** (bloom, time kick x3, tension reset, scar reset) and the **global rest** (exposure floor) apply to every scene. The DROP column lists only scene-specific extras.
- **Generator rules** (in `check()`), kept and extended:
  - `react` counts as an audio kind: still at most one audio source per parameter.
  - Nothing routes into an `integrate` parameter.
  - Every scene must have a REST route (`react.energy` or `react.rest`), a HIT route (`env.hit` or `env.pulse`), a BODY route and a BUILD route (`react.tension`).
- **Anti-face rule** (the owner hates faces and eyes): no reaction may **open holes**. Mesh Body's `hole` gets no audio route, and Halo Ring's surge only brightens and grows the ring. It never adds a "pupil".

### 5.2 The table

| # Scene | REST (silence looks like…) and its route | HIT (accent; ticks = same env at tick level) | BODY (bass) | BUILD (tension, the "inhale") | DROP (extra) | Range / default edits |
|---|---|---|---|---|---|---|
| **01 Hot Blobs** | A few dim islands of liquid on black; rings still spread. `energy → threshold −0.5 @.5` (threshold .595 → .465) | Accent fires `env.pulse` (fixed 1/80 ms) `→ drop 1.0`: throw + stone. Throw size = `impact` ← REACT (existing macro route) | `body → emission +0.3 @.4` (0.88 → 1.68) | `tension → ripple +0.5` (the surface grows restless) | `env.pulse` x1 + `env.drop → impact +0.4` (it throws harder through the drop) | none |
| **02 Dot Relief** | A flat, quiet dot grid (Zwobot 55). `energy → depth +0.6 @.5` (.04 → .40) | `hit → surge 1.0` (lift x2.6, dots x1.8, light x2.6 at peak) | `body → dot_size +0.3 @.4` | `tension → tilt +0.35` (the relief rises toward you) | New form now (source advance) + `env.drop → zoom −0.25` (punch in, eases back) | `surge` max 1 → **2**; `depth` default stays |
| **03 One Bit** | Mostly black; only the brightest core and a few speckles. `energy → threshold −0.35 @.5` (.555 → .345). **Remove** `descriptor.centroid → threshold` | `hit → surge 1.0` (punch in, threshold drops) | `body → zoom −0.05` (a small push-in with the bass) | `tension → dither +0.4`, and `tension → frame 1.0` with `inputMin .55`, release 400 ms (the letterbox closes in at high tension; the drop blows it open) | `env.drop → invert 1.0` (negative for about a bar, one-off) | `surge` max → **1.8**. Forms advance on `event.accent` (was `event.snare`) |
| **04 Corridor** | A dark hall dissolving into fog; the near lights barely glow. `energy → fog −0.5 @.5` (fog .59 → .11) | `hit → emission +0.35` (the lights flare), plus the **time kick**: the camera lurches forward | `body → width −0.12 @.4` (walls press in with the bass) | `tension → flicker +0.6` (the lights stutter) | `env.drop → sway_amt +0.4` (the camera swings wide for a bar); time kick x3 = a big surge down the hall | Remove `hat → flicker`, `bass → emission`, `build → fog`. Travel cap 2.5 u/s stays |
| **05 Fibers** | Hairline filaments around the knot. `energy → thickness +0.5 @.5` (.16 → .64) | `hit → tear 1.0` (radial tear burst) | `body → swell +0.9` | `tension → knot +0.4` (the dark knot tightens) | `env.drop → zoom −0.2` (the web rushes at you) | `tear` max 1 → **2**. `density` is not driven by energy (it steps layers in and out) |
| **06 Terminal** | A nearly empty screen: a few lines and the cursor. `energy → fill +0.8 @.5` (.2 → 1.0), **release 800 ms** (lines must not blink) | `hit → jump 0.75`: accents jump 3 lines; ticks (0.3 x 0.75) do not reach a line, so they do nothing by design | `body → emission +0.25 @.4` | `tension → background +0.5` (the blots spread over the text) | global only (the lurch scrolls the text) | Remove `high → fill`, `build → background` |
| **07 Ink** | A few small blots on dark paper. `energy → coverage +0.4 @.5` (.24 → .48) | `hit → rip 1.0` (a band tears sideways) | `body → swell +0.9` (was `env.swell`) | `tension → smear +0.5` (smear bands multiply) | global only: the scar reset **heals the holes** over 2 bars (the paper renews) | none |
| **08 Signal Fog** | Near-black; a thin veil of particles. `energy → density +0.5 @.5` (.05 → .55) | `hit → surge 1.0` (a particle flare) | `body → emission +0.2 @.4` | `tension → haze +0.5` (the haze swallows the forms) | `env.drop → focus +0.8`: the forms **snap into focus** and melt back over the release | `surge` max → **1.6**. Remove `build → focus` |
| **09 Mesh Body** | The whole body, re-knit, dim and slow. `energy → emission +0.25 @.5` (0.72 → 1.67) | `hit → surge 1.0` (+30 % swell at peak) | `body → size +0.12` | `tension → erosion +0.7` (erodes into dust, as now) | global only: the tension reset **re-knits** the body over a bar | `surge` max 1 → **2.5**. **No audio on `hole`** (eyes) |
| **10 Morphogen** | A pale, flat pattern; growth continues. `energy → relief +0.5 @.5` (.23 → .98) | Accent fires `env.pulse → seed_amt 1.0`: seeds that grow roots (**a lasting consequence**) | `body → emission +0.25 @.4` | `tension → pattern +0.2` (the regime drifts, fingerprint → mitosis) | `env.pulse` x1 + `env.drop → boundary −0.3` (the culture floods past its island) | none |
| **11 Halo Ring** | **One thin clean ring on black** (Zwobot 32). `energy → fur +0.6 @.5` (.1 → .7) | `hit → surge 1.0` (the ring flares and grows) | `body → low +0.8` (slow lobes) | `tension → fray +0.5` (the ring frays) | `env.drop → radius +0.25` (it expands and settles) | `surge` max → **1.6**; `low` default .3 → **.05**; `hair` default .4 → **.15** |
| **12 Emergence** | A faint trace in darkness. `energy → visible +0.7 @.5` (.10 → .80). Replaces `presence → visible` | `hit → surge 1.0` | `body → zoom −0.08` (the form swells toward you) | `tension → dissolve +0.7` (into dust, as now) | global only: the tension reset makes the form **appear on the drop** | `surge` max → **1.8** |
| **13 Negative** | A dark negative with only cyan wires. `energy → neg.threshold −0.35 @.5` | `hit → neg.storm 0.9`; **accents** reseed (the camera pose changes every Nth accent via `cam.cuts` ← REACT) | `body → cam.surge +0.5` (was `env.swell`) | `tension → neg.dither +0.5` | Reseed (a guaranteed new angle) + `env.drop → neg.polarity −1.0` (it goes positive for a beat, then fades back to negative) | The reseed trigger moves from `event.kick` to `event.accent` |
| **14 Media Negative** | Dark; only the image's fine detail in cyan. `energy → neg.threshold −0.35 @.5` (.705 → .595) | `hit → neg.storm 0.9` | `body → frame.surge +0.5` | `tension → neg.dither +0.5` | `env.drop → neg.polarity −1.0` + a clip reframe (existing SHOTS reframe path) | `frame.surge` max → **2.5** |
| **15 Media Lines** | Only the strongest contours. `energy → lines.threshold −0.3 @.5` (.069 → .011) | `hit → lines.flash 1.0` (the lines burn) | `body → frame.surge +0.5` | `tension → lines.radius +0.3` (the lines soften and thicken) | `env.drop → lines.fill +0.6` (the image floods in, then recedes to lines) | `threshold` default .022 → **.04**; `frame.surge` max → **2.5**. Remove `kick → frame.surge`, `snare → flash` |
| **16 Membrane** | **A flat sheet: a single line of points** seen at a tilt. `energy → depth +0.7 @.5` (.04 → .46) | `hit → surge 1.0` (the folds leap) | `body → fold +0.3 @.4` | `tension → dof +0.4` (the focus band narrows; everything else melts) | `env.drop → haze +0.5` (a pale haze bursts behind and settles) | `surge` max → **2**. Remove `build → haze` |

### 5.3 Surface, snare and scar routes

| Scene | AIR (`react.air`, centre .3) | Snare (`env.snare`) | SCAR (`react.scar`) |
|---|---|---|---|
| 01 Hot Blobs | none | `→ rim +0.35` | none |
| 02 Dot Relief | `→ noise_floor +0.25` | none | none |
| 03 One Bit | `→ speckle +0.3` | `→ texture_amt +0.3` (the maze flickers) | none |
| 04 Corridor | none | `→ surge 0.6` (FOV punch; half the lights blow out: a stutter) | none |
| 05 Fibers | none | `→ emission +0.15` | `→ density +0.3` (layers accumulate slowly through a section) |
| 06 Terminal | none | `→ invert 1.0` (a band inverts) | `→ decay +0.5` (the text wears out) |
| 07 Ink | none | none | `→ dots +0.5` (holes punch through as the section goes on) |
| 08 Signal Fog | none | none | none |
| 09 Mesh Body | none | none | none |
| 10 Morphogen | none | none | none |
| 11 Halo Ring | `→ hair +0.7` (radial hairs; silence = bare ring) | none | none |
| 12 Emergence | `→ streak +0.3` | none | none |
| 13 Negative | `→ neg.outline +0.3` | `→ neg.flash 1.0` | none |
| 14 Media Negative | `→ neg.outline +0.3` | `→ neg.flash 1.0` | none |
| 15 Media Lines | `→ lines.dots +0.3` | none | none |
| 16 Membrane | `→ sparkle +0.5` | none | none |

### 5.4 The new envelope and trigger set (generator `ENVS` and `triggers()`)

| id | Type | Style | Triggered by |
|---|---|---|---|
| `hit` | ad, retrigger max | `hit` (CHARACTER) | `event.accent` x1; `event.tick` with `scale: "tick"` |
| `snare` | ad, restart | `snare` | `event.snare` with `scale: "snare"` |
| `pulse` | ad, 1 / 80 ms (fixed, was `drop`) | none | `event.accent` (and `event.drop` in Hot Blobs and Morphogen) |
| `drop` | ad, max | `drop` | `event.drop` |
| `phrase` | lfo, 64 beats | none | unchanged |

- **Removed:** `kick`, `hat` and `swell` (replaced by `hit`, `air` and `body`).
- **`userTrigger`:** handled in the engine as a forced accent, so presets need no separate trigger.
- **Reseeds:** only Negative (on accents and on the drop).

---

## 6. Out-of-the-box ideas, rated

Wow is rated 1-5 (for this owner). Effort: **S** ≤ 0.5 day, **M** 1-2 days, **L** 3 days or more.

| # | Idea | What it does | Wow | Effort | In package? |
|---|---|---|---|---|---|
| 1 | **Silence as an instrument** (REST) | Quiet makes the picture darker and sparser, never dead. The first hit afterwards is an event. | 5 | S | **Yes** |
| 2 | **The drop** | One detected (or manual) drop gives a single big event: bloom, lurch, release of the build, scars wiped, per-scene extras | 5 | S-M | **Yes** |
| 3 | **Accents, not every hit** | Rank-based selection: only meaningful hits become full events; the others are small ticks or nothing | 4 | S | **Yes** |
| 4 | **Time kick** | A hit advances the scene clock by an eased, fixed amount: motion that never snaps back and never runs away | 4 | S | **Yes** |
| 5 | **Scars (reaction memory)** | Accents accumulate marks through a section (film dust, Ink holes, Terminal decay); rest heals them; the drop wipes them | 4 | S | **Yes** |
| 6 | **Tempo-locked breathing** | The whole picture inhales toward each downbeat (1-2.5 % zoom, scaled by energy). A floor of life for drumless music | 3 | S | **Yes** |
| 7 | **Anticipation devices** | Tension closes a letterbox (One Bit), tightens the knot (Fibers), narrows the focus (Membrane); the drop releases them. A visible "inhale" before the drop | 4 | S | **Yes** (through the tension routes) |
| 8 | **Light leak on the accent** (showcase 47) | 1-2 large soft red-amber blobs, screen-blended after halation. They bloom on accents, drift from the frame edge and fade over 0.4-1.2 s; strength = Halation x CHARACTER | 4 | S-M | Stretch (day 4) |
| 9 | **GO on DROP** | Cue a scene; GO arms it to fire on the next detected drop (fallback: the next 16-bar line). The scene change *is* the drop. It uses the existing cue + GO. | 5 | S | Next |
| 10 | **Red only on the event** | A palette mode "mono + red accent": the image is bone or grey in play, and the gradient map's mid colour blends toward red only with `env.hit` and `env.drop`. It matches "red only in lines" (showcase 33, 57, 12) | 5 | S-M | Next |
| 11 | **Scope: the sound itself** (showcase 39) | A new `/v2/wave` blob (256-512 decimated L/R samples per frame) feeds a `waveTex` uniform. It powers a Scope scene (Y, XY Lissajous, ring) with a phosphor decay of 0.85-0.93, and the same wave as an optional hairline layer over any scene | 5 | M-L (2-3 d) | Next |
| 12 | **MIDI cue track** (dance show) | On the lead instance: velocity ≥ 100 = forced accent, lower = tick, one reserved note = DROP, another = "hold dark" (force REST). A choreographer's cue clip in Live then plays the picture exactly, the same control signal for sound and image | 4 (5 for the show) | S | Partly (velocity accents are in the package) |
| 13 | **Afterimage** | On a strong accent, capture the frame and let it fade as a low-opacity *negative* over about 1.5 s, like a retinal afterimage. Film-like, and it fits the Negative taste | 4 | M | Later |
| 14 | **Hit-to-cut grammar** | Cuts only on accents that land on bar lines. Cut density follows tension (more as the build rises), one guaranteed cut on the drop, none in REST | 4 | S | Later (mostly settings on existing SHOTS and auto-cut) |
| 15 | **Flash to grey, not white** (showcase 51, 41) | The drop's first frame goes to mid-grey paper with a red hairline instead of white. Softer than a strobe; darkroom-like | 3 | S | Later |
| 16 | **Tension meter in the UI** | A thin bar in the LISTEN strip that fills with the build and flashes on the drop, so the performer can *play* against it (hold CALM through a build, release on the drop) | 3 | S | **Yes** (lamps) |
| 17 | **Ghost pulse** | If music plays but no hit is detected for 8 bars (PULSE and PUNCH only), the transport clock supplies faint beat ticks at 50 % of the tick level | 2 | S | Later |
| 18 | **Stereo spread** | L/R difference drives horizontal spread and split. Needs L/R features in the protocol | 3 | M | Later |

---

## 7. Recommended package (about 3.5 days)

**Goal:** the biggest felt improvement in the smallest change. It gives:
- contrast (REST)
- selection (accents)
- persistence (hold, time kick, scars)
- anticipation and release (tension, DROP)
- auto-calibration
- two controls instead of twelve

### Day 1: the Reaction Bus (engine)

- **`engine/Source/Reaction.h/.cpp` (new):**
  - `ReactionStyle` holds the 3 CHARACTER rows of 4.3.
  - `ReactionShaper::update (Signals&, const Clock&, float react, int style, float reactTrim, bool calm, double dt, double now)` implements 3.3-3.7 and fills `signals.reaction`.
  - It appends `accent`, `tick` and `drop` events.
  - It keeps a manual drop flag set by OSC.
- **`Signals.h/.cpp`:**
  - Append `accent`, `tick` and `drop` to `EventType` and to `eventNames`.
  - Add a `ReactionState reaction` member.
- **`MainComponent.cpp` (frame loop):**
  - After `clock.update`, call `reactionShaper.update(...)` with macro slot 4 as REACT.
  - Feed the scene clock: `drive = (0.6 · body + 0.4 · energy) · contAmount`, and push from 4.4.
  - For each `accent` (and PUNCH `tick`), call `sceneClock.kick(...)`.
  - Handle `/v2/style` and `/v2/trigger drop`.
- **`Motion.h`:** `SceneClock::kick` and the eased consumption of 3.8.

### Day 2: modulation, look pass, plug-in

- **`Modulation.h/.cpp`:**
  - Add source kinds `react.body`, `react.energy`, `react.air`, `react.breath`, `react.tension`, `react.rest` and `react.scar` (readSource, isClosedByReact, isAudioDriven).
  - Split gains: continuous kinds use `contAmount`, envelopes use `hitAmount`.
  - `Envelope.holdSeconds`.
  - Styled envelopes: read the times per frame from `signals.reaction.style`.
  - Trigger action `scale`.
  - `macroActivity[4]` = the hit envelope.
- **`PresetV2.h/.cpp`:**
  - Accept the `react.*` prefix in `sourceOk`.
  - Parse the modulator `style` and `holdMs`, and the action `scale`.
  - Accept `event.accent`, `event.tick` and `event.drop` in triggers and in source `advance`.
- **`LookPass.cpp`:**
  - The `exposure` uniform (right after the fetch).
  - Breath zoom in the Drift transform (drop the `drift > 0.001` guard for the zoom term).
  - `dust · (1 + 0.8 · scar)`.
  - Flash and SHOTS on `accent`, scaled by `min(1, hitAmount)`.
- **`PresetManager.cpp`:** auto-cut "kick" mode on `accent`.
- **Plug-in** (`PluginProcessor.cpp`, `AnalysisWorker.cpp`, `PluginEditor.cpp`):
  - Append `character` (choice, default PULSE) and the `drop` action.
  - Snapshot the `character`.
  - Send `/v2/style`.
  - REACT label and words on macro5, and the CHARACTER 3-segment switch.
  - Move Reactivity, Softness, Push, Hit Sens, Trim and Response into the expert area.
  - Add the REST, TENSION and DROP lamps from `/v2/status`.
  - Keep the full visual redesign for the Claude Design pass. Here only the controls change place.

### Day 3: the presets

- **`engine/tools/build_instrument_presets.py`:**
  - `route()` defaults for kind `react`: attack and release 0.
  - Remove the automatic `scaleBy: macro.impact` on env routes.
  - New `ENVS` and `triggers()` (5.4).
  - Rename the old short `drop` envelope to `pulse`.
  - Macro label `impact` → `react` everywhere.
  - Extend `check()` with the rules of 5.1.
- **All 16 scenes per 5.2 and 5.3:** the routes, range maxima, default edits and route removals listed there.
- Run the generator. The engine validates each file on load.

### Day 4 (half): tuning and proof (the other half is the optional light leak)

- Run `analysis/tools/analyze_wav` on 4 of the owner's tracks: a quiet ambient piece, downtempo with drums, a track with a clear build and drop, and a drone. Feed the engine, log `ReactionState` and the events to a CSV (a debug flag), and adjust the table values.
  - This is where the 4.3 numbers get their final tuning. They are starting points.
- Run `preset_regression_test.py` and the freeze test. At Speed 0 and with Freeze, accents must not move a scene.
- Make contact sheets per scene: a REST frame, a pre-accent frame, the accent frame plus 3 frames, and a drop frame.
- Reuse the forensic doc's measurement harness if it provides one.

**Acceptance criteria** (measured like the Zwobot analysis: 64x36 luma, 8-60 fps samples):

| Check | Target |
|---|---|
| REST vs PLAY mean luminance | REST ≤ 45 % of PLAY (BREATHE), ≤ 55 % (PULSE); never below 3/255 mean (grain and film base alive) |
| Accent rate at REACT 50, 4-on-the-floor at 100-120 BPM | BREATHE 0.25-1 per bar, PULSE 1-2 per bar, PUNCH 2-4 per bar; never more than 3 per second |
| Accent visibility (PULSE, REACT 50, every scene) | Mean frame difference between the accent frame and 2 frames before ≥ 20/255. Three frames later still ≥ 50 % of that (the hold). |
| Drop | Fires once, within 1 beat of the real drop, on the build-and-drop track. No drop in 5 minutes of a track without one. |
| Body range | During a groove, body p5 → p95 spans ≥ 0.7 |
| Clamping | No reaction-driven parameter sits at its max more than 15 % of the time |
| Runaway | At REACT 100 PUNCH, the average scene speed over any 4 s is ≤ x2.5 of the knob speed (the combined governor in 3.8) |
| Freeze | Speed 0 / Freeze: zero motion with accents (existing freeze test passes) |

**If only 2 days are available**, build this core:
- the auto-ranged `body` and `energy`
- accents (without the governor)
- REST with the exposure floor
- the time kick
- REACT curves (`contGain`, `hitGain`) with PULSE only
- per scene, the REST, HIT and BODY routes

Then add CHARACTER, tension, drop and scar in the next round.

---

## 8. Risks and open questions

**Risks**

| Risk | Mitigation |
|---|---|
| **Drop detection on ambient material** (slow swells, no kick) | The drop needs `primed` (tension or a recent rest) **and** a kick-like transient or a bass jump. BREATHE also uses a higher threshold. The manual DROP action (and a MIDI note) covers the dance show. |
| **Rank-based accents on very sparse music** (one candidate a minute) | The "return after a gap" rule makes these accents anyway, which is the desired result. In BREATHE, ticks are off. |
| **The time kick with media clips** | `MediaBin` advances with `speed`, not `sceneDt`, so clips do not lurch. That is acceptable; add it later if wanted. |
| **Parameter clamping from bigger ranges** | The acceptance check "≤ 15 % at max" covers it. The look pass Crush and tone map absorb peaks. |
| **The owner prefers today's steadier picture in some scenes** | REACT 0-25 keeps the tuned neutral pose, because routes centre on neutral. CALM stays one press away. |
| **The forensic doc may find a detection problem** (kicks missed on mastered tracks) | That is an analyser fix (onset floors) that sits *under* this design. The accent selector and the auto-range do not depend on it, but they need candidates to exist. |

**Questions for the owner**

1. **The default CHARACTER:** PULSE for the set and BREATHE for *Before It Disappears*?
2. **How dark may silence get?** The proposal is about a quarter of the brightness in BREATHE and a third in PULSE, with grain always alive.
3. **The drop:** do your tracks have drops you want the picture to land on, or should the drop only ever be manual (a button or a MIDI note)?
4. **Is a hit allowed to change the picture lastingly?** Examples are seeds, a new form or a new camera angle, but only on accents and drops. Or should it always return to where it was?
5. **"Red only on the event"** (idea 10): worth trying as a palette mode?

---

## 9. Sources

**Opened for this document:**
- Synesthesia, *SSF Audio Uniforms* (definitions of Level, Hits, Time, Presence, OnBeat, Intensity): https://app.synesthesia.live/docs/ssf/audio_uniforms.html
- Synesthesia, *SSF Best Practices* (every scene reactive, a toggle when reactivity is a nuisance, highlight what is unique to the scene): https://app.synesthesia.live/docs/ssf/best_practices.html
- Derivative, *Audio Analysis 2.0: Extended Audio-reactive Components* (asymmetric lag; beat-count logic on every 4th, 8th or 16th beat): https://derivative.ca/community-post/asset/audio-analysis-20-extended-audio-reactive-components/73946
- Ableton, *Interview with Robert Henke about Lumière* (sound and lasers driven by the same control signals, improvised together): https://www.ableton.com/en/blog/robert-henke-lumiere-lasers-interview/

**Project documents:**
- `docs/REFERENCE-ZWOBOT-SHOWCASE.md`: 57 clips; synthesis patterns 3, 7 and 8; recommendations 2, 3 and 6a. Zwobot page: https://www.zwobotmax.com/showcase, analysed there, not re-opened here.
- `docs/REFERENCE-ZWOBOT-V3-TRAILER.md`, `docs/RESEARCH-REPORT.md` (B3 motion design, B4 mapping hierarchy, anti-patterns), `docs/REVIEW-2026-09-24-HE.md` (control redesign, latency path), `docs/product-design/DESIGN-DOSSIER.md` (2.3 the five "how much" concepts, 4.7), `docs/product-design/AUDIO-SOURCE-ROUTING-RESEARCH.md` (LISTEN TO = "what").

**Code read:**
- `engine/Source/`: `FeatureBus.*`, `Signals.*`, `Modulation.*`, `Motion.h`, `MainComponent.cpp`, `LookPass.*`, `PresetManager.cpp`, `PresetV2.*`, `ISFShader.cpp`
- `engine/Shaders/Instrument/*.fs`
- `engine/tools/build_instrument_presets.py`: now, and at `09279ba` for the comparison
- `plugin/Source/PluginProcessor.cpp`
- `analysis/src/Analyzer.cpp`, `AdaptiveNormalizer.cpp`, `OnsetTracker.cpp`
