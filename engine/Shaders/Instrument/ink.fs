/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Ink: slow blots of ink or blood soaking through paper, thresholded hard with torn edges; dot-matrix holes punched through the dark, horizontal smear bands dragged across it. Kicks swell the blots and rip a band sideways.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "morph", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 },
    { "NAME": "fall", "TYPE": "float", "DEFAULT": 0.0, "MIN": -1000.0, "MAX": 1000.0 },
    { "NAME": "scale", "TYPE": "float", "DEFAULT": 1.6, "MIN": 0.5, "MAX": 5.0 },
    { "NAME": "coverage", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.2, "MAX": 0.8 },
    { "NAME": "swell", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "smear", "TYPE": "float", "DEFAULT": 0.3, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "rip", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "dots", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.2, "MAX": 4.0 }
  ]
}*/
#include "common.glsl"

void main()
{
    vec2 p = vjCentered();
    vec2 uv = gl_FragCoord.xy / RENDERSIZE.xy;

    // Smear bands: rows dragged horizontally, more on a kick.
    float bandId = floor (uv.y * 24.0);
    float bandOn = step (1.0 - smear * 0.35 - rip * 0.5, vjHash12 (vec2 (bandId, floor (morph * 0.5 + rip * 3.0))));
    float drag = bandOn * (vjHash12 (vec2 (bandId, 2.0)) - 0.5) * (0.3 + 1.2 * rip);
    vec2 q = vec2 (p.x * mix (1.0, 0.12, bandOn * smear) + drag, p.y);

    vec2 sp = q * scale + vec2 (0.0, fall * 0.2);
    vec2 warp = vec2 (vjFbm (sp * 0.7 + morph * 0.03), vjFbm (sp * 0.7 + vec2 (5.2, 1.3) - morph * 0.02));
    float f = vjFbm (sp + 2.2 * warp);
    // Paper fibre: tiny noise on the threshold gives torn, bleeding edges.
    float fibre = vjNoise (gl_FragCoord.xy * 0.35) * 0.06 + vjNoise (gl_FragCoord.xy * 0.08) * 0.05;
    float th = 1.0 - coverage - swell * 0.08;
    float inkMask = step (th, f + fibre);
    // Soaked halo: a lighter wet ring around every blot.
    float halo = smoothstep (th - 0.12, th, f + fibre) * (1.0 - inkMask);

    // Dot-matrix holes punched through the ink, in clusters.
    float cell = RENDERSIZE.y / 70.0;
    vec2 id = floor (gl_FragCoord.xy / cell);
    vec2 fc = fract (gl_FragCoord.xy / cell) - 0.5;
    float cluster = step (1.0 - dots * 0.5, vjHash12 (floor (id / vec2 (6.0, 3.0)) + floor (morph * 0.2)));
    float hole = (1.0 - smoothstep (0.22, 0.3, length (fc))) * cluster * step (0.4, vjHash12 (id));

    float v = inkMask * (0.32 + 0.35 * f) + halo * 0.08;
    v = mix (v, 0.0, hole * inkMask);
    v *= 1.0 + swell * 0.5;

    vec3 c = vec3 (1.0, 0.35, 0.25) * v * emission;
    gl_FragColor = vec4 (c, 1.0);
}
