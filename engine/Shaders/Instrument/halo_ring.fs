/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Halo ring (after The Noise Diary's ring): a thin bright ring on black whose edge is pushed by the sound - bass swells slow lobes, highs grow fine radial hairs, mids thicken grey fur in drifting patches. Kicks flare the ring.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "radius", "TYPE": "float", "DEFAULT": 0.55, "MIN": 0.2, "MAX": 0.95 },
    { "NAME": "spin", "TYPE": "float", "DEFAULT": 0.0, "MIN": -100000.0, "MAX": 100000.0 },
    { "NAME": "wobble", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 },
    { "NAME": "low", "TYPE": "float", "DEFAULT": 0.3, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "hair", "TYPE": "float", "DEFAULT": 0.4, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "fur", "TYPE": "float", "DEFAULT": 0.4, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "surge", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.3, "MIN": 0.2, "MAX": 4.0 },
    { "NAME": "fray", "TYPE": "float", "DEFAULT": 0.1, "MIN": 0.0, "MAX": 1.0 }
  ]
}*/
#include "common.glsl"

// Periodic 1D noise around the circle (angle in turns, period 1).
float ringNoise (float turns, float freq, float t)
{
    float x = turns * freq;
    float i = floor (x), f = fract (x);
    float a = vjHash12 (vec2 (mod (i, freq), t));
    float b = vjHash12 (vec2 (mod (i + 1.0, freq), t));
    return mix (a, b, f * f * (3.0 - 2.0 * f));
}

void main()
{
    vec2 p = vjCentered();
    float r = length (p);
    float turns = fract (atan (p.y, p.x) / VJ_TAU + 0.5 + spin * 0.02);
    float px = 2.0 / RENDERSIZE.y;

    // Slow lobes (bass): two octaves of smooth noise, crossfaded in time.
    float tw = wobble * 0.15;
    float lobes = mix (ringNoise (turns, 5.0, floor (tw)), ringNoise (turns, 5.0, floor (tw) + 1.0), fract (tw)) - 0.5;
    lobes += 0.5 * (mix (ringNoise (turns, 11.0, floor (tw) + 7.0), ringNoise (turns, 11.0, floor (tw) + 8.0), fract (tw)) - 0.5);
    float R = radius * (1.0 + 0.1 * surge) + lobes * low * 0.12;

    // The core line.
    float d = r - R;
    float core = exp (-abs (d) / (px * 1.2)) + 0.35 * exp (-abs (d) / (px * 6.0));
    // Erode: the line breaks into arcs that drift round slowly.
    float gapN = ringNoise (turns, 23.0, floor (wobble * 0.2)) * 0.6 + ringNoise (turns, 61.0, 3.0) * 0.4;
    core *= mix (1.0, smoothstep (fray * 0.85 - 0.04, fray * 0.85 + 0.04, gapN), step (0.001, fray));

    // Radial hairs (highs): one strand per angular bin, random length that
    // re-rolls a few times a second, reaching both out and (shorter) in.
    float bins = 720.0;
    float bin = floor (turns * bins);
    float binCentre = (bin + 0.5) / bins;
    float across = abs (turns - binCentre) * VJ_TAU * r;
    float roll = floor (vj_time * 6.0 + vjHash12 (vec2 (bin, 3.0)) * 6.0);
    float h1 = vjHash12 (vec2 (bin, roll));
    float len = hair * 0.16 * h1 * h1 * (0.4 + ringNoise (turns, 7.0, floor (wobble * 0.3)));
    float along = d > 0.0 ? d / max (len, 1e-4) : -d / max (len * 0.5, 1e-4);
    float strand = step (0.35, vjHash12 (vec2 (bin, roll + 5.0))) * (1.0 - smoothstep (0.0, px * 0.9, across))
                 * (1.0 - smoothstep (0.0, 1.0, along)) * step (0.0, 1.0 - along);

    // Grey fur (mids): wide soft bands on drifting stretches of the ring.
    float furPatch = smoothstep (0.55, 0.85, ringNoise (turns, 4.0, floor (wobble * 0.1)) * 0.5
                                           + ringNoise (turns, 9.0, floor (wobble * 0.1) + 3.0) * 0.5);
    float furWidth = fur * 0.05 * furPatch;
    float fuzz = exp (-abs (d) / max (furWidth, px)) * furPatch * fur * (0.6 + 0.4 * vjHash12 (gl_FragCoord.xy + floor (vj_time * 24.0)));

    float v = core * (1.0 + 1.5 * surge) + strand * 0.7 + fuzz * 0.45;
    gl_FragColor = vec4 (vec3 (v * emission), 1.0);
}
