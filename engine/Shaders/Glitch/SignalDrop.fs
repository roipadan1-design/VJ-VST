/*{
    "DESCRIPTION": "Horizontal signal-dropout bars — random-height bands of the image go to black, imitating analog tape dropout or broadcast signal loss. Intensity reacts to the engine's bass uniform so drops pulse on kicks. A rare band flashes white (head clog artifact).",
    "CREDIT": "internal",
    "CATEGORIES": ["Glitch"],
    "INPUTS": [
        { "NAME": "inputImage", "TYPE": "image" },
        { "NAME": "intensity", "TYPE": "float", "DEFAULT": 0.15, "MIN": 0.0, "MAX": 1.0, "LABEL": "Drop Intensity" },
        { "NAME": "barCount",  "TYPE": "float", "DEFAULT": 10.0, "MIN": 2.0, "MAX": 40.0, "LABEL": "Bar Count" },
        { "NAME": "speed",     "TYPE": "float", "DEFAULT": 0.6,  "MIN": 0.0, "MAX": 4.0,  "LABEL": "Drop Speed" }
    ]
}*/

float hash(float n) { return fract(sin(n) * 43758.5453); }
float hash2(float n) { return fract(sin(n * 1.731) * 12345.678); }

void main() {
    vec2 uv = isf_FragNormCoord;

    float bar  = floor(uv.y * barCount);
    float tick = floor(TIME * speed);

    float r1 = hash(bar + tick * 7.3);
    float r2 = hash2(bar + tick * 3.1 + 0.5);

    float eff = clamp(intensity + bass * 0.5, 0.0, 1.0);

    vec4 col = IMG_NORM_PIXEL(inputImage, uv);

    // Black dropout
    float isDropped = step(1.0 - eff * 0.85, r1);
    // Rare white flash (head clog)
    float isFlash   = step(0.96, r2) * step(0.55, eff);

    col.rgb = mix(col.rgb, vec3(0.0), isDropped);
    col.rgb = mix(col.rgb, vec3(0.95), isFlash);

    gl_FragColor = vec4(col.rgb, 1.0);
}
