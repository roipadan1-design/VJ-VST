/*{
    "DESCRIPTION": "Chromatic-aberration-style RGB channel split. A generic, well-known glitch building block - not a locked-in aesthetic, just a chain stage to layer with others.",
    "CREDIT": "internal",
    "CATEGORIES": ["Glitch"],
    "INPUTS": [
        { "NAME": "inputImage", "TYPE": "image" },
        { "NAME": "amount", "TYPE": "float", "DEFAULT": 0.01, "MIN": 0.0, "MAX": 0.2, "LABEL": "Shift Amount" }
    ]
}*/

void main()
{
    vec2 uv = isf_FragNormCoord;
    vec2 offset = vec2(amount, 0.0);

    float r = IMG_NORM_PIXEL(inputImage, uv + offset).r;
    float g = IMG_NORM_PIXEL(inputImage, uv).g;
    float b = IMG_NORM_PIXEL(inputImage, uv - offset).b;

    gl_FragColor = vec4(r, g, b, 1.0);
}
