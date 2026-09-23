/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Morphogen: Gray-Scott reaction-diffusion grown at quarter resolution (8 steps a frame), lit as a relief. Pattern slides between fingerprint, coral, mitosis and worm families; kicks plant new seeds, which grow roots across the frame.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "pattern", "TYPE": "float", "DEFAULT": 0.35, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "rate", "TYPE": "float", "DEFAULT": 0.9, "MIN": 0.2, "MAX": 1.0 },
    { "NAME": "seed_amt", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "zoom", "TYPE": "float", "DEFAULT": 1.0, "MIN": 0.6, "MAX": 2.0 },
    { "NAME": "relief", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 1.5 },
    { "NAME": "boundary", "TYPE": "float", "DEFAULT": 0.55, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.2, "MAX": 4.0 }
  ],
  "PASSES": [
    { "TARGET": "rd", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.25", "HEIGHT": "0.25" },
    { "TARGET": "rd", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.25", "HEIGHT": "0.25" },
    { "TARGET": "rd", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.25", "HEIGHT": "0.25" },
    { "TARGET": "rd", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.25", "HEIGHT": "0.25" },
    { "TARGET": "rd", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.25", "HEIGHT": "0.25" },
    { "TARGET": "rd", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.25", "HEIGHT": "0.25" },
    { "TARGET": "rd", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.25", "HEIGHT": "0.25" },
    { "TARGET": "rd", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.25", "HEIGHT": "0.25" },
    { }
  ]
}*/
#include "common.glsl"

// rd.r = 1 - A (so the zero-initialised buffer means A = 1), rd.g = B.

vec2 feedKill (float t)
{
    // fingerprint -> coral -> mitosis -> worms (Pearson / Sims families)
    vec2 a = vec2 (0.037, 0.060), b = vec2 (0.055, 0.062), c = vec2 (0.0367, 0.0649), d = vec2 (0.078, 0.061);
    t = clamp (t, 0.0, 1.0) * 3.0;
    if (t < 1.0) return mix (a, b, t);
    if (t < 2.0) return mix (b, c, t - 1.0);
    return mix (c, d, t - 2.0);
}

void main()
{
    if (PASSINDEX < 8)
    {
        vec2 texel = 1.0 / RENDERSIZE;
        vec2 uv = gl_FragCoord.xy * texel;
        vec2 c = texture2D (rd, uv).rg;
        vec2 lap = -c;
        lap += 0.2 * (texture2D (rd, uv + vec2 (texel.x, 0.0)).rg + texture2D (rd, uv - vec2 (texel.x, 0.0)).rg
                    + texture2D (rd, uv + vec2 (0.0, texel.y)).rg + texture2D (rd, uv - vec2 (0.0, texel.y)).rg);
        lap += 0.05 * (texture2D (rd, uv + texel).rg + texture2D (rd, uv - texel).rg
                     + texture2D (rd, uv + vec2 (texel.x, -texel.y)).rg + texture2D (rd, uv + vec2 (-texel.x, texel.y)).rg);

        float A = 1.0 - c.r, B = c.g;
        float lapA = -lap.r, lapB = lap.g;
        vec2 fk = feedKill (pattern);
        float reaction = A * B * B;
        float nA = A + rate * (1.0 * lapA - reaction + fk.x * (1.0 - A));
        float nB = B + rate * (0.5 * lapB + reaction - (fk.y + fk.x) * B);

        // Seeds: a kick plants one at a new place (vj_seed is reseeded on
        // kicks); a slow trickle keeps the culture from ever dying out.
        vec2 aspect = vec2 (RENDERSIZE.x / RENDERSIZE.y, 1.0);
        vec2 at = vec2 (0.2, 0.2) + vjHash22 (vec2 (vj_seed, 5.3)) * 0.6;
        float d = length ((uv - at) * aspect);
        nB += seed_amt * 0.5 * (1.0 - smoothstep (0.012, 0.03, d));
        float trickle = step (0.99985, vjHash12 (gl_FragCoord.xy + floor (TIME * 3.0) * 11.0));
        nB += trickle * 0.8;

        // Soft circular boundary: the culture lives in an island.
        float edge = length ((uv - 0.5) * aspect);
        float limit = mix (0.9, 0.42, boundary);
        nB *= 1.0 - smoothstep (limit - 0.06, limit, edge);

        gl_FragColor = vec4 (1.0 - clamp (nA, 0.0, 1.0), clamp (nB, 0.0, 1.0), 0.0, 1.0);
    }
    else
    {
        vec2 uv = 0.5 + (isf_FragNormCoord - 0.5) / zoom;
        vec2 t = 1.5 / (RENDERSIZE * 0.25);
        float b = texture2D (rd, uv).g;
        float bx = texture2D (rd, uv + vec2 (t.x, 0.0)).g - texture2D (rd, uv - vec2 (t.x, 0.0)).g;
        float by = texture2D (rd, uv + vec2 (0.0, t.y)).g - texture2D (rd, uv - vec2 (0.0, t.y)).g;

        // Relief lighting of the B concentration.
        vec3 n = normalize (vec3 (-bx * relief * 6.0, -by * relief * 6.0, 1.0));
        float light = clamp (dot (n, normalize (vec3 (-0.5, 0.6, 0.6))), 0.0, 1.0);
        float body = smoothstep (0.12, 0.35, b);
        float v = body * (0.35 + 0.75 * light) + smoothstep (0.3, 0.5, b) * 0.25;
        gl_FragColor = vec4 (vec3 (v * emission), 1.0);
    }
}
