"""Synthetic downtempo groove for the reactivity forensics (90 BPM, 48 kHz, stereo 16-bit).

Sections (bars of 4 beats, 2.667 s each):
  0-3   A  full groove, loud (analyser warm-up; also measured)
  4-7   B  breakdown: pad + held bass + soft hats, no kick/snare, -8 dB
  8-9   C  build: snare roll 8ths -> 16ths, noise riser, -8 dB -> 0 dB
  10-17 D  drop: full groove, loud
  18-21 E  the same groove at -12 dB (a pure gain change: "quiet")
  22-25 F  full groove, loud again

Groove (16th grid): kick on steps 0, 7, 10; snare 4, 12; hats on 8ths with accents;
bass notes under the kicks; pad chords Am Am F G.

Writes groove90.wav and groove90_truth.json (kick/snare times, section spans, gain curve).
Usage: python make_groove.py [out_dir]
"""
import json, os, sys, wave
import numpy as np

SR = 48000
BPM = 90.0
BEAT = 60.0 / BPM
STEP = BEAT / 4
BAR = BEAT * 4
BARS = 26
rng = np.random.default_rng(7)

N = int(BARS * BAR * SR) + SR
mix = np.zeros((2, N))


def env_exp(n, tau_s):
    return np.exp(-np.arange(n) / (tau_s * SR))


def add(sig, t, gain=1.0, pan=0.0):
    i = int(round(t * SR))
    n = min(len(sig), N - i)
    if n <= 0:
        return
    l = gain * np.sqrt(0.5 * (1 - pan)); r = gain * np.sqrt(0.5 * (1 + pan))
    mix[0, i:i + n] += l * sig[:n]
    mix[1, i:i + n] += r * sig[:n]


def onepole_lp(x, fc):
    a = np.exp(-2 * np.pi * fc / SR)
    y = np.empty_like(x); s = 0.0
    for k in range(len(x)):
        s = (1 - a) * x[k] + a * s; y[k] = s
    return y


def kick():
    n = int(0.45 * SR); t = np.arange(n) / SR
    f = 45 + 75 * np.exp(-t / 0.035)
    ph = 2 * np.pi * np.cumsum(f) / SR
    body = np.sin(ph) * env_exp(n, 0.16)
    click = rng.standard_normal(n) * env_exp(n, 0.002) * 0.4
    return np.tanh(1.6 * (body + click)) * 0.95


def snare():
    n = int(0.3 * SR); t = np.arange(n) / SR
    noise = rng.standard_normal(n)
    noise = noise - onepole_lp(noise, 1200)          # high-passed noise
    tone = np.sin(2 * np.pi * 185 * t) * env_exp(n, 0.05)
    return (0.55 * noise * env_exp(n, 0.07) + 0.5 * tone) * 0.8


def hat(open_=False):
    n = int((0.25 if open_ else 0.06) * SR)
    x = rng.standard_normal(n)
    x = x - onepole_lp(x, 7000)
    return x * env_exp(n, 0.09 if open_ else 0.018) * 0.5


def bass_note(freq, dur):
    n = int(dur * SR); t = np.arange(n) / SR
    saw = 2 * ((freq * t) % 1.0) - 1
    x = 0.6 * np.sin(2 * np.pi * freq * t) + 0.25 * saw
    x = onepole_lp(x, 300)
    a = np.minimum(1, t / 0.005) * np.exp(-t / (dur * 0.9 + 0.05))
    return x * a


def pad_chord(freqs, dur):
    n = int(dur * SR); t = np.arange(n) / SR
    x = np.zeros(n)
    for f in freqs:
        for det in (-0.12, 0.0, 0.11):
            ff = f * 2 ** (det / 12)
            x += 2 * ((ff * t + rng.random()) % 1.0) - 1
    x = onepole_lp(x / (3 * len(freqs)), 1400)
    a = np.minimum(1, t / 0.6) * np.minimum(1, (dur - t) / 0.4)
    return x * np.clip(a, 0, 1)


K, S = kick(), snare()
H, HO = hat(), hat(True)
A1, F1, G1 = 55.0, 43.65, 49.0
CHORDS = [[220, 261.6, 329.6], [220, 261.6, 329.6], [174.6, 220, 261.6], [196, 246.9, 293.7]]
ROOTS = [A1, A1, F1, G1]

def section(bar):
    if bar < 4: return 'A'
    if bar < 8: return 'B'
    if bar < 10: return 'C'
    if bar < 18: return 'D'
    if bar < 22: return 'E'
    return 'F'

GAIN_DB = {'A': 0, 'B': -8, 'D': 0, 'E': -12, 'F': 0}

kicks, snares = [], []
gain_curve = []  # (time, dB) at every 16th
for bar in range(BARS):
    sec = section(bar)
    t0 = bar * BAR
    ci = bar % 4
    for step in range(16):
        t = t0 + step * STEP
        if sec == 'C':
            prog = ((bar - 8) * 16 + step) / 32.0
            g_db = -8 + 8 * prog
        else:
            g_db = GAIN_DB[sec]
        g = 10 ** (g_db / 20)
        gain_curve.append((round(t, 4), g_db))
        swing = 0.012 if step % 2 else 0.0
        if sec in 'ADEF':
            if step in (0, 7, 10):
                add(K, t, 0.9 * g); kicks.append(round(t, 4))
            if step in (4, 12):
                add(S, t, 0.7 * g, 0.05); snares.append(round(t, 4))
            if step % 2 == 0:
                acc = 1.0 if step % 4 == 2 else 0.6
                add(HO if step == 14 else H, t + swing, 0.22 * acc * g, 0.3)
            if step in (0, 7, 10):
                dur = {0: 7, 7: 3, 10: 6}[step] * STEP * 0.95
                add(bass_note(ROOTS[ci] * (2 if step == 7 else 1), dur), t, 0.55 * g)
        elif sec == 'B':
            if step % 4 == 2:
                add(H, t, 0.12 * g, 0.3)
            if step == 0:
                add(bass_note(ROOTS[ci], BAR * 0.98), t, 0.45 * g)
        elif sec == 'C':
            roll = 2 if bar == 8 else 1
            if step % roll == 0:
                add(S, t, (0.35 + 0.35 * prog) * g, 0.05); snares.append(round(t, 4))
            if step % 2 == 0:
                add(H, t, 0.18 * g, 0.3)
        if step == 0:
            pg = {'A': 0.5, 'B': 0.8, 'C': 0.8, 'D': 0.5, 'E': 0.5, 'F': 0.5}[sec]
            add(pad_chord(CHORDS[ci], BAR), t0, pg * (10 ** (GAIN_DB.get(sec, -4) / 20)), 0.0)

# Build riser (bars 8-9): band-rising noise swell.
t0 = 8 * BAR; n = int(2 * BAR * SR)
x = rng.standard_normal(n); tt = np.arange(n) / n
lp = np.empty(n); s = 0.0
for k in range(n):
    fc = 400 + 7000 * tt[k] ** 2
    a = np.exp(-2 * np.pi * fc / SR); s = (1 - a) * x[k] + a * s; lp[k] = s
add(lp * tt ** 1.5 * 0.5, t0, 1.0)

peak = np.max(np.abs(mix))
mix *= 0.89 / peak
out_dir = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.abspath(__file__))
pcm = (np.clip(mix.T, -1, 1) * 32767).astype('<i2')
with wave.open(os.path.join(out_dir, 'groove90.wav'), 'wb') as w:
    w.setnchannels(2); w.setsampwidth(2); w.setframerate(SR); w.writeframes(pcm.tobytes())

spans = {}
for bar in range(BARS):
    spans.setdefault(section(bar), [bar * BAR, (bar + 1) * BAR])[1] = (bar + 1) * BAR
truth = {'bpm': BPM, 'bar_seconds': BAR, 'kicks': kicks, 'snares': snares, 'sections': spans,
         'gain_db_16ths': gain_curve, 'duration': N / SR}
with open(os.path.join(out_dir, 'groove90_truth.json'), 'w') as f:
    json.dump(truth, f, indent=1)
print('wrote groove90.wav: %.1f s, %d kicks, %d snares, sections %s' % (N / SR, len(kicks), len(snares),
      {k: [round(a, 2), round(b, 2)] for k, (a, b) in spans.items()}))
