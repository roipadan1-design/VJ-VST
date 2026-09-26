"""Measures how strongly a captured engine run reacts to the test groove.

  python analyze_capture.py <truth.json> <groove.wav> <run.bin> [<run.bin> ...]  -> JSON lines on stdout

Frame dump format (patch_dump.py): per frame a 120-byte header
  '<Iiiid8f8i8f' = magic, w, h, nEvents, now, levelRel, bassRel, midRel, highRel, levelAbs,
                   build, presence, pad, evType[8], evStrength[8]
followed by 160x90 RGB (rows bottom-up).

Metrics (luminance Y in 0-1; the picture is resampled onto a 60 Hz display grid - the frame on
screen at each tick - so runs at different frame rates compare fairly):
  motion D(t)   mean |Y(t) - Y(t - 33 ms)| over the frame (60 Hz grid, 2-tick lag)
  a  hit contrast  mean D in the 150 ms after each kick / median D between kicks
                   (between = >350 ms after the last kick and >100 ms before the next);
                   loud groove sections A (bars 1-3), D, F
  a2 hit excess    mean D after kicks minus the between-kick median (absolute, x1000)
  b  correlation   Pearson r of the audio RMS envelope (50 ms, dB) with luminance L and with
                   motion M (D smoothed 150 ms); also on a slow (1 s) envelope
  c  quiet/loud    section E (same groove at -12 dB) vs D+F; breakdown B vs drop D:
                   ratios of mean motion and of mean luminance, hit contrast inside E
  d  hit size      per kick: largest mean |Y - Y(pre-kick frame)| within 150 ms (x1000),
                   the share of the frame whose luminance moves > 0.08, and that change
                   relative to the pre-kick frame's mean brightness
  e  hit step      what the kick ADDS: mean |Y(k+lag) - Y(k-1 tick)| after kicks minus the
                   same measured from control instants between kicks (lag 50 / 150 ms; x1000),
                   and the extra share of the frame that moves > 0.08 at 50 ms (percentage points)
Optional 4th+ arg form:  run.bin@ADF  restricts the loud sections used by a/d/e (e.g. @A).
"""
import json, struct, sys, wave
import numpy as np

HDR = struct.Struct('<Iiiid8f8i8f')
W, H = 160, 90
REC = HDR.size + W * H * 3
GRID = 1.0 / 60.0


def load_dump(path):
    raw = np.fromfile(path, dtype=np.uint8)
    n = len(raw) // REC
    raw = raw[:n * REC].reshape(n, REC)
    t = np.empty(n); lev = np.empty(n); bass = np.empty(n)
    events = []
    for i in range(n):
        h = HDR.unpack(raw[i, :HDR.size].tobytes())
        assert h[0] == 0x44464a56
        t[i] = h[4]; lev[i] = h[5]; bass[i] = h[6]
        ne = h[3]
        for k in range(min(ne, 8)):
            events.append((h[4], h[13 + k], h[21 + k]))
    px = raw[:, HDR.size:].reshape(n, H, W, 3).astype(np.float32) / 255.0
    Y = (0.2126 * px[..., 0] + 0.7152 * px[..., 1] + 0.0722 * px[..., 2])
    return t, lev, bass, events, Y


def wav_env(path, win):
    with wave.open(path) as w:
        sr = w.getframerate(); ch = w.getnchannels()
        x = np.frombuffer(w.readframes(w.getnframes()), dtype='<i2').astype(np.float64) / 32768.0
    x = x.reshape(-1, ch).mean(axis=1)
    hop = int(sr * 0.01)
    n = len(x) // hop
    p = (x[:n * hop] ** 2).reshape(n, hop).mean(axis=1)
    k = max(1, int(win / 0.01))
    p = np.convolve(p, np.ones(k) / k, mode='same')
    return np.arange(n) * 0.01 + 0.005, 10 * np.log10(p + 1e-9)


def align(events, kicks):
    """Engine-clock offset of audio time 0 (includes detection + network latency)."""
    ek = np.array([e[0] for e in events if e[1] == 0])
    best = (0, 0.0)
    for e in ek[:12]:
        for k in kicks[:12]:
            o = e - k
            m = np.sum(np.min(np.abs(ek[:, None] - (kicks[None, :] + o)), axis=1) < 0.03)
            if m > best[0]:
                best = (m, o)
    o = best[1]
    d = ek[:, None] - (kicks[None, :] + o)
    j = np.argmin(np.abs(d), axis=1)
    dd = d[np.arange(len(ek)), j]
    ok = np.abs(dd) < 0.03
    return o + np.median(dd[ok]), int(ok.sum()), len(ek)


def pearson(a, b):
    a = a - a.mean(); b = b - b.mean()
    return float((a * b).sum() / np.sqrt((a * a).sum() * (b * b).sum() + 1e-12))


def analyse(truth, wav, path, loud_secs='ADF'):
    t, lev, bass, events, Y = load_dump(path)
    kicks = np.array(truth['kicks'])
    off, matched, nk = align(events, kicks)
    tau = t - off                                          # audio time of each frame
    fps = 1.0 / np.median(np.diff(t))

    # Display grid 60 Hz over the audio.
    g = np.arange(0.5, truth['sections']['F'][1] - 0.1, GRID)
    idx = np.searchsorted(tau, g, side='right') - 1
    valid = idx >= 0
    g = g[valid]; idx = idx[valid]
    Yg = Y[idx]
    L = Yg.mean(axis=(1, 2))
    # Motion over 2 ticks (33 ms): robust to engines rendering anywhere between 28 and 60 fps.
    D = np.zeros(len(g))
    D[2:] = np.abs(Yg[2:] - Yg[:-2]).mean(axis=(1, 2))
    sm = int(round(0.15 / GRID))
    M = np.convolve(D, np.ones(sm) / sm, mode='same')

    S = truth['sections']
    def in_sec(x, names, trim_first=0.0):
        m = np.zeros(len(x), bool)
        for s in names:
            a, b = S[s]
            m |= (x >= a + trim_first) & (x < b)
        return m

    loud = in_sec(g, loud_secs) & (g > truth['bar_seconds'])   # skip the analyser's first bar
    # time since last kick / until next kick
    ki = np.searchsorted(kicks, g, side='right') - 1
    since = np.where(ki >= 0, g - kicks[np.clip(ki, 0, None)], 9)
    nxt = np.where(ki + 1 < len(kicks), kicks[np.clip(ki + 1, 0, len(kicks) - 1)] - g, 9)
    between = loud & (since > 0.35) & (nxt > 0.10)
    base = float(np.median(D[between]))

    def kick_stats(names, first=0.0):
        post, peak, size, area, rel = [], [], [], [], []
        for k in kicks:
            if not any(S[s][0] + first <= k < S[s][1] for s in names) or k < truth['bar_seconds']:
                continue
            w = (g >= k) & (g < k + 0.15)
            if w.sum() < 3:
                continue
            post.append(D[w].mean()); peak.append(D[w].max())
            j0 = np.searchsorted(g, k) - 1
            pre = Yg[j0]
            ch = np.abs(Yg[w] - pre[None])
            m = ch.mean(axis=(1, 2))
            size.append(m.max())
            area.append((ch > 0.08).mean(axis=(1, 2)).max())
            rel.append(m.max() / (pre.mean() + 0.02))
        return np.array(post), np.array(peak), np.array(size), np.array(area), np.array(rel)

    post, peak, size, area, rel = kick_stats(loud_secs)

    # e: control-subtracted hit step
    def step_at(times, lag):
        out, ar = [], []
        for k in times:
            j0 = np.searchsorted(g, k) - 1
            j1 = np.searchsorted(g, k + lag)
            if j0 < 0 or j1 >= len(g):
                continue
            ch = np.abs(Yg[j1] - Yg[j0])
            out.append(ch.mean()); ar.append((ch > 0.08).mean())
        return np.mean(out), np.mean(ar)
    lk = [k for k in kicks if any(S[s][0] <= k < S[s][1] for s in loud_secs) and k >= truth['bar_seconds']]
    ctrl = g[between & (nxt > 0.2)][::7]
    e = {}
    for lag in (0.05, 0.15):
        mk, ak = step_at(lk, lag); mc, ac = step_at(ctrl, lag)
        e['e_hit_step_%dms_x1000' % int(lag * 1000)] = round(float((mk - mc) * 1000), 1)
        if lag == 0.05:
            e['e_hit_area_extra_pct'] = round(float((ak - ac) * 100), 1)
            e['e_ctrl_step_50ms_x1000'] = round(float(mc * 1000), 1)
    res = {'run': path.replace('\\', '/').split('/')[-1], 'fps': round(fps, 1),
           'kick_events_matched': '%d/%d' % (matched, nk), 'offset_ms': None,
           'base_D_x1000': round(base * 1000, 2),
           'a_hit_contrast': round(float(post.mean() / base), 2),
           'a_peak_contrast': round(float(peak.mean() / base), 2),
           'a2_hit_excess_x1000': round(float((post.mean() - base) * 1000), 2),
           'd_hit_size_x1000': round(float(size.mean() * 1000), 1),
           'd_hit_area_pct': round(float(area.mean() * 100), 1),
           'd_hit_rel_pct': round(float(rel.mean() * 100), 1),
           'mean_L': round(float(L[loud].mean()), 3)}
    res.update(e)

    # b: correlations with the audio envelope
    for win, tag in ((0.05, 'fast'), (1.0, 'slow')):
        et, edb = wav_env(wav, win)
        env = np.interp(g, et, edb)
        use = g > truth['bar_seconds']
        res['b_r_L_%s' % tag] = round(pearson(L[use], env[use]), 2)
        Ms = M if tag == 'fast' else np.convolve(D, np.ones(60) / 60, mode='same')
        res['b_r_M_%s' % tag] = round(pearson(Ms[use], env[use]), 2)

    # c: quiet vs loud
    E = in_sec(g, 'E', 1.0); DF = in_sec(g, 'DF', 1.0); B = in_sec(g, 'B', 1.0); Dd = in_sec(g, 'D', 1.0)
    res['c_motion_loud_over_quiet'] = round(float(D[DF].mean() / max(D[E].mean(), 1e-6)), 2)
    res['c_L_loud_over_quiet'] = round(float(L[DF].mean() / max(L[E].mean(), 1e-6)), 2)
    res['c_motion_drop_over_breakdown'] = round(float(D[Dd].mean() / max(D[B].mean(), 1e-6)), 2)
    res['c_L_drop_over_breakdown'] = round(float(L[Dd].mean() / max(L[B].mean(), 1e-6)), 2)
    pq, _, sq, _, _ = kick_stats('E', 1.0)
    betweenE = in_sec(g, 'E', 1.0) & (since > 0.35) & (nxt > 0.10)
    res['c_hit_contrast_quiet'] = round(float(pq.mean() / np.median(D[betweenE])), 2) if len(pq) else None
    res['c_hit_size_quiet_x1000'] = round(float(sq.mean() * 1000), 1) if len(sq) else None
    res['offset_ms'] = round(off * 1000 % 1000, 1)

    # kick-triggered average of luminance change (for the report's curves)
    lags = np.arange(-2, 31)
    curves = []
    for k in kicks:
        if not any(S[s][0] <= k < S[s][1] for s in 'DF'):
            continue
        j = np.searchsorted(g, k)
        if j - 3 < 0 or j + 31 >= len(g):
            continue
        curves.append(np.abs(Yg[j + lags] - Yg[j - 1][None]).mean(axis=(1, 2)))
    res['kta_x1000'] = [round(float(v) * 1000, 1) for v in np.mean(curves, axis=0)]
    return res


if __name__ == '__main__':
    truth = json.load(open(sys.argv[1]))
    for p in sys.argv[3:]:
        path, _, secs = p.partition('@')
        r = analyse(truth, sys.argv[2], path, secs or 'ADF')
        if secs:
            r['run'] += '@' + secs
        print(json.dumps(r))
        sys.stdout.flush()
