// ===== VJ Project — Original cube-sphere (unchanged mechanics) + VHS glitch world =====
// The 3D content is exactly the artist's original sketch: same double loop
// (rotateZ/rotateX/translate/box), same orbitControl camera. Only its color
// palette and a post-process shader change, to place it inside the reference
// aesthetic (image "5/11": black background, violet-to-coral vertical color
// streaks, heavy analog grain, a couple of distinct scan-line tears, and a
// mirrored black seam through the vertical center).
//
// No motion-trail/feedback effect here on purpose: p5's WEBGL background()
// alpha has known long-standing bugs (github.com/processing/p5.js/issues/4596,
// /1064) — a real trail needs ping-pong framebuffers, deliberately deferred
// to keep this pass low-risk. Ask if you want that layered in next.

let socket;
let bpm = 120;
let bar = 1;
let beatInBar = 1;
let lastBeatInBar = 1;
let beatFlash = 0; // spikes to 1 on each new beat, decays every frame

let audioCtx, analyser, micNode, freqData;
let audioStarted = false;
let bassLevel = 0;
let trebleLevel = 0;

let bgShader; // textures the big enveloping sphere behind the cubes
let bgSphereRadius; // set in setup() relative to canvas size

// Real camera-aware vertex shader (the official, proven pattern from p5's
// own shader examples: p5js.org/examples/3d-filter-shader and
// github.com/aferriss/p5jsShaderExamples/6_3d/6-1_rectangle) — NOT a
// screen-space shortcut. Because this runs through uProjectionMatrix /
// uModelViewMatrix like any normal 3D object, drawing it on a sphere means
// orbitControl's drag-to-rotate and scroll-to-zoom affect it exactly like
// they affect the cubes: it's real geometry in the same camera, not a flat
// overlay on the screen.
const bgVert = `
precision highp float;
attribute vec3 aPosition;
attribute vec2 aTexCoord;
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
varying vec2 vTexCoord;
void main() {
  vTexCoord = aTexCoord;
  gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aPosition, 1.0);
}
`;

// STEP 3a TEST — a UV gradient, not the final pattern yet. The point right
// now is only to confirm the sphere is textured AND that dragging/zooming
// with orbitControl visibly rotates/moves it, before adding the real
// streak/mirror/grain pattern on top of this same mechanism.
const bgFrag = `
precision highp float;
varying vec2 vTexCoord;
void main() {
  gl_FragColor = vec4(vTexCoord.x, vTexCoord.y, 0.6, 1.0);
}
`;

function setup() {
  // pixelDensity(1) keeps the offscreen buffer at CSS resolution instead of 2x.
  // On a fullscreen canvas the doubled buffer is what makes filter()/framebuffer
  // passes crash on some GPUs — this is the documented WEBGL stability fix.
  pixelDensity(1);
  createCanvas(windowWidth, windowHeight, WEBGL);
  angleMode(DEGREES);
  colorMode(HSB, 360, 100, 100, 1);
  noFill();
  describe(
    'A dense sphere of glowing wireframe cubes, fullscreen and mouse/scroll-orbitable, wrapped in a VHS-glitch treatment: chromatic aberration, analog grain, scan-line tears and a mirrored black seam, reacting to Ableton Live tempo and live audio.'
  );

  bgShader = createShader(bgVert, bgFrag);

  // The enveloping background sphere must sit well behind the cube-sphere
  // but still inside the camera's far clipping plane at any zoom level.
  // p5's default far plane may be too tight for a sphere this large, so set
  // it explicitly, generous relative to the sphere radius below.
  bgSphereRadius = min(windowWidth, windowHeight) * 6;
  perspective(PI / 3, windowWidth / windowHeight, 1, bgSphereRadius * 3);

  // If the GPU context ever drops (the real cause of a sudden black screen),
  // say so loudly instead of failing silently.
  const gl = drawingContext;
  if (gl && gl.canvas) {
    gl.canvas.addEventListener('webglcontextlost', (e) => {
      console.error('WEBGL CONTEXT LOST — GPU dropped the canvas.', e);
      setStatus(false, 'WEBGL קרס — רענן את הדף');
    });
  }

  connectBridge();
  requestMicPermissionForLabels();
  document.getElementById('audioBtn').addEventListener('click', startAudio);
}

function windowResized() {
  resizeCanvas(windowWidth, windowHeight);
  bgSphereRadius = min(windowWidth, windowHeight) * 6;
  perspective(PI / 3, windowWidth / windowHeight, 1, bgSphereRadius * 3);
}

function draw() {
  beatFlash *= 0.9;
  updateAudioLevels();

  const bass = audioStarted ? bassLevel : (sin(frameCount * 0.03) * 0.5 + 0.5);
  const treble = audioStarted ? trebleLevel : (sin(frameCount * 0.13) * 0.3 + 0.3);
  const pulse = audioStarted ? bassLevel : (sin(frameCount * 2) * 0.5 + 0.5);
  const radius = min(width, height) * (0.3 + pulse * 0.08);
  const boxSize = max(10, radius * 0.05);

  background(0);
  orbitControl(); // sets the camera BEFORE drawing anything — same camera
                   // now drives both the background sphere and the cubes.

  // --- STEP 3a TEST: enveloping background sphere, UV-gradient only so far.
  //     Real 3D geometry in the same camera as the cubes, so drag/zoom
  //     rotates and zooms it exactly like the cubes — not a screen overlay.
  //     fill(255) before drawing: noFill() is set globally below for the
  //     wireframe cubes, and the earlier bug showed p5 can skip submitting
  //     shaded geometry entirely when noFill() is active, regardless of the
  //     shader — so fill is restored just for this one draw. ---
  push();
  fill(255);
  noStroke();
  shader(bgShader);
  sphere(bgSphereRadius, 24, 16);
  pop();
  resetShader();

  // --- The original cubes, on top, UNCHANGED and clean: same double loop,
  //     same rotate/translate/box. Normal depth testing (sphere is much
  //     bigger/farther) keeps them in front, no clearDepth() trick needed. ---
  noFill();
  strokeWeight(max(3, min(width, height) * 0.007));
  for (let zAngle = 0; zAngle < 180; zAngle += 15) {
    for (let xAngle = 0; xAngle < 360; xAngle += 15) {
      // 265->380 (not 265->20 directly) so the sweep stays inside
      // violet/magenta/coral and never crosses green/cyan/yellow.
      const hue = map(zAngle, 0, 180, 265, 380) % 360;
      stroke(hue, 70, 85 + beatFlash * 15);
      push();
      rotateZ(zAngle);
      rotateX(xAngle);
      translate(0, radius, 0);
      box(boxSize);
      pop();
    }
  }
}

// ---------- Ableton bridge (WebSocket, see bridge/server.js) ----------

function connectBridge() {
  try {
    socket = new WebSocket('ws://localhost:8081');

    socket.onopen = () => setStatus(true, 'מחובר');
    socket.onclose = () => {
      setStatus(false, 'מנותק — מנסה שוב...');
      setTimeout(connectBridge, 2000);
    };
    socket.onerror = () => setStatus(false, 'שגיאה');

    socket.onmessage = (event) => {
      const msg = JSON.parse(event.data);
      if (msg.type === 'tempo') {
        bpm = msg.value;
        updateText('bpmText', bpm.toFixed(1));
      } else if (msg.type === 'beat') {
        bar = msg.bar;
        beatInBar = msg.beatInBar;
        if (beatInBar !== lastBeatInBar) {
          beatFlash = 1;
          lastBeatInBar = beatInBar;
        }
        updateText('beatText', `${bar}.${beatInBar}`);
      }
    };
  } catch (e) {
    console.warn('Bridge connection failed, running without it', e);
  }
}

function setStatus(on, text) {
  document.getElementById('wsDot').classList.toggle('on', on);
  document.getElementById('wsStatus').textContent = text;
}

// ---------- Audio (VB-Cable via raw Web Audio API) ----------

async function requestMicPermissionForLabels() {
  try {
    const tempStream = await navigator.mediaDevices.getUserMedia({ audio: true });
    tempStream.getTracks().forEach((t) => t.stop());
  } catch (e) {
    console.warn('Mic permission not granted yet', e);
  }
  await populateAudioDevices();
}

async function populateAudioDevices() {
  const select = document.getElementById('audioSource');
  const devices = await navigator.mediaDevices.enumerateDevices();
  const inputs = devices.filter((d) => d.kind === 'audioinput');

  select.innerHTML = '';
  if (inputs.length === 0) {
    select.innerHTML = '<option value="">אין מכשירי קלט</option>';
    return;
  }
  for (const d of inputs) {
    const opt = document.createElement('option');
    opt.value = d.deviceId;
    opt.textContent = d.label || 'מכשיר קלט (ללא שם)';
    if (/CABLE/i.test(d.label)) opt.selected = true;
    select.appendChild(opt);
  }
}

async function startAudio() {
  const deviceId = document.getElementById('audioSource').value;
  const btn = document.getElementById('audioBtn');
  try {
    const stream = await navigator.mediaDevices.getUserMedia({
      audio: deviceId ? { deviceId: { exact: deviceId } } : true,
    });
    audioCtx = new (window.AudioContext || window.webkitAudioContext)();
    await audioCtx.resume();
    micNode = audioCtx.createMediaStreamSource(stream);
    analyser = audioCtx.createAnalyser();
    analyser.fftSize = 1024;
    freqData = new Uint8Array(analyser.frequencyBinCount);
    micNode.connect(analyser);
    audioStarted = true;
    btn.textContent = 'אודיו מחובר ✓';
  } catch (e) {
    console.error('Audio start failed', e);
    btn.textContent = 'נכשל — נסו שוב';
  }
}

function updateAudioLevels() {
  if (!audioStarted) return;
  analyser.getByteFrequencyData(freqData);

  const bassRaw = averageRange(freqData, 0, 8) / 255;
  const trebleRaw = averageRange(freqData, 40, 100) / 255;

  bassLevel = lerp(bassLevel, bassRaw, 0.2);
  trebleLevel = lerp(trebleLevel, trebleRaw, 0.2);

  updateText('bassText', bassLevel.toFixed(2));
  updateText('trebText', trebleLevel.toFixed(2));
}

function averageRange(arr, start, end) {
  let sum = 0;
  for (let i = start; i < end; i++) sum += arr[i];
  return sum / (end - start);
}

function updateText(id, text) {
  const el = document.getElementById(id);
  if (el) el.textContent = text;
}
