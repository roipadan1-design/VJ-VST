/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Lumia (after Thomas Wilfred's lumia and the aurora curtain): two or three veils of light hang in the dark and fold slowly. Each veil has a bright hem, faint rays that rise from it and fade, and burns brightest where the sheet folds and is seen edge-on. Nothing is thresholded and nothing is hit: the veils drift, unfurl with the music's level and settle toward the horizon in silence. Ambient family.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "drift", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 },
    { "NAME": "sway", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 },
    { "NAME": "veils", "TYPE": "float", "DEFAULT": 2.0, "MIN": 1.0, "MAX": 3.0 },
    { "NAME": "horizon", "TYPE": "float", "DEFAULT": -0.3, "MIN": -0.8, "MAX": 0.4 },
    { "NAME": "height", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.1, "MAX": 1.4 },
    { "NAME": "fold", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 1.5 },
    { "NAME": "lift", "TYPE": "float", "DEFAULT": 0.2, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "rays", "TYPE": "float", "DEFAULT": 0.55, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "fineness", "TYPE": "float", "DEFAULT": 30.0, "MIN": 6.0, "MAX": 80.0 },
    { "NAME": "diffuse", "TYPE": "float", "DEFAULT": 0.2, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "shimmer", "TYPE": "float", "DEFAULT": 0.2, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "swell", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.5 },
    { "NAME": "haze", "TYPE": "float", "DEFAULT": 0.25, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "tint", "TYPE": "float", "DEFAULT": 0.35, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.0, "MIN": 0.2, "MAX": 3.0 }
  ]
}*/
#include "common.glsl"

// Each veil is a vertical sheet of light seen from below, like an aurora
// curtain or one of Wilfred's slow skeins. x is folded into u, the position
// along the sheet; where the fold compresses u (du/dx -> 0) the sheet is seen
// edge-on and the light piles up into a bright vertical band. The hem is a
// slow curve in u; rays hang along the field direction (vertical) and fade
// upward. Cost: ~11 value-noise taps per veil, 2-3 veils, plus 2 for the haze.

float foldAt (float x, float t, float o)
{
    return x + fold * (0.62 * (vjNoise (vec2 (x * 0.85 + o, t * 0.33 + o)) - 0.5)
                     + 0.30 * (vjNoise (vec2 (x * 2.3 - o, t * 0.57 + 1.7)) - 0.5));
}

void main()
{
    vec2 p = vjCentered();
    float t = drift;

    float grow = 1.0 + 0.45 * swell;                    // a hit unfurls the rays, slowly
    float amp = 0.10 + 0.28 * lift;                     // hem undulation (the bass)
    float hemSoft = mix (0.014, 0.16, diffuse);         // lower border: crisp -> dissolved

    // Own colours (only seen with the plug-in's "Scene Colors" palette; any
    // other palette maps the luminance): pale hem, darker cooler top.
    vec3 hemCol = mix (vec3 (0.55, 0.95, 0.82), vec3 (1.00, 0.80, 0.55), tint);
    vec3 topCol = mix (vec3 (0.30, 0.36, 0.95), vec3 (0.80, 0.34, 0.42), tint);

    vec3 col = vec3 (0.0);
    for (int i = 0; i < 3; ++i)
    {
        float fi = float (i);
        float w = clamp (veils - fi, 0.0, 1.0);          // the third veil fades in with "veils"
        if (w <= 0.001)
            break;

        float o = fi * 17.31 + 3.7;
        float depth = 1.0 - 0.22 * fi;                  // farther veils: dimmer, shorter, higher
        float x = p.x * (0.8 + 0.25 * fi) + fi * 1.9;
        float tv = t * (1.0 + 0.23 * fi);

        float u = foldAt (x, tv, o);
        float dudx = (foldAt (x + 0.004, tv, o) - u) / 0.004;
        float edgeOn = clamp (1.0 / max (abs (dudx), 0.3), 0.0, 3.0);
        float face = mix (0.5, 1.0, smoothstep (-0.25, 0.25, dudx)); // the back of a fold is dimmer

        float hemY = horizon + 0.24 * fi
                   + amp * ((vjNoise (vec2 (u * 1.1 + o, tv * 0.42)) - 0.5) * 1.6
                          + (vjNoise (vec2 (u * 2.9 - o, tv * 0.71 + 5.0)) - 0.5) * 0.6);
        // The fold also runs in depth: seen from below, the part of the sheet
        // that swings away sits higher, so the hem draws the drapery.
        hemY += 0.45 * (u - x);
        float d = p.y - hemY;                           // > 0 above the hem

        // Rays: streaks along the field lines, travelling slowly along the sheet.
        float rr = 0.65 * vjNoise (vec2 (u * fineness + sway * 3.0 + o, p.y * 0.7 + tv * 0.05))
                 + 0.35 * vjNoise (vec2 (u * fineness * 2.3 - sway * 2.0, p.y * 1.3 - tv * 0.08 + o));
        float H = height * depth * grow * (0.45 + 0.9 * vjNoise (vec2 (u * 3.1 + o, tv * 0.2)));
        float body = exp (-max (d, 0.0) / H) * exp (min (d, 0.0) / hemSoft);
        float hem = exp (-max (d, 0.0) / (hemSoft * 2.5 + 0.02)) * exp (min (d, 0.0) / (hemSoft + 0.004));
        float ray = mix (1.0, 0.05 + 3.2 * rr * rr * rr, rays);
        if (shimmer > 0.001)
            ray *= mix (1.0, 0.55 + 0.9 * vjNoise (vec2 (u * fineness * 0.45 + o, tv * 2.5)), shimmer);

        // Where along its length the veil exists at all: most of the sky stays dark.
        float span = smoothstep (0.45, 0.85, vjNoise (vec2 (u * 0.6 + o * 0.7, tv * 0.11 + fi * 3.0)));

        // Erode: the sheet tears into wisps.
        float wisp = 1.0;
        if (diffuse > 0.001)
            wisp = mix (1.0, smoothstep (0.3, 0.72, vjNoise (vec2 (u * 1.4 - o, p.y * 2.2 - tv * 0.3))), diffuse);

        float I = 0.3 * w * depth * (body * ray + hem * 0.6) * (0.35 + 0.65 * edgeOn * sqrt (edgeOn)) * face * span * wisp;
        float k = clamp (max (d, 0.0) / H, 0.0, 1.0);
        col += mix (hemCol, topCol, smoothstep (0.0, 0.8, k)) * I;
    }

    // Haze: a little of the veils' light scattered in the air around them
    // (the fogged room of a drone concert), never more than a glow.
    float hz = 0.6 * vjNoise (p * vec2 (0.9, 1.6) + vec2 (t * 0.07, 0.0))
             + 0.4 * vjNoise (p * vec2 (2.1, 3.3) - vec2 (t * 0.05, t * 0.02));
    float band = exp (-abs (p.y - horizon - 0.15) * 1.4);
    col += mix (hemCol, topCol, 0.5) * haze * 0.03 * hz * band * (0.6 + 0.2 * veils);

    col = mix (vec3 (vjLuma (col)), col, 0.8);          // keep it muted
    col *= emission * (1.0 + 0.5 * swell);
    gl_FragColor = vec4 (col + vec3 (0.0005), 1.0);
}
