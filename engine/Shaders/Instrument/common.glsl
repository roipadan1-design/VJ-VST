// Shared helpers for the Instrument shader family (pulled in with
// #include "common.glsl"). Generators write LINEAR colour; the engine's
// finish pass does bloom, tone mapping and the sRGB encode.

#define VJ_PI 3.14159265359
#define VJ_TAU 6.28318530718

mat2 vjRot (float a) { float c = cos (a), s = sin (a); return mat2 (c, -s, s, c); }

float vjHash12 (vec2 p)
{
    vec3 p3 = fract (vec3 (p.xyx) * 0.1031);
    p3 += dot (p3, p3.yzx + 33.33);
    return fract ((p3.x + p3.y) * p3.z);
}

vec2 vjHash22 (vec2 p)
{
    vec3 p3 = fract (vec3 (p.xyx) * vec3 (0.1031, 0.1030, 0.0973));
    p3 += dot (p3, p3.yzx + 33.33);
    return fract ((p3.xx + p3.yz) * p3.zy);
}

float vjNoise (vec2 p)
{
    vec2 i = floor (p), f = fract (p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix (mix (vjHash12 (i), vjHash12 (i + vec2 (1.0, 0.0)), u.x),
                mix (vjHash12 (i + vec2 (0.0, 1.0)), vjHash12 (i + vec2 (1.0, 1.0)), u.x), u.y);
}

float vjFbm (vec2 p)
{
    float v = 0.0, a = 0.5;
    mat2 m = mat2 (1.6, 1.2, -1.2, 1.6);
    for (int i = 0; i < 5; ++i)
    {
        v += a * vjNoise (p);
        p = m * p;
        a *= 0.5;
    }
    return v;
}

// Aspect-correct centred coordinates: y in -1..1, x scaled by aspect.
vec2 vjCentered() { return (2.0 * gl_FragCoord.xy - RENDERSIZE.xy) / max (RENDERSIZE.y, 1.0); }

float vjLuma (vec3 c) { return dot (c, vec3 (0.2126, 0.7152, 0.0722)); }

vec3 vjToLinear (vec3 c) { return pow (max (c, vec3 (0.0)), vec3 (2.2)); }

// Samples an image "contain"-fitted in the centred space p (height = 2 / zoom).
// Returns premultiplied-ish rgba with alpha 0 outside the image.
vec4 vjSampleFit (sampler2D img, vec2 size, vec2 p, float zoom)
{
    float aspect = size.x / max (size.y, 1.0);
    vec2 uv = p * zoom * 0.5;
    uv.x /= aspect;
    uv += 0.5;
    float inside = step (0.0, uv.x) * step (uv.x, 1.0) * step (0.0, uv.y) * step (uv.y, 1.0);
    vec4 c = texture2D (img, clamp (uv, 0.0, 1.0));
    return c * inside;
}

// Curated cosine palettes (IQ form), chosen by index; returned in linear light.
//   0 ice/violet  1 ember  2 acid teal  3 magenta-gold  4 thermal  5 holo
vec3 vjPaletteSet (float t, float s)
{
    vec3 a, b, c, d;
    if (s < 0.5)      { a = vec3 (0.50, 0.50, 0.58); b = vec3 (0.45, 0.45, 0.42); c = vec3 (1.0);             d = vec3 (0.55, 0.62, 0.72); }
    else if (s < 1.5) { a = vec3 (0.58, 0.38, 0.30); b = vec3 (0.45, 0.35, 0.30); c = vec3 (1.0, 0.9, 0.8);   d = vec3 (0.00, 0.10, 0.18); }
    else if (s < 2.5) { a = vec3 (0.45, 0.55, 0.50); b = vec3 (0.42, 0.45, 0.40); c = vec3 (0.9, 1.0, 1.0);   d = vec3 (0.30, 0.20, 0.10); }
    else if (s < 3.5) { a = vec3 (0.62, 0.45, 0.60); b = vec3 (0.40, 0.38, 0.42); c = vec3 (1.0);             d = vec3 (0.80, 0.92, 0.30); }
    else if (s < 4.5) { a = vec3 (0.50, 0.35, 0.45); b = vec3 (0.50, 0.45, 0.50); c = vec3 (1.0, 1.0, 0.6);   d = vec3 (0.60, 0.80, 0.95); }
    else              { a = vec3 (0.55);             b = vec3 (0.45);             c = vec3 (1.0);             d = vec3 (0.00, 0.33, 0.67); }
    return vjToLinear (clamp (a + b * cos (VJ_TAU * (c * t + d)), 0.0, 1.0));
}

// Palette family from `base` (0-5), advanced by palette-advance triggers
// (vj_palette, smoothed), crossfading between neighbouring sets.
vec3 vjPalette (float t, float base)
{
    float s = base + vj_palette;
    float s0 = mod (floor (s), 6.0), s1 = mod (floor (s) + 1.0, 6.0);
    return mix (vjPaletteSet (t, s0), vjPaletteSet (t, s1), smoothstep (0.0, 1.0, fract (s)));
}

// Thermal / false-colour map (black -> indigo -> magenta -> orange -> yellow -> white).
vec3 vjThermal (float t)
{
    t = clamp (t, 0.0, 1.0);
    vec3 c = mix (vec3 (0.0, 0.0, 0.02), vec3 (0.10, 0.05, 0.55), smoothstep (0.00, 0.25, t));
    c = mix (c, vec3 (0.85, 0.05, 0.60), smoothstep (0.20, 0.50, t));
    c = mix (c, vec3 (1.00, 0.45, 0.05), smoothstep (0.45, 0.75, t));
    c = mix (c, vec3 (1.00, 0.95, 0.35), smoothstep (0.70, 0.92, t));
    return vjToLinear (mix (c, vec3 (1.0), smoothstep (0.92, 1.0, t)));
}

// Thin-film iridescence: a rainbow that shifts with "thickness".
vec3 vjIridescent (float thickness)
{
    return vjToLinear (0.5 + 0.5 * cos (VJ_TAU * (thickness + vec3 (0.0, 0.33, 0.67))));
}
