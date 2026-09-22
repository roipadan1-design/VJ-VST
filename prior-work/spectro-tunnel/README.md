# Spectro Tunnel

מנוע ויזואלי אודיו-ריאקטיבי (WebGL/Three.js) שרץ **בתוך מכשיר Max for Live עצמאי**
על ערוץ ה-Master ב-Ableton Live 10. המכשיר פותח חלון ויזואל משלו (`jweb`) — בלי דפדפן
חיצוני, בלי אינטרנט. הכל לוקאלי.

```
[Ableton Master] -> [SpectroTunnel.amxd]
       |  אנליזה (8 band) -> node.script(bridge.js) -> WebSocket :8765 (localhost)
       |                                                      |
       |  כפתור OPEN VISUAL -> חלון [jweb] בתוך המכשיר  <------+
       v
   גורר את חלון הוויזואל למסך ה-HDMI
```

> `localhost` = המחשב שלך בלבד. עובד גם בלי חיבור לאינטרנט. אין Chrome נפרד —
> ה-`jweb` הוא חלון מוטמע של המכשיר.

---

## מבנה

```
spectro-tunnel/
  engine/            # המנוע — נפתח ב-Chrome
    index.html
    engine.js
    three.min.js
  max/
    bridge.js        # Node for Max: שרת WebSocket + קבצים סטטיים
    build_maxpat.js  # מחולל את ה-.maxpat (להרצה חוזרת אם משנים פרמטרים)
    SpectroTunnel.maxpat   # הפאצ' המוכן -> ממנו אתה אופה .amxd
    node_modules/    # תלות 'ws'
```

---

## הרכבת המכשיר ב-Ableton (פעם אחת)

המרת ה-`.maxpat` ל-`.amxd` היא הצעד היחיד שחייב להיעשות בתוך Max — Live אופה את ה-`.amxd`.

1. ב-Ableton: גרור **Max Audio Effect** (ריק) לערוץ ה-**Master**.
2. לחץ על אייקון העיפרון/Edit במכשיר — נפתח **Max**.
3. ב-Max: **File → Open…** ובחר `max/SpectroTunnel.maxpat`.
4. **File → Save As…** ← שמור בשם **`SpectroTunnel.amxd`** לתוך התיקייה `max/`
   (חשוב: ליד `bridge.js`, כדי ש-node.script ימצא אותו ואת `node_modules`).
5. סגור את חלון ה-Max. המכשיר עכשיו חי על ה-Master ושמור עם ה-Live Set.

> בפעמים הבאות פשוט גורר את `SpectroTunnel.amxd` מהדפדפן/Finder ישירות לערוץ.

### דרישות
- Ableton Live **Suite** או רישיון Max for Live פעיל.
- **Node** מותקן ומוגדר ב-Max (Options → … או שהאובייקט `node.script` עולה ירוק).
  אם `node.script` לא עולה — זו תקלת Node-for-Max, לא הפאצ'. דווח לי.

---

## הרצה

1. הרכב/הנח את `SpectroTunnel.amxd` על ה-Master (ראה למעלה).
   האובייקט `node.script` רץ עם `@autostart 1` → השרת עולה אוטומטית על `:8765`.
2. במכשיר, לחץ **OPEN VISUAL** → נפתח חלון הוויזואל (`jweb`).
3. גרור את החלון למסך ה-HDMI ומקסם אותו.
4. נגן מוזיקה ב-Ableton → המנהרה מגיבה.

### מקשים בתוך חלון הוויזואל
- `F` — מסך מלא · `H` — הסתר/הצג UI · `רווח` — Pause · `G` — Glitch toggle

> אם החלון נפתח ריק/שחור: ייתכן שה-`jweb` נטען לפני שהשרת עלה. סגור את החלון
> ולחץ OPEN VISUAL שוב, או חכה 2–3 שניות אחרי הוספת המכשיר ואז פתח.

### אינדיקטור חיבור (פינה שמאלית תחתונה)
- **WS** — מחובר למכשיר (מצב עבודה) · **MIC** — מיקרופון מקומי · **DEMO** — אין חיבור

---

## פרמטרים (10 live.dial במכשיר, MIDI-mappable, נשמרים עם הסט)

| Dial | טווח | ברירת מחדל | משפיע על |
|---|---|---|---|
| Sensitivity | 0–3 | 1.30 | gain האנליזה |
| Relief | 0.2–2.5 | 1.10 | גובה הטופוגרפיה |
| Agitation | 0–1.5 | 0.70 | תגובתיות מקומית בקצה |
| Beams | 0–1.5 | 0.85 | קרני אור מטרנזיינטים |
| Chaos | 0–1 | 0.20 | שבירה אורגנית |
| Melt | 0–1 | 0.50 | החלקה זמנית |
| Color Shift | 0–360 | 0 | סיבוב פלטה |
| Glitch | 0–1 | 0.12 | RGB split + רעש |
| Tunnel Gap | 9–40 | 18 | רוחב המנהרה |
| Scroll Speed | 0.1–1.5 | 0.70 | מהירות גלישה |

---

## בדיקה ללא Ableton (Standalone)

לבדוק את המנוע + שרת בלי Max:

```bash
cd max
node bridge.js --test      # תבנית סינוס סינתטית 60Hz
```

ואז `http://localhost:8765/` ב-Chrome — המנהרה זזה לפי הסינוס. בלי `--test` השרת
עולה במצב idle (מגיש קבצים, מחכה לחיבור) והמנוע יראה DEMO/יציב.

---

## טכני: חוזה הנתונים

WebSocket על `ws://localhost:8765`, JSON:

```jsonc
{ "type":"audio", "bass":0..1,"mid":0..1,"high":0..1,"energy":0..1,
  "spec":[96], "spike":[96] }            // ~60Hz
{ "type":"ctrl", "relief":1.1, "hue":0, ... }   // בשינוי dial
```

המכשיר שולח 8 בנדים גולמיים (`spec8`); `bridge.js` מחיל Sensitivity, מרחיב ל-96,
מחשב bass/mid/high ו-spike (spectral flux), ומשדר. לשנות מספר בנדים/תדרים:
ערוך את `CF`/`Q` ב-`build_maxpat.js` + `N_BANDS` ב-`bridge.js`, הרץ `node build_maxpat.js`,
ושמור מחדש ל-`.amxd`.
