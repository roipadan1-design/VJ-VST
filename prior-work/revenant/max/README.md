# REVENANT — Max for Live device

ראש סרוק שמתפרק לפי הסאונד של הערוץ שעליו המכשיר יושב. ויזואל WebGL שרץ בחלון
נפרד (jweb) שנפתח אוטומטית — בלי דפדפן חיצוני, הכל לוקאלי.

```
[Ableton track audio] → [Revenant.amxd]
     │  plugin~ → 8-band analysis → node.script(bridge.js) → WebSocket :8765
     │                                                            │
     │  visual window (jweb) נפתח אוטומטית ~2.5s אחרי הוספה  ←──────┘
     ▼
  גרור את החלון למסך ה-HDMI · F = מסך מלא
```

המכשיר הוא **Audio Effect** — הוא מקבל את הסאונד של הערוץ שעליו הוא יושב, וזה
הטריגר. זיהוי הטרנזיינטים (kick/hat, סף אדפטיבי) קורה בתוך הויזואל עצמו.

---

## הרצה ראשונה

1. ודא ש-`dist/` קיים (הויזואל הבנוי). אם לא — מהשורש של הפרויקט:
   ```bash
   npm install
   npm run build        # יוצר revenant/dist
   ```
   (ה-bridge מחפש קודם `max/dist`, ואז `../dist`. כדי לבנות מכשיר נייד —
   העתק את `dist/` לתוך `max/dist/`.)

2. ודא ש-`max/node_modules/ws` קיים (כבר מצורף). אם לא:
   ```bash
   cd max && npm install
   ```

3. ב-Ableton: גרור את **`Revenant.amxd`** לערוץ שמנגן אודיו (לא Master בהכרח —
   כל ערוץ עם סאונד). דרוש Ableton **Suite** או רישיון Max for Live, ו-Node
   מוגדר ב-Max (האובייקט `node.script` עולה ירוק).

4. אחרי ~2.5 שניות חלון הויזואל נפתח אוטומטית. גרור אותו למסך ה-HDMI ולחץ **F**
   למסך מלא. נגן מוזיקה → הראש מגיב לקצב.

> אם החלון נפתח שחור: ה-jweb עלה לפני שהשרת. סגור ולחץ **OPEN** במכשיר, או חכה 2–3
> שניות. ודא ש-`node.script` ירוק.

---

## נובי מאקרו (Presentation — MIDI-mappable, נשמרים עם הסט)

| Knob | טווח | ברירת מחדל | משפיע על |
|---|---|---|---|
| Reactivity | 0–2 | 1.0 | עוצמת התגובה הכוללת (kick→disint, hat→glitch, beat→blue) |
| Disintegrate | 0–2 | 0.9 | עוצמת ההתפרקות |
| Glitch | 0–2 | 0.7 | עוצמת ה-datamosh |
| Blue Flash | 0–2 | 0.2 | עוצמת הבזק הכחול |
| Bloom | 0–3 | 0.95 | זוהר |
| Rotation | 0–1.2 | 0.22 | מהירות הסיבוב |
| Grain | 0–1 | 1.0 | רעש פילם |
| Transient | 0.2–3 | 1.0 | רגישות זיהוי הטרנזיינטים |

right-click על נוב → **MIDI Map** (או דרך כפתור MIDI של Ableton).

### מקשים בחלון הויזואל
`F` מסך מלא · `H` הסתר/הצג HUD · `G` הסתר/הצג פאנל בקרה · דאבל-קליק = מסך מלא

---

## חוזה הנתונים (WebSocket `ws://localhost:8765`)

```jsonc
{ "type":"audio", "bass":0..1,"mid":0..1,"high":0..1,"energy":0..1,
  "spec":[96], "spike":[96] }                 // ~60Hz מה-8 band
{ "type":"ctrl", "disint":0.9, "glitch":0.7, ... }   // מהנובים
```

הויזואל מחשב טרנזיינטים (kick=תדרים נמוכים, hat=גבוהים) מ-`spec` עם סף אדפטיבי,
אז התגובה לקצב חזקה ועקבית בלי לכוונן ספים ידנית.

## בדיקה בלי Ableton
```bash
cd max
node bridge.js --test         # דפוס kick סינתטי 110 BPM
```
ואז `http://localhost:8765/` בדפדפן — הראש מגיב.

## בנייה מחדש של המכשיר
שינית פרמטרים/נובים? הרץ:
```bash
cd max && node build_amxd.js   # מייצר Revenant.maxpat + Revenant.amxd
```
