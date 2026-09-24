import numpy as np
from scipy import ndimage as ndi
from mock_util import *
import sheet

W, H = 1280, 720
r = Renderer(W, H)
os.makedirs('out/mock', exist_ok=True)
yy, xx = np.mgrid[0:H, 0:W].astype(np.float32)
cx, cy = W/2, H/2
rad = np.hypot(xx-cx, yy-cy) / (H/2)          # 1.0 = half height
ang = np.arctan2(yy-cy, xx-cx)
R_MARK = 0.82                                  # the show's attractor: one ring (never two dots)

def norm(L):
    m = np.percentile(L, 99.7) + 1e-6
    return np.clip(L / m, 0, 1.2)

def fixed_noise(seed, sigma=6):
    rng = np.random.default_rng(seed)
    n = ndi.gaussian_filter(rng.random((H, W)).astype(np.float32), sigma)
    n = (n - n.min()) / (n.max() - n.min())
    return n

HOLES = fixed_noise(11, 5)

def ring_mark(width=0.012):
    return np.exp(-((rad - R_MARK)/width)**2)

def radial_pull(L, k):
    rs = (rad - k*R_MARK) / (1-k)
    sx = cx + np.cos(ang) * rs * (H/2)
    sy = cy + np.sin(ang) * rs * (H/2)
    return ndi.map_coordinates(L, [sy, sx], order=1, mode='constant')

def ring_from(M, band=0.06, core=0.9):
    """The memory wrapped onto the show's one ring: its texture survives, its shape does not."""
    rb = 0.42 + (rad - R_MARK) / band * 0.32
    sx = cx + np.cos(ang*1.0) * np.abs(rb) * (H/2)
    sy = cy + np.sin(ang*1.0) * np.abs(rb) * (H/2)
    tex = ndi.map_coordinates(M, [sy, sx], order=1, mode='constant')
    env = np.exp(-((rad - R_MARK)/band)**2)
    return np.clip(tex * env * 1.4 + ring_mark(0.006) * core * 0.6, 0, 1)

def recall(L, n):
    """One memory after n recalls: blur -> re-level -> quantise -> fixed holes -> hardening -> drift to the ring."""
    M = L.copy()
    for i in range(1, n+1):
        M = ndi.gaussian_filter(M, 0.5 + 0.15*i)
        M = np.clip(M / (np.percentile(M, 99.6) + 1e-6), 0, 1)      # recall restores the level ("savings")
        lv = max(3, 14 - 2*i)
        M = np.round(M * (lv - 1)) / (lv - 1)
        d = 0.018 * i
        M = M * np.clip((HOLES - d) / 0.03, 0, 1)                    # losses at fixed places (Basinski)
        if i >= 5:
            soft = ndi.gaussian_filter(M, 2.0)
            M = np.clip(M + (M - soft) * 0.4*(i-4), 0, 1)
            M = np.where(M > 0.35, 1.0, M * 0.25)                    # the oldest memory is the clearest
    a = np.clip((n - 6) / 8.0, 0, 1)                                  # after ~6 recalls it drifts to the mark
    return np.clip(M * (1 - a) + ring_from(M) * a, 0, 1)

def outline(L, keep_gist=0.25):
    g = ndi.gaussian_filter(L, 6)
    e = np.hypot(ndi.sobel(ndi.gaussian_filter(L,1.2), 0), ndi.sobel(ndi.gaussian_filter(L,1.2), 1))
    e = norm(e)
    e = ndi.gaussian_filter(e, 0.8)
    return np.clip(g*keep_gist + e*0.9, 0, 1)

def boundary_extension(L, s=0.78, seed=3):
    small = ndi.zoom(L, s, order=1)
    h, w = small.shape
    y0, x0 = (H-h)//2, (W-w)//2
    # invented surround: the memory mirrored beyond its own edge, blurred and warped
    padY, padX = y0+40, x0+40
    ext = np.pad(small, ((padY, padY), (padX, padX)), mode='reflect')[:H+80, :W+80][40:40+H, 40:40+W]
    rng = np.random.default_rng(seed)
    wx = ndi.gaussian_filter(rng.standard_normal((H, W)), 40) * 900
    wy = ndi.gaussian_filter(rng.standard_normal((H, W)), 40) * 900
    ext = ndi.map_coordinates(ndi.gaussian_filter(ext, 5), [yy+wy, xx+wx], order=1, mode='reflect')
    dx = np.maximum(np.maximum(x0 - xx, xx - (x0+w)), 0)
    dy = np.maximum(np.maximum(y0 - yy, yy - (y0+h)), 0)
    dist = np.hypot(dx, dy) / W
    att = np.exp(-dist / 0.12) * 0.75
    out = ext * att
    out[y0:y0+h, x0:x0+w] = small
    # the frame itself: a hairline of light
    edge = np.zeros((H, W), np.float32)
    edge[y0:y0+h, x0:x0+2] = edge[y0:y0+h, x0+w-2:x0+w] = 1
    edge[y0:y0+2, x0:x0+w] = edge[y0+h-2:y0+h, x0:x0+w] = 1
    return np.clip(out + ndi.gaussian_filter(edge, 0.7)*0.35, 0, 1)

def voronoi(images, cells=16, seed=5, gap=2.5):
    rng = np.random.default_rng(seed)
    pts = rng.random((cells, 2)) * [W, H]
    d = np.stack([np.hypot(xx-p[0], yy-p[1]) for p in pts])
    order = np.argsort(d, axis=0)
    d1 = np.take_along_axis(d, order[:1], 0)[0]; d2 = np.take_along_axis(d, order[1:2], 0)[0]
    idx = order[0]
    pick = rng.integers(0, len(images), cells)
    out = np.zeros((H, W), np.float32)
    for c in range(cells):
        m = idx == c
        img = images[pick[c]]
        # each fragment is shifted a little: a moment that never happened
        sh = rng.integers(-60, 60, 2)
        out[m] = np.roll(img, (sh[1], sh[0]), (0, 1))[m]
    seam = np.clip((d2 - d1) / gap, 0, 1)
    return out * seam


def grey(L, gain=3.0):
    return np.repeat((L*gain)[..., None], 3, axis=2).astype(np.float32)

# ---- captures from the engine (the "opening" of the show)
mesh, _ = scene_linear(r, '09 - Mesh Body.json', frames=60)
emer, _ = scene_linear(r, '12 - Emergence.json', frames=60, image_index=7, signals={'presence':0.9}, macros={'intensity':0.8})
fog,  _ = scene_linear(r, '08 - Signal Fog.json', frames=60, macros={'detail':0.4})
Lm, Le, Lf = norm(lum(mesh)), norm(lum(emer)), norm(lum(fog))

out = {}
out['d1_1_capture'] = finish(r, grey(Lm, 1.2), 'Bone')
out['d1_2_recall2'] = finish(r, grey(recall(Lm, 2), 1.2), 'Nitrate')
out['d1_3_recall5'] = finish(r, grey(recall(Lm, 5), 1.1), 'Nitrate')
out['d1_4_recall9_hard'] = finish(r, grey(recall(Lm, 9), 1.0), 'Ash')
out['d1_5_outline'] = finish(r, grey(outline(Lm), 1.2), 'Nitrate')
out['d1_6_boundary'] = finish(r, grey(boundary_extension(recall(Lm, 2)), 1.2), 'Nitrate')
out['d1_7_recombine'] = finish(r, grey(voronoi([recall(Lm,1), outline(Le, 0.6), Lf*0.8]), 1.2), 'Ash')
conv = recall(Lm, 14)
out['d1_8_mark'] = finish(r, grey(conv, 1.0), 'Cyanotype')
for k, img in out.items():
    img.save('out/mock/%s.png' % k)
    print(k, [round(x*100, 1) for x in band_stats(img)])
sheet.sheet(['out/mock/%s.png' % k for k in out], 'out/mock_d1.jpg', cols=4, tw=380, labels=list(out))
