# חומרים מפרויקטים קודמים — מה לקחנו ולמה

עותקים (לא קישורים) של קוד ומסמכים מארבעה פרויקטים קודמים, שנאספו 23.09.2026.
הקבצים כאן הם **חומר עיון והשראה**, לא חלק מהבילד. כשמשהו מכאן נכנס למנוע —
הוא עובר המרה (בד"כ ל-ISF / C++ / פאץ' Max) ונכנס לתיקייה המתאימה בפרויקט.

לא הועתקו בכוונה: `node_modules`, `dist`, `three.min.js` (ספרייה חיצונית), מודלים `.glb`
(נכסי דוגמה של three.js), קבצי TouchDesigner `.toe`, ומדיה כבדה (פריימים/וידאו).
המקור נשאר במקומו — ראה נתיבים בכל סעיף.

---

## 1. Revenant — `C:\Users\ROI\silenced wind\revenant`

**מה זה:** ראש סרוק (three.js) שמתפרק לנקודות לפי הסאונד, עם datamosh, הבזק כחול, bloom ו-grain.
M4L Audio Effect: `plugin~` → 8 פילטרי `reson~` → `node.script` (Node for Max) → WebSocket → חלון `jweb`.
זה המקור הויזואלי של הפריסטים `07 - Revenant Portrait` / `08 - Signal Portrait` אצלנו.

| קובץ | למה שווה |
|---|---|
| [src/audio.js](revenant/src/audio.js) | **זיהוי kick/hat נפרד** מ-spectral flux: סף אדפטיבי (ממוצע רץ × מקדם + רצפה), cooldown נפרד (90ms לבס, 60ms להיי — כמעט זהה למה שהמחקר הציע: 90/45). החלקת bands ב"קפיץ" (spring) במקום one-pole. מצב **demo** סינתטי ו-**mic fallback** — עובד בלי Ableton. |
| [src/loop.js](revenant/src/loop.js) | שכבת מיפוי עם **היררכיית תפקידים**: kick→התפרקות, hat→גליץ', kick חזק→הבזק כחול, mid→נשירה. בדיוק "role hierarchy" מהמחקר. |
| [src/post.js](revenant/src/post.js) | שיידרי פוסט מוכנים להמרה ל-ISF: **Datamosh** (slices / pixel-drag / blocks / seam), **Blue Invert Flash**. סדר השרשרת: Bloom+CA → Datamosh → Noise+Scanline+Vignette+Flash, על **HalfFloat** (HDR) — זה צינור ה-finish שהמחקר מבקש. |
| [max/build_amxd.js](revenant/max/build_amxd.js) | **מחולל פאץ' M4L מקוד** (JS) שכותב גם `.maxpat` וגם `.amxd` ישירות — כולל `live.dial` עם `parameter_*` מלאים. אצלנו יש רק `compile_amxd.py` (עוטף) — זה המשלים: מכשירים חדשים (MIDI Bridge, Analyzer v2) כדאי לייצר מקוד ולא לערוך JSON ידנית. |
| [max/bridge.js](revenant/max/bridge.js) | גשר Node for Max: HTTP+WS על אותו פורט, `--test` עם דפוס kick סינתטי 110BPM. |
| `extract_frames.mjs`, `shoot.mjs` | חילוץ פריימי רפרנס מוידאו + צילום מסך אוטומטי (Puppeteer) — לבדיקות ויזואליות. |

מדיה שנשארה במקור: `reference/ref_*.png` (12 פריימי רפרנס), `public/models/LeePerrySmith.glb`, `shots/`.

## 2. Spectro Tunnel — `C:\Users\ROI\spectro-tunnel`

**מה זה:** מנהרה/טופוגרפיה ספקטרלית (three.js) עם "עולמות" מתחלפים, קרני אור מטרנזיינטים, פלטות "BoC", feedback ו-bloom. רץ על ה-Master ב-Live 10.

| קובץ | למה שווה |
|---|---|
| [max/build_maxpat.js](spectro-tunnel/max/build_maxpat.js) | מחולל הפאץ' המלא ביותר: פילטרבנק `reson~` (CF 60…12k, Q 5…12), **`plugsync~`** לביט/בר (מה שהמחקר ממליץ במקום polling), `transport` ל-BPM, תת-פאץ' `pfft~` שנבנה מקוד, `live.menu` למצב טריגר, 10 `live.dial`. **הערה:** הניסיון להוציא מגניטודות מ-`pfft~` לרשימה לא הושלם (ראה ההערות בקוד) — זו בדיוק הסיבה שאני ממליץ על external ב-C++ ל-FFT. |
| [max/bridge.js](spectro-tunnel/max/bridge.js) | **מצבי טריגר TRANSPORT / TRANSIENT / HYBRID** + שני ספים (punch / cut) — דפוס טוב ל"מתי מחליפים סצנה": שעון Live, מכה, או שילוב. |
| [engine/engine.js](spectro-tunnel/engine/engine.js) | צינור פוסט פשוט ועובד: feedback-zoom עם `max(scene, prev*decay)`, bright-pass, **blur גאוסיאני 9-tap בשני כיוונים**, composite סופי עם glitch/grain/cut. מערכת "עולמות" + `triggerCut()` (החלפת פלטה+עולם+גלגול מצלמה על כל cut) ו-`autoRandomize()` בטווחים מוגבלים. פלטות MOODS. |
| `audio_visualizer_prototype_v6.html` | אב-טיפוס מוקדם, דפדפן בלבד. |
| [max/verify_pipe.js](spectro-tunnel/max/verify_pipe.js) | בודק אוטומטי שהגשר מגיש HTTP ומזרים WS — דפוס לבדיקות end-to-end בלי Ableton. |

שיעור שחזר גם אצלנו: `max/_stale_userlib_backup/` — עותק ישן ב-User Library שגבר על החדש. (אותו באג שתוקן אצלנו ב-22.09.)

## 3. 3D coding — `C:\Users\ROI\3D coding`

| קובץ | למה שווה |
|---|---|
| [KNOWLEDGE.md](3d-coding/KNOWLEDGE.md) §15 | **מתודולוגיית ניתוח רפרנס** (פלטה/טקסטורה/מבנה/תנועה/מצלמה/זמן → טבלת "סימפטום→טכניקה" → מימוש → אימות על פיקסלים אמיתיים) + **checklist מלכודות** שנתקלנו בהן בפועל. להשתמש בזה בכל פריסט חדש. |
| [learning/RESEARCH-GLITCH-AESTHETICS.md](3d-coding/learning/RESEARCH-GLITCH-AESTHETICS.md) | קטלוג טכניקות גליץ' (datamosh, pixel sort, RGB split, VHS, ASCII, matrix rain, עיוות גאומטריה) + סנכרון לביט. |
| [learning/RESEARCH-VIDEO-ART.md](3d-coding/learning/RESEARCH-VIDEO-ART.md) | למה TD נראה טוב + מתכונים: feedback, GPGPU particles, bloom, instancing, noise displacement, slit-scan/Rutt-Etra. |
| [learning/RESEARCH-ABLETON-RESOLUME-BRIDGE.md](3d-coding/learning/RESEARCH-ABLETON-RESOLUME-BRIDGE.md) | NDI/Spout ל-Resolume, MIDI Clock, ableton-js, VB-Cable. כולל טבלת **מה מותקן במחשב** (NDI Tools, loopMIDI, VB-CABLE; Videosync/LiveGrabber מותקנים אך לא תואמים ל-Live 10). |
| [learning/RESEARCH-BERLIN-VIDEOART.md](3d-coding/learning/RESEARCH-BERLIN-VIDEOART.md) | כיוון אסתטי: תעשייתי-מונוכרומטי, אמנים לעקוב. |
| [bridge/server.js](3d-coding/bridge/server.js) | גשר ableton-js ↔ WebSocket. **שיעור חשוב בהערות:** polling של מטרים דרך ableton-js גרם לגמגום בטמפו/ביט — ה-Live API איטי; לא לדגום דרכו בקצב גבוה. תומך ב-"lastKnown" לקליינט שמתחבר באמצע. |
| [glitch-face/src/glitchShader.js](3d-coding/glitch-face/src/glitchShader.js) | שיידר גליץ' לפי **48 bands אנכיים** (tear + aberration מהבהב + feedback + grain + scanlines) — מועמד ישיר ל-ISF. |
| [glitch-face/src/rng.js](3d-coding/glitch-face/src/rng.js) | `heldRandom()` — אקראיות **דטרמיניסטית** שקופצת לערכים ומחזיקה (נראית כמו השחתת סיגנל, לא "נשימה"). בדיוק ה"seeded S&H" שהמחקר דורש, וגם מאפשר רינדור אופליין זהה ללייב. |
| `glitch-face/capture.js` + `server.mjs` | רינדור אופליין פריים-פריים → MP4. |

לא הועתק: `touchdesigner-mcp-td/` (קוד צד שלישי), קבצי `.toe`, `references/` (≈150MB וידאו).

## 4. M4L Connection Kit (Ableton, רישיון MIT) — `...\coding\m4l-connection-kit-main`

מכשירי Max 8 רשמיים של Ableton. רלוונטיים ארבעה:

| מכשיר | למה שווה |
|---|---|
| [OSC MIDI Send](m4l-connection-kit/OSC%20MIDI%20Send/) | `midiin → midiparse → poly → udpsend` + `midiout` (pass-through). **זה השלד של ה-"VJ MIDI Bridge"** מהמפרט — תווים שמתנגנים בפועל → OSC, בלי לשבור את הכלי שאחריו. |
| [OSC Send](m4l-connection-kit/OSC%20Send/) | כפתור **Map** לכל פרמטר ב-Live → כתובת OSC, עם min/max/**curve**. דפוס מוכן ל"כל נוב ב-Live יכול לשלוט במנוע". |
| [OSC TouchOSC](m4l-connection-kit/OSC%20TouchOSC/) | דפוס **Learn** (לומד כתובת נכנסת) + Map. |
| [OSC Monitor](m4l-connection-kit/OSC%20Monitor/) | מוניטור OSC לדיבאג. **שים לב:** לא להריץ על 9000 במקביל למנוע (אותה בעיית "שני מאזינים" מ-USAGE.md). |

---

## איך זה משתלב בתוכנית (Phase 3)

1. **ניתוח אודיו:** הלוגיקה של `revenant/src/audio.js` (flux נפרד לבס/היי, סף אדפטיבי, cooldown) היא אב-טיפוס עובד של מה שה-AnalysisCore צריך — נשדרג אותה לפי המפרט (median+MAD, נרמול Q20/Q95) ונשתמש בה כ-baseline להשוואה בבדיקות.
2. **בדיקה בלי Ableton:** לאמץ את דפוס `--test`/demo: מקור אודיו סינתטי שמזין את אותו חוזה — גם במנוע שלנו.
3. **מכשירי M4L חדשים מקוד:** לאחד את `build_maxpat.js`/`build_amxd.js` עם `compile_amxd.py` שלנו למחולל אחד; `plugsync~` לשעון; MIDI Bridge על בסיס OSC MIDI Send.
4. **Finish pipeline (Phase 4):** bright-pass + blur דו-כיווני מ-Spectro Tunnel, סדר השרשרת מ-Revenant (HalfFloat → bloom → datamosh → grain/scanline/vignette → flash).
5. **שיידרים להמרה ל-ISF:** Datamosh (slices/drag/blocks/seam), Blue Invert Flash, Band-Tear Glitch (48 bands), Feedback-Zoom.
6. **החלפת סצנות:** מצבי TRANSPORT/TRANSIENT/HYBRID + "cut" שמשנה פלטה+עולם יחד.
7. **אפשרות שלא הייתה על השולחן:** `node.script` (Node for Max) רץ ב-Max 8 בתוך M4L — מתאים ללוגיקה (נרמול, ספים, מצבים, רשת), **לא** ל-FFT בקצב אודיו. יכול להיות דרך ביניים זולה לפני/במקום external ב-C++ לחלק הלוגי.
