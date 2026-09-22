/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Feedback smear (effect): drags the incoming image through a flowing, slowly zooming memory buffer with chromatic edges - melted, liquid trails. Frame-rate independent decay.",
  "CATEGORIES": ["Effect", "Instrument"],
  "INPUTS": [
    { "NAME": "inputImage", "TYPE": "image" },
    { "NAME": "persistence", "TYPE": "float", "DEFAULT": 0.8, "MIN": 0.05, "MAX": 4.0 },
    { "NAME": "flow", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 },
    { "NAME": "push", "TYPE": "float", "DEFAULT": 0.004, "MIN": 0.0, "MAX": 0.03 },
    { "NAME": "zoom", "TYPE": "float", "DEFAULT": 0.004, "MIN": -0.03, "MAX": 0.03 },
    { "NAME": "aberration", "TYPE": "float", "DEFAULT": 0.002, "MIN": 0.0, "MAX": 0.02 },
    { "NAME": "inject", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "flush", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 }
  ],
  "PASSES": [
    { "TARGET": "memory", "PERSISTENT": true, "FLOAT": true },
    { }
  ]
}*/
#include "common.glsl"

void main()
{
    vec2 uv = isf_FragNormCoord;

    if (PASSINDEX == 0)
    {
        // Advect last frame's memory along a slowly turning noise flow.
        vec2 c = uv - 0.5;
        vec2 n = vec2 (vjFbm (uv * 3.0 + flow * 0.1), vjFbm (uv * 3.0 + vec2 (7.3, 1.1) - flow * 0.1)) - 0.5;
        vec2 from = 0.5 + c * (1.0 - zoom) - n * push;
        vec3 mem;
        mem.r = texture2D (memory, from + vec2 (aberration, 0.0)).r;
        mem.g = texture2D (memory, from).g;
        mem.b = texture2D (memory, from - vec2 (aberration, 0.0)).b;

        // Decay by a time constant, not per frame: trails last the same at 30 or 144 fps.
        float retain = exp (-TIMEDELTA / persistence) * (1.0 - flush);
        vec3 src = IMG_NORM_PIXEL (inputImage, uv).rgb;
        vec3 outc = max (mem * retain, src * inject);
        gl_FragColor = vec4 (min (outc, vec3 (64.0)), 1.0);   // bounded: no runaway HDR
    }
    else
    {
        vec3 src = IMG_NORM_PIXEL (inputImage, uv).rgb;
        vec3 mem = IMG_NORM_PIXEL (memory, uv).rgb;
        gl_FragColor = vec4 (max (src, mem), 1.0);
    }
}
