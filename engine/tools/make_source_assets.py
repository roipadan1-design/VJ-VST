"""Generates the engine's default source imagery: original, shaded 3D-looking
masks/skulls/heads (RGBA, transparent background) for the schema-2 presets.

The reference analysis (docs/REFERENCE-ZWOBOT-V3-TRAILER.md) shows the look
comes from strong raw material - masks, skulls, faces - pushed through one bold
treatment. These are procedurally drawn (signed-distance shapes -> height map
-> lit relief) so the project ships its own content; drop your own PNGs into
Media/Images/User (or any folder a preset points at) to use real material.

    python make_source_assets.py            # writes engine/Media/Images/Masks/*.png
"""
import os
import numpy as np
from PIL import Image

SIZE = 1024
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'Media', 'Images', 'Masks')

yy, xx = np.mgrid[0:SIZE, 0:SIZE].astype(np.float32)
X = (xx - SIZE / 2) / (SIZE / 2)   # -1..1, +x right
Y = (SIZE / 2 - yy) / (SIZE / 2)   # -1..1, +y up
rng = np.random.default_rng(7)


# --- signed-distance primitives (negative inside) ---------------------------
def ellipse(cx, cy, rx, ry):
    return (np.sqrt(((X - cx) / rx) ** 2 + ((Y - cy) / ry) ** 2) - 1.0) * min(rx, ry)

def circle(cx, cy, r):
    return np.sqrt((X - cx) ** 2 + (Y - cy) ** 2) - r

def box(cx, cy, hx, hy, r=0.0):
    qx = np.abs(X - cx) - hx + r
    qy = np.abs(Y - cy) - hy + r
    return np.sqrt(np.maximum(qx, 0) ** 2 + np.maximum(qy, 0) ** 2) + np.minimum(np.maximum(qx, qy), 0) - r

def smin(a, b, k):
    h = np.clip(0.5 + 0.5 * (b - a) / k, 0, 1)
    return b * (1 - h) + a * h - k * h * (1 - h)

def union(a, b): return np.minimum(a, b)
def subtract(a, b): return np.maximum(a, -b)


def value_noise(scale, octaves=5):
    out = np.zeros_like(X)
    amp, freq = 0.5, scale
    for _ in range(octaves):
        grid = rng.random((int(freq) + 3, int(freq) + 3)).astype(np.float32)
        gx = (X * 0.5 + 0.5) * freq
        gy = (Y * 0.5 + 0.5) * freq
        ix, iy = np.floor(gx).astype(int), np.floor(gy).astype(int)
        fx, fy = gx - ix, gy - iy
        fx, fy = fx * fx * (3 - 2 * fx), fy * fy * (3 - 2 * fy)
        a, b = grid[iy, ix], grid[iy, ix + 1]
        c, d = grid[iy + 1, ix], grid[iy + 1, ix + 1]
        out += amp * ((a * (1 - fx) + b * fx) * (1 - fy) + (c * (1 - fx) + d * fx) * fy)
        amp *= 0.5
        freq *= 2.0
    return out


def render(sdf, bulge=0.9, detail=0.0, strokes=None, light=(-0.45, 0.55, 0.7), tint=(1.0, 1.0, 1.0)):
    """Height from the SDF (dome profile), lit with a key + rim light."""
    depth = np.clip(-sdf, 0, None)
    height = np.sqrt(np.clip(depth / 0.25, 0, 1)) * bulge
    if detail > 0:
        height += detail * (value_noise(12) - 0.5)
    if strokes is not None:
        height -= 0.12 * strokes  # carved lines

    gy, gx = np.gradient(height)
    n = np.dstack([-gx * SIZE * 0.06, -gy * SIZE * 0.06 * -1, np.ones_like(height)])
    n /= np.linalg.norm(n, axis=2, keepdims=True)
    l = np.array(light, dtype=np.float32)
    l /= np.linalg.norm(l)
    diffuse = np.clip((n * l).sum(axis=2), 0, 1)
    rim = np.clip(1 - n[..., 2], 0, 1) ** 2
    spec = np.clip((n * np.array([0.2, 0.4, 1.0]) / np.linalg.norm([0.2, 0.4, 1.0])).sum(axis=2), 0, 1) ** 24
    shade = 0.12 + 0.78 * diffuse + 0.35 * rim + 0.6 * spec
    if strokes is not None:
        shade *= 1 - 0.85 * strokes

    alpha = np.clip(0.5 - sdf * SIZE * 0.5, 0, 1)  # ~1px antialiased edge
    rgb = np.clip(np.dstack([shade * t for t in tint]), 0, 1)
    return np.dstack([rgb, alpha])


def stroke_line(points, width):
    """Distance field of a polyline -> 0..1 stroke mask."""
    d = np.full_like(X, 10.0)
    for (ax, ay), (bx, by) in zip(points[:-1], points[1:]):
        px, py = X - ax, Y - ay
        vx, vy = bx - ax, by - ay
        h = np.clip((px * vx + py * vy) / (vx * vx + vy * vy), 0, 1)
        d = np.minimum(d, np.sqrt((px - vx * h) ** 2 + (py - vy * h) ** 2))
    return np.clip((width - d) / (width * 0.4), 0, 1)


def skull():
    head = smin(circle(0, 0.18, 0.62), box(0, -0.42, 0.34, 0.26, 0.12), 0.18)
    head = subtract(head, ellipse(-0.25, 0.05, 0.19, 0.22))
    head = subtract(head, ellipse(0.25, 0.05, 0.19, 0.22))
    head = subtract(head, smin(ellipse(0, -0.2, 0.07, 0.11), circle(0, -0.26, 0.05), 0.04))
    for i in range(6):
        head = subtract(head, box(-0.25 + i * 0.1, -0.5, 0.012, 0.09))
    head = subtract(head, box(0, -0.43, 0.32, 0.012))
    cracks = stroke_line([(0.1, 0.78), (0.18, 0.55), (0.12, 0.4), (0.22, 0.28)], 0.012)
    return render(head, 0.95, 0.06, cracks)


def hockey():
    m = ellipse(0, 0, 0.55, 0.78)
    m = subtract(m, ellipse(-0.2, 0.18, 0.12, 0.08))
    m = subtract(m, ellipse(0.2, 0.18, 0.12, 0.08))
    for (hx, hy) in [(-0.3, -0.1), (0.3, -0.1), (-0.15, -0.25), (0.15, -0.25), (0, -0.35), (-0.22, -0.42), (0.22, -0.42),
                     (0, -0.55), (-0.1, 0.45), (0.1, 0.45), (0, 0.6), (-0.35, 0.3), (0.35, 0.3)]:
        m = subtract(m, circle(hx, hy, 0.032))
    ridge = stroke_line([(0, 0.7), (0, -0.05)], 0.02)
    chevrons = stroke_line([(-0.42, 0.5), (-0.3, 0.62), (-0.18, 0.5)], 0.02) + stroke_line([(0.18, 0.5), (0.3, 0.62), (0.42, 0.5)], 0.02)
    return render(m, 0.8, 0.02, np.clip(chevrons, 0, 1) - 0.5 * ridge, tint=(0.95, 1.0, 0.92))


def grin():
    f = circle(0, 0, 0.7)
    f = subtract(f, circle(-0.26, 0.2, 0.14))
    f = subtract(f, circle(0.26, 0.2, 0.14))
    mouth = box(0, -0.3, 0.42, 0.13, 0.12)
    f = subtract(f, mouth)
    f = union(f, box(0, -0.3, 0.38, 0.004))
    teeth = sum(stroke_line([(-0.32 + i * 0.08, -0.2), (-0.32 + i * 0.08, -0.4)], 0.008) for i in range(9))
    graffiti = stroke_line([(-0.55, 0.55), (-0.35, 0.4), (-0.45, 0.3), (-0.2, 0.22)], 0.018)
    return render(f, 0.9, 0.03, np.clip(teeth + graffiti, 0, 1), tint=(1.0, 0.97, 0.95))


def tribal():
    m = smin(ellipse(0, -0.05, 0.45, 0.7), union(box(-0.45, 0.62, 0.05, 0.2, 0.05), box(0.45, 0.62, 0.05, 0.2, 0.05)), 0.12)
    diamond = lambda cx: np.abs(X - cx) * 1.3 + np.abs(Y - 0.18) - 0.14
    m = subtract(subtract(m, diamond(-0.2)), diamond(0.2))
    zig = stroke_line([(-0.3, -0.35), (-0.18, -0.25), (-0.06, -0.35), (0.06, -0.25), (0.18, -0.35), (0.3, -0.25)], 0.03)
    brow = stroke_line([(-0.4, 0.38), (0, 0.46), (0.4, 0.38)], 0.02)
    return render(m, 1.0, 0.08, np.clip(zig + brow, 0, 1), tint=(0.9, 0.95, 1.0))


def alien():
    h = smin(ellipse(0, 0.12, 0.55, 0.6), ellipse(0, -0.45, 0.18, 0.3), 0.3)
    eye = lambda s: ((X * s - 0.22) * np.cos(0.5) + (Y - 0.02) * np.sin(0.5)) ** 2 / 0.05 + (-(X * s - 0.22) * np.sin(0.5) + (Y - 0.02) * np.cos(0.5)) ** 2 / 0.01 - 1.0
    h = subtract(subtract(h, eye(1.0) * 0.1), eye(-1.0) * 0.1)
    return render(h, 1.0, 0.04)


def stone_head():
    h = smin(ellipse(0, 0.05, 0.5, 0.72), box(0, 0.3, 0.52, 0.06, 0.05), 0.1)
    h = subtract(h, box(-0.2, 0.16, 0.12, 0.035, 0.02))
    h = subtract(h, box(0.2, 0.16, 0.12, 0.035, 0.02))
    h = smin(h, box(0, -0.05, 0.06, 0.18, 0.05), 0.05)
    h = subtract(h, box(0, -0.42, 0.2, 0.025, 0.02))
    return render(h, 0.9, 0.18, tint=(0.92, 0.9, 0.86))


def orb():
    o = circle(0, 0, 0.72)
    rings = sum(stroke_line([(np.cos(a) * 0.72, np.sin(a) * 0.72 * 0.3 + k * 0.2), (-np.cos(a) * 0.72, -np.sin(a) * 0.72 * 0.3 + k * 0.2)], 0.01)
                for k, a in [(-2, 0.1), (-1, 0.1), (0, 0.1), (1, 0.1), (2, 0.1)])
    return render(o, 1.2, 0.02, np.clip(rings, 0, 1))


def monolith():
    m = box(0, 0, 0.32, 0.78, 0.04)
    for i in range(7):
        m = subtract(m, box(0, 0.6 - i * 0.2, 0.2, 0.025))
    return render(m, 0.6, 0.05, light=(0.6, 0.4, 0.7))


SHAPES = {
    '01_skull': skull, '02_hockey': hockey, '03_grin': grin, '04_tribal': tribal,
    '05_alien': alien, '06_stone_head': stone_head, '07_orb': orb, '08_monolith': monolith,
}

if __name__ == '__main__':
    os.makedirs(OUT, exist_ok=True)
    for name, make in SHAPES.items():
        rgba = make()
        Image.fromarray((np.clip(rgba, 0, 1) * 255).astype(np.uint8), 'RGBA').save(os.path.join(OUT, name + '.png'), optimize=True)
        print('wrote', name)
    user = os.path.join(OUT, '..', 'User')
    os.makedirs(user, exist_ok=True)
    open(os.path.join(user, 'README.txt'), 'w').write('Drop your own PNG/JPG sources here (transparent PNGs work best).\n')
