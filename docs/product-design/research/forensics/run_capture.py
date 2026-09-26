"""Runs one engine build on one scene, streams the test WAV into it and records every frame.

The engine builds used here are scratch builds of 09279ba (old) and HEAD (new) with one
identical, env-gated addition (patch_dump.py): VJ_FRAMEDUMP=<file> makes the engine append
each rendered frame (box-averaged to 160x90 RGB) plus the signals/events it used that frame.

  python run_capture.py <engine.exe> <analyze_wav.exe> <wav> <scene name> <out.bin> <controls.json>

controls.json: {"macros": [8], "look": [16], "move": [6] (optional), "palette": [9], "quality": 1.0}
- what the VJ Analyzer plug-in would send with its default parameters for that version.
"""
import json, os, socket, struct, subprocess, sys, time


def osc(addr, *args):
    def pad(b):
        return b + b'\0' * (4 - len(b) % 4)
    tags = ','
    data = b''
    for a in args:
        if isinstance(a, str):
            tags += 's'; data += pad(a.encode())
        elif isinstance(a, int):
            tags += 'i'; data += struct.pack('>i', a)
        else:
            tags += 'f'; data += struct.pack('>f', float(a))
    return pad(addr.encode()) + pad(tags.encode()) + data


def send_controls(sock, c, scene):
    target = ('127.0.0.1', 9000)
    s = lambda *m: sock.sendto(osc(*m), target)
    s('/v2/quality', float(c.get('quality', 1.0)))
    for i, v in enumerate(c['macros']):
        s('/v2/macro', i, float(v))
    for i, v in enumerate(c['look']):
        s('/v2/look', i, float(v))
    for i, v in enumerate(c.get('move', [])):
        s('/v2/move', i, float(v))
    s('/v2/react', 1.0, 1.0, 1.0, 1.0, 1.0)
    s('/v2/palette', *[float(x) for x in c['palette']], 1.0)
    s('/v2/preset', scene)


def frames_in(path):
    try:
        return os.path.getsize(path) // (120 + 160 * 90 * 3)
    except OSError:
        return 0


def main():
    engine, analyzer, wav, scene, out, controls = sys.argv[1:7]
    c = json.load(open(controls))
    if os.path.exists(out):
        os.remove(out)
    env = dict(os.environ, VJ_FRAMEDUMP=os.path.abspath(out))
    proc = subprocess.Popen([engine], cwd=os.path.dirname(engine), env=env)
    try:
        t0 = time.time()
        while frames_in(out) < 30:
            if time.time() - t0 > 30 or proc.poll() is not None:
                raise SystemExit('engine did not start rendering')
            time.sleep(0.2)
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        for _ in range(3):              # UDP: say it three times
            send_controls(sock, c, scene)
            time.sleep(0.3)
        time.sleep(2.0)
        n0 = frames_in(out)
        subprocess.run([analyzer, wav, '--send', '127.0.0.1:9000', '--bpm', '90'], check=True,
                       stdout=subprocess.DEVNULL)
        time.sleep(0.8)
        n1 = frames_in(out)
        print('%s: %d frames during the audio' % (os.path.basename(out), n1 - n0))
    finally:
        subprocess.run(['taskkill', '/PID', str(proc.pid)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        try:
            proc.wait(timeout=8)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait()


if __name__ == '__main__':
    main()
