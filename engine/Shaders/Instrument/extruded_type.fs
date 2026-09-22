/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Extruded type: a word from the text source extruded into chunky 3D, faced in chrome/holo colour, smeared and RGB-split on hits, over angular duotone shards.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "word", "TYPE": "image" },
    { "NAME": "depth", "TYPE": "float", "DEFAULT": 0.12, "MIN": 0.0, "MAX": 0.35 },
    { "NAME": "angle", "TYPE": "float", "DEFAULT": -0.6, "MIN": -3.14, "MAX": 3.14 },
    { "NAME": "zoom", "TYPE": "float", "DEFAULT": 1.1, "MIN": 0.5, "MAX": 2.5 },
    { "NAME": "smear", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "aberration", "TYPE": "float", "DEFAULT": 0.004, "MIN": 0.0, "MAX": 0.03 },
    { "NAME": "hue", "TYPE": "float", "DEFAULT": 0.15, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "background", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "wobble", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.3, "MIN": 0.3, "MAX": 4.0 }
  ]
}*/
#include "common.glsl"

float glyph (vec2 q)
{
    return vjSampleFit (word, word_size, q, zoom).a;
}

// Angular background: large triangles in two palette tones.
vec3 shardsBackground (vec2 p)
{
    vec2 q = vjRot (0.4 + wobble * 0.02) * p * 1.3;
    vec2 cell = floor (q);
    vec2 f = fract (q);
    float diag = step (f.x, f.y);
    float h = vjHash12 (cell + diag * 7.0);
    vec3 a = vjPalette (0.62, 0.0 + hue * 5.99);
    vec3 b = vjPalette (0.05, 1.0 + hue * 5.99);
    return mix (a * 0.35, b * 0.55, step (0.5, h)) * (0.4 + 0.6 * h);
}

void main()
{
    vec2 p = vjCentered();
    p += vec2 (sin (wobble * 0.7), cos (wobble * 0.5)) * 0.02;

    vec2 dir = vec2 (cos (angle), sin (angle));
    vec3 col = shardsBackground (p) * background;

    // Extrusion: march back along `dir`; the first layer hit gives the depth shade.
    float hitDepth = -1.0;
    for (int i = 0; i < 24; ++i)
    {
        float k = float (i) / 23.0;
        if (glyph (p + dir * depth * k) > 0.5)
        {
            hitDepth = k;
            break;
        }
    }

    if (hitDepth >= 0.0)
    {
        vec3 side = vjPalette (0.08 + hitDepth * 0.2, 1.0 + hue * 5.99) * mix (1.0, 0.25, hitDepth);
        col = hitDepth < 0.02 ? vec3 (0.0) : side;
    }

    // Face with horizontal smear and chromatic split.
    float sm = smear * 0.25;
    float faceR = 0.0, faceG = 0.0, faceB = 0.0;
    for (int i = 0; i < 8; ++i)
    {
        float o = (float (i) / 7.0 - 0.5) * sm;
        faceR += glyph (p + vec2 (o + aberration, 0.0));
        faceG += glyph (p + vec2 (o, 0.0));
        faceB += glyph (p + vec2 (o - aberration, 0.0));
    }
    vec3 face = vec3 (faceR, faceG, faceB) / 8.0;
    vec3 faceColour = mix (vjPalette (p.y * 0.5 + 0.2, 1.0 + hue * 5.99) * 1.8, vjIridescent (p.x * 0.8 + wobble * 0.05) * 1.4, 0.35);
    col = mix (col, faceColour, clamp (face, 0.0, 1.0));
    col += faceColour * smoothstep (0.4, 0.6, face.g) * 0.25;       // glossy lift

    gl_FragColor = vec4 (col * emission, 1.0);
}
