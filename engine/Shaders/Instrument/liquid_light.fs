/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Liquid Light (after the 1960s oil-and-water light shows - the Joshua Light Show, Boyle & Hills, Bill Ham): a clock glass of dense dye and clear oil on a dim overhead projector, stirred by slow convection and a slowly rocked dish. Light only leaks where the dye thins; oil lenses drift, stretch, merge and split, each outlined by a dark meniscus with a caustic just inside it, magnifying the dye film beneath. A persistent half-resolution buffer carries the liquid, so whatever the flow does stays. Ambient family.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "flow", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 },
    { "NAME": "stir", "TYPE": "float", "DEFAULT": 0.35, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "turn", "TYPE": "float", "DEFAULT": 0.0, "MIN": -1.0, "MAX": 1.0 },
    { "NAME": "scale", "TYPE": "float", "DEFAULT": 1.0, "MIN": 0.4, "MAX": 2.5 },
    { "NAME": "oil", "TYPE": "float", "DEFAULT": 0.4, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "melt", "TYPE": "float", "DEFAULT": 0.25, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "eddies", "TYPE": "float", "DEFAULT": 0.4, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "density", "TYPE": "float", "DEFAULT": 4.5, "MIN": 0.5, "MAX": 8.0 },
    { "NAME": "heat", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "drip", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "lens", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 1.5 },
    { "NAME": "rim", "TYPE": "float", "DEFAULT": 0.8, "MIN": 0.0, "MAX": 2.0 },
    { "NAME": "clarity", "TYPE": "float", "DEFAULT": 0.25, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "tint", "TYPE": "float", "DEFAULT": 0.4, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "lamp", "TYPE": "float", "DEFAULT": 1.0, "MIN": 0.2, "MAX": 3.0 }
  ],
  "PASSES": [
    { "TARGET": "liquid", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.5", "HEIGHT": "0.5" },
    { }
  ]
}*/
#include "common.glsl"

// liquid (half resolution, float, persistent):
//   r = dye A (indigo, absorbs red), b = dye B (amber, absorbs blue): 0..1.5
//   g = oil phase: 0 water .. 1 oil (Allen-Cahn: it separates into sharp
//       domains by itself, diffusion sets the width of the meniscus)
//   a = 0.5 once initialised (the engine clears targets to opaque black, a = 1)
// Every step runs on the scene clock (vj_dt): Speed 0 / Freeze hold the liquid.

float streamFn (vec2 q, float t)
{
    return (vjNoise (q * 0.55 + vec2 (t * 0.21, -t * 0.13)) - 0.5)
         + (vjNoise (q * 1.4 + vec2 (-t * 0.34, t * 0.27) + 7.3) - 0.5) * 0.45 * (0.3 + eddies)
         + (vjNoise (q * 3.1 + vec2 (t * 0.5, t * 0.41) - 3.1) - 0.5) * 0.2 * eddies;
}

// Divergence-free convection (curl of a stream function) plus the rocked dish.
vec2 velocity (vec2 q, float t)
{
    float e = 0.05;
    float s0 = streamFn (q, t);
    float sx = streamFn (q + vec2 (e, 0.0), t);
    float sy = streamFn (q + vec2 (0.0, e), t);
    return vec2 (sy - s0, s0 - sx) / e * stir * 0.6 + turn * 0.12 * vec2 (-q.y, q.x);
}

// Where oil and dye want to be (slowly moving reservoirs the liquid relaxes to,
// so it never settles into one flat colour and never dies out).
float oilBase (vec2 q, float t)
{
    // Mostly one broad octave: no pinholes or specks inside the oil (a small
    // dark hole in a pale lens reads as an eye - never wanted here).
    float n = 0.82 * vjNoise (q * 2.0 * scale + vec2 (t * 0.05, 3.0))
            + 0.18 * vjNoise (vjRot (1.3) * q * 4.3 * scale - vec2 (1.3, t * 0.07));
    float th = mix (0.72, 0.38, oil);
    return smoothstep (th - 0.07, th + 0.07, n);
}

float dyeBase (vec2 q, float t, float o)
{
    // Octaves turned against each other so the value-noise lattice never lines up.
    float n = 0.52 * vjNoise (q * 2.4 * scale + vec2 (o, t * 0.04))
            + 0.33 * vjNoise (vjRot (0.9) * q * 5.5 * scale - vec2 (t * 0.03, o))
            + 0.15 * vjNoise (vjRot (2.1) * q * 11.0 * scale + vec2 (o * 1.7, t * 0.05));
    return smoothstep (0.27, 0.73, n);
}

void main()
{
    if (PASSINDEX == 0)
    {
        vec2 texel = 1.0 / RENDERSIZE;
        vec2 uv = gl_FragCoord.xy * texel;
        float aspect = RENDERSIZE.x / RENDERSIZE.y;
        vec2 P = (uv - 0.5) * vec2 (aspect, 1.0) * 2.0;     // same units as vjCentered()
        vec4 here = texture2D (liquid, uv);

        if (abs (here.a - 0.5) > 0.25)
        {
            // Born: pour the reservoirs in as they are.
            gl_FragColor = vec4 (dyeBase (P, flow, 0.0), oilBase (P, flow), dyeBase (P, flow, 17.0), 0.5);
            return;
        }

        float dt = min (abs (vj_dt), 0.033);
        float h = min (dt, 0.018);                          // explicit diffusion stays stable

        // Semi-Lagrangian advection: fetch from where the liquid came from.
        vec2 vel = velocity (P, flow);
        vec2 from = uv - vel * dt / (2.0 * vec2 (aspect, 1.0));
        vec4 s = texture2D (liquid, from);
        vec4 lap = texture2D (liquid, from + vec2 (texel.x, 0.0)) + texture2D (liquid, from - vec2 (texel.x, 0.0))
                 + texture2D (liquid, from + vec2 (0.0, texel.y)) + texture2D (liquid, from - vec2 (0.0, texel.y))
                 - 4.0 * s;

        // Oil: phase separation (pushes to 0 or 1) against diffusion (the meniscus width).
        float phi = s.g;
        phi += h * (mix (3.0, 10.0, melt) * lap.g + 6.0 * phi * (1.0 - phi) * (2.0 * phi - 1.0));
        phi = mix (phi, oilBase (P, flow), 0.22 * dt);

        // Dye: carried, a little diffusion, slowly topped up from the reservoir.
        float dd = mix (0.0, 6.0, melt);
        float A = s.r + h * dd * lap.r;
        float B = s.b + h * dd * lap.b;
        A = mix (A, dyeBase (P, flow, 0.0), 0.3 * dt);
        B = mix (B, dyeBase (P, flow, 17.0), 0.3 * dt);

        // A hit lets clear spirit run in along the current: it thins the dye in
        // a ragged streak that the flow then carries off (one place per bar).
        // Never a round clear disc - a lit disc in the dark reads as an eye.
        if (drip > 0.001)
        {
            float bar = floor (vj_beat / 4.0);
            vec2 at = (vjHash22 (vec2 (bar, 7.7)) - 0.5) * vec2 (aspect * 1.3, 1.3);
            vec2 dir = normalize (vel + vec2 (1e-4, 0.0));
            vec2 rel = P - at;
            vec2 local = vec2 (dot (rel, dir), dot (rel, vec2 (-dir.y, dir.x)));
            float streak = exp (-local.x * local.x / 0.09 - local.y * local.y / 0.004);
            float ragged = smoothstep (0.35, 0.8, vjNoise (P * 9.0 + bar * 3.1));
            float thin = drip * streak * ragged * dt * 0.9;
            A -= thin * A;
            B -= thin * B;
        }

        gl_FragColor = vec4 (clamp (A, 0.0, 1.5), clamp (phi, 0.0, 1.0), clamp (B, 0.0, 1.5), 0.5);
    }
    else
    {
        vec2 uv = gl_FragCoord.xy / RENDERSIZE;
        vec2 P = vjCentered();
        vec2 sSize = floor (RENDERSIZE * 0.5 + 0.5);
        vec2 tx = 1.0 / sSize;

        vec4 s = texture2D (liquid, uv);

        // Interface: the local slope of the oil phase (per state texel).
        vec2 g = vec2 (texture2D (liquid, uv + vec2 (tx.x, 0.0)).g - texture2D (liquid, uv - vec2 (tx.x, 0.0)).g,
                       texture2D (liquid, uv + vec2 (0.0, tx.y)).g - texture2D (liquid, uv - vec2 (0.0, tx.y)).g) * 0.5;
        // The dome of each oil lens, seen at a coarser scale (for refraction).
        vec2 G = vec2 (texture2D (liquid, uv + vec2 (tx.x * 7.0, 0.0)).g - texture2D (liquid, uv - vec2 (tx.x * 7.0, 0.0)).g,
                       texture2D (liquid, uv + vec2 (0.0, tx.y * 7.0)).g - texture2D (liquid, uv - vec2 (0.0, tx.y * 7.0)).g);

        float dome = smoothstep (0.35, 0.95, s.g);
        vec4 under = texture2D (liquid, uv - G * lens * tx * 16.0);   // the film beneath, magnified

        // Transmission of the lamp through the dyes (Beer-Lambert); the oil has
        // pushed most of the dye aside, a thin refracted film stays under it.
        vec3 kA = vec3 (1.0, 0.72, 0.30) * (1.25 - 0.7 * tint);
        vec3 kB = vec3 (0.28, 0.62, 1.0) * (0.55 + 0.7 * tint);
        float dens = density * (1.0 - 0.5 * heat);
        vec3 water = exp (-dens * 1.6 * (s.r * kA + s.b * kB));
        vec3 film = exp (-dens * 0.6 * (under.r * kA + under.b * kB)) * mix (0.02, 0.4, clarity);
        vec3 trans = mix (water, film, dome);

        // Meniscus: signed distance to the interface in output pixels
        // (positive inside the oil), from the phase value and its slope.
        float slope = length (g) * 0.5;                       // phase change per output pixel
        float sd = (s.g - 0.5) / max (slope, 1e-4);
        float near = smoothstep (0.004, 0.02, slope);
        float meniscus = exp (-sd * sd / 1.6) * near;
        float caustic = exp (-(sd - 2.6) * (sd - 2.6) / 3.5) * near;

        // A dim projector lamp: a broad, slowly wandering hot spot, never a disc.
        vec2 lc = vec2 (0.4 * sin (flow * 0.37), 0.22 * cos (flow * 0.29));
        float lampF = 0.3 + 0.7 * exp (-dot (P - lc, P - lc) * 0.3);
        vec3 lampCol = vec3 (1.0, 0.93, 0.82);

        vec3 col = lampCol * lampF * trans;
        col *= 1.0 - 0.8 * meniscus;
        col += lampCol * lampF * caustic * rim * (0.6 + 0.6 * heat) * (0.3 + 2.0 * vjLuma (film));

        col = mix (vec3 (vjLuma (col)), col, 0.6);            // muted, like old gel
        gl_FragColor = vec4 (col * lamp * 0.9, 1.0);
    }
}
