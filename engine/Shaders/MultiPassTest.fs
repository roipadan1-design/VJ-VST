/*{
    "DESCRIPTION": "Internal diagnostic: pass 0 writes a checkerboard into bufferA, pass 1 samples bufferA and inverts it. Proves the multi-pass FBO chain, not a real preset.",
    "CREDIT": "internal test",
    "CATEGORIES": ["Test"],
    "INPUTS": [],
    "PASSES": [
        { "TARGET": "bufferA" },
        {}
    ]
}*/

void main()
{
    vec2 uv = isf_FragNormCoord;

    if (PASSINDEX == 0)
    {
        vec2 grid = floor(uv * 8.0);
        float checker = mod(grid.x + grid.y, 2.0);
        gl_FragColor = vec4(checker, 1.0 - checker, 0.0, 1.0);
    }
    else
    {
        vec4 prev = IMG_NORM_PIXEL(bufferA, uv);
        gl_FragColor = vec4(1.0 - prev.rgb, 1.0);
    }
}
