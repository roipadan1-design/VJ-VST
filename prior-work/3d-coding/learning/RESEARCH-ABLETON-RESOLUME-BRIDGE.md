# מחקר: שידור לResolume + חיבור ל-Ableton Live 10 (אודיו + קצב + אוטומציות)

המשך ל-[RESEARCH-VIDEO-ART.md](RESEARCH-VIDEO-ART.md). נכתב 2026-07-20. המטרה: לבנות צינור מלא: **הקוד שלנו (דפדפן) → Resolume Arena → פרוג'קטור/לדים**, כשה-**קצב וסאונד מגיעים מ-Ableton Live 10** ומניעים אוטומציות והתניות בוויז'ואל.

---

## ארכיטקטורת המערכת (סקירה)

```
Ableton Live 10 ──┬── MIDI Clock ──► loopMIDI (כבל וירטואלי) ──► Web MIDI API ──┐
                   │                                                            │
                   └── Remote Script (ableton-js) ──► Node.js bridge ──WebSocket┤
                                                                                 ▼
Ableton audio out ──► VB-Cable (כבל אודיו וירטואלי) ──► getUserMedia ──► Web Audio FFT ──► sketch.js (p5.js)
                                                                                 │
                                                                                 ▼
                                                                    Canvas WebGL ──► Spout/NDI ──► Resolume Arena ──► יציאת פרוג'קטור/LED (מובנה ב-Resolume)
```

שלושה נתיבי מידע נפרדים, כל אחד לצורך אחר:
1. **MIDI Clock** — קצב מדויק (BPM, פאזת ביט/בר) בלי שום קוד בצד Ableton. הכי פשוט ואמין.
2. **ableton-js** — גישה לכל מודל האובייקטים של Live: אילו קליפים מתנגנים, ערכי מאקרו/פייידרים, מצב טרנספורט. זה נותן "התניות" (if track X playing → visual Y).
3. **VB-Cable + Web Audio FFT** — אודיו אמיתי (bass/mid/treble) ישירות בדפדפן, בלי תלות ב-Ableton בכלל.

השידור החוצה (וידאו): הדפדפן → Spout/NDI → Resolume → פרוג'קטור/LED (הפלט הפיזי כבר טבעי ב-Resolume, לא צריך כלי נוסף).

---

## חלק א׳: שידור הוויז'ואל ל-Resolume

### אופציה 1 — NDI (מומלץ להתחלה: חינמי, רשמי, יציב)
NDI עובד ברשת (גם lokal-loopback על אותו מחשב) ומכניס דחיסה קלה → עיכוב זעיר, אבל אפס עלות והכי נתמך.

**התקנה:**
1. הורדה: NDI Tools מ-[ndi.video](https://ndi.video/tools/) (חינם).
2. ⚠️ ב-**Windows** הכלי נקרא **"Screen Capture"** (לא "Scan Converter" — זה השם בגרסת Mac בלבד!). יש גם **Screen Capture HX** משודרג (האצת GPU, דחיסה) להורדה נפרדת: [ndi.video/tools/screen-capture](https://ndi.video/tools/screen-capture/).
3. הרצת **Screen Capture** → לבחור את חלון הדפדפן שמריץ את [index.html](../index.html) (או אזור מסך מוגדר) → לתת שם NDI.
4. ב-Resolume Arena: Sources panel → NDI → יופיע המקור אוטומטית.

מקורות: [NDI Inputs/Outputs – Resolume](https://resolume.com/support/NDI_inputs_and_outputs), [דיון בפורום](https://resolume.com/forum/viewtopic.php?t=14317).

### אופציה 2 — Spout (איכות מקסימלית, אפס דחיסה, אבל בתשלום קל)
Spout משתף טקסטורת GPU ישירות (DirectX/OpenGL) על אותו מחשב — **אפס דחיסה, אפס latency נוסף**. עדיף אם המחשב שמריץ את הדפדפן הוא אותו מחשב שמריץ את Resolume.

- **SpoutBrowser** — דפדפן Chromium ייעודי עם Spout sender מובנה, בדיוק בשביל להריץ WebGL/p5/Hydra ולשלוח ל-Resolume. קוד פתוח, אפשר לבנות בעצמכם מ-[GitHub](https://github.com/bntre/SpoutBrowser) (חינם) או להוריד בינארי מוכן מ-[itch.io](https://bntr.itch.io/spout-browser) (תרומה קטנה, ~$6, גרסה עם/בלי watermark). תומך ערוץ אלפא (רקע שקוף)!
- אלטרנטיבה: [cef-spout](https://github.com/fg-uulm/cef-spout) — Chromium Embedded עם רינדור off-screen + Spout, לגרסה יותר "תכנותית".

**המלצה:** להתחיל עם NDI (חינמי, 10 דקות הקמה) ולשדרג ל-Spout/SpoutBrowser כשה-latency יהיה קריטי (מופע חי).

---

## חלק ב׳: קצב מ-Ableton — MIDI Clock (הכי פשוט, מומלץ ראשון)

עובד עם **כל גרסת Ableton**, בלי Remote Script, בלי Python.

### כלים להתקנה
1. **[loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html)** (Tobias Erichsen, חינם) — יוצר פורט MIDI וירטואלי ב-Windows.
2. אין צורך בכלום בצד JavaScript — **Web MIDI API מובנה בכרום**, אין ספרייה להתקין.

### הקמה
1. התקינו loopMIDI, צרו פורט בשם `Ableton2Viz`.
2. ב-Ableton: Preferences → Link/Tempo/MIDI → תחת MIDI Ports, הפעילו **Sync Output** על הפורט `Ableton2Viz`.
3. ב-Ableton: Preferences → Link/Tempo → Send MIDI Clock = On (או פשוט הפעלת Sync על אותו פורט שולח 24 pulses-per-quarter-note MIDI Clock אוטומטית ברגע שהטרנספורט רץ).
4. בדפדפן: `navigator.requestMIDIAccess()` → להאזין להודעות MIDI Clock (status byte `0xF8`), לספור 24 פולסים = רבע תיבה, לחשב BPM ולעדכן פאזת ביט בזמן אמת.

זה מספיק כדי לכתוב תנאים כמו: "כל 4 תיבות – להחליף סצנה", "על כל רבע – פעימת bloom".

---

## חלק ג׳: שליטה עשירה — ableton-js (תואם Live 10!)

⚠️ חשוב: **AbletonOSC** (הפרויקט הפופולרי ביותר) **דורש Live 11+ ולא תומך ב-Live 10**. עבור Live 10 יש שתי חלופות תומכות:

| כלי | תמיכה בגרסה | פרוטוקול | מה מקבלים |
|---|---|---|---|
| **[ableton-js](https://github.com/leolabs/ableton-js)** (leolabs) | Live 10 **וגם** 11 | UDP+JSON, עטוף ב-Node.js API נוח | tempo, is_playing, track/clip state, meter levels (!), macros — הכל דרך אובייקטים JS |
| **[LiveOSC](https://github.com/ideoforms/LiveOSC)** (הפורק הישן) | Live 9.6–10 בלבד | OSC גולמי | תמיכה דומה אך API גולמי יותר, קהילה קטנה יותר |

**המלצה: ableton-js** — פעיל, מתוחזק, API נוח ב-TypeScript, ומחזיר גם **meter levels** (עוצמת שמע לפי track!) — כלומר אפשר לקבל ריאקטיביות בסיסית לאודיו *בלי בכלל VB-Cable*, ישירות מ-Ableton.

### התקנה (Windows, Live 10)

```bash
# בתיקיית bridge/ של הפרויקט
npm install ableton-js ws
```

1. להעתיק את תיקיית `midi-script` (מגיעה בתוך חבילת ableton-js ב-`node_modules/ableton-js/midi-script`) אל:
   `%USERPROFILE%\Documents\Ableton\User Library\Remote Scripts\AbletonJS`
2. ב-Ableton: Preferences → Link/Tempo/MIDI → Control Surface → לבחור `AbletonJS`.
3. להפעיל מחדש את Ableton.

### דוגמת קוד (bridge server)
ר' [bridge/server.js](../bridge/server.js) שכתבתי בפרויקט — מתחבר ל-Ableton, מאזין ל-tempo/is_playing/meter, ומשדר הכל דרך WebSocket לדפדפן.

מקורות: [ableton-js README](https://github.com/leolabs/ableton-js), [AbletonOSC (להשוואה, Live 11+ בלבד)](https://github.com/ideoforms/AbletonOSC), [LiveOSC (חלופה ל-Live 10)](https://github.com/ideoforms/LiveOSC).

---

## חלק ד׳: אודיו אמיתי (FFT) — VB-Cable

לריאקטיביות עשירה (bass/mid/treble נפרדים, לא רק עוצמה כללית):

### התקנה
1. **[VB-CABLE](https://vb-audio.com/Cable/)** (חינם, תרומה אופציונלית) — כבל אודיו וירטואלי ל-Windows.
2. ב-Windows Sound Settings: לקבוע את **פלט** Ableton (או Master output device) ל-`CABLE Input`.
   - טיפ: אם רוצים גם לשמוע את הסאונד "בחיים" בו-זמנית (לא רק לשלוח ל-CABLE), אפשר להשתמש ב-[VoiceMeeter](https://vb-audio.com/Voicemeeter/) (גם מבית VB-Audio) שמאפשר Mix של פלט למספר יעדים.
3. בדפדפן: `navigator.mediaDevices.getUserMedia({audio: {deviceId: ...}})` עם בחירת `CABLE Output` כמכשיר הקלט (כמו מיקרופון מזויף).
4. `p5.sound`: `new p5.AudioIn()` → לבחור את אותו device → `new p5.FFT()` → `fft.getEnergy("bass"/"mid"/"treble")`.

מקורות: [VB-Audio Cable](https://vb-audio.com/Cable/), [דוגמה חיה — MangoWave](https://github.com/Louis-Mascari/MangoWave).

---

## המלצת סדר הקמה (מהקל לכבד)

1. **NDI** — 10 דקות, לוודא שהוויז'ואל בכלל מגיע ל-Resolume ולפרוג'קטור.
2. **MIDI Clock (loopMIDI)** — קצב בסיסי, מאפשר כבר "אוטומציות לפי BPM" (פעימות, סצנות שמתחלפות כל N תיבות).
3. **VB-Cable + p5.sound FFT** — ריאקטיביות אמיתית לאודיו (bass מזיז את הכדור, treble מבזיק).
4. **ableton-js bridge** — כשצריך "תנאים" אמיתיים (איזה קליפ מתנגן, מצב מאקרו) ולא רק קצב/אודיו גולמי.
5. שדרוג ל-**Spout/SpoutBrowser** כש-latency הופך קריטי למופע חי.

---

## סטטוס בפועל (נבדק 2026-07-21)

| רכיב | סטטוס | הערה |
|---|---|---|
| NDI Tools | ✅ מותקן | הכלי בשם **Screen Capture** (לא Scan Converter — זה Mac) |
| loopMIDI | ✅ מותקן | צריך עדיין ליצור פורט ולהפעיל Sync Output ב-Ableton |
| VB-CABLE | ✅ מותקן | צריך עדיין להגדיר כפלט של Ableton כשנרצה FFT אמיתי |
| Node.js + bridge deps | ✅ מותקן | `npm install` רץ בהצלחה בתיקיית `bridge/` |
| AbletonJS remote script | ⏳ עדיין לא הועתק | ר' שלבים למטה |
| Ableton Live | Live 10 Suite בלבד | **הוחלט להישאר על 10** (לא לשדרג ל-12) |
| Videosync / VIZZable / LiveGrabber | מותקנים אך **לא בשימוש** | Videosync 2.3.2 דורש Live 12 ב-Windows; LiveGrabber דורש Live 11.1+. לא תואמים ל-Live 10 שלנו — משאירים בצד, לא מוחקים. |

### הצעדים שנשארו להשלמת ה-bridge
1. להעתיק `bridge/node_modules/ableton-js/midi-script` → `%USERPROFILE%\Documents\Ableton\User Library\Remote Scripts\AbletonJS`
2. ב-Ableton: Preferences → Link/Tempo/MIDI → Control Surface → לבחור `AbletonJS` → להפעיל מחדש
3. `cd bridge && npm start` — לוודא בלוג "Connected to Ableton Live"
4. לפתוח את **NDI Screen Capture**, ללכוד את חלון הדפדפן עם [index.html](../index.html), לוודא שהמקור מופיע ב-Resolume

## קבצים שנוצרו בפרויקט בעקבות המחקר

- [bridge/package.json](../bridge/package.json), [bridge/server.js](../bridge/server.js) — שרת גשר Node.js ל-Ableton (ableton-js) + WebSocket לדפדפן.
- [bridge/README.md](../bridge/README.md) — הוראות הרצה מהירות.
