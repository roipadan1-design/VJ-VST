/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Signal fog (after Rainer Kohlberger): extremely fine particles flutter across a dark field; within the blur, large forms condense and dissolve. Focus pulls the forms out of the haze (build-ups sharpen them), the bass clock drifts the field, kicks flare the particle density.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "clock", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 },
    { "NAME": "scale", "TYPE": "float", "DEFAULT": 1.3, "MIN": 0.4, "MAX": 4.0 },
    { "NAME": "focus", "TYPE": "float", "DEFAULT": 0.35, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "density", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "haze", "TYPE": "float", "DEFAULT": 0.25, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "grain_size", "TYPE": "float", "DEFAULT": 1.4, "MIN": 1.0, "MAX": 4.0 },
    { "NAME": "surge", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.2, "MAX": 4.0 }
  ]
}*/
#include "common.glsl"

// Two slowly drifting noise slices crossfaded - a pseudo-3D volume that
// evolves without shimmering.
float volume (vec2 p, float t)
{
    float z = t * 0.07;
    float k = fract (z);
    float a = vjFbm (p + vec2 (floor (z) * 17.3, floor (z) * 9.1) + vec2 (t * 0.05, -t * 0.03));
    float b = vjFbm (p + vec2 ((floor (z) + 1.0) * 17.3, (floor (z) + 1.0) * 9.1) + vec2 (t * 0.05, -t * 0.03));
    return mix (a, b, smoothstep (0.0, 1.0, k));
}

void main()
{
    vec2 p = vjCentered();
    vec2 sp = p * scale;

    // The form: a thresholded volume whose edge width is the "blur".
    float warp = vjFbm (sp * 0.6 - clock * 0.02);
    float v = volume (sp + (warp - 0.5) * 0.8, clock);
    float soft = mix (0.22, 0.015, focus);
    float form = smoothstep (0.52 - soft, 0.52 + soft, v);

    // Particles: a fresh speckle field every film frame (24 fps), denser where
    // the form is; a kick flares the whole field.
    float cell = grain_size * RENDERSIZE.y / 1080.0;
    vec2 id = floor (gl_FragCoord.xy / cell);
    float frame = floor (TIME * 24.0);
    float n = vjHash12 (id + vec2 (frame * 13.17, frame * 7.31));
    float amount = density * (0.08 + 0.92 * form) * (1.0 + 1.5 * surge);
    float speck = step (1.0 - amount * 0.55, n);

    float value = speck * (0.35 + 0.65 * form) + form * haze * 0.35 + haze * 0.03;
    // Faint horizontal streaks inside the form, like a scanned print.
    value += form * 0.06 * (vjNoise (vec2 (gl_FragCoord.y * 0.35, clock * 3.0)) - 0.5);

    gl_FragColor = vec4 (vec3 (max (value, 0.0) * emission), 1.0);
}
