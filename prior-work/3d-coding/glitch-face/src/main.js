import * as THREE from 'three';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';
import { KTX2Loader } from 'three/addons/loaders/KTX2Loader.js';
import { MeshoptDecoder } from 'three/addons/libs/meshopt_decoder.module.js';
import { heldRandom, breathe } from './rng.js';
import { GlitchShader, CopyShader, BAND_COUNT } from './glitchShader.js';

// ---------------------------------------------------------------------------
// Config — overridable via URL query string so the offline capture script
// (capture.js) can pin exact dimensions/fps/duration independent of the
// interactive preview.
// ---------------------------------------------------------------------------
const params = new URLSearchParams(location.search);
const CONFIG = {
  width: Number(params.get('w')) || 1080,
  height: Number(params.get('h')) || 1920,
  fps: Number(params.get('fps')) || 30,
  duration: Number(params.get('dur')) || 6, // seconds
  capture: params.get('capture') === '1',
  seed: Number(params.get('seed')) || 7, // change this to get a different corruption "take"
};
CONFIG.totalFrames = Math.round(CONFIG.fps * CONFIG.duration);
window.__CONFIG = CONFIG;

const hud = document.getElementById('hud');

// ---------------------------------------------------------------------------
// Renderer / scene / camera
// ---------------------------------------------------------------------------
const canvas = document.createElement('canvas');
document.body.appendChild(canvas);
canvas.style.maxWidth = '100vw';
canvas.style.maxHeight = '100vh';
canvas.style.width = 'auto';
canvas.style.height = '100vh';
canvas.style.margin = '0 auto';
canvas.style.display = 'block';

const renderer = new THREE.WebGLRenderer({ canvas, antialias: true, preserveDrawingBuffer: true });
renderer.setPixelRatio(1); // fixed so live preview == captured frames, pixel for pixel
renderer.setSize(CONFIG.width, CONFIG.height, true);
renderer.outputColorSpace = THREE.SRGBColorSpace;
renderer.toneMapping = THREE.ACESFilmicToneMapping;
renderer.toneMappingExposure = 1.15;

const scene = new THREE.Scene();
scene.background = new THREE.Color(0x000000);

const camera = new THREE.PerspectiveCamera(28, CONFIG.width / CONFIG.height, 0.01, 20);

// Key / rim / fill lighting tuned for a moody portrait — the corruption
// shader adds the colour, the lighting just needs to read the sculpt clearly.
const key = new THREE.DirectionalLight(0xfff2e6, 4.8);
key.position.set(0.5, 0.6, 1.6);
scene.add(key);

const rim = new THREE.DirectionalLight(0x7ad8ff, 3.6);
rim.position.set(-1.2, 0.5, -0.6);
scene.add(rim);

const rim2 = new THREE.DirectionalLight(0xff3ea5, 2.2);
rim2.position.set(1.3, -0.1, -0.9);
scene.add(rim2);

const fill = new THREE.HemisphereLight(0x445566, 0x0a0a0a, 1.4);
scene.add(fill);

// ---------------------------------------------------------------------------
// Load the face (real ARKit-named morph targets + a baked performance clip)
// ---------------------------------------------------------------------------
const ktx2 = new KTX2Loader()
  .setTranscoderPath('/node_modules/three/examples/jsm/libs/basis/')
  .detectSupport(renderer);

const loader = new GLTFLoader();
loader.setKTX2Loader(ktx2);
loader.setMeshoptDecoder(MeshoptDecoder);

/** @type {THREE.SkinnedMesh[]} */
let morphMeshes = [];
let mixer = null;
let modelRoot = null;
let clipDuration = 0;

// Curated subset of the 52 ARKit blend shapes to corrupt. Grouped so each
// family gets timing that suits it (eyes snap fast, jaw/mouth hold longer).
const GLITCH_TARGETS = [
  { name: 'jawOpen', seed: 1.1, group: 'slow' },
  { name: 'jawLeft', seed: 2.3, group: 'slow' },
  { name: 'jawRight', seed: 3.7, group: 'slow' },
  { name: 'mouthFunnel', seed: 4.2, group: 'mid' },
  { name: 'mouthPucker', seed: 5.9, group: 'mid' },
  { name: 'mouthLeft', seed: 6.6, group: 'mid' },
  { name: 'mouthRight', seed: 7.4, group: 'mid' },
  { name: 'mouthStretch_L', seed: 8.8, group: 'mid' },
  { name: 'mouthStretch_R', seed: 9.1, group: 'mid' },
  { name: 'cheekPuff', seed: 10.5, group: 'slow' },
  { name: 'noseSneer_L', seed: 11.2, group: 'fast' },
  { name: 'noseSneer_R', seed: 12.9, group: 'fast' },
  { name: 'eyeWide_L', seed: 13.3, group: 'fast' },
  { name: 'eyeWide_R', seed: 14.1, group: 'fast' },
  { name: 'eyeSquint_L', seed: 15.6, group: 'fast' },
  { name: 'eyeSquint_R', seed: 16.4, group: 'fast' },
  { name: 'browInnerUp', seed: 17.8, group: 'mid' },
  { name: 'browDown_L', seed: 18.2, group: 'mid' },
  { name: 'browDown_R', seed: 19.0, group: 'mid' },
  { name: 'tongueOut', seed: 20.5, group: 'slow' },
];

const GROUP_OPTS = {
  slow: { maxInterval: 0.42, burstProb: 0.3, attack: 0.4, burstMin: 0.3, burstMax: 1.0 },
  mid: { maxInterval: 0.24, burstProb: 0.38, attack: 0.28, burstMin: 0.3, burstMax: 1.0 },
  fast: { maxInterval: 0.1, burstProb: 0.45, attack: 0.15, burstMin: 0.4, burstMax: 1.0 },
};

loader.load(
  '/assets/models/facecap.glb',
  (gltf) => {
    modelRoot = gltf.scene;
    scene.add(modelRoot);

    modelRoot.traverse((obj) => {
      if (obj.isMesh && obj.morphTargetDictionary) {
        morphMeshes.push(obj);
      }
    });

    if (gltf.animations && gltf.animations.length) {
      mixer = new THREE.AnimationMixer(modelRoot);
      for (const clip of gltf.animations) {
        mixer.clipAction(clip).play();
        clipDuration = Math.max(clipDuration, clip.duration);
      }
    }

    frameCamera(modelRoot);
    window.__ready = true;
    if (!CONFIG.capture) start();
  },
  undefined,
  (err) => {
    hud.textContent = 'LOAD ERROR: ' + err.message;
    console.error(err);
  }
);

function frameCamera(root) {
  const box = new THREE.Box3().setFromObject(root);
  const size = new THREE.Vector3();
  const center = new THREE.Vector3();
  box.getSize(size);
  box.getCenter(center);

  // Portrait crop: eyes around the upper third, chin near the bottom edge —
  // look at a point slightly below the bbox center (mouth/chin area, since
  // faces have more volume above the eyeline than below) and pull back
  // enough that the whole face (not just the crown) fills the frame.
  const lookY = center.y - size.y * 0.12;
  const camDist = size.y * 1.05;
  camera.position.set(center.x, center.y + size.y * 0.02, center.z + camDist);
  camera.lookAt(center.x, lookY, center.z);
}

// ---------------------------------------------------------------------------
// Post-processing: manual ping-pong pipeline (scene -> glitch composite ->
// screen), with a feedback texture for the corrupted-trail smear look.
// ---------------------------------------------------------------------------
const sceneRT = new THREE.WebGLRenderTarget(CONFIG.width, CONFIG.height, { colorSpace: THREE.SRGBColorSpace });
let prevRT = new THREE.WebGLRenderTarget(CONFIG.width, CONFIG.height, { colorSpace: THREE.SRGBColorSpace });
let currRT = new THREE.WebGLRenderTarget(CONFIG.width, CONFIG.height, { colorSpace: THREE.SRGBColorSpace });

const fsCamera = new THREE.OrthographicCamera(-1, 1, 1, -1, 0, 1);
const fsGeo = new THREE.PlaneGeometry(2, 2);

const glitchMat = new THREE.ShaderMaterial(GlitchShader);
glitchMat.uniforms.uResolution.value.set(CONFIG.width, CONFIG.height);
const glitchQuad = new THREE.Mesh(fsGeo, glitchMat);
const glitchScene = new THREE.Scene();
glitchScene.add(glitchQuad);

const copyMat = new THREE.ShaderMaterial(CopyShader);
const copyQuad = new THREE.Mesh(fsGeo, copyMat);
const copyScene = new THREE.Scene();
copyScene.add(copyQuad);

// Per-band state driven by held-random noise, regenerated each frame.
const bandOffsets = glitchMat.uniforms.uBands.value; // array of THREE.Vector2, mutated in place

function updateBands(t, corruptionEnvelope) {
  for (let i = 0; i < BAND_COUNT; i++) {
    const seed = CONFIG.seed * 31.7 + i * 13.37;
    const shift = heldRandom(t, seed, {
      maxInterval: 0.09,
      burstProb: 0.06 + corruptionEnvelope * 0.55,
      restValue: 0,
      burstMin: 0.02,
      burstMax: 0.09,
      attack: 0.05,
    });
    const sign = heldRandom(t + 100.0, seed + 0.5, { maxInterval: 0.09, burstProb: 1.0, restValue: 0.5, burstMin: 0, burstMax: 1, attack: 0.0 }) > 0.5 ? 1 : -1;
    const cut = heldRandom(t, seed + 500.0, {
      maxInterval: 0.12,
      burstProb: 0.02 + corruptionEnvelope * 0.4,
      restValue: 0,
      burstMin: 0.35,
      burstMax: 0.75,
      attack: 0.0,
    });
    bandOffsets[i].set(shift * sign, cut);
  }
}

// ---------------------------------------------------------------------------
// Per-frame update: morph targets + shader uniforms, both pure functions of t
// ---------------------------------------------------------------------------
function updateGlitch(t) {
  if (mixer && clipDuration > 0) mixer.setTime(t % clipDuration);

  // global corruption envelope: alternates calmer / heavily-corrupted stretches
  const envelope = heldRandom(t, CONFIG.seed * 3.1 + 999, {
    maxInterval: 1.7,
    burstProb: 0.55,
    restValue: 0.12,
    burstMin: 0.55,
    burstMax: 1.0,
    attack: 0.5,
  });

  for (const target of GLITCH_TARGETS) {
    const opts = GROUP_OPTS[target.group];
    const glitchVal = heldRandom(t, CONFIG.seed * 5.3 + target.seed, opts);
    const breathVal = breathe(t, target.seed, 0.6) * 0.12;
    const value = Math.min(1, Math.max(glitchVal, breathVal) * (0.35 + envelope * 0.9));

    for (const mesh of morphMeshes) {
      const idx = mesh.morphTargetDictionary[target.name];
      if (idx === undefined) continue;
      const base = mesh.morphTargetInfluences[idx] || 0;
      mesh.morphTargetInfluences[idx] = Math.max(base, value);
    }
  }

  updateBands(t, envelope);

  const fastAb = heldRandom(t, CONFIG.seed * 7.1 + 321, { maxInterval: 0.1, burstProb: 0.4, restValue: 0.15, burstMin: 0.4, burstMax: 1.0, attack: 0.05 });
  glitchMat.uniforms.uAberration.value = 0.15 + envelope * 0.5 + fastAb * 0.35;
  glitchMat.uniforms.uFeedback.value = envelope * 0.55 * heldRandom(t, CONFIG.seed * 2.2 + 654, { maxInterval: 0.5, burstProb: 0.5, restValue: 0.1, burstMin: 0.4, burstMax: 1, attack: 0.3 });
  glitchMat.uniforms.uGrain.value = 0.035 + envelope * 0.06;
  glitchMat.uniforms.uScanline.value = 0.22;
  glitchMat.uniforms.uSeed.value = t * 13.0 + CONFIG.seed;

  const pushR = heldRandom(t, CONFIG.seed * 9.9 + 111, { maxInterval: 0.9, burstProb: 0.4, restValue: 0, burstMin: 0.06, burstMax: 0.16, attack: 0.4 });
  const pushG = heldRandom(t, CONFIG.seed * 9.9 + 222, { maxInterval: 1.1, burstProb: 0.35, restValue: 0, burstMin: 0.04, burstMax: 0.14, attack: 0.4 });
  const pushB = heldRandom(t, CONFIG.seed * 9.9 + 333, { maxInterval: 1.3, burstProb: 0.3, restValue: 0, burstMin: 0.04, burstMax: 0.12, attack: 0.4 });
  glitchMat.uniforms.uColorPush.value.set(pushR * envelope, pushG * envelope, -pushB * envelope * 0.6);

  return envelope;
}

function renderAtTime(t) {
  const envelope = updateGlitch(t);

  renderer.setRenderTarget(sceneRT);
  try { renderer.render(scene, camera); } catch (e) { console.error('SCENE PASS FAILED', e); throw e; }

  glitchMat.uniforms.tDiffuse.value = sceneRT.texture;
  glitchMat.uniforms.tPrev.value = prevRT.texture;
  renderer.setRenderTarget(currRT);
  try { renderer.render(glitchScene, fsCamera); } catch (e) { console.error('GLITCH PASS FAILED', e); throw e; }

  copyMat.uniforms.tDiffuse.value = currRT.texture;
  renderer.setRenderTarget(null);
  try { renderer.render(copyScene, fsCamera); } catch (e) { console.error('COPY PASS FAILED', e); throw e; }

  // ping-pong swap
  const tmp = prevRT;
  prevRT = currRT;
  currRT = tmp;

  if (!CONFIG.capture) {
    hud.textContent = `t=${t.toFixed(2)}s  env=${envelope.toFixed(2)}  ${CONFIG.width}x${CONFIG.height}@${CONFIG.fps}`;
  }
}

// Exposed for capture.js: renders one deterministic frame by index.
window.__setFrame = (frameIndex) => {
  const t = frameIndex / CONFIG.fps;
  renderAtTime(t);
};

// ---------------------------------------------------------------------------
// Live preview loop (skipped entirely in capture mode)
// ---------------------------------------------------------------------------
function start() {
  const clock = new THREE.Clock();
  function loop() {
    requestAnimationFrame(loop);
    const t = clock.getElapsedTime() % CONFIG.duration;
    renderAtTime(t);
  }
  loop();
}
