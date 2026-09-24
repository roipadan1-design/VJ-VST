/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Pylon: a lattice transmission tower seen from the ground, drawn as a daylight photograph (bright overcast sky, dark steel with lit rims, a grated platform). Meant to feed the Negative Split treatment. Kicks can jump-cut the camera to a new angle (cuts), otherwise it orbits slowly.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "orbit", "TYPE": "float", "DEFAULT": 0.0, "MIN": -100000.0, "MAX": 100000.0 },
    { "NAME": "fov", "TYPE": "float", "DEFAULT": 1.1, "MIN": 0.5, "MAX": 2.2 },
    { "NAME": "levels", "TYPE": "float", "DEFAULT": 8.0, "MIN": 4.0, "MAX": 12.0 },
    { "NAME": "beam", "TYPE": "float", "DEFAULT": 0.06, "MIN": 0.02, "MAX": 0.16 },
    { "NAME": "cuts", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "tilt", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "clouds", "TYPE": "float", "DEFAULT": 0.4, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "surge", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 }
  ]
}*/
#include "common.glsl"

// Camera basis, shared by the helpers below.
vec3 camPos, camRight, camUp, camFwd;
float focal;

// Nearest beam so far: coverage, rim light and depth of whichever beam covers the pixel most.
float cover, rim, coverDepth;

vec2 project (vec3 w, out float z)
{
    vec3 d = w - camPos;
    z = dot (d, camFwd);
    return vec2 (dot (d, camRight), dot (d, camUp)) * focal / max (z, 1e-3);
}

// Shades one projected beam (screen points A, B at depths z1, z2) as a dark
// cylinder with a lit rim on the side that faces the (screen-space) sun.
void seg2 (vec2 p, vec2 A, vec2 B, float z1, float z2, float r, float px)
{
    vec2 ba = B - A;
    float h = clamp (dot (p - A, ba) / max (dot (ba, ba), 1e-9), 0.0, 1.0);
    vec2 off = p - A - ba * h;
    float z = mix (z1, z2, h);
    float halfW = r * focal / z;
    float w = max (halfW, px * 0.5);
    float d2 = dot (off, off);
    if (d2 > (w + px) * (w + px))
        return;                                     // most pixels leave here
    vec2 n = normalize (vec2 (-ba.y, ba.x) + 1e-9);
    float across = dot (off, n);
    float c = (1.0 - smoothstep (w - px * 0.5, w + px * 0.5, abs (across))) * clamp (halfW / (px * 0.5), 0.15, 1.0);
    if (c > cover || (c > 0.5 && z < coverDepth))
    {
        float side = sign (dot (n, vec2 (0.6, 0.8)));
        // Only beams a few pixels wide show a lit rim; hairlines stay plain dark lines.
        rim = smoothstep (0.2, 0.9, across / w * side) * smoothstep (0.8 * px, 3.5 * px, halfW);
        cover = max (cover, c);
        coverDepth = z;
    }
}

// A beam from a to b in world space, clipped at the near plane first.
void strut (vec2 p, vec3 a, vec3 b, float r, float px)
{
    const float nearZ = 0.08;
    float za = dot (a - camPos, camFwd), zb = dot (b - camPos, camFwd);
    if (za < nearZ && zb < nearZ)
        return;
    if (za < nearZ) a = mix (a, b, (nearZ - za) / (zb - za));
    if (zb < nearZ) b = mix (b, a, (nearZ - zb) / (za - zb));
    float z1, z2;
    vec2 A = project (a, z1), B = project (b, z2);
    seg2 (p, A, B, z1, z2, r, px);
}

vec3 corner (int k, float y, float height)
{
    float w = 1.6 * (1.0 - 0.78 * y / height);
    float sx = (k == 0 || k == 3) ? 1.0 : -1.0;
    float sz = k < 2 ? 1.0 : -1.0;
    return vec3 (sx * w, y, sz * w);
}

void main()
{
    vec2 p = vjCentered();
    float px = 2.0 / RENDERSIZE.y;

    // Camera pose: a new one on every Nth kick (vj_seed counts kicks), none when cuts = 0.
    float every = floor (mix (9.0, 1.0, cuts));
    float pose = cuts < 0.02 ? 0.0 : floor (vj_seed / every);
    vec4 h = vec4 (vjHash22 (vec2 (pose, 3.1)), vjHash22 (vec2 (pose, 7.7)));
    float inside = step (0.7, h.x);                      // under the tower (30%) vs out in the field
    float radius = mix (mix (3.5, 7.0, h.y), mix (0.2, 0.9, h.y), inside);
    float angle = orbit + h.z * VJ_TAU;
    float height = 10.0;
    camPos = vec3 (radius * cos (angle), 0.25 + 0.4 * h.w, radius * sin (angle));
    vec3 target = vec3 (0.0, height * mix (0.35, 0.8, tilt) * mix (0.8, 1.15, h.w), 0.0);
    camFwd = normalize (target - camPos);
    float roll = (h.w - 0.5) * 0.9;
    vec3 upHint = vec3 (sin (roll), cos (roll), 0.0);
    camRight = normalize (cross (camFwd, upHint));
    camUp = cross (camRight, camFwd);
    focal = 1.0 / (fov * (1.0 - 0.06 * surge));

    // --- sky: bright overcast, brighter toward the horizon, soft cloud banks
    vec3 rd = normalize (p.x * camRight + p.y * camUp + focal * camFwd);
    float up = clamp (rd.y, 0.0, 1.0);
    float sky = mix (0.95, 0.72, pow (up, 0.6));
    vec2 cuv = rd.xz / max (rd.y, 0.08) * 0.6;
    float cl = vjFbm (cuv * 1.4 + vec2 (orbit * 0.05, 0.0));
    sky -= clouds * 0.35 * smoothstep (0.45, 0.8, cl);

    // --- the tower: a tapered square lattice, legs + girts + X-bracing per
    // level. Corners are projected once per level (4 new points) and the 16
    // beams between them are shaded in 2D; only levels that reach behind the
    // camera take the slower clipped 3D path.
    cover = 0.0; rim = 0.0; coverDepth = 1e9;
    float lv = floor (levels);
    const float nearZ = 0.08;
    vec2 P0[4], P1[4];
    float Z0[4], Z1[4];
    for (int k = 0; k < 4; ++k)
        P0[k] = project (corner (k, 0.0, height), Z0[k]);

    for (int i = 0; i < 12; ++i)
    {
        float fi = float (i);
        if (fi >= lv) break;
        float y0 = height * fi / lv, y1 = height * (fi + 1.0) / lv;
        float r = beam * (1.0 - 0.5 * fi / lv);
        float zmin = 1e9;
        for (int k = 0; k < 4; ++k)
        {
            P1[k] = project (corner (k, y1, height), Z1[k]);
            zmin = min (zmin, min (Z0[k], Z1[k]));
        }

        if (zmin > nearZ)
        {
            for (int k = 0; k < 4; ++k)
            {
                int k1 = k == 3 ? 0 : k + 1;
                seg2 (p, P0[k], P1[k], Z0[k], Z1[k], r * 1.4, px);      // leg
                seg2 (p, P0[k], P0[k1], Z0[k], Z0[k1], r * 0.8, px);   // girt
                seg2 (p, P0[k], P1[k1], Z0[k], Z1[k1], r * 0.6, px);   // X-bracing
                seg2 (p, P0[k1], P1[k], Z0[k1], Z1[k], r * 0.6, px);
            }
        }
        else
        {
            for (int k = 0; k < 4; ++k)
            {
                int k1 = k == 3 ? 0 : k + 1;
                vec3 p00 = corner (k, y0, height), p01 = corner (k1, y0, height);
                vec3 p10 = corner (k, y1, height), p11 = corner (k1, y1, height);
                strut (p, p00, p10, r * 1.4, px);
                strut (p, p00, p01, r * 0.8, px);
                strut (p, p00, p11, r * 0.6, px);
                strut (p, p01, p10, r * 0.6, px);
            }
        }

        for (int k = 0; k < 4; ++k)
        {
            P0[k] = P1[k];
            Z0[k] = Z1[k];
        }
    }

    // Cross-arms near the top, like a transmission pylon.
    float ya = height * 0.74, wa = 1.6 * (1.0 - 0.78 * 0.74);
    for (int s = 0; s < 2; ++s)
    {
        float sx = s == 0 ? 1.0 : -1.0;
        vec3 tip = vec3 (sx * wa * 3.2, ya + 0.35, 0.0);
        strut (p, vec3 (sx * wa, ya, wa), tip, beam * 0.9, px);
        strut (p, vec3 (sx * wa, ya, -wa), tip, beam * 0.9, px);
        strut (p, vec3 (sx * wa, ya + 0.9, 0.0), tip, beam * 0.7, px);
        strut (p, vec3 (sx * wa * 2.1, ya + 0.1, wa * 0.5), vec3 (sx * wa * 2.1, ya + 0.1, -wa * 0.5), beam * 0.5, px);
    }

    // --- grated platform under the arms: dark plate, sky shows through the slots
    float plateY = ya - 0.05;
    float t = (plateY - camPos.y) / max (rd.y, 1e-4);
    vec3 hit = camPos + rd * clamp (t, 0.0, 400.0);
    vec2 q = hit.xz;
    const float spacing = 0.07;
    float fw = max (fwidth (q.x / spacing), 1e-4);             // outside any branch: derivatives stay valid
    float plate = 0.0, grate = 0.0;
    float inPlate = step (abs (q.x), wa * 1.9) * step (abs (q.y), wa * 1.25) * step (1e-4, rd.y) * step (0.0, t);
    float plateDepth = dot (hit - camPos, camFwd);
    if (inPlate > 0.5 && (cover < 0.5 || plateDepth < coverDepth))
    {
        float slot = smoothstep (0.5 - fw, 0.5 + fw, abs (fract (q.x / spacing) - 0.5) * 2.0);
        float edgeFade = clamp (1.0 / (fw * 3.0), 0.0, 1.0);    // slots melt to grey far away
        plate = 1.0;
        grate = mix (0.5, slot, edgeFade);
    }

    // Photograph: steel is dark, rims catch the sun - a glint brighter than
    // the sky itself (the float target keeps it), so the detail layer finds it.
    float steel = 0.08 + 1.5 * rim;
    float photo = sky;
    photo = mix (photo, mix (0.06, 0.55, grate), plate);
    photo = mix (photo, steel, cover);
    photo = max (photo + (vjHash12 (gl_FragCoord.xy + floor (vj_time * 24.0)) - 0.5) * 0.02, 0.0);

    gl_FragColor = vec4 (vec3 (photo), 1.0);
}
