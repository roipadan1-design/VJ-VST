# p5.js — בסיס ידע לפרויקט ה-VJ

מסמך זה מרכז את כל הטכניקות שנלמדו מהאתר הרשמי של p5.js (דוגמאות, טוטוריאלים, רפרנס) + מקורות נוספים. זה "המוח" של הפרויקט — נרחיב אותו ככל שנלמד עוד.

---

## 1. יסודות WEBGL

- `createCanvas(w, h, WEBGL)` — מצב תלת מימד. הראשית (0,0,0) במרכז המסך. X ימינה, Y למטה, Z אל הצופה.
- `angleMode(DEGREES)` — עבודה במעלות במקום רדיאנים.
- `debugMode()` — מציג רשת וצירים לצורך פיתוח.
- `orbitControl()` — שליטה במצלמה עם עכבר/מגע (קריאה בכל פריים ב-draw).

## 2. טרנספורמציות

- `translate(x, y, z)`, `rotateX/Y/Z(angle)`, `scale(x, y, z)`, `shearX/Y()`.
- טרנספורמציות מצטברות! `push()` / `pop()` מבודדים שינויי מצב (טרנספורמציה + סטייל).
- סדר חשוב: translate → rotate → scale (ברירת מחדל). לסיבוב סביב ציר חיצוני: rotate ואז translate (כמו בשלד שלנו — כך נבנה כדור הקוביות).
- סימטריה: `scale(-1, 1)` או לולאת rotate.

## 3. צורות תלת מימד (פרימיטיבים)

`plane()`, `box()`, `cylinder()`, `cone()`, `torus()`, `sphere()`, `ellipsoid()` — כולן עם פרמטרים של גודל ורזולוציה (detailX/detailY — כמה מקטעים, משפיע על ביצועים ומראה).
- `loadModel('file.obj')` — טעינת מודלים חיצוניים (OBJ/STL, למשל מ-Blender). פרמטר normalize מנרמל גודל. הצגה עם `model()`.

## 4. גאומטריה מותאמת אישית (Custom Geometry)

ארבע דרכים:
1. `loadModel()` — קבצים חיצוניים.
2. `beginShape()` / `vertex(x,y,z)` / `endShape()` — בנייה ידנית. `normal(x,y,z)` לפני vertex קובע כיוון תאורה. מצבים: TRIANGLES, TRIANGLE_STRIP, QUAD_STRIP ועוד.
3. `buildGeometry(callback)` — "מקליט" ציורים לגאומטריה יעילה שמצוירת עם `model()` — קריטי לביצועים כשחוזרים על אותה צורה הרבה פעמים! `geometry.normalize()` ממרכז ומנרמל. `freeGeometry()` משחרר זיכרון GPU.
4. מחלקת `p5.Geometry` — שליטה מלאה: `vertices[]`, `faces[]`, `computeNormals()`.

דוגמת הנחש (01_Custom_Geometry): buildGeometry עם צורה אקראית פרוצדורלית, ואז ריצוף model() ב-81 עותקים — מהיר מאוד.

## 5. תאורה (Lights)

- `ambientLight(color)` — אור אחיד מכל הכיוונים.
- `directionalLight(r,g,b, dx,dy,dz)` — אור מכוון (שמש).
- `pointLight(r,g,b, x,y,z)` — נורה, מקרין לכל הכיוונים.
- `spotLight(r,g,b, x,y,z, dx,dy,dz, angle, concentration)` — קונוס אור.
- `imageLight(img)` / `panorama(img)` — תאורה מתמונה 360.
- `lights()` — קיצור לתאורה סטנדרטית. `noLights()` — כיבוי. `lightFalloff()` — דעיכה.
- עד 5 אורות מכל סוג (חוץ מ-imageLight — אחד).

## 6. חומרים (Materials)

- `fill()` — צבע בסיס (diffuse).
- `ambientMaterial(color)` — מגיב לאור אמביינט בלבד.
- `specularMaterial(color)` + `shininess(n)` — הברקות (1 = מטושטש, גבוה = חד). `metalness()`.
- `emissiveMaterial(color)` — זוהר עצמי, לא תלוי בתאורה. מצוין ל-VJ (נאון!).
- `normalMaterial()` — צבעי RGB לפי כיוון פני השטח — דיבוג ואפקט פסיכדלי.
- `texture(img)` + `textureMode()`, `textureWrap()` — עטיפת תמונה/וידאו/framebuffer על צורה.
- ניתן לשלב ambient+specular+fill+stroke יחד.

## 7. מצלמה

- `camera(x,y,z, cx,cy,cz, ux,uy,uz)` — מיקום, נקודת מבט, וקטור up.
- `perspective(fov, aspect, near, far)` — פרספקטיבה (FOV משנה עיוות/דרמה).
- `ortho()` — הטלה איזומטרית (גדלים קבועים). `frustum()`.
- `createCamera()` / `setCamera()` — ריבוי מצלמות ומעבר ביניהן (חיתוכים בסגנון VJ!).

## 8. שיידרים (GLSL)

- `createShader(vertSrc, fragSrc)` / `loadShader('x.vert','x.frag')`, החלה עם `shader(s)`, איפוס עם `resetShader()`.
- Vertex shader — רץ פר-קודקוד, קובע `gl_Position` באמצעות `uProjectionMatrix * uModelViewMatrix * vec4(aPosition, 1.0)`.
- Fragment shader — רץ פר-פיקסל, קובע `gl_FragColor`. צבעים בטווח 0.0–1.0.
- העברת נתונים: **attributes** (פר-קודקוד: aPosition, aTexCoord, aNormal) → **varyings** (אינטרפולציה בין השלבים, למשל vTexCoord) ← **uniforms** (מהסקיצה: `s.setUniform('name', value)`).
- `createFilterShader(fragSrc)` — אפקט על כל הקנבס. `tex0` = טקסטורת הקנבס, דגימה: `texture2D(tex0, vTexCoord)`.
- p5.js 2.0: **p5.strands** — כתיבת שיידרים ב-JavaScript! `baseMaterialShader().modify()`, buildColorShader/buildFilterShader וכו', hooks כמו worldInputs / filterColor, uniforms אוטומטיים.

## 9. Framebuffers — שכבות ואפקטים (חשוב מאוד ל-VJ)

- `createFramebuffer()` — משטח ציור על ה-GPU. מהיר בהרבה מ-`createGraphics()` (אין העברת נתונים CPU↔GPU).
- `layer.begin()` ... `layer.end()` — ציור לתוך השכבה. הצגה: `image(layer, ...)` או `texture(layer)`.
- `layer.color` ו-`layer.depth` — טקסטורות צבע ועומק שאפשר להזין לשיידר.
- **Depth of Field** (דוגמת Framebuffer Blur): מציירים סצנה ל-framebuffer, ואז שיידר שמטשטש לפי מרחק מ-focal plane (דגימה ספירלית של 20 שכנים).
- **Feedback loops / ping-pong**: שני framebuffers שמזינים זה את זה — הפריים הקודם נכנס לפריים הבא. אפקטי שובל/אינסוף קלאסיים ל-VJ. להשתמש ב-`{ format: FLOAT }` נגד הצטברות שגיאות עיגול. `clearDepth()` לאיפוס עומק בפידבק תלת מימדי.

## 10. מתמטיקה ויצירתיות

- `noise(x, y, z)` — Perlin noise: תנועה אורגנית, שדות זרימה, טרנים.
- `map()`, `lerp()`, `lerpColor()`, `paletteLerp()`, `constrain()`, `dist()`, `mag()`.
- `sin()/cos()` — תנועה מחזורית, גלים, מסלולים מעגליים.
- `random()`, `randomGaussian()`, `randomSeed()` — אקראיות ניתנת לשחזור.
- **Mandelbrot** (מהדוגמה): איטרציה z = z² + c פר-פיקסל דרך `loadPixels()`/`pixels[]`/`updatePixels()`; צביעה לפי מהירות התבדרות עם `lerpColor` על שורש הערך המנורמל. פרמטר w = זום.
- מערכות חלקיקים (Smoke Particles, Flocking): מחלקות עם position/velocity/lifespan, כוחות.

## 11. צבע ומיזוג

- `colorMode(HSB)` — עבודה בגוון/רוויה/בהירות, מושלם לסיבובי צבע ב-VJ. גם LAB/OKLab ב-p5 2.0.
- `blendMode()` — ADD, MULTIPLY, SCREEN ועוד — הצטברות אור.
- `tint()`, `filter()`, `beginClip()/endClip()`.

## 12. ביצועים (קריטי ל-VJ בזמן אמת)

1. `buildGeometry()` לצורות חוזרות — לא לחשב מחדש כל פריים.
2. Framebuffer במקום Graphics.
3. שיידרים במקום מניפולציית pixels.
4. `pixelDensity(1)` על מסכי רזולוציה גבוהה.
5. `setAttributes({ antialias: false })` אם לא קריטי.
6. פחות אורות = מהיר יותר; strokes ב-WEBGL יקרים.
7. p5.Graphics סטטי → המרה ל-p5.Image עם `.get()`.
8. ניטור: `frameRate()` + פרופיילר בדפדפן.

## 13. אינטראקציה וקלט

- עכבר: `mouseX/Y`, `movedX/Y`, `mouseDragged()`, `mouseWheel()`, `mousePressed()`.
- מקלדת: `keyPressed()`, `keyIsDown()` — טריגרים ל-VJ.
- `touches` — מולטיטאץ'. `deviceMoved()` — חיישני טלפון.
- מדיה: `createVideo()`, `createCapture()` (מצלמת רשת כטקסטורה!), p5.sound לניתוח אודיו (FFT — ריאקטיביות למוזיקה, נלמד בהמשך).
- ייצוא: `saveCanvas()`, `saveFrames()`, `saveGif()`.

## 14. השלד שלנו — ניתוח

[sketch.js](sketch.js) — כדור קוביות: לולאה כפולה (zAngle 0–180, xAngle 0–360, צעדי 30°) → rotateZ+rotateX ואז translate(0,400,0) → כל קוביה ממוקמת על מעטפת כדור ברדיוס 400. `noFill()` + stroke סגול = מראה wireframe.

**כיווני שדרוג עתידיים:** פעימת רדיוס עם sin/noise, סיבוב אוטומטי, צבעי HSB מסתובבים, emissiveMaterial + blendMode(ADD), buildGeometry לביצועים, framebuffer feedback לשובלים, שיידר פוסט-אפקט, ריאקטיביות לאודיו, מיפוי למקלדת.

## 15. ניתוח רפרנס (תמונה/וידאו) ותרגום ל-TD — מתודולוגיה קבועה

זו לא רשימת טכניקות (זה בקבצי המחקר) — זו **שיטת העבודה עצמה**: מה לעשות בכל פעם שמגיע רפרנס חדש, כדי להבין בדיוק מה קורה בו ולתרגם את זה נכון ל-TD, בלי ניחוש ובלי פינג-פונג מיותר.

### א. פירוק ויזואלי שיטתי — לתאר במילים לפני שנוגעים בקוד
לכל רפרנס (תמונה או וידאו) לענות על כל השאלות האלה **במפורש**, לפני שמציעים תוכנית:
1. **פלטת צבע** — טווח גוונים, רוויה, ניגודיות. מונוכרום או צבעוני? חם/קר?
2. **טקסטורה/רעש** — גרעיניות (grain), פסים, פיקסליות, בנדינג, חלקות?
3. **מבנה** — שטוח (2D overlay) או תלת-ממד אמיתי (יש עומק/parallax)? חלקיקים (הרבה עותקים קטנים) או mesh אחד רציף? wireframe (קווי מתאר) או מלא (shaded/solid)?
4. **תנועה** — סטטי לגמרי, זורם חלק (scroll/drift), קופצני-גליצ'י (חיתוכים פתאומיים), מחזורי/מקצבי (synced לביט)?
5. **מצלמה** (אם יש תחושת תלת-ממד) — סטטית, נעה בקו, מקיפה (orbit), זום?
6. **(לוידאו בלבד) מבנה זמן** — אורך לולאה, כמות/קצב קאטים, סוג מעברים, האם ניכר סנכרון לביט?

### ב. זיהוי טכניקה — טבלת "טביעת אצבע" (symptom → technique)
להשוות את מה שתואר בסעיף א' מול הקטלוג הקיים (`learning/RESEARCH-GLITCH-AESTHETICS.md`, `learning/RESEARCH-BERLIN-VIDEOART.md`):

| מה רואים | טכניקה סבירה |
|---|---|
| פסים מתוחים לכיוון אחד, כאילו פיקסלים "נמרחו" לפי בהירות | Pixel Sorting |
| תזוזה נגררת/"נמסה" בין פריימים, כאילו הקומפרסיה נכשלה | Datamoshing |
| שוליים אדום-ציאן/סגול סביב קצוות | Chromatic Aberration / RGB Split |
| קווים אופקיים חוזרים + רעש גרעיני + "קפיצות" מקומיות | VHS/אנלוגי |
| תווים/מספרים נופלים בעמודות עם שובל דועך | Matrix Digital Rain (POPs+instancing+Feedback) |
| טקסט/סמלים ירוקים על שחור, גופן monospace, סורק CRT | Terminal/Hackercore |
| מבנה חוזר של אלמנטים זהים (הרבה עותקים קטנים) | GPU Instancing (בדיוק מה שהכדור-קוביות שלנו כבר עושה) |
| מונוכרום קשה, גיאומטריה חדה, בטון/מתכת, ניגודיות גבוהה | תעשייתי/ברגהיין-סטייל (כיוון-נגד לפלטה הנוכחית) |
| תמונה מטושטשת ברזולוציה נמוכה, חלל מוכר-אך-מוזר, נוסטלגי | Weirdcore/Dreamcore/Liminal (לא גליץ' טכני — יותר טון רגשי) |
| קובייה/מודל שמתעוות/נקרע פיזית (לא רק פיקסלים) | Vertex Displacement בשיידר (GLSL MAT, לא Displace TOP רגיל) |

### ג. מיפוי לרכיבי TD בפועל
אחרי שזיהינו טכניקה — למצוא אותה בטבלת ה-TD-nodes בקבצי המחקר לפני שמתחילים לבנות (לא לנחש שמות פרמטרים — לבדוק עם `get_td_node_parameters` בפועל בתוך TD, כמו שעשינו כל הסשן).

### ד. תהליך עבודה מומלץ כשמגיע רפרנס חדש
1. לתאר במילים (סעיף א') ולזהות טכניקה (סעיף ב') — **לפני** כל שורת קוד.
2. להציע תוכנית קטנה וממוקדת (מה משתנה, מה נשאר), ולחכות לאישור לפני שינוי משמעותי.
3. לבנות **גרסת-מינימום** של המנגנון (2-3 נקודות/פריים בודד/דוגמה קטנה), לא את כל ההיקף מיד.
4. לוודא בפועל עם `get_top_image`/`numpyArray` (פיקסלים אמיתיים מה-render) — לא להצהיר "זה עובד" על בסיס הנחה.
5. רק אחרי אימות — להרחיב להיקף המלא, ולשמור (`.toe`) עם אישור המשתמש.

### ה. מלכודות ידועות — checklist לבדוק לפני שמצהירים שמשהו עובד
(כל אחת מהן גרמה לבאג אמיתי בפרויקט הזה בפועל — לא תיאורטי)
- **Far clip של המצלמה** — גיאומטריה מעבר ל-`far` נחתכת לגמרי ובשקט, בלי שגיאה.
- **`numinstances`** על `geometryCOMP` לא עוקב אוטומטית אחרי גודל טבלת המקור — לקבוע במפורש.
- **SOP חדש שנוצר** — דגלי `render`/`display` כבויים כברירת מחדל (בניגוד ל-SOPs שכבר קיימים בפרויקט מתבנית ישנה).
- **כיוון compositeTOP ("over")** — יכול להיות הפוך ממה שמצפים; `swaporder` הוא הפתרון, לבדוק אמפירית לא להניח כיוון.
- **מתיחת תמונה בודדת על כדור/משטח גדול** → טשטוש חמור (לא בעיית רזולוציה/פילטר — בעיה מתמטית מובנית). הפתרון: tiling+mirror, לא "רזולוציה גבוהה יותר".
- **מודל גיאומטרי-אנליטי (חישוב ידני של פריסטום מצלמה וכו')** יכול לסטות משמעותית מההתנהגות האמיתית של TD (ראינו פער של פי 10+ בין חיזוי לתוצאה בפועל) — **תמיד להעדיף בדיקה אמפירית על פיקסלים אמיתיים** על פני מודל תיאורטי.
- **`executeDAT` עם `framestart=True` חי** יתערבב עם בדיקות-יחידה ידניות (קלט עכבר אמיתי ממשיך לזרום תוך כדי הבדיקה) — לכבות `framestart`, לבדוק, להחזיר.
- **TD חוזר למצב השמור האחרון בקריסה/ריסטארט** — לשמור אחרי כל אבן-דרך מאומתת (עם אישור המשתמש לשמירה), לא רק בסוף.

---

## מחקר מתקדם

מחקר עומק על TouchDesigner, אלטרנטיבות (Hydra, cables.gl, vvvv, Notch...), מתכוני האפקטים שלהם ואיך משחזרים אותם ב-WebGL — כולל מפת דרכים והצינור ל-Resolume: [learning/RESEARCH-VIDEO-ART.md](learning/RESEARCH-VIDEO-ART.md)

מחקר על שידור לResolume (NDI/Spout) וחיבור ל-Ableton Live 10 (MIDI Clock, ableton-js, VB-Cable+FFT) לאוטומציות ואודיו-ריאקטיביות, כולל שרת גשר עובד: [learning/RESEARCH-ABLETON-RESOLUME-BRIDGE.md](learning/RESEARCH-ABLETON-RESOLUME-BRIDGE.md) + [bridge/](bridge/)

מחקר עומק על אסתטיקות גליץ'/וויבקודינג/codecore לתוכן קצר (קטלוג טכניקות: datamoshing, pixel sorting, RGB split, VHS, ASCII/matrix rain, עיוות גיאומטריה תלת-ממדית — כולל איך כל אחת נבנית ב-TD ספציפית, והתאמה לפלטפורמת עריכה/סנכרון-ביט): [learning/RESEARCH-GLITCH-AESTHETICS.md](learning/RESEARCH-GLITCH-AESTHETICS.md)

מחקר עומק טכני על MetaHuman (Rig Logic, capture pipeline, ייצוא FBX/Alembic — כולל דרך ישירה ל-TD דרך FBX COMP בלי Unreal בזמן ריצה) ועל האקוסיסטם הקיים של Claude Code + TouchDesigner MCP (תקדים קהילתי אמיתי, לא ניסוי בודד): [learning/RESEARCH-METAHUMAN-AND-CLAUDE-TD.md](learning/RESEARCH-METAHUMAN-AND-CLAUDE-TD.md)

מחקר עומק על סצנת ה-VJ/וידאו-ארט של ברלין (ברגהיין, CTM Festival, transmediale, אסתטיקה פוסט-דיגיטלית) ואמנים ספציפיים שעובדים ב-TD (Stanislav Glazov, elekktronaut, Studio MXZEHN, Helin Ulas) — כולל כיוון-נגד תעשייתי-מונוכרומטי כאלטרנטיבה לפלטת הצבע הנוכחית: [learning/RESEARCH-BERLIN-VIDEOART.md](learning/RESEARCH-BERLIN-VIDEOART.md)

## מקורות נוספים שנאספו

- The Coding Train (דניאל שיפמן) — מאות טוטוריאלים: https://thecodingtrain.com
- fxhash — מדריך אמנות ג'נרטיבית: https://www.fxhash.xyz/article/beginner's-guide-to-learning-p5.js-for-generative-art
- GenerativeMedia.club — קורסים חינמיים: https://generativemedia.club/blog/free-tutorials-and-courses-for-p5-js-beginners/
- אמנות ג'נרטיבית עם Tweakpane (פאנל שליטה חי — שימושי מאוד ל-VJ): https://alexcodesart.com/building-an-interactive-generative-art-with-p5-js-tweakpane-and-watercolor-effects/
- קוד הדוגמאות הרשמי שהורדנו: [learning/examples/](learning/examples/)
