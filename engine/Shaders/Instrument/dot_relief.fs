/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Dot relief: the source rendered as a tilted grid of dots displaced by its brightness - a point-cloud / Rutt-Etra style relief in white dots on black.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "source", "TYPE": "image" },
    { "NAME": "grid", "TYPE": "float", "DEFAULT": 90.0, "MIN": 30.0, "MAX": 220.0 },
    { "NAME": "depth", "TYPE": "float", "DEFAULT": 0.18, "MIN": 0.0, "MAX": 0.6 },
    { "NAME": "tilt", "TYPE": "float", "DEFAULT": 0.35, "MIN": 0.0, "MAX": 0.8 },
    { "NAME": "dot_size", "TYPE": "float", "DEFAULT": 0.42, "MIN": 0.1, "MAX": 0.9 },
    { "NAME": "zoom", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.6, "MAX": 3.0 },
    { "NAME": "sway", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 },
    { "NAME": "surge", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "tint", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "hue", "TYPE": "float", "DEFAULT": 0.1, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "noise_floor", "TYPE": "float", "DEFAULT": 0.3, "MIN": 0.0, "MAX": 1.0 }
  ]
}*/
#include "common.glsl"

float heightAt (vec2 q)
{
    // Source luminance (masked by alpha) over a faint noise terrain, so the
    // grid never goes fully flat between images.
    vec4 s = vjSampleFit (source, source_size, q, zoom);
    float terrain = (0.6 * vjNoise (q * 3.0 + sway * 0.1) + 0.4 * vjNoise (q * 7.0 - sway * 0.07)) * noise_floor * 0.5;
    return max (vjLuma (s.rgb) * s.a, terrain);
}

void main()
{
    vec2 p = vjCentered();

    // Perspective tilt: rows compress toward the top of the frame.
    float persp = 1.0 + tilt * p.y;
    vec2 q = vec2 (p.x / persp, p.y * (1.0 + tilt * 0.5));
    q = vjRot (0.15 * sin (sway * 0.3)) * q;

    float cell = 2.0 / grid;
    vec3 col = vec3 (0.0);

    // Each output pixel checks the dots in the rows below it that could have
    // been lifted up into it by their height (enough rows for the max lift).
    int rowsBelow = int (min (28.0, ceil (depth * 1.8 / cell) + 1.0));
    for (int k = 0; k < 28; ++k)
    {
        if (k > rowsBelow)
            break;
        vec2 id = floor (q / cell) - vec2 (0.0, float (k));
        vec2 center = (id + 0.5) * cell;
        float h = heightAt (center);
        float lift = h * depth * (1.0 + 0.8 * surge);
        vec2 dotPos = center + vec2 (0.0, lift);
        float r = cell * dot_size * (0.35 + 0.65 * h) * (1.0 + 0.4 * surge);
        float d = length ((q - dotPos) * vec2 (1.0, 1.0)) - r;
        float aa = fwidth (q.y) * 1.2;
        float dotMask = 1.0 - smoothstep (-aa, aa, d);
        float bright = (0.25 + 0.9 * h) * dotMask;
        vec3 c = mix (vec3 (1.0), vjPalette (h + id.y * 0.01, hue * 5.99) * 1.4, tint);
        col = max (col, c * bright);
    }

    col *= 1.0 + surge * 0.8;
    gl_FragColor = vec4 (col, 1.0);
}
