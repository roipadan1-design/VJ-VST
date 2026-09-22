/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Holo shards: the source coated in thin-film iridescence with circular scan rings, surrounded by white glass shards that burst outward on hits.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "source", "TYPE": "image" },
    { "NAME": "zoom", "TYPE": "float", "DEFAULT": 1.25, "MIN": 0.6, "MAX": 3.0 },
    { "NAME": "spin", "TYPE": "float", "DEFAULT": 0.0, "MIN": -1000.0, "MAX": 1000.0 },
    { "NAME": "film", "TYPE": "float", "DEFAULT": 2.2, "MIN": 0.5, "MAX": 6.0 },
    { "NAME": "shimmer", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 },
    { "NAME": "shards", "TYPE": "float", "DEFAULT": 0.35, "MIN": 0.0, "MAX": 0.9 },
    { "NAME": "burst", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "rings", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.3, "MIN": 0.3, "MAX": 4.0 }
  ]
}*/
#include "common.glsl"

// Voronoi-based shard field: sharp white polygons drifting outward.
float shardField (vec2 p, float t)
{
    vec2 radial = normalize (p + 1e-4);
    vec2 q = p - radial * t * 0.25;                   // everything flows outward
    q *= 6.0;
    vec2 cell = floor (q), f = fract (q);
    float best = 8.0, second = 8.0;
    vec2 bestId = vec2 (0.0);
    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x)
        {
            vec2 g = vec2 (float (x), float (y));
            vec2 o = vjHash22 (cell + g);
            float d = length (g + o - f);
            if (d < best) { second = best; best = d; bestId = cell + g; }
            else if (d < second) second = d;
        }
    float keep = step (1.0 - shards, vjHash12 (bestId * 1.31));
    // Crisp polygon interior shrunk from the cell edges.
    return keep * smoothstep (0.05, 0.09, second - best);
}

void main()
{
    vec2 p = vjCentered();
    vec2 q = vjRot (spin * 0.15) * p;
    q *= 1.0 - 0.06 * burst;

    vec4 s = vjSampleFit (source, source_size, q, zoom);
    float l = vjLuma (s.rgb);

    // Thin-film: thickness follows the surface shading + slow shimmer.
    float thickness = l * film + length (q) * 0.6 + shimmer * 0.1;
    vec3 holo = vjIridescent (thickness) * (0.35 + 0.9 * l);
    float ringLines = smoothstep (0.85, 1.0, sin (length (q) * 55.0 - shimmer * 2.0)) * rings;
    holo += vec3 (0.6, 0.1, 0.2) * ringLines * s.a;
    vec3 col = holo * s.a * emission;

    // Shards behind/around the figure; the burst throws them outward.
    float sh = shardField (p, shimmer * 0.3 + burst * 1.5);
    col += vec3 (1.0) * sh * (1.0 - s.a) * (0.8 + 1.5 * burst);

    gl_FragColor = vec4 (col, 1.0);
}
