"use strict";
/*
 * REVENANT — bridge.js
 *
 * Node-for-Max script that runs INSIDE the Revenant.amxd device.
 * - Serves the built REVENANT app (../dist) over HTTP on :8765
 * - Runs a WebSocket server on the same port
 * - Receives 8-band analysis ("spec8") from the Max patch, expands to the
 *   { type:'audio', bass,mid,high,energy, spec:[96], spike:[96] } contract,
 *   and broadcasts to the visual (which does its own transient detection).
 * - Relays macro-knob "ctrl <key> <val>" messages straight through.
 *
 * Standalone test (no Ableton):
 *   node bridge.js --test     # synthetic 110 BPM kick pattern
 *   open http://localhost:8765/
 */

const http = require("http");
const fs = require("fs");
const path = require("path");
const WebSocket = require("ws");

let maxApi = null;
try { maxApi = require("max-api"); } catch (_) { /* standalone */ }

const PORT = 8765;
// Prefer a dist copied next to the device (portable), else the repo's ../dist.
const LOCAL_DIST = path.resolve(__dirname, "dist");
const REPO_DIST  = path.resolve(__dirname, "..", "dist");
const DIST_DIR   = fs.existsSync(LOCAL_DIST) ? LOCAL_DIST : REPO_DIST;

function log(m)    { if (maxApi) { try { maxApi.post(String(m)); } catch (_) {} } else console.log(m); }
function logErr(m) { if (maxApi) { try { maxApi.post(String(m), maxApi.POST_LEVELS.ERROR); } catch (_) {} } else console.error(m); }

/* ---------- static HTTP ---------- */
const MIME = {
  ".html": "text/html; charset=utf-8", ".js": "application/javascript; charset=utf-8",
  ".css": "text/css; charset=utf-8", ".json": "application/json; charset=utf-8",
  ".png": "image/png", ".jpg": "image/jpeg", ".jpeg": "image/jpeg", ".svg": "image/svg+xml",
  ".glb": "model/gltf-binary", ".ico": "image/x-icon", ".map": "application/json",
};

const server = http.createServer((req, res) => {
  try {
    let urlPath = decodeURIComponent((req.url || "/").split("?")[0]);
    if (urlPath === "/" || urlPath === "") urlPath = "/index.html";
    const filePath = path.resolve(path.join(DIST_DIR, urlPath));
    if (!filePath.startsWith(DIST_DIR)) { res.statusCode = 403; res.end("forbidden"); return; }
    fs.readFile(filePath, (err, data) => {
      if (err) { res.statusCode = 404; res.end("not found"); return; }
      res.setHeader("Content-Type", MIME[path.extname(filePath).toLowerCase()] || "application/octet-stream");
      res.setHeader("Cache-Control", "no-cache");
      res.end(data);
    });
  } catch (e) { res.statusCode = 500; res.end("server error"); logErr("http: " + (e && e.message)); }
});

/* ---------- WebSocket ---------- */
const wss = new WebSocket.Server({ server });
const clients = new Set();
wss.on("connection", (ws) => {
  clients.add(ws);
  log("ws client connected (" + clients.size + ")");
  ws.on("close", () => clients.delete(ws));
  ws.on("error", () => clients.delete(ws));
});
function broadcast(obj) {
  let data; try { data = JSON.stringify(obj); } catch (e) { return; }
  for (const c of clients) if (c.readyState === WebSocket.OPEN) { try { c.send(data); } catch (_) {} }
}

server.on("error", (e) => {
  if (e && e.code === "EADDRINUSE") { log("port " + PORT + " busy — retry 2s"); setTimeout(() => { try { server.close(); } catch (_) {} server.listen(PORT); }, 2000); }
  else logErr("server: " + (e && e.message));
});
server.listen(PORT, () => {
  log("REVENANT bridge ready  http://localhost:" + PORT + "/  (dist: " + DIST_DIR + ")");
  log(maxApi ? "  mode: Max for Live" : (wantsTest ? "  mode: standalone --test" : "  mode: standalone idle"));
});

/* ---------- analysis ---------- */
const N_BANDS = 8, SPEC_N = 96, RAW_GAIN = 9.0, CURVE = 0.6;
const ctrlState = { sens: 1.3 };
const avgBands = new Array(N_BANDS).fill(0);
const clamp01 = x => x < 0 ? 0 : x > 1 ? 1 : x;
const shape = v => clamp01(Math.pow(clamp01(v), CURVE));

function emitSpec8(vals) {
  const g = ctrlState.sens || 1.0;
  const raw = new Array(N_BANDS);
  for (let i = 0; i < N_BANDS; i++) raw[i] = shape((+vals[i] || 0) * RAW_GAIN * g);
  const spk = new Array(N_BANDS);
  for (let i = 0; i < N_BANDS; i++) { avgBands[i] += (raw[i] - avgBands[i]) * 0.15; spk[i] = clamp01(Math.max(0, raw[i] - avgBands[i]) * 3.0); }
  const spec = new Array(SPEC_N), spike = new Array(SPEC_N);
  for (let i = 0; i < SPEC_N; i++) {
    const f = (i / (SPEC_N - 1)) * (N_BANDS - 1), a = Math.floor(f), b = Math.min(N_BANDS - 1, a + 1), t = f - a;
    spec[i] = raw[a] * (1 - t) + raw[b] * t; spike[i] = spk[a] * (1 - t) + spk[b] * t;
  }
  const bass = clamp01((raw[0] + raw[1]) * 0.5);
  const mid  = clamp01((raw[2] + raw[3] + raw[4]) / 3);
  const high = clamp01((raw[5] + raw[6] + raw[7]) / 3);
  broadcast({ type: "audio", bass, mid, high, energy: (bass + mid + high) / 3, spec, spike });
}

if (maxApi) {
  maxApi.addHandler("spec8", (...vals) => emitSpec8(vals));
  maxApi.addHandler("ctrl", (...args) => {
    const obj = { type: "ctrl" };
    for (let i = 0; i + 1 < args.length; i += 2) {
      const k = String(args[i]), v = +args[i + 1];
      if (isFinite(v)) { obj[k] = v; if (k === "sens") ctrlState.sens = v; }
    }
    broadcast(obj);
  });
  maxApi.addHandler("sens", (v) => { const f = +v; if (isFinite(f)) ctrlState.sens = f; });
}

/* ---------- standalone test ---------- */
const wantsTest = !maxApi && (process.argv.includes("--test") || process.env.TEST_PATTERN === "1");
if (wantsTest) {
  log("test: synthetic 110 BPM kick pattern");
  const COLS = 96, spec = new Array(COLS), spike = new Array(COLS), t0 = Date.now();
  let lastBeat = -1;
  setInterval(() => {
    const t = (Date.now() - t0) / 1000;
    const beat = Math.floor(t * 110 / 60);
    const kick = beat !== lastBeat; lastBeat = beat;
    for (let i = 0; i < COLS; i++) {
      const f = i / COLS;
      let v = 0.06 + 0.10 * Math.max(0, Math.sin(t * 2 - f * 6)) * (1 - 0.4 * f);
      if (kick && i < 8) v += 0.6;
      if (kick && beat % 2 === 1 && i > 60) v += 0.4;
      spec[i] = v; spike[i] = kick && (i < 8 || i > 60) ? 0.8 : 0;
    }
    const bass = kick ? 0.9 : 0.2, mid = 0.2, high = (kick && beat % 2 === 1) ? 0.7 : 0.15;
    broadcast({ type: "audio", bass, mid, high, energy: (bass + mid + high) / 3, spec, spike });
  }, 16);
}
