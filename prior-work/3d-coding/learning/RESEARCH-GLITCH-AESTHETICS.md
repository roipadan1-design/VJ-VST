# מחקר עומק — אסתטיקות גליץ' / וויבקודינג / codecore (מאגר ידע לתוכן קצר)

מסמך זה מרכז מחקר על הז'אנר החזותי שמעניין את הפרויקט: סרטונים קצרים (כמה שניות) שחותכים בין פריימים/טקסטורות/תלת-ממד/גליץ', מסונכרנים למוזיקה. המטרה: כשמביאים רפרנס, לזהות **איזו טכניקה ספציפית** עומדת מאחוריו ו**איך לבנות אותה ב-TD** — בלי ניחוש.

---

## 1. מפת הז'אנר — מאיפה codecore בא

**וויבקודינג (vibecoding)** כתופעת תוכן: סרטוני "פיתוח בהרגשה/וייב" — מסך קוד/AI-קידוד אמיתי, לרוב עם אסתטיקה חזותית נלווית שמדגישה את ה"טכנולוגיות". זה ז'אנר-אב שממנו נובעים כמה תת-תיוגים בפורמט "-core" (בדיוק כמו cottagecore/glitchcore — כל תת-קהילה מקבלת שם "-core" משלה):

- **codecore** — התת-ז'אנר הספציפי: חיתוך בין קטעי מסך-קוד/מצלמה אמיתיים לבין אפקט **digital rain בסגנון מטריקס** (עמודות תווים/מספרים נופלים עם שובל דועך).
- **terminalcore / hackercore** — אסתטיקת טרמינל: פוספור ירוק על שחור כמעט-שחור, גופן monospace, סורק CRT (scanlines), סמן מהבהב, ASCII box-drawing, "boot sequence" מדומה.
- **glitchcore** — המשפחה הרחבה יותר (לא ספציפי לקוד): כל אסתטיקת התקלה/שיבוש דיגיטלי — ראו קטלוג טכניקות בסעיף 2.
- **weirdcore / dreamcore / liminal space** — משפחה אחרת, קרובה אך לא זהה: פחות "תקלה טכנית", יותר תחושת אי-נוחות/דז'ה-וו — תמונות מטושטשות ברזולוציה נמוכה, גרפיקת אינטרנט מוקדמת, חללים ריקים/מוכרים-אך-מוזרים (מסדרונות, חדרי המתנה). **דומה חזותית לפעמים לגליץ'** (גם שם יש VHS/רעש/טקסט קריפטי) אבל המניע הרגשי שונה — worth knowing להבדיל בין "זה גליץ'" לבין "זה weirdcore" כשמנתחים רפרנס.

**המשותף לכולם**: חיתוך מהיר, מוזיקה עם ריגוש, פסים/רעש/גרעיניות, פלטת צבע מוגבלת (לרוב ירוק-שחור למטריקס/hackercore, או סגול-ורוד-כתום לגליץ' אנלוגי — בדיוק כמו תמונות הרפרנס שכבר יש בפרויקט).

---

## 2. קטלוג טכניקות גליץ' (מה קורה בפועל בכל אפקט)

### 2.1 Datamoshing (דאטה-מוש)
טכניקת שיבוש שמדמה כשלון דחיסת וידאו: משתמשים ב-**optical flow** (וקטורי תנועה בין פריימים עוקבים) כדי להזיז פיקסלים מהפריים הקודם לפי התנועה בפריים הנוכחי, ב-**Feedback Loop**. התוצאה: תנועה "נמסה"/נגררת בין פריימים, כאילו ה-keyframes נמחקו מקובץ וידאו דחוס.
- **ב-TD**: Feedback TOP שמזין את עצמו בחזרה + Optical Flow TOP לחישוב הווקטורים + Displace TOP שמזיז פיקסלים לפי אותם וקטורים. (זה בדיוק אותו family טכני כמו ה-Displace TOP שכבר השתמשנו בו בפרויקט — רק המקור לוקטור השונה: אצלנו זה היה Noise CHOP, בדאטה-מוש אמיתי זה Optical Flow אמיתי מהוידאו).

### 2.2 Pixel Sorting (מיון פיקסלים)
פותח ע"י Kim Asendorf. לוקחים מקטע פיקסלים (שורה/עמודה) ו**ממיינים אותם** לפי קריטריון — בהירות, גוון, ערוץ אדום/ירוק/כחול. יוצר "פסי המסה" מתוחים לכיוון אחד, עם threshold שקובע אילו פיקסלים בכלל נכנסים למיון (כדי לא למיין את כל התמונה, רק אזורים בהירים/כהים מסוימים).
- **ב-TD**: יש GLSL TOP ייעודי (חיפוש "pixel sort touchdesigner" — יש טוטוריאל מלא ב-Derivative/AllTouchDesigner). המנגנון: shader שממיין ערכים לאורך ציר X או Y בתוך buffer.

### 2.3 Chromatic Aberration / RGB Split (סטיית צבע כרומטית)
מפרידים את שלושת ערוצי הצבע (R/G/B) ומזיזים כל אחד במרחק/כיוון קצת שונה — יוצר את השוליים האדום-ציאן הקלאסיים. פשוט וזול חישובית.
- **ב-TD**: GLSL TOP עם UV offset שונה לכל ערוץ (`texture2D(tex, uv + offsetR).r`, וכו'), או פשוט 3 עותקי TOP מוזזים ב-Transform TOP ומחוברים ב-Add/Composite עם ערוץ בודד מכל אחד.

### 2.4 VHS / אנלוגי
משלב: **scanlines** (קווים אופקיים חוזרים), **tracking errors** (עיוות/קפיצה אופקית מקומית באזור מסוים), **color bleeding** (טשטוש כרומה — בדיוק כמו chroma subsampling של קלטת), רעש גרעיני (grain), לפעמים "generational loss" (כאילו הוקלט מקלטת שהועתקה כמה פעמים — פחות ניגודיות, יותר רעש בכל דור).
- **ב-TD**: שילוב של Noise TOP (לגרעיניות) + Ramp/Pattern TOP עבור scanlines (קווים חוזרים לאורך ציר Y, easy עם Ramp מחזורי או ביטוי sin על ה-Y) + Displace TOP מקומי לאזור ה-tracking error.

### 2.5 Databending
פתיחת קובץ תמונה כטקסט גולמי (hex editor) ועריכה ישירה של הבייטים — משבש את הפענוח ומייצר עיוותי צבע/מבנה אקראיים ולא-חזויים לגמרי (שונה מ-pixel sorting שהוא דטרמיניסטי/נשלט). פחות רלוונטי ל-real-time ב-TD (זה תהליך offline על קובץ), אבל שימושי כ**מקור טקסטורה** — אפשר לדטה-בנד תמונה מראש ולהכניס אותה כטקסטורה סטטית.

### 2.6 ASCII / Terminal Aesthetic
המרת תמונה/וידאו לתווי ASCII (כל בלוק פיקסלים הופך לתו לפי בהירות), מוצג בגופן monospace, לרוב ירוק-על-שחור עם phosphor glow (בלור/זוהר קל).
- **ב-TD**: יש טכניקת "ASCII TOP" נפוצה — Edge TOP או Luma-based lookup לבחירת תו, Text TOP/Instancing להצגת התווים בפועל (**ממש אותו מנגנון instancing שכבר בנינו לכדור-הקוביות!** — כל "פיקסל" הופך למופע אחד עם תו/צבע לפי הבהירות במקום).

### 2.7 Digital Rain / Matrix Code Rain
(פורט בהרחבה בתשובה קודמת) — עמודות תווים נופלים עם שובל דועך. ב-TD: POPs (חלקיקים) + Text מופעל + Feedback TOP לשובל. יש טוטוריאלים ייעודיים ב-AllTouchDesigner וביוטיוב תחת "Matrix Code Rain Tutorial for TouchDesigner" ו-"Neon Matrix Text Cascade Using Particle POPs".

### 2.8 עיוות גיאומטריה תלת-ממדית (glitch על מודל 3D, לא רק תמונה)
הזזת vertices של מש בזמן אמת (**vertex displacement** ב-vertex shader) — כל קודקוד זז לפי מפת רעש/טקסטורה, לא רק הפיקסלים על המסך. זה שונה מ-Displace TOP (שעובד על תמונה 2D) — כאן משנים את הגיאומטריה עצמה במרחב.
- **ב-TD**: GLSL MAT (custom vertex shader) שמזיז `P` (position) לפי `texture()` sample של noise/וידאו, במקום phongMAT הרגיל. זה מתקדם יותר ממה שבנינו עד עכשיו (השתמשנו רק ב-per-instance transform, לא בעיוות-קודקודים בתוך השיידר) — **כיוון מעניין להרחבה עתידית** לכדור-הקוביות אם ירצו "לקרוע"/לעוות את הקוביות עצמן, לא רק להזיז אותן.

---

## 3. פלטפורמות — מה מתאים לכל שלב בזרימת העבודה

| שלב | פלטפורמה מומלצת | למה |
|---|---|---|
| בניית האפקט הגנרטיבי/תלת-ממדי בזמן אמת (הכדור, הרקע, הגליץ' החי) | **TouchDesigner** | כבר בנוי, node-based, real-time, אפשר MCP לבנייה ואימות אוטומטי (כמו שעשינו כל הסשן) |
| עריכת הסרטון הסופי (חיתוך, קיצוב לביט, טקסט/כתוביות) | **CapCut** (הכי מהיר לפורמט קצר+רשתות) או **After Effects** (יותר שליטה, פלאגינים כמו Trapcode Particular) | CapCut מזהה ביטים אוטומטית ויש בו פרסטים מוכנים לגליץ'/VHS; AE נותן שליטה עדינה יותר לשכבות/keying |
| מקור לאפקט 2D סטטי מהיר (בלי real-time) | **Processing/p5.js** או שיידר ב-**Shadertoy** | הכי מהיר לפרוטוטייפ אלגוריתם חדש (pixel sort, ASCII) לפני שמעבירים ל-TD |
| דטה-בנדינג/עיבוד קבצים גולמי | **Hex editor** על קובץ סטטי (offline, לא real-time) | תהליך חד-פעמי ליצירת טקסטורת-מקור, לא לשימוש חי |

**מסקנה מעשית**: ה-**גנרציה החיה** (מה שרואים על המסך, כולל תגובתיות לעכבר/מוזיקה) נשארת ב-TD — זה מה שכבר עובד. **העריכה הסופית לפורמט קצר+מוזיקה** (חיתוכים, קיצוב לביט, ייצוא ל-9:16) קורית אחרי-כן בכלי עריכה (CapCut/AE), על ההקלטות/renders שיוצאים מ-TD.

---

## 3.5 פורטרטים גליצ'יים ב-Unreal Engine (רפרנס אינסטגרם — פנים שנגלצות)

טרנד ספציפי שהמשתמש נתקל בו: תמונת פנים של בן אדם שהופכת למודל תלת-ממדי וגליצ'ית בזמן אמת. **אמן מזוהה (אושר ע"י המשתמש)**: חשבון אינסטגרם `cloudeiobellini` — כנראה **Claudio Bellini**, אמן מולטי-דיסציפלינרי בתחום 3D/גנרטיבי (רקע במוזיקה אלקטרונית + מדיה חדשה, הוצג ב-Mutek San Francisco, Athens Digital Arts Festival, Super Nova Denver) — **worth מעקב ישיר אחרי החשבון לרפרנסים נוספים**. המשתמש אישר גם שהשם הרשמי של השיטה הוא **MetaHuman** (מאשר את הניתוח למטה — פייפליין ה-photogrammetry-ל-MetaHuman הוא אכן הבסיס הנכון, לא ניחוש).

פירוק הטכניקה שמאחורי זה (לא אחד אלא **שילוב** של כמה שכבות):

**שלב 1 — הפנים הופכות לגיאומטריה תלת-ממדית** (לא רק תמונה שטוחה):
- הדרך המקצועית: **photogrammetry/AI מכמה תמונות** — כלים כמו KeenTools FaceBuilder (לרוב כ-40 תמונות מזוויות שונות) בונים מש תלת-ממדי אמיתי של הפנים, ואז מכניסים אותו כ-MetaHuman או Mesh-to-MetaHuman ב-Unreal.
- יש גם כלים חדשים יותר ל**תמונה בודדת** (AI מנחש עומק/צורה מתמונה 2D אחת — פחות מדויק, אבל מספיק לאפקט חזותי).
- **המשמעות**: זו לא "פילטר על תמונה" — יש גיאומטריית פנים אמיתית ב-3D, ואפשר לזוז סביבה/לסובב מצלמה — בדיוק כמו שאנחנו כבר עושים עם הכדור-קוביות.

**שלב 2 — הגליץ' עצמו: שתי שכבות טכניות נפרדות, לרוב משולבות יחד**
1. **Datamosh בזמן אמת (ברמת הפיקסלים/הטקסטורה)** — ב-Unreal יש plugin ייעודי ומוכר בשם **UEDatamosh**: הוא **מנצל את ה-motion vectors שה-engine כבר מחשב באופן טבעי** (ל-TAA/motion blur מובנים) — מזיז את הפריים הקודם לפי אותם וקטורים ומערבב עם הפריים הנוכחי, בזמן אמת, בלי צורך בחישוב optical-flow נפרד. זה שונה/יעיל יותר ממה שתיארנו קודם ל-TD (שם היינו צריכים Optical Flow TOP נפרד) — **שווה לבדוק אם ל-TD יש buffer של velocity/motion vectors חשוף כ-TOP** שאפשר לנצל באותו אופן (לא מאומת עדיין — לבדוק בפועל, לא להניח).
2. **עיוות גיאומטריה אמיתי (World Displacement על המש)** — חבילות שיידרים כמו "Art Of Shader – Distortion And Glitches" (40 שיידרים) עובדות גם כ-**Post Process**, גם כ-**Niagara FX**, וגם כ-**Mesh Material with World Displacement** — כלומר אותו glitch shader יכול לרוץ הן כפילטר-מסך והן כעיוות אמיתי של קודקודי המש (הפנים ממש "נקרעות"/זזות במרחב, לא רק הצבע שלהן משתבש). **זו בדיוק הטכניקה שסומנה במסמך הזה כ"כיוון להרחבה עתידית, לא נבנה עדיין"** (GLSL MAT עם vertex displacement) — הרפרנס הזה הוא דוגמה קונקרטית למה זה נראה כשעושים את זה נכון.

**תרגום ל-TD (בלי Unreal/MetaHuman בכלל — אפשר לקרב את האפקט ישירות ב-TD)**:
- **מקור גיאומטריה זול**: במקום photogrammetry מלא, אפשר להשתמש בתמונת-פנים בודדת כ-**height/displacement map** על `gridSOP`/`sphereSOP` פשוט (Displace SOP או vertex shader שמזיז את ה-Z לפי בהירות התמונה) — נותן "תבליט" תלת-ממדי גס של הפנים, מספיק לאפקט חזותי בלי צורך בכלי photogrammetry חיצוניים.
- **שכבת datamosh**: בדיוק כמו בסעיף 2.1 — Feedback TOP + Optical Flow TOP + Displace TOP, מופעל על הרינדור של הפנים (לא על וידאו חיצוני).
- **שכבת עיוות-מש אמיתי**: `phongMAT`/`constantMAT` מוחלף ב-**GLSL MAT מותאם אישית**, ששם `P` (מיקום קודקוד) מוזז לפי דגימת noise/טקסטורה בתוך ה-vertex shader — בדיוק אותו רעיון שסומן בסעיף 2.8 של המסמך הזה, עכשיו עם מוטיבציה קונקרטית וברורה.
- **סדר עבודה מומלץ** (לפי המתודולוגיה ב-`KNOWLEDGE.md` סעיף 15): קודם לבנות גרסת-מינימום — פנים סטטיות עם displacement קל בלבד, לוודא שרואים תבליט תלת-ממדי אמיתי (עם `get_top_image`, לא הנחה) — ורק אז להוסיף את שכבת ה-datamosh/הגליץ' מעליה.

---

## 4. סנכרון לביט — טכניקה מעשית

- העיקרון הבסיסי: כל חיתוך/החלפת-אפקט קורה **בדיוק על הביט**.
- כלים לזיהוי ביט אוטומטי: CapCut מזהה ביטים ומסמן אותם על ה-timeline; יש גם VSDC "Edit the Beat" (אוטומטי לגמרי, מבוסס ניתוח waveform לתדר/עוצמה).
- כללי אצבע לפי קצב: ביטים מהירים → "Flash"/"Camera Shake"/חיתוך גליץ' חד; שירים איטיים → Cross-fade/Blur חלק יותר.
- **לפרויקט שלנו ספציפית**: כבר יש גשר Ableton→WebSocket (ב-`bridge/`) שמזהה ביט/BPM בזמן אמת — זה בדיוק התשתית לגרום לאפקט הגליץ'/החיתוך ב-**TD עצמו** להגיב לביט חי, לא רק בעריכה אחרי-כן. שווה לחבר את זה מחדש כשעוברים לשלב האודיו-ריאקטיביות (כבר תוכנן כ"כיוון להמשך" קודם בשיחה).

---

## 5. חיבור ישיר למה שכבר בנוי בפרויקט

- הרקע (תמונות 1/11, 5/11, 9/11) הוא בדיוק אסתטיקת **VHS/גליץ' אנלוגי** מסעיף 2.4 — פסים, גרעיניות, seam שחור. אפשר עכשיו לתייג במדויק אילו טכניקות ליצור עוד כאלה (או להזיז/לשנות בזמן אמת): Noise TOP לגרעיניות + Ramp מחזורי ל-scanlines + GLSL RGB-split קל.
- הכדור-קוביות ה-wireframe כבר מזכיר מבנה **ASCII/instancing** (סעיף 2.6) — התשתית להמיר אותו ל"ענן תווים/אותיות" (כמו בבריף ה-CONTROL) היא **אותה משפחה טכנית** בדיוק כמו טכניקת ה-ASCII/matrix rain שמצאנו.
- ה-Displace TOP שכבר נוסה (ונדחה כי היה "נמס מדי") הוא בדיוק ליבת ה-datamoshing (סעיף 2.1) — הלקח: להשתמש בו **במינון עדין הרבה יותר**, ואולי עם optical flow אמיתי (מוידאו, לא רעש) במקום Noise CHOP גנרי, כדי שהתנועה תרגיש "כמו כשל דחיסה אמיתי" ולא "המסה מופשטת".

---

## 6. מגמות אנימציה/וידאו רחבות יותר (מעבר לגליץ' ספציפית) — 2026

הרחבה מעבר לגליץ' — סגנונות פופולריים כלליים בעולם הוידאו/אנימציה שרלוונטיים לסרטונים קצרים:

- **מטל שיידרים (Metal Shaders)** — גימור מתכתי שכופף אור בעדינות, משקף סביבות מופשטות, משנה גוון עם תזוזת מצלמה — תחושה עתידית/פרימיום.
- **מעברים חלקים מבוססי-אובייקט** — למשל עיגול שמתרחב וממלא את המסך והופך לרקע הסצנה הבאה (טכניקת מעבר "seamless" פופולרית).
- **מיזוג 2D/3D** — שילוב מרחב תלת-ממדי עם קווי-מתאר דו-ממדיים, טקסטורות ציוריות, צורות גרפיות ואפקט "מצויר ביד" באותה סצנה.
- **תלת-ממד ציורי (Painterly 3D)** — טקסטורות דמויות-מברשת, תאורה מסוגננת, על גבי סצנות 3D מורכבות — נותן תחושה חמה/עשויה-ביד לעומת 3D "נקי" מדי.
- **אורגני/עשוי-ביד בכוונה** — מגמה מוצהרת: "לא-מושלם בכוונה" (imperfect on purpose) — **מתכתב ישירות עם האסתטיקה הפוסט-דיגיטלית** שכבר מתועדת ב-`RESEARCH-BERLIN-VIDEOART.md`.
- **מיקס-מדיה (Mixed Media)** — שילוב 2D+3D+פוטג' אמיתי (live action) יחד. **"Grunge Street Openers"** — פתיחים שמשתמשים בתמונות, טקסטורות נייר, אבק, שריטות, light-leaks — **ממש אותה משפחה חזותית כמו תמונות הרפרנס שלנו** (VHS/גרעיניות/פסים).
- **טיפוגרפיה קינטית (Kinetic Typography)** — טקסט שזז/מונפש כאלמנט מרכזי, לא רק כתובית — רלוונטי ישירות לבריף "CONTROL" שכבר כתבנו.
- **אסתטיקת "זכוכית נוזלית" (Liquid Glass)** — שהתחילה עם Apple: תנועות זרימה חלקות + השתקפות/שבירה של זכוכית, טקסטורות אורגניות נוספות.
- **AI כמאיץ workflow, לא תחליף** — הקונצנזוס ל-2026: AI מזרז רינדור/איטרציה, לא "יוצר לבד" — תואם בדיוק את אופן העבודה שלנו (כיוון אמנותי ממך, ביצוע+אימות ממני).

---

## מקורות

- [Corecore — Wikipedia](https://en.wikipedia.org/wiki/Corecore)
- [Core Trend Dictionary 2026 — TikTok Micro-Trends](https://www.acloset.app/magazine/core-trend-dictionary-2026-decoding-tiktok-micro-trends-wit/)
- [Matrix Code Rain Tutorial for TouchDesigner — AllTouchDesigner](https://alltd.org/matrix-code-rain-tutorial-for-touchdesigner/)
- [TouchDesigner: Neon Matrix Text Cascade Digital Rain Using Particle POPs](https://www.youtube.com/watch?v=YHHDyUIEpD8)
- [Glitchology — What is glitchcore?](https://glitchology.com/frequently-asked-questions/what-is-glitchcore/)
- [Glitchology — Data Moshing glossary](https://glitchology.com/glossary/data-moshing/)
- [Glitchology — Pixel Sorting: How It Works](https://glitchology.com/pixel-sorting/)
- [Glitch effect (RGB shift) with UV offset — GLSL TOP in TouchDesigner](https://alltd.org/glitch-effect-rgb-shift-with-uv-offset-glsl-top-in-touchdesigner/)
- [Datamoshing in TouchDesigner — Interactive & Immersive HQ (Part 1–3)](https://interactiveimmersive.io/blog/touchdesigner-resources/datamoshing-in-touchdesigner/)
- [Glitches, pixel sorting and data moshing — TouchDesigner (Derivative community tutorial)](https://derivative.ca/community-post/tutorial/glitches-pixel-sorting-and-data-moshing/64888)
- [Weirdcore aesthetic — Wikipedia](https://en.wikipedia.org/wiki/Weirdcore_aesthetic)
- [Liminal Space, Dreamcore & Weirdcore Explained — Vapor95](https://vapor95.com/blogs/darknet/liminal-space-dreamcore-and-weirdcore)
- [Terminal Design System — UI Design Prompts](https://uidesignprompts.com/prompts/terminal-design)
- [Beat-Sync Video Editing: Complete Guide — Beat2Cut](https://beat2cut.com/blog/beat-sync-video-editing-complete-guide/)
- [VSDC — Edit the Beat automatic sync](https://www.videosoftdev.com/vsdc-releases-edit-the-beat-tool)
- [CapCut — Free Glitch Transition Templates](https://www.capcut.com/explore/free-glitch-transition-effect)
- [Breakdown Of Real-Time Datamoshing In Unreal Engine 5 — 80.lv](https://80.lv/articles/breakdown-of-real-time-datamoshing-in-unreal-engine-5)
- [UEDatamosh plugin devlog — petyu.itch.io](https://petyu.itch.io/uedatamosh/devlog/779738/a-few-words)
- [Datamosh Effect — Fab (Unreal Marketplace)](https://www.fab.com/listings/f54fa19f-b19a-4330-88a6-a467f6a00bca)
- [Art Of Shader — Distortion And Glitches (40 shaders, Post Process/Niagara/Mesh World Displacement) — Fab](https://www.fab.com/listings/0223faf6-76e1-4049-ae3a-82ea1daa296f)
- [Creating MetaHumans from photos with FaceBuilder — Epic Developer Community Forums](https://forums.unrealengine.com/t/creating-metahumans-from-photos-with-facebuilder/621603)
- [Photo To MetaHuman: Easy Guide To 3D Character Creation](https://yelzkizi.org/metahuman-from-a-photo/)
- [The top 5 animation trends that will dominate 2026 — Lummi](https://www.lummi.ai/blog/animation-trends-2026)
- [From vivid gradients to liquid glass: Key animation trends for 2026 — Senate Media](https://www.senatemedia.co.uk/2026/02/from-vivid-gradients-to-liquid-glass-key-animation-trends-for-2026/)
- [16 Animation Trends to Watch in 2026 — GarageFarm](https://garagefarm.net/blog/animation-trends-to-watch)
