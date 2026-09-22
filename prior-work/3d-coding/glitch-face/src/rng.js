// Deterministic pseudo-random helpers. Every function here is a pure function
// of its numeric inputs, so the whole visual is 100% reproducible from `t`
// (seconds) — required for frame-by-frame offline capture to match the live
// preview exactly.

export function hash1(x) {
  const s = Math.sin(x * 12.9898) * 43758.5453;
  return s - Math.floor(s);
}

export function hash2(x, y) {
  return hash1(x * 127.1 + y * 311.7 + 74.7);
}

/**
 * "Held" pseudo-random signal: snaps to a new random target at irregular
 * intervals and eases into it, instead of smoothly oscillating. This reads
 * as digital corruption / signal drop rather than an organic breathing loop.
 *
 * @param {number} t     time in seconds
 * @param {number} seed  unique channel id
 * @param {object} opts
 *   maxInterval  grid size (s) used to derive irregular hold segments
 *   burstProb    probability a given segment jumps away from restValue
 *   restValue    value held between bursts (usually 0)
 *   burstMin/Max range for burst target values
 *   attack       fraction of the segment spent easing into the new value (0 = hard snap)
 */
export function heldRandom(t, seed, opts = {}) {
  const {
    maxInterval = 0.22,
    burstProb = 0.35,
    restValue = 0,
    burstMin = 0.35,
    burstMax = 1.0,
    attack = 0.3,
  } = opts;

  const grid = maxInterval;
  const cell = Math.floor(t / grid);
  const cellT = t / grid - cell;

  const nSeg = 1 + Math.floor(hash2(seed, cell) * 3); // 1..3 sub-segments per grid cell
  const segIdx = Math.min(nSeg - 1, Math.floor(cellT * nSeg));
  const segLocal = cellT * nSeg - segIdx;

  const valueFor = (segIndexAbs) => {
    const segSeed = seed + segIndexAbs * 7.77;
    const isBurst = hash2(segSeed, 1.234) < burstProb;
    return isBurst ? burstMin + hash2(segSeed, 5.678) * (burstMax - burstMin) : restValue;
  };

  // absolute segment index across the whole timeline (monotonic, so "previous" is well defined)
  const absIdx = cell * 3 + segIdx; // 3 = max possible sub-segments, keeps ordering stable
  const target = valueFor(absIdx);
  const prevTarget = valueFor(absIdx - 1);

  const a = attack <= 0 ? 1 : Math.min(segLocal / attack, 1);
  const eased = 1 - Math.pow(1 - a, 3);
  return prevTarget + (target - prevTarget) * eased;
}

/** Smooth low-amplitude "breathing" component, layered under the held-random bursts. */
export function breathe(t, seed, speed = 1.0) {
  return 0.5 + 0.5 * Math.sin(t * speed + seed * 6.283);
}
