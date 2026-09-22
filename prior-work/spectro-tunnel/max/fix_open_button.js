"use strict";
// Moves the OPEN VISUAL button into the visible M4L device area (to the right of
// the dial grid) by editing the .amxd patcher JSON in place and fixing the
// 'ptch' chunk size header. Byte-accurate; no Max re-bake needed.
const fs = require("fs");
const FILE = "SpectroTunnel.amxd";

fs.copyFileSync(FILE, FILE + ".prefix.bak");
const b = fs.readFileSync(FILE);

// header: bytes 0..31  (ampf/ver/type/meta.../ptch + ptch-size@28)
const header = Buffer.from(b.slice(0, 32));
let json = b.toString("latin1", 32);

const oldR = 'presentation_rect" : [ 20.0, 170.0, 130.0, 20.0 ]';
const newR = 'presentation_rect" : [ 410.0, 50.0, 150.0, 60.0 ]';
if (json.indexOf(oldR) === -1) { console.error("open button rect not found"); process.exit(1); }
json = json.replace(oldR, newR);

const jsonBuf = Buffer.from(json, "latin1");
header.writeUInt32LE(jsonBuf.length, 28); // update ptch chunk size
fs.writeFileSync(FILE, Buffer.concat([header, jsonBuf]));

console.log("moved OPEN button to right of dials.");
console.log("new file size:", jsonBuf.length + 32, " (ptch size:", jsonBuf.length + ")");
