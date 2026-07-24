/*{
    "DESCRIPTION": "Randomly displaces horizontal block-rows sideways - a generic datamosh/block-shift glitch building block. Static 'intensity' blends with the engine's audio-reactive 'bass' uniform, so it pulses harder on kicks even without a preset audioMapping.",
    "CREDIT": "internal",
    "CATEGORIES": ["Glitch"],
    "INPUTS": [
        { "NAME": "inputImage", "TYPE": "image" },
        { "NAME": "intensity", "TYPE": "float", "DEFAULT": 0.3, "MIN": 0.0, "MAX": 1.0, "LABEL": "Intensity" },
        { "NAME": "blockCount", "TYPE": "float", "DEFAULT": 24.0, "MIN": 4.0, "MAX": 64.0, "LABEL": "Block Rows" }
    ]
}*/

float hash(float n)
{
    return fract(sin(n) * 43758.5453123);
}

void main()
{
    vec2 uv = isf_FragNormCoord;

    float rowIndex = floor(uv.y * blockCount);
    float seed = rowIndex + floor(TIME * 12.0);
    float amount = clamp(intensity + bass * 0.6, 0.0, 1.0);

    float jitter = (hash(seed) - 0.5) * amount;
    // Only displace some rows, not all - keeps this reading as a glitch, not a shear.
    jitter *= step(0.6, hash(seed * 1.37));

    vec2 displaced = vec2(uv.x + jitter, uv.y);
    gl_FragColor = vec4(IMG_NORM_PIXEL(inputImage, displaced).rgb, 1.0);
}
