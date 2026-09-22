import * as THREE from 'three'
import {
  EffectComposer,
  EffectPass,
  RenderPass,
  BloomEffect,
  ChromaticAberrationEffect,
  NoiseEffect,
  ScanlineEffect,
  VignetteEffect,
  Effect,
  BlendFunction,
} from 'postprocessing'

// --- Custom: datamosh — horizontal slice offsets + block displacement + smear ---
const datamoshFrag = /* glsl */`
uniform float uGlitch;
uniform float uTime;
uniform float uSlice;
uniform float uBlock;
uniform float uSmear;
uniform float uSeam;

float hash2(vec2 p){
  return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void mainImage(const in vec4 inputColor, const in vec2 uv, out vec4 outputColor){
  vec2 u = uv;
  float t = floor(uTime * 9.0);
  float g = uGlitch;

  // 1) Coarse horizontal SLICES jump sideways (the head cut + offset look)
  float slice  = floor(uv.y * 14.0);
  float sActive = step(0.55, hash2(vec2(slice, t)));
  float sOff    = (hash2(vec2(slice, t + 4.0)) - 0.5) * 2.0;
  u.x += sActive * sOff * g * 0.18 * uSlice;

  // 2) Fine pixel-drag bands stretch content laterally into streaks
  float row    = floor(uv.y * 90.0);
  float actv = step(0.66, hash2(vec2(row, t))) * g;
  float dir  = sign(hash2(vec2(row, 7.0)) - 0.5);
  u.x += actv * dir * (0.05 + 0.22 * hash2(vec2(row, t + 1.0))) * uSmear;

  // 3) Rectangular BLOCK displacement (chunky datamosh, strongest near peaks)
  vec2 blk = floor(uv * vec2(16.0, 26.0));
  float bj = step(0.86, hash2(blk + t)) * g;
  u += (vec2(hash2(blk) - 0.5, (hash2(blk + 9.0) - 0.5) * 0.4)) * bj * 0.10 * uBlock;

  // 4) Vertical tear seam
  float seamX = hash2(vec2(t, 3.0));
  float seam  = smoothstep(0.010, 0.0, abs(uv.x - seamX)) * g;
  u.x += seam * 0.10 * dir * uSeam;

  vec4 col = texture2D(inputBuffer, u);

  // Bright scan-row streaks on strongly active bands
  col.rgb += vec3(0.07, 0.10, 0.16) * actv * step(0.88, hash2(vec2(row, t + 5.0))) * uSmear;
  // subtle cool tint on block seams (kept mostly grayscale)
  col.rgb += vec3(0.015, 0.025, 0.05) * bj * uBlock;

  outputColor = col;
}
`

class DatamoshEffect extends Effect {
  constructor() {
    super('DatamoshEffect', datamoshFrag, {
      blendFunction: BlendFunction.NORMAL,
      uniforms: new Map([
        ['uGlitch', new THREE.Uniform(0)],
        ['uTime',   new THREE.Uniform(0)],
        ['uSlice',  new THREE.Uniform(1)],
        ['uBlock',  new THREE.Uniform(1)],
        ['uSmear',  new THREE.Uniform(1)],
        ['uSeam',   new THREE.Uniform(1)],
      ]),
    })
  }
  set glitch(v) { this.uniforms.get('uGlitch').value = v }
  set time(v)   { this.uniforms.get('uTime').value   = v }
  set slice(v)  { this.uniforms.get('uSlice').value  = v }
  set block(v)  { this.uniforms.get('uBlock').value  = v }
  set smear(v)  { this.uniforms.get('uSmear').value  = v }
  set seam(v)   { this.uniforms.get('uSeam').value   = v }
}

// --- Custom: Blue invert flash (electric-blue flood, grayscale dressed on top) ---
const blueInvertFrag = /* glsl */`
uniform float uBlue;
uniform vec3  uBlueHue;

void mainImage(const in vec4 inputColor, const in vec2 uv, out vec4 outputColor){
  vec3  c   = inputColor.rgb;
  float lum = dot(c, vec3(0.299, 0.587, 0.114));

  float onTop = smoothstep(0.12, 0.55, lum);
  vec3  bf    = mix(uBlueHue, vec3(pow(lum, 0.85)), onTop);
  bf = mix(bf, bf + uBlueHue * 0.25, (1.0 - onTop) * 0.5);

  outputColor = vec4(mix(c, bf, clamp(uBlue, 0.0, 1.0)), inputColor.a);
}
`

class BlueInvertEffect extends Effect {
  constructor() {
    super('BlueInvertEffect', blueInvertFrag, {
      blendFunction: BlendFunction.NORMAL,
      uniforms: new Map([
        ['uBlue',    new THREE.Uniform(0)],
        ['uBlueHue', new THREE.Uniform(new THREE.Color(0.05, 0.16, 1.0))],
      ]),
    })
  }
  set blue(v) { this.uniforms.get('uBlue').value = v }
  setHue(r, g, b) { this.uniforms.get('uBlueHue').value.setRGB(r, g, b) }
}

// -------------------------------------------------------

export function createComposer(renderer, scene, camera) {
  const composer = new EffectComposer(renderer, {
    frameBufferType: THREE.HalfFloatType,
  })

  composer.addPass(new RenderPass(scene, camera))

  const bloom = new BloomEffect({
    intensity:           0.95,
    luminanceThreshold:  0.62,
    luminanceSmoothing:  0.22,
    mipmapBlur:          true,
    radius:              0.4,
  })

  const ca = new ChromaticAberrationEffect({
    offset:   new THREE.Vector2(0.0010, 0.0008),
    radialModulation: true,
    modulationOffset: 0.30,
  })

  const noise = new NoiseEffect({
    blendFunction: BlendFunction.OVERLAY,
    premultiply:   true,
  })
  noise.blendMode.opacity.value = 0.5

  const scanline = new ScanlineEffect({ density: 1.4 })
  scanline.blendMode.opacity.value = 0.08

  const vignette = new VignetteEffect({ darkness: 0.55, offset: 0.3 })

  const datamosh   = new DatamoshEffect()
  const blueInvert = new BlueInvertEffect()

  composer.addPass(new EffectPass(camera, bloom, ca))
  composer.addPass(new EffectPass(camera, datamosh))
  composer.addPass(new EffectPass(camera, noise, scanline, vignette, blueInvert))

  return {
    composer,
    effects: { bloom, ca, noise, scanline, vignette, datamosh, blueInvert },
  }
}

export function updatePost(effects, envelopes, time, params) {
  const { datamosh, blueInvert } = effects
  datamosh.glitch = envelopes.glitch
  datamosh.time   = time
  blueInvert.blue = envelopes.blue
  if (params) {
    datamosh.slice = params.dmSlice
    datamosh.block = params.dmBlock
    datamosh.smear = params.dmSmear
    datamosh.seam  = params.dmSeam
  }
}
