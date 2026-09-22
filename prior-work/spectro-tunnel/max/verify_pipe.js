"use strict";
// One-shot verifier: confirms bridge.js serves the engine over HTTP and streams
// audio frames over WebSocket. Run while `node bridge.js --test` is up.
const http = require("http");
const WebSocket = require("ws");

function getHttp(path) {
  return new Promise((resolve, reject) => {
    http.get("http://localhost:8765" + path, (res) => {
      let body = "";
      res.on("data", (d) => (body += d));
      res.on("end", () => resolve({ status: res.statusCode, len: body.length, body }));
    }).on("error", reject);
  });
}

(async () => {
  let ok = true;
  try {
    const idx = await getHttp("/");
    const isHtml = idx.body.includes("<canvas") || idx.body.toLowerCase().includes("<!doctype");
    console.log(`HTTP /            -> ${idx.status}, ${idx.len} bytes, looks-like-html: ${isHtml}`);
    ok = ok && idx.status === 200 && isHtml;

    const eng = await getHttp("/engine.js");
    console.log(`HTTP /engine.js   -> ${eng.status}, ${eng.len} bytes`);
    ok = ok && eng.status === 200 && eng.len > 1000;

    const three = await getHttp("/three.min.js");
    console.log(`HTTP /three.min.js-> ${three.status}, ${three.len} bytes`);
    ok = ok && three.status === 200 && three.len > 100000;
  } catch (e) {
    console.log("HTTP error: " + e.message);
    ok = false;
  }

  await new Promise((resolve) => {
    const ws = new WebSocket("ws://localhost:8765");
    let frames = 0, sample = null;
    const done = () => { try { ws.close(); } catch (_) {} resolve(); };
    ws.on("open", () => console.log("WS open"));
    ws.on("message", (data) => {
      let m; try { m = JSON.parse(data); } catch { return; }
      if (m.type === "audio") {
        frames++;
        if (!sample) sample = m;
        if (frames >= 30) {
          console.log(`WS audio frames  -> received ${frames} in <1s`);
          console.log(`  sample: bass=${sample.bass.toFixed(2)} mid=${sample.mid.toFixed(2)} high=${sample.high.toFixed(2)} spec[${sample.spec.length}] spike[${sample.spike.length}]`);
          done();
        }
      }
    });
    ws.on("error", (e) => { console.log("WS error: " + e.message); ok = false; done(); });
    setTimeout(() => { if (frames < 30) { console.log(`WS: only ${frames} frames (server up but no stream?)`); ok = false; } done(); }, 2500);
  });

  console.log(ok ? "\nPIPE OK — server + engine + WS stream all good." : "\nPIPE FAIL — see above.");
  process.exit(ok ? 0 : 1);
})();
