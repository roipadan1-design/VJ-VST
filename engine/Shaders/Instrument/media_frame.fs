/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Media frame: the performer's loaded image, fitted (contain or cover) with a slow drift - breathing zoom and pan on the scene clock - and a small punch-in on hits. Meant to feed a treatment stage.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "source", "TYPE": "image" },
    { "NAME": "zoom", "TYPE": "float", "DEFAULT": 1.0, "MIN": 0.5, "MAX": 3.0 },
    { "NAME": "fit", "TYPE": "float", "DEFAULT": 1.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "wander", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 },
    { "NAME": "roam", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "surge", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "autolevel", "TYPE": "float", "DEFAULT": 1.0, "MIN": 0.0, "MAX": 1.0 }
  ]
}*/
#include "common.glsl"

void main()
{
    vec2 p = vjCentered();
    float sa = RENDERSIZE.x / max (RENDERSIZE.y, 1.0);
    float ia = source_size.x / max (source_size.y, 1.0);

    // Ken Burns: a slow breathing zoom and pan (wander = integrated rate).
    float t = wander;
    float z = zoom * (1.0 + roam * 0.12 * (0.5 + 0.5 * sin (t * 0.23))) * (1.0 + 0.06 * surge);
    vec2 pan = roam * 0.1 * vec2 (sin (t * 0.17), cos (t * 0.13)) / z;

    // contain (fit 0) .. cover (fit 1): half-height of the image in screen units.
    float k = mix (min (1.0, sa / ia), max (1.0, sa / ia), fit) * z;
    vec2 uv = (p / (vec2 (ia, 1.0) * k) + pan) * 0.5 + 0.5;
    float inside = step (0.0, uv.x) * step (uv.x, 1.0) * step (0.0, uv.y) * step (uv.y, 1.0);
    vec4 c = texture2D (source, clamp (uv, 0.0, 1.0));

    // Auto-level: the smallest mip level is the image's average. An adaptive
    // curve x / (x + avg) puts that average at mid grey, so dark and bright
    // photos alike split around the middle for the treatments.
    vec4 mean = texture2D (source, vec2 (0.5), 14.0);
    float avg = clamp (vjLuma (mean.rgb) / max (mean.a, 0.05), 0.02, 0.9);
    vec3 levelled = c.rgb / (c.rgb + vec3 (avg));
    vec3 rgb = clamp (mix (c.rgb, levelled, autolevel), 0.0, 1.0);

    // Alpha = where the image is: treatments keep everything outside it black
    // (a negative would otherwise turn the empty border into solid colour).
    float a = c.a * inside;
    gl_FragColor = vec4 (rgb * a, a);
}
