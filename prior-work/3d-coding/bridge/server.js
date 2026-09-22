// Bridges Ableton Live 10 (via ableton-js) to the browser over WebSocket.
// Browser sketch connects to ws://localhost:8081 and receives JSON:
//   { type: "tempo", value: 126.0 }
//   { type: "isPlaying", value: true }
//   { type: "beat", bar: 3, beatInBar: 1 }
//
// Track meter polling was removed: real audio-reactivity now comes from the
// browser's own Web Audio FFT (via VoiceMeeter), so there's no need to also
// poll every track's meter over the UDP link to Live — that was adding one
// round-trip per track per tick and was the actual cause of periodic
// tempo/beat stutter (confirmed via "Command took longer than expected"
// warnings in the ableton-js logs under load).
//
// Ableton can restart or drop the connection mid-session (e.g. after installing
// a new Remote Script). Every poll is wrapped so a transient disconnect just
// skips a tick instead of crashing the whole bridge.

import { Ableton } from "ableton-js";
import { WebSocketServer } from "ws";

const WS_PORT = 8081;

const ableton = new Ableton({ logger: console });
const wss = new WebSocketServer({ port: WS_PORT });
const clients = new Set();

// Last known value per message type, keyed so a newly connected browser tab
// (which missed earlier broadcasts) is immediately caught up instead of
// showing "--" until the next change happens to occur.
const lastKnown = new Map();

function broadcast(msg) {
  const key = msg.type === "meter" ? `meter-${msg.trackIndex}` : msg.type;
  lastKnown.set(key, msg);
  const payload = JSON.stringify(msg);
  for (const client of clients) {
    if (client.readyState === client.OPEN) client.send(payload);
  }
}

wss.on("connection", (ws) => {
  clients.add(ws);
  ws.on("close", () => clients.delete(ws));
  for (const msg of lastKnown.values()) {
    ws.send(JSON.stringify(msg));
  }
});

let numerator = 4;
let isPlayingCached = false;

// One UDP round-trip per tick (current_song_time only) instead of two —
// is_playing is kept in sync via the listener below, not polled here.
async function pollBeat() {
  try {
    if (!isPlayingCached) return;
    const songTime = await ableton.song.get("current_song_time");
    const beat = Math.floor(songTime);
    const bar = Math.floor(beat / numerator) + 1;
    const beatInBar = (beat % numerator) + 1;
    broadcast({ type: "beat", bar, beatInBar, songTime });
  } catch (err) {
    // Ableton momentarily unreachable (restart, project load, etc.) — skip this tick
  }
}

async function main() {
  await ableton.start();
  console.log("Connected to Ableton Live");

  const song = ableton.song;

  // Live can briefly disconnect/reset right after a Remote Script loads —
  // retry the initial sync instead of letting one failed request kill the process.
  while (true) {
    try {
      const tempo = await song.get("tempo");
      broadcast({ type: "tempo", value: tempo });
      song.addListener("tempo", (value) => broadcast({ type: "tempo", value }));

      isPlayingCached = await song.get("is_playing");
      broadcast({ type: "isPlaying", value: isPlayingCached });
      song.addListener("is_playing", (value) => {
        isPlayingCached = value;
        broadcast({ type: "isPlaying", value });
      });

      numerator = await song.get("signature_numerator");
      break;
    } catch (err) {
      console.warn("Initial sync failed, retrying in 2s:", err?.message || err);
      await new Promise((r) => setTimeout(r, 2000));
    }
  }

  // 10Hz is plenty for beat-synced visuals and puts far less load on the
  // UDP link to Live than the original 30Hz + per-track meter polling did
  // (that combination was the cause of periodic tempo/beat stutter).
  setInterval(pollBeat, 1000 / 10);

  console.log(`WebSocket bridge listening on ws://localhost:${WS_PORT}`);
}

process.on("unhandledRejection", (err) => {
  console.error("Ignored disconnect-related error:", err?.message || err);
});

main().catch((err) => {
  console.error("Failed to connect to Ableton:", err);
  process.exit(1);
});
