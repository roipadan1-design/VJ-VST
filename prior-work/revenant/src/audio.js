// Audio reactivity for REVENANT.
// Contract identical to the spectro-tunnel Max4Live bridge:
//   ws://localhost:8765  →  { type:'audio', bass,mid,high,energy, spec:[96], spike:[96] }
//   plus { type:'beat'|'punch'|'cut'|'ctrl', ... }
// Falls back to local mic (getUserMedia) and then to a calm synthetic demo.
//
// Transient detection is done HERE (from the spectrum), uniformly for ws/mic/demo,
// with an ADAPTIVE threshold and separate KICK (low) / HAT (high) onsets so the
// visual punches on the beat regardless of source.

const WS_URL = 'ws://localhost:8765'
const COLS   = 96

export function createAudio() {
  const state = {
    mode:  'demo',
    bass:  0, mid: 0, high: 0, energy: 0,
    pulse: 0,            // any onset (kick or hat), decays
    kick:  0,            // low-frequency transient env
    hat:   0,            // high-frequency transient env
    flux:  0,
    spec:  new Float32Array(COLS),
    spike: new Float32Array(COLS),
  }

  let onCtrl = null
  const setCtrlHandler = (fn) => { onCtrl = fn }

  const env  = { bass: 0, mid: 0, high: 0 }
  const slow = new Float32Array(COLS)
  const bandE = new Float32Array(COLS)
  const spr  = { bass: 0, mid: 0, high: 0, vb: 0, vm: 0, vh: 0 }
  let energy = 0

  // ── transient detection state ──
  const prevSpec = new Float32Array(COLS)
  let lowFluxAvg = 0, highFluxAvg = 0
  let lowCooldown = 0, highCooldown = 0
  // sensitivity (1 = default). Higher = lower threshold = more triggers.
  let transientSens = 1.0
  const setTransientSens = (v) => { transientSens = Math.max(0.1, v) }

  // ── WebSocket (Max bridge) ──
  let ws = null, retry = null
  function connect() {
    try { ws = new WebSocket(WS_URL) } catch { schedule(); return }
    ws.onopen = () => { if (state.mode !== 'mic') state.mode = 'ws' }
    ws.onmessage = (ev) => {
      let m; try { m = JSON.parse(ev.data) } catch { return }
      if (!m || typeof m !== 'object') return
      switch (m.type) {
        case 'audio':
          if (state.mode === 'mic') return
          if (state.mode !== 'ws') state.mode = 'ws'
          if (typeof m.bass === 'number') env.bass = m.bass
          if (typeof m.mid  === 'number') env.mid  = m.mid
          if (typeof m.high === 'number') env.high = m.high
          if (Array.isArray(m.spec)) for (let i = 0; i < Math.min(COLS, m.spec.length); i++) state.spec[i] = +m.spec[i] || 0
          break
        case 'beat':  if (m.beat === 1) { state.kick = Math.max(state.kick, 1) } ; state.pulse = Math.max(state.pulse, 0.7); break
        case 'punch': { const s = isFinite(m.strength) ? m.strength : 0.6; state.kick = Math.max(state.kick, s); state.pulse = Math.max(state.pulse, s); break }
        case 'cut':   state.pulse = 1.2; state.kick = 1; state.hat = 1; break
        case 'ctrl':  if (onCtrl) onCtrl(m); break
      }
    }
    const down = () => { if (state.mode === 'ws') state.mode = 'demo'; schedule() }
    ws.onerror = down; ws.onclose = down
  }
  function schedule() { if (retry) return; retry = setTimeout(() => { retry = null; connect() }, 3000) }

  // ── Mic fallback ──
  let analyser = null, freqData = null, sampleRate = 44100, bandIdx = []
  function computeBandIndex() {
    bandIdx = []
    const fLo = 32, fHi = 15000, hz = sampleRate / 2 / analyser.frequencyBinCount
    for (let i = 0; i < COLS; i++) {
      const a = fLo * Math.pow(fHi / fLo, i / COLS)
      const b = fLo * Math.pow(fHi / fLo, (i + 1) / COLS)
      bandIdx.push([Math.max(1, (a / hz) | 0), Math.max(2, Math.ceil(b / hz))])
    }
  }
  async function enableMic() {
    try {
      const st = await navigator.mediaDevices.getUserMedia({ audio: { echoCancellation: false, noiseSuppression: false, autoGainControl: false } })
      const ac = new (window.AudioContext || window.webkitAudioContext)()
      sampleRate = ac.sampleRate
      const src = ac.createMediaStreamSource(st)
      analyser = ac.createAnalyser(); analyser.fftSize = 2048; analyser.smoothingTimeConstant = 0.4
      src.connect(analyser); freqData = new Uint8Array(analyser.frequencyBinCount)
      computeBandIndex(); state.mode = 'mic'
      return true
    } catch (e) { return false }
  }

  const follow = (t, c) => (t > c ? t : c * 0.90)

  // demo synthetic beat clock
  let demoT = 0, demoBeat = -1

  function read(dt, gain = 1.3) {
    if (state.mode === 'mic' && analyser) {
      analyser.getByteFrequencyData(freqData)
      const avg = (a, b) => { let s = 0; for (let i = a; i < b; i++) s += freqData[i]; return (s / (b - a)) / 255 }
      env.bass = Math.min(1, follow(avg(1, 12)  * gain, env.bass))
      env.mid  = Math.min(1, follow(avg(12, 90) * gain, env.mid))
      env.high = Math.min(1, follow(avg(90, 360)* gain, env.high))
      for (let i = 0; i < COLS; i++) { const r = bandIdx[i]; state.spec[i] = Math.pow(Math.min(1, avg(r[0], r[1]) * gain * 1.1), 0.85) }
    } else if (state.mode === 'demo') {
      demoT += dt
      const tt = performance.now() * 0.0009
      for (let i = 0; i < COLS; i++) { const f = i / COLS; state.spec[i] = (0.05 + 0.07 * Math.abs(Math.sin(tt * 1.2 + f * 9))) * (1 - f * 0.4) }
      env.bass = 0.10 + 0.08 * Math.abs(Math.sin(tt))
      env.mid  = 0.09
      env.high = 0.06 + 0.06 * Math.abs(Math.sin(tt * 2.3))
      // synthetic 110 BPM kick so it looks alive standalone
      const beatN = Math.floor(demoT / (60 / 110))
      if (beatN !== demoBeat) {
        demoBeat = beatN
        state.kick = 1; state.pulse = Math.max(state.pulse, 0.9)
        if (beatN % 2 === 1) state.hat = 0.7           // off-beat hat
        for (let i = 0; i < 6; i++) state.spec[i] += 0.4 // kick spike into spectrum
      }
    }
    // (ws mode: env.* + spec filled by handler)

    // ── per-band energy + KICK/HAT transient detection (spectral flux) ──
    let flux = 0, lowFlux = 0, highFlux = 0
    for (let i = 0; i < COLS; i++) {
      const s = state.spec[i]
      const d = s - prevSpec[i]
      if (d > 0) { flux += d; if (i < 18) lowFlux += d; else if (i > 52) highFlux += d }
      prevSpec[i] = s
      // smoothed band energy + spike for HUD/spectrum
      const a = state.spec[Math.max(0, i - 2)], b = state.spec[i], c = state.spec[Math.min(COLS - 1, i + 2)]
      const tg = (a + b * 2 + c) / 4
      bandE[i] = tg > bandE[i] ? tg : bandE[i] * 0.88
      slow[i] += (bandE[i] - slow[i]) * 0.12
      state.spike[i] = Math.max(state.spike[i] * 0.85, Math.max(0, bandE[i] - slow[i]) * 3.2)
    }
    state.flux = flux / COLS

    // adaptive thresholds: running average of each group's flux + margin
    lowFluxAvg  += (lowFlux  - lowFluxAvg)  * 0.10
    highFluxAvg += (highFlux - highFluxAvg) * 0.10
    const lowThresh  = (lowFluxAvg  * 1.8 + 0.06) / transientSens
    const highThresh = (highFluxAvg * 2.0 + 0.05) / transientSens

    lowCooldown  -= dt; highCooldown -= dt
    if (lowFlux > lowThresh && lowCooldown <= 0) {
      state.kick = Math.min(1.5, lowFlux / Math.max(0.0001, lowThresh) * 0.8)
      state.pulse = Math.max(state.pulse, state.kick)
      lowCooldown = 0.09
    }
    if (highFlux > highThresh && highCooldown <= 0) {
      state.hat = Math.min(1.2, highFlux / Math.max(0.0001, highThresh) * 0.7)
      state.pulse = Math.max(state.pulse, state.hat * 0.8)
      highCooldown = 0.06
    }

    energy += (((env.bass + env.mid + env.high) / 3) - energy) * 0.06
    state.energy = energy

    spr.vb += (env.bass - spr.bass) * 0.22; spr.vb *= 0.62; spr.bass += spr.vb
    spr.vm += (env.mid  - spr.mid)  * 0.22; spr.vm *= 0.62; spr.mid  += spr.vm
    spr.vh += (env.high - spr.high) * 0.30; spr.vh *= 0.50; spr.high += spr.vh
    state.bass = Math.max(0, spr.bass); state.mid = Math.max(0, spr.mid); state.high = Math.max(0, spr.high)

    // envelope decays (fast attack already set above; release here)
    state.kick  *= Math.exp(-dt * 7.0)
    state.hat   *= Math.exp(-dt * 9.0)
    state.pulse *= Math.exp(-dt * 5.0)
  }

  connect()
  return { state, read, enableMic, setCtrlHandler, setTransientSens }
}
