/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Media lines (effect): fine light linework on black drawn from any image - a difference of two blurs finds the contours (like XDoG), thin and film-friendly - plus a sparse stipple of dots in the bright areas.",
  "CATEGORIES": ["Effect", "Instrument"],
  "INPUTS": [
    { "NAME": "inputImage", "TYPE": "image" },
    { "NAME": "radius", "TYPE": "float", "DEFAULT": 1.6, "MIN": 0.8, "MAX": 4.0 },
    { "NAME": "threshold", "TYPE": "float", "DEFAULT": 0.04, "MIN": 0.005, "MAX": 0.2 },
    { "NAME": "dots", "TYPE": "float", "DEFAULT": 0.3, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "fill", "TYPE": "float", "DEFAULT": 0.06, "MIN": 0.0, "MAX": 0.5 },
    { "NAME": "flash", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.2, "MAX": 4.0 }
  ]
}*/
#include "common.glsl"

float lumAt (vec2 uv) { return vjLuma (IMG_NORM_PIXEL (inputImage, uv).rgb); }

// Mean luminance on a ring of 8 taps at radius r (pixels).
float ring (vec2 uv, vec2 texel, float r)
{
    float s = 0.0;
    for (int i = 0; i < 8; ++i)
    {
        float a = float (i) * 0.7853982 + 0.39;
        s += lumAt (uv + vec2 (cos (a), sin (a)) * texel * r);
    }
    return s * 0.125;
}

void main()
{
    vec2 uv = isf_FragNormCoord;
    vec2 texel = 1.0 / RENDERSIZE.xy;
    float scale = RENDERSIZE.y / 1080.0;

    float l0 = lumAt (uv);
    float near = (l0 + ring (uv, texel, radius * scale)) * 0.5;
    float far = ring (uv, texel, radius * scale * 2.2);

    // Difference of the two blurs: a thin contour on both sides of every
    // edge, antialiased by its own magnitude.
    float dog = abs (near - far);
    float line = smoothstep (threshold, threshold * 2.2 + 0.01, dog);

    // Stipple: sparse single-pixel dots where the image is bright (a fixed
    // pattern, so a frozen picture holds).
    float h = vjHash12 (floor (gl_FragCoord.xy));
    float dotMask = step (h, dots * 0.35 * l0 * l0);

    float v = line * (1.0 + 1.5 * flash) + dotMask * 0.8 + l0 * fill;
    gl_FragColor = vec4 (vjToLinear (vec3 (clamp (v, 0.0, 1.0))) * emission, 1.0);
}
