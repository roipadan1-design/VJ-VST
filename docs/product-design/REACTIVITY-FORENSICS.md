# Reaction to sound: old vs new, measured (26.9.2026)

## בקצרה

- **מה נמדד:** אותו גרוב סינתטי (90 BPM, קטעים של גרוב, שבירה, בנייה ודרופ, קטע שקט) הוזרם לשלוש גרסאות של המנוע.
  - הגרסה שבדקת ב-24.9 (`09279ba`)
  - הגרסה שהתלוננת עליה
  - הגרסה החדשה אחרי הכוונון

  כל פריים הוקלט ונמדד בשבע סצנות.
- **למה הישנה הרגישה חזקה יותר:** בכל קיק היא "ערבבה" את הסצנה מחדש (reseed), והבס האיץ את התנועה. הגרסה שאחריה העיפה את שני הדברים, ומה שנשאר היה מכה שנעלמת תוך 2-3 פריימים.
- **הגרסה החדשה, מכות:** המכות גדולות יותר מבגרסה הישנה ברוב הסצנות. למשל Dot Relief פי 3, Hot Blobs פי 11, Fibers פי 1.7. ב-Halo Ring המכות כמעט שוות, וב-Emergence (סצנה חשוכה בכוונה) הן עדיין קטנות יותר.
- **הגרסה החדשה, מעקב רציף:** התמונה עוקבת אחרי המוזיקה כל הזמן, ולא רק במכות. המתאם בין בהירות התמונה לעוצמת השיר עלה מ-0.0-0.2 ל-0.4-0.6.
- **הגרסה החדשה, דרופ:** הדרופ מזוהה במקום הנכון, מיד אחרי הבנייה.

## Method

- **Audio:** `research/forensics/make_groove.py`, a 70 s synthetic downtempo groove at 90 BPM with a known kick / snare timeline. Sections:
  - A: loud
  - B: breakdown, -8 dB, no drums
  - C: build (snare roll and a riser)
  - D: the drop
  - E: the same groove at -12 dB
  - F: loud again
- **Engines:** each build carries the env-gated frame dump (`VJ_FRAMEDUMP`, `engine/Source/MainComponent.cpp`). It writes every rendered frame as 160x90 RGB, plus the signals and events of that frame.
  - old = `09279ba` with its own `analyze_wav`
  - new = the current code
- **Runs:** `run_capture.py` starts one engine on one scene with the plug-in's default controls for that version, then streams the WAV with `analyze_wav --send`. `analyze_capture.py` aligns the kicks and computes the metrics. `batch_compare.py` runs the whole comparison.
- **Metrics, on a 60 Hz grid:**
  - **hit step**: the mean absolute change of the frame 50 ms after a kick, minus the same measure at random control times (x1000)
  - **hit size**: the largest change within 150 ms of a kick
  - **hit contrast**: the frame-to-frame motion just after kicks, relative to the motion between kicks
  - **r(L)**: the correlation of frame brightness with the song's 1 s loudness envelope

## Results

| Scene | Version | Hit step at 50 ms | Hit size | Hit contrast | r(L), follows the music | Drop / breakdown brightness |
|---|---|---|---|---|---|---|
| Hot Blobs | old | 4.4 | 170 | 1.1 | 0.21 | 1.15 |
| | first new | 23.9 | 90 | 1.5 | 0.66 | 1.81 |
| | **tuned** | **50.6** | 107 | **5.4** | 0.11 | 1.01 |
| Dot Relief | old | 48.2 | 76 | 7.2 | 0.10 | 1.07 |
| | first new | 48.4 | 74 | 2.6 | 0.39 | 1.47 |
| | **tuned** | **142.0** | **177** | 4.8 | **0.42** | **2.02** |
| Corridor | old | 58.5 | 123 | 5.4 | 0.16 | 0.89 |
| | first new | 20.5 | 57 | 3.6 | 0.08 | 0.88 |
| | **tuned** | **57.7** | **116** | **5.9** | 0.17 | 0.97 |
| Fibers | old | 68.0 | 108 | 7.3 | 0.24 | 1.21 |
| | first new | 61.6 | 94 | 3.4 | 0.50 | 1.46 |
| | **tuned** | **115.3** | **150** | 4.2 | **0.48** | **1.64** |
| Mesh Body | old | 19.7 | 39 | 7.6 | -0.01 | 0.92 |
| | first new | 15.2 | 25 | 2.4 | 0.60 | 1.79 |
| | **tuned** | **33.6** | **44** | 3.9 | **0.58** | **1.96** |
| Halo Ring | old | 35.0 | 45 | 15.4 | 0.02 | 1.03 |
| | first new | 10.5 | 14 | 4.5 | 0.38 | 1.25 |
| | **tuned** | 32.4 | 39 | 8.7 | **0.37** | 1.38 |
| Emergence | old | 28.8 | 46 | 35.4 | 0.04 | 0.72 |
| | first new | 2.8 | 6 | 3.3 | 0.37 | 0.91 |
| | **tuned** | 18.6 | 22 | 8.0 | **0.43** | 1.11 |

**Reading:**
- "Hit contrast" is a ratio to the motion between kicks. It falls whenever the picture moves more with the music between the hits, which is the intended change. The absolute measures (hit step, hit size) are the ones that say whether a kick is *seen*.
- **Events:** the new engine selected 43 accents out of 82 kicks and fired 3 drops per run:
  - at the start of the music (after silence)
  - at the drop after the build
  - when the groove returned from -12 dB
- **The "first new" row** is the reaction layer as first built from REACTION-DESIGN.md. Its hits were too selective: snares competed with kicks for the ranking, and only 40 % of candidates became accents.

## Root causes (why the Sept 25 build felt weaker than Sept 24)

1. **Reseed on every kick was removed.** In `09279ba` every scene's kick trigger bumped `vj_seed` and re-dealt layouts, so every kick left a lasting change. Afterwards only Negative did.
2. **Band clocks were removed.** Bass was routed into the motion rates of 10 scenes; the redesign's rule "nothing routes into a rate" took that out. The global Push (at most x1.3) did not replace it.
3. **Spikes instead of events.** The hit envelopes decayed exponentially with a half-life of about 53 ms, so a hit was at half strength 2-3 frames later.
4. **Hits were scaled down.** The Impact default of 0.4 gave x0.8, and every trigger was further scaled by 0.4 + 0.6 x strength. Mesh Body's kick came out as a +3.7 % swell.
5. **No rest state**, so there was nothing to react against.

## What changed (the fix)

- **The reaction layer** (`engine/Source/Reaction.h/.cpp`):
  - auto-ranged body, energy and air
  - accents chosen by rank (kicks lead; snares join only in PUNCH)
  - a hold on every hit (PULSE: 50 ms hold, 240 ms half-life)
  - the time kick (an accent moves the scene forward by half a beat of its own motion, eased in)
  - a short exposure punch on every accent (+0.3 stop in PULSE)
  - REST: silence darkens to about a third, and the grain stays alive
  - tension, and the DROP (detected or by hand)
  - scars
- **All 16 scenes re-routed** (`engine/tools/build_instrument_presets.py`): a REST, HIT, BODY and BUILD route each, with bigger hit ranges. Emergence also gets an emission flash on accents.
- **One control:** REACT (macro 5, formerly Impact; default 50 = as designed), with the character BREATHE / PULSE / PUNCH (`/v2/style`).
- **Build-up only counts a fresh rise.** The section tracker re-primes when music starts after silence, and tension needs a fall-back before it counts again after a drop. A steady groove no longer reads as "build-up".

## Not verified

- **Real tracks.** Only the synthetic groove was measured. The owner's own tracks (ambient pads without kicks, mastered electronica) are the next test.
- **The drop at the very start.** Music starting after silence counts as a drop: a bloom and a lurch. If that is unwanted, the "primed by a recent rest" rule is the one to drop.
- **Live.** Nothing here was measured inside Live; the plug-in path was exercised with the harness only.
