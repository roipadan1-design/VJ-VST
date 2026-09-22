"use strict";
/*
 * build_amxd.js — generates Revenant.maxpat AND Revenant.amxd (Max 8 / M4L
 * Audio Effect) programmatically, so the JSON is always valid and the .amxd
 * can be dropped straight onto a track in Ableton Live (no Max bake needed).
 *
 * Run:  node build_amxd.js
 *
 * Device signal flow:
 *   plugin~ (L,R) -> plugout~              [audio passes through untouched]
 *           \-> +~ -> *~0.5 (mono) -> 8x reson~ band-pass -> 8x avg~
 *   metro 16 -> t b x8 -> bang each avg~ -> pack 8 -> [prepend spec8] -> node.script
 *   8x live.dial (macro knobs) -> [prepend ctrl <key>] -> node.script
 *   node.script bridge.js  -> serves ../dist + WebSocket :8765
 *   loadbang -> deferlow -> delay 2500 -> "open" -> pcontrol -> [p visual] (jweb)
 *       => the fullscreen-able visual window opens automatically on load
 */

const fs = require("fs");
const path = require("path");

let _id = 0;
const nid = () => "obj-" + (++_id);
const boxes = [];
const lines = [];
const box = (b) => { boxes.push({ box: b }); return b.id; };
const conn = (s, so, d, di) => lines.push({ patchline: { source: [s, so], destination: [d, di] } });

function newobj(text, x, y, w, opts) {
  return box(Object.assign({ id: nid(), maxclass: "newobj", text, numinlets: 1, numoutlets: 0, patching_rect: [x, y, w || 90, 22] }, opts || {}));
}
function msg(text, x, y, w, opts) {
  return box(Object.assign({ id: nid(), maxclass: "message", text, numinlets: 2, numoutlets: 1, outlettype: [""], patching_rect: [x, y, w || 60, 22] }, opts || {}));
}
function comment(text, x, y, w) {
  return box({ id: nid(), maxclass: "comment", text, numinlets: 1, numoutlets: 0, patching_rect: [x, y, w || 200, 20] });
}

comment("REVENANT — audio analysis -> Node -> WebSocket -> visual (auto-opens). Macro knobs are MIDI-mappable.", 30, 8, 640);

/* ---------- audio I/O + mono ---------- */
const pluginId  = newobj("plugin~", 40, 50, 60, { numinlets: 0, numoutlets: 2, outlettype: ["signal", "signal"] });
const plugoutId = newobj("plugout~", 40, 560, 66, { numinlets: 2, numoutlets: 0 });
const sumId     = newobj("+~", 200, 90, 40, { numinlets: 2, numoutlets: 1, outlettype: ["signal"] });
const monoId    = newobj("*~ 0.5", 200, 125, 50, { numinlets: 2, numoutlets: 1, outlettype: ["signal"] });
conn(pluginId, 0, plugoutId, 0); conn(pluginId, 1, plugoutId, 1);
conn(pluginId, 0, sumId, 0); conn(pluginId, 1, sumId, 1); conn(sumId, 0, monoId, 0);

/* ---------- 8-band filter bank ---------- */
const CF = [60, 150, 380, 850, 1800, 3600, 7000, 12000];
const Q  = [5, 6, 7, 8, 9, 10, 11, 12];
const NB = CF.length;
const avgIds = [];
for (let i = 0; i < NB; i++) {
  const x = 40 + i * 95;
  const r = newobj(`reson~ 1. ${CF[i]} ${Q[i]}`, x, 200, 90, { numinlets: 3, numoutlets: 1, outlettype: ["signal"] });
  const a = newobj("avg~", x, 245, 50, { numinlets: 1, numoutlets: 1, outlettype: ["float"] });
  conn(monoId, 0, r, 0); conn(r, 0, a, 0); avgIds.push(a);
}

/* ---------- metro clock -> pack 8 -> spec8 -> node ---------- */
const loadbangId = newobj("loadbang", 700, 50, 70, { numoutlets: 1, outlettype: ["bang"] });
const startMsg   = msg("1", 700, 85, 24);
const metroId    = newobj("metro 10", 700, 120, 70, { numinlets: 2, numoutlets: 1, outlettype: ["bang"] });
conn(loadbangId, 0, startMsg, 0); conn(startMsg, 0, metroId, 0);

const trigId = newobj("t" + " b".repeat(NB), 700, 160, 200, { numinlets: 1, numoutlets: NB, outlettype: Array(NB).fill("bang") });
conn(metroId, 0, trigId, 0);
const packId = newobj(`pack ${Array(NB).fill("0.").join(" ")}`, 700, 360, 200, { numinlets: NB, numoutlets: 1, outlettype: [""] });
for (let i = 0; i < NB; i++) { conn(trigId, i, avgIds[i], 0); conn(avgIds[i], 0, packId, i); }
const specPrepend = newobj("prepend spec8", 700, 400, 110, { numinlets: 1, numoutlets: 1, outlettype: [""] });
conn(packId, 0, specPrepend, 0);

const nodeId = newobj("node.script bridge.js @autostart 1 @watch 1", 700, 460, 300, { numinlets: 1, numoutlets: 2, outlettype: ["", "bang"] });
conn(specPrepend, 0, nodeId, 0);
comment("bridge.js: serves ../dist on :8765, broadcasts analysis + macro ctrl over WebSocket", 700, 500, 420);

/* ---------- macro knobs (live.dial -> ctrl <key>) ---------- */
const DIALS = [
  ["react",     "Reactivity",   0,    2.0,  1.0],
  ["disint",    "Disintegrate", 0,    2.0,  0.9],
  ["glitch",    "Glitch",       0,    2.0,  0.7],
  ["blue",      "Blue Flash",   0,    2.0,  0.2],
  ["bloom",     "Bloom",        0,    3.0,  0.95],
  ["rotate",    "Rotation",     0,    1.2,  0.22],
  ["grain",     "Grain",        0,    1.0,  1.0],
  ["transient", "Transient",    0.2,  3.0,  1.0],
];
comment("MACRO CONTROLS (Presentation) — right-click a knob → MIDI-map. Saved with the Live Set.", 40, 300, 560);
DIALS.forEach((d, i) => {
  const [key, longname, mn, mx, init] = d;
  const px = 40 + (i % 4) * 95, py = 340 + Math.floor(i / 4) * 95;
  const dialId = box({
    id: nid(), maxclass: "live.dial", varname: key,
    parameter_enable: 1, numinlets: 1, numoutlets: 2, outlettype: ["", "float"],
    patching_rect: [px, py, 60, 48],
    presentation: 1, presentation_rect: [20 + (i % 4) * 80, 20 + Math.floor(i / 4) * 64, 56, 48],
    saved_attribute_attributes: { valueof: {
      parameter_longname: longname, parameter_shortname: longname.length > 11 ? key : longname,
      parameter_type: 0, parameter_mmin: mn, parameter_mmax: mx,
      parameter_initial: [init], parameter_initial_enable: 1, parameter_unitstyle: 1
    } }
  });
  const prep = newobj(`prepend ctrl ${key}`, px, py + 55, 120, { numinlets: 1, numoutlets: 1, outlettype: [""] });
  conn(dialId, 0, prep, 0); conn(prep, 0, nodeId, 0);
});

/* ---------- visual window (jweb) — auto-opens on load ---------- */
function buildVisualSubpatcher() {
  const sb = [], sl = [];
  const sbox = (b) => { sb.push({ box: b }); return b.id; };
  const sconn = (a, ao, d, di) => sl.push({ patchline: { source: [a, ao], destination: [d, di] } });
  // An inlet is REQUIRED so [p visual] has an inlet for pcontrol to attach to —
  // without it the pcontrol→p connection is dropped on load and "open" does nothing.
  const inId = sbox({ id: nid(), maxclass: "inlet", numinlets: 0, numoutlets: 1, outlettype: [""], patching_rect: [20, 10, 30, 30] });
  const lb = sbox({ id: nid(), maxclass: "newobj", text: "loadbang", numinlets: 1, numoutlets: 1, outlettype: ["bang"], patching_rect: [70, 10, 70, 22] });
  // Retry the load a few times — the node.script bridge needs a moment to boot,
  // so the first attempt at device-load can hit a not-yet-listening server.
  const delA = sbox({ id: nid(), maxclass: "newobj", text: "del 1500", numinlets: 2, numoutlets: 1, outlettype: ["bang"], patching_rect: [70, 45, 70, 22] });
  const delB = sbox({ id: nid(), maxclass: "newobj", text: "del 3500", numinlets: 2, numoutlets: 1, outlettype: ["bang"], patching_rect: [150, 45, 70, 22] });
  const urlMsg = sbox({ id: nid(), maxclass: "message", text: "url http://localhost:8765/", numinlets: 2, numoutlets: 1, outlettype: [""], patching_rect: [20, 80, 220, 22] });
  // jweb is in PRESENTATION (fills the window); the control objects are not, so
  // the opened window shows ONLY the visual — clean, no Max objects.
  const web = sbox({ id: nid(), maxclass: "jweb", varname: "revweb", numinlets: 1, numoutlets: 2, outlettype: ["", ""],
    patching_rect: [20, 120, 1280, 720], presentation: 1, presentation_rect: [0, 0, 1600, 900] });
  sconn(inId, 0, urlMsg, 0);                 // reload when opened/poked
  sconn(lb, 0, urlMsg, 0);                   // immediate attempt
  sconn(lb, 0, delA, 0); sconn(delA, 0, urlMsg, 0);   // retry @1.5s
  sconn(lb, 0, delB, 0); sconn(delB, 0, urlMsg, 0);   // retry @3.5s
  sconn(urlMsg, 0, web, 0);
  return { fileversion: 1, appversion: { major: 8, minor: 5, revision: 5, architecture: "x64", modernui: 1 },
    classnamespace: "box", rect: [0, 0, 1600, 900], openinpresentation: 1, boxes: sb, lines: sl };
}
const visualId = box({ id: nid(), maxclass: "newobj", text: "p visual", numinlets: 1, numoutlets: 0, patching_rect: [40, 470, 70, 22], patcher: buildVisualSubpatcher() });
const pctrlId = newobj("pcontrol", 40, 440, 70, { numinlets: 1, numoutlets: 1, outlettype: ["bang"] });
conn(pctrlId, 0, visualId, 0);

// manual OPEN button (presentation)
const openMsg = msg("open", 40, 410, 60, { presentation: 1, presentation_rect: [20, 150, 130, 26] });
conn(openMsg, 0, pctrlId, 0);

// auto-open: loadbang -> deferlow -> delay 2500 (wait for bridge) -> open
const vLoad   = newobj("loadbang", 180, 410, 70, { numoutlets: 1, outlettype: ["bang"] });
const vDefer  = newobj("deferlow", 180, 438, 70, { numinlets: 1, numoutlets: 1, outlettype: ["bang"] });
const vDelay  = newobj("delay 2500", 180, 466, 80, { numinlets: 2, numoutlets: 1, outlettype: ["bang"] });
conn(vLoad, 0, vDefer, 0); conn(vDefer, 0, vDelay, 0); conn(vDelay, 0, pctrlId, 0);
comment("VISUAL auto-opens ~2.5s after load. Drag the window to your HDMI screen, press F for fullscreen.", 130, 410, 360);

/* ---------- assemble + write ---------- */
const patcherObj = {
  fileversion: 1, appversion: { major: 8, minor: 5, revision: 5, architecture: "x64", modernui: 1 },
  classnamespace: "box", rect: [80, 80, 1120, 720], openrect: [0, 0, 0, 200], bglocked: 0,
  openinpresentation: 1, default_fontsize: 12.0, default_fontface: 0, default_fontname: "Arial",
  gridonopen: 1, gridsize: [15.0, 15.0], gridsnaponopen: 1, objectsnaponopen: 1,
  statusbarvisible: 2, toolbarvisible: 1, lefttoolbarpinned: 0, toptoolbarpinned: 0,
  righttoolbarpinned: 0, bottomtoolbarpinned: 0, toolbars_unpinned_last_save: 0,
  tallnewobj: 0, boxanimatetime: 200, enablehscroll: 1, enablevscroll: 1, devicewidth: 0.0,
  description: "", digest: "", tags: "", style: "", subpatcher_template: "",
  assistshowspatchername: 0, title: "REVENANT", boxes, lines, dependency_cache: [], autosave: 0,
};
const patcher = { patcher: patcherObj };

const outPat = path.join(__dirname, "Revenant.maxpat");
fs.writeFileSync(outPat, JSON.stringify(patcher, null, 2));
console.log("wrote " + outPat);

// AMXD: "ampf" LE32(4) "aaaameta" LE32(4) LE32(1) "ptch" LE32(jsonLen+1) JSON \0
const jsonBuf = Buffer.from(JSON.stringify(patcher, null, "\t").replace(/\n/g, "\r\n"), "utf8");
const amxd = Buffer.alloc(32 + jsonBuf.length + 1);
amxd.write("ampf", 0, 4, "ascii"); amxd.writeUInt32LE(4, 4);
amxd.write("aaaameta", 8, 8, "ascii"); amxd.writeUInt32LE(4, 16); amxd.writeUInt32LE(1, 20);
amxd.write("ptch", 24, 4, "ascii"); amxd.writeUInt32LE(jsonBuf.length + 1, 28);
jsonBuf.copy(amxd, 32); amxd[32 + jsonBuf.length] = 0;
const outAmxd = path.join(__dirname, "Revenant.amxd");
fs.writeFileSync(outAmxd, amxd);
console.log("wrote " + outAmxd + " (" + amxd.length + " bytes)");
console.log("boxes: " + boxes.length + ", lines: " + lines.length);
