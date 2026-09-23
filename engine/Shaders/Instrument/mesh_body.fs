/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Mesh body (after The Noise Diary): a breathing sphere-like body woven from thousands of fine plexus lines, with openings that drift across it. Erosion (build-ups) breaks the lines and scatters the points into dust; in breakdowns the body re-knits.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "turn", "TYPE": "float", "DEFAULT": 0.0, "MIN": -100000.0, "MAX": 100000.0 },
    { "NAME": "breathe", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 },
    { "NAME": "size", "TYPE": "float", "DEFAULT": 0.62, "MIN": 0.3, "MAX": 1.1 },
    { "NAME": "density", "TYPE": "float", "DEFAULT": 11.0, "MIN": 5.0, "MAX": 22.0 },
    { "NAME": "erosion", "TYPE": "float", "DEFAULT": 0.15, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "hole", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "surge", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.2, "MAX": 4.0 }
  ]
}*/
#include "common.glsl"

float segment (vec2 p, vec2 a, vec2 b)
{
    vec2 pa = p - a, ba = b - a;
    float h = clamp (dot (pa, ba) / dot (ba, ba), 0.0, 1.0);
    return length (pa - ba * h);
}

// Jittered point of a plexus cell, wandering slowly.
vec2 cellPoint (vec2 id, float t)
{
    vec2 h = vjHash22 (id);
    return id + 0.5 + 0.38 * sin (h * 6.2831 + t * (0.3 + 0.5 * h.yx));
}

// One plexus layer: lines from the centre cell's point to its 8 neighbours
// plus the four cross links, short lines bright, long lines faint.
float plexus (vec2 q, float t, float px, float erode, float seed)
{
    vec2 id = floor (q);
    vec2 pts[9];
    int k = 0;
    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x)
            pts[k++] = cellPoint (id + vec2 (float (x), float (y)) + seed * 31.0, t);

    float v = 0.0;
    for (int i = 0; i < 9; ++i)
    {
        if (i == 4)
            continue;
        float len = length (pts[4] - pts[i]);
        float keep = step (erode, vjHash12 (floor (pts[4] + pts[i]) * 7.13 + seed));
        float d = segment (q, pts[4], pts[i]);
        v += (1.0 - smoothstep (0.0, px * 1.4, d)) * smoothstep (1.9, 0.6, len) * keep;
    }
    // cross links between the edge neighbours
    float c1 = segment (q, pts[1], pts[3]), c2 = segment (q, pts[1], pts[5]);
    float c3 = segment (q, pts[7], pts[3]), c4 = segment (q, pts[7], pts[5]);
    float cross = (1.0 - smoothstep (0.0, px * 1.4, min (min (c1, c2), min (c3, c4))));
    v += cross * 0.6 * step (erode, vjHash12 (id * 3.7 + seed));

    // the points themselves, glinting
    float glint = (1.0 - smoothstep (0.0, px * 2.5, length (q - pts[4]))) * 1.5;
    return v * 0.55 + glint;
}

void main()
{
    vec2 p = vjCentered();
    float t = breathe;
    float r = length (p);
    vec2 dir = r > 1e-4 ? p / r : vec2 (1.0, 0.0);

    // Silhouette: a sphere whose radius is displaced by slow noise.
    float radius = size * (1.0 + 0.06 * sin (t * 0.7) + 0.12 * surge) * (0.88 + 0.24 * vjFbm (dir * 1.3 + t * 0.12));
    float inside = smoothstep (radius, radius - 0.04, r);
    float z = sqrt (max (radius * radius - r * r, 0.0)) / radius;   // 1 at the centre, 0 at the rim

    // Openings drifting across the body.
    vec2 hc = vec2 (sin (t * 0.21), cos (t * 0.17)) * radius * 0.45;
    float holeMask = smoothstep (0.08, 0.22 + 0.15 * (1.0 - hole), length (p - hc) + 0.06 * vjFbm (p * 4.0 + t * 0.2));
    holeMask = mix (1.0, holeMask, hole);

    // Map onto the sphere so the mesh bunches toward the rim, then spin.
    vec2 s = p / (0.35 + 0.65 * z);
    s = vjRot (turn * 0.1) * s;
    float px = density * 2.0 / RENDERSIZE.y / (0.35 + 0.65 * z);

    float erode = erosion * 0.85;
    float mesh = plexus (s * density, t * 0.6, px, erode, 0.0);
    mesh += 0.6 * plexus (s * density * 1.9 + 7.0, t * 0.8, px * 1.9, erode, 1.0);

    // Rim glow and depth falloff: the far side is fainter.
    float shade = 0.35 + 0.65 * (1.0 - z) + 0.25 * z;
    float body = mesh * inside * holeMask * shade;

    // Erosion scatters dust outward from the surface.
    float dustR = r - radius;
    float dust = step (0.985 - erosion * 0.04, vjHash12 (floor (gl_FragCoord.xy / 2.0) + floor (t * 8.0)))
               * smoothstep (0.35 * erosion + 0.02, 0.0, abs (dustR)) * erosion;

    float v = body + dust * 0.8 + inside * holeMask * 0.03;
    gl_FragColor = vec4 (vec3 (v * emission), 1.0);
}
