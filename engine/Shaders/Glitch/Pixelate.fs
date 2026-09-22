/*{
    "DESCRIPTION": "Classic mosaic/pixelate block-quantize. A generic, well-known building block - not a locked-in aesthetic, just a chain stage to layer with others.",
    "CREDIT": "internal",
    "CATEGORIES": ["Glitch"],
    "INPUTS": [
        { "NAME": "inputImage", "TYPE": "image" },
        { "NAME": "blockSize", "TYPE": "float", "DEFAULT": 12.0, "MIN": 1.0, "MAX": 120.0, "LABEL": "Block Size (px)" }
    ]
}*/

void main()
{
    vec2 uv = isf_FragNormCoord;
    vec2 pixelSize = max(vec2(blockSize), vec2(1.0)) / RENDERSIZE;

    vec2 quantized = (floor(uv / pixelSize) + 0.5) * pixelSize;
    gl_FragColor = vec4(IMG_NORM_PIXEL(inputImage, quantized).rgb, 1.0);
}
