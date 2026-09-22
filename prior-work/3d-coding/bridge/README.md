# VJ ↔ Ableton Bridge

מגשר בין Ableton Live 10 לסקיצת ה-p5.js, דרך WebSocket. רקע מלא: [../learning/RESEARCH-ABLETON-RESOLUME-BRIDGE.md](../learning/RESEARCH-ABLETON-RESOLUME-BRIDGE.md).

## הקמה חד-פעמית

```bash
cd bridge
npm install
```

העתיקו את `node_modules/ableton-js/midi-script` אל:
`%USERPROFILE%\Documents\Ableton\User Library\Remote Scripts\AbletonJS`

ב-Ableton Live: Preferences → Link/Tempo/MIDI → Control Surface → בחרו `AbletonJS` → הפעילו מחדש את Ableton.

## הרצה

```bash
npm start
```

ישמע ב-`ws://localhost:8081`. כל שינוי טמפו/פליי/מטר משודר אוטומטית כ-JSON.

## חיבור מהדפדפן (sketch.js)

```js
const socket = new WebSocket("ws://localhost:8081");
socket.onmessage = (event) => {
  const msg = JSON.parse(event.data);
  if (msg.type === "tempo") { /* ... */ }
  if (msg.type === "beat") { /* msg.bar, msg.beatInBar */ }
  if (msg.type === "meter") { /* msg.trackIndex, msg.value */ }
};
```
