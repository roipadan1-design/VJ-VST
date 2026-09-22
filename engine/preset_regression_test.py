"""Automated preset regression test for VJ Engine.

Launches a fresh engine instance on a dedicated OSC port, cycles through
every preset in Presets/, and for each one:
  - sends /preset/select <index>
  - waits for the crossfade + shader compile to settle
  - sends /debug/snapshot (writes VJEngine_snapshot.png next to the exe)
  - reads the PNG back and flags likely-broken renders (solid black/blank,
    or near-zero color variance - the signature of a failed shader compile
    or an empty framebuffer)
  - greps the fresh tail of VJEngine.log for load/compile errors logged
    during that preset's window

This turns the manual "/debug/snapshot + eyeball the PNG" workflow used
earlier this session into an unattended check that can be rerun after any
shader/preset/engine change, without needing screen access.

Usage: python preset_regression_test.py [--port 9001] [--exe path/to/VJ Engine.exe]
"""
import argparse
import glob
import os
import socket
import struct
import subprocess
import sys
import time

from PIL import Image, ImageStat

ENGINE_DIR = os.path.dirname(os.path.abspath(__file__))


def osc_msg(addr, *args):
    def pad(b):
        while len(b) % 4:
            b += b'\0'
        return b

    def padstr(s):
        return pad(s.encode() + b'\0')

    msg = padstr(addr)
    types = ','
    argbytes = b''
    for a in args:
        if isinstance(a, str):
            types += 's'
            argbytes += padstr(a)
        else:
            types += 'f'
            argbytes += struct.pack('>f', float(a))
    return msg + padstr(types) + argbytes


def find_default_exe():
    for cfg in ('Release', 'Debug'):
        p = os.path.join(ENGINE_DIR, 'build', 'VJEngine_artefacts', cfg, 'VJ Engine.exe')
        if os.path.isfile(p):
            return p
    return None


def list_presets():
    preset_dir = os.path.join(ENGINE_DIR, 'Presets')
    files = sorted(glob.glob(os.path.join(preset_dir, '*.json')))
    return [os.path.splitext(os.path.basename(f))[0] for f in files]


def wait_for_stable_file(path, timeout=5.0, poll=0.1):
    """Wait until the file exists and its size stops changing (write finished)."""
    deadline = time.time() + timeout
    last_size = -1
    stable_checks = 0
    while time.time() < deadline:
        if os.path.isfile(path):
            size = os.path.getsize(path)
            if size > 0 and size == last_size:
                stable_checks += 1
                if stable_checks >= 2:
                    return True
            else:
                stable_checks = 0
            last_size = size
        time.sleep(poll)
    return os.path.isfile(path)


def analyze_snapshot(path):
    img = Image.open(path).convert('RGB')
    stat = ImageStat.Stat(img)
    r_mean, g_mean, b_mean = stat.mean
    variance = sum(stat.var)  # sum of per-channel variance as a single "flatness" score

    is_flat = variance < 4.0  # near-uniform image: blank/black/single-color frame
    is_black = r_mean < 2 and g_mean < 2 and b_mean < 2
    return {
        'mean_rgb': (round(r_mean, 1), round(g_mean, 1), round(b_mean, 1)),
        'variance': round(variance, 2),
        'suspect': is_flat or is_black,
    }


def tail_new_lines(log_path, offset):
    if not os.path.isfile(log_path):
        return [], offset
    with open(log_path, 'r', encoding='utf-8', errors='replace') as f:
        f.seek(offset)
        lines = f.readlines()
        new_offset = f.tell()
    return lines, new_offset


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=9001)
    ap.add_argument('--exe', default=None)
    ap.add_argument('--settle', type=float, default=2.0, help='seconds to wait after preset/select before snapshot')
    ap.add_argument('--video', default=None, help='video file to preload, for presets whose effectChain expects one')
    args = ap.parse_args()

    exe = args.exe or find_default_exe()
    if not exe:
        print('Could not find VJ Engine.exe under build/VJEngine_artefacts/{Release,Debug}/')
        sys.exit(1)

    presets = list_presets()
    if not presets:
        print('No presets found in Presets/')
        sys.exit(1)

    snapshot_path = os.path.join(ENGINE_DIR, 'build', 'VJEngine_artefacts', 'Debug', 'VJEngine_snapshot.png')
    log_path = os.path.join(ENGINE_DIR, 'build', 'VJEngine_artefacts', 'Debug', 'VJEngine.log')
    # snapshot/log land next to whichever exe config actually ran
    snapshot_path = os.path.join(os.path.dirname(exe), 'VJEngine_snapshot.png')
    log_path = os.path.join(os.path.dirname(exe), 'VJEngine.log')

    launch_args = [exe, '--osc-port', str(args.port)]
    if args.video:
        launch_args.append(args.video)
    print('Launching:', ' '.join(f'"{a}"' if ' ' in a else a for a in launch_args))
    proc = subprocess.Popen(launch_args, cwd=os.path.dirname(exe))
    time.sleep(3.0)  # window creation + GL context + initial shader compile

    log_offset = os.path.getsize(log_path) if os.path.isfile(log_path) else 0

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    dest = ('127.0.0.1', args.port)

    def send(addr, *a):
        sock.sendto(osc_msg(addr, *a), dest)

    results = []
    try:
        for idx, name in enumerate(presets):
            if os.path.isfile(snapshot_path):
                os.remove(snapshot_path)

            send('/preset/select', idx)
            time.sleep(args.settle)
            send('/debug/snapshot')
            wait_for_stable_file(snapshot_path)

            errors = []
            log_lines, log_offset = tail_new_lines(log_path, log_offset)
            for line in log_lines:
                low = line.lower()
                if 'could not load' in low or 'could not compile' in low or 'error' in low or 'failed' in low:
                    errors.append(line.strip())

            if not os.path.isfile(snapshot_path):
                results.append({'preset': name, 'ok': False, 'reason': 'no snapshot written', 'errors': errors})
                continue

            stats = analyze_snapshot(snapshot_path)
            ok = not stats['suspect'] and not errors
            results.append({'preset': name, 'ok': ok, 'stats': stats, 'errors': errors})
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()

    print()
    print(f'{"PRESET":<45} {"RESULT":<8} DETAIL')
    print('-' * 100)
    any_fail = False
    for r in results:
        status = 'PASS' if r['ok'] else 'FAIL'
        if not r['ok']:
            any_fail = True
        detail = ''
        if 'stats' in r:
            detail = f"mean_rgb={r['stats']['mean_rgb']} variance={r['stats']['variance']}"
        elif 'reason' in r:
            detail = r['reason']
        print(f'{r["preset"]:<45} {status:<8} {detail}')
        for e in r.get('errors', []):
            print(f'    log: {e}')

    print()
    print('ALL PASS' if not any_fail else 'FAILURES DETECTED')
    sys.exit(1 if any_fail else 0)


if __name__ == '__main__':
    main()
