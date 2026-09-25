import numpy as np
from scipy import ndimage as ndi
from mock_util import *
from PIL import ImageDraw, ImageFont
import sheet
W, H = 1280, 720
r = Renderer(W, H)
yy, xx = np.mgrid[0:H, 0:W].astype(np.float32)
yy = (H - 1) - yy   # synthetic images: y measured from the top, like the stage
rng = np.random.default_rng(9)
def grey(L, gain=1.0): return np.repeat((L*gain)[..., None], 3, axis=2).astype(np.float32)
def noise1d(n, sigma, seed):
    g = np.random.default_rng(seed).standard_normal(n); g = ndi.gaussian_filter1d(g, sigma); return (g-g.min())/(g.max()-g.min())
out = {}
# ---------------- Direction 2: A familiar light
y0 = H*0.22                                              # a horizon in the upper quarter (above heads)
along = 0.35 + 0.65*np.abs((xx/W)-0.5)*2                # brightest at the edges, dimmest behind centre stage
along *= 0.75 + 0.25*noise1d(W, 60, 3)[None, :]
seam = np.exp(-((yy-y0)/1.1)**2)*1.0 + np.exp(-np.abs(yy-y0)/28.0)*0.06
out['d2_1_seam'] = finish(r, grey(seam*along, 2.2), 'Bone', look={'halation':0.5})
halo, sc = scene_linear(r, '11 - Halo Ring.json', frames=90, macros={'scale':1.0,'form':0.3,'detail':0.3})
out['d2_2_ring_hollow'] = finish(r, halo, 'Ash', post=sc.preset['post'])
em, sc = scene_linear(r, '12 - Emergence.json', frames=60, image_index=7, signals={'presence':0.35}, macros={'form':0.9,'scale':0.35})
out['d2_3_emergence_streaks'] = finish(r, em, 'Bone', post=sc.preset['post'])
glow = np.clip((yy - H*0.45)/(H*0.55), 0, 1)**1.6        # low horizon light: the dancers become shadows
glow = glow * (0.85 + 0.15*ndi.gaussian_filter(rng.random((H, W)), 30)*3)
out['d2_4_silhouette_field'] = finish(r, grey(glow, 0.9), 'Tungsten', look={'grain':0.45,'halation':0.3})
sky = np.clip(1.0 - np.abs((yy/H) - 0.62)*1.6, 0, 1)**2.2 * 0.35
rad = np.hypot(xx-W/2, yy-H/2)/(H/2)
ring = np.exp(-((rad-0.82)/0.008)**2)*0.9 + np.exp(-((rad-0.82)/0.06)**2)*0.12
out['d2_5_sky_blue'] = finish(r, grey(sky + ring, 1.0), 'Cyanotype', look={'halation':0.4})
pt = np.exp(-(((xx-W*0.93)**2 + (yy-H*0.30)**2)/(2*3.0**2)))*4.0 + np.exp(-np.hypot(xx-W*0.93, yy-H*0.30)/60.0)*0.05
out['d2_6_glimpse'] = finish(r, grey(pt, 1.0), 'Tungsten', look={'halation':0.8})
# ---------------- Direction 3: The weather of memory
fog, sc = scene_linear(r, '08 - Signal Fog.json', frames=90, macros={'detail':0.35,'scale':0.8}, signals={'build':0.6})
out['d3_1_fog'] = finish(r, fog, 'Ash', post=sc.preset['post'])
me, sc = scene_linear(r, '09 - Mesh Body.json', frames=90, signals={'build':0.95}, macros={'scale':0.7})
out['d3_2_mesh_erode'] = finish(r, me, 'Nitrate', post=sc.preset['post'])
ed, sc = scene_linear(r, '12 - Emergence.json', frames=90, image_index=5, signals={'build':0.9,'presence':0.9}, macros={'scale':0.6,'intensity':0.8})
out['d3_3_emergence_dust'] = finish(r, ed, 'Bone', post=sc.preset['post'])
fi, sc = scene_linear(r, '05 - Fibers.json', frames=90, macros={'detail':0.1,'intensity':0.35,'scale':0.7})
out['d3_4_fibers'] = finish(r, fi, 'Ash', post=sc.preset['post'])
mo, sc = scene_linear(r, '10 - Morphogen.json', frames=500, macros={'scale':0.9,'erode':0.0,'intensity':0.0,'detail':1.0})
out['d3_5_morphogen'] = finish(r, mo*0.8, 'Nitrate', post=sc.preset['post'], look={'crush':0.55})
cr, sc = scene_linear(r, '11 - Halo Ring.json', frames=90, macros={'scale':0.9,'form':1.0,'erode':0.9,'detail':0.9}, signals={'high.activity':0.8,'mid.activity':0.7,'bass.activity':0.5})
out['d3_6_cloud_ring'] = finish(r, cr, 'Cyanotype', post=sc.preset['post'])
for k, img in out.items():
    img.save('out/mock/%s.png' % k); print(k, [round(x*100,1) for x in band_stats(img)])
sheet.sheet(['out/mock/%s.png' % k for k in out if k.startswith('d2')], 'out/mock_d2.jpg', cols=3, tw=420, labels=[k for k in out if k.startswith('d2')])
sheet.sheet(['out/mock/%s.png' % k for k in out if k.startswith('d3')], 'out/mock_d3.jpg', cols=3, tw=420, labels=[k for k in out if k.startswith('d3')])
