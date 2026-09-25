import numpy as np
from scipy import ndimage as ndi
from mock_util import *
from PIL import ImageDraw, ImageFont
import mock_memory as mm   # re-runs the d1 set (cheap) and gives us the helpers
r = mm.r
fi, sc = scene_linear(r, '05 - Fibers.json', frames=90, macros={'detail':0.25,'intensity':0.4,'scale':0.7})
Lf = mm.norm(lum(fi))
img = finish(r, mm.grey(mm.boundary_extension(mm.recall(Lf, 1), s=0.7), 1.1), 'Nitrate')
img.save('out/mock/d1_6b_boundary_fibers.png'); print('boundary fibers', band_stats(img))
# stage luminance zones drawn over the hollow ring
base = Image.open('out/mock/d2_2_ring_hollow.png').convert('RGB')
W, H = base.size
ov = Image.new('RGBA', base.size, (0,0,0,0)); d = ImageDraw.Draw(ov)
f = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', 22)
fs = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf', 17)
top_body, bot_body = int(H*(1-0.65)), int(H*(1-0.25))
d.rectangle([0, top_body, W, bot_body], fill=(200, 40, 30, 70))
d.rectangle([0, 0, W, int(H*0.25)], fill=(60, 140, 220, 45))
d.rectangle([0, 0, int(W*0.15), H], fill=(60, 220, 120, 35)); d.rectangle([int(W*0.85), 0, W, H], fill=(60, 220, 120, 35))
d.text((W*0.5, top_body+14), 'BODY ZONE  (25-65% of height)  mean <= 5% while dancing', font=f, fill=(255,230,220,255), anchor='mt')
d.text((W*0.5, 14), 'LIGHT ZONE  top 25%  (above heads)', font=f, fill=(220,235,255,255), anchor='mt')
d.text((W*0.075, H*0.5), 'EDGE\n15%', font=f, fill=(210,255,225,255), anchor='mm', align='center')
d.text((W*0.925, H*0.5), 'EDGE\n15%', font=f, fill=(210,255,225,255), anchor='mm', align='center')
d.text((W*0.5, H-18), 'calibrate the band to real sightlines in the venue (front row / centre / extreme side)', font=fs, fill=(230,230,230,230), anchor='mb')
Image.alpha_composite(base.convert('RGBA'), ov).convert('RGB').save('out/mock/stage_body_zone.png')
print('ok')
