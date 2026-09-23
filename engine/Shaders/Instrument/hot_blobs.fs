/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Hot blobs, liquid: a thresholded fluid mass that behaves like water being hit by sound. Each kick throws the mass (momentum), gravity pulls it back down and viscosity slows it; the hit also drops a stone that sends rings across the surface, refracting the blobs as they pass.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "scale", "TYPE": "float", "DEFAULT": 1.8, "MIN": 0.5, "MAX": 5.0 },
    { "NAME": "flow", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 },
    { "NAME": "threshold", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.3, "MAX": 0.7 },
    { "NAME": "drop", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "impact", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 2.0 },
    { "NAME": "gravity", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 2.0 },
    { "NAME": "viscosity", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "ripple", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 1.5 },
    { "NAME": "rim", "TYPE": "float", "DEFAULT": 0.7, "MIN": 0.0, "MAX": 2.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.2, "MAX": 4.0 }
  ],
  "PASSES": [
    { "TARGET": "state", "PERSISTENT": true, "FLOAT": true, "WIDTH": "0.02", "HEIGHT": "0.02" },
    { }
  ]
}*/
#include "common.glsl"

// "state" is a tiny float buffer read texel by texel (row 0):
//   texel 0 = (position.xy, velocity.xy) of the mass
//   texel 1 = (previous drop value, drop counter, -, -)
//   texel 2..5 = the last four stones: (centre.xy, birth time, strength)

#define NUM_RINGS 4

vec4 readTexel (float i, vec2 size) { return texture2D (state, vec2 (i + 0.5, 0.5) / size); }

// Height of the rings at p (centred coordinates).
float rings (vec2 p, vec2 size)
{
    float h = 0.0;
    float speed = mix (0.75, 0.3, viscosity);
    float fade = mix (0.5, 1.6, viscosity);
    for (int i = 0; i < NUM_RINGS; ++i)
    {
        vec4 r = readTexel (2.0 + float (i), size);
        float age = TIME - r.z;
        if (r.w <= 0.0 || age < 0.0 || age > 5.0)
            continue;
        float d = length (p - r.xy) - age * speed;
        float width = 0.06 + 0.12 * age;
        float wave = sin (d * 38.0 / (1.0 + age)) * exp (-d * d / (width * width));
        h += r.w * wave * exp (-age * fade) / (1.0 + 3.0 * age);
    }
    return h;
}

void main()
{
    if (PASSINDEX == 0)
    {
        vec2 size = RENDERSIZE;
        float i = floor (gl_FragCoord.x);
        vec4 out4 = readTexel (i, size);
        if (gl_FragCoord.y > 1.0 || i > 5.0)
        {
            gl_FragColor = vec4 (0.0);
            return;
        }

        float dt = clamp (TIMEDELTA, 0.0, 0.05);
        vec4 book = readTexel (1.0, size);
        bool newDrop = drop > 0.15 && book.x <= 0.15;

        if (i < 0.5)
        {
            // The mass: a hit throws it up (and a little sideways), gravity
            // brings it back down, viscosity bleeds the motion away.
            vec2 pos = out4.xy, vel = out4.zw;
            float side = vjHash12 (vec2 (vj_seed, 9.1)) - 0.5;
            vel += vec2 (side * 0.8, 1.0) * drop * impact * 12.0 * dt;
            vel.y -= gravity * 1.6 * dt;
            vel *= exp (-dt * mix (0.4, 4.5, viscosity));
            pos += vel * dt;
            pos = mod (pos + 500.0, 1000.0) - 500.0;
            out4 = vec4 (pos, vel);
        }
        else if (i < 1.5)
        {
            out4 = vec4 (drop, book.y + (newDrop ? 1.0 : 0.0), 0.0, 0.0);
        }
        else if (newDrop && abs (i - 2.0 - mod (book.y, float (NUM_RINGS))) < 0.5)
        {
            // A new stone lands somewhere on screen (vj_seed is reseeded on kicks).
            float aspect = 16.0 / 9.0;
            vec2 at = (vjHash22 (vec2 (vj_seed, 3.7)) * 2.0 - 1.0) * vec2 (aspect * 0.75, 0.7);
            out4 = vec4 (at, TIME, impact);
        }
        gl_FragColor = out4;
    }
    else
    {
        vec2 p = vjCentered();
        vec2 stateSize = floor (RENDERSIZE * 0.02 + 0.5);
        vec4 motion = readTexel (0.0, stateSize);
        vec2 pos = motion.xy, vel = motion.zw;

        // Rings: height + screen-space slope; the slope refracts the mass.
        float wh = rings (p, stateSize) * ripple;
        vec2 grad = vec2 (dFdx (wh), dFdy (wh)) * RENDERSIZE.y * 0.5;

        // The mass: domain-warped noise, moved by the simulated position and
        // stretched along its velocity (it smears into drips when it falls fast).
        vec2 q = p - grad * 0.08;
        float speed = length (vel);
        vec2 dir = speed > 1e-4 ? vel / speed : vec2 (0.0, 1.0);
        float stretch = 1.0 / (1.0 + speed * 0.35);
        vec2 along = dir * dot (q, dir);
        q = along * stretch + (q - along);
        vec2 sp = q * scale - pos * scale * 0.6;
        vec2 warp = vec2 (vjFbm (sp + vec2 (0.0, flow)), vjFbm (sp + vec2 (3.7, -flow * 0.7)));
        float f = vjFbm (sp + 1.6 * warp) + wh * 0.04;

        float th = threshold - drop * impact * 0.05;
        float aa = fwidth (f) * 1.5 + 1e-4;
        float inside = smoothstep (th - aa, th + aa, f);
        float edge = 1.0 - smoothstep (0.0, aa * 3.0 + 0.004, abs (f - th));

        // Liquid shading: the field is a height, lit like a glossy surface.
        vec2 fg = vec2 (dFdx (f), dFdy (f)) * RENDERSIZE.y * 0.12 + grad * 0.15;
        vec3 nrm = normalize (vec3 (-fg, 1.0));
        vec3 key = normalize (vec3 (-0.4, 0.6, 0.7));
        float diffuse = clamp (dot (nrm, key), 0.0, 1.0);
        float spec = pow (clamp (dot (reflect (-key, nrm), vec3 (0.0, 0.0, 1.0)), 0.0, 1.0), 28.0);
        float depth = smoothstep (th, th + 0.22, f);

        float body = (0.18 + 0.55 * depth) * (0.45 + 0.7 * diffuse);
        vec3 warm = vec3 (1.0, 0.32, 0.22);
        vec3 col = warm * body * emission * inside;
        col += vec3 (1.0, 0.85, 0.8) * spec * 1.6 * inside;
        col += vec3 (1.0, 0.7, 0.6) * edge * rim;

        // Rings on the black: the crests catch the light.
        col += warm * clamp (wh, 0.0, 1.0) * 0.5 * (1.0 - inside);

        col += vec3 (0.004, 0.002, 0.002);
        gl_FragColor = vec4 (col, 1.0);
    }
}
