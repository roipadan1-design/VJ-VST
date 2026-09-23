"""Generates the engine's default source imagery: abstract 3D forms (RGBA,
transparent background) for the schema-2 presets - nothing figurative, no
faces or masks, just objects with strong silhouettes and deep relief that
survive a hard threshold / dot treatment.

Each form is a signed-distance field raymarched on the CPU (numpy), lit with
a key light, a rim light and cheap ambient occlusion, then written as a
grey-scale RGBA PNG. The engine's look pass colours everything, so the forms
only carry light and shape.

    python make_source_assets.py            # writes engine/Media/Images/Forms/*.png
"""
import os
import numpy as np
from PIL import Image

SIZE = 1024
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'Media', 'Images', 'Forms')


# --- SDF helpers on (N,3) point arrays ---------------------------------------
def length(v): return np.sqrt((v * v).sum(axis=-1))

def sphere(p, r): return length(p) - r

def box(p, b, r=0.0):
    q = np.abs(p) - (np.asarray(b) - r)
    return length(np.maximum(q, 0)) + np.minimum(q.max(axis=-1), 0) - r

def torus(p, R, r):
    q = np.stack([np.sqrt(p[:, 0] ** 2 + p[:, 2] ** 2) - R, p[:, 1]], axis=-1)
    return length(q) - r

def cyl_z(p, r): return np.sqrt(p[:, 0] ** 2 + p[:, 1] ** 2) - r

def smin(a, b, k):
    h = np.clip(0.5 + 0.5 * (b - a) / k, 0, 1)
    return b * (1 - h) + a * h - k * h * (1 - h)

def rot(p, ax, a):
    c, s = np.cos(a), np.sin(a)
    q = p.copy()
    i, j = {'x': (1, 2), 'y': (0, 2), 'z': (0, 1)}[ax]
    q[:, i] = c * p[:, i] - s * p[:, j]
    q[:, j] = s * p[:, i] + c * p[:, j]
    return q

def gyroid(p, f):
    q = p * f
    return (np.sin(q[:, 0]) * np.cos(q[:, 1]) + np.sin(q[:, 1]) * np.cos(q[:, 2]) + np.sin(q[:, 2]) * np.cos(q[:, 0])) / f


# --- the forms -----------------------------------------------------------------
def gyroid_core(p):
    """Sphere hollowed into a gyroid lattice, with a solid inner core."""
    p = rot(rot(p, 'y', 0.5), 'x', 0.35)
    shell = np.maximum(sphere(p, 0.95), np.abs(gyroid(p, 7.0)) - 0.035)
    return np.minimum(shell, sphere(p, 0.42))

def twisted_ring(p):
    """Thick torus twisted along its length, cut with ribs."""
    p = rot(rot(p, 'x', 1.1), 'z', 0.3)
    a = np.arctan2(p[:, 2], p[:, 0])
    q = p.copy()
    rr = np.sqrt(p[:, 0] ** 2 + p[:, 2] ** 2) - 0.62
    c, s = np.cos(a * 1.5), np.sin(a * 1.5)
    x2, y2 = c * rr - s * p[:, 1], s * rr + c * p[:, 1]
    d = np.maximum(np.abs(x2) - 0.2, np.abs(y2) - 0.09)
    ribs = np.abs(np.sin(a * 22.0)) * 0.04 - 0.012
    return np.maximum(d, -ribs) * 0.7

def fractured_shell(p):
    """Sphere shell broken into drifting slabs."""
    p = rot(p, 'y', 0.6)
    out = np.full(len(p), 10.0)
    rng = np.random.default_rng(3)
    for i in range(7):
        n = rng.normal(size=3); n /= np.linalg.norm(n)
        off = n * (0.05 + 0.1 * rng.random())
        q = p - off
        shell = np.abs(sphere(q, 0.82)) - 0.07
        slab = np.abs((q @ n) - (i - 3) * 0.22) - 0.1
        out = np.minimum(out, np.maximum(shell, slab))
    return out

def cluster(p):
    """Viscous cluster of merging spheres - frozen liquid."""
    p = rot(p, 'z', 0.2)
    rng = np.random.default_rng(11)
    d = np.full(len(p), 10.0)
    for _ in range(9):
        c = np.clip(rng.normal(size=3), -1.6, 1.6) * np.array([0.3, 0.3, 0.25])
        d = smin(d, sphere(p - c, 0.18 + 0.2 * rng.random()), 0.22)
    return d

def perforated_slab(p):
    """Tilted thick slab drilled with a grid of holes."""
    p = rot(rot(p, 'y', 0.55), 'x', -0.4)
    d = box(p, (0.62, 0.85, 0.12), 0.03)
    q = p.copy()
    q[:, 0] = np.mod(p[:, 0] + 0.1, 0.2) - 0.1
    q[:, 1] = np.mod(p[:, 1] + 0.1, 0.2) - 0.1
    return np.maximum(d, -cyl_z(q, 0.055))

def ribbon(p):
    """A wide band folded and twisted through space."""
    p = rot(p, 'x', 0.3)
    t = p[:, 1] * 2.2
    c, s = np.cos(t), np.sin(t)
    q = p.copy()
    q[:, 0] = c * p[:, 0] - s * p[:, 2]
    q[:, 2] = s * p[:, 0] + c * p[:, 2]
    q[:, 0] += 0.18 * np.sin(p[:, 1] * 3.0)
    return box(q, (0.42, 0.9, 0.035), 0.02) * 0.6

def sponge(p):
    """Two iterations of a Menger sponge, turned on its corner."""
    p = rot(rot(p, 'y', 0.78), 'x', 0.6)
    d = box(p, (0.6, 0.6, 0.6))
    s = 1.0
    for _ in range(3):
        a = np.mod(p * s / 0.6 * 1.0 + 1.0, 2.0) - 1.0
        s *= 3.0
        r = np.abs(1.0 - 3.0 * np.abs(a))
        da = np.maximum(r[:, 0], r[:, 1]); db = np.maximum(r[:, 1], r[:, 2]); dc = np.maximum(r[:, 2], r[:, 0])
        c = (np.minimum(da, np.minimum(db, dc)) - 1.0) / s * 0.6
        d = np.maximum(d, c)
    return d

def spine(p):
    """Discs stacked along a bending curve, fused together."""
    d = np.full(len(p), 10.0)
    for i in range(11):
        t = (i - 5) / 5.0
        c = np.array([0.35 * np.sin(t * 2.2), t * 0.85, 0.2 * np.cos(t * 1.7)])
        q = rot(p - c, 'z', t * 0.9)
        disc = np.maximum(np.sqrt(q[:, 0] ** 2 + q[:, 2] ** 2) - (0.34 - 0.12 * abs(t)), np.abs(q[:, 1]) - 0.035)
        d = smin(d, disc, 0.05)
    return d


# --- renderer ------------------------------------------------------------------
def render(sdf, steps=110):
    yy, xx = np.mgrid[0:SIZE, 0:SIZE].astype(np.float32)
    u = (xx - SIZE / 2) / (SIZE / 2)
    v = (SIZE / 2 - yy) / (SIZE / 2)
    ro = np.array([0.0, 0.0, 3.2])
    rd = np.stack([u.ravel(), v.ravel(), np.full(u.size, -2.7)], axis=-1)
    rd /= length(rd)[:, None]

    t = np.full(u.size, 1.6)
    hit = np.zeros(u.size, bool)
    alive = np.ones(u.size, bool)
    for _ in range(steps):
        idx = np.nonzero(alive)[0]
        if idx.size == 0:
            break
        pos = ro + rd[idx] * t[idx, None]
        d = sdf(pos)
        t[idx] += d * 0.8
        done = d < 5e-4
        hit[idx[done]] = True
        alive[idx[done | (t[idx] > 5.0)]] = False

    rgb = np.zeros((u.size, 3), np.float32)
    idx = np.nonzero(hit)[0]
    p = ro + rd[idx] * t[idx, None]
    e = 1e-3
    n = np.stack([sdf(p + [e, 0, 0]) - sdf(p - [e, 0, 0]),
                  sdf(p + [0, e, 0]) - sdf(p - [0, e, 0]),
                  sdf(p + [0, 0, e]) - sdf(p - [0, 0, e])], axis=-1)
    n /= length(n)[:, None] + 1e-9

    key = np.array([-0.5, 0.7, 0.6]); key /= np.linalg.norm(key)
    back = np.array([0.7, -0.2, -0.6]); back /= np.linalg.norm(back)
    diffuse = np.clip(n @ key, 0, 1)
    rim = np.clip(1 + (n * rd[idx]).sum(-1), 0, 1) ** 3
    backlight = np.clip(n @ back, 0, 1)
    h = (key - rd[idx]); h /= length(h)[:, None]
    spec = np.clip((n * h).sum(-1), 0, 1) ** 40

    ao = np.ones(len(idx), np.float32)
    for k in range(1, 6):
        dist = 0.04 * k
        ao -= (dist - sdf(p + n * dist)) * (0.5 ** k) * 6.0
    ao = np.clip(ao, 0, 1)

    shade = (0.06 + 0.8 * diffuse * ao + 0.45 * rim + 0.25 * backlight + 0.7 * spec) * (0.35 + 0.65 * ao)
    rgb[idx] = np.clip(shade, 0, 1)[:, None]
    alpha = hit.astype(np.float32)
    img = np.concatenate([rgb, alpha[:, None]], axis=-1).reshape(SIZE, SIZE, 4)
    # 2x2 box filter on the silhouette for a softer edge
    img[..., 3] = (img[..., 3] + np.roll(img[..., 3], 1, 0) + np.roll(img[..., 3], 1, 1) + np.roll(np.roll(img[..., 3], 1, 0), 1, 1)) / 4
    return img


FORMS = {
    '01_gyroid_core': gyroid_core, '02_twisted_ring': twisted_ring, '03_fractured_shell': fractured_shell,
    '04_cluster': cluster, '05_perforated_slab': perforated_slab, '06_ribbon': ribbon,
    '07_sponge': sponge, '08_spine': spine,
}

if __name__ == '__main__':
    import sys
    os.makedirs(OUT, exist_ok=True)
    only = set(sys.argv[1:])
    for name, sdf in FORMS.items():
        if only and name not in only:
            continue
        rgba = render(sdf)
        Image.fromarray((np.clip(rgba, 0, 1) * 255).astype(np.uint8), 'RGBA').save(os.path.join(OUT, name + '.png'), optimize=True)
        print('wrote', name, flush=True)
    user = os.path.join(OUT, '..', 'User')
    os.makedirs(user, exist_ok=True)
    open(os.path.join(user, 'README.txt'), 'w').write('Drop your own PNG/JPG sources here (transparent PNGs work best).\n')
