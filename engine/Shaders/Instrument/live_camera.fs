/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Live Camera: the webcam (WEBCAM button) or a video file dropped straight on the engine's own window, drawn plainly - a different pathway from the plug-in's MEDIA-tab LOAD button (see Media Raw for that one). No treatment stage: at every knob's default this is exactly what the camera sees. v1 stretches the frame to fill rather than aspect-correct fitting - a plain inputImage binding reports the render target's own size, not the camera/video's native resolution (see ISFShader.cpp's externalImageTexture path), so a correct contain/cover fit would need the capture's real width/height threaded through FrameContext first; documented as a follow-up rather than risking a wrong-aspect crop here.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "inputImage", "TYPE": "image" },
    { "NAME": "gain", "TYPE": "float", "DEFAULT": 1.0, "MIN": 0.4, "MAX": 2.0 },
    { "NAME": "mirror", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "zoom", "TYPE": "float", "DEFAULT": 1.0, "MIN": 0.6, "MAX": 2.5 },
    { "NAME": "fade", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "sharpen", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 }
  ]
}*/
#include "common.glsl"

void main()
{
    // Crop-zoom around centre (bigger zoom = more magnified), then an
    // optional horizontal mirror. At the defaults (zoom 1, mirror 0) this is
    // the identity: uv == isf_FragNormCoord, no crop, no flip.
    vec2 zuv = clamp ((isf_FragNormCoord - 0.5) / max (zoom, 1e-4) + 0.5, 0.0, 1.0);
    vec2 uv = mix (zuv, vec2 (1.0 - zuv.x, zuv.y), mirror);
    vec3 rgb = IMG_NORM_PIXEL (inputImage, uv).rgb;

    // Detail: a small unsharp mask against a 4-tap cross blur. Zero at rest.
    if (sharpen > 0.001)
    {
        vec2 texel = 1.0 / max (RENDERSIZE, vec2 (1.0));
        vec3 blur = (IMG_NORM_PIXEL (inputImage, uv + vec2 (texel.x, 0.0)).rgb +
                     IMG_NORM_PIXEL (inputImage, uv - vec2 (texel.x, 0.0)).rgb +
                     IMG_NORM_PIXEL (inputImage, uv + vec2 (0.0, texel.y)).rgb +
                     IMG_NORM_PIXEL (inputImage, uv - vec2 (0.0, texel.y)).rgb) * 0.25;
        rgb = clamp (rgb + (rgb - blur) * sharpen * 2.0, 0.0, 1.0);
    }

    // Erode: a worn, faded print - desaturate toward its own luma and lift
    // the blacks a touch. Zero at rest.
    if (fade > 0.001)
    {
        vec3 worn = mix (rgb, vec3 (vjLuma (rgb)), 0.5);
        worn = mix (worn, vec3 (1.0), 0.06);
        rgb = mix (rgb, worn, fade);
    }

    gl_FragColor = vec4 (clamp (rgb * gain, 0.0, 4.0), 1.0);
}
