/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Liquid chrome: a flowing metaball height field lit like polished metal (black/white studio reflections), with a source mask floating in it. Linear HDR out.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "mask", "TYPE": "image" },
    { "NAME": "scale", "TYPE": "float", "DEFAULT": 1.6, "MIN": 0.5, "MAX": 4.0 },
    { "NAME": "flow", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 },
    { "NAME": "surge", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "contrast", "TYPE": "float", "DEFAULT": 1.4, "MIN": 0.5, "MAX": 3.0 },
    { "NAME": "tint", "TYPE": "float", "DEFAULT": 0.15, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "hue", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "mask_mix", "TYPE": "float", "DEFAULT": 0.8, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "mask_zoom", "TYPE": "float", "DEFAULT": 1.35, "MIN": 0.6, "MAX": 3.0 },
    { "NAME": "glow", "TYPE": "float", "DEFAULT": 0.4, "MIN": 0.0, "MAX": 2.0 }
  ]
}*/
#include "common.glsl"

float field (vec2 p, float t)
{
    // Domain-warped fBm makes the soft, pooling blobs of liquid metal.
    vec2 q = vec2 (vjFbm (p + vec2 (0.0, t * 0.6)), vjFbm (p + vec2 (5.2, 1.3) - t * 0.4));
    return vjFbm (p + 1.8 * q + vec2 (t * 0.15, 0.0));
}

// Studio environment: bright soft boxes and a hard horizon, in black & white.
float environment (vec3 r)
{
    float horizon = smoothstep (-0.05, 0.1, r.y);
    float boxes = smoothstep (0.55, 0.7, sin (r.x * 5.0 + r.y * 2.0) * 0.5 + 0.5);
    float top = smoothstep (0.6, 0.95, r.y);
    return 0.03 + 0.55 * horizon * boxes + 1.6 * top + 0.25 * horizon;
}

void main()
{
    vec2 p = vjCentered();
    float t = flow;
    vec2 sp = p * scale;

    // Height field; the surge (kick) sharpens and lifts the blobs.
    float e = 1.5 / RENDERSIZE.y * scale;
    float h  = field (sp, t);
    float hx = field (sp + vec2 (e, 0.0), t);
    float hy = field (sp + vec2 (0.0, e), t);
    float k = mix (3.0, 7.0, surge) * contrast;
    vec3 n = normalize (vec3 ((h - hx) * k / e * 0.02, (h - hy) * k / e * 0.02, 1.0));

    vec3 viewDir = vec3 (0.0, 0.0, -1.0);
    vec3 r = reflect (viewDir, n);
    float env = environment (r);

    // Fresnel-ish rim and deep crevices.
    float crevice = smoothstep (0.35, 0.62, h);
    float lum = pow (env, contrast) * mix (0.15, 1.0, crevice);
    lum += surge * 0.6 * pow (max (r.y, 0.0), 8.0);

    vec3 metal = vec3 (lum);
    metal = mix (metal, metal * vjPalette (h * 0.4 + r.x * 0.1, hue * 5.99) * 1.6, tint);

    // Source mask floating in the chrome, reflecting the same environment.
    vec4 m = vjSampleFit (mask, mask_size, p + 0.03 * vec2 (sin (t * 0.7), cos (t * 0.5)), mask_zoom);
    vec3 maskLit = vjToLinear (m.rgb) * (0.6 + 0.9 * env * 0.5) * (1.0 + surge);
    float halo = smoothstep (0.0, 1.0, vjSampleFit (mask, mask_size, p, mask_zoom * 0.93).a) * (1.0 - m.a);

    vec3 col = metal;
    col = mix (col, maskLit * 1.4, m.a * mask_mix);
    col += halo * glow * mask_mix * vjPalette (0.2, hue * 5.99) * 0.8;

    gl_FragColor = vec4 (col, 1.0);
}
