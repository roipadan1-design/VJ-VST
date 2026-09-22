/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Original Pulse Halo starter. Linear HDR output; host supplies ISF uniforms. Not GPU-compiled in this research.",
  "CATEGORIES": [
    "Generator"
  ],
  "INPUTS": [
    {
      "NAME": "radius",
      "TYPE": "float",
      "DEFAULT": 0.45,
      "MIN": 0.1,
      "MAX": 0.9
    },
    {
      "NAME": "thickness",
      "TYPE": "float",
      "DEFAULT": 0.012,
      "MIN": 0.002,
      "MAX": 0.05
    },
    {
      "NAME": "emission",
      "TYPE": "float",
      "DEFAULT": 1.4,
      "MIN": 0.1,
      "MAX": 8
    },
    {
      "NAME": "glow",
      "TYPE": "float",
      "DEFAULT": 0.3,
      "MIN": 0,
      "MAX": 1
    },
    {
      "NAME": "hue",
      "TYPE": "float",
      "DEFAULT": 0.55,
      "MIN": 0,
      "MAX": 1
    },
    {
      "NAME": "drift",
      "TYPE": "float",
      "DEFAULT": 0.12,
      "MIN": 0,
      "MAX": 1
    },
    {
      "NAME": "beat_pulse",
      "TYPE": "float",
      "DEFAULT": 0,
      "MIN": 0,
      "MAX": 1
    },
    {
      "NAME": "shape_warp",
      "TYPE": "float",
      "DEFAULT": 0.025,
      "MIN": 0,
      "MAX": 0.2
    }
  ]
}*/
// Original implementation; requires host-provided TIME and RENDERSIZE.
// External graph handles bloom, tone mapping and output conversion.
void main() {
    vec2 p = (2.0 * gl_FragCoord.xy - RENDERSIZE.xy) / max(RENDERSIZE.y, 1.0);
    float a = atan(p.y, p.x);
    float wander = shape_warp * drift * sin(3.0 * a + 0.30 * TIME);
    float r = max(0.02, radius + 0.045 * beat_pulse + wander);
    float d = abs(length(p) - r) - thickness;
    float aa = max(fwidth(d), 1.0 / max(RENDERSIZE.y, 1.0));
    float edge = 1.0 - smoothstep(-aa, aa, d);
    float haze = exp(-max(d, 0.0) * 48.0);
    vec3 phase = vec3(0.00, 0.18, 0.42);
    vec3 accent = 0.52 + 0.48 * cos(6.28318530718 * (hue + phase));
    vec3 background = vec3(0.0008, 0.0012, 0.0020);
    float light = edge * emission * (1.0 + 0.45 * beat_pulse) + haze * glow;
    gl_FragColor = vec4(background + max(accent, vec3(0.0)) * light, 1.0);
}
