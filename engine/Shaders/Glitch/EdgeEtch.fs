/*{
    "DESCRIPTION": "Sobel edge extraction — turns the source image into glowing structural lines on a crushable dark field. At low threshold many fine edges appear; at high threshold only the strongest outlines survive. The non-edge field darkens independently (darkness). A slight blue-grey tint on the edges reads as forensic/clinical.",
    "CREDIT": "internal",
    "CATEGORIES": ["Glitch"],
    "INPUTS": [
        { "NAME": "inputImage", "TYPE": "image" },
        { "NAME": "threshold", "TYPE": "float", "DEFAULT": 0.18, "MIN": 0.0, "MAX": 1.0, "LABEL": "Edge Threshold" },
        { "NAME": "glow",      "TYPE": "float", "DEFAULT": 0.35, "MIN": 0.0, "MAX": 1.0, "LABEL": "Edge Glow" },
        { "NAME": "darkness",  "TYPE": "float", "DEFAULT": 0.65, "MIN": 0.0, "MAX": 1.0, "LABEL": "Field Darkness" }
    ]
}*/

float lum(vec2 uv) {
    vec4 c = IMG_NORM_PIXEL(inputImage, clamp(uv, vec2(0.0), vec2(1.0)));
    return dot(c.rgb, vec3(0.299, 0.587, 0.114));
}

void main() {
    vec2 uv  = isf_FragNormCoord;
    vec2 px  = 1.0 / RENDERSIZE;

    // 3x3 Sobel
    float tl = lum(uv + vec2(-1,-1)*px);
    float tc = lum(uv + vec2( 0,-1)*px);
    float tr = lum(uv + vec2( 1,-1)*px);
    float ml = lum(uv + vec2(-1, 0)*px);
    float mr = lum(uv + vec2( 1, 0)*px);
    float bl = lum(uv + vec2(-1, 1)*px);
    float bc = lum(uv + vec2( 0, 1)*px);
    float br = lum(uv + vec2( 1, 1)*px);

    float gx = -tl - 2.0*ml - bl + tr + 2.0*mr + br;
    float gy = -tl - 2.0*tc - tr + bl + 2.0*bc + br;
    float edge = length(vec2(gx, gy));

    float mask = smoothstep(threshold - 0.04, threshold + 0.04, edge);

    // Field: original image, crushed toward black
    vec4 orig  = IMG_NORM_PIXEL(inputImage, uv);
    float field = dot(orig.rgb, vec3(0.299, 0.587, 0.114)) * (1.0 - darkness);

    // Edge glow also reacts to the engine's high-frequency uniform
    float glowBoost = glow + high * 0.4;
    float brightness = max(field, mask * (0.8 + glowBoost * 0.5));

    // Forensic blue-grey tint on edge regions
    vec3 edgeTint = vec3(brightness * 0.72, brightness * 0.83, brightness);
    vec3 col = mix(vec3(brightness), edgeTint, mask * 0.65);

    gl_FragColor = vec4(col, 1.0);
}
