# מחקר עומק: TouchDesigner, וידאו-ארט ואיך מרחיבים את היכולות שלנו

מסמך המשך ל-[KNOWLEDGE.md](../KNOWLEDGE.md). נכתב 2026-07-20.

---

## 1. TouchDesigner — למה זה נראה כל כך טוב

### הארכיטקטורה (מה אפשר ללמוד ממנה)

TouchDesigner בנוי ממשפחות אופרטורים שמחוברים בגרף זרימת נתונים:

| משפחה | תפקיד | המקבילה אצלנו |
|---|---|---|
| **TOPs** (Texture) | כל פעולות התמונה — רצות על GPU, מהירות מאוד | Framebuffers + שיידרים |
| **CHOPs** (Channel) | אותות שליטה: אודיו, תנועה, LFO, טיימינג | משתני JS, p5.sound FFT, sin/noise |
| **SOPs** (Surface) | גאומטריה תלת מימדית (CPU — דווקא איטי!) | p5.Geometry / buildGeometry |
| **POPs** (Point) | נקודות/חלקיקים על GPU — הדור החדש | GPGPU בטקסטורות (ר' סעיף 3) |
| **MATs** | חומרים | materials + שיידרים |
| **DATs** | טבלאות/קוד | JS רגיל |

**התובנה המרכזית:** הכוח של TD הוא לא קסם — זה *צינור GPU עקבי*: כל דבר הוא טקסטורה, כל טקסטורה יכולה לשלוט בכל דבר (תמונה → תזוזת קודקודים → צבע → חלקיקים). את אותו עיקרון אנחנו בונים עם framebuffers ושיידרים.

### מתכוני החתימה של TD (מה שהופך את התוצאות ליפות)

1. **Feedback loop** — הפריים הקודם מוזן חזרה עם טרנספורמציה קלה (זום/סיבוב/טשטוש/דעיכה) → שובלים, עשן, אינסוף. האפקט המזוהה ביותר עם TD.
2. **Instancing מונע-נתונים** — אלפי עותקים של צורה אחת, כשטקסטורה/אודיו קובעים מיקום, גודל, צבע וסיבוב של כל עותק.
3. **Noise displacement** — Noise (Perlin/simplex/alligator...) שמזיז קודקודים של משטח/ספירה + תאורה → הבלובים האורגניים המפורסמים.
4. **GPU particles** — מיקומי חלקיקים חיים בטקסטורת float; שיידר מעדכן אותם כל פריים בלולאת feedback. מיליוני חלקיקים.
5. **Audio-reactive pipeline** — Audio In → Analyze (פיצול לתדרים: bass/mid/high) → Lag/Filter (החלקה!) → מיפוי לפרמטרים ויזואליים. ההחלקה היא הסוד — בלעדיה הכל מרצד.
6. **Post-processing stack** — Bloom/Glow, Blur, Edge detection, Chromatic aberration, Color grading. שכבת הליטוש שגורמת להכל להיראות "יקר".
7. **Rutt-Etra** — בהירות תמונה → תזוזת Z של קווי סריקה תלת מימדיים (מחווה לסינתיסייזר וידאו משנות ה-70).
8. **Slit-scan / Time Machine** — כל שורת פיקסלים נדגמת מפריים אחר בזמן → מריחות זמן סוריאליסטיות.
9. **Point clouds** — סריקות תלת מימד/מצלמות עומק כעננים של נקודות + feedback.

מקורות: [Derivative – audio reactive + GLSL](https://derivative.ca/community-post/tutorial/touchdesigner-audio-reactive-visuals-glsl/73339), [Interactive & Immersive HQ](https://interactiveimmersive.io/blog/touchdesigner-lessons/audio-reactive-visuals-a-beginner-guide/), [סיכום טכניקות noise](https://cdm.link/touchdesigner-noise-tutorials/), [Noise displacement](https://www.simonaa.media/tutorials/noisedisplacement), [Slit-scan](https://alltd.org/slit-scan-effect-touchdesigner-tutorial/), [תיעוד אופרטורים](https://docs.derivative.ca/Operator).

---

## 2. מפת האלטרנטיבות

מתוך [השוואת סביבות וידאו בזמן אמת](https://github.com/hrtlacek/rtv) ומקורות נוספים:

- **TouchDesigner** — הסטנדרט המקצועי להתקנות ומופעים. חינם ללימוד (עד 1280x1280), רישיון בתשלום למסחרי. Windows/macOS. גרפי-נודים + Python.
- **vvvv gamma** — מקבילה אירופאית, חינמית יותר, חזקה במולטי-משתמש והתקנות פיזיות. Windows.
- **Notch** — הכי "הוליוודי": מוטת רינדור עצומה, משולב ישירות בשרתי מדיה של הופעות ענק. יקר.
- **Max/MSP + Jitter** — הוותיקה; חזקה באודיו, משולבת ב-Ableton (Max for Live!). רלוונטי לנו כי אתה בעולם של אבלטון.
- **Hydra** (hydra.ojack.xyz) — לייב-קודינג של וידאו-סינת' בדפדפן, בהשראת סינתזה אנלוגית מודולרית: מקורות → טרנספורמציות → מודולציה → feedback בשרשור פונקציות קצר. **משתלב ישירות עם p5.js!** חינמי וקוד פתוח.
- **cables.gl** — עורך נודים ב-WebGL בדפדפן — "TouchDesigner של הרשת". חינמי, אפשר לייצא פרויקטים.
- **Isadora / Vuo / Smode** — ממוקדי תיאטרון/מופע.
- **KodeLife / Shadertoy** — לייב-קודינג GLSL טהור. Shadertoy = מכרה הזהב הגדול בעולם של קוד שיידרים.
- **three.js / openFrameworks / openrndr** — ספריות קוד (כמו p5 אבל חזקות/מהירות יותר). three.js היא המסלול הטבעי שלנו קדימה כשנתקע בתקרת p5.
- **Unity / Unreal** — מנועי משחק; עוצמה מקסימלית, מורכבות מקסימלית.

**המסקנה שלי בשבילנו:** הסטאק p5.js → (בהמשך three.js) + Hydra + שיידרים הוא הדרך הנכונה — כל העקרונות של TD ניתנים למימוש ב-WebGL, אנחנו נשארים בקוד (שזה הכוח שלנו — vibe coding), הכל חינמי, והכל רץ בדפדפן שאפשר להזרים ל-Resolume.

---

## 3. איך משחזרים את המראה של TD אצלנו (מתכונים טכניים)

### א. Feedback / Ping-Pong (הבסיס לרוב האפקטים)
שני `createFramebuffer({ format: FLOAT })`. כל פריים: מציירים את הקודם לתוך הנוכחי דרך שיידר שעושה זום/סיבוב/דעיכה קלים + מציירים את התוכן החדש מעל; מחליפים ביניהם. ר' [מדריך render targets של Maxime Heckel](https://blog.maximeheckel.com/posts/beautiful-and-mind-bending-effects-with-webgl-render-targets/).

### ב. GPGPU Particles (מיליוני חלקיקים)
מיקומי החלקיקים נשמרים כפיקסלים בטקסטורת float (חלקיק = פיקסל, RGB = XYZ). שיידר "סימולציה" קורא את הטקסטורה הקודמת, מוסיף מהירות/כוחות/noise, כותב לחדשה (ping-pong). שיידר "רינדור" דוגם את הטקסטורה פר-קודקוד וממקם נקודות. ר' [הפרק על GPU Particles](https://github.com/interactiveimmersivehq/Introduction-to-touchdesigner/blob/master/GLSL/12-7-GPU-Particle-Systems.md) — העיקרון זהה ב-WebGL.

### ג. Bloom / Glow
רינדור הסצנה ל-framebuffer → חילוץ אזורים בהירים (threshold) → טשטוש גאוסיאני דו-שלבי (אופקי+אנכי, ping-pong) → חיבור עם המקור ב-additive. זה האפקט שהכי "מוכר" מראה מקצועי. ר' [Three.js Journey – post-processing](https://threejs-journey.com/lessons/post-processing).

### ד. Instancing מונע-נתונים
ב-p5: `buildGeometry` + לולאת model() עם פרמטרים מ-FFT/noise (עד מאות עותקים), או שיידר vertex עם uniform-ים (אלפי עותקים). ב-three.js: `InstancedMesh` אמיתי (מאות אלפים).

### ה. Noise displacement
שיידר vertex שמזיז קודקודי sphere/plane לפי `noise(position + time)` לאורך הנורמל + תאורה. ב-p5 2.0 אפשר גם דרך p5.strands hooks בלי GLSL גולמי.

### ו. Audio-reactive (השלב הבא המתבקש שלנו)
`p5.sound`: `p5.AudioIn` (מיקרופון/כרטיס קול) → `p5.FFT` → `fft.getEnergy("bass"/"mid"/"treble")` → **החלקה עם lerp** (`smoothed = lerp(smoothed, raw, 0.1)`) → מיפוי לרדיוס/צבע/עוצמת bloom. בדיוק צינור ה-CHOPs של TD.

### ז. Slit-scan / Rutt-Etra
Slit-scan: מערך של N פריימים אחרונים (framebuffers), כל שורה נדגמת מפריים אחר. Rutt-Etra: וידאו/מצלמה כטקסטורה, בהירות → Z displacement של רשת קווים.

---

## 4. הצינור ל-Resolume (יש לך Arena!)

p5 רץ בדפדפן ואין ל-Resolume מקור דפדפן מובנה. הדרכים המוכחות:
1. **NDI Scan Converter** (חינם, NDI Tools) — לוכד חלון דפדפן ומשדר NDI; Arena קולט NDI ישירות. הכי פשוט.
2. **OBS** — לוכד את הדפדפן ומוציא Spout/NDI/מצלמה וירטואלית אל Arena.
3. **Spout** — עם כלי גשר ייעודיים. ר' [פורום Resolume](https://resolume.com/forum/viewtopic.php?t=14317), [פרויקטי Spout](https://leadedge.github.io/spout-projects.html).

זה אומר: הוויז'ואלס שאנחנו כותבים בקוד → שכבה חיה ב-Arena, עם אפקטים ומיקסים של Resolume מעל. שילוב מנצח.

---

## 5. מפת דרכים מוצעת להרחבת היכולות

שלבים, מהקל לכבד — כל שלב נבנה על הקודם:

1. **שדרוג השלד** — HSB, emissive+ADD, פעימת noise, buildGeometry. (יש לנו את כל הידע)
2. **Post-FX ראשון** — createFilterShader: bloom פשוט, chromatic aberration, vignette.
3. **Feedback trails** — ping-pong framebuffers עם FLOAT. שובלים לכדור הקוביות.
4. **Audio-reactive** — p5.sound FFT עם החלקה; מיפוי bass→פעימה, high→נצנוץ.
5. **Noise displacement** — ספירה/משטח "נושמים" עם שיידר vertex או p5.strands.
6. **GPGPU particles** — מערכת חלקיקים בטקסטורות.
7. **Hydra כשכבת מיקס** — לשלב את p5 בתוך Hydra (`s0.init({src: canvas})`) לאפקטי feedback/מודולציה מיידיים.
8. **הזרמה ל-Resolume** — NDI מהדפדפן ל-Arena למופע אמיתי.
9. **אם נגיע לתקרה** — מעבר ל-three.js (InstancedMesh, EffectComposer, מיליוני חלקיקים).

---

## מקורות עיקריים

- [השוואת סביבות RT video (GitHub rtv)](https://github.com/hrtlacek/rtv)
- [Hydra docs](https://hydra.ojack.xyz/docs/) · [cables.gl](https://cables.gl)
- [Derivative (TouchDesigner) tutorials](https://derivative.ca/tutorials) · [docs.derivative.ca](https://docs.derivative.ca/Operator)
- [Interactive & Immersive HQ](https://interactiveimmersive.io) — בלוג הלימוד הגדול של TD
- [AllTouchDesigner](https://alltd.org) — מאגר טוטוריאלים לפי אפקט
- [Introduction to TouchDesigner — GPU Particles](https://github.com/interactiveimmersivehq/Introduction-to-touchdesigner/blob/master/GLSL/12-7-GPU-Particle-Systems.md)
- [Maxime Heckel — render targets](https://blog.maximeheckel.com/posts/beautiful-and-mind-bending-effects-with-webgl-render-targets/)
- [Three.js Journey — post-processing](https://threejs-journey.com/lessons/post-processing) · [Codrops](https://tympanus.net/codrops/tag/postprocessing/)
- [Shadertoy](https://www.shadertoy.com) — מאגר שיידרים ענק
