import numpy as np, moderngl, os
from PIL import Image
from vjrender import *

STAGE_LOOK = {'grain':0.3,'crush':0.3,'halation':0.35,'weave':0.3,'dust':0.25,'blacks':0.0,'trails':0.1}

def scene_linear(r, preset, frames=90, macros=None, signals=None, env=None, overrides=None, image_index=2, dt=1/30.0, seed=0.0, image_path=None):
    sc = Scene(r, preset, image_index=image_index, seed=seed)
    if image_path: sc.forms = [image_path]
    if macros: sc.macros.update(macros)
    if signals: sc.signals.update(signals)
    if env: sc.env.update(env)
    out=None
    for i in range(frames):
        out = sc.step(dt, overrides)
    data = np.frombuffer(out.read_tex.read(), dtype=np.float16).reshape(out.h, out.w, 4).astype(np.float32)
    return data[..., :3], sc

def to_tex(r, arr):
    h, w = arr.shape[:2]
    a = np.concatenate([arr.astype(np.float32), np.ones((h, w, 1), np.float32)], axis=2)
    t = r.ctx.texture((w, h), 4, a.tobytes(), dtype='f4')
    t.filter = (moderngl.LINEAR, moderngl.LINEAR)
    return t

def finish(r, arr, palette, look=None, post=None, duo=0.0, frames=2):
    look = dict(STAGE_LOOK, **(look or {}))
    post = post or {'bloom': {'enabled': True, 'amount': 0.2, 'threshold': 0.9}, 'toneMap': 'reinhard', 'exposureEv': 0.3, 'vignette': 0.25}
    L = Look(r)
    t = to_tex(r, arr)
    img = None
    for i in range(frames):
        img = L.render(t, post, look, palette, duo=duo, t=1.0 + i/30)
    t.release()
    return img

def lum(a): return 0.2126*a[...,0]+0.7152*a[...,1]+0.0722*a[...,2]

def band_stats(img, lo=0.25, hi=0.65):
    a = np.asarray(img).astype('f4')/255.0
    lin = np.where(a <= 0.04045, a/12.92, ((a+0.055)/1.055)**2.4)
    y = 0.2126*lin[...,0]+0.7152*lin[...,1]+0.0722*lin[...,2]
    h = y.shape[0]
    band = y[int(h*(1-hi)):int(h*(1-lo))]
    return float(y.mean()), float(band.mean())
