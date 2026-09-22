/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Zebra warp: op-art black stripes bent by a flowing field over a violet-cyan gradient, framed by stepped horizontal bars that jump on hits.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "stripes", "TYPE": "float", "DEFAULT": 14.0, "MIN": 4.0, "MAX": 40.0 },
    { "NAME": "warp", "TYPE": "float", "DEFAULT": 1.4, "MIN": 0.0, "MAX": 4.0 },
    { "NAME": "flow", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 },
    { "NAME": "band", "TYPE": "float", "DEFAULT": 0.35, "MIN": 0.1, "MAX": 1.0 },
    { "NAME": "bars", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "hue", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.3, "MAX": 4.0 },
    { "NAME": "twist", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 }
  ]
}*/
#include "common.glsl"

void main()
{
    vec2 p = vjCentered();
    vec2 uv = gl_FragCoord.xy / RENDERSIZE.xy;

    vec2 w = vec2 (vjFbm (p * 1.3 + flow * 0.2), vjFbm (p * 1.3 + vec2 (4.1, 2.7) - flow * 0.15));
    vec2 q = p + warp * (w - 0.5);
    q = vjRot (twist * sin (flow * 0.3) * 0.8) * q;
    float s = sin (q.x * stripes + q.y * stripes * 0.35);
    float aa = fwidth (s) * 1.2;
    float zebra = smoothstep (-aa, aa, s);

    // Violet -> cyan gradient under the stripes.
    vec3 violet = vjToLinear (vec3 (0.55, 0.15, 0.95)), cyan = vjToLinear (vec3 (0.15, 0.85, 1.0));
    vec3 grad = mix (violet, cyan, uv.y);
    grad = mix (grad, vjPalette (uv.y, hue * 5.99) * 1.3, step (0.02, hue) * 0.6);
    grad *= 1.1 + 0.4 * sin (q.y * 2.0 + flow);

    // A central vertical band holds the pattern; stepped bars reach in from the sides.
    float center = step (abs (p.x), band * (RENDERSIZE.x / RENDERSIZE.y));
    float rows = floor (uv.y * 10.0);
    float barLen = vjHash12 (vec2 (rows, floor (vj_beat))) * (0.3 + bars * 0.7);
    float fromLeft = step (uv.x, barLen * 0.5) * step (0.5, vjHash12 (vec2 (rows, 7.0)));
    float fromRight = step (1.0 - barLen * 0.5, uv.x) * step (vjHash12 (vec2 (rows, 7.0)), 0.5);
    float barMask = (fromLeft + fromRight) * step (0.35, fract (uv.y * 10.0)) * step (fract (uv.y * 10.0), 0.8);

    vec3 col = grad * zebra * center;
    col += mix (vjPalette (0.8, hue * 5.99), vec3 (0.8, 0.9, 1.0), 0.3) * barMask * (0.6 + bars);
    gl_FragColor = vec4 (col * emission, 1.0);
}
