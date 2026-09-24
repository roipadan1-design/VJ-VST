/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Negative Split (effect): splits any image into two layers - BODY, the inverted luminance with a contrast curve (red), and DETAIL, the image's fine bright structure (cyan) - then optionally breaks both into independent per-channel 1-bit noise, the dithered 'signal storm' state. Writes body to red and detail to green/blue; with a 'duo' palette the look pass colours them with the performer's palette (body = mid colour, detail = light colour).",
  "CATEGORIES": ["Effect", "Instrument"],
  "INPUTS": [
    { "NAME": "inputImage", "TYPE": "image" },
    { "NAME": "polarity", "TYPE": "float", "DEFAULT": 1.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "threshold", "TYPE": "float", "DEFAULT": 0.45, "MIN": 0.1, "MAX": 0.9 },
    { "NAME": "contrast", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "detail", "TYPE": "float", "DEFAULT": 1.0, "MIN": 0.0, "MAX": 3.0 },
    { "NAME": "outline", "TYPE": "float", "DEFAULT": 0.3, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "dither", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "storm", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "cell", "TYPE": "float", "DEFAULT": 2.0, "MIN": 1.0, "MAX": 6.0 },
    { "NAME": "flash", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 }
  ]
}*/
#include "common.glsl"

float lumAt (vec2 uv) { return vjLuma (IMG_NORM_PIXEL (inputImage, uv).rgb); }

void main()
{
    vec2 uv = isf_FragNormCoord;
    vec2 texel = 1.0 / RENDERSIZE.xy;

    // 3x3 neighbourhood at 1.5 px: a blur for the high-pass, Sobel for outlines.
    vec2 o = texel * 1.5;
    float l00 = lumAt (uv + vec2 (-o.x, -o.y)), l10 = lumAt (uv + vec2 (0.0, -o.y)), l20 = lumAt (uv + vec2 (o.x, -o.y));
    float l01 = lumAt (uv + vec2 (-o.x, 0.0)),  l11 = lumAt (uv),                    l21 = lumAt (uv + vec2 (o.x, 0.0));
    float l02 = lumAt (uv + vec2 (-o.x, o.y)),  l12 = lumAt (uv + vec2 (0.0, o.y)),  l22 = lumAt (uv + vec2 (o.x, o.y));
    float blur = (l00 + l20 + l02 + l22 + 2.0 * (l10 + l01 + l21 + l12) + 4.0 * l11) / 16.0;
    float gx = (l20 + 2.0 * l21 + l22) - (l00 + 2.0 * l01 + l02);
    float gy = (l02 + 2.0 * l12 + l22) - (l00 + 2.0 * l10 + l20);
    float sobel = length (vec2 (gx, gy));

    // BODY: the negative, through a contrast curve around the threshold.
    float l = clamp (l11, 0.0, 1.0);
    float inv = mix (l, 1.0 - l, polarity);
    float w = mix (0.5, 0.04, contrast);
    float body = mix (inv, smoothstep (threshold - w, threshold + w, inv), contrast);

    // DETAIL: what is finer and brighter than its surroundings (lit rims,
    // wires, highlights), plus a little of every outline.
    float fine = max (l11 - blur - 0.01, 0.0) * 6.0;
    float edge = clamp ((fine + sobel * outline * 1.5) * detail * (1.0 + 2.0 * flash), 0.0, 1.0);
    // Where the body is solid, it wins: a thin dark line stays a red core
    // with cyan fringes instead of turning cyan altogether.
    edge *= 1.0 - 0.85 * smoothstep (0.35, 0.85, body);

    // Per-channel 1-bit noise ("storm"): each layer gets its own random
    // threshold per cell, re-dealt 24 times a second. Lifting the floor as
    // the storm grows turns flat areas into a red/cyan mosaic.
    float s = clamp (dither + storm, 0.0, 1.0);
    if (s > 0.001)
    {
        vec2 cellId = floor (gl_FragCoord.xy / cell);
        float frame = floor (vj_time * 24.0);
        float nr = vjHash12 (cellId + frame * 17.31);
        float ng = vjHash12 (cellId * 1.37 + 91.7 + frame * 11.13);
        float liftR = mix (body, 0.18 + 0.72 * body, s);
        float liftG = mix (edge, 0.14 + 0.8 * edge, s);
        body = mix (body, step (nr, liftR), s);
        edge = mix (edge, step (ng, liftG), s);
    }

    gl_FragColor = vec4 (vjToLinear (vec3 (body, edge, edge)), 1.0);
}
