import * as THREE from 'three'

const SVGNS = 'http://www.w3.org/2000/svg'

let svg         = null
let leaders     = []     // head-anchored callouts
let floaters    = []     // free-floating callouts in space
let scopeCtx    = null
let specCtx     = null
let frameN      = 0
let baseIds     = []
let cornerLocal = []
let formulaEl   = null
let keyEl       = null
let decryptEl   = null
let statusEl    = null

const SCALE_VALS = ['1.000', '0.752', '0.574', '0.397', '0.199', '0.000']
const GREEK  = 'ψΣλφΔΩ∂∮∇ξζηθμπτ'
const REDACT = '█▓░'
const HEX    = '0123456789ABCDEF'

const rnd = (a, b) => a + Math.random() * (b - a)
const pick = s => s[(Math.random() * s.length) | 0]
function hex(n) { let s = ''; for (let i = 0; i < n; i++) s += pick(HEX); return s }
function redactHex(n, solved) {
  let s = ''
  for (let i = 0; i < n; i++) s += (i < solved ? pick(HEX) : pick(REDACT))
  return s
}

function el(tag, attrs) {
  const e = document.createElementNS(SVGNS, tag)
  for (const k in attrs) e.setAttribute(k, attrs[k])
  return e
}

function makeLeader(extra) {
  const dot      = el('circle', { r: extra ? 1.6 : 2.2, class: 'ldr-dot' })
  const callLine = el('line', { class: 'ldr-call' })
  const dropLine = el('line', { class: 'ldr-drop' })
  const label    = el('text', { class: extra ? 'ldr-label ldr-crypt' : 'ldr-label' })
  svg.appendChild(dropLine)
  svg.appendChild(callLine)
  svg.appendChild(dot)
  svg.appendChild(label)
  return { dot, callLine, dropLine, label }
}

export function initHud(container, anchors, bbox) {
  const hud = document.getElementById('hud')

  svg = el('svg', { id: 'hud-svg' })
  svg.setAttribute('width', '100%'); svg.setAttribute('height', '100%')
  svg.style.position = 'absolute'; svg.style.inset = '0'
  hud.appendChild(svg)

  if (bbox) {
    cornerLocal = [
      new THREE.Vector3(bbox.min.x, bbox.min.y, bbox.max.z),
      new THREE.Vector3(bbox.max.x, bbox.min.y, bbox.max.z),
      new THREE.Vector3(bbox.min.x, bbox.min.y, bbox.min.z),
      new THREE.Vector3(bbox.max.x, bbox.min.y, bbox.min.z),
    ]
  }

  anchors.forEach(() => { leaders.push(makeLeader(false)); baseIds.push(74000 + (Math.random() * 900 | 0)) })

  // Extra free-floating cryptic callouts (orbit the head in local space)
  for (let i = 0; i < 5; i++) {
    floaters.push({
      L: makeLeader(true),
      pos: new THREE.Vector3(rnd(-1.1, 1.1), rnd(-0.9, 1.0), rnd(-0.6, 0.7)),
      seed: i * 2.3,
      txt: '',
    })
  }

  // ── Static DOM ──
  const tl = document.createElement('div')
  tl.className = 'top-left'
  tl.innerHTML = `
    <div>REVENANT</div>
    <div style="opacity:.55">threshold · subj_00</div>
    <div style="opacity:.4;margin-top:3px">SCAN · 3D · FORENSIC</div>
    <div class="status" id="hud-status" style="margin-top:6px">◌ LINK <span id="hud-mode">DEMO</span></div>
  `
  hud.appendChild(tl)
  statusEl = () => document.getElementById('hud-mode')

  const tr = document.createElement('div')
  tr.className = 'top-right'; tr.id = 'hud-tr'
  hud.appendChild(tr)

  const scale = document.createElement('div')
  scale.className = 'scale-left'
  SCALE_VALS.forEach(v => { const sp = document.createElement('span'); sp.textContent = v; scale.appendChild(sp) })
  hud.appendChild(scale)

  // ── Uncracked formula panel ──
  const fp = document.createElement('div')
  fp.className = 'formula-panel'
  fp.innerHTML = `
    <div class="fp-h">// UNRESOLVED · ENTROPY MAP</div>
    <div id="fp-lines"></div>
    <div class="fp-key">KEY <span id="fp-key">····</span></div>
    <div class="fp-decrypt"><span>DECRYPT</span><div class="fp-bar"><i id="fp-bar"></i></div><span id="fp-pct">00.0%</span></div>
  `
  hud.appendChild(fp)
  formulaEl = document.getElementById('fp-lines')
  keyEl     = document.getElementById('fp-key')
  decryptEl = { bar: document.getElementById('fp-bar'), pct: document.getElementById('fp-pct') }

  // ── Bottom panel ──
  const bottom = document.createElement('div')
  bottom.className = 'bottom-panel'

  const scopeWrap = document.createElement('div')
  scopeWrap.className = 'scope-box'
  const scopeCanvas = document.createElement('canvas')
  scopeCanvas.id = 'scope-canvas'; scopeCanvas.width = 200; scopeCanvas.height = 44
  scopeCtx = scopeCanvas.getContext('2d')
  scopeWrap.appendChild(scopeCanvas)
  const specCanvas = document.createElement('canvas')
  specCanvas.id = 'spec-canvas'; specCanvas.width = 200; specCanvas.height = 22
  specCtx = specCanvas.getContext('2d')
  scopeWrap.appendChild(specCanvas)
  const scopeRead = document.createElement('div')
  scopeRead.className = 'scope-read'; scopeRead.id = 'scope-read'
  scopeWrap.appendChild(scopeRead)
  bottom.appendChild(scopeWrap)

  const readouts = document.createElement('div')
  readouts.className = 'readouts'; readouts.id = 'hud-readouts'
  readouts.innerHTML = `
    <div><span class="lbl">DSP </span><span id="r-dsp">0.000</span></div>
    <div><span class="lbl">ROT </span><span id="r-rot">0.0°</span></div>
    <div><span class="lbl">BASS</span><span id="r-bass">0.000</span></div>
    <div><span class="lbl">HIGH</span><span id="r-high">0.000</span></div>
    <div><span class="lbl">FLUX</span><span id="r-flux">0.000</span></div>
  `
  bottom.appendChild(readouts)
  hud.appendChild(bottom)
}

const _v3 = new THREE.Vector3()
function project(vec, headGroup, camera, w, h) {
  _v3.copy(vec).applyMatrix4(headGroup.matrixWorld).project(camera)
  return { x: (_v3.x * 0.5 + 0.5) * w, y: (-_v3.y * 0.5 + 0.5) * h, front: _v3.z < 1 }
}

// throttled cryptic content
let cryptT = 0
let keyStr = '····'
let decryptPct = 0

export function updateHud(phase, env, camera, headGroup, anchors, audioState, formula = 1) {
  frameN++
  const w = window.innerWidth, h = window.innerHeight
  const { disint, shed, blue, camZ } = env
  const A = audioState || { bass: 0, mid: 0, high: 0, energy: 0, flux: 0, pulse: 0, spec: null, mode: 'demo' }

  const fade = (1 - Math.min(1, disint / 0.9))

  // ── Head leader lines ──
  leaders.forEach((L, i) => {
    if (!anchors[i]) return
    const p = project(anchors[i], headGroup, camera, w, h)
    const vis = p.front && fade > 0.05
    const op = vis ? fade : 0
    L.dot.style.opacity = op; L.callLine.style.opacity = op * 0.8
    L.dropLine.style.opacity = op * 0.4; L.label.style.opacity = op
    if (!vis) return
    const side = i % 2 === 0 ? -1 : 1
    const lx = p.x + side * 48, ly = p.y - 30 - (i % 3) * 9
    L.dot.setAttribute('cx', p.x); L.dot.setAttribute('cy', p.y)
    L.callLine.setAttribute('x1', p.x); L.callLine.setAttribute('y1', p.y)
    L.callLine.setAttribute('x2', lx); L.callLine.setAttribute('y2', ly)
    L.label.setAttribute('x', lx + (side < 0 ? -4 : 4)); L.label.setAttribute('y', ly - 3)
    L.label.setAttribute('text-anchor', side < 0 ? 'end' : 'start')
    const id = baseIds[i] + (frameN % 7)
    L.label.textContent = `${id}·${(0.4 + i * 0.09).toFixed(3)}`
    if (cornerLocal.length) {
      const c = project(cornerLocal[i % cornerLocal.length], headGroup, camera, w, h)
      L.dropLine.setAttribute('x1', p.x); L.dropLine.setAttribute('y1', p.y)
      L.dropLine.setAttribute('x2', c.x); L.dropLine.setAttribute('y2', c.y)
    }
  })

  // ── Floating cryptic callouts ──
  const fOp = formula * fade
  floaters.forEach((f, i) => {
    const p = project(f.pos, headGroup, camera, w, h)
    const vis = p.front && fOp > 0.05
    const L = f.L
    const op = vis ? fOp : 0
    L.dot.style.opacity = op * 0.9; L.callLine.style.opacity = op * 0.55; L.label.style.opacity = op
    L.dropLine.style.opacity = 0
    if (!vis) return
    const side = i % 2 ? 1 : -1
    const lx = p.x + side * 40, ly = p.y - 14
    L.dot.setAttribute('cx', p.x); L.dot.setAttribute('cy', p.y)
    L.callLine.setAttribute('x1', p.x); L.callLine.setAttribute('y1', p.y)
    L.callLine.setAttribute('x2', lx); L.callLine.setAttribute('y2', ly)
    L.label.setAttribute('x', lx + (side < 0 ? -3 : 3)); L.label.setAttribute('y', ly)
    L.label.setAttribute('text-anchor', side < 0 ? 'end' : 'start')
    if (f.txt) L.label.textContent = f.txt
  })

  // ── Top-right readout + audio meters ──
  const tc = (phase * 8).toFixed(2).padStart(5, '0')
  const fr = String(frameN).padStart(5, '0')
  const tr = document.getElementById('hud-tr')
  if (tr) {
    const bar = (v) => '▍'.repeat(Math.max(0, Math.min(8, Math.round(v * 8)))).padEnd(8, '·')
    tr.innerHTML = `
      <div>FRM <span class="blue">${fr}</span></div>
      <div>TC  <span class="blue">${tc}s</span></div>
      <div class="mtr">B <span class="blue">${bar(A.bass)}</span></div>
      <div class="mtr">M <span class="blue">${bar(A.mid)}</span></div>
      <div class="mtr">H <span class="blue">${bar(A.high)}</span></div>
    `
  }

  const set = (id, v) => { const e = document.getElementById(id); if (e) e.textContent = v }
  set('r-dsp', disint.toFixed(3))
  set('r-rot', (((headGroup.rotation.y * 180 / Math.PI) % 360 + 360) % 360).toFixed(1) + '°')
  set('r-bass', A.bass.toFixed(3))
  set('r-high', A.high.toFixed(3))
  set('r-flux', A.flux.toFixed(3))
  set('scope-read', `WAVE ${(0.3 + (A.energy) * 0.6).toFixed(3)} · BLU ${blue.toFixed(2)}`)

  // status
  const modeEl = document.getElementById('hud-mode')
  if (modeEl) {
    modeEl.textContent = A.mode.toUpperCase()
    modeEl.className = A.mode === 'ws' ? 'm-ws' : A.mode === 'mic' ? 'm-mic' : 'm-demo'
  }

  // ── Cryptic formula (throttled) ──
  cryptT += 1
  if (formulaEl && cryptT % 6 === 0) {
    const driveSolve = Math.min(1, A.energy * 1.2 + 0.1)
    const lines = []
    const N = Math.max(2, Math.round(formula * 6))
    for (let i = 0; i < N; i++) {
      const g1 = pick(GREEK), g2 = pick(GREEK)
      const solved = (Math.random() < driveSolve)
      const rhs = solved ? `0x${hex(4)}` : `0x${redactHex(4, (driveSolve * 4) | 0)}`
      const op  = pick('=≡≠→⊕⊘∴')
      lines.push(`<div>${g1}<sub>${(i + 1)}</sub>∂t ${op} ${rhs}·${g2}${Math.random() < 0.4 ? '?' : ''}</div>`)
    }
    formulaEl.innerHTML = lines.join('')

    // partially-solved key, occasionally advancing then resetting (never cracks)
    const solvedChars = Math.min(15, Math.floor(decryptPct / 100 * 16))
    keyStr = redactHex(16, solvedChars)
    if (keyEl) keyEl.textContent = keyStr.replace(/(.{4})/g, '$1 ').trim()

    // decrypt % wanders, driven by energy, but stalls below 100
    decryptPct += (A.energy * 8 - 1.5)
    if (decryptPct > 92) decryptPct = 28 + Math.random() * 10  // resets — uncrackable
    if (decryptPct < 0) decryptPct = 0
  }
  if (decryptEl) {
    decryptEl.bar.style.width = decryptPct.toFixed(1) + '%'
    decryptEl.pct.textContent = decryptPct.toFixed(1) + '%'
  }
  // assign cryptic text to floaters occasionally
  if (cryptT % 18 === 0) {
    floaters.forEach(f => { f.txt = `${pick(GREEK)}=${hex(2)}·${redactHex(3, Math.random() * 3 | 0)}` })
  }

  // ── Oscilloscope (audio waveform) ──
  if (scopeCtx) {
    const W = 200, H = 44
    scopeCtx.clearRect(0, 0, W, H)
    scopeCtx.strokeStyle = 'rgba(110,165,255,0.8)'
    scopeCtx.lineWidth = 1.1
    scopeCtx.beginPath()
    const spec = A.spec
    for (let x = 0; x < W; x++) {
      const t = x / W
      let wave
      if (spec) {
        const idx = (t * (spec.length - 1)) | 0
        wave = (spec[idx] - 0.2) * 2.0 * (1 + A.bass) + Math.sin(t * Math.PI * 30 + frameN * 0.2) * A.high * 0.4
      } else {
        wave = Math.sin(t * Math.PI * 8 + phase * 6.28) * 0.5
      }
      const y = H / 2 - wave * (H * 0.4)
      x === 0 ? scopeCtx.moveTo(x, y) : scopeCtx.lineTo(x, y)
    }
    scopeCtx.stroke()
  }

  // ── Spectrum bars (audio) ──
  if (specCtx) {
    const W = 200, H = 22, BARS = 32, bw = W / BARS - 1
    specCtx.clearRect(0, 0, W, H)
    const spec = A.spec
    for (let i = 0; i < BARS; i++) {
      let v
      if (spec) { const idx = (i / BARS * spec.length) | 0; v = Math.min(1, spec[idx] * 2.2) }
      else v = Math.abs(Math.sin(i * 0.4 + phase * 6.28)) * 0.4
      const bh = v * H
      specCtx.fillStyle = i % 4 === 0 ? 'rgba(110,165,255,0.7)' : 'rgba(200,220,255,0.35)'
      specCtx.fillRect(i * (bw + 1), H - bh, bw, bh)
    }
  }
}
