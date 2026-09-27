"""Builds the schema-2 scene presets (engine/Presets/xx - *.json).

Each preset = one generative treatment (Shaders/Instrument/*.fs) + a musical
mapping. Colour is NOT decided here: the engine's global look pass maps every
scene through the performer's 3-colour palette (plugin "Look" section), so
scenes only need to deliver light, shape and motion.

The control model (docs/REVIEW-2026-09-24-HE.md, section 5) - four groups,
one rule each:

  MOVE   "0 = still". Speed (macro 1), Glide (macro 6), Drift, Push, Sync,
         Reverse, Freeze are GLOBAL: the engine's scene clock multiplies every
         integrated ("rate") parameter and drives the shaders' vj_time.
         So scenes only state their designed speeds (the rate's default) and
         NEVER route anything into a rate - checked below.
  REACT  "hits never touch speed". The engine's reaction layer (Reaction.h)
         turns the analysis into react.* signals and accent / tick / drop
         events; REACT (macro 4) and the character (BREATHE / PULSE / PUNCH)
         scale them globally, so scenes only say WHAT each one moves:
           react.energy  -> the REST look (centre .5 = the tuned look, silence
                            carries the scene to its rest pose)
           env.hit       -> the HIT event (accents at 1, ticks at the tick level)
           react.body    -> the BODY (bass)
           react.tension -> the BUILD (the inhale before a drop)
           env.drop      -> the scene's own DROP extra (bloom, lurch and the
                            tension / scar reset are global)
           react.air / env.snare / react.scar -> surface, snare, memory
  SHAPE  "every knob visible in every scene, always the same direction":
           0 Intensity  light / energy / amount
           2 Form       the scene's character (its own name in the plug-in)
           3 Scale      bigger as it rises
           5 Erode      pristine -> worn, torn, dissolved
           7 Detail     density / fineness
         Each preset names what these do for it (macro labels), and the
         plug-in shows those names under the knobs.
  DIRT   film and digital disturbance live in the global look pass.

Other rules checked here: a parameter takes at most one audio-driven source
(audio / descriptor / hit envelope) - nothing reacts twice; speeds are never
negative (direction is the global Reverse switch); no hit reseeds a scene by
default (random jumps) - a scene that wants cuts asks for it (Negative, on
accents only). Every scene has a REST, HIT, BODY and BUILD route.

  envs     hit, snare, drop (shape from the character, live), pulse (1/80 ms)
  lfos     phrase (16 bars, on the scene clock)
  user     the manual hit (Space bar / /v2/trigger / plugin HIT) is a forced
           accent in the engine - no trigger of its own

Run after editing:  python build_instrument_presets.py
The engine validates every file on load and logs any rejection to VJEngine.log.
"""
import glob
import json
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'Presets')

# id, slot, generic label, default. Speed and Glide are read by the engine's
# scene clock, never routed by a scene.
MACRO_SLOTS = [
    ('intensity', 0, 'Intensity', 0.5),
    ('speed', 1, 'Speed', 0.5),
    ('form', 2, 'Form', 0.5),
    ('scale', 3, 'Scale', 0.5),
    ('react', 4, 'React', 0.5),
    ('erode', 5, 'Erode', 0.5),
    ('glide', 6, 'Glide', 0.25),
    ('detail', 7, 'Detail', 0.5),
]
SHAPE_MACROS = ('intensity', 'form', 'scale', 'erode', 'detail')
GLOBAL_MACROS = ('speed', 'glide')


def macros(labels):
    """The 8 macro slots, with this scene's own names for what they do."""
    out = []
    for mid, slot, generic, default in MACRO_SLOTS:
        out.append({'id': mid, 'slot': slot, 'label': labels.get(mid, generic), 'default': default})
    return out


ENVS = [
    # Shapes come from the performer's character (Reaction.h), every frame.
    {'id': 'hit', 'type': 'ad', 'attackMs': 0, 'decayMs': 1300, 'peak': 1, 'retrigger': 'max', 'style': 'hit'},
    {'id': 'snare', 'type': 'ad', 'attackMs': 2, 'decayMs': 800, 'peak': 1, 'retrigger': 'restart', 'style': 'snare'},
    {'id': 'drop', 'type': 'ad', 'attackMs': 0, 'decayMs': 4000, 'peak': 1, 'retrigger': 'max', 'style': 'drop'},
    # Fixed one-shot (throws, seeds): 1 ms up, gone in 80 ms.
    {'id': 'pulse', 'type': 'ad', 'attackMs': 1, 'decayMs': 80, 'peak': 1, 'retrigger': 'restart'},
    # Phrase-locked: one cycle every 16 bars (on the scene clock) - slow drift for ambient sets.
    {'id': 'phrase', 'type': 'lfo', 'shape': 'sine', 'periodBeats': 64, 'phaseOffset': 0, 'polarity': 'bipolar'},
]


def triggers(reseed_on_accent=False, pulse_on_drop=False):
    accent = [{'type': 'envelope', 'target': 'hit', 'amount': 1},
              {'type': 'envelope', 'target': 'pulse', 'amount': 1}]
    drop = [{'type': 'envelope', 'target': 'drop', 'amount': 1}]
    if pulse_on_drop:
        drop.append({'type': 'envelope', 'target': 'pulse', 'amount': 1})
    if reseed_on_accent:
        accent.append({'type': 'reseed'})
        drop.append({'type': 'reseed'})
    return [
        {'id': 'accent', 'on': 'event.accent', 'minStrength': 0, 'refractoryMs': 0, 'quantize': 'none', 'actions': accent},
        {'id': 'tick', 'on': 'event.tick', 'minStrength': 0, 'refractoryMs': 0, 'quantize': 'none',
         'actions': [{'type': 'envelope', 'target': 'hit', 'amount': 1, 'scale': 'tick'}]},
        {'id': 'snare', 'on': 'event.snare', 'minStrength': 0.1, 'refractoryMs': 70, 'quantize': 'none',
         'actions': [{'type': 'envelope', 'target': 'snare', 'amount': 1, 'scale': 'snare'}]},
        {'id': 'drop', 'on': 'event.drop', 'minStrength': 0, 'refractoryMs': 0, 'quantize': 'none', 'actions': drop},
    ]


FORMS = {'type': 'images', 'folder': 'Images/Forms', 'advance': 'bar', 'every': 2, 'order': 'random'}
# The performer's active media slot; the forms show while nothing is loaded.
MEDIA = {'type': 'media', 'fallback': 'Images/Forms', 'advance': 'none'}


def param(name, lo, hi, default, label=None, cyclic=False, integrate=False):
    p = {'name': name, 'label': label or name.replace('_', ' ').title(), 'min': lo, 'max': hi,
         'default': default, 'unit': 'normalized', 'cyclic': cyclic}
    if integrate:
        p['integrate'] = True
    return p


def route(src, dst, amount, center=0.0, curve='linear', exponent=1.0, attack=None, release=None, lo=0.0, hi=1.0,
          scale_by=None):
    kind = src.split('.')[0]
    if attack is None:
        attack = {'macro': 60, 'audio': 12, 'descriptor': 120}.get(kind, 0)
    if release is None:
        release = {'macro': 60, 'audio': 250, 'descriptor': 400}.get(kind, 0)
    r = {'id': (src + '-' + dst).replace('.', '-')[:64].lower(), 'source': src, 'destination': 'stage.' + dst,
         'inputMin': lo, 'inputMax': hi, 'curve': curve, 'exponent': exponent, 'center': center,
         'amount': amount, 'attackMs': attack, 'releaseMs': release}
    if scale_by:
        r['scaleBy'] = scale_by
    return r


def preset(file_name, pid, name, description, stages, routes, labels, post=None, transition=None, seed=1,
           reseed_on_accent=False, pulse_on_drop=False):
    body = {
        'schemaVersion': '2.0',
        'id': pid,
        'name': name,
        'description': description,
        'author': 'VJ VST',
        'license': 'project-internal',
        'engine': {'minimumVersion': '0.2.0', 'shaderProfile': 'isf-legacy', 'workingColorSpace': 'linear-srgb', 'featureContract': '2.0'},
        'sourcePolicy': {'bass': 'kick-or-mix', 'mid': 'snare-or-mix', 'high': 'hat-or-mix', 'level': 'mix'},
        'stages': stages,
        'macros': macros(labels),
        'modulators': ENVS,
        'routes': routes,
        'triggers': triggers(reseed_on_accent, pulse_on_drop),
        'transition': transition or {'type': 'cut', 'durationMs': 0, 'quantize': 'beat', 'historyOnEnter': 'reset', 'retarget': 'snapshot-current'},
        # Grain lives in the global look pass now; the preset only finishes light.
        'post': post or {'bloom': {'enabled': True, 'amount': 0.22, 'threshold': 0.9, 'levels': 5},
                         'toneMap': 'reinhard', 'exposureEv': 0.3, 'outputColorSpace': 'srgb', 'grain': 0.0, 'vignette': 0.25},
        'performance': {'qualityTier': 'medium', 'sceneBudgetMs1080p': 3.0, 'deterministicSeed': seed, 'notes': 'target, not measured'},
    }
    check(file_name, body)
    return file_name, body


def stage(sid, shader, params, sources=None, kind='generator', scale=1.0):
    s = {'id': sid, 'kind': kind, 'shader': 'Instrument/' + shader, 'targetFormat': 'rgba16f', 'scale': scale,
         'inputs': {}, 'parameters': params}
    if sources:
        s['sources'] = sources
    return s


MEDIA_FRAME_PARAMS = None   # filled in below (needs param())
NEGATIVE_PARAMS = None
DUO_POST = {'palette': 'duo', 'bloom': {'enabled': True, 'amount': 0.12, 'threshold': 0.85, 'levels': 4},
            'toneMap': 'none', 'exposureEv': 0.0, 'outputColorSpace': 'srgb', 'grain': 0.0, 'vignette': 0.2}


def check(file_name, body):
    """The control-model rules (see the module docstring)."""
    params = {}
    for st in body['stages']:
        for p in st['parameters']:
            params['stage.%s.%s' % (st['id'], p['name'])] = p
            if p.get('integrate'):
                assert p['min'] >= 0, '%s: speed %s can go negative (direction is Reverse)' % (file_name, p['name'])
    audio_sources = {}
    shape_used = set()
    for r in body['routes']:
        dst, src = r['destination'], r['source']
        assert dst in params, '%s: route to unknown %s' % (file_name, dst)
        assert not params[dst].get('integrate'), '%s: %s routes into speed %s' % (file_name, src, dst)
        kind, _, rest = src.partition('.')
        if kind == 'macro':
            assert rest not in GLOBAL_MACROS, '%s: %s is global (scene clock), not routable' % (file_name, src)
            assert rest in SHAPE_MACROS or rest == 'react', '%s: unknown macro %s' % (file_name, src)
            if rest in SHAPE_MACROS:
                shape_used.add(rest)
        if kind in ('audio', 'descriptor', 'env', 'react'):
            audio_sources.setdefault(dst, []).append(src)
    for dst, srcs in audio_sources.items():
        assert len(srcs) == 1, '%s: %s reacts twice (%s)' % (file_name, dst, ', '.join(srcs))
    sources = {r['source'] for r in body['routes']}
    for need, options in (('REST', ('react.energy', 'react.rest')), ('HIT', ('env.hit', 'env.pulse')),
                          ('BODY', ('react.body',)), ('BUILD', ('react.tension',))):
        assert sources & set(options), '%s: no %s route (%s)' % (file_name, need, ' / '.join(options))
    missing = set(SHAPE_MACROS) - shape_used
    assert not missing, '%s: knobs that do nothing here: %s' % (file_name, ', '.join(sorted(missing)))
    labels = {m['id']: m['label'] for m in body['macros']}
    for mid in SHAPE_MACROS:
        assert labels[mid] and labels[mid][0].islower(), '%s: give %s a scene-specific name' % (file_name, mid)


MEDIA_FRAME_PARAMS = [param('zoom', 0.6, 2.5, 1.0), param('fit', 0, 1, 0.0), param('wander', 0.0, 0.5, 0.1, integrate=True),
                      param('roam', 0, 1, 0.5), param('surge', 0, 2.5, 0)]
NEGATIVE_PARAMS = [param('polarity', 0, 1, 1), param('threshold', 0.2, 0.8, 0.45), param('contrast', 0, 1, 0.6),
                   param('detail', 0, 3, 1.0), param('outline', 0, 1, 0.12), param('dither', 0, 1, 0.0),
                   param('storm', 0, 1, 0), param('cell', 1, 6, 2), param('flash', 0, 1, 0)]

MEDIA_NEGATIVE_PARAMS = [dict(p, default=d) if p['name'] in d_ else p for p in NEGATIVE_PARAMS
                         for d_ in [{'threshold': 0.6, 'contrast': 0.75, 'detail': 1.4}] for d in [d_.get(p['name'])]]


def rest(dst, amount, **kw):
    """REST: silence carries the scene to its rest pose; centre .5 = the tuned look."""
    return route('react.energy', dst, amount, 0.5, **kw)


def hit(dst, amount=1.0):
    return route('env.hit', dst, amount)


def body(dst, amount, center=0.0):
    return route('react.body', dst, amount, center)


def build(dst, amount, **kw):
    return route('react.tension', dst, amount, **kw)


def drop(dst, amount):
    return route('env.drop', dst, amount)


def air(dst, amount):
    return route('react.air', dst, amount, 0.3)


def snare(dst, amount):
    return route('env.snare', dst, amount)


def scar(dst, amount):
    return route('react.scar', dst, amount)


PRESETS = [
    preset('01 - Hot Blobs.json', 'hot-blobs', 'Hot Blobs',
           'A liquid mass hit by sound: each accent throws it and drops a stone into a ripple tank, gravity pulls it back. '
           'In silence only a few dim islands remain.',
           [stage('blobs', 'hot_blobs.fs', [
               param('scale', 0.8, 4.0, 1.8), param('flow', 0.0, 0.6, 0.1, integrate=True), param('threshold', 0.38, 0.64, 0.53),
               param('drop', 0, 1, 0), param('impact', 0, 2, 0.8), param('gravity', 0, 2, 0.6),
               param('viscosity', 0, 1, 0.45), param('ripple', 0, 1.5, 0.7), param('rim', 0, 2, 0.7),
               param('emission', 0.3, 3.0, 1.2), param('curl', 0.4, 3.0, 1.6)])],
           [route('macro.intensity', 'blobs.emission', 0.6, 0.5), route('macro.intensity', 'blobs.rim', 0.4, 0.5),
            route('macro.form', 'blobs.ripple', 0.8, 0.5), route('macro.scale', 'blobs.scale', -0.7, 0.5),
            route('macro.erode', 'blobs.threshold', 0.5, 0.5), route('macro.detail', 'blobs.curl', 0.9, 0.5),
            route('macro.react', 'blobs.impact', 0.9, 0.5),
            rest('blobs.threshold', -0.5), route('env.pulse', 'blobs.drop', 1.0), body('blobs.emission', 0.3, 0.4),
            build('blobs.ripple', 0.5), drop('blobs.impact', 0.4), snare('blobs.rim', 0.35),
            route('lfo.phrase', 'blobs.scale', 0.12, 0.5)],
           {'intensity': 'glow', 'form': 'ripples', 'scale': 'blob size', 'erode': 'break-up', 'detail': 'curl',
            'react': 'throw'},
           seed=202, pulse_on_drop=True),

    preset('02 - Dot Relief.json', 'dot-relief', 'Dot Relief',
           'Abstract 3D forms rebuilt as a tilted relief of dots; accents lift the relief, a new form every 2 bars. '
           'In silence it flattens to a quiet dot grid.',
           [stage('dots', 'dot_relief.fs', [
               param('grid', 40, 200, 90), param('depth', 0, 0.6, 0.22), param('tilt', 0, 0.8, 0.35),
               param('dot_size', 0.1, 0.9, 0.45), param('zoom', 0.7, 2.6, 1.15), param('sway', 0.0, 1.0, 0.2, integrate=True),
               param('surge', 0, 2, 0), param('tint', 0, 1, 0), param('hue', 0, 1, 0.1, cyclic=True),
               param('noise_floor', 0, 1, 0.12)], {'source': FORMS})],
           [route('macro.intensity', 'dots.dot_size', 0.4, 0.5), route('macro.form', 'dots.tilt', 0.8, 0.5),
            route('macro.scale', 'dots.zoom', -0.5, 0.5), route('macro.erode', 'dots.noise_floor', 0.8, 0.5),
            route('macro.detail', 'dots.grid', 0.8, 0.5),
            rest('dots.depth', 0.6), hit('dots.surge'), body('dots.dot_size', 0.3, 0.4), build('dots.tilt', 0.35),
            drop('dots.zoom', -0.25), air('dots.noise_floor', 0.25),
            route('lfo.phrase', 'dots.tilt', 0.15, 0.5)],
           {'intensity': 'dots', 'form': 'tilt', 'scale': 'zoom', 'erode': 'dissolve', 'detail': 'grid'},
           seed=303),

    preset('03 - One Bit.json', 'one-bit', 'One Bit',
           'Hard 1-bit threshold of abstract 3D forms with halftone edges and speckle; forms swap on every second accent, '
           'build-ups close a letterbox, the drop flips it negative.',
           [stage('bits', 'one_bit.fs', [
               param('threshold', 0.2, 0.8, 0.45), param('dither', 0, 1, 0.5), param('cell', 2, 14, 5),
               param('zoom', 0.6, 3.2, 1.3), param('spin', 0.0, 0.6, 0.1, integrate=True), param('invert', 0, 1, 0),
               param('frame', 0, 1, 0), param('speckle', 0, 1, 0.25), param('surge', 0, 1.8, 0),
               param('texture_amt', 0, 1, 0.5)],
               {'source': dict(FORMS, advance='event.accent')})],
           [route('macro.intensity', 'bits.threshold', -0.4, 0.5), route('macro.intensity', 'bits.texture_amt', 0.3, 0.5),
            route('macro.form', 'bits.dither', 0.8, 0.5), route('macro.scale', 'bits.zoom', -0.8, 0.5),
            route('macro.erode', 'bits.speckle', 0.8, 0.5), route('macro.detail', 'bits.cell', -0.8, 0.5),
            rest('bits.threshold', -0.35), hit('bits.surge'), body('bits.zoom', -0.05),
            build('bits.dither', 0.4), build('bits.frame', 1.0, lo=0.55, release=400),
            drop('bits.invert', 1.0), air('bits.speckle', 0.3), snare('bits.texture_amt', 0.3)],
           {'intensity': 'white', 'form': 'dither', 'scale': 'zoom', 'erode': 'speckle', 'detail': 'cell'},
           seed=404),

    preset('04 - Corridor.json', 'corridor', 'Corridor',
           'Moving forward through an endless service corridor; accents flare the lights and lurch the camera forward, '
           'snares punch the lens. In silence the hall dissolves into fog.',
           [stage('hall', 'corridor.fs', [
               # Capped at 2.5 u/s: faster, the wall texture aliases at 60 fps and reads as strobing.
               param('travel', 0.0, 2.5, 1.0, integrate=True), param('sway', 0.0, 1.5, 0.4, integrate=True),
               param('sway_amt', 0, 1, 0.3), param('width', 0.5, 2.5, 1.0), param('surge', 0, 1, 0),
               param('flicker', 0, 1, 0), param('detail', 0, 1, 0.5), param('fog', 0.05, 1.0, 0.35),
               param('emission', 0.2, 4.0, 1.1), param('decay', 0, 1, 0.2)])],
           [route('macro.intensity', 'hall.emission', 0.6, 0.5), route('macro.form', 'hall.fog', 0.7, 0.5),
            route('macro.scale', 'hall.width', 0.7, 0.5), route('macro.erode', 'hall.decay', 0.8, 0.5),
            route('macro.detail', 'hall.detail', 1.0, 0.5),
            rest('hall.fog', -0.5), hit('hall.emission', 0.35), body('hall.width', -0.12, 0.4),
            build('hall.flicker', 0.6), drop('hall.sway_amt', 0.4), snare('hall.surge', 0.6),
            route('lfo.phrase', 'hall.sway_amt', 0.2, 0.5)],
           {'intensity': 'lights', 'form': 'fog', 'scale': 'width', 'erode': 'dead lights', 'detail': 'grime'},
           seed=505),

    preset('05 - Fibers.json', 'fibers', 'Fibers',
           'A dense web of filaments around a dark knot - roots, nerves, torn cloth; bass swells it, accents tear it outward, '
           'layers pile up through a section. In silence only hairlines remain.',
           [stage('web', 'fibers.fs', [
               param('drift', 0.0, 1.2, 0.35, integrate=True), param('turn', 0.0, 0.3, 0.04, integrate=True),
               param('zoom', 0.5, 3.0, 1.2), param('density', 0, 1, 0.5), param('thickness', 0.05, 1.0, 0.4),
               param('tear', 0, 2, 0), param('swell', 0, 1, 0), param('knot', 0, 1, 0.6),
               param('emission', 0.2, 4.0, 1.4)])],
           [route('macro.intensity', 'web.emission', 0.6, 0.5), route('macro.intensity', 'web.thickness', 0.4, 0.5),
            route('macro.form', 'web.knot', 0.8, 0.5), route('macro.scale', 'web.zoom', 0.7, 0.5),
            route('macro.erode', 'web.tear', 0.6, 0.5), route('macro.detail', 'web.density', 1.0, 0.5),
            rest('web.thickness', 0.5), hit('web.tear'), body('web.swell', 0.9), build('web.knot', 0.4),
            drop('web.zoom', -0.2), snare('web.emission', 0.15), scar('web.density', 0.3),
            route('lfo.phrase', 'web.zoom', 0.12, 0.5)],
           {'intensity': 'glow', 'form': 'knot', 'scale': 'zoom', 'erode': 'tear', 'detail': 'layers'},
           seed=606),

    preset('06 - Terminal.json', 'terminal', 'Terminal',
           'An invented machine script scrolling over dark blots; accents jump the scroll, snares invert a band, '
           'the text wears out through a section. In silence the screen is nearly empty.',
           [stage('term', 'terminal.fs', [
               param('scroll', 0.0, 6.0, 1.0, integrate=True), param('rows', 10, 60, 26), param('fill', 0, 1, 0.6),
               param('jump', 0, 1, 0), param('invert', 0, 1, 0), param('background', 0, 1, 0.5),
               param('morph', 0.0, 1.0, 0.2, integrate=True), param('emission', 0.2, 4.0, 1.3),
               param('decay', 0, 1, 0.1)])],
           [route('macro.intensity', 'term.emission', 0.6, 0.5), route('macro.form', 'term.background', 0.9, 0.5),
            route('macro.scale', 'term.rows', -0.8, 0.5), route('macro.erode', 'term.decay', 0.8, 0.5),
            route('macro.detail', 'term.fill', 1.0, 0.5),
            rest('term.fill', 0.8, release=800), hit('term.jump', 0.75), body('term.emission', 0.25, 0.4),
            build('term.background', 0.5), snare('term.invert', 1.0), scar('term.decay', 0.5)],
           {'intensity': 'glow', 'form': 'blots', 'scale': 'text size', 'erode': 'dropout', 'detail': 'density'},
           post={'bloom': {'enabled': True, 'amount': 0.15, 'threshold': 1.0, 'levels': 4},
                 'toneMap': 'reinhard', 'exposureEv': 0.5, 'outputColorSpace': 'srgb', 'grain': 0.0, 'vignette': 0.15},
           seed=707),

    preset('07 - Ink.json', 'ink', 'Ink',
           'Blots of ink or blood soaking through paper, torn edges, dot holes and smear bands; accents rip it, bass swells it, '
           'holes punch through as a section goes on and heal on the drop.',
           [stage('ink', 'ink.fs', [
               param('morph', 0.0, 1.2, 0.25, integrate=True), param('fall', 0.0, 0.8, 0.1, integrate=True),
               param('scale', 0.5, 5.0, 1.6), param('coverage', 0.2, 0.8, 0.36), param('swell', 0, 1, 0),
               param('smear', 0, 1, 0.3), param('rip', 0, 1, 0), param('dots', 0, 1, 0.5),
               param('emission', 0.2, 4.0, 1.3), param('ragged', 0, 3, 1.0)])],
           [route('macro.intensity', 'ink.coverage', 0.5, 0.5), route('macro.intensity', 'ink.emission', 0.3, 0.5),
            route('macro.form', 'ink.smear', 0.8, 0.5), route('macro.scale', 'ink.scale', -0.6, 0.5),
            route('macro.erode', 'ink.dots', 1.0, 0.5), route('macro.detail', 'ink.ragged', 0.9, 0.5),
            rest('ink.coverage', 0.4), hit('ink.rip'), body('ink.swell', 0.9), build('ink.smear', 0.5),
            scar('ink.dots', 0.5)],
           {'intensity': 'ink', 'form': 'smear', 'scale': 'blot size', 'erode': 'holes', 'detail': 'torn edges'},
           seed=808),

    preset('08 - Signal Fog.json', 'signal-fog', 'Signal Fog',
           'After Rainer Kohlberger: extremely fine particles flutter over black while large forms condense out of the haze; '
           'build-ups swallow them in haze, the drop snaps them into focus.',
           [stage('fog', 'signal_fog.fs', [
               param('clock', 0.0, 0.6, 0.15, integrate=True), param('scale', 0.4, 4.0, 1.3), param('focus', 0, 1, 0.3),
               param('density', 0, 1, 0.3), param('haze', 0, 1, 0.25), param('grain_size', 1.0, 4.0, 1.4),
               param('surge', 0, 1.6, 0), param('emission', 0.2, 4.0, 1.3)])],
           [route('macro.intensity', 'fog.emission', 0.5, 0.5), route('macro.intensity', 'fog.haze', 0.5, 0.5),
            route('macro.form', 'fog.grain_size', 0.6, 0.5), route('macro.scale', 'fog.scale', -0.6, 0.5),
            route('macro.erode', 'fog.focus', -0.6, 0.5), route('macro.detail', 'fog.density', 0.8, 0.5),
            rest('fog.density', 0.5), hit('fog.surge'), body('fog.emission', 0.2, 0.4), build('fog.haze', 0.5),
            drop('fog.focus', 0.8),
            route('lfo.phrase', 'fog.focus', 0.15, 0.5)],
           {'intensity': 'glow', 'form': 'grain', 'scale': 'form size', 'erode': 'blur', 'detail': 'particles'},
           post={'bloom': {'enabled': True, 'amount': 0.18, 'threshold': 0.9, 'levels': 5},
                 'toneMap': 'reinhard', 'exposureEv': 0.4, 'outputColorSpace': 'srgb', 'grain': 0.0, 'vignette': 0.3},
           seed=909),

    preset('09 - Mesh Body.json', 'mesh-body', 'Mesh Body',
           'After The Noise Diary: a breathing body woven from fine plexus lines with drifting openings; accents swell it, '
           'build-ups erode it into dust, the drop re-knits it.',
           [stage('mesh', 'mesh_body.fs', [
               param('turn', 0.0, 0.4, 0.08, integrate=True), param('breathe', 0.0, 1.5, 0.3, integrate=True),
               param('size', 0.3, 1.1, 0.62), param('density', 5, 22, 11), param('erosion', 0, 1, 0.12),
               param('hole', 0, 1, 0.75), param('surge', 0, 3.0, 0), param('emission', 0.2, 4.0, 1.2)])],
           [route('macro.intensity', 'mesh.emission', 0.6, 0.5), route('macro.form', 'mesh.hole', 0.8, 0.5),
            route('macro.scale', 'mesh.size', 0.6, 0.5), route('macro.erode', 'mesh.erosion', 0.7, 0.5),
            route('macro.detail', 'mesh.density', 0.9, 0.5),
            rest('mesh.emission', 0.25), hit('mesh.surge'), body('mesh.size', 0.12), build('mesh.erosion', 0.7),
            route('lfo.phrase', 'mesh.size', 0.1, 0.5)],
           {'intensity': 'glow', 'form': 'opening', 'scale': 'size', 'erode': 'erosion', 'detail': 'mesh'},
           seed=1010),

    preset('10 - Morphogen.json', 'morphogen', 'Morphogen',
           'Reaction-diffusion grown live and lit as a relief; accents plant seeds that grow roots, Form slides between '
           'fingerprint, coral, mitosis and worms, the drop floods the culture past its island.',
           [stage('rd', 'morphogen.fs', [
               param('pattern', 0, 1, 0.35), param('rate', 0.2, 1.0, 0.9), param('seed_amt', 0, 1, 0),
               param('zoom', 0.6, 2.0, 1.0), param('relief', 0, 1.5, 0.6), param('boundary', 0, 1, 0.55),
               param('emission', 0.2, 4.0, 1.2)])],
           [route('macro.intensity', 'rd.emission', 0.6, 0.5), route('macro.form', 'rd.pattern', 0.9, 0.5),
            route('macro.scale', 'rd.zoom', 0.6, 0.5), route('macro.scale', 'rd.boundary', -0.6, 0.5),
            route('macro.erode', 'rd.boundary', 0.5, 0.5), route('macro.detail', 'rd.relief', 0.8, 0.5),
            rest('rd.relief', 0.5), route('env.pulse', 'rd.seed_amt', 1.0), body('rd.emission', 0.25, 0.4),
            build('rd.pattern', 0.2), drop('rd.boundary', -0.3),
            route('lfo.phrase', 'rd.pattern', 0.15, 0.5)],
           {'intensity': 'glow', 'form': 'pattern', 'scale': 'zoom', 'erode': 'island', 'detail': 'relief'},
           transition={'type': 'cut', 'durationMs': 0, 'quantize': 'beat', 'historyOnEnter': 'keep', 'retarget': 'snapshot-current'},
           seed=1111, pulse_on_drop=True),

    preset('11 - Halo Ring.json', 'halo-ring', 'Halo Ring',
           'After The Noise Diary: a thin ring on black whose edge the sound pushes - bass swells slow lobes, highs grow radial hairs, '
           'accents flare it, build-ups fray it. In silence one clean ring remains.',
           [stage('ring', 'halo_ring.fs', [
               param('radius', 0.2, 0.95, 0.55), param('spin', 0.0, 0.4, 0.05, integrate=True),
               param('wobble', 0.0, 2.0, 0.4, integrate=True), param('low', 0, 1, 0.05), param('hair', 0, 1, 0.15),
               param('fur', 0, 1, 0.4), param('surge', 0, 3.0, 0), param('emission', 0.2, 4.0, 1.3),
               param('fray', 0, 1, 0.1)])],
           [route('macro.intensity', 'ring.emission', 0.5, 0.5), route('macro.form', 'ring.fur', 0.8, 0.5),
            route('macro.scale', 'ring.radius', 0.6, 0.5), route('macro.erode', 'ring.fray', 0.8, 0.5),
            route('macro.detail', 'ring.hair', 0.8, 0.5),
            rest('ring.fur', 0.6), hit('ring.surge'), body('ring.low', 0.8), build('ring.fray', 0.5),
            drop('ring.radius', 0.25), air('ring.hair', 0.7),
            route('lfo.phrase', 'ring.radius', 0.08, 0.5)],
           {'intensity': 'glow', 'form': 'fur', 'scale': 'radius', 'erode': 'fray', 'detail': 'hair'},
           seed=1212),

    preset('12 - Emergence.json', 'emergence', 'Emergence',
           'After The Noise Diary: an abstract form rises out of darkness with vertical light streaks, dissolves into dust as the '
           'music builds, appears on the drop. In silence only a faint trace.',
           [stage('rise', 'emergence.fs', [
               param('zoom', 0.6, 3.0, 1.7), param('visible', 0, 1, 0.45), param('streak', 0, 1, 0.5),
               param('dissolve', 0, 1, 0.12), param('drift', 0.0, 1.5, 0.6, integrate=True),
               param('surge', 0, 4.0, 0), param('emission', 0.2, 4.0, 0.95), param('fine', 0, 1, 0.5)],
               {'source': dict(FORMS, every=8)})],
           [route('macro.intensity', 'rise.emission', 0.5, 0.5), route('macro.form', 'rise.streak', 0.8, 0.5),
            route('macro.scale', 'rise.zoom', -0.6, 0.5), route('macro.erode', 'rise.dissolve', 0.7, 0.5),
            route('macro.detail', 'rise.fine', 0.8, 0.5),
            rest('rise.visible', 0.7), hit('rise.surge'), hit('rise.emission', 0.8), body('rise.zoom', -0.08), build('rise.dissolve', 0.7),
            air('rise.streak', 0.3),
            route('lfo.phrase', 'rise.visible', 0.25, 0.5)],
           {'intensity': 'glow', 'form': 'streaks', 'scale': 'size', 'erode': 'dissolve', 'detail': 'grain'},
           seed=1313),

    preset('13 - Negative.json', 'negative', 'Negative',
           'A lattice pylon from the ground, split into two layers: the negative body in red, lit rims and wires in cyan. '
           'Accents break it into per-channel 1-bit noise and (with React up) jump-cut the camera; the drop turns it positive for a beat. '
           'Colours follow the palette as two layers (body = mid, detail = light); Scene Colors = red/cyan.',
           [stage('cam', 'pylon.fs', [
               param('orbit', 0.0, 0.3, 0.12, integrate=True), param('fov', 0.6, 1.8, 1.1), param('levels', 5, 12, 8),
               param('beam', 0.03, 0.12, 0.06), param('cuts', 0, 1, 0.5), param('tilt', 0, 1, 0.5),
               param('clouds', 0, 1, 0.4), param('surge', 0, 1, 0)]),
            stage('neg', 'negative_split.fs', NEGATIVE_PARAMS, kind='effect')],
           [route('macro.intensity', 'neg.threshold', -0.5, 0.5),
            route('macro.form', 'neg.outline', 0.8, 0.5), route('macro.form', 'neg.detail', 0.3, 0.5),
            route('macro.scale', 'cam.fov', -0.8, 0.5),
            route('macro.erode', 'neg.dither', 0.6, 0.5), route('macro.erode', 'neg.cell', 0.5, 0.5),
            route('macro.detail', 'cam.levels', 0.9, 0.5), route('macro.detail', 'cam.beam', -0.4, 0.5),
            route('macro.react', 'cam.cuts', 1.0, 0.5),
            rest('neg.threshold', -0.35), hit('neg.storm', 0.9), body('cam.surge', 0.5), build('neg.dither', 0.5),
            drop('neg.polarity', -1.0), air('neg.outline', 0.3), snare('neg.flash', 1.0),
            route('lfo.phrase', 'cam.tilt', 0.1, 0.5)],
           {'intensity': 'red body', 'form': 'cyan', 'scale': 'lens', 'erode': 'noise', 'detail': 'lattice',
            'react': 'hits + cuts'},
           post={'palette': 'duo', 'bloom': {'enabled': True, 'amount': 0.12, 'threshold': 0.85, 'levels': 4},
                 'toneMap': 'none', 'exposureEv': 0.0, 'outputColorSpace': 'srgb', 'grain': 0.0, 'vignette': 0.2},
           seed=1414, reseed_on_accent=True),

    # --- The performer's own images (MediaBin: drop a PNG/JPEG on the engine or LOAD in the plug-in).
    preset('14 - Media Negative.json', 'media-negative', 'Media Negative',
           'Your loaded image through the Negative treatment: the negative body in red, fine bright detail in cyan, '
           'accents break it into per-channel 1-bit noise, the drop turns it positive. Shows the built-in forms until an image is loaded.',
           [stage('frame', 'media_frame.fs', MEDIA_FRAME_PARAMS, {'source': MEDIA}),
            # Photos are levelled around their average (media_frame), so the threshold
            # sits above it: only what is darker than the image's average turns red.
            stage('neg', 'negative_split.fs', MEDIA_NEGATIVE_PARAMS, kind='effect')],
           [route('macro.intensity', 'neg.threshold', -0.5, 0.5),
            route('macro.form', 'neg.outline', 0.8, 0.5), route('macro.form', 'neg.detail', 0.3, 0.5),
            route('macro.scale', 'frame.zoom', 0.6, 0.5),
            route('macro.erode', 'neg.dither', 0.6, 0.5), route('macro.erode', 'neg.cell', 0.5, 0.5),
            route('macro.detail', 'neg.contrast', 0.6, 0.5),
            rest('neg.threshold', -0.35), hit('neg.storm', 0.9), body('frame.surge', 0.5), build('neg.dither', 0.5),
            drop('neg.polarity', -1.0), air('neg.outline', 0.3), snare('neg.flash', 1.0)],
           {'intensity': 'red body', 'form': 'cyan', 'scale': 'zoom', 'erode': 'noise', 'detail': 'contrast'},
           post=DUO_POST, seed=1515),

    preset('15 - Media Lines.json', 'media-lines', 'Media Lines',
           'Your loaded image redrawn as fine light linework on black with a sparse stipple, drifting slowly; '
           'accents burn the lines, the drop floods the image in. Shows the built-in forms until an image is loaded.',
           [stage('frame', 'media_frame.fs', MEDIA_FRAME_PARAMS, {'source': MEDIA}),
            stage('lines', 'media_lines.fs', [
               param('radius', 0.8, 4.0, 1.6), param('threshold', 0.005, 0.2, 0.04), param('dots', 0, 1, 0.3),
               param('fill', 0, 0.5, 0.1), param('flash', 0, 1, 0), param('emission', 0.2, 4.0, 1.8)], kind='effect')],
           [route('macro.intensity', 'lines.emission', 0.6, 0.5), route('macro.intensity', 'lines.fill', 0.3, 0.5),
            route('macro.form', 'lines.dots', 0.8, 0.5), route('macro.scale', 'frame.zoom', 0.6, 0.5),
            route('macro.erode', 'lines.threshold', 0.5, 0.5), route('macro.detail', 'lines.radius', -0.6, 0.5),
            rest('lines.threshold', -0.3), hit('lines.flash'), body('frame.surge', 0.5), build('lines.radius', 0.3),
            drop('lines.fill', 0.6), air('lines.dots', 0.3)],
           {'intensity': 'glow', 'form': 'stipple', 'scale': 'zoom', 'erode': 'fading lines', 'detail': 'fine lines'},
           seed=1616),

    preset('16 - Membrane.json', 'membrane', 'Membrane',
           'After the un_source reel: a crumpled sheet of thousands of points; its folds pile the points into bright ridges, '
           'a band stays in focus while the rest melts into bokeh. Accents make the folds leap; in silence it lies flat. '
           'Pair with SHOTS and HUD for the cuts of the reel.',
           [stage('sheet', 'membrane.fs', [
               param('morph', 0.0, 0.6, 0.12, integrate=True), param('pan', 0.0, 0.5, 0.05, integrate=True),
               param('zoom', 0.5, 3.0, 1.2), param('depth', 0, 0.6, 0.25), param('grid', 60, 260, 150),
               param('fold', 0, 1, 0.6), param('sparse', 0, 1, 0.45), param('focus', 0, 1, 0.45), param('dof', 0, 1, 0.6),
               param('haze', 0, 1, 0.35), param('sparkle', 0, 1, 0), param('surge', 0, 2, 0),
               param('emission', 0.2, 4.0, 1.3)])],
           [route('macro.intensity', 'sheet.emission', 0.6, 0.5), route('macro.form', 'sheet.fold', 0.8, 0.5),
            route('macro.scale', 'sheet.zoom', -0.6, 0.5), route('macro.erode', 'sheet.sparse', 0.8, 0.5),
            route('macro.detail', 'sheet.grid', 0.8, 0.5),
            rest('sheet.depth', 0.7), hit('sheet.surge'), body('sheet.fold', 0.3, 0.4), build('sheet.dof', 0.4),
            drop('sheet.haze', 0.5), air('sheet.sparkle', 0.5),
            route('lfo.phrase', 'sheet.focus', 0.15, 0.5)],
           {'intensity': 'glow', 'form': 'folds', 'scale': 'size', 'erode': 'thinning', 'detail': 'points'},
           post={'bloom': {'enabled': True, 'amount': 0.2, 'threshold': 0.85, 'levels': 5},
                 'toneMap': 'reinhard', 'exposureEv': 0.3, 'outputColorSpace': 'srgb', 'grain': 0.0, 'vignette': 0.3},
           seed=1717),

    # --- Ambient family (docs/research/AMBIENT-STYLE-DIRECTION.md): music with no kicks, for lying on
    # your back. Same rules as above, played differently: every audio route is slow (seconds of attack,
    # longer release), driven by sustained signals (energy, presence, centroid, body, tension); the
    # required HIT route is a slow swell, never a jolt; scenes arrive with a long crossfade on the bar.
    preset('17 - Lumia.json', 'lumia', 'Lumia',
           'After Thomas Wilfred\'s lumia and the aurora: veils of light hang in the dark and fold slowly, brightest where '
           'a fold is seen edge-on. The level unfurls them, build-ups bring in a third veil, an accent is a slow swell; '
           'in silence they settle toward the horizon. Ambient: try BREATHE, React To kick off, palette Cyanotype or Scene Colors.',
           [stage('sky', 'lumia.fs', [
               param('drift', 0.0, 0.25, 0.05, integrate=True), param('sway', 0.0, 0.2, 0.03, integrate=True),
               param('veils', 1, 3, 2.0), param('horizon', -0.8, 0.4, -0.3), param('height', 0.1, 1.4, 0.45),
               param('fold', 0, 1.5, 0.9), param('lift', 0, 1, 0.2), param('rays', 0, 1, 0.55),
               param('fineness', 6, 80, 30), param('diffuse', 0, 1, 0.2), param('shimmer', 0, 1, 0.2),
               param('swell', 0, 1.5, 0), param('haze', 0, 1, 0.25), param('tint', 0, 1, 0.35),
               param('emission', 0.2, 3.0, 1.0)])],
           [route('macro.intensity', 'sky.emission', 0.6, 0.5), route('macro.form', 'sky.fold', 0.8, 0.5),
            route('macro.scale', 'sky.height', 0.7, 0.5), route('macro.erode', 'sky.diffuse', 0.8, 0.5),
            route('macro.detail', 'sky.fineness', 0.8, 0.5),
            rest('sky.horizon', 0.4, attack=2500, release=6000),
            route('env.hit', 'sky.swell', 1.0, attack=700, release=5000),
            route('react.body', 'sky.lift', 0.6, 0.3, attack=1500, release=5000),
            route('react.tension', 'sky.veils', 0.6, attack=3000, release=10000),
            route('env.drop', 'sky.haze', 0.6, attack=2000, release=9000),
            route('react.air', 'sky.shimmer', 0.5, 0.3, attack=2000, release=5000),
            route('descriptor.centroid', 'sky.tint', 0.6, 0.45, attack=5000, release=10000),
            route('descriptor.presence', 'sky.rays', 0.35, 0.4, attack=3000, release=6000),
            route('lfo.phrase', 'sky.fold', 0.12, 0.5)],
           {'intensity': 'glow', 'form': 'folds', 'scale': 'height', 'erode': 'wisps', 'detail': 'rays'},
           post={'bloom': {'enabled': True, 'amount': 0.3, 'threshold': 0.75, 'levels': 6},
                 'toneMap': 'reinhard', 'exposureEv': 0.2, 'outputColorSpace': 'srgb', 'grain': 0.0, 'vignette': 0.35},
           transition={'type': 'crossfade', 'durationMs': 4000, 'quantize': 'bar', 'historyOnEnter': 'reset',
                       'retarget': 'snapshot-current'},
           seed=1818),

    preset('18 - Liquid Light.json', 'liquid-light', 'Liquid Light',
           'After the 1960s oil-and-water light shows: dense dye and clear oil in a clock glass on a dim projector, stirred '
           'by slow convection. Light leaks where the dye thins; oil lenses drift, merge and split with a dark meniscus and a '
           'caustic inside. The level thins the dye, an accent drops clear spirit that opens a window, build-ups heat the glass. '
           'Ambient: try BREATHE, React To kick off, palette Nitrate, Tungsten or Scene Colors.',
           [stage('glass', 'liquid_light.fs', [
               param('flow', 0.0, 0.3, 0.06, integrate=True), param('stir', 0, 1, 0.35), param('turn', -1, 1, 0.0),
               param('scale', 0.4, 2.5, 1.0), param('oil', 0, 1, 0.4), param('melt', 0, 1, 0.25),
               param('eddies', 0, 1, 0.4), param('density', 0.5, 8.0, 4.5), param('heat', 0, 1, 0),
               param('drip', 0, 1, 0), param('lens', 0, 1.5, 0.6), param('rim', 0, 2, 0.8),
               param('clarity', 0, 1, 0.25), param('tint', 0, 1, 0.4), param('lamp', 0.2, 3.0, 1.0)])],
           [route('macro.intensity', 'glass.lamp', 0.6, 0.5), route('macro.form', 'glass.oil', 0.8, 0.5),
            route('macro.scale', 'glass.scale', -0.6, 0.5), route('macro.erode', 'glass.melt', 0.8, 0.5),
            route('macro.detail', 'glass.eddies', 0.9, 0.5),
            rest('glass.density', -0.35, attack=2500, release=6000),
            route('env.hit', 'glass.drip', 1.0, attack=300, release=2500),
            route('react.body', 'glass.lens', 0.4, 0.3, attack=1500, release=5000),
            route('react.tension', 'glass.heat', 0.7, attack=3000, release=10000),
            route('env.drop', 'glass.clarity', 0.5, attack=2000, release=9000),
            route('react.air', 'glass.rim', 0.35, 0.3, attack=1500, release=5000),
            route('descriptor.centroid', 'glass.tint', 0.6, 0.45, attack=5000, release=10000),
            route('descriptor.presence', 'glass.lamp', 0.25, 0.4, attack=3000, release=6000),
            route('lfo.phrase', 'glass.turn', 0.5, 0.5)],
           {'intensity': 'lamp', 'form': 'oil', 'scale': 'blob size', 'erode': 'melt', 'detail': 'eddies'},
           post={'bloom': {'enabled': True, 'amount': 0.25, 'threshold': 0.8, 'levels': 5},
                 'toneMap': 'reinhard', 'exposureEv': 0.4, 'outputColorSpace': 'srgb', 'grain': 0.0, 'vignette': 0.3},
           transition={'type': 'crossfade', 'durationMs': 5000, 'quantize': 'bar', 'historyOnEnter': 'reset',
                       'retarget': 'snapshot-current'},
           seed=1919),
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
