"""Builds the schema-2 "Instrument" presets (engine/Presets/1x - *.json).

Each preset = one strong treatment (Shaders/Instrument/*.fs) + raw material
(sources) + a musical mapping. The shared conventions keep every preset
playable the same way:

  macros   slot 0 Intensity  slot 1 Motion  slot 2 Color  slot 3 Space
  envs     kick (3/180 ms), snare (2/140 ms), hat (1/70 ms) fired by role events
  user     the manual hit (Space bar / /v2/trigger / plugin button) fires "kick"

Run after editing:  python build_instrument_presets.py
The engine validates every file on load and logs any rejection to VJEngine.log.
"""
import json, os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'Presets')

MACROS = [
    {'id': 'intensity', 'slot': 0, 'label': 'Intensity', 'default': 0.5},
    {'id': 'motion', 'slot': 1, 'label': 'Motion', 'default': 0.4},
    {'id': 'color', 'slot': 2, 'label': 'Color', 'default': 0.5},
    {'id': 'space', 'slot': 3, 'label': 'Space', 'default': 0.5},
]

ENVS = [
    {'id': 'kick', 'type': 'ad', 'attackMs': 3, 'decayMs': 180, 'peak': 1, 'retrigger': 'max'},
    {'id': 'snare', 'type': 'ad', 'attackMs': 2, 'decayMs': 140, 'peak': 1, 'retrigger': 'restart'},
    {'id': 'hat', 'type': 'ad', 'attackMs': 1, 'decayMs': 70, 'peak': 1, 'retrigger': 'restart'},
    {'id': 'drift', 'type': 'lfo', 'shape': 'sine', 'periodBeats': 16, 'phaseOffset': 0, 'polarity': 'bipolar'},
]

TRIGGERS = [
    {'id': 'kick', 'on': 'event.kick', 'minStrength': 0.1, 'refractoryMs': 90, 'quantize': 'none',
     'actions': [{'type': 'envelope', 'target': 'kick', 'amount': 1}]},
    {'id': 'snare', 'on': 'event.snare', 'minStrength': 0.1, 'refractoryMs': 70, 'quantize': 'none',
     'actions': [{'type': 'envelope', 'target': 'snare', 'amount': 1}]},
    {'id': 'hat', 'on': 'event.hat', 'minStrength': 0.1, 'refractoryMs': 45, 'quantize': 'none',
     'actions': [{'type': 'envelope', 'target': 'hat', 'amount': 1}]},
    {'id': 'user', 'on': 'event.userTrigger', 'minStrength': 0, 'refractoryMs': 50, 'quantize': 'none',
     'actions': [{'type': 'envelope', 'target': 'kick', 'amount': 1}]},
]

MASKS = {'type': 'images', 'folder': 'Images/Masks', 'advance': 'bar', 'every': 2, 'order': 'random'}


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
    return {'id': (src + '-' + dst).replace('.', '-')[:64].lower(), 'source': src, 'destination': 'stage.' + dst,
            'inputMin': lo, 'inputMax': hi, 'curve': curve, 'exponent': exponent, 'center': center,
            'amount': amount, 'attackMs': attack, 'releaseMs': release}


def preset(file_name, pid, name, description, stages, routes, extra_triggers=(), post=None, transition=None, seed=1):
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
        'triggers': TRIGGERS + list(extra_triggers),
        'transition': transition or {'type': 'cut', 'durationMs': 0, 'quantize': 'bar', 'historyOnEnter': 'reset', 'retarget': 'snapshot-current'},
        'post': post or {'bloom': {'enabled': True, 'amount': 0.25, 'threshold': 0.9, 'levels': 5},
                         'toneMap': 'reinhard', 'exposureEv': 0.3, 'outputColorSpace': 'srgb', 'grain': 0.025, 'vignette': 0.2},
        'performance': {'qualityTier': 'medium', 'sceneBudgetMs1080p': 3.0, 'deterministicSeed': seed, 'notes': 'target, not measured'},
    }


def stage(sid, shader, params, sources=None, kind='generator', scale=1.0):
    s = {'id': sid, 'kind': kind, 'shader': 'Instrument/' + shader, 'targetFormat': 'rgba16f', 'scale': scale,
         'inputs': {}, 'parameters': params}
    if sources:
        s['sources'] = sources
    return s


PALETTE_ON_SNARE = {'id': 'palette', 'on': 'event.snare', 'minStrength': 0.3, 'refractoryMs': 1500, 'quantize': 'beat',
                    'actions': [{'type': 'palette-advance', 'steps': 1}]}

PRESETS = [
    preset('10 - Liquid Chrome.json', 'liquid-chrome', 'Liquid Chrome',
           'Flowing black/white chrome with a mask floating in it; kicks surge the metal, masks change every 2 bars.',
           [stage('chrome', 'liquid_chrome.fs', [
               param('scale', 0.6, 3.2, 1.1), param('flow', 0.0, 0.8, 0.14, 'Flow', integrate=True),
               param('surge', 0, 1, 0), param('contrast', 0.8, 2.8, 1.9), param('tint', 0, 1, 0.12),
               param('hue', 0, 1, 0.6, cyclic=True), param('mask_mix', 0, 1, 0.85), param('mask_zoom', 0.8, 2.6, 1.35),
               param('glow', 0, 2, 0.4)], {'mask': MASKS})],
           [route('macro.intensity', 'chrome.contrast', 0.5, 0.5), route('macro.intensity', 'chrome.glow', 0.5, 0.5),
            route('macro.motion', 'chrome.flow', 0.9, 0.4), route('macro.color', 'chrome.hue', 1.0, 0.5),
            route('macro.color', 'chrome.tint', 0.6, 0.5), route('macro.space', 'chrome.scale', -0.6, 0.5),
            route('macro.space', 'chrome.mask_zoom', 0.5, 0.5),
            route('env.kick', 'chrome.surge', 1.0), route('env.snare', 'chrome.tint', 0.4),
            route('audio.bass.activity', 'chrome.scale', -0.1, curve='power', exponent=1.5),
            route('audio.high.activity', 'chrome.glow', 0.25), route('lfo.drift', 'chrome.mask_zoom', 0.08, 0.5)],
           seed=101),

    preset('11 - Hot Blobs.json', 'hot-blobs', 'Hot Blobs',
           'Thresholded liquid shapes filled with a vivid gradient and white rims; the palette steps on snares.',
           [stage('blobs', 'hot_blobs.fs', [
               param('scale', 0.8, 4.0, 1.8), param('flow', 0.0, 0.6, 0.12, integrate=True), param('threshold', 0.38, 0.64, 0.53),
               param('rim', 0, 2, 0.7), param('gradient', 0.0, 1.5, 0.3, integrate=True), param('hue', 0, 1, 0.5, cyclic=True),
               param('surge', 0, 1, 0), param('emission', 0.3, 3.0, 1.2), param('grain', 0, 1, 0.35)])],
           [route('macro.intensity', 'blobs.emission', 0.6, 0.5), route('macro.intensity', 'blobs.rim', 0.4, 0.5),
            route('macro.motion', 'blobs.flow', 0.9, 0.4), route('macro.motion', 'blobs.gradient', 0.6, 0.4),
            route('macro.color', 'blobs.hue', 1.0, 0.5), route('macro.space', 'blobs.scale', -0.7, 0.5),
            route('macro.space', 'blobs.threshold', 0.4, 0.5),
            route('env.kick', 'blobs.surge', 1.0), route('env.hat', 'blobs.grain', 0.4),
            route('audio.level.activity', 'blobs.threshold', -0.25, curve='power', exponent=1.4)],
           extra_triggers=[PALETTE_ON_SNARE], seed=202),

    preset('12 - Dot Relief.json', 'dot-relief', 'Dot Relief',
           'Masks rebuilt as a tilted relief of white dots; kicks lift the relief, snares tint it.',
           [stage('dots', 'dot_relief.fs', [
               param('grid', 40, 200, 90), param('depth', 0, 0.6, 0.2), param('tilt', 0, 0.8, 0.35),
               param('dot_size', 0.1, 0.9, 0.45), param('zoom', 0.7, 2.6, 1.2), param('sway', 0.0, 1.0, 0.2, integrate=True),
               param('surge', 0, 1, 0), param('tint', 0, 1, 0), param('hue', 0, 1, 0.1, cyclic=True),
               param('noise_floor', 0, 1, 0.3)], {'source': MASKS})],
           [route('macro.intensity', 'dots.dot_size', 0.4, 0.5), route('macro.intensity', 'dots.depth', 0.4, 0.5),
            route('macro.motion', 'dots.sway', 0.9, 0.4), route('macro.color', 'dots.hue', 1.0, 0.5),
            route('macro.color', 'dots.tint', 0.3, 0.5), route('macro.space', 'dots.grid', 0.6, 0.5),
            route('macro.space', 'dots.zoom', -0.4, 0.5),
            route('env.kick', 'dots.surge', 1.0), route('env.snare', 'dots.tint', 0.8),
            route('audio.bass.activity', 'dots.depth', 0.2), route('audio.high.activity', 'dots.noise_floor', 0.3)],
           seed=303),

    preset('13 - One Bit.json', 'one-bit', 'One Bit',
           'Hard 1-bit threshold with halftone edges and colour speckle; masks swap on every second snare, hats flip polarity.',
           [stage('bits', 'one_bit.fs', [
               param('threshold', 0.2, 0.8, 0.45), param('dither', 0, 1, 0.5), param('cell', 2, 14, 5),
               param('zoom', 0.6, 3.2, 1.4), param('spin', -0.6, 0.6, 0.1, integrate=True), param('invert', 0, 1, 0),
               param('frame', 0, 1, 0), param('speckle', 0, 1, 0.25), param('surge', 0, 1, 0),
               param('texture_amt', 0, 1, 0.5)],
               {'source': {'type': 'images', 'folder': 'Images/Masks', 'advance': 'event.snare', 'every': 2, 'order': 'random'}})],
           [route('macro.intensity', 'bits.texture_amt', 0.5, 0.5), route('macro.intensity', 'bits.speckle', 0.5, 0.5),
            route('macro.motion', 'bits.spin', 0.8, 0.4), route('macro.color', 'bits.dither', 0.8, 0.5),
            route('macro.space', 'bits.zoom', 0.8, 0.5), route('macro.space', 'bits.frame', 1.0, 0.5, curve='smoothstep'),
            route('env.kick', 'bits.surge', 1.0), route('env.hat', 'bits.invert', 1.0),
            route('audio.high.activity', 'bits.speckle', 0.3), route('descriptor.centroid', 'bits.threshold', -0.2, 0.5)],
           seed=404),

    preset('14 - Thermal Slices.json', 'thermal-slices', 'Thermal Slices',
           'Heat-map false colour on the masks, torn into horizontal slices on every hit.',
           [stage('thermal', 'thermal_slices.fs', [
               param('zoom', 0.7, 3.2, 1.5), param('drift', -0.5, 0.5, 0.08, integrate=True), param('slices', 4, 60, 18),
               param('tear', 0, 1, 0), param('map_offset', -0.5, 0.5, 0), param('contrast', 0.5, 3, 1.3),
               param('aberration', 0, 0.04, 0.006), param('texture_amt', 0, 1, 0.45), param('emission', 0.3, 3.5, 1.3)],
               {'source': MASKS})],
           [route('macro.intensity', 'thermal.emission', 0.5, 0.5), route('macro.intensity', 'thermal.contrast', 0.4, 0.5),
            route('macro.motion', 'thermal.drift', 0.7, 0.4), route('macro.color', 'thermal.map_offset', 0.8, 0.5),
            route('macro.space', 'thermal.zoom', 0.8, 0.5), route('macro.space', 'thermal.slices', 0.5, 0.5),
            route('env.kick', 'thermal.tear', 0.9), route('env.snare', 'thermal.tear', 0.6),
            route('env.hat', 'thermal.aberration', 0.4), route('descriptor.centroid', 'thermal.map_offset', 0.2, 0.5)],
           seed=505),

    preset('15 - Holo Shards.json', 'holo-shards', 'Holo Shards',
           'Iridescent thin-film masks with scan rings, white glass shards bursting outward on kicks.',
           [stage('holo', 'holo_shards.fs', [
               param('zoom', 0.7, 2.6, 1.25), param('spin', -0.6, 0.6, 0.12, integrate=True), param('film', 0.5, 6, 2.2),
               param('shimmer', 0.0, 2.0, 0.4, integrate=True), param('shards', 0, 0.9, 0.35), param('burst', 0, 1, 0),
               param('rings', 0, 1, 0.5), param('emission', 0.3, 3.5, 1.3)], {'source': MASKS})],
           [route('macro.intensity', 'holo.emission', 0.5, 0.5), route('macro.intensity', 'holo.shards', 0.4, 0.5),
            route('macro.motion', 'holo.spin', 0.8, 0.4), route('macro.motion', 'holo.shimmer', 0.8, 0.4),
            route('macro.color', 'holo.film', 0.7, 0.5), route('macro.space', 'holo.zoom', 0.7, 0.5),
            route('env.kick', 'holo.burst', 1.0), route('env.snare', 'holo.rings', 0.6),
            route('audio.high.activity', 'holo.shards', 0.25)],
           seed=606),

    preset('16 - Zebra Warp.json', 'zebra-warp', 'Zebra Warp',
           'Op-art stripes bent by a flowing field over a violet-cyan gradient, stepped bars jump in on kicks.',
           [stage('zebra', 'zebra_warp.fs', [
               param('stripes', 4, 40, 14), param('warp', 0, 4, 1.4), param('flow', 0.0, 1.0, 0.18, integrate=True),
               param('band', 0.1, 1.0, 0.35), param('bars', 0, 1, 0), param('hue', 0, 1, 0, cyclic=True),
               param('emission', 0.3, 3.5, 1.2), param('twist', 0, 1, 0)])],
           [route('macro.intensity', 'zebra.emission', 0.5, 0.5), route('macro.intensity', 'zebra.stripes', 0.4, 0.5),
            route('macro.motion', 'zebra.flow', 0.9, 0.4), route('macro.color', 'zebra.hue', 1.0, 0.5),
            route('macro.space', 'zebra.band', 0.8, 0.5),
            route('env.kick', 'zebra.bars', 1.0), route('audio.bass.activity', 'zebra.warp', 0.25),
            route('descriptor.energyTrend', 'zebra.twist', 0.8, 0.0, lo=0.0, hi=1.0)],
           extra_triggers=[PALETTE_ON_SNARE], seed=707),

    preset('17 - Ikeda Bars.json', 'ikeda-bars', 'Ikeda Bars',
           'Minimal white lines that re-arrange on the beat; code bars on hats, hard inverts on snares.',
           [stage('bars', 'ikeda_bars.fs', [
               param('density', 0.05, 0.9, 0.35), param('columns', 8, 200, 48), param('thickness', 0.05, 0.9, 0.18),
               param('step_rate', 0.25, 8, 2), param('code', 0, 1, 0), param('flash', 0, 1, 0),
               param('brightness', 0.3, 4, 1.6), param('scan', 0.0, 2.0, 0.3, integrate=True)])],
           [route('macro.intensity', 'bars.density', 0.6, 0.5), route('macro.intensity', 'bars.brightness', 0.4, 0.5),
            route('macro.motion', 'bars.step_rate', 0.8, 0.4), route('macro.color', 'bars.thickness', 0.6, 0.5),
            route('macro.space', 'bars.columns', 0.7, 0.5),
            route('env.hat', 'bars.code', 1.0), route('env.kick', 'bars.code', 0.5), route('env.snare', 'bars.flash', 1.0)],
           post={'bloom': {'enabled': True, 'amount': 0.15, 'threshold': 1.2, 'levels': 4},
                 'toneMap': 'reinhard', 'exposureEv': 0.8, 'outputColorSpace': 'srgb', 'grain': 0.01, 'vignette': 0.0},
           seed=808),

    preset('18 - Signal Type.json', 'signal-type', 'Signal Type',
           'Chunky extruded words over angular duotone shards, a new word every beat, smeared and split on kicks.',
           [stage('type', 'extruded_type.fs', [
               param('depth', 0, 0.35, 0.12), param('angle', -3.14, 3.14, -0.6), param('zoom', 0.6, 2.2, 1.1),
               param('smear', 0, 1, 0), param('aberration', 0, 0.03, 0.004), param('hue', 0, 1, 0.15, cyclic=True),
               param('background', 0, 1, 0.6), param('wobble', 0.0, 1.5, 0.3, integrate=True), param('emission', 0.3, 3.5, 1.3)],
               {'word': {'type': 'text', 'words': ['VOLT', 'NERVE', 'SIGNAL', 'ECHO', 'NULL'], 'font': 'Arial Black',
                         'advance': 'beat', 'every': 1, 'order': 'sequence'}})],
           [route('macro.intensity', 'type.depth', 0.6, 0.5), route('macro.intensity', 'type.emission', 0.3, 0.5),
            route('macro.motion', 'type.wobble', 0.8, 0.4), route('macro.color', 'type.hue', 1.0, 0.5),
            route('macro.space', 'type.zoom', 0.7, 0.5), route('macro.space', 'type.background', 0.6, 0.5),
            route('env.kick', 'type.smear', 1.0), route('env.snare', 'type.aberration', 0.6),
            route('lfo.drift', 'type.angle', 0.12, 0.5)],
           seed=909),

    preset('19 - Mask Kaleido.json', 'mask-kaleido', 'Mask Kaleido',
           'Masks tiled and mirrored over a wavy duotone field, solarised; snares turn the tiles.',
           [stage('kaleido', 'mask_kaleido.fs', [
               param('tiles', 1, 8, 3), param('wave', 0, 1.5, 0.35), param('flow', 0.0, 1.2, 0.2, integrate=True),
               param('solarize', 0, 1, 0.5), param('turn', 0, 1, 0), param('hue', 0, 1, 0.55, cyclic=True),
               param('fill', 0.4, 1.4, 0.85), param('emission', 0.3, 3.5, 1.1)], {'source': MASKS})],
           [route('macro.intensity', 'kaleido.solarize', 0.7, 0.5), route('macro.intensity', 'kaleido.emission', 0.3, 0.5),
            route('macro.motion', 'kaleido.flow', 0.9, 0.4), route('macro.color', 'kaleido.hue', 1.0, 0.5),
            route('macro.space', 'kaleido.tiles', 0.8, 0.5),
            route('env.snare', 'kaleido.turn', 1.0), route('env.kick', 'kaleido.fill', 0.2),
            route('audio.bass.activity', 'kaleido.wave', 0.3)],
           seed=1001),

    preset('20 - Molten Smear.json', 'molten-smear', 'Molten Smear',
           'Holographic masks dragged through a flowing feedback memory - melted, liquid trails; kicks push the flow.',
           [stage('holo', 'holo_shards.fs', [
               param('zoom', 0.7, 2.6, 1.3), param('spin', -0.6, 0.6, 0.1, integrate=True), param('film', 0.5, 6, 2.6),
               param('shimmer', 0.0, 2.0, 0.5, integrate=True), param('shards', 0, 0.9, 0.15), param('burst', 0, 1, 0),
               param('rings', 0, 1, 0.3), param('emission', 0.3, 3.5, 1.2)], {'source': MASKS}),
            stage('smear', 'feedback_smear.fs', [
                param('persistence', 0.1, 4.0, 1.2), param('flow', 0.0, 1.0, 0.25, integrate=True),
                param('push', 0, 0.03, 0.006), param('zoom', -0.02, 0.02, 0.004), param('aberration', 0, 0.02, 0.003),
                param('inject', 0, 1, 0.7), param('flush', 0, 1, 0)], kind='effect')],
           [route('macro.intensity', 'smear.persistence', 0.6, 0.5), route('macro.motion', 'smear.flow', 0.8, 0.4),
            route('macro.motion', 'holo.spin', 0.6, 0.4), route('macro.color', 'holo.film', 0.7, 0.5),
            route('macro.space', 'smear.zoom', 0.6, 0.5), route('macro.space', 'holo.zoom', 0.5, 0.5),
            route('env.kick', 'smear.push', 0.7), route('env.kick', 'holo.burst', 0.8),
            route('env.snare', 'smear.aberration', 0.5), route('audio.bass.activity', 'smear.push', 0.2)],
           transition={'type': 'crossfade', 'durationMs': 800, 'quantize': 'bar', 'historyOnEnter': 'reset', 'retarget': 'snapshot-current'},
           seed=1111),
]

if __name__ == '__main__':
    for file_name, body in PRESETS:
        with open(os.path.join(OUT, file_name), 'w', encoding='utf-8') as f:
            json.dump(body, f, indent=2)
            f.write('\n')
        print('wrote', file_name)
