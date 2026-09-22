const TWO_PI = Math.PI * 2

function bump(p, c, w) {
  const dx = Math.min(Math.abs(p - c), 1 - Math.abs(p - c))
  return Math.exp(-dx * dx / (2 * w * w))
}

// Time-based choreography (the standalone / demo "performance"), blended with live
// audio when a Max bridge or mic is connected. Returns RAW envelopes (0..~1.5);
// main.js multiplies them by the GUI scales.
export function computeEnvelopes(phase, audio, k) {
  // ── time choreography (calm) ──
  const big   = bump(phase, 0.62, 0.060)
  const pre   = bump(phase, 0.40, 0.030)
  const hits  = pre * 0.35
  const tDisint = Math.min(big * 1.0 + hits, 1.5)
  const tShed   = 0.03 + 0.08 * (1 - Math.cos(phase * TWO_PI)) / 2
  const tBlue   = Math.min(bump(phase, 0.66, 0.018) + bump(phase, 0.675, 0.010) * 0.5, 1.0)
  const tGlitch = Math.min(big * 1.25 + hits * 0.8 + tBlue * 0.9, 1.0)

  let disint, shed, blue, glitch, kick = 0

  if (audio && (audio.mode === 'ws' || audio.mode === 'mic' || audio.mode === 'demo')) {
    // ── audio / transient-driven ──
    const ar = k || {}
    kick = audio.kick
    // KICK punches disintegration; sustained bass adds a floor.
    disint = Math.min(audio.kick * (ar.bassDisint ?? 1.4) + audio.bass * (ar.bassDisint ?? 1.4) * 0.45 + audio.mid * 0.12, 1.6)
    // HAT/high drive glitch.
    glitch = Math.min(audio.hat * (ar.highGlitch ?? 1.2) + audio.high * (ar.highGlitch ?? 1.2) * 0.4 + audio.pulse * 0.25, 1.0)
    // Blue flashes on strong transients (mostly kick).
    blue   = Math.min(audio.kick * (ar.beatBlue ?? 0.9) * 0.8 + audio.hat * (ar.beatBlue ?? 0.9) * 0.35, 1.0)
    shed   = 0.02 + audio.mid * 0.2
  } else {
    disint = tDisint; shed = tShed; blue = tBlue; glitch = tGlitch
  }

  const energy = audio ? audio.energy : 0
  // camera kicks back on transients for visible movement
  const camZ = 4.5 + Math.sin(phase * TWO_PI) * 0.10 - kick * 0.35
  const nodX = Math.sin(phase * TWO_PI) * 0.02

  return { disint, shed, blue, glitch, camZ, nodX, energy, kick }
}
