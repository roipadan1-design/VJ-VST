/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Hot blobs: a thresholded liquid field; inside filled with a vivid moving gradient, white rims at the edges, deep black outside.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "scale", "TYPE": "float", "DEFAULT": 1.8, "MIN": 0.5, "MAX": 5.0 },
    { "NAME": "flow", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 },
    { "NAME": "threshold", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.3, "MAX": 0.7 },
    { "NAME": "rim", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 2.0 },
    { "NAME": "gradient", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 },
    { "NAME": "hue", "TYPE": "float", "DEFAULT": 0.55, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "surge", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.2, "MIN": 0.2, "MAX": 4.0 },
    { "NAME": "grain", "TYPE": "float", "DEFAULT": 0.35, "MIN": 0.0, "MAX": 1.0 }
  ]
}*/
#include "common.glsl"

float field (vec2 p, float t)
{
    vec2 q = vec2 (vjFbm (p + vec2 (0.0, t)), vjFbm (p + vec2 (3.7, -t * 0.7)));
    return vjFbm (p + 1.6 * q);
}

void main()
{
    vec2 p = vjCentered();
    vec2 sp = p * scale;
    float f = field (sp, flow);

    // The kick swells the blobs by lowering the threshold for a moment.
    float th = threshold - surge * 0.08;
    float aa = fwidth (f) * 1.5 + 1e-4;
    float inside = smoothstep (th - aa, th + aa, f);
    float edge = 1.0 - smoothstep (0.0, aa * 3.0 + 0.004, abs (f - th));

    // Vivid gradient inside, dragged by its own slow flow and the field.
    float g = f * 1.6 + p.y * 0.35 + gradient * 0.15 + vjFbm (sp * 0.7 + gradient * 0.05) * 0.6;
    // Hot ramp: deep magenta -> pink -> orange -> yellow, shifted by hue.
    float h = fract (g * 0.35 + hue);
    vec3 hot = mix (vec3 (0.45, 0.0, 0.25), vec3 (1.0, 0.08, 0.45), smoothstep (0.0, 0.35, h));
    hot = mix (hot, vec3 (1.0, 0.35, 0.02), smoothstep (0.3, 0.65, h));
    hot = mix (hot, vec3 (1.0, 0.85, 0.15), smoothstep (0.6, 0.9, h));
    hot = mix (hot, vec3 (0.45, 0.0, 0.25), smoothstep (0.88, 1.0, h));
    vec3 fill = hot * emission;
    fill *= 0.55 + 0.45 * smoothstep (th, th + 0.2, f);         // darker near the rim
    fill += (vjHash12 (gl_FragCoord.xy + fract (flow) * 97.0) - 0.5) * grain * 0.25 * inside;

    vec3 col = fill * inside;
    col += vec3 (1.0) * edge * rim * (1.0 + 1.5 * surge);        // white rims
    col += vec3 (0.004, 0.003, 0.008);                            // near-black, never grey

    gl_FragColor = vec4 (col, 1.0);
}
