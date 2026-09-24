"""Offline stills renderer for the VJ Engine's schema-2 presets (scratch tool, not part of the repo).

Runs the real ISF scene shaders (engine/Shaders/Instrument) through a port of the engine's
FinishPass (bloom, exposure, Reinhard, vignette, sRGB) and LookPass final shader (grain,
crush, palette, lifted blacks, halation, weave fringing, dust), with the modulation maths
of Modulation.cpp (base + sum(amount * (source - centre)), integrated rates).

Needs an X display (xvfb-run) for moderngl on Mesa llvmpipe.
"""
import json
import math
import os
import re
import sys

import moderngl
import numpy as np
from PIL import Image

ENGINE = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'engine'))
SHADERS = os.path.join(ENGINE, 'Shaders')
PRESETS = os.path.join(ENGINE, 'Presets')
MEDIA = os.path.join(ENGINE, 'Media')

PALETTES = {  # plugin/Source/PluginProcessor.cpp presetPalette()
    'Blood': ('050000', 'e01008', 'ff9a86'), 'Ember': ('080200', 'ff4a00', 'ffd27a'),
    'Bone': ('060606', '8a8580', 'f4f0e8'), 'Ice': ('00040a', '1a6cff', 'c8f4ff'),
    'Acid': ('020600', '5cff1a', 'f0ffc0'), 'Violet': ('05000a', '8a1aff', 'ffa8f0'),
    'Rust': ('0a0402', '9a3a12', 'e8b890'), 'Nitrate': ('0d0b09', '7f6b52', 'f3e6ce'),
    'Cyanotype': ('05080c', '2f5f79', 'd8eef2'), 'Tungsten': ('070504', '86461f', 'ffd9a8'),
    'Ash': ('0a0a0a', '6b6a66', 'edebe4'), 'Split': ('000000', 'ff1a12', '1ae6ff'),
}


def hex3(h):
    return tuple(int(h[i:i + 2], 16) / 255.0 for i in (0, 2, 4))


VERT = """#version 330
in vec2 position;
out vec2 isf_FragNormCoord;
out vec2 uv;
void main() { gl_Position = vec4(position, 0.0, 1.0); isf_FragNormCoord = position * 0.5 + 0.5; uv = isf_FragNormCoord; }
"""


def to330(body):
    body = body.replace('gl_FragColor', 'vj_FragColor').replace('#include', 'include')
    body = re.sub(r'\btexture2D\b', 'texture', body)
    body = re.sub(r'\bvarying\b', 'in', body)
    return body


def split_isf(path):
    src = open(path, encoding='utf-8').read()
    start, end = src.index('/*'), src.index('*/')
    header = json.loads(src[start + 2:end])
    body = src[end + 2:]
    out = []
    for line in body.splitlines():
        m = re.match(r'\s*#include\s+"([^"]+)"', line)
        if m:
            line = open(os.path.join(os.path.dirname(path), m.group(1)), encoding='utf-8').read()
        out.append(line)
    return header, '\n'.join(out)


def glsl_type(t):
    return {'float': 'float', 'bool': 'bool', 'event': 'bool', 'long': 'int', 'point2D': 'vec2',
            'color': 'vec4', 'image': 'sampler2D'}.get(t)


def load_post_sources():
    """The engine's own post shaders, read straight from LookPass.cpp / FinishPass.cpp (always in sync)."""
    def grab(path, name):
        src = open(os.path.join(ENGINE, 'Source', path), encoding='utf-8').read()
        return re.search(r'const char\* const ' + name + r' = R"\((.*?)\)";', src, re.S).group(1)
    L, F = 'LookPass.cpp', 'FinishPass.cpp'
    return (grab(L, 'extractSource'), grab(L, 'blurSource'), grab(L, 'finalSource'),
            grab(F, 'compositeSource'), grab(L, 'trailSource'))


class Target:
    def __init__(self, ctx, w, h, components=4, dtype='f2'):
        self.w, self.h = w, h
        self.tex = [ctx.texture((w, h), components, dtype=dtype) for _ in range(2)]
        for t in self.tex:
            t.filter = (moderngl.LINEAR, moderngl.LINEAR)
            t.repeat_x = t.repeat_y = False
        self.fbo = [ctx.framebuffer(color_attachments=[t]) for t in self.tex]
        for f in self.fbo:
            f.use()
            f.clear(0, 0, 0, 1)
        self.read = 0

    @property
    def read_tex(self):
        return self.tex[self.read]

    def write_fbo(self):
        return self.fbo[1 - self.read]

    def swap(self):
        self.read = 1 - self.read


class Renderer:
    def __init__(self, w=1280, h=720):
        self.ctx = moderngl.create_standalone_context()
        self.w, self.h = w, h
        quad = np.array([-1, -1, 1, -1, -1, 1, 1, 1], dtype='f4')
        self.vbo = self.ctx.buffer(quad.tobytes())
        rng = np.random.default_rng(7)
        g = (rng.random((256, 256, 4)) * 255).astype('u1')
        self.grain_tex = self.ctx.texture((256, 256), 4, g.tobytes())
        self.grain_tex.filter = (moderngl.LINEAR, moderngl.LINEAR)
        b = (rng.random((64, 64, 4)) * 255).astype('u1')
        self.blue_tex = self.ctx.texture((64, 64), 4, b.tobytes())
        self.blue_tex.filter = (moderngl.NEAREST, moderngl.NEAREST)
        self.image_cache = {}
        self._build_post()

    # ---------------------------------------------------------------- helpers
    def program(self, frag):
        prog = self.ctx.program(vertex_shader=VERT, fragment_shader=frag)
        vao = self.ctx.vertex_array(prog, [(self.vbo, '2f', 'position')])
        return prog, vao

    def image(self, path):
        if path not in self.image_cache:
            im = Image.open(path).convert('RGBA').transpose(Image.FLIP_TOP_BOTTOM)
            t = self.ctx.texture(im.size, 4, im.tobytes())
            t.filter = (moderngl.LINEAR, moderngl.LINEAR)
            self.image_cache[path] = t
        return self.image_cache[path]

    @staticmethod
    def setu(prog, name, value):
        if name in prog:
            try:
                prog[name].value = value
            except Exception:
                pass

    def bind(self, prog, textures):
        for unit, (name, tex) in enumerate(textures.items()):
            if name in prog:
                tex.use(location=unit)
                prog[name].value = unit

    # ------------------------------------------------------------ post chain
    def _build_post(self):
        head = '#version 330\nin vec2 uv;\nout vec4 vj_FragColor;\n'
        EXTRACT, BLUR, FINAL, COMPOSITE, TRAIL = load_post_sources()
        self.p_extract = self.program(head + to330(EXTRACT))
        self.p_blur = self.program(head + to330(BLUR))
        self.p_final = self.program(head + to330(FINAL))
        self.p_comp = self.program(head + to330(COMPOSITE))
        self.p_trail = self.program(head + to330(TRAIL))

    def draw(self, pv, fbo, viewport):
        fbo.use()
        self.ctx.viewport = viewport
        pv[1].render(moderngl.TRIANGLE_STRIP)


class Scene:
    """One schema-2 preset: stages, modulation and persistent buffers."""

    def __init__(self, r, preset_file, image_index=0, seed=0.0):
        self.r = r
        self.preset = json.load(open(os.path.join(PRESETS, preset_file), encoding='utf-8'))
        self.stages = []
        for st in self.preset['stages']:
            path = os.path.join(SHADERS, st['shader'])
            header, body = split_isf(path)
            uni = ['uniform float TIME;', 'uniform float TIMEDELTA;', 'uniform vec2 RENDERSIZE;', 'uniform int PASSINDEX;',
                   'uniform float level;', 'uniform float bass;', 'uniform float mid;', 'uniform float high;',
                   'uniform float beatphase;', 'uniform float onset;', 'uniform float vj_palette;', 'uniform float vj_seed;',
                   'uniform float vj_beat;', 'uniform float vj_time;', 'uniform float vj_speed;', 'uniform float vj_dt;']
            images = []
            for inp in header.get('INPUTS', []):
                t = glsl_type(inp['TYPE'])
                if not t:
                    continue
                uni.append('uniform %s %s;' % (t, inp['NAME']))
                if t == 'sampler2D':
                    uni.append('uniform vec2 %s_size;' % inp['NAME'])
                    images.append(inp['NAME'])
            passes = header.get('PASSES', [{}]) or [{}]
            targets = []
            for p in passes:
                if p.get('TARGET') and p['TARGET'] not in targets:
                    targets.append(p['TARGET'])
                    uni.append('uniform sampler2D %s;' % p['TARGET'])
            helpers = ('vec4 IMG_NORM_PIXEL (sampler2D img, vec2 normCoord) { return texture (img, normCoord); }\n'
                       'vec4 IMG_PIXEL (sampler2D img, vec2 pixelCoord) { return texture (img, pixelCoord / RENDERSIZE); }\n'
                       'vec4 IMG_THIS_PIXEL (sampler2D img) { return texture (img, isf_FragNormCoord); }\n'
                       'vec4 IMG_THIS_NORM_PIXEL (sampler2D img) { return texture (img, isf_FragNormCoord); }\n')
            frag = ('#version 330\nin vec2 isf_FragNormCoord;\nout vec4 vj_FragColor;\n' + '\n'.join(uni) + '\n'
                    + helpers + to330(body))
            prog = r.program(frag)
            w = int(r.w * st.get('scale', 1.0))
            h = int(r.h * st.get('scale', 1.0))
            tgt = {}
            for p in passes:
                name = p.get('TARGET')
                if name and name not in tgt:
                    ws = float(p.get('WIDTH', 1.0)) if p.get('WIDTH') else 1.0
                    hs = float(p.get('HEIGHT', 1.0)) if p.get('HEIGHT') else 1.0
                    tgt[name] = Target(r.ctx, max(1, int(w * ws)), max(1, int(h * hs)))
            out = Target(r.ctx, w, h)
            params = {p['name']: p for p in st['parameters']}
            self.stages.append(dict(def_=st, prog=prog, passes=passes, targets=tgt, out=out, images=images,
                                    params=params, integ={k: 0.0 for k in params}))
        self.macros = {m['id']: m['default'] for m in self.preset['macros']}
        self.signals = {}
        self.env = {}
        self.time = 0.0
        self.beat = 0.0
        self.seed = seed
        self.image_index = image_index
        forms = sorted(os.listdir(os.path.join(MEDIA, 'Images', 'Forms')))
        self.forms = [os.path.join(MEDIA, 'Images', 'Forms', f) for f in forms]

    def source_value(self, src):
        kind, _, rest = src.partition('.')
        if kind == 'macro':
            return self.macros.get(rest, 0.5)
        if kind == 'env':
            return self.env.get(rest, 0.0)
        if kind == 'lfo':
            return 0.5 + 0.5 * math.sin(2 * math.pi * self.beat / 64.0)
        if kind in ('audio', 'descriptor'):
            return self.signals.get(rest, 0.0)
        return 0.0

    def compute_params(self, stage_index, dt):
        st = self.stages[stage_index]
        acc = {k: 0.0 for k in st['params']}
        sid = st['def_']['id']
        for rt in self.preset['routes']:
            dst = rt['destination'].split('.')
            if dst[1] != sid:
                continue
            raw = self.source_value(rt['source'])
            u = min(max((raw - rt['inputMin']) / (rt['inputMax'] - rt['inputMin']), 0.0), 1.0)
            q = u ** rt.get('exponent', 1.0) if rt.get('curve') == 'power' else u
            gain = 1.0
            if rt.get('scaleBy'):
                gain = 2.0 * self.macros.get(rt['scaleBy'].split('.')[1], 0.5)
            acc[dst[2]] += gain * rt['amount'] * (q - rt['center'])
        values = {}
        for name, p in st['params'].items():
            base = (p['default'] - p['min']) / (p['max'] - p['min'])
            v = base + acc[name]
            v = v - math.floor(v) if p.get('cyclic') else min(max(v, 0.0), 1.0)
            phys = p['min'] + v * (p['max'] - p['min'])
            if p.get('integrate'):
                st['integ'][name] += phys * dt
                phys = st['integ'][name]
            values[name] = phys
        return values

    def step(self, dt=1 / 30.0, overrides=None):
        self.time += dt
        self.beat += dt * 2.0  # 120 bpm
        prev_out = None
        for i, st in enumerate(self.stages):
            prog, vao = st['prog']
            values = self.compute_params(i, dt)
            if overrides:
                values.update(overrides.get(st['def_']['id'], {}))
            for k, v in values.items():
                Renderer.setu(prog, k, float(v))
            common = dict(TIME=self.time, TIMEDELTA=dt, vj_time=self.time, vj_dt=dt, vj_speed=1.0,
                          vj_beat=self.beat, vj_seed=self.seed, vj_palette=0.0,
                          level=self.signals.get('level.activity', 0.0), bass=self.signals.get('bass.activity', 0.0),
                          mid=self.signals.get('mid.activity', 0.0), high=self.signals.get('high.activity', 0.0),
                          beatphase=self.beat % 1.0, onset=0.0)
            for k, v in common.items():
                Renderer.setu(prog, k, float(v))
            texs = {}
            for name in st['images']:
                if name == 'inputImage' and prev_out is not None:
                    texs[name] = prev_out.read_tex
                    Renderer.setu(prog, name + '_size', (float(prev_out.w), float(prev_out.h)))
                else:
                    img = self.r.image(self.forms[self.image_index % len(self.forms)])
                    texs[name] = img
                    Renderer.setu(prog, name + '_size', (float(img.width), float(img.height)))
            for pi, p in enumerate(st['passes']):
                Renderer.setu(prog, 'PASSINDEX', pi)
                tnames = {n: t.read_tex for n, t in st['targets'].items()}
                self.r.bind(prog, dict(texs, **tnames))
                name = p.get('TARGET')
                tgt = st['targets'][name] if name else st['out']
                Renderer.setu(prog, 'RENDERSIZE', (float(tgt.w), float(tgt.h)))
                fbo = tgt.write_fbo()
                fbo.use()
                self.r.ctx.viewport = (0, 0, tgt.w, tgt.h)
                vao.render(moderngl.TRIANGLE_STRIP)
                tgt.swap()
                if not name:
                    pass
            prev_out = st['out']
        return prev_out


class Look:
    """Port of FinishPass + LookPass for stills."""

    def __init__(self, r):
        self.r = r
        w, h = r.w, r.h
        self.finished = Target(r.ctx, w, h)
        self.bloom = [Target(r.ctx, w // 4, h // 4), Target(r.ctx, w // 4, h // 4)]
        self.halo = [Target(r.ctx, w // 4, h // 4), Target(r.ctx, w // 4, h // 4)]
        self.hist = Target(r.ctx, w, h)
        self.final = r.ctx.framebuffer(color_attachments=[r.ctx.texture((w, h), 4)])
        self.frame = 0

    def blur(self, src_tex, pair, threshold, prog_extract=True):
        r = self.r
        pe = r.p_extract[0]
        Renderer.setu(pe, 'texel', (1.0 / r.w, 1.0 / r.h))
        Renderer.setu(pe, 'threshold', threshold)
        r.bind(pe, {'source': src_tex})
        r.draw(r.p_extract, pair[0].write_fbo(), (0, 0, pair[0].w, pair[0].h))
        pair[0].swap()
        pb = r.p_blur[0]
        for _ in range(2):
            Renderer.setu(pb, 'direction', (1.0 / pair[0].w, 0.0))
            r.bind(pb, {'source': pair[0].read_tex})
            r.draw(r.p_blur, pair[1].write_fbo(), (0, 0, pair[1].w, pair[1].h))
            pair[1].swap()
            Renderer.setu(pb, 'direction', (0.0, 1.0 / pair[0].h))
            r.bind(pb, {'source': pair[1].read_tex})
            r.draw(r.p_blur, pair[0].write_fbo(), (0, 0, pair[0].w, pair[0].h))
            pair[0].swap()
        return pair[0].read_tex

    def render(self, scene_tex, post, look, palette, palette_mix=1.0, duo=0.0, t=0.0):
        r = self.r
        # finish: bloom + exposure + tone map + vignette + sRGB
        bl = post.get('bloom', {})
        bloom_tex = self.blur(scene_tex, self.bloom, bl.get('threshold', 0.9))
        pc = r.p_comp[0]
        Renderer.setu(pc, 'bloomAmount', bl.get('amount', 0.0) * 3.0 if bl.get('enabled') else 0.0)
        Renderer.setu(pc, 'exposure', 2.0 ** post.get('exposureEv', 0.0))
        Renderer.setu(pc, 'toneMap', 1.0 if post.get('toneMap', 'reinhard') == 'reinhard' else 0.0)
        Renderer.setu(pc, 'vignette', post.get('vignette', 0.0))
        Renderer.setu(pc, 'grain', 0.0)
        Renderer.setu(pc, 'resolution', (float(r.w), float(r.h)))
        r.bind(pc, {'scene': scene_tex, 'bloom': bloom_tex})
        r.draw(r.p_comp, self.finished.write_fbo(), (0, 0, r.w, r.h))
        self.finished.swap()
        # trails
        pt = r.p_trail[0]
        retain = look.get('trails', 0.1)
        Renderer.setu(pt, 'retain', 0.0 if retain < 0.01 else 0.6 + 0.38 * retain)
        Renderer.setu(pt, 'push', 0.004 * retain)
        r.bind(pt, {'scene': self.finished.read_tex, 'previous': self.hist.read_tex})
        r.draw(r.p_trail, self.hist.write_fbo(), (0, 0, r.w, r.h))
        self.hist.swap()
        img = self.hist.read_tex
        halo_tex = self.blur(img, self.halo, 0.75)
        # final film composite
        self.frame += 1
        pf = r.p_final[0]
        c0, c1, c2 = (hex3(x) for x in PALETTES[palette]) if isinstance(palette, str) else palette
        vals = dict(resolution=(float(r.w), float(r.h)), time=t, gain=1.0, grain=look.get('grain', 0.3),
                    crush=look.get('crush', 0.35), glitch=0.0, glitchSeed=1.0, smear=0.0, symbols=0.0, symbolSeed=1.0,
                    flashAmount=0.0, flashType=0.0, halation=look.get('halation', 0.35), weave=look.get('weave', 0.3),
                    dust=look.get('dust', 0.3), blacks=look.get('blacks', 0.35), filmFrame=float(self.frame),
                    weaveOffset=(0.3, -0.2), flicker=0.0, scratchA=(0, 0, 0, 0), scratchB=(0, 0, 0, 0),
                    hairA=(0, 0, 0, 0), hairB=(0, 0), c0=c0, c1=c1, c2=c2, paletteMix=palette_mix, duo=duo,
                    drift=look.get('drift', 0.0), driftTime=t)
        for k, v in vals.items():
            Renderer.setu(pf, k, v)
        r.bind(pf, {'image': img, 'halo': halo_tex, 'grainTex': r.grain_tex, 'blueTex': r.blue_tex})
        r.draw(r.p_final, self.final, (0, 0, r.w, r.h))
        data = self.final.read(components=3)
        return Image.frombytes('RGB', (r.w, r.h), data).transpose(Image.FLIP_TOP_BOTTOM)


def luminance_stats(img):
    a = np.asarray(img).astype('f4') / 255.0
    lin = np.where(a <= 0.04045, a / 12.92, ((a + 0.055) / 1.055) ** 2.4)
    y = 0.2126 * lin[..., 0] + 0.7152 * lin[..., 1] + 0.0722 * lin[..., 2]
    ys = 0.2126 * a[..., 0] + 0.7152 * a[..., 1] + 0.0722 * a[..., 2]
    return {'apl_linear': float(y.mean()), 'mean_srgb': float(ys.mean()),
            'black_pct': float((ys < 0.08).mean() * 100), 'bright_pct': float((ys > 0.6).mean() * 100)}
