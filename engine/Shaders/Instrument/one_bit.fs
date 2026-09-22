/*{
  "ISFVSN": "2",
  "DESCRIPTION": "One bit: hard black/white threshold of the source with halftone-dithered edges, slow zoom/spin, colour speckle and an optional inverted letterbox frame.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "source", "TYPE": "image" },
    { "NAME": "threshold", "TYPE": "float", "DEFAULT": 0.45, "MIN": 0.1, "MAX": 0.9 },
    { "NAME": "dither", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "cell", "TYPE": "float", "DEFAULT": 5.0, "MIN": 2.0, "MAX": 14.0 },
    { "NAME": "zoom", "TYPE": "float", "DEFAULT": 1.3, "MIN": 0.5, "MAX": 3.5 },
    { "NAME": "spin", "TYPE": "float", "DEFAULT": 0.0, "MIN": -1000.0, "MAX": 1000.0 },
    { "NAME": "invert", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "frame", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "speckle", "TYPE": "float", "DEFAULT": 0.25, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "surge", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "texture_amt", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 }
  ]
}*/
#include "common.glsl"

// 4x4 Bayer matrix as a function (GLSL 1.20 friendly).
float bayer4 (vec2 px)
{
    vec2 a = mod (floor (px), 4.0);
    float i = a.x + a.y * 4.0;
    float v = 0.0;
    if      (i < 0.5)  v = 0.0;  else if (i < 1.5)  v = 8.0;  else if (i < 2.5)  v = 2.0;  else if (i < 3.5)  v = 10.0;
    else if (i < 4.5)  v = 12.0; else if (i < 5.5)  v = 4.0;  else if (i < 6.5)  v = 14.0; else if (i < 7.5)  v = 6.0;
    else if (i < 8.5)  v = 3.0;  else if (i < 9.5)  v = 11.0; else if (i < 10.5) v = 1.0;  else if (i < 11.5) v = 9.0;
    else if (i < 12.5) v = 15.0; else if (i < 13.5) v = 7.0;  else if (i < 14.5) v = 13.0; else v = 5.0;
    return (v + 0.5) / 16.0;
}

void main()
{
    vec2 p = vjCentered();
    vec2 q = vjRot (spin * 0.2) * p;
    q *= 1.0 - 0.08 * surge;                              // punch in on hits

    vec4 s = vjSampleFit (source, source_size, q, zoom);
    // Busy background texture so the threshold has "maze" detail around the source.
    float maze = 0.5 + 0.5 * sin (vjFbm (q * 3.5 + spin * 0.03) * 38.0);
    float tex = maze * texture_amt * 0.9;
    float lum = mix (tex * 0.8, vjLuma (s.rgb), s.a);

    float cellPx = cell * (RENDERSIZE.y / 1080.0);
    float screenDot = bayer4 (gl_FragCoord.xy / max (cellPx * 0.5, 1.0));
    float th = threshold - surge * 0.15;
    float bit = step (th + (screenDot - 0.5) * dither * 0.5, lum);
    bit = mix (bit, 1.0 - bit, step (0.5, invert));

    // Letterbox frame: white border, content inside an inset black window.
    vec2 uv = gl_FragCoord.xy / RENDERSIZE.xy;
    float win = step (0.12, uv.y) * step (uv.y, 0.88) * step (0.08, uv.x) * step (uv.x, 0.92);
    float border = (1.0 - win) * step (0.5, frame);
    float stripes = step (0.5, fract (gl_FragCoord.x / max (cellPx, 1.0))) * step (0.4, bayer4 (gl_FragCoord.xy / 3.0));
    float v = mix (bit, 1.0 - stripes * 0.9, border);

    vec3 col = vec3 (v);
    // Sparse colour speckle on the edges, like compression noise.
    float edge = abs (dFdx (bit)) + abs (dFdy (bit));
    float n = vjHash12 (floor (gl_FragCoord.xy / 2.0) + floor (spin * 10.0));
    col += edge * speckle * step (0.7, n) * vjPalette (n, 5.0) * 2.0;

    gl_FragColor = vec4 (col, 1.0);
}
