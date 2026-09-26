"""Contact sheet (12 frames spread over the run) for each capture: python sheets.py run.bin [...]"""
import sys
import numpy as np
from PIL import Image
from analyze_capture import HDR, REC, W, H

for path in sys.argv[1:]:
    raw = np.memmap(path, dtype=np.uint8, mode='r')
    n = len(raw) // REC
    tiles = [np.array(raw[i * REC + HDR.size:(i + 1) * REC]).reshape(H, W, 3)[::-1]
             for i in np.linspace(n * 0.1, n - 1, 12).astype(int)]
    grid = np.concatenate([np.concatenate(tiles[r * 4:(r + 1) * 4], axis=1) for r in range(3)], axis=0)
    Image.fromarray(grid).save(path.rsplit('.', 1)[0] + '_sheet.png')
