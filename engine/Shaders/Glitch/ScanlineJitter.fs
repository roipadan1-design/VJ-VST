/*{
    "DESCRIPTION": "Per-scanline horizontal jitter plus a faint rolling brightness band - VHS/CRT-desync-style glitch building block. Jitter speed follows the engine's audio-reactive 'high' uniform, the rolling band follows 'beatphase'.",
    "CREDIT": "internal",
    "CATEGORIES": ["Glitch"],
    "INPUTS": [
        { "NAME": "inputImage", "TYPE": "image" },
        { "NAME": "jitterAmount", "TYPE": "float", "DEFAULT": 0.02, "MIN": 0.0, "MAX": 0.2, "LABEL": "Jitter Amount" },
        { "NAME": "lineDensity", "TYPE": "float", "DEFAULT": 120.0, "MIN": 20.0, "MAX": 400.0, "LABEL": "Line Density" }
    ]
}*/

float hash(float n)
{
    return fract(sin(n) * 12345.6789);
}

void main()
{
    vec2 uv = isf_FragNormCoord;

    float line = floor(uv.y * lineDensity);
    float speed = 1.0 + high * 6.0;
    float jitter = (hash(line + floor(TIME * speed * 10.0)) - 0.5) * (jitterAmount + high * 0.05);

    vec2 shifted = vec2(uv.x + jitter, uv.y);
    vec3 color = IMG_NORM_PIXEL(inputImage, shifted).rgb;

    // Faint rolling bright band tied to the beat phase.
    float band = smoothstep(0.05, 0.0, abs(fract(uv.y - beatphase) - 0.5) - 0.02);
    color += band * 0.15;

    gl_FragColor = vec4(color, 1.0);
}
