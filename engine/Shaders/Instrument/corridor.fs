/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Corridor: an endless service corridor seen while moving forward - panelled walls, cable runs, doorways and ceiling lights, lit from the lights and fogged into the vanishing point. Kicks surge the camera forward and blow the lights out; hats flicker them.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "travel", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 },
    { "NAME": "sway", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 },
    { "NAME": "sway_amt", "TYPE": "float", "DEFAULT": 0.3, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "width", "TYPE": "float", "DEFAULT": 1.0, "MIN": 0.5, "MAX": 2.5 },
    { "NAME": "surge", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "flicker", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "detail", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "fog", "TYPE": "float", "DEFAULT": 0.35, "MIN": 0.05, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.2, "MAX": 4.0 }
  ]
}*/
#include "common.glsl"

// Surface pattern of a wall/floor/ceiling at (u across, z along).
float wallPattern (vec2 w, float kind)
{
    float panelZ = fract (w.y * 0.5);
    float panelU = fract (w.x * 1.5);
    float seams = smoothstep (0.0, 0.03, panelZ) * smoothstep (1.0, 0.97, panelZ);
    seams *= mix (1.0, smoothstep (0.0, 0.04, panelU) * smoothstep (1.0, 0.96, panelU), 0.6);

    float id = vjHash12 (floor (vec2 (w.x * 1.5, w.y * 0.5)) + kind * 17.0);
    float shade = 0.25 + 0.75 * id * id;

    // Cable runs along the walls.
    float cables = 0.0;
    for (int i = 0; i < 3; ++i)
    {
        float y = 0.35 + float (i) * 0.12 + 0.02 * sin (w.y * 1.3 + float (i));
        cables += 1.0 - smoothstep (0.008, 0.02, abs (w.x - y));
    }
    cables *= step (kind, 0.5) * detail;

    // Doorways: dark recesses every few metres on the walls.
    float door = 0.0;
    float cell = floor (w.y / 6.0);
    if (kind < 0.5 && vjHash12 (vec2 (cell, 3.0)) > 0.45)
    {
        float z = fract (w.y / 6.0) * 6.0;
        door = step (2.0, z) * step (z, 3.1) * step (w.x, 0.8);
    }

    float grime = vjFbm (w * vec2 (3.0, 1.2) + id * 5.0);
    float stains = smoothstep (0.45, 0.75, vjFbm (w * vec2 (1.2, 0.5) + 9.0));
    float v = shade * seams * mix (1.0, 0.25 + 1.1 * grime, detail) * (1.0 - 0.8 * stains * detail);
    v = mix (v, 0.05, door * 0.9);
    v = mix (v, 0.02, clamp (cables, 0.0, 1.0));
    return v;
}

void main()
{
    vec2 p = vjCentered();
    float s = sway_amt * (1.0 + surge);
    vec3 ro = vec3 (0.25 * s * sin (sway * 0.7), 0.12 * s * sin (sway * 1.1), 0.0);
    vec3 rd = normalize (vec3 (p * (1.0 - 0.18 * surge), 1.4));
    rd.xy = vjRot (0.06 * s * sin (sway * 0.5)) * rd.xy;

    float hw = width, hh = 0.75;
    float tx = (sign (rd.x) * hw - ro.x) / rd.x;
    float ty = (sign (rd.y) * hh - ro.y) / rd.y;
    float t = min (tx, ty);
    vec3 hit = ro + rd * t;
    float z = hit.z + travel;

    float v;
    float kind;
    vec2 w;
    if (tx < ty) { kind = 0.0; w = vec2 ((hit.y + hh) / (2.0 * hh), z); }             // walls
    else         { kind = rd.y > 0.0 ? 1.0 : 2.0; w = vec2 ((hit.x + hw) / (2.0 * hw), z); } // ceiling / floor
    v = wallPattern (w, kind);

    // Ceiling light strips every 4 m light the section around them.
    float lightIdx = floor (z / 4.0 + 0.5);
    float dz = z - lightIdx * 4.0;
    float on = step (0.25, vjHash12 (vec2 (lightIdx, 1.0)));
    float blink = 1.0 - flicker * step (0.5, vjHash12 (vec2 (lightIdx, floor (TIME * 20.0))));
    float lit = on * blink * (1.0 - 0.85 * surge * step (0.5, vjHash12 (vec2 (lightIdx, 7.0))));
    float pool = exp (-dz * dz * 0.18) * lit;
    float strip = 0.0;
    if (kind == 1.0)
        strip = (1.0 - smoothstep (0.1, 0.16, abs (dz))) * (1.0 - smoothstep (0.06, 0.12, abs (w.x - 0.5))) * lit;

    float light = 0.06 + 1.0 * pool;
    float fogAmt = 1.0 - exp (-t * fog * 0.35);
    float col = v * light * (1.0 - fogAmt) + strip * 3.0;
    col += fogAmt * 0.04;

    vec3 c = vec3 (1.0, 0.45, 0.35) * col * emission;
    gl_FragColor = vec4 (c, 1.0);
}
