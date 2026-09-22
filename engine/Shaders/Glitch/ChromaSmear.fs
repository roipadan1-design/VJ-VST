/*{
    "DESCRIPTION": "Tri-directional chromatic smear — R, G, B channels are displaced 120 degrees apart so they bleed into each other from different angles, creating a prismatic separation that's more organic than a clean horizontal RGB shift. Amount reacts to the engine's high-frequency uniform.",
    "CREDIT": "internal",
    "CATEGORIES": ["Glitch"],
    "INPUTS": [
        { "NAME": "inputImage", "TYPE": "image" },
        { "NAME": "amount", "TYPE": "float", "DEFAULT": 0.005, "MIN": 0.0, "MAX": 0.05, "LABEL": "Smear Amount" },
        { "NAME": "angle",  "TYPE": "float", "DEFAULT": 0.0,   "MIN": -3.14159, "MAX": 3.14159, "LABEL": "Base Angle (rad)" }
    ]
}*/

#define PI 3.14159265359

void main() {
    vec2 uv = isf_FragNormCoord;
    float a = clamp(amount + high * 0.018, 0.0, 0.08);

    // Three bleed directions, 120deg apart, aspect-corrected
    float aspect = RENDERSIZE.x / RENDERSIZE.y;
    vec2 rDir = vec2(cos(angle),            sin(angle))            * vec2(1.0, aspect);
    vec2 gDir = vec2(cos(angle + 2.0944),   sin(angle + 2.0944))  * vec2(1.0, aspect);
    vec2 bDir = vec2(cos(angle + 4.1888),   sin(angle + 4.1888))  * vec2(1.0, aspect);

    // Normalize directions so diagonal and axis-aligned are equal strength
    rDir = normalize(rDir);
    gDir = normalize(gDir);
    bDir = normalize(bDir);

    float r = IMG_NORM_PIXEL(inputImage, clamp(uv + rDir * a,        vec2(0.0), vec2(1.0))).r;
    float g = IMG_NORM_PIXEL(inputImage, clamp(uv + gDir * a * 0.75, vec2(0.0), vec2(1.0))).g;
    float b = IMG_NORM_PIXEL(inputImage, clamp(uv + bDir * a * 1.25, vec2(0.0), vec2(1.0))).b;

    gl_FragColor = vec4(r, g, b, 1.0);
}
