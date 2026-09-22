import './style.css'
import * as THREE from 'three'
import GUI from 'lil-gui'
import { buildHeadGeometry } from './head.js'
import { createPoints }      from './points.js'
import { createSurface }     from './surface.js'
import { createCage }        from './cage.js'
import { createComposer, updatePost } from './post.js'
import { initHud, updateHud }        from './hud.js'
import { computeEnvelopes }          from './loop.js'
import { createAudio }               from './audio.js'

const TWO_PI = Math.PI * 2

// ── Visible boot/error overlay (so a jweb black screen always says why) ───────
function showError(msg) {
  let el = document.getElementById('boot-error')
  if (!el) { el = document.createElement('div'); el.id = 'boot-error'; document.body.appendChild(el) }
  el.style.cssText = 'position:fixed;inset:0;display:flex;align-items:center;justify-content:center;' +
    'padding:24px;color:#9cf;font:13px/1.5 ui-monospace,Consolas,monospace;background:#000;' +
    'white-space:pre-wrap;text-align:center;z-index:99999'
  el.textContent = 'REVENANT — boot error:\n\n' + msg
}
window.addEventListener('error', (e) => showError((e.error && e.error.stack) || e.message || String(e)))
window.addEventListener('unhandledrejection', (e) => showError('promise: ' + ((e.reason && e.reason.stack) || e.reason)))

// ── Params (defaults tuned by ROI) ────────────────────────────────────────────
const params = {
  // motion
  rotateSpeed:  0.22,
  dolly:        1.0,
  nod:          1.0,
  // particles
  pointSize:    3.0,
  disintScale:  0.9,
  shedScale:    0.0,
  pointBright:  1.0,
  spread:       1.0,
  // surface
  surfContrast: 1.6,
  surfBright:   1.0,
  surfOpacity:  1.0,
  surfDrag:     1.0,
  // datamosh / glitch
  glitchScale:  0.7,
  dmSlice:      1.0,
  dmBlock:      1.0,
  dmSmear:      1.0,
  dmSeam:       1.0,
  // colour / post
  bloomInt:     0.95,
  grainAmt:     1.0,
  scanline:     0.08,
  vignette:     0.55,
  chroma:       1.0,
  blueScale:    0.2,
  blueHue:      '#0a2bff',
  // hud
  hudOpacity:   1.0,
  cageOpacity:  0.28,
  formula:      1.0,
  // audio reactivity
  audioGain:    1.3,
  transientSens: 1.0,
  bassDisint:   1.6,
  highGlitch:   1.3,
  beatBlue:     1.0,
  energyBloom:  0.6,
}

// ── Renderer ──────────────────────────────────────────────────────────────────
const canvas   = document.getElementById('scene')
let renderer
try {
  renderer = new THREE.WebGLRenderer({ canvas, antialias: false, powerPreference: 'high-performance' })
} catch (e) {
  showError('WebGL could not start in this window.\n' + (e && e.message) +
    '\n\nIf this is the Max jweb window, your Max/GPU may not allow WebGL — open\nhttp://localhost:8765/ in Chrome instead.')
  throw e
}
renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2))
renderer.setSize(window.innerWidth, window.innerHeight)
renderer.outputColorSpace = THREE.SRGBColorSpace

const scene  = new THREE.Scene()
const camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 0.1, 100)
camera.position.set(0, 0, 4.5)

// ── Audio ─────────────────────────────────────────────────────────────────────
const audio = createAudio()

// Macro knobs from the Max device arrive as { type:'ctrl', <key>:<val> }.
// Each key maps to a REVENANT param (these are the MIDI-mappable live.dials).
const CTRL_MAP = {
  react:     v => { params.bassDisint = v * 1.6; params.highGlitch = v * 1.3; params.beatBlue = v * 1.0 },
  disint:    v => { params.disintScale = v },
  glitch:    v => { params.glitchScale = v },
  blue:      v => { params.blueScale = v },
  bloom:     v => { params.bloomInt = v },
  rotate:    v => { params.rotateSpeed = v },
  grain:     v => { params.grainAmt = v },
  pointsize: v => { params.pointSize = v },
  transient: v => { params.transientSens = v; audio.setTransientSens(v) },
}
audio.setCtrlHandler((m) => {
  let touched = false
  for (const k in m) { if (k !== 'type' && CTRL_MAP[k]) { CTRL_MAP[k](+m[k]); touched = true } }
  if (touched) applyParams()
})

// ── Phase control (Playwright freeze) ─────────────────────────────────────────
let frozenPhase = null
let startTime   = performance.now() / 1000
let spin        = 0
let lastT       = performance.now() / 1000

window.__setPhase = (p) => { frozenPhase = p }
window.__unfreeze = ()  => { frozenPhase = null; startTime = performance.now() / 1000 }
window.__params   = params
window.__audio    = audio
window.__rebuild  = () => location.reload()

// ── Head + group ──────────────────────────────────────────────────────────────
let headGroup  = null
let pointsMesh = null
let surface    = null
let cage       = null
let anchors    = []

const { composer, effects } = createComposer(renderer, scene, camera)

buildHeadGeometry((p) => console.log(`Loading model: ${(p * 100).toFixed(0)}%`))
  .then(({ geo, anchors: a, meshGeo, map }) => {
    anchors = a
    const dpr  = renderer.getPixelRatio()
    surface    = createSurface(meshGeo, map)
    pointsMesh = createPoints(geo, dpr)
    cage       = createCage(geo)

    headGroup = new THREE.Group()
    headGroup.add(surface)
    headGroup.add(pointsMesh)
    headGroup.add(cage)
    scene.add(headGroup)

    geo.computeBoundingBox()
    initHud(document.getElementById('hud'), anchors, geo.boundingBox)
    applyParams()
    window.__loaded = true
    console.log('REVENANT: model ready')
  })
  .catch(err => console.error('Failed to load head:', err))

// ── Apply non-realtime params to effects/uniforms ─────────────────────────────
function applyParams() {
  effects.noise.blendMode.opacity.value    = params.grainAmt * 0.7
  effects.scanline.blendMode.opacity.value = params.scanline
  effects.vignette.blendMode.opacity.value = params.vignette
  effects.ca.offset = new THREE.Vector2(0.0010 * params.chroma, 0.0008 * params.chroma)
  const c = new THREE.Color(params.blueHue)
  effects.blueInvert.setHue(c.r, c.g, c.b)

  if (surface) {
    const u = surface.material.uniforms
    u.uContrast.value = params.surfContrast
    u.uBright.value   = params.surfBright
    u.uOpacity.value  = params.surfOpacity
    u.uDrag.value     = params.surfDrag
  }
  if (pointsMesh) {
    const u = pointsMesh.material.uniforms
    u.uSpread.value  = params.spread
    u.uPBright.value = params.pointBright
  }
  if (cage) cage.children.forEach((ch, i) => {
    ch.material.opacity = params.cageOpacity * (i === 0 ? 1 : 0.35)
  })
  const hud = document.getElementById('hud')
  if (hud) hud.style.opacity = params.hudOpacity
}

// ── lil-gui ───────────────────────────────────────────────────────────────────
const gui = new GUI({ title: 'REVENANT // controls', width: 250 })
gui.domElement.style.position = 'fixed'
gui.domElement.style.top      = '12px'
gui.domElement.style.right    = '12px'
const ch = (c) => c.onChange(applyParams)

const fMotion = gui.addFolder('Motion')
fMotion.add(params, 'rotateSpeed', 0.0, 1.2, 0.01).name('Rotation Speed')
fMotion.add(params, 'dolly',       0.0, 2.0, 0.05).name('Camera Dolly')
fMotion.add(params, 'nod',         0.0, 2.0, 0.05).name('Nod')

const fPart = gui.addFolder('Particles')
fPart.add(params, 'pointSize',   0.3, 5.0, 0.05).name('Point Size')
fPart.add(params, 'disintScale', 0.0, 2.0, 0.05).name('Disint Scale')
fPart.add(params, 'shedScale',   0.0, 2.0, 0.05).name('Shed Scale')
ch(fPart.add(params, 'pointBright', 0.0, 2.0, 0.05).name('Particle Bright'))
ch(fPart.add(params, 'spread',      0.0, 2.0, 0.05).name('H-Spread'))

const fSurf = gui.addFolder('Surface')
ch(fSurf.add(params, 'surfContrast', 0.8, 3.0, 0.05).name('Contrast'))
ch(fSurf.add(params, 'surfBright',   0.4, 2.0, 0.05).name('Brightness'))
ch(fSurf.add(params, 'surfOpacity',  0.0, 1.0, 0.02).name('Opacity'))
ch(fSurf.add(params, 'surfDrag',     0.0, 2.5, 0.05).name('Dissolve Drag'))

const fGlitch = gui.addFolder('Datamosh / Glitch')
fGlitch.add(params, 'glitchScale', 0.0, 2.0, 0.05).name('Glitch Master')
ch(fGlitch.add(params, 'dmSlice', 0.0, 2.0, 0.05).name('Slice Offset'))
ch(fGlitch.add(params, 'dmBlock', 0.0, 2.0, 0.05).name('Block Displace'))
ch(fGlitch.add(params, 'dmSmear', 0.0, 2.0, 0.05).name('H-Smear'))
ch(fGlitch.add(params, 'dmSeam',  0.0, 2.0, 0.05).name('Tear Seam'))

const fPost = gui.addFolder('Colour / Post')
fPost.add(params, 'bloomInt', 0.0, 3.0, 0.05).name('Bloom')
ch(fPost.add(params, 'grainAmt', 0.0, 1.0, 0.02).name('Grain'))
ch(fPost.add(params, 'scanline', 0.0, 0.4, 0.01).name('Scanline'))
ch(fPost.add(params, 'vignette', 0.0, 1.0, 0.02).name('Vignette'))
ch(fPost.add(params, 'chroma',   0.0, 3.0, 0.05).name('Chromatic Ab.'))
fPost.add(params, 'blueScale', 0.0, 2.0, 0.05).name('Blue Flash')
ch(fPost.addColor(params, 'blueHue').name('Blue Hue'))

const fHud = gui.addFolder('HUD')
ch(fHud.add(params, 'hudOpacity',  0.0, 1.0, 0.02).name('HUD Opacity'))
ch(fHud.add(params, 'cageOpacity', 0.0, 0.8, 0.02).name('Cage Opacity'))
ch(fHud.add(params, 'formula',     0.0, 1.0, 0.02).name('Formula Density'))

const fAudio = gui.addFolder('Audio Reactivity')
fAudio.add(params, 'audioGain',  0.2, 3.0, 0.05).name('Sensitivity')
fAudio.add(params, 'transientSens', 0.2, 3.0, 0.05).name('Transient Sens')
  .onChange(v => audio.setTransientSens(v))
fAudio.add(params, 'bassDisint', 0.0, 3.0, 0.05).name('Kick → Disint')
fAudio.add(params, 'highGlitch', 0.0, 3.0, 0.05).name('Hat → Glitch')
fAudio.add(params, 'beatBlue',   0.0, 2.0, 0.05).name('Beat → Blue')
fAudio.add(params, 'energyBloom',0.0, 2.0, 0.05).name('Energy → Bloom')
fAudio.add({ mic: () => audio.enableMic() }, 'mic').name('🎤 Enable Mic')

fPost.close(); fSurf.close(); fGlitch.close(); fHud.close()

// ── Resize ────────────────────────────────────────────────────────────────────
window.addEventListener('resize', () => {
  const w = window.innerWidth, h = window.innerHeight
  camera.aspect = w / h
  camera.updateProjectionMatrix()
  renderer.setSize(w, h)
  composer.setSize(w, h)
})

// ── Export ────────────────────────────────────────────────────────────────────
const LOOP_SECONDS = 8
function startExport() {
  const stream = canvas.captureStream(60)
  const rec    = new MediaRecorder(stream, { mimeType: 'video/webm;codecs=vp9', videoBitsPerSecond: 24e6 })
  const chunks = []
  rec.ondataavailable = e => { if (e.data.size) chunks.push(e.data) }
  rec.onstop = () => {
    const blob = new Blob(chunks, { type: 'video/webm' })
    const a = document.createElement('a')
    a.href = URL.createObjectURL(blob); a.download = 'revenant_loop.webm'; a.click()
    URL.revokeObjectURL(a.href); console.log('Export saved.')
  }
  rec.start(); setTimeout(() => rec.stop(), LOOP_SECONDS * 1000 + 200)
  console.log(`Recording ${LOOP_SECONDS}s…`)
}
window.__export = startExport
gui.add({ exportLoop: startExport }, 'exportLoop').name('⏺ Export Loop (8s)')

// ── Render loop ───────────────────────────────────────────────────────────────
function tick() {
  requestAnimationFrame(tick)

  const now = performance.now() / 1000
  const dt  = Math.min(0.05, now - lastT); lastT = now
  const elapsed = now - startTime
  const phase   = frozenPhase !== null ? frozenPhase : (elapsed % LOOP_SECONDS) / LOOP_SECONDS

  audio.read(dt, params.audioGain)

  // Frozen (Playwright) → force time-choreography for deterministic shots.
  const env = computeEnvelopes(phase, frozenPhase !== null ? null : audio.state, {
    bassDisint: params.bassDisint,
    highGlitch: params.highGlitch,
    beatBlue:   params.beatBlue,
  })

  const e = {
    disint: env.disint * params.disintScale,
    shed:   env.shed   * params.shedScale,
    blue:   env.blue   * params.blueScale,
    glitch: env.glitch * params.glitchScale,
    camZ:   4.5 + (env.camZ - 4.5) * params.dolly,
    nodX:   env.nodX * params.nod,
    energy: env.energy,
    kick:   env.kick || 0,
  }

  if (headGroup) {
    // Rotation: continuous & controllable (frozen → deterministic for shots)
    if (frozenPhase !== null) spin = phase * TWO_PI
    else spin += dt * params.rotateSpeed
    headGroup.rotation.y = spin
    headGroup.rotation.x = e.nodX
    // transient jolt — head punches forward/scales on the beat (visible movement)
    headGroup.scale.setScalar(1 + e.kick * 0.04)

    const tNow = frozenPhase !== null ? phase * LOOP_SECONDS : elapsed
    const pu = pointsMesh.material.uniforms
    pu.uTime.value       = tNow
    pu.uDisint.value     = e.disint
    pu.uShed.value       = e.shed
    pu.uPointScale.value = params.pointSize * renderer.getPixelRatio()

    const su = surface.material.uniforms
    su.uTime.value   = tNow
    su.uDisint.value = e.disint

    effects.bloom.intensity = params.bloomInt + e.energy * params.energyBloom

    camera.position.z = e.camZ
    camera.updateProjectionMatrix()

    updatePost(effects, e, tNow, params)
    updateHud(phase, e, camera, headGroup, anchors, audio.state, params.formula)
  }

  composer.render()
}

tick()

// ── Fullscreen (for the Max visual window on the HDMI screen) ──────────────────
function goFullscreen()  { if (!document.fullscreenElement) document.documentElement.requestFullscreen?.().catch(() => {}) }
function exitFullscreen(){ if (document.fullscreenElement)  document.exitFullscreen?.().catch(() => {}) }
function toggleFullscreen() { document.fullscreenElement ? exitFullscreen() : goFullscreen() }

// The very first user gesture (click or key) enters fullscreen — browsers/jweb
// require a gesture, so this is the reliable path. F toggles, Esc exits.
function firstGesture() {
  goFullscreen()
  window.removeEventListener('pointerdown', firstGesture)
  window.removeEventListener('keydown', firstGesture)
}
window.addEventListener('pointerdown', firstGesture)
window.addEventListener('keydown', firstGesture)
// best-effort auto-attempt (silently ignored if the gesture policy blocks it)
setTimeout(goFullscreen, 1000)

window.addEventListener('keydown', (e) => {
  const k = e.key.toLowerCase()
  if (k === 'f') toggleFullscreen()
  else if (k === 'h') { const h = document.getElementById('hud'); if (h) h.style.display = h.style.display === 'none' ? '' : 'none' }
  else if (k === 'g') gui.domElement.style.display = gui.domElement.style.display === 'none' ? '' : 'none'
})
canvas.addEventListener('dblclick', toggleFullscreen)
