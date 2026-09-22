/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Thermal slices: the source false-coloured through a heat-map, torn into horizontal slices that jump on hits, with chromatic fringes.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "source", "TYPE": "image" },
    { "NAME": "zoom", "TYPE": "float", "DEFAULT": 1.5, "MIN": 0.6, "MAX": 3.5 },
    { "NAME": "drift", "TYPE": "float", "DEFAULT": 0.0, "MIN": -1000.0, "MAX": 1000.0 },
    { "NAME": "slices", "TYPE": "float", "DEFAULT": 18.0, "MIN": 4.0, "MAX": 60.0 },
    { "NAME": "tear", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "map_offset", "TYPE": "float", "DEFAULT": 0.0, "MIN": -0.5, "MAX": 0.5 },
    { "NAME": "contrast", "TYPE": "float", "DEFAULT": 1.3, "MIN": 0.5, "MAX": 3.0 },
    { "NAME": "aberration", "TYPE": "float", "DEFAULT": 0.006, "MIN": 0.0, "MAX": 0.04 },
    { "NAME": "texture_amt", "TYPE": "float", "DEFAULT": 0.45, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.3, "MIN": 0.3, "MAX": 4.0 }
  ]
}*/
#include "common.glsl"

float heat (vec2 q)
{
    vec4 s = vjSampleFit (source, source_size, q, zoom);
    float body = vjLuma (s.rgb) * s.a;
    // Surrounding "air" has its own drifting heat so the frame is never empty.
    float air = vjFbm (q * 2.2 + vec2 (drift * 0.05, 0.0)) * texture_amt;
    return max (body, air * (1.0 - s.a * 0.7));
}

void main()
{
    vec2 p = vjCentered();
    vec2 uv = gl_FragCoord.xy / RENDERSIZE.xy;

    // Slice tear: per-row offsets re-randomised each beat, scaled by the hit envelope.
    float row = floor (uv.y * slices);
    float beatSeed = floor (vj_beat * 2.0);
    float jump = (vjHash12 (vec2 (row, beatSeed)) - 0.5) * 2.0;
    float rowOn = step (0.45, vjHash12 (vec2 (row * 1.7, beatSeed + 3.0)));
    float offset = jump * rowOn * tear * 0.5 + drift * 0.02;
    vec2 q = p + vec2 (offset, 0.0);

    float hR = heat (q + vec2 (aberration, 0.0));
    float hG = heat (q);
    float hB = heat (q - vec2 (aberration, 0.0));

    float k = contrast;
    vec3 col;
    col.r = vjThermal (pow (hR, 1.0 / k) + map_offset).r;
    col.g = vjThermal (pow (hG, 1.0 / k) + map_offset).g;
    col.b = vjThermal (pow (hB, 1.0 / k) + map_offset).b;

    // Bright stretched streaks on the torn rows.
    col += vec3 (0.3, 0.1, 0.6) * rowOn * tear * step (0.93, vjHash12 (vec2 (row, beatSeed + 9.0)));

    gl_FragColor = vec4 (col * emission, 1.0);
}
