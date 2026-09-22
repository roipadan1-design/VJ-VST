import * as THREE from 'three'

// Textured scan surface — the photographic base that reads as a solid head when
// coherent, then dissolves + smears horizontally as disintegration rises.
const vertexShader = /* glsl */`
uniform float uTime;
uniform float uDisint;
uniform float uDrag;

varying vec2  vUv;
varying float vFade;

float hash(vec3 p){ return fract(sin(dot(p, vec3(12.9, 78.2, 37.7))) * 43758.5); }

void main() {
  vUv = uv;
  vec3 p = position;

  float r = hash(position * 7.0);

  // Horizontal smear: vertices drag sideways as the scan "reads out" laterally
  float dragSide = sign(r - 0.5);
  p.x += uDisint * (0.10 + r * 0.9) * dragSide * uDrag;
  // slight vertical striation jitter
  p.y += sin(position.y * 40.0 + uTime * 3.0) * uDisint * 0.02;

  // Per-vertex dropout so the mesh erodes into the point cloud
  vFade = 1.0 - smoothstep(0.0, 0.85, uDisint * (0.4 + r * 1.2));

  gl_Position = projectionMatrix * modelViewMatrix * vec4(p, 1.0);
}
`

const fragmentShader = /* glsl */`
uniform sampler2D uMap;
uniform float uDisint;
uniform float uContrast;
uniform float uBright;
uniform float uOpacity;

varying vec2  vUv;
varying float vFade;

void main() {
  vec3 t = texture2D(uMap, vUv).rgb;
  float lum = dot(t, vec3(0.299, 0.587, 0.114));

  // High-contrast matte grayscale — the photographic scan is the hero element.
  float g = pow(lum, uContrast);
  g = clamp((g - 0.05) * 1.7, 0.0, 1.0);

  // Dark areas transparent (head emerges from black); lit areas solid & matte.
  float alpha = smoothstep(0.03, 0.42, g) * vFade * uOpacity;
  if (alpha < 0.01) discard;

  vec3 col = vec3(0.30 * uBright + g * 0.72 * uBright);
  gl_FragColor = vec4(col, alpha);
}
`

export function createSurface(meshGeo, map) {
  map.colorSpace = THREE.SRGBColorSpace
  const mat = new THREE.ShaderMaterial({
    vertexShader,
    fragmentShader,
    uniforms: {
      uTime:     { value: 0 },
      uDisint:   { value: 0 },
      uMap:      { value: map },
      uDrag:     { value: 1.0 },
      uContrast: { value: 1.6 },
      uBright:   { value: 1.0 },
      uOpacity:  { value: 1.0 },
    },
    transparent: true,
    depthWrite:  false,
    side:        THREE.FrontSide,
  })
  const mesh = new THREE.Mesh(meshGeo, mat)
  mesh.frustumCulled = false
  return mesh
}
