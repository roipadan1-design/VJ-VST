/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Membrane (after the un_source reel): a crumpled sheet made of thousands of points, seen at a tilt. Where it folds the points pile up into bright ridges; depth of field keeps a band in focus and melts the rest into soft bokeh; a pale haze drifts behind. Monochrome, slow.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "morph", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 },
    { "NAME": "pan", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 },
    { "NAME": "zoom", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.5, "MAX": 3.0 },
    { "NAME": "depth", "TYPE": "float", "DEFAULT": 0.25, "MIN": 0.0, "MAX": 0.6 },
    { "NAME": "grid", "TYPE": "float", "DEFAULT": 150.0, "MIN": 60.0, "MAX": 260.0 },
    { "NAME": "fold", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "sparse", "TYPE": "float", "DEFAULT": 0.2, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "focus", "TYPE": "float", "DEFAULT": 0.45, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "dof", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "haze", "TYPE": "float", "DEFAULT": 0.35, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "sparkle", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "surge", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.2, "MAX": 4.0 }
  ]
}*/
#include "common.glsl"

// Height of the crumpled sheet: domain-warped ridged noise (folds).
float sheet (vec2 x)
{
    vec2 w = vec2 (vjFbm (x * 0.6 + vec2 (morph * 0.05, 0.0)), vjFbm (x * 0.6 + vec2 (3.1, morph * 0.04))) - 0.5;
    float n = vjFbm (x + 1.8 * w);
    float r = 1.0 - abs (2.0 * n - 1.0);
    return pow (r, mix (1.0, 5.0, fold));
}

void main()
{
    vec2 p = vjCentered();
    float px = 2.0 / RENDERSIZE.y;

    // Tilted plane: rows compress toward the top (farther away).
    float tilt = 0.55;
    float persp = 1.0 + tilt * p.y;
    vec2 q = vec2 (p.x / persp, p.y * (1.0 + tilt * 0.5));
    vec2 sq = (q + vec2 (pan * 0.06, pan * 0.02)) * zoom;

    // Height and slope once per pixel; neighbours use the tangent plane.
    float e = 0.01;
    float h = sheet (sq);
    vec2 grad = vec2 (sheet (sq + vec2 (e, 0.0)) - h, sheet (sq + vec2 (0.0, e)) - h) / e;
    float amp = depth * (1.0 + 0.6 * surge) / zoom;
    float steep = clamp (length (grad) * 0.12, 0.0, 1.0);          // folds seen edge-on

    // Points: one jittered point per cell of the sheet, lifted by its height.
    float cs = 2.0 / grid;
    vec2 base = q - vec2 (0.0, h * amp);                            // approximate inverse of the lift
    vec2 id = floor (base / cs);
    float focusY = focus * 2.0 - 1.0;
    float defocus = abs (p.y - focusY) * dof;
    float radius = cs * 0.1 + defocus * cs * 1.4;                   // bokeh grows away from the focus band
    float weight = 1.0 / (1.0 + pow (radius / max (px, 1e-4), 2.0) * 0.25);

    float v = 0.0;
    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x)
        {
            vec2 cid = id + vec2 (float (x), float (y));
            vec2 jit = vjHash22 (cid);
            // Flat stretches thin out; the folds keep every point.
            float keep = step (mix (0.35, 0.97, sparse) * (1.0 - steep * 0.8), vjHash12 (cid * 1.7 + 3.0));
            vec2 pt = (cid + jit) * cs;
            float hp = h + dot (grad, (pt - base) * zoom);
            vec2 at = pt + vec2 (0.0, hp * amp);
            float d = length (q - at);
            float lit = 0.06 + 1.9 * steep * steep + 0.2 * hp;
            float glint = 1.0 + sparkle * 3.0 * step (0.985, vjHash12 (cid + floor (morph * 3.0)));
            v += exp (-d * d / max (radius * radius, 1e-8)) * lit * keep * glint;
        }
    v *= weight;

    // The membrane is a torn piece floating in black, not a floor: a soft,
    // ragged silhouette that drifts with the sheet.
    vec2 c = p * vec2 (0.62, 0.95) + 0.25 * vec2 (sin (morph * 0.21), cos (morph * 0.17));
    float edge = length (c) + 0.35 * (vjFbm (p * 1.6 + morph * 0.05) - 0.5);
    v *= smoothstep (1.0, 0.55, edge);

    // Haze: soft pale smoke drifting behind the sheet.
    float fog = vjFbm (q * 1.3 + vec2 (morph * 0.03, pan * 0.01)) * vjFbm (q * 0.7 - morph * 0.02);
    v += haze * smoothstep (0.25, 0.6, fog) * 0.18 * smoothstep (1.3, 0.3, edge);

    gl_FragColor = vec4 (vec3 (v * emission), 1.0);
}
