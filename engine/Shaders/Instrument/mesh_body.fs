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
    { "NAME": "hole", "TYPE": "float", "DEFAULT": 0.75, "MIN": 0.0, "MAX": 1.0 },
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

// A link between two plexus points. Everything about it (erosion, fade)
// depends only on the two endpoints (their fixed cell ids for erosion), so
// every cell that draws the same link draws it identically - no broken
// dashes at cell borders, no blinking while the points wander.
float link (vec2 q, vec2 a, vec2 b, vec2 ia, vec2 ib, float px, float erode, float seed)
{
    float len = length (a - b);
    float keep = step (erode, vjHash12 ((ia + ib) * 7.13 + abs (ia - ib) * 1.7 + seed));
    return (1.0 - smoothstep (0.0, px * 1.4, segment (q, a, b))) * smoothstep (1.9, 0.6, len) * keep;
}

// One plexus layer: lines from the centre cell's point to its 8 neighbours
// plus the four cross links, short lines bright, long lines faint.
float plexus (vec2 q, float t, float px, float erode, float seed)
{
    vec2 id = floor (q);
    vec2 pts[9];
    vec2 ids[9];
    int k = 0;
    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x)
        {
            ids[k] = id + vec2 (float (x), float (y));
            pts[k] = cellPoint (ids[k] + seed * 31.0, t) - seed * 31.0;
            ++k;
        }

    float v = 0.0;
    for (int i = 0; i < 9; ++i)
        if (i != 4)
            v = max (v, link (q, pts[4], pts[i], ids[4], ids[i], px, erode, seed));
    // cross links between the edge neighbours
    v = max (v, link (q, pts[1], pts[3], ids[1], ids[3], px, erode, seed));
    v = max (v, link (q, pts[1], pts[5], ids[1], ids[5], px, erode, seed));
    v = max (v, link (q, pts[7], pts[3], ids[7], ids[3], px, erode, seed));
    v = max (v, link (q, pts[7], pts[5], ids[7], ids[5], px, erode, seed));

    // the points themselves, glinting
    float glint = (1.0 - smoothstep (0.0, px * 2.5, length (q - pts[4]))) * 1.5;
    return v * 0.8 + glint;
}

void main()
{
    vec2 p = vjCentered();
    float t = breathe;
    float r = length (p);
    vec2 dir = r > 1e-4 ? p / r : vec2 (1.0, 0.0);

    // Silhouette: a lumpy, slowly deforming body (not a clean sphere), with a
    // bite taken out of one side that wanders round the rim.
    float lump = vjFbm (dir * 1.3 + t * 0.12) + 0.5 * vjFbm (dir * 3.1 - t * 0.09);
    float biteAngle = t * 0.11 + 2.0;
    float bite = smoothstep (0.55, 1.0, dot (dir, vec2 (cos (biteAngle), sin (biteAngle)))) * hole * 0.4;
    float radius = size * (1.0 + 0.06 * sin (t * 0.7) + 0.12 * surge) * (0.72 + 0.42 * lump) * (1.0 - bite);
    float inside = smoothstep (radius, radius - 0.04, r);
    // Long strands are allowed to poke out past the surface.
    float reach = smoothstep (radius * 1.22, radius * 0.9, r);
    float z = sqrt (max (radius * radius - r * r, 0.0)) / radius;   // 1 at the centre, 0 at the rim

    // One large, irregular opening drifting across the body (a single void -
    // two round ones read as eyes, and the body must stay abstract).
    vec2 hc = vec2 (sin (t * 0.21), cos (t * 0.17) * 0.8) * radius * 0.4;
    float wob = 0.12 * (vjFbm (p * 3.0 + t * 0.2) - 0.5);
    vec2 hp = (p - hc) * vec2 (1.0, 1.6 + 0.4 * sin (t * 0.3));          // stretched, not round
    float open1 = smoothstep (0.06, 0.16 + 0.2 * hole, length (hp) + wob);
    float holeMask = mix (1.0, open1, clamp (hole * 1.4, 0.0, 1.0));

    // Map onto the sphere so the mesh bunches toward the rim. The front
    // surface turns one way; the back surface (mirrored, dimmer) the other,
    // so the parallax between them reads as a volume.
    float bend = 0.35 + 0.65 * z;
    vec2 s = p / bend;
    vec2 front = vjRot (turn * 0.1) * s;
    vec2 back = vjRot (-turn * 0.1 + 1.3) * (s * vec2 (-1.0, 1.0));
    float px = density * 2.0 / RENDERSIZE.y / bend;

    float erode = erosion * 0.85;
    float mesh = plexus (front * density, t * 0.6, px, erode, 0.0);
    mesh += 0.55 * plexus (front * density * 1.9 + 7.0, t * 0.8, px * 1.9, erode, 1.0);
    mesh += 0.4 * plexus (back * density * 1.3 + 3.0, t * 0.5, px * 1.3, erode, 2.0);

    // Long strands: a coarse layer whose lines span several fine cells.
    float strands = plexus (front * density * 0.45 + 11.0, t * 0.4, px * 0.45, erode * 0.6, 3.0);

    // Rim glow and depth falloff: the far side is fainter.
    float shade = 0.35 + 0.65 * (1.0 - z) + 0.25 * z;
    float body = mesh * inside * holeMask * shade + strands * 0.45 * reach * holeMask;

    // Erosion scatters dust outward from the surface.
    float dustR = r - radius;
    float dust = step (0.985 - erosion * 0.04, vjHash12 (floor (gl_FragCoord.xy / 2.0) + floor (t * 8.0)))
               * smoothstep (0.35 * erosion + 0.02, 0.0, abs (dustR)) * erosion;

    float v = body + dust * 0.8 + inside * holeMask * 0.03;
    gl_FragColor = vec4 (vec3 (v * emission), 1.0);
}
