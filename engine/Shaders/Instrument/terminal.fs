/*{
  "ISFVSN": "2",
  "DESCRIPTION": "Terminal: lines of an invented machine script scroll over a dark, blotted field - glyphs built from a 5x7 grid, number runs, inverted highlight bars and a block cursor. Kicks jump the scroll and burn a new line in, snares invert a band.",
  "CATEGORIES": ["Generator", "Instrument"],
  "INPUTS": [
    { "NAME": "scroll", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 100000.0 },
    { "NAME": "rows", "TYPE": "float", "DEFAULT": 26.0, "MIN": 10.0, "MAX": 60.0 },
    { "NAME": "fill", "TYPE": "float", "DEFAULT": 0.6, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "jump", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "invert", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "background", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
    { "NAME": "morph", "TYPE": "float", "DEFAULT": 0.0, "MIN": 0.0, "MAX": 1000.0 },
    { "NAME": "emission", "TYPE": "float", "DEFAULT": 1.3, "MIN": 0.2, "MAX": 4.0 }
  ]
}*/
#include "common.glsl"

// A glyph from an invented alphabet: 5x7 bits hashed from its id, with a
// vertical stem and a bar forced in often enough that it reads as writing.
float glyph (vec2 f, float id)
{
    vec2 g = floor (f * vec2 (5.0, 7.0));
    if (f.x < 0.0 || f.x >= 1.0 || f.y < 0.0 || f.y >= 1.0)
        return 0.0;
    float bit = step (0.62, vjHash12 (g + id * 13.7));
    float stem = step (0.5, vjHash12 (vec2 (id, 1.0))) * step (abs (g.x - 1.0), 0.5);
    float bar = step (0.55, vjHash12 (vec2 (id, 2.0))) * step (abs (g.y - floor (vjHash12 (vec2 (id, 3.0)) * 7.0)), 0.5);
    return max (bit, max (stem, bar));
}

void main()
{
    vec2 uv = gl_FragCoord.xy / RENDERSIZE.xy;
    float aspect = RENDERSIZE.x / RENDERSIZE.y;

    // Background: dark ink blots.
    vec2 bp = vec2 (uv.x * aspect, uv.y) * 2.5;
    float blot = vjFbm (bp + vec2 (morph * 0.02, 0.0) + vjFbm (bp * 1.7 - morph * 0.01));
    float bg = smoothstep (0.6, 0.8, blot) * background * 0.14;

    // Text grid: rows scroll upward; a kick jumps the scroll a few lines.
    float rowH = 1.0 / rows;
    float y = uv.y + scroll * rowH + floor (jump * 4.0) * rowH;
    float row = floor (y / rowH);
    float fy = fract (y / rowH);
    float cols = rows * aspect * 1.45;
    float col = floor (uv.x * cols);
    float fx = fract (uv.x * cols);

    float rowSeed = vjHash12 (vec2 (row, 5.0));
    float lineLen = floor (mix (4.0, cols * 0.9, pow (vjHash12 (vec2 (row, 6.0)), 0.7)));
    float indent = floor (vjHash12 (vec2 (row, 7.0)) * 4.0) * 2.0;
    float present = step (rowSeed, fill) * step (indent, col) * step (col, indent + lineLen);
    // Word gaps.
    present *= step (0.14, vjHash12 (vec2 (col, row) * 0.37 + 1.0));

    float numeric = step (0.8, vjHash12 (vec2 (row, 9.0)));
    float id = floor (vjHash12 (vec2 (col, row)) * (numeric > 0.5 ? 10.0 : 40.0)) + numeric * 100.0;
    float ink = glyph (vec2 (fx * 1.3 - 0.12, fy * 1.25 - 0.1), id) * present;

    // Highlighted lines are inverted bars.
    float highlight = step (0.93, vjHash12 (vec2 (row, 11.0))) * step (indent, col) * step (col, indent + lineLen);
    ink = mix (ink, 1.0 - ink, highlight);

    // Block cursor at the end of the newest line, blinking on the beat.
    float cursorRow = floor ((1.0 - 0.5 * rowH + scroll * rowH + floor (jump * 4.0) * rowH) / rowH) - 3.0;
    float cursor = step (abs (row - cursorRow), 0.5) * step (abs (col - (indent + lineLen + 1.0)), 0.5) * step (0.5, fract (vj_beat));

    float v = max (bg, max (ink, cursor));
    // Snare: invert a band of the screen.
    float band = step (abs (uv.y - vjHash12 (vec2 (floor (TIME * 4.0), 1.0))), 0.12 * invert);
    v = mix (v, 1.0 - v, band);

    vec3 c = vec3 (1.0, 0.4, 0.3) * v * emission;
    gl_FragColor = vec4 (c, 1.0);
}
