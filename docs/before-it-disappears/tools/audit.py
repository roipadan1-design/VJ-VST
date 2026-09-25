import json, os, time
from vjrender import *
OUT='out/audit'; os.makedirs(OUT, exist_ok=True)
r = Renderer(1280, 720)
files = sorted(f for f in os.listdir(PRESETS) if f.endswith('.json'))
LOOK = {'grain':0.3,'crush':0.35,'halation':0.35,'weave':0.3,'dust':0.3,'blacks':0.35,'trails':0.1}
stats = {}
for f in files:
    for pal in ['Nitrate','Cyanotype','Ash']:
        look = Look(r)
        sc = Scene(r, f, image_index=2)
        n = 400 if 'Morphogen' in f else 90
        img=None
        for i in range(n):
            out = sc.step()
            if i >= n-3:
                duo = 1.0 if sc.preset['post'].get('palette') == 'duo' else 0.0
                img = look.render(out.read_tex, sc.preset['post'], LOOK, pal, duo=duo, t=sc.time)
        name = f[:2] + '-' + sc.preset['id'] + '-' + pal.lower()
        img.save(os.path.join(OUT, name + '.png'))
        stats[name] = luminance_stats(img)
        print(name, {k: round(v,3) for k,v in stats[name].items()}, flush=True)
json.dump(stats, open(os.path.join(OUT,'stats.json'),'w'), indent=1)
