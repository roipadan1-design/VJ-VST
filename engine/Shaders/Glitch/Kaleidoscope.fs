/*{
    "DESCRIPTION": "Radial kaleidoscope mirror - folds the image into N repeating pie-slice segments around the center. A generic, well-known building block - not a locked-in aesthetic, just a chain stage to layer with others.",
    "CREDIT": "internal",
    "CATEGORIES": ["Glitch"],
    "INPUTS": [
        { "NAME": "inputImage", "TYPE": "image" },
        { "NAME": "segments", "TYPE": "float", "DEFAULT": 6.0, "MIN": 2.0, "MAX": 24.0, "LABEL": "Segments" },
        { "NAME": "twist", "TYPE": "float", "DEFAULT": 0.0, "MIN": -3.14159, "MAX": 3.14159, "LABEL": "Twist (rad)" }
    ]
}*/

#define PI 3.14159265359

void main()
{
    vec2 uv = isf_FragNormCoord - vec2(0.5);
    float radius = length(uv);
    float angle = atan(uv.y, uv.x) + twist;

    float wedge = PI / max(segments, 2.0);
    // Fold into a single wedge, then mirror alternate wedges so edges line up
    // seamlessly (a plain mod() leaves a visible seam at each wedge boundary).
    angle = mod(angle, 2.0 * wedge);
    angle = abs(angle - wedge);

    vec2 folded = vec2(cos(angle), sin(angle)) * radius + vec2(0.5);
    gl_FragColor = vec4(IMG_NORM_PIXEL(inputImage, folded).rgb, 1.0);
}
