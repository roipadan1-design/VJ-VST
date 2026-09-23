"""Builds the schema-2 scene presets (engine/Presets/0x - *.json).

Each preset = one generative treatment (Shaders/Instrument/*.fs) + a musical
mapping. Colour is NOT decided here: the engine's global look pass maps every
scene through the performer's 3-colour palette (plugin "Look" section), so
scenes only need to deliver light, shape and motion. Shared conventions keep
every scene playable the same way:

  macros   0 Intensity  1 Motion  2 Color  3 Space
           4 Impact     (scales every hit reaction - routes carry scaleBy)
           5 Gravity    (pull / fall / drift)
           6 Viscosity  (how thick and slow the motion is)
           7 Detail     (density / complexity)
  envs     kick (3/180 ms), snare (2/140 ms), hat (1/70 ms), swell (5/900 ms, kick)
  user     the manual hit (Space bar / /v2/trigger / plugin button) fires "kick"

Run after editing:  python build_instrument_presets.py
The engine validates every file on load and logs any rejection to VJEngine.log.
"""
import glob
import json
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'Presets')

MACROS = [
    {'id': 'intensity', 'slot': 0, 'label': 'Intensity', 'default': 0.5},
    {'id': 'motion', 'slot': 1, 'label': 'Motion', 'default': 0.4},
    {'id': 'color', 'slot': 2, 'label': 'Color', 'default': 0.5},
    {'id': 'space', 'slot': 3, 'label': 'Space', 'default': 0.5},
    {'id': 'impact', 'slot': 4, 'label': 'Impact', 'default': 0.5},
    {'id': 'gravity', 'slot': 5, 'label': 'Gravity', 'default': 0.5},
    {'id': 'viscosity', 'slot': 6, 'label': 'Viscosity', 'default': 0.5},
    {'id': 'detail', 'slot': 7, 'label': 'Detail', 'default': 0.5},
]

ENVS = [
    {'id': 'kick', 'type': 'ad', 'attackMs': 3, 'decayMs': 180, 'peak': 1, 'retrigger': 'max'},
    {'id': 'snare', 'type': 'ad', 'attackMs': 2, 'decayMs': 140, 'peak': 1, 'retrigger': 'restart'},
    {'id': 'hat', 'type': 'ad', 'attackMs': 1, 'decayMs': 70, 'peak': 1, 'retrigger': 'restart'},
    {'id': 'swell', 'type': 'ad', 'attackMs': 5, 'decayMs': 900, 'peak': 1, 'retrigger': 'max'},
    {'id': 'drop', 'type': 'ad', 'attackMs': 1, 'decayMs': 60, 'peak': 1, 'retrigger': 'restart'},
    {'id': 'drift', 'type': 'lfo', 'shape': 'sine', 'periodBeats': 16, 'phaseOffset': 0, 'polarity': 'bipolar'},
]

KICK_ACTIONS = [{'type': 'envelope', 'target': 'kick', 'amount': 1},
                {'type': 'envelope', 'target': 'swell', 'amount': 1},
                {'type': 'envelope', 'target': 'drop', 'amount': 1},
                {'type': 'reseed'}]

TRIGGERS = [
    {'id': 'kick', 'on': 'event.kick', 'minStrength': 0.1, 'refractoryMs': 90, 'quantize': 'none', 'actions': KICK_ACTIONS},
    {'id': 'snare', 'on': 'event.snare', 'minStrength': 0.1, 'refractoryMs': 70, 'quantize': 'none',
     'actions': [{'type': 'envelope', 'target': 'snare', 'amount': 1}]},
    {'id': 'hat', 'on': 'event.hat', 'minStrength': 0.1, 'refractoryMs': 45, 'quantize': 'none',
     'actions': [{'type': 'envelope', 'target': 'hat', 'amount': 1}]},
    {'id': 'user', 'on': 'event.userTrigger', 'minStrength': 0, 'refractoryMs': 50, 'quantize': 'none', 'actions': KICK_ACTIONS},
]

FORMS = {'type': 'images', 'folder': 'Images/Forms', 'advance': 'bar', 'every': 2, 'order': 'random'}


def param(name, lo, hi, default, label=None, cyclic=False, integrate=False):
    p = {'name': name, 'label': label or name.replace('_', ' ').title(), 'min': lo, 'max': hi,
         'default': default, 'unit': 'normalized', 'cyclic': cyclic}
    if integrate:
        p['integrate'] = True
    return p


def route(src, dst, amount, center=0.0, curve='linear', exponent=1.0, attack=None, release=None, lo=0.0, hi=1.0):
    kind = src.split('.')[0]
    if attack is None:
        attack = {'macro': 20, 'audio': 30, 'descriptor': 120}.get(kind, 0)
    if release is None:
        release = {'macro': 180, 'audio': 220, 'descriptor': 400}.get(kind, 0)
    r = {'id': (src + '-' + dst).replace('.', '-')[:64].lower(), 'source': src, 'destination': 'stage.' + dst,
         'inputMin': lo, 'inputMax': hi, 'curve': curve, 'exponent': exponent, 'center': center,
         'amount': amount, 'attackMs': attack, 'releaseMs': release}
    if kind == 'env':
        r['scaleBy'] = 'macro.impact'   # every hit reaction follows the Impact knob
    return r


def preset(file_name, pid, name, description, stages, routes, post=None, transition=None, seed=1):
    return file_name, {
        'schemaVersion': '2.0',
        'id': pid,
        'name': name,
        'description': description,
        'author': 'VJ VST',
        'license': 'project-internal',
        'engine': {'minimumVersion': '0.2.0', 'shaderProfile': 'isf-legacy', 'workingColorSpace': 'linear-srgb', 'featureContract': '2.0'},
        'sourcePolicy': {'bass': 'kick-or-mix', 'mid': 'snare-or-mix', 'high': 'hat-or-mix', 'level': 'mix'},
        'stages': stages,
        'macros': MACROS,
        'modulators': ENVS,
        'routes': routes,
        'triggers': TRIGGERS,
        'transition': transition or {'type': 'cut', 'durationMs': 0, 'quantize': 'beat', 'historyOnEnter': 'reset', 'retarget': 'snapshot-current'},
        # Grain lives in the global look pass now; the preset only finishes light.
        'post': post or {'bloom': {'enabled': True, 'amount': 0.22, 'threshold': 0.9, 'levels': 5},
                         'toneMap': 'reinhard', 'exposureEv': 0.3, 'outputColorSpace': 'srgb', 'grain': 0.0, 'vignette': 0.25},
        'performance': {'qualityTier': 'medium', 'sceneBudgetMs1080p': 3.0, 'deterministicSeed': seed, 'notes': 'target, not measured'},
    }


def stage(sid, shader, params, sources=None, kind='generator', scale=1.0):
    s = {'id': sid, 'kind': kind, 'shader': 'Instrument/' + shader, 'targetFormat': 'rgba16f', 'scale': scale,
         'inputs': {}, 'parameters': params}
    if sources:
        s['sources'] = sources
    return s


PRESETS = [
    preset('01 - Hot Blobs.json', 'hot-blobs', 'Hot Blobs',
           'A liquid mass hit by sound: each kick throws it and drops a stone into a ripple tank, gravity pulls it back, viscosity slows it.',
           [stage('blobs', 'hot_blobs.fs', [
               param('scale', 0.8, 4.0, 1.8), param('flow', 0.0, 0.6, 0.1, integrate=True), param('threshold', 0.38, 0.64, 0.53),
               param('drop', 0, 1, 0), param('impact', 0, 2, 0.8), param('gravity', 0, 2, 0.6),
               param('viscosity', 0, 1, 0.45), param('ripple', 0, 1.5, 0.7), param('rim', 0, 2, 0.7),
               param('emission', 0.3, 3.0, 1.2)])],
           [route('macro.intensity', 'blobs.emission', 0.6, 0.5), route('macro.intensity', 'blobs.rim', 0.4, 0.5),
            route('macro.motion', 'blobs.flow', 0.9, 0.4), route('macro.color', 'blobs.ripple', 0.6, 0.5),
            route('macro.space', 'blobs.scale', -0.7, 0.5), route('macro.space', 'blobs.threshold', 0.4, 0.5),
            route('macro.impact', 'blobs.impact', 0.9, 0.5), route('macro.gravity', 'blobs.gravity', 0.9, 0.5),
            route('macro.viscosity', 'blobs.viscosity', 1.0, 0.5), route('macro.detail', 'blobs.scale', 0.35, 0.5),
            route('env.drop', 'blobs.drop', 1.0), route('env.snare', 'blobs.rim', 0.3),
            route('audio.level.activity', 'blobs.threshold', -0.2, curve='power', exponent=1.4)],
           seed=202),

    preset('02 - Dot Relief.json', 'dot-relief', 'Dot Relief',
           'Abstract 3D forms rebuilt as a tilted relief of dots; kicks lift the relief, a new form every 2 bars.',
           [stage('dots', 'dot_relief.fs', [
               param('grid', 40, 200, 90), param('depth', 0, 0.6, 0.22), param('tilt', 0, 0.8, 0.35),
               param('dot_size', 0.1, 0.9, 0.45), param('zoom', 0.7, 2.6, 1.15), param('sway', 0.0, 1.0, 0.2, integrate=True),
               param('surge', 0, 1, 0), param('tint', 0, 1, 0), param('hue', 0, 1, 0.1, cyclic=True),
               param('noise_floor', 0, 1, 0.12)], {'source': FORMS})],
           [route('macro.intensity', 'dots.dot_size', 0.4, 0.5), route('macro.intensity', 'dots.depth', 0.4, 0.5),
            route('macro.motion', 'dots.sway', 0.9, 0.4), route('macro.color', 'dots.noise_floor', 0.6, 0.5),
            route('macro.space', 'dots.zoom', -0.4, 0.5), route('macro.detail', 'dots.grid', 0.8, 0.5),
            route('macro.gravity', 'dots.tilt', 0.8, 0.5), route('macro.viscosity', 'dots.sway', -0.4, 0.5),
            route('env.kick', 'dots.surge', 1.0), route('env.swell', 'dots.depth', 0.3),
            route('audio.bass.activity', 'dots.depth', 0.2), route('audio.high.activity', 'dots.noise_floor', 0.3)],
           seed=303),

    preset('03 - One Bit.json', 'one-bit', 'One Bit',
           'Hard 1-bit threshold of abstract 3D forms with halftone edges and speckle; forms swap on every second snare, hats flip polarity.',
           [stage('bits', 'one_bit.fs', [
               param('threshold', 0.2, 0.8, 0.45), param('dither', 0, 1, 0.5), param('cell', 2, 14, 5),
               param('zoom', 0.6, 3.2, 1.3), param('spin', -0.6, 0.6, 0.1, integrate=True), param('invert', 0, 1, 0),
               param('frame', 0, 1, 0), param('speckle', 0, 1, 0.25), param('surge', 0, 1, 0),
               param('texture_amt', 0, 1, 0.5)],
               {'source': dict(FORMS, advance='event.snare')})],
           [route('macro.intensity', 'bits.texture_amt', 0.5, 0.5), route('macro.intensity', 'bits.speckle', 0.5, 0.5),
            route('macro.motion', 'bits.spin', 0.8, 0.4), route('macro.color', 'bits.dither', 0.8, 0.5),
            route('macro.space', 'bits.zoom', 0.8, 0.5), route('macro.space', 'bits.frame', 1.0, 0.5, curve='smoothstep'),
            route('macro.detail', 'bits.cell', -0.8, 0.5), route('macro.gravity', 'bits.threshold', 0.4, 0.5),
            route('env.kick', 'bits.surge', 1.0), route('env.hat', 'bits.invert', 1.0),
            route('audio.high.activity', 'bits.speckle', 0.3), route('descriptor.centroid', 'bits.threshold', -0.2, 0.5)],
           seed=404),

    preset('04 - Corridor.json', 'corridor', 'Corridor',
           'Moving forward through an endless service corridor; kicks surge the camera and blow the lights, hats flicker them.',
           [stage('hall', 'corridor.fs', [
               param('travel', 0.0, 8.0, 1.6, integrate=True), param('sway', 0.0, 1.5, 0.4, integrate=True),
               param('sway_amt', 0, 1, 0.3), param('width', 0.5, 2.5, 1.0), param('surge', 0, 1, 0),
               param('flicker', 0, 1, 0), param('detail', 0, 1, 0.5), param('fog', 0.05, 1.0, 0.35),
               param('emission', 0.2, 4.0, 1.1)])],
           [route('macro.intensity', 'hall.emission', 0.6, 0.5), route('macro.motion', 'hall.travel', 0.9, 0.4),
            route('macro.color', 'hall.fog', 0.7, 0.5), route('macro.space', 'hall.width', 0.7, 0.5),
            route('macro.detail', 'hall.detail', 1.0, 0.5), route('macro.gravity', 'hall.sway_amt', 1.0, 0.5),
            route('macro.viscosity', 'hall.sway', -0.5, 0.5),
            route('env.swell', 'hall.travel', 1.0), route('env.kick', 'hall.surge', 1.0), route('env.hat', 'hall.flicker', 0.8),
            route('audio.bass.activity', 'hall.emission', 0.15)],
           seed=505),

    preset('05 - Fibers.json', 'fibers', 'Fibers',
           'A dense web of filaments around a dark knot - roots, nerves, torn cloth; bass swells it, kicks tear it outward.',
           [stage('web', 'fibers.fs', [
               param('drift', -0.5, 1.5, 0.35, integrate=True), param('turn', -0.4, 0.4, 0.04, integrate=True),
               param('zoom', 0.5, 3.0, 1.2), param('density', 0, 1, 0.5), param('thickness', 0.05, 1.0, 0.4),
               param('tear', 0, 1, 0), param('swell', 0, 1, 0), param('knot', 0, 1, 0.6),
               param('emission', 0.2, 4.0, 1.4)])],
           [route('macro.intensity', 'web.emission', 0.6, 0.5), route('macro.intensity', 'web.thickness', 0.4, 0.5),
            route('macro.motion', 'web.drift', 0.8, 0.4), route('macro.motion', 'web.turn', 0.4, 0.4),
            route('macro.color', 'web.knot', 0.8, 0.5), route('macro.space', 'web.zoom', 0.7, 0.5),
            route('macro.detail', 'web.density', 1.0, 0.5), route('macro.gravity', 'web.turn', -0.5, 0.5),
            route('macro.viscosity', 'web.drift', -0.5, 0.5),
            route('env.kick', 'web.tear', 1.0), route('env.swell', 'web.tear', 0.3),
            route('audio.bass.activity', 'web.swell', 0.8), route('env.snare', 'web.thickness', 0.3)],
           seed=606),

    preset('06 - Terminal.json', 'terminal', 'Terminal',
           'An invented machine script scrolling over dark blots; kicks jump the scroll, snares invert a band.',
           [stage('term', 'terminal.fs', [
               param('scroll', 0.0, 6.0, 1.0, integrate=True), param('rows', 10, 60, 26), param('fill', 0, 1, 0.6),
               param('jump', 0, 1, 0), param('invert', 0, 1, 0), param('background', 0, 1, 0.5),
               param('morph', 0.0, 1.0, 0.2, integrate=True), param('emission', 0.2, 4.0, 1.3)])],
           [route('macro.intensity', 'term.emission', 0.6, 0.5), route('macro.motion', 'term.scroll', 0.9, 0.4),
            route('macro.color', 'term.background', 0.9, 0.5), route('macro.space', 'term.rows', 0.8, 0.5),
            route('macro.detail', 'term.fill', 1.0, 0.5), route('macro.gravity', 'term.scroll', 0.3, 0.5),
            route('macro.viscosity', 'term.morph', -0.5, 0.5),
            route('env.kick', 'term.jump', 1.0), route('env.snare', 'term.invert', 1.0),
            route('audio.high.activity', 'term.fill', 0.2)],
           post={'bloom': {'enabled': True, 'amount': 0.15, 'threshold': 1.0, 'levels': 4},
                 'toneMap': 'reinhard', 'exposureEv': 0.5, 'outputColorSpace': 'srgb', 'grain': 0.0, 'vignette': 0.15},
           seed=707),

    preset('07 - Ink.json', 'ink', 'Ink',
           'Blots of ink or blood soaking through paper, torn edges, dot holes and smear bands; kicks swell and rip it.',
           [stage('ink', 'ink.fs', [
               param('morph', 0.0, 1.2, 0.25, integrate=True), param('fall', -0.5, 1.0, 0.1, integrate=True),
               param('scale', 0.5, 5.0, 1.6), param('coverage', 0.2, 0.8, 0.36), param('swell', 0, 1, 0),
               param('smear', 0, 1, 0.3), param('rip', 0, 1, 0), param('dots', 0, 1, 0.5),
               param('emission', 0.2, 4.0, 1.3)])],
           [route('macro.intensity', 'ink.coverage', 0.5, 0.5), route('macro.intensity', 'ink.emission', 0.3, 0.5),
            route('macro.motion', 'ink.morph', 0.9, 0.4), route('macro.color', 'ink.smear', 0.8, 0.5),
            route('macro.space', 'ink.scale', -0.6, 0.5), route('macro.detail', 'ink.dots', 1.0, 0.5),
            route('macro.gravity', 'ink.fall', 0.9, 0.5), route('macro.viscosity', 'ink.morph', -0.5, 0.5),
            route('env.kick', 'ink.rip', 1.0), route('env.swell', 'ink.swell', 0.8),
            route('audio.bass.activity', 'ink.swell', 0.3)],
           seed=808),
]

if __name__ == '__main__':
    keep = {name for name, _ in PRESETS}
    for old in glob.glob(os.path.join(OUT, '*.json')):
        if os.path.basename(old) not in keep:
            os.remove(old)
            print('removed', os.path.basename(old))
    for file_name, body in PRESETS:
        with open(os.path.join(OUT, file_name), 'w', encoding='utf-8') as f:
            json.dump(body, f, indent=2)
            f.write('\n')
        print('wrote', file_name)
