/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Fibers: a dense web of thin filaments - roots, nerves, torn cloth - in layers of different depth around a dark knot. Bass swells the web, kicks tear it outward from the knot, the whole mass drifts and slowly turns.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "drift", "TYPE": "float", "DEFAULT": 0.0, "MIN": -1000.0, "MAX": 1000.0 },
    { "NAME": "turn", "TYPE": "float", "DEFAULT": 0.0, "MIN": -1000.0, "MAX": 1000.0 },
    { "NAME": "zoom", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.5, "MAX": 3.0 },
    { "NAME": "density", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "thickness", "TYPE": "float", "DEFAULT": 0.4, "MIN": 0.05, "MAX": 1.0 },
    { "NAME": "tear", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "swell", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "knot", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.3, "MIN": 0.2, "MAX": 4.0 }
  ]
}*/
#include "common.glsl"

// One layer of filaments: ridges of a domain-warped noise, stretched along a
// direction so they read as fibres rather than cells.
float fiberLayer (vec2 p, float seed, float width)
{
    vec2 dir = vec2 (cos (seed * 2.4), sin (seed * 2.4));
    vec2 q = vec2 (dot (p, dir), dot (p, vec2 (-dir.y, dir.x)));
    q *= vec2 (0.35, 2.2);
    vec2 w = vec2 (vjFbm (q * 0.8 + seed * 3.1 + drift * 0.05), vjFbm (q * 0.8 - seed * 1.7 - drift * 0.04));
    float n = vjNoise (q * 2.0 + w * 3.0 + seed * 11.0);
    float ridge = 1.0 - abs (n * 2.0 - 1.0);
    return pow (ridge, mix (90.0, 12.0, width));
}

void main()
{
    vec2 p = vjCentered() / zoom;
    p = vjRot (turn * 0.05) * p;

    // Kicks tear the web outward from the knot.
    float r = length (p);
    vec2 radial = r > 1e-4 ? p / r : vec2 (0.0);
    p -= radial * tear * 0.25 * exp (-r * 1.5);
    p *= 1.0 - 0.1 * swell;

    float v = 0.0;
    int layers = int (3.0 + density * 4.0);
    for (int i = 0; i < 7; ++i)
    {
        if (i >= layers)
            break;
        float fi = float (i);
        float depth = 1.0 / (1.0 + fi * 0.35);
        float layer = fiberLayer (p * (1.0 + fi * 0.45), fi + 1.0, thickness * depth);
        v += layer * mix (0.35, 1.0, depth);
    }

    // Soft glow of the mass + the dark knot it grows from.
    float body = vjFbm (p * 2.0 + drift * 0.03) * 0.35;
    v = v + body * body * (1.0 + swell);
    float hole = smoothstep (0.55, 0.0, length (p * vec2 (1.4, 2.4) + vec2 (0.05, 0.0) + 0.15 * (vjFbm (p * 3.0) - 0.5)));
    v *= 1.0 - hole * knot * 0.95;
    v *= 1.0 + tear * 0.6;

    vec3 col = vec3 (1.0, 0.35, 0.25) * v * emission;
    gl_FragColor = vec4 (col, 1.0);
}
