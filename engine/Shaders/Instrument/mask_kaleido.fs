/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Mask kaleido: the source repeated and mirrored across a tile grid over a wavy duotone field, solarised chrome, each tile turning on its own phase.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "source", "TYPE": "image" },
    { "NAME": "tiles", "TYPE": "float", "DEFAULT": 3.0, "MIN": 1.0, "MAX": 8.0 },
    { "NAME": "wave", "TYPE": "float", "DEFAULT": 0.35, "MIN": 0.0, "MAX": 1.5 },
    { "NAME": "flow", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 },
    { "NAME": "solarize", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "turn", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "hue", "TYPE": "float", "DEFAULT": 0.55, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "fill", "TYPE": "float", "DEFAULT": 0.85, "MIN": 0.4, "MAX": 1.4 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.1, "MIN": 0.3, "MAX": 4.0 }
  ]
}*/
#include "common.glsl"

void main()
{
    vec2 p = vjCentered();

    // Duotone wavy field behind everything.
    float field = sin (p.y * 4.0 + vjFbm (p * 1.5 + flow * 0.2) * 5.0 * wave + flow);
    vec3 dark = vjPalette (0.7, 0.0 + hue * 5.99) * 0.08;
    vec3 light = vjToLinear (vec3 (0.82, 0.92, 0.84));
    vec3 col = mix (dark, light * 0.8, smoothstep (-0.2, 0.2, field));

    // Tile space with mirroring on alternate cells.
    vec2 g = p * tiles * 0.5 + vec2 (flow * 0.05, 0.0);
    vec2 id = floor (g);
    vec2 f = fract (g) - 0.5;
    f.x *= mod (id.x, 2.0) < 0.5 ? 1.0 : -1.0;
    float phase = vjHash12 (id) * VJ_TAU;
    f = vjRot (turn * sin (flow * 0.5 + phase) * 0.9) * f;
    f.y += 0.06 * wave * sin (flow + id.x * 1.7);

    vec4 s = vjSampleFit (source, source_size, f * 2.0, 1.0 / fill);
    float l = vjLuma (s.rgb);
    // Solarise: fold the brightness back on itself for the "negative chrome" look.
    float sol = mix (l, abs (l * 2.0 - 1.0), solarize);
    vec3 m = mix (dark * 3.0, light * 1.3, sol);
    m += vjPalette (l + 0.3, 2.0) * 0.25 * step (0.85, l);    // occasional acid glints

    col = mix (col, m, s.a);
    // Dark outline around each tiled figure.
    float outline = s.a * (1.0 - smoothstep (0.0, 0.25, vjSampleFit (source, source_size, f * 2.0 * 1.04, 1.0 / fill).a));
    col *= 1.0 - outline * 0.9;

    gl_FragColor = vec4 (col * emission, 1.0);
}
