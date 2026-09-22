import * as THREE from 'three'

// Particle layer. NOT a glowing dust over the face — these stay almost invisible
// while the head is coherent and only EMERGE as the scan disintegrates, carrying
// the grayscale scan tone (no cyan, no bloom halo).
const vertexShader = /* glsl */`
attribute vec3  aNormal;
attribute float aRand;
attribute float aAccent;
attribute float aInner;
attribute float aColor;

uniform float uTime;
uniform float uDisint;
uniform float uShed;
uniform float uPointScale;
uniform float uSpread;

varying float vInner;
varying float vBright;
varying float vEmerge;

void main() {
  vInner  = aInner;
  vBright = aColor;

  vec3 p = position;

  vec3 rv = vec3(
    sin(aRand * 127.1 + 1.0),
    sin(aRand * 311.7 + 2.0),
    sin(aRand * 74.37 + 3.0)
  );
  vec3 rdir = normalize(aNormal + rv * 1.3);

  // Horizontal-biased disintegration (uSpread widens the lateral smear)
  float dragSide = sign(rv.x);
  vec3  hdir = normalize(vec3(dragSide * (1.4 + uSpread * 1.6) + rdir.x, rdir.y * 0.35, rdir.z * 0.6));
  float scatter = uDisint * (0.25 + aRand * 1.6);
  p += hdir * scatter;
  p += rdir * scatter * 0.22;

  // Particles emerge only as the head disintegrates (a faint base for texture).
  // Edge points (steeper randomness) emerge slightly earlier.
  vEmerge = clamp(uDisint * 1.25 + step(0.93, aRand) * uShed * 0.4, 0.0, 1.0);

  vec4 mvPos = modelViewMatrix * vec4(p, 1.0);
  float sz = (aInner > 0.5 ? 1.3 : 2.0 + aColor * 1.0);
  sz *= uPointScale;
  gl_PointSize = clamp(sz * 300.0 / (-mvPos.z), 0.5, 9.0);
  gl_Position = projectionMatrix * mvPos;
}
`

const fragmentShader = /* glsl */`
uniform float uPBright;

varying float vInner;
varying float vBright;
varying float vEmerge;

void main() {
  vec2  pc = gl_PointCoord - 0.5;
  float d  = length(pc) * 2.0;
  float a  = 1.0 - smoothstep(0.3, 1.0, d);
  if (a < 0.01) discard;

  // Grayscale, tracking the scan tone. No color tint, no glow.
  float b = pow(vBright, 1.4);
  vec3  col = vec3(0.55 + b * 0.45);
  float alpha = a * (0.25 + b * 0.6) * vEmerge * uPBright;
  if (vInner > 0.5) alpha *= 0.5;

  if (alpha < 0.01) discard;
  gl_FragColor = vec4(col * alpha, alpha);
}
`

export function createPointsMaterial(dpr) {
  return new THREE.ShaderMaterial({
    vertexShader,
    fragmentShader,
    uniforms: {
      uTime:       { value: 0 },
      uDisint:     { value: 0 },
      uShed:       { value: 0.1 },
      uPointScale: { value: dpr },
      uSpread:     { value: 1.0 },
      uPBright:    { value: 1.0 },
    },
    transparent: true,
    depthWrite:  false,
    // Normal blending (not additive) so dense areas don't bloom into a white halo.
    blending:    THREE.NormalBlending,
  })
}

export function createPoints(geo, dpr) {
  const mat    = createPointsMaterial(dpr)
  const points = new THREE.Points(geo, mat)
  points.frustumCulled = false
  return points
}
