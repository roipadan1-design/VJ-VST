import sys, os
from PIL import Image, ImageDraw, ImageFont
def sheet(paths, out, cols=4, tw=400, labels=None):
    th = int(tw*9/16); rows=(len(paths)+cols-1)//cols
    S = Image.new('RGB', (cols*tw+(cols+1)*6, rows*(th+22)+6), (24,24,24))
    d = ImageDraw.Draw(S)
    try: font = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf', 13)
    except Exception: font = ImageFont.load_default()
    for i,p in enumerate(paths):
        im = Image.open(p).convert('RGB').resize((tw,th), Image.LANCZOS)
        x = 6+(i%cols)*(tw+6); y = 6+(i//cols)*(th+22)
        S.paste(im,(x,y)); d.text((x+2,y+th+3), labels[i] if labels else os.path.basename(p)[:-4], fill=(220,220,220), font=font)
    S.save(out, quality=88)
if __name__=='__main__':
    d='out/audit'
    for pal in ['nitrate','cyanotype','ash']:
        ps=sorted(os.path.join(d,f) for f in os.listdir(d) if f.endswith(pal+'.png'))
        sheet(ps, 'out/sheet_'+pal+'.jpg')
