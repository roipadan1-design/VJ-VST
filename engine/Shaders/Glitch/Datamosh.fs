/*{
    "DESCRIPTION": "Feedback-based datamosh-style smear. A PERSISTENT buffer holds the last displayed frame; each new frame is blended against it based on local motion (color difference) and a 'freeze' control, with the held-back content warped along a naive gradient-based fake motion vector so it smears rather than just cross-fading. A generic feedback/glitch technique proving real PERSISTENT-buffer reuse in an EffectChain stage - not a locked-in look. 'motionSmear' also blends with the engine's audio-reactive 'bass' uniform.",
    "CREDIT": "internal",
    "CATEGORIES": ["Glitch"],
    "INPUTS": [
        { "NAME": "inputImage", "TYPE": "image" },
        { "NAME": "freeze", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0, "LABEL": "Freeze Amount" },
        { "NAME": "motionSmear", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0, "LABEL": "Motion Smear" },
        { "NAME": "dragAmount", "TYPE": "float", "DEFAULT": 0.015, "MIN": 0.0, "MAX": 0.05, "LABEL": "Warp Drag" }
    ],
    "PASSES": [
        { "TARGET": "feedback", "PERSISTENT": true },
        {}
    ]
}*/

void main()
{
    vec2 uv = isf_FragNormCoord;

    if (PASSINDEX == 0)
    {
        vec4 newColor = IMG_NORM_PIXEL(inputImage, uv);
        vec4 oldColor = IMG_NORM_PIXEL(feedback, uv); // feedback's content from the previous frame

        float diff = length(newColor.rgb - oldColor.rgb);
        float motion = smoothstep(0.05, 0.4, diff); // 0 = static area, 1 = high local motion

        // Fake "motion vector": local luminance gradient of the new frame,
        // used to warp the feedback lookup so held-back content drags/
        // smears rather than just cross-fading in place.
        vec2 gradient = vec2(
            IMG_NORM_PIXEL(inputImage, uv + vec2(0.003, 0.0)).r - IMG_NORM_PIXEL(inputImage, uv - vec2(0.003, 0.0)).r,
            IMG_NORM_PIXEL(inputImage, uv + vec2(0.0, 0.003)).r - IMG_NORM_PIXEL(inputImage, uv - vec2(0.0, 0.003)).r
        );
        vec2 dragUV = clamp(uv + gradient * dragAmount * motion, 0.0, 1.0);
        vec4 draggedOld = IMG_NORM_PIXEL(feedback, dragUV);

        float reactiveSmear = clamp(motionSmear + bass * 0.3, 0.0, 1.0);
        float holdAmount = clamp(freeze + motion * reactiveSmear, 0.0, 1.0);

        gl_FragColor = vec4(mix(newColor.rgb, draggedOld.rgb, holdAmount), 1.0);
    }
    else
    {
        gl_FragColor = vec4(IMG_NORM_PIXEL(feedback, uv).rgb, 1.0);
    }
}
