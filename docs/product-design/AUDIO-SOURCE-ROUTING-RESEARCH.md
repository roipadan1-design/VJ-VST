# Audio source routing: who does the picture listen to?

*26 September 2026. Research only: no code was changed. Scope: Ableton Live 10.1.43 on Windows 11, Intel Iris Xe laptop, the VJ Analyzer VST3 (JUCE 8) and the VJ Engine. Max for Live is avoided (trial / unreliable on this machine).*

---

## חלק א׳: בשבילך (עברית, בקצרה)

### איך זה עובד היום

- **כל עותק של VJ Analyzer שומע רק את הערוץ שהוא יושב עליו**, ושולח את מה ששמע **ישר למנוע**, לא למאסטר. כלומר "הרחבה שיושבת על ערוץ ושולחת הלאה" כבר קיימת. היא פשוט לא נוחה לשליטה.
- **לכל עותק יש "תפקיד".**
  - **MIX** (בדרך כלל על המאסטר) קובע את כל ה"זרימה": עוצמה, בס/אמצע/גבוה, בנייה ושבירה.
  - **KICK / SNARE / HAT** נותנים רק **מכות** מדויקות מהערוץ שלהם, במקום לנחש אותן מתוך המיקס.
- **BASS ו-TEXTURE הם כמעט רק תווית.** שום סצנה לא קוראת אותם. ויותר מזה: עותק TEXTURE על ערוץ פדים יכול לייצר "קיקים" מזויפים כשאין עותק KICK.
- **"VISUALS REACT TO" לא בוחר ערוצים. הוא בוחר סוג תגובה** (מכות קיק, סנר, האט, באס, עוצמה). כיבוי KICK מכבה כל "קיק", לא משנה מאיזה ערוץ הוא הגיע.
- **Send macros דלוק כברירת מחדל בכל עותק.** אם שניים דלוקים, הם "נלחמים" פעם בשנייה: כפתור שסובבת יכול לקפוץ חזרה, ו-Blackout יכול להיכבות לבד.
- **שני עותקי MIX:** המנוע מדלג ביניהם כמה פעמים בשנייה, והתמונה רועדת.
- **אין היום דרך נוחה לשנות באמצע הופעה "למי מקשיבים".** צריך להזיז פלאגינים או לשנות תפקיד בעכבר.
- **התדרים קבועים ואוטומטיים.** הבס למכות הוא 30-160 הרץ, האמצע 200-2,500, והגבוה 5,000-16,000.

### האפשרויות

**א. "סדר בבית"**
- **הרעיון:** לתקן את מה שקיים. רק עותק אחד מוביל, אף פעם לא שניים. BASS מתחיל לעבוד באמת. אין יותר רעד. ובפלאגין הראשי מופיעה רשימה של כל העותקים, עם שם הערוץ ב-Live ונורית שמהבהבת.
- **בהופעה:** אין עדיין תנועה חדשה. אבל הדברים המבלבלים נעלמים, ורואים מי מזין את התמונה.
- **יתרון:** נחוץ בכל מקרה, זול.
- **חיסרון:** לא פותר את "להחליף ערוץ באמצע השיר".
- **מאמץ:** בערך 3 ימים.

**ב. ערוץ VISUALS (ההמלצה המרכזית)**
- **הרעיון:** יוצרים ב-Live ערוץ Return אחד בשם VISUALS ושמים עליו את VJ Analyzer בתפקיד FOCUS. הערוץ הזה שקט: הקהל לא שומע אותו. לכל ערוץ ב-Live כבר יש כפתור Send. כמה שמסובבים את ה-Send של ערוץ ל-VISUALS, ככה התמונה מקשיבה לו יותר.
- **בהופעה:** הפתיחה של הטרק היא רק פדים. אתה מעלה את ה-Send של ערוץ הפדים, והתמונה מתחילה לנשום איתם. כשהתופים נכנסים, אתה מעלה את ה-Send של התופים ומוריד את הפדים, והמכות מתחילות לחתוך. בפלאגין הראשי יש פיידר אחד, **MIX ⇄ VISUALS**, שמחזיר בתנועה אחת ל"כל השיר".
- **יתרונות:**
  - זו תנועה שאתה כבר מכיר.
  - אפשר למפות כל Send לכפתור ב-Korg.
  - אפשר לכתוב אותה כאוטומציה ב-Arrangement, וזה מושלם למופע המחול.
  - עותק אחד בלבד, כמעט בלי עלות CPU.
- **חסרונות:**
  - צריך הקמה חד-פעמית (Return ופיידר שלו למטה, או כפתור "השתק יציאה" שנוסיף).
  - ה-VISUALS שומע סכום, ולא יודע לבד מה קיק ומה פד. הפיצול לבס/אמצע/גבוה עדיין עובד.
- **מאמץ:** בערך 4-5 ימים, מעל א׳.
- **טריק:** אם מעבירים את ה-Return ל-Pre, ערוץ שהפיידר שלו למטה (הקהל לא שומע אותו) עדיין יכול להזיז את התמונה. זה "ערוץ רפאים" רק לוויזואל.

**ג. "לוח מקשיבים"**
- **הרעיון:** שמים עותק קטן על כל ערוץ חשוב. בפלאגין הראשי מופיעה טבלה: שם הערוץ, נורית, פיידר "כמה להקשיב" וכפתור SOLO.
- **בהופעה:** לוחצים SOLO על "Pads", וכל התמונה מקשיבה רק לפדים. מבטלים, והכל חוזר.
- **יתרון:** הכי מדויק. המנוע יודע מה בא מאיפה, למשל קיק לחוד ופד לחוד.
- **חסרונות:** הרבה עותקים. הכי הרבה ממשק. המיפוי ל-Korg לפי מספר שורה עלול לזוז.
- **מאמץ:** בערך 1-1.5 שבועות.

**ד. כניסת Sidechain לפלאגין הראשי**
- **הרעיון:** בפלאגין שעל המאסטר בוחרים מהתפריט של Live ערוץ אחד שיהיה "הפוקוס".
- **בהופעה:** בוחרים לפני השיר: "השיר הזה מקשיב לשירה".
- **יתרון:** אין צורך ב-Return.
- **חסרונות:**
  - בחירה בעכבר, כנראה בלי מיפוי MIDI, אז זה לא לשינוי חי.
  - ב-Live 10 זה מתועד, אבל לא נבדק עם הפלאגין שלנו.
- **מאמץ:** בערך 3 ימים, ועוד חצי יום בדיקה.

**ה. שליטה בתדרים**
- **הרעיון:** לכל מקור בוחרים "מאיפה לקחת מכות": הכל / נמוך (קיק) / אמצע (סנר, קול) / גבוה (האט). ידיות חופשיות רק במסך העריכה.
- **בהופעה:** כמעט לא נוגעים בזה. זו הכנה לפני השיר.
- **יתרון:** מתקן מקרים שבהם הבס "מתחזה" לקיק.
- **חיסרון:** עוד כפתורים.
- **מאמץ:** בערך 2-3 ימים לפריסטים.

### ההמלצה שלי

**א׳ + ב׳ עכשיו.** ה-Send ל-VISUALS הוא ה"מי", ופיידר MIX ⇄ VISUALS הוא ה"כמה".
- LISTEN TO נשאר ה"מה": מכות, באס, עוצמה.
- ה-Moments (סנפשוטים A-D) ישמרו גם את הפיידר ואת LISTEN TO.
- את ג׳ נשמור לאחר כך, רק אם תרגיש שחסר.
- את ה׳ נעשה רק כפריסטים, לא כידיות חופשיות.
- את ד׳ לא נבנה, כי ב׳ עושה אותו טוב יותר.

---

## Part B: full report (English)

### 1. Summary

- **Today's topology is already "senders on tracks, one brain".**
  - Every VJ Analyzer instance streams its own analysis over OSC straight to the engine.
  - The engine's `FeatureBus` merges the streams by role.
- **The problems are not the topology.** They are:
  1. The merge rules are opaque, and partly broken (BASS / TEXTURE do nothing useful, two MIX instances jitter, two "Send macros" instances fight).
  2. There is no performable control of *which* tracks feed the picture.
- **The most intuitive Live-native control is a send knob.** Recommendation:
  1. Fix the merge rules (Option A).
  2. Add a **FOCUS role** for an analyser on a silent **VISUALS return track**, plus one **Mix ⇄ Focus** fader in the lead instance (Option B).
- **Everything B needs exists in Live 10:** sends, returns, Pre/Post, MIDI Map, automation and clip envelopes. It needs no Max for Live.
- **VST3 sidechain is documented for Live 10.1**, but it is a set-up-time chooser, not a performance control. It is not recommended now.

### 2. How it works today (read from the code)

#### 2.1 Data path

```
Live track (any)                       VJ Engine
+-----------------------------+        +-----------------------------------------------+
| VJ Analyzer (role R)        |  OSC   | FeatureBus.handleMessage                      |
|  audio thread -> SpscFifo   | -----> |   sources[sourceId] = latest frame, role R    |
|  worker: vj::Analyzer       |  UDP   |   events -> pending                           |
|   /v2/frame   ~120 Hz       |  9000  | FeatureBus.takeSnapshot (per rendered frame)  |
|   /v2/spectrum ~60 Hz       |        |   pick ONE "mix" source -> continuous signals |
|   /v2/event   per onset/MIDI|        |   role resolution -> kick/snare/hat events    |
|   /v2/hello   1 Hz          |        | applyReactMask (REACT TO)                     |
|   controls (if Send macros) |        | ModulationRuntime (routes, triggers)          |
+-----------------------------+        | LookPass (flash, shots), SceneClock (push)    |
                                       +-----------------------------------------------+
```

- The plug-in is a pass-through stereo effect (`plugin/Source/PluginProcessor.cpp`, `BusesProperties` with one main in and one main out, no sidechain bus).
- Analysis runs on a worker thread, never on Live's audio thread (`plugin/Source/AnalysisWorker.h`).
- The instance's `sourceId` is random per load. The hello name is always the literal `"VJ Analyzer"` (`AnalysisWorker.cpp:383`), so the engine cannot tell which Live track a source sits on.

#### 2.2 What the analyser measures (fixed bands)

| Signal | Range | Where |
|---|---|---|
| 6 bands | 20-60-150-400-2k-6k-16k Hz | `analysis/src/Analyzer.cpp:13` |
| bass / mid / high aggregates | bands 0-1 / 2-3 / 4-5 (20-150 / 150-2k / 2k-16k Hz) | `Analyzer.h:19` |
| Onset regions (hits) | bass 30-160 Hz (time-domain 160 Hz low-pass), mid 200-2500 Hz, high 5k-16k Hz | `Analyzer.cpp:87-89, 127` |
| 32-band spectrum, centroid, flatness, rolloff, flux, energyTrend | 30 Hz-16 kHz | `Analyzer.cpp` |
| Relative values | adaptive normaliser, 8 s history, Q20/Q95 anchors | `AdaptiveNormalizer.h` |

- The relative ("activity") values are **gain-invariant by design**: after a few seconds, a quiet source reads as active as a loud one.
- Onset detection is also gain-invariant (there is a test, `testGainInvariance`).
- This matters for any "send level = amount" design (§9.3).

#### 2.3 Role resolution in `FeatureBus::takeSnapshot` (`engine/Source/FeatureBus.cpp`)

| Question | What the code does | Evidence |
|---|---|---|
| Which source drives the continuous signals (level, bass/mid/high, bands, spectrum, descriptors)? | **One** source. It is the MIX-role source whose *most recent packet arrived last*. If there is no MIX source, it is `sources.begin()`, the lowest random `sourceId`, i.e. an arbitrary instance of any role. | lines 301, 308 |
| Two MIX instances? | Both stream at ~120 Hz, so the "latest packet" winner flips between them from frame to frame. The picture alternates between two analyses (jitter), and both instances' transients are emitted, so hits are doubled (the preset refractory time partly hides this). | 301, 384 |
| KICK / SNARE / HAT source | Its bass / mid / high transients become `event.kick` / `snare` / `hat`. Its other transients are discarded. A MIDI note on it is a hit, and MIDI suppresses its audio hits for 2 s. While such a source is live, the MIX's transients no longer stand in for that drum. | 356-389 |
| MIX source's transients | Emitted raw (`bassTransient`, …). They also stand in for kick / snare / hat when no dedicated source is live. | 384-387 |
| **BASS / TEXTURE source** | `roles[bass/texture].live/levelRel` are computed, but **nothing reads them** (the only reader, `Modulation.cpp:226-228`, reads kick / snare / hat). Their transients take the MIX path: raw events, **plus stand-in kick / snare / hat**. So a TEXTURE instance on a pad track can fire "kicks". | 296-299, 382-387 |
| Dedicated role source vs mix band | With `sourcePolicy` (all 16 presets use the defaults), `audio.bass.*` reads the **KICK track's overall level** when a KICK source is live. Likewise `mid` reads SNARE and `high` reads HAT. A sustained bassline therefore never reaches `audio.bass`. | `Modulation.cpp:235-240`, `PresetV2.cpp:59-62` |
| Transport / tempo | Taken from the MIX source if it reports a valid transport, else from any source. | 303 |
| Silent source | Fades out 250 ms after its last frame, over 300 ms; dropped after 2 s. | `FeatureBus.h:47` |

#### 2.4 What the 16 scenes actually read (all `engine/Presets/*.json`)

| Source | Presets using it | Notes |
|---|---|---|
| `event.kick` / `snare` / `hat` / `userTrigger` triggers | 16 / 16 / 16 / 16 | drive the `env.kick`, `env.snare`, `env.hat`, `env.swell` and `env.drop` envelopes |
| `env.kick` → stage params | 14 routes | the dominant reaction |
| `descriptor.build` | 14 | from the MIX level (SectionTracker, `Signals.cpp`) |
| `audio.high.activity` | 9 | the HAT level if a HAT instance is live |
| `audio.bass.activity` | 4 (Dot Relief, Corridor, Fibers, Halo Ring) | the KICK level if a KICK instance is live |
| `audio.mid`, `audio.level`, `presence`, `centroid` | 1 each | |
| `audio.kick/snare/hat.activity`, `audio.band0..5`, `spectrum`, `flux`, `event.midiNote`, `*Transient` | **0** | the spectrum and bands are only drawn in the plug-in UI |

Other consumers of the mix signals:
- `LookPass` (Flash / SHOTS / HUD on kick and snare events, dust on `build`).
- `PresetManager` kick cuts (Cut Rate = KICK).
- `SceneClock` Push: `0.6·bassRel + 0.4·levelRel` (`MainComponent.cpp:202`).
- Scene shaders do **not** read the legacy `bass` / `level` uniforms directly (checked by grep over `engine/Shaders/Instrument/*.fs`).

**Consequence:** MIDI notes on a MIX instance change nothing visible, because no preset triggers on `event.midiNote`.

#### 2.5 What "VISUALS REACT TO" does exactly

It is a global gate, sent as `/v2/react` by the instance with Send macros on. It closes **kinds of signal**, not tracks (`Signals.cpp applyReactMask`, `Modulation.cpp isClosedByReact`):

| Toggle | Drops these events | Glides these routes to neutral |
|---|---|---|
| KICK | kick, bassTransient | `audio.kick.activity` |
| SNARE | snare, midTransient | mid, snare activity, bands 2-3 |
| HAT | hat, highTransient | high, hat activity, bands 4-5 |
| BASS | none | bass rel/abs, bands 0-1 |
| LEVEL | none | level, centroid / flatness / rolloff / flux / energyTrend, build, presence |

**Leaks:**
- Push (`MainComponent.cpp:202`) reads `bassRel` / `levelRel` directly, so closing BASS and LEVEL does not stop Push.
- LookPass dust reads `build` ungated.

#### 2.6 "Send macros" (`sendControls`)

- The default is **on** (`PluginProcessor.cpp:177`).
- Every instance with it on sends macros, LOOK, palette, REACT TO, MOVE, media and blackout.
- Once a second it marks everything as unsent and re-sends it all (`AnalysisWorker.cpp:384`, "survives lost datagrams").
- **Two instances with it on overwrite each other every second.** A knob you turned on the master snaps back to the kick instance's value, and a Blackout can be undone.
- Instances with it off dim 60 % of their UI without explanation (dossier issue 7).

#### 2.7 Weaknesses, ranked for the performer

1. **No performable "who drives the picture" control.** Changing it means moving plug-ins or clicking roles.
2. **REACT TO looks like a source selector but is a signal-kind gate.** The same names as the role buttons make it worse (dossier issue 2).
3. **BASS / TEXTURE are labels with side effects:** fake kicks, and no bassline reactivity.
4. **Two MIX instances make the picture jitter and double the hits.** A missing MIX makes an arbitrary instance the mix.
5. **Several Send-macros instances fight once a second.**
6. **The lead cannot see the other sources.** Lamps show only the instance's own onsets, and sources have no track names.
7. **Relative normalisation hides level.** A quiet source reads as busy as a loud one, so "less of this track" cannot be expressed by level.

### 3. Routing options in Live 10.1 (Windows), verified

The Ableton Knowledge tool returns the **Live 12** manual. Features below are marked with the Live version that the **Live 10 release notes** (opened, https://www.ableton.com/en/release-notes/live-10/) prove. The Live 10 reference manual PDF (https://cdn-resources.ableton.com/resources/0b/c1/0bc1007e-bd0b-4d6f-b52d-ee0054f3a6f8/l10manual_en.pdf) exceeded the fetch tool's size limit and could not be read. The ableton.com `/live-manual/10/` pages redirect-loop outside a browser.

| # | Mechanism | Live version | Evidence | Status |
|---|---|---|---|---|
| R1 | One instance per track (inserts) | any (VST3 needs **10.1**) | 10.1 notes: "Introduced support for VST3." | Works today |
| R2 | Track name / index / colour visible to a VST3 | **10.1.2** | 10.1.2 notes: "Added support for the IInfoListener interface in VST3 plug-ins. Supported properties include Live's track name, track index and track color." JUCE forwards it to `AudioProcessor::updateTrackProperties` (checked: `JUCE/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:841, 1164`) | Usable on 10.1.43 |
| R3 | **Third-party plug-in sidechain chooser** in the Device View | **10.1** | 10.1 notes: "Sidechain routings, Gain and Mix parameters are now displayed in third-party plug-ins in the Device View." The Live 12 manual says the choosers "allow you to select any of Live's internal routing points" (https://www.ableton.com/en/manual/working-with-instruments-and-effects/, via the Knowledge tool) | Documented; **not tested with our JUCE build** (see R3 risks) |
| R4 | Extra VST3 input buses as **Audio To** destinations | **10.1** | 10.1 notes: "Additional audio input and output buses from VST3 plug-ins can be used from track 'Audio To' and 'Audio From' routing choosers, as with VST2 and AU plug-ins." | Documented. **But Audio To moves the track's output away from the Master**, so the audience stops hearing it. This is only usable through extra "tap" tracks |
| R5 | Sends / return tracks, Pre/Post toggle per return | long-standing (Live 10 has them) | Live 12 manual "Return Tracks and the Main track" (https://www.ableton.com/en/manual/mixing/) | Standard feature |
| R6 | Devices sit **before** the track mixer (fader) | long-standing | Live 12 manual: "Signals travel from Live's tracks into their respective device chains and then into the track mixer" (https://www.ableton.com/en/manual/routing-and-i-o/) | A return's plug-in sees the signal even with its fader at -inf (inference from signal order; **test T3**) |
| R7 | Audio From another track: Pre FX / Post FX / Post Mixer; per-Rack-chain tap points | long-standing (Rack-chain taps: version not verified) | Live 12 manual, "Internal Routing Points", "Routing Points in Racks" | Lets a "listener track" tap, e.g., one Drum Rack chain (the kick pad) without touching it |
| R8 | "Sends Only" output | long-standing | Live 12 manual, "Creating Submixes" | Useful for a silent listener track |
| R9 | Resampling input (Master → a track) | long-standing | Live 12 manual, "Resampling" | Adds nothing over an instance on the Master |
| R10 | Delay compensation incl. **return tracks**; "Reduced Latency When Monitoring" | Live 10 has Delay Compensation; the option's introduction version is unknown | Live 12 manual, "Device Delay Compensation": "including those on the return tracks … input-monitored tracks … may be out of sync with … Return tracks" | See §7.2 |
| R11 | MIDI Map mode for sends; Mapping Browser Min / Max and invert | long-standing | Live 12 manual, "Assigning MIDI Remote Control", "The Mapping Browser" | Sends are MIDI-mappable |
| R12 | Session clip envelopes can automate mixer controls (sends) | long-standing | Live 12 manual, "Mixer and Device Clip Envelopes" (https://www.ableton.com/en/manual/clip-envelopes/) | A clip can carry its own VISUALS send |
| R13 | VST3 plug-in controlling the track's volume / pan / mute / solo / **sends** (PreSonus extensions) | **10.1.9** | 10.1.9 notes: "VST3 plugins can now use the PreSonus VST extensions to observe the name and index of the track they are on and to control Live's track volume, pan, mute, solo and sends." | Interesting for later (e.g. a source instance with a "feed visuals" button that moves its own send). Not needed now; JUCE support for these extensions is unknown |
| R14 | External loopback (VB-Cable, Voicemeeter, stems to extra outputs) into the engine | n/a | Synesthesia documents this route (https://synesthesia.live/docs/faq/audio.html) | Rejected: extra driver buffers, a second audio device, and we already sit inside Live |

**R3 risks (sidechain on Live 10 + JUCE VST3):**
- A third-party tutorial claims Live 11 and earlier "often did not expose the plugin's secondary input bus" and that native VST3 sidechain came with Live 12 (https://ringmodsidechain.com/tutorials/sidechain-routing-in-ableton-live-complete-guide). This **contradicts Ableton's own 10.1 notes**, so the official notes are trusted, but it is a reason to test.
- JUCE forum reports show Live-specific VST3 sidechain quirks:
  - VST3 and AU behaving differently in Live 10, unresolved (https://forum.juce.com/t/vst3-and-au-behave-differently-in-ableton-10/42849).
  - Reading an unrouted aux bus returns the previous output buffer; the fix is to read only when the bus is active (https://forum.juce.com/t/vst3-synth-in-ableton-sidechains-to-itself-when-no-sidechain-input-is-selected/51507).
- **Inference:** Live's routing choosers are not MIDI-mappable, so a sidechain source is chosen with the mouse. This is not verified in the manual.

### 4. Live switching UX: evaluated ideas

| Idea | Verdict for this performer | Why |
|---|---|---|
| **Send knobs on a VISUALS return** | **Yes, the core gesture** | One knob per track, on the channel strip he already reads. MIDI-mappable (R11), automatable in the Arrangement (dance show) and in clip envelopes (R12). A continuous crossfade comes for free: one send down, another up. It also scales to any number of tracks at one instance's CPU cost |
| **Mix ⇄ Focus fader in the lead** | **Yes** | One gesture back to "the whole song". Also the safety net when all sends are down |
| **LISTEN TO lamp-toggles (kinds)** | **Keep** (dossier) | Now clearly "what kind", while sends are "which tracks" |
| Snapshots (Moments) store the listen state | **Yes** | Store the Focus fader + LISTEN TO. Today snapshots store macros, LOOK (including Reactivity) and MOVE, but not the REACT toggles (`PluginProcessor.cpp:76-84`) |
| Per-source weight faders / SOLO in the lead (source board) | Later (Option C) | The most precise, but it needs one instance per source and slot-to-track mapping |
| Crossfading between two sources | Covered | Covered by sends + the Focus fader. Mapping one CC inversely to two sends in Live 10 is **not verified**, so do not promise it |
| Linking to clip / scene launching | Yes, via Live itself | A clip envelope on the track's VISUALS send (R12). There is no Remote Script or M4L work |
| Per-scene default listen setup | **No** | It would silently override the performer's hands. Scenes keep `sourcePolicy` only |
| "Follow the loudest" (automatic) | Optional mode inside C, off by default | Unpredictable for a cue-locked dance show. With B, the sum on the bus is already "loudest dominates" |
| Mapping matrix (source × target) | **No** | The competitor research already flags open matrices as "I lost control". Fixed semantics: sends = which tracks, LISTEN TO = kinds, Focus = how much |
| MIDI notes as a source | Keep as is | A KICK instance with a MIDI kick clip is the most precise hit source |

### 5. How others do it (pages opened)

| Tool | How the performer chooses the audio that drives visuals | Source |
|---|---|---|
| **Resolume Avenue / Arena** | Per parameter: External FFT (audio device), **Composition FFT**, or **Clip / Layer / Group FFT**. Then L / M / H buttons, Gain and Fall, and "adjusting the in and out points below the audio spectrum display" for custom bands | https://resolume.com/support/en/parameter-animation |
| **Synesthesia** | "doesn't have advanced input selection options. It will default to the first input on your device and combine all channels … into one mix." Windows "Desktop Audio" or VoiceMeeter for loopback | https://synesthesia.live/docs/faq/audio.html |
| **TouchDesigner** | Audio Device In CHOP. With ASIO, "this parameter lets you pick which input channels to use"; buffer size "will effect latency" | https://docs.derivative.ca/Audio_Device_In_CHOP |
| **LiveGrabber (Showsync, M4L)** | Exactly our topology: GrabberSender on the Main track, **AnalysisGrabber on any track** ("position it anywhere between your effects"), three filters (low / high / band pass) | https://support.showsync.com/sync-tools/livegrabber/introduction |
| **Videosync users** | Per-track M4L analysers (AnalysisGrabber, Envelope Follower) mapped to video parameters. Users call the OSC round-trip "bit fiddly" | https://forum.showsync.com/t/audio-analysis-within-ableton-livegrabber-or-equivalent/1592 |
| **Envelop for Live** (spatial *audio*, not visual) | Source Panner devices on tracks feed one Master Bus device. The wiki now states **Live 12 Suite + Max for Live** | https://github.com/EnvelopSound/EnvelopForLive/wiki |
| **Photism** | "sits on any audio track like an EQ". A "Sidechain companion device" exists in the paid version, but its behaviour is not documented on the pages opened | https://photism.app/ , https://photism.app/lite/ |
| **Zwobot** | Only "Trigger effects and crossfader by beat/tempo or sound". The audio routing is not documented on the page opened | https://www.zwobotmax.com/ |

**Take-away:**
- Nobody in the Live-plug-in space offers a performable "which tracks" control. They rely on where you drop the device (per track) or on a single mix.
- Resolume's per-layer FFT is the closest to "choose the source", but it is chosen per parameter, which is too fine for him.
- A send-fed bus is a Live-native, performable version of Resolume's "Group FFT".

### 6. Frequency control: is it worth it?

- **Today:** fixed and automatic (§2.2). Hit regions are well chosen for drums: the 160 Hz bass low-pass and a fire-on-rise kick detector were tuned on 2026-09-25.
- **Where it hurts:**
  - A bassline or 808 on a MIX or FOCUS source produces "kicks" (30-160 Hz transients).
  - Voice consonants produce "snares".
  - Airy pads can trip the high region, which the relative floor in `OnsetTracker` mostly stops.
- **Recommendation:**
  - Keep it automatic.
  - Add **band presets per source**, not free handles in PLAY:

    | Preset | Range |
    |---|---|
    | FULL | today |
    | LOW / KICK | 40-120 Hz |
    | MID / SNARE-VOICE | 200-2500 Hz |
    | HIGH / HATS-AIR | 5-16 kHz |
    | SUB / BASS | 30-90 Hz |

  - Put these in EDIT › LISTEN (source rows) and in the SOURCE view, with the existing "Detect" threshold line (dossier §4.5).
  - Free crossover handles on the spectrum can come later behind a "Custom" item.
  - For an ambient artist this is set-up work, not a show gesture. **Worth doing only after B.**
- **Feasibility:**
  - `Analyzer::prepare` builds band weights and region bins once. The analyser spec says "No allocation after prepare()".
  - Precompute the preset tables at prepare, and switch an index at runtime.
  - The low-pass (`LowPass::design`) is cheap to redesign on the worker thread.
  - About 2-3 days including tests (the existing 26 DSP acceptance tests are the harness).

### 7. Technical feasibility

#### 7.1 CPU

- **Measured on this laptop:** `analyze_wav.exe` analysed 120 s of 48 kHz stereo in **3.6 s wall time, including file reading**, i.e. at least 33× real time. One analyser therefore costs **no more than ~3 % of one core**. This is a single run on a synthetic signal.
- It runs on the plug-in's own worker thread, so it barely shows in Live's CPU meter.
- **4-6 instances ≈ 12-18 % of one core.** The UDP traffic per instance (≈ 180 small packets/s) is negligible.
- Option B **reduces** the count: one FOCUS instance can replace several role instances.
- **GPU (Iris Xe):** unaffected. The source count changes only the tiny `FeatureBus` merge; the scenes render the same.

#### 7.2 Latency (visual timing vs what is heard)

Live's delay compensation aligns every path, "including those on the return tracks" (R10). An analyser at any point in the graph sees the audio **at or before** the moment it reaches the Master's last device, never later:

| Placement | Analysis timing relative to audible | Notes |
|---|---|---|
| Master insert (today) | Baseline. Earlier by the latency of devices after it on the Master | |
| Track insert (KICK etc.) | Baseline, or **earlier** by the downstream compensation + Master chain latency | |
| **Return (VISUALS)** | Same as a Master insert (compensated sends) | Inference from R10; **test T6** |
| Device-view sidechain | Unknown for Live 10 | Whether Live 10 compensates plug-in sidechain paths is not documented in the pages we reached |
| Audio From listener track (Monitor In) | May differ if "Reduced Latency When Monitoring" is on | Live 12 manual warns of this |
| External loopback | Worst: extra driver buffers + a second device | Rejected |

- **Analysis latency is the same everywhere:** hop 256 samples (5.3 ms at 48 kHz), frames sent at 120 Hz, localhost UDP < 1 ms, events carry `ageMs` and expire after 150 ms.
- **Visual Lookahead on a FOCUS instance still works.** The return's reported latency makes Live delay everything else, even though the return is silent.

#### 7.3 MIDI

- MIDI keeps its role: `MIDI To` → a KICK instance's track, and every note is a kick.
- The FOCUS instance on a return does not take MIDI. Whether Live lets a MIDI track target a plug-in on a return track is **unknown**.
- **Quick win in Option A:** make a MIDI note on the lead or FOCUS instance usable. For example, allow the `event.midiNote` trigger in the "Impact / swell" envelope of every scene, so ambient MIDI clips (pads) can breathe the picture. Today no scene reacts to `midiNote`.

#### 7.4 OSC multi-source

- The protocol is already per-source (`sourceId`, role, seq and event IDs, de-duplication).
- The FOCUS role is **one new role string**. Old engines parse unknown roles as `mix` (`FeatureBus::parseRole`), which is harmless but wrong, so the engine and plug-in ship together.

### 8. The options in detail

#### Option A: tidy what exists (≈ 3 days)

```
 Kick trk [VJ KICK]──┐
 Snare trk[VJ SNARE]─┤  OSC (+ track name)   ┌──────────── VJ Engine ─────────────┐
 Bass trk [VJ BASS]──┼──────────────────────► │ stable MIX choice (lead / oldest)  │
 Master   [VJ MIX ★]─┘                        │ BASS -> audio.bass (bassline)      │
            ★ = the one LEAD                  │ no fake kicks from BASS/other      │
                                              │ echo /v2/sources -> lead's lamps   │
                                              └────────────────────────────────────┘
```

1. **One lead.**
   - The engine reports `leadId` in `/v2/status`.
   - The first instance that claims lead (hello flag) keeps it. Others stop sending controls and show the SOURCE view with "TAKE LEAD".
   - The default for new instances becomes "lead only if no lead exists".
   - This fixes the once-a-second fight (§2.6).
2. **Stable MIX:**
   - Use the lead's MIX, else the oldest-joined MIX, instead of the latest packet.
   - Transients from a second MIX are ignored.
   - With no MIX at all, the lead (whatever its role) is the mix, not a random `sourceId`.
3. **BASS wired:** `audio.bass.*` ← the BASS source's low aggregate when live, else the current kick-or-mix policy. BASS transients no longer become kicks.
4. **TEXTURE** is replaced by FOCUS (Option B). Until then it stops producing fallback hits.
5. **Track names:** `updateTrackProperties` → hello name, e.g. "KICK · 808 Kick".
6. **Engine echo** `/v2/sources` at 10 Hz to the lead: id, role, name, level, last-hit age. This feeds the dossier's LISTEN lamps and a read-only source list.
7. **Close the REACT leaks:** Push is gated by BASS / LEVEL, dust by LEVEL.

- **Pros:** removes every "why did that happen". It is prerequisite plumbing for B and C.
- **Cons:** no new performance gesture.

#### Option B: VISUALS return + Focus fader (≈ 4-5 days on top of A; **recommended**)

```
 Pads   ─┬─► Master (heard)          Return "VISUALS" (silent)
         └─send A──────────┐         ┌──────────────────────────┐
 Drums  ─┬─► Master        ├───────► │ VJ Analyzer  role FOCUS  │──OSC──┐
         └─send A──────────┤         │ output muted, fixed gain │       │
 Voice  ─┬─► Master        │         └──────────────────────────┘       ▼
         └─send A──────────┘                                   ┌──── VJ Engine ────┐
 Master [VJ MIX ★ LEAD: Focus fader MIX⇄VISUALS]──OSC────────► │ blend(mix, focus) │
                                                               └───────────────────┘
```

**Set-up, once per Live set:**
1. Insert a return track and name it VISUALS.
2. Drop VJ Analyzer on it and choose role FOCUS. The output mutes itself, and the SOURCE view shows a 4-step card.
3. Leave Pre/Post on **Post**: the picture hears what the audience hears. Switch to **Pre** for "ghost" feeds: a track with its fader down still drives the picture.
4. MIDI-map the VISUALS send of 4-8 key tracks, or group tracks, to Korg knobs.

**In the show:**
- The **sends** decide *which tracks*.
- The lead's **Focus fader** (host parameter `focus`, MIDI-mappable, stored in Moments) decides *how much the picture listens to VISUALS versus the whole mix*.
- **LISTEN TO** decides *which kinds* of reaction.
- For the dance show, the sends are automated in the Arrangement (cue-locked).
- In Session, a clip can carry its own send envelope.

**Pros:**
- Live-native and performable.
- One instance.
- It covers "crossfade between sources", "link to clip launching" and "MIDI mapping" with Live's own features.

**Cons:**
- One return track is used.
- The bus is a sum, so the engine cannot tell the pad from the kick inside it. Keep a KICK instance or MIDI if kick precision matters.
- A muted-output plug-in on a return is unusual, so the UI must say it loudly.

#### Option C: source board in the lead (≈ 1-1.5 weeks on top of A)

```
 Kick  [VJ src]──┐                               lead instance, EDIT › LISTEN
 Pads  [VJ src]──┤ OSC ─► Engine: weighted merge ◄── rows: ● 808 Kick  [SOLO][====  ] HITS FLOW
 Voice [VJ src]──┤        w_i per source              ● Pads      [SOLO][======] HITS FLOW
 Master[VJ MIX★]─┘                                    ● Voice     [SOLO][==    ] HITS FLOW
```

- The engine merges continuous signals as a weighted mean of the sources' relative activity. Each source's hits are scaled by its weight, with role / band mapping.
- The rows are ordered by Live track index (R2).
- Eight host parameters, `listen1..8`, give positional MIDI mapping.
- An optional "AUTO" mode sets the weights from smoothed activity with a 2-bar hold.
- **Pros:** the most precise. Per-source bands and HITS / FLOW chips.
- **Cons:**
  - The most UI.
  - N instances.
  - Slot ↔ track mapping shifts when tracks are reordered.
  - It duplicates what the sends already do.

#### Option D: sidechain FOCUS input on the lead (≈ 0.5-day spike + 2-3 days)

```
 Vocal trk ──(Live device-view Sidechain chooser)──┐
 Master [VJ MIX ★ + sidechain in] ──► Analyzer A (main = mix)
                                     Analyzer B (sidechain = focus) ──OSC──► Engine blend
```

- Add an optional stereo sidechain bus: `BusesProperties().withInput ("Sidechain", stereo, false)`.
- Accept a disabled bus in `isBusesLayoutSupported`, and read it only when active (JUCE thread above).
- Run a second `vj::Analyzer` on the worker, reporting as a FOCUS source.
- **Pros:** no return track.
- **Cons:**
  - One source only.
  - Chosen with the mouse in Live's Device View (not performable).
  - Unverified on Live 10 + JUCE.
  - Doubles the lead's analysis CPU.
- **Dominated by B.** Revisit only if he dislikes the return.

#### Option E: band presets per source (≈ 2-3 days, after A/B)

```
 source (any role) ─► Analyzer [band preset: FULL | LOW | MID | HIGH | SUB | custom]
                        onset region + hit band switch  ─► events tagged as today
```

- See §6.
- UI: a segmented chooser in the SOURCE view and in EDIT › LISTEN, with a spectrum strip that shows the chosen region.

### 9. Recommended design: A + B

#### 9.1 Mental model (the UI copy says exactly this)

- **Which tracks:** the VISUALS send on each track.
- **How much:** the Focus fader, MIX ⇄ VISUALS.
- **What kind:** the LISTEN TO lamps (KICK / SNARE / HAT / BASS / LEVEL).
- **Drums, precisely (optional):** a KICK / SNARE / HAT instance on the track, or a MIDI clip into it.

#### 9.2 Data flow and protocol (backward compatible)

- **Role index 5:** "Texture" becomes **"Focus"** (the saved parameter index is kept). The protocol string becomes `"focus"`, and the engine also accepts `"texture"` as an alias.
- **`/v2/hello`:**
  - The name becomes the Live track name.
  - New optional arg 5 is `i flags`:
    - bit0 = wants lead
    - bit1 = output muted
    - bit2 = fixed-gain normaliser
- **`/v2/listen f focus`:** from the lead only (0 = whole mix, 1 = VISUALS). Sent like the other controls, and re-sent with them.
- **`/v2/status`:** gains `i leadId` and `f focusEffective`.
- **`/v2/sources`** (engine → lead, 10 Hz): per source `i id, s role, s name, f levelAbs, f hitAgeSec`.

#### 9.3 Plug-in changes (`plugin/Source`)

- **FOCUS role behaviour:**
  1. **Output mute**, default on. `processBlock` clears the buffer after analysis. The header shows "SILENT · feeds the picture only". If the instance is not on a return (IInfoListener gives no return flag; infer it from the name, or just warn when a FOCUS instance hears audio while its track is also audible on the Master), show a warning.
  2. **Fixed-gain normaliser** (a new `NormalizerMode::fixed`: rel = the calibrated absolute mapping, e.g. -54…-6 dBFS). This makes the **send level mean something**: send at -12 dB is visibly less than 0 dB. The adaptive mode would re-learn any level within about 8 s and erase the gesture.
  3. **Level-weighted hits:** onset strength × the band's absolute level, and no hits below about -48 dBFS. A barely-open send cannot fire full kicks.
- **Lead instance:**
  - New parameter `focus` (0-1, default 1; Live name "Listen Focus").
  - Add `focus` + `reactKick…reactLevel` to `snapshotParamIds()`, so Moments store the listen state.
- **Lead election and track name** (Option A).
- **UI, consistent with DESIGN-DOSSIER.md:**
  - **PLAY:** the LISTEN row becomes `[ MIX ────●──── VISUALS ]` followed by the 5 LISTEN TO lamp-toggles.
    - The fader is greyed out, with the tooltip "add a VISUALS return (EDIT › LISTEN › How)", when no FOCUS source is live.
    - A small lamp next to VISUALS glows with the bus level. If the bus has been silent for more than 4 bars while Live plays, the lamp shows "VISUALS silent — turn up a send".
  - **EDIT › LISTEN:**
    - Focus, Impact, Decay, Listen amount, Calm, as in the dossier.
    - A **Sources** list: name, role, level bar, hit lamp (read-only in A/B, and the seed of Option C).
    - A "How to feed the picture" 4-step card.
  - **SOURCE view (FOCUS):** a big level meter, a "last hit" lamp, output-muted state, the band chooser (Option E later), and the same 4-step card.

#### 9.4 Engine changes (`engine/Source`)

- **`FeatureBus`:**
  - Stable `mix` (lead or oldest) and stable `focus` (oldest FOCUS).
  - `f = focusParam × fade(focus)`. Every continuous field becomes `lerp(mix, focus, f)`: level, bass / mid / high, bands, spectrum, descriptors and energyTrend. `build` / `presence` therefore follow what is listened to. Transport stays from the mix.
  - **Events:**
    - MIX transients: strength × (1-f).
    - FOCUS transients: strength × f.
    - Both keep the stand-in kick / snare / hat rule while no dedicated role source is live.
    - Drop anything under 0.05.
  - BASS role wiring and removal of the TEXTURE fallback (Option A).
  - `describeSources` includes the names.
  - New `/v2/sources` echo.
- **`Signals`:** `float focus`, `bool focusLive`, and the `RoleActivity` for focus. The `SourceRole` enum name changes from texture to focus.
- **`Modulation` / `MainComponent`:**
  - `sourcePolicy` "bass" gains `"bass-or-kick-or-mix"` (the new default).
  - REACT gates on Push and dust.
- **`MainComponent`:** handle `/v2/listen`, `leadId`, `/v2/sources`.
- **Presets:** no JSON change needed. Optionally, add an `event.midiNote` → swell trigger in the ambient scenes (Option A quick win).

#### 9.5 Test plan

**Offline (no Live):**
1. `analyze_wav drums.wav --send --role mix` plus `analyze_wav pads.wav --send --role focus` at the same time. Script `/v2/listen` 0 → 1 → 0 and log `Signals` (engine diagnostics). Pass if:
   - `bassRel` / `levelRel` move from the drums to the pads within ≤ 1 frame of the smoothing time;
   - kicks fade out in strength as the fader rises;
   - nothing jumps.
2. **Two MIX senders:** assert no alternation (the chosen source id is constant for 60 s).
3. **Two lead claimants:** assert that only one sends controls, and that a macro set on the lead is not reverted within 5 s.
4. **BASS source with a sustained bassline:** `audio.bass.activity` follows it, and no `event.kick` comes from it.
5. **Fixed normaliser:** -12 dB input reads ≤ 60 % of the 0 dB reading after 20 s (the adaptive mode reads the same). Hits are suppressed below -48 dBFS. Add both to `analysis_tests`.
6. The existing `preset_regression_test.py` and `soak_test.py` still pass.

**In Live 10.1.43 (owner's laptop):**
- **T1 (Option D gate):** insert a test build with a sidechain bus. Does Device View show a Sidechain chooser for the VST3? Does audio arrive? (30 min.)
- **T2:** VISUALS return + FOCUS instance. Confirm no audio leaks to the Master at any fader position.
- **T3:** return fader at -inf with output mute off. Does the analyser still see signal (R6)?
- **T4:** MIDI-map a send to the Korg. Does the picture follow within ~100 ms?
- **T5:** Arrangement send automation, and a Session clip send envelope. Does the picture follow?
- **T6:** put a latency-reporting plug-in (e.g. a Limiter with lookahead) on the Pads track. Kick flashes from VISUALS vs the Master instance must be aligned by eye and by the engine's event-age log.
- **T7:** mute (Track Activator) a track with a Post send, and then with Pre. Does it still feed? (This decides the "ghost track" advice.)
- **T8:** CPU meter with 1 vs 5 instances. Record Live's meter and Task Manager.
- **T9 (the owner's click path, per the memory "show results immediately"):** from an empty set, follow the card. The picture must visibly change on the first send turn. If it does not, the UI must say why.

#### 9.6 Effort

| Item | Days |
|---|---|
| A. Lead election + controls fight fix | 0.75 |
| A. Stable MIX, BASS wiring, TEXTURE fallback removal, REACT leaks | 0.75 |
| A. Track names + `/v2/sources` echo + lead lamps / source list | 1.25 |
| B. FOCUS role: output mute, fixed normaliser, level-weighted hits (+ tests) | 1.5 |
| B. FeatureBus blend + events + `/v2/listen`, Signals | 1 |
| B. Focus parameter, snapshots, PLAY / LISTEN UI, set-up card | 1.25 |
| Docs (USAGE, QUICKSTART-HE) + in-Live test session | 1 |
| **Total A + B** | **≈ 7.5 days** |
| Option C (later) | +5-7 |
| Option D (spike, then decide) | +0.5 / +2-3 |
| Option E presets | +2-3 |

### 10. Open questions for the owner

1. **Post or Pre** for the VISUALS return by default? Post means "the picture hears what the audience hears". Pre enables "ghost" tracks that only drive the picture.
2. When every VISUALS send is down and the fader is on VISUALS, should the picture **rest** (honest: nothing is sent) or **fall back to the whole mix**?
3. For the dance show, will the listen changes be **written as Arrangement automation** (cue-locked) or **played on the Korg**? This decides whether Focus goes on a Korg fader or stays in Moments.

### 11. Sources opened

**Ableton:**
- https://www.ableton.com/en/release-notes/live-10/ (downloaded and grepped: 10.1, 10.1.2, 10.1.9 bullets quoted above)
- https://www.ableton.com/en/release-notes/live-11/ (grepped for latency and sidechain items)
- Live 12 manual pages returned by the Ableton Knowledge tool:
  - https://www.ableton.com/en/manual/working-with-instruments-and-effects/
  - https://www.ableton.com/en/manual/routing-and-i-o/
  - https://www.ableton.com/en/manual/mixing/
  - https://www.ableton.com/en/manual/clip-envelopes/
  - https://www.ableton.com/en/manual/midi-and-key-remote-control/
- **Not readable:**
  - https://help.ableton.com/hc/en-us/articles/209775325-Sidechaining-a-third-party-plug-in (403)
  - the Live 10 manual PDF (too large)
  - https://www.ableton.com/en/live-manual/10/ pages (redirect loop)

**Third party:**
- https://ringmodsidechain.com/tutorials/sidechain-routing-in-ableton-live-complete-guide (conflicts with Ableton's notes; see §3)
- https://forum.juce.com/t/vst3-and-au-behave-differently-in-ableton-10/42849
- https://forum.juce.com/t/vst3-synth-in-ableton-sidechains-to-itself-when-no-sidechain-input-is-selected/51507
- https://resolume.com/support/en/parameter-animation
- https://synesthesia.live/docs/faq/audio.html
- https://docs.derivative.ca/Audio_Device_In_CHOP
- https://support.showsync.com/sync-tools/livegrabber/introduction
- https://forum.showsync.com/t/audio-analysis-within-ableton-livegrabber-or-equivalent/1592
- https://support.showsync.com/videosync/basics/working-with-instruments-and-effects
- https://support.showsync.com/videosync/device-reference/isf-shader (no audio-input information on this page)
- https://github.com/EnvelopSound/EnvelopForLive/wiki
- https://photism.app/
- https://photism.app/lite/
- https://www.zwobotmax.com/

**Local code** (all claims in §2):
- `engine/Source/FeatureBus.{h,cpp}`, `Signals.{h,cpp}`, `Modulation.{h,cpp}`, `MainComponent.cpp`, `LookPass.cpp`, `PresetV2.cpp`
- `plugin/Source/PluginProcessor.{h,cpp}`, `AnalysisWorker.{h,cpp}`, `PluginEditor.cpp`
- `analysis/include/vj/*.h`, `analysis/src/Analyzer.cpp`
- `engine/Presets/*.json`
- JUCE `juce_audio_plugin_client_VST3.cpp`
