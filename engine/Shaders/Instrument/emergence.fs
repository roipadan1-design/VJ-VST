/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Emergence (after The Noise Diary's heron): an abstract form rises out of total darkness, its highlights smeared into vertical light streaks; as the music builds it dissolves from the edges into drifting white dust, and re-forms in the quiet.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "source", "TYPE": "image" },
    { "NAME": "zoom", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.6, "MAX": 3.0 },
    { "NAME": "visible", "TYPE": "float", "DEFAULT": 0.8, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "streak", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "dissolve", "TYPE": "float", "DEFAULT": 0.15, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "drift", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 },
    { "NAME": "surge", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.2, "MAX": 4.0 },
    { "NAME": "fine", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 }
  ]
}*/
#include "common.glsl"

float formAt (vec2 q)
{
    vec4 s = vjSampleFit (source, source_size, q, zoom);
    // Contrast: mids sink into the dark, only the highlights stay lit.
    return pow (vjLuma (s.rgb) * s.a, 1.8);
}

void main()
{
    vec2 p = vjCentered();
    // A slow rise: the form floats up a little as it appears.
    vec2 q = p + vec2 (0.0, -0.06 * (1.0 - visible));
    // The form floats: a slow breath and sway on the scene clock.
    q = q * (1.0 + 0.05 * sin (drift * 0.35)) + 0.035 * vec2 (sin (drift * 0.23), cos (drift * 0.19));
    float base = formAt (q);

    // Vertical light streaks from the highlights (both directions, fading).
    float streaks = 0.0;
    float stepY = 0.012 * (0.4 + streak);
    for (int k = 1; k <= 12; ++k)
    {
        float w = exp (-float (k) / 5.0);
        float up = formAt (q + vec2 (0.0, stepY * float (k)));
        float dn = formAt (q - vec2 (0.0, stepY * float (k)));
        streaks += (max (up - 0.4, 0.0) + max (dn - 0.4, 0.0)) * w;
    }
    // Break the streaks into columns so they read as light, not blur.
    float column = 0.6 + 0.4 * vjNoise (vec2 (gl_FragCoord.x * 0.35, drift * 0.3));
    streaks *= streak * column * 0.45;

    // Dissolve: the form erodes from its darker, outer parts first.
    float grainN = vjNoise (p * mix (40.0, 170.0, fine) + drift * 0.05) * 0.6 + vjHash12 (floor (gl_FragCoord.xy / 1.5)) * 0.4;
    float keep = step (dissolve * 1.15, grainN * (0.35 + 0.65 * base) + base * 0.4);
    float body = base * keep * 0.85;

    // Dust: eroded material lifting off as fine motes (24 fps flutter).
    float frame = floor (vj_time * 24.0);
    vec2 lift = vec2 (0.02 * sin (drift * 0.7 + p.y * 9.0), 0.08 * dissolve * fract (drift * 0.05 + vjHash12 (floor (p * 30.0))));
    float srcBelow = formAt (q - lift);
    float mote = step (0.985 - 0.02 * dissolve, vjHash12 (floor (gl_FragCoord.xy / 1.5) + frame * 7.3));
    float dust = mote * srcBelow * dissolve * 1.6;

    float v = (body + streaks + dust) * visible * (1.0 + 0.8 * surge);
    gl_FragColor = vec4 (vec3 (v * emission), 1.0);
}
