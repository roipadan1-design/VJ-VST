import { Vector2, Vector3 } from 'three';

export const BAND_COUNT = 48;

export const GlitchShader = {
  uniforms: {
    tDiffuse: { value: null },
    tPrev: { value: null },
    uResolution: { value: new Vector2(1, 1) },
    uAberration: { value: 0.0 },
    uFeedback: { value: 0.0 },
    uColorPush: { value: new Vector3(0, 0, 0) },
    uGrain: { value: 0.05 },
    uScanline: { value: 0.25 },
    uSeed: { value: 0.0 },
    uBands: { value: new Array(BAND_COUNT).fill(0).map(() => new Vector2(0, 0)) },
  },

  vertexShader: /* glsl */ `
    varying vec2 vUv;
    void main() {
      vUv = uv;
      gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    }
  `,

  fragmentShader: /* glsl */ `
    varying vec2 vUv;
    uniform sampler2D tDiffuse;
    uniform sampler2D tPrev;
    uniform vec2 uResolution;
    uniform float uAberration;
    uniform float uFeedback;
    uniform vec3 uColorPush;
    uniform float uGrain;
    uniform float uScanline;
    uniform float uSeed;

    #define BAND_COUNT 48
    uniform vec2 uBands[BAND_COUNT];

    float hash(vec2 p) {
      return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
    }

    void main() {
      vec2 uv = vUv;

      int bandIdx = int(clamp(uv.y, 0.0, 0.9999) * float(BAND_COUNT));
      vec2 band = uBands[bandIdx];

      // horizontal block tear
      uv.x = clamp(uv.x + band.x, 0.0, 1.0);

      // chromatic aberration, strength flickers per band so it reads as
      // signal corruption rather than a constant stylised fringe
      float ab = uAberration * (0.35 + 0.65 * hash(vec2(float(bandIdx), uSeed)));
      vec2 dir = vec2(1.0, 0.06);
      vec3 col;
      col.r = texture2D(tDiffuse, clamp(uv + dir * ab, 0.0, 1.0)).r;
      col.g = texture2D(tDiffuse, uv).g;
      col.b = texture2D(tDiffuse, clamp(uv - dir * ab, 0.0, 1.0)).b;

      // per-band flicker / cut lines
      col *= mix(1.0, 0.4, band.y);

      // feedback trail (previous composited frame bleeding through)
      vec3 prevCol = texture2D(tPrev, uv).rgb;
      col = mix(col, prevCol, clamp(uFeedback, 0.0, 0.92));

      // full-frame corrupted colour push
      col += uColorPush;

      // fine grain
      float g = hash(uv * uResolution + uSeed * 91.7);
      col += (g - 0.5) * uGrain;

      // scanlines
      float sl = sin(uv.y * uResolution.y * 3.14159265);
      col *= 1.0 - uScanline * 0.5 * (1.0 - (sl * 0.5 + 0.5));

      gl_FragColor = vec4(clamp(col, 0.0, 1.0), 1.0);
    }
  `,
};

export const CopyShader = {
  uniforms: { tDiffuse: { value: null } },
  vertexShader: GlitchShader.vertexShader,
  fragmentShader: /* glsl */ `
    varying vec2 vUv;
    uniform sampler2D tDiffuse;
    void main() {
      gl_FragColor = texture2D(tDiffuse, vUv);
    }
  `,
};
