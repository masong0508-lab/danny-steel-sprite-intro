"""Software OBJ renderer for the host OAM dump: python3 tools/render_intro.py oam.bin scenes.pkl out.png  (sheet of key frames + worst-scanline cost)"""
import sys, pickle, numpy as np
from PIL import Image
SZ = [[(8,8),(16,16),(32,32),(64,64)], [(16,8),(32,8),(32,16),(64,32)], [(8,16),(8,32),(16,32),(32,64)]]
raw = np.fromfile(sys.argv[1], np.uint16).reshape(-1, 512); sc = pickle.load(open(sys.argv[2], 'rb'))
def tiles(words): return np.array(words, np.uint32)
def render(oam, words, pal, bd=(0, 0, 0)):
    img = np.zeros((160, 240, 3), np.uint8); img[:] = bd; cost = np.zeros(160, int); T = tiles(words)
    def pix(tile, bw, bh, x, y):
        tw = bw // 8; t = tile + (y // 8) * tw + x // 8; w = int(T[t * 8 + (y % 8)]) if t * 8 + 7 < len(T) else 0
        return (w >> (4 * (x % 8))) & 15
    done = np.zeros((160, 240), bool)
    for e in range(8, 128):
        a0, a1, a2 = (int(v) for v in oam[e * 4:e * 4 + 3])
        if a0 & 0x300 == 0x200: continue
        aff, dbl = bool(a0 & 0x100), bool(a0 & 0x200); shape, size = a0 >> 14, a1 >> 14; bw, bh = SZ[shape][size]
        y, x = a0 & 255, a1 & 511; x = x - 512 if x >= 240 else x; y = y - 256 if y >= 160 else y
        Bw, Bh = (bw * 2, bh * 2) if aff and dbl else (bw, bh); tile, bank = a2 & 1023, a2 >> 12
        if aff:
            g = (a1 >> 9) & 31; m = [int(np.int16(oam[16 * g + k])) / 256 for k in (3, 7, 11, 15)]
        for yy in range(Bh):
            sy = y + yy
            if not 0 <= sy < 160: continue
            cost[sy] += (10 + 2 * Bw) if aff else Bw
            for xx in range(Bw):
                sx = x + xx
                if not 0 <= sx < 240 or done[sy, sx]: continue
                if aff:
                    ox, oy = xx - Bw / 2, yy - Bh / 2; u = int(m[0] * ox + m[1] * oy + bw / 2); v = int(m[2] * ox + m[3] * oy + bh / 2)
                    if not (0 <= u < bw and 0 <= v < bh): continue
                else: u, v = xx, yy
                p = pix(tile, bw, bh, u, v)
                if p:
                    c = pal[bank * 16 + p]; img[sy, sx] = ((c & 31) << 3, ((c >> 5) & 31) << 3, ((c >> 10) & 31) << 3); done[sy, sx] = True
    return img, cost.max()
frames = []; i = 0
for name, words, pal, layers, n in sc:
    fr = [0, n // 4, n // 2, 3 * n // 4, n - 1] if n > 2 else [0]
    for f in fr: frames.append((name, f, i + f))
    i += n if n > 2 else 1
sheet = Image.new('RGB', (240 * 5, 160 * 6)); worst = 0
for k, (name, f, gi) in enumerate(frames):
    si = [s[0] for s in sc].index(name); img, c = render(raw[gi], sc[si][1], [v for p in sc[si][2] for v in p], (224, 224, 224) if name == "city" else (0, 0, 0)); worst = max(worst, c)
    print(name, f, 'max scanline cost', c)
    sheet.paste(Image.fromarray(img), ((k % 5) * 240, (k // 5) * 160))
sheet.save(sys.argv[3]); print('worst', worst, '(limit 1210)')
