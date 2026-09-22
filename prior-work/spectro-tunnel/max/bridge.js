"use strict";
/*
 * Spectro Tunnel — bridge.js  (v2)
 *
 * Serves engine/ as static HTTP and runs a WebSocket server on the same port.
 * - Inside Max for Live (node.script): exposes handlers Max can send to
 *   ('audio', 'ctrl', 'send', 'transport', 'fftdata'). Payload is broadcast to WS clients.
 * - Standalone (`node bridge.js`): emits a synthetic 60 Hz sine/spike pattern
 *   so the browser side can be tested without Ableton.
 *
 * v2 additions:
 *   - transport handler: receives beat/bar/bpm from Live's [transport] object
 *   - fftdata handler: transient detection via spectral flux with cooldown
 *   - mode handler: TRANSPORT(0) / TRANSIENT(1) / HYBRID(2)
 *   - punch_thresh / cut_thresh exposed as ctrl params
 */

const http = require("http");
const fs = require("fs");
const path = require("path");
const WebSocket = require("ws");

let maxApi = null;
try { maxApi = require("max-api"); } catch (_) { /* standalone */ }

const PORT = 8765;
const ENGINE_DIR = path.resolve(__dirname, "..", "engine");

function log(msg) {
  if (maxApi) { try { maxApi.post(String(msg)); } catch (_) {} }
  else console.log(msg);
}
function logErr(msg) {
  if (maxApi) { try { maxApi.post(String(msg), maxApi.POST_LEVELS.ERROR); } catch (_) { try { maxApi.post(String(msg)); } catch (_) {} } }
  else console.error(msg);
}

/* ---------- static HTTP ---------- */
const MIME = {
  ".html": "text/html; charset=utf-8",
  ".js":   "application/javascript; charset=utf-8",
  ".css":  "text/css; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".png":  "image/png",
  ".jpg":  "image/jpeg",
  ".jpeg": "image/jpeg",
  ".svg":  "image/svg+xml",
  ".ico":  "image/x-icon",
  ".map":  "application/json"
};

const server = http.createServer((req, res) => {
  try {
    let urlPath = decodeURIComponent((req.url || "/").split("?")[0]);
    if (urlPath === "/" || urlPath === "") urlPath = "/index.html";
    const filePath = path.resolve(path.join(ENGINE_DIR, urlPath));
    if (!filePath.startsWith(ENGINE_DIR)) {
      res.statusCode = 403; res.end("forbidden"); return;
    }
    fs.readFile(filePath, (err, data) => {
      if (err) { res.statusCode = 404; res.end("not found"); return; }
      const ext = path.extname(filePath).toLowerCase();
      res.setHeader("Content-Type", MIME[ext] || "application/octet-stream");
      res.setHeader("Cache-Control", "no-cache");
      res.end(data);
    });
  } catch (e) {
    res.statusCode = 500; res.end("server error");
    logErr("http error: " + (e && e.message));
  }
});

/* ---------- WebSocket ---------- */
const wss = new WebSocket.Server({ server });
const clients = new Set();

wss.on("connection", (ws, req) => {
  clients.add(ws);
  log("ws client connected (" + clients.size + " total)");
  ws.on("close", () => { clients.delete(ws); log("ws client disconnected (" + clients.size + " total)"); });
  ws.on("error", () => { clients.delete(ws); });
});

function broadcast(obj) {
  let data;
  try { data = JSON.stringify(obj); } catch (e) { return; }
  for (const c of clients) {
    if (c.readyState === WebSocket.OPEN) {
      try { c.send(data); } catch (_) {}
    }
  }
}

server.on("error", (e) => {
  if (e && e.code === "EADDRINUSE") {
    log("port " + PORT + " in use — retrying in 2s");
    setTimeout(() => { try { server.close(); } catch (_) {} server.listen(PORT); }, 2000);
  } else {
    logErr("http server error: " + (e && e.message));
  }
});

server.listen(PORT, () => {
  log("Spectro Tunnel bridge v2 ready");
  log("  http://localhost:" + PORT + "/");
  log("  ws://localhost:" + PORT + "/");
  log("  engine dir: " + ENGINE_DIR);
  if (maxApi) log("  mode: Max for Live");
  else if (wantsTestPattern) log("  mode: standalone --test");
  else log("  mode: standalone (idle — pass --test for sine pattern)");
});

/* ---------- analysis helpers ---------- */
const N_BANDS = 8;
const SPEC_N  = 96;
const RAW_GAIN = 9.0;
const CURVE    = 0.6;

const ctrlState = { sens: 1.3 };
const avgBands  = new Array(N_BANDS).fill(0);

function clamp01(x) { return x < 0 ? 0 : x > 1 ? 1 : x; }
function shape(v)   { return clamp01(Math.pow(clamp01(v), CURVE)); }

/* ---------- trigger mode + thresholds ---------- */
// MODE: 0=TRANSPORT, 1=TRANSIENT, 2=HYBRID (default)
let triggerMode = 2;
let punchThresh = 0.15;
let cutThresh   = 0.45;

/* ---------- transient detection state ---------- */
const FFT_SIZE = 512;
let prevMags = new Float32Array(FFT_SIZE);
let lastTriggerTime = 0;
const COOLDOWN = 120; // ms between triggers

/* ---------- Max integration ---------- */
if (maxApi) {
  maxApi.addHandler("send", (s) => {
    if (typeof s !== "string") return;
    try { const obj = JSON.parse(s); broadcast(obj); }
    catch (e) { logErr("send: bad JSON"); }
  });

  // Primary 8-band analysis: [pack 0. x8] -> [prepend spec8] -> [node.script]
  maxApi.addHandler("spec8", (...vals) => {
    const g = ctrlState.sens || 1.0;
    const raw = new Array(N_BANDS);
    for (let i = 0; i < N_BANDS; i++) raw[i] = shape((+vals[i] || 0) * RAW_GAIN * g);

    const spk = new Array(N_BANDS);
    for (let i = 0; i < N_BANDS; i++) {
      avgBands[i] += (raw[i] - avgBands[i]) * 0.15;
      spk[i] = clamp01(Math.max(0, raw[i] - avgBands[i]) * 3.0);
    }

    const spec  = new Array(SPEC_N);
    const spike = new Array(SPEC_N);
    for (let i = 0; i < SPEC_N; i++) {
      const f = (i / (SPEC_N - 1)) * (N_BANDS - 1);
      const a = Math.floor(f), b = Math.min(N_BANDS - 1, a + 1), t = f - a;
      spec[i]  = raw[a] * (1 - t) + raw[b] * t;
      spike[i] = spk[a] * (1 - t) + spk[b] * t;
    }

    const bass = clamp01((raw[0] + raw[1]) * 0.5);
    const mid  = clamp01((raw[2] + raw[3] + raw[4]) / 3);
    const high = clamp01((raw[5] + raw[6] + raw[7]) / 3);
    const energy = (bass + mid + high) / 3;

    broadcast({ type: "audio", bass, mid, high, energy, spec, spike });
  });

  maxApi.addHandler("audio", (bass, mid, high, energy, spec, spike) => {
    broadcast({
      type: "audio",
      bass: +bass || 0, mid: +mid || 0, high: +high || 0, energy: +energy || 0,
      spec: Array.isArray(spec) ? spec : [], spike: Array.isArray(spike) ? spike : []
    });
  });

  // ctrl as flat key/value list: ctrl relief 1.1
  maxApi.addHandler("ctrl", (...args) => {
    const obj = { type: "ctrl" };
    for (let i = 0; i + 1 < args.length; i += 2) {
      const k = String(args[i]); const v = +args[i + 1];
      if (isFinite(v)) { obj[k] = v; ctrlState[k] = v; }
    }
    broadcast(obj);
  });

  // --- NEW: transport handler ---
  // Max sends: transport <beat> <bar> <bpm>
  // beat is 1-based (1=downbeat), bar is 1-based
  maxApi.addHandler("transport", (...args) => {
    const beat = +args[0] || 1;
    const bar  = +args[1] || 1;
    const bpm  = +args[2] || 120;
    if (triggerMode === 0 || triggerMode === 2) {
      broadcast({ type: "beat", beat: beat, bar: bar, bpm: bpm });
    }
  });

  // --- NEW: fftdata handler (transient detection via spectral flux) ---
  // Max sends: fftdata <mag0> <mag1> ... <magN>
  // magnitudes from pfft~ are logarithmic — we normalize before flux calc
  maxApi.addHandler("fftdata", (...rawMags) => {
    if (triggerMode === 0) return; // transport-only mode, skip transient detection

    const len = Math.min(rawMags.length, FFT_SIZE);
    let flux = 0;
    for (let i = 0; i < len; i++) {
      const mag = Math.max(0, +rawMags[i] || 0);
      const diff = mag - prevMags[i];
      if (diff > 0) flux += diff;
      prevMags[i] = mag;
    }

    const now = Date.now();
    if (now - lastTriggerTime < COOLDOWN) return;

    const norm = len > 0 ? flux / len : 0;
    if (norm > cutThresh) {
      broadcast({ type: "cut", strength: Math.min(1, norm / cutThresh) });
      lastTriggerTime = now;
    } else if (norm > punchThresh) {
      broadcast({ type: "punch", strength: Math.min(1, norm / punchThresh) });
      lastTriggerTime = now;
    }
  });

  // --- NEW: mode handler ---
  // Max sends: mode <0|1|2>
  maxApi.addHandler("mode", (m) => {
    const v = +m;
    if (v === 0 || v === 1 || v === 2) {
      triggerMode = v;
      log("trigger mode: " + ["TRANSPORT", "TRANSIENT", "HYBRID"][triggerMode]);
    }
  });

  // --- NEW: threshold handlers ---
  // Max sends: punch_thresh <float>  or  cut_thresh <float>
  maxApi.addHandler("punch_thresh", (v) => {
    const f = +v;
    if (isFinite(f) && f > 0) { punchThresh = f; log("punch thresh: " + f); }
  });
  maxApi.addHandler("cut_thresh", (v) => {
    const f = +v;
    if (isFinite(f) && f > 0) { cutThresh = f; log("cut thresh: " + f); }
  });
}

/* ---------- standalone test pattern ---------- */
const wantsTestPattern =
  !maxApi && (process.argv.includes("--test") || process.env.TEST_PATTERN === "1");

if (wantsTestPattern) {
  log("test pattern: emitting synthetic 60 Hz sine/spike + beat clock");
  const COLS = 96;
  const spec  = new Array(COLS);
  const spike = new Array(COLS);
  const t0 = Date.now();
  let lastBeat = 0;
  const BPM = 120;
  setInterval(() => {
    const t = (Date.now() - t0) / 1000;
    for (let i = 0; i < COLS; i++) {
      const f = i / COLS;
      const v = 0.12 + 0.55 * Math.max(0, Math.sin(t * 2.0 - f * 6.0)) * (1.0 - 0.4 * f);
      spec[i]  = v;
      spike[i] = (Math.sin(t * 5 + i * 0.6) > 0.96) ? 0.7 : 0;
    }
    const bass   = 0.30 + 0.40 * Math.abs(Math.sin(t * 1.2));
    const mid    = 0.25 + 0.25 * Math.abs(Math.sin(t * 1.7 + 1));
    const high   = 0.15 + 0.40 * Math.abs(Math.sin(t * 2.3 + 2));
    const energy = (bass + mid + high) / 3;
    broadcast({ type: "audio", bass, mid, high, energy, spec, spike });

    // synthetic beat clock
    const beatNum = Math.floor(t * BPM / 60);
    if (beatNum > lastBeat) {
      lastBeat = beatNum;
      const beat = (beatNum % 4) + 1;
      const bar = Math.floor(beatNum / 4) + 1;
      broadcast({ type: "beat", beat: beat, bar: bar, bpm: BPM });
    }
  }, 16);
}
