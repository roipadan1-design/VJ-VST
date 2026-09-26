"""Old (09279ba) vs new engine on the same groove, several scenes; then the analysis table.

  python batch_compare.py <out_dir> <old_engine.exe> <old_analyze_wav.exe> <new_engine.exe> <new_analyze_wav.exe>

Each engine build must carry the VJ_FRAMEDUMP frame dump (MainComponent.cpp).
Writes <out_dir>/<tag>-<scene>.bin and prints one JSON line per run (analyze_capture.py).
"""
import json, os, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
SCENES = ['Hot Blobs', 'Dot Relief', 'Corridor', 'Fibers', 'Mesh Body', 'Halo Ring', 'Emergence']


def main():
    out, old_exe, old_an, new_exe, new_an = sys.argv[1:6]
    wav = os.path.join(out, 'groove90.wav')
    truth = os.path.join(out, 'groove90_truth.json')
    runs = []
    for scene in SCENES:
        for tag, exe, an, ctl in (('old', old_exe, old_an, 'controls_old_defaults.json'),
                                  ('new', new_exe, new_an, 'controls_v3.json')):
            path = os.path.join(out, '%s-%s.bin' % (tag, scene.replace(' ', '_')))
            subprocess.run([sys.executable, os.path.join(HERE, 'run_capture.py'), exe, an, wav, scene, path,
                            os.path.join(HERE, ctl)], check=False)
            runs.append(path)
    subprocess.run([sys.executable, os.path.join(HERE, 'analyze_capture.py'), truth, wav] + runs, check=False)


if __name__ == '__main__':
    main()
