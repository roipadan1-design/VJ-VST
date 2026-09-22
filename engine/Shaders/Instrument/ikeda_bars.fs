/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Ikeda bars: minimal white vertical lines on black that re-arrange on the beat, with horizontal code bars and a hard invert on accents.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "density", "TYPE": "float", "DEFAULT": 0.35, "MIN": 0.05, "MAX": 0.9 },
    { "NAME": "columns", "TYPE": "float", "DEFAULT": 48.0, "MIN": 8.0, "MAX": 200.0 },
    { "NAME": "thickness", "TYPE": "float", "DEFAULT": 0.18, "MIN": 0.05, "MAX": 0.9 },
    { "NAME": "step_rate", "TYPE": "float", "DEFAULT": 2.0, "MIN": 0.25, "MAX": 8.0 },
    { "NAME": "code", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "flash", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "brightness", "TYPE": "float", "DEFAULT": 1.6, "MIN": 0.2, "MAX": 5.0 },
    { "NAME": "scan", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 }
  ]
}*/
#include "common.glsl"

void main()
{
    vec2 uv = gl_FragCoord.xy / RENDERSIZE.xy;
    float stepIndex = floor (vj_beat * step_rate);

    // Column layout changes on every step.
    float col = floor (uv.x * columns);
    float on = step (1.0 - density, vjHash12 (vec2 (col, stepIndex)));
    float within = fract (uv.x * columns);
    float line = on * step (abs (within - 0.5), thickness * 0.5);

    // Lines have random vertical extents, like a waveform printout.
    float top = mix (0.55, 1.0, vjHash12 (vec2 (col, stepIndex + 11.0)));
    float bottom = mix (0.0, 0.45, vjHash12 (vec2 (col, stepIndex + 23.0)));
    line *= step (bottom, uv.y) * step (uv.y, top);

    // Code bars: thin horizontal segments on hits.
    float band = floor (uv.y * 36.0);
    float barOn = step (1.0 - code * 0.6, vjHash12 (vec2 (band, stepIndex + 5.0)));
    float barX = vjHash12 (vec2 (band, stepIndex + 9.0));
    float bar = barOn * step (abs (uv.x - barX), 0.12 + 0.2 * code) * step (fract (uv.y * 36.0), 0.35);

    // A slow scanning sliver for texture between steps.
    float sliver = step (abs (fract (uv.x - scan * 0.05) - 0.5), 0.0015) * 0.35;

    float v = clamp (line + bar + sliver, 0.0, 1.0);
    v = mix (v, 1.0 - v, step (0.6, flash));          // hard invert on accents
    gl_FragColor = vec4 (vec3 (v) * brightness, 1.0);
}
