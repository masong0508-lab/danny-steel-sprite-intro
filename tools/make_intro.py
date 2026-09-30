"""Builds src/intro_data.c : every intro scene as 4bpp OBJ tiles + block tables + keyframes.  Run: python3 tools/make_intro.py
Needs Pillow, numpy, opencv-python, ffmpeg (for video stills).  Source footage path: env INTRO_SRC (default /mnt/user-data/uploads/)."""
import sys, os, math, numpy as np, cv2
sys.path.insert(0, os.path.dirname(__file__))
from intro_assets import *
SHAPES = {(8,8):(0,0),(16,16):(0,1),(32,32):(0,2),(64,64):(0,3),(16,8):(1,0),(32,8):(1,1),(32,16):(1,2),(64,32):(1,3),(8,16):(2,0),(8,32):(2,1),(16,32):(2,2),(32,64):(2,3)}
G = 'Project_09-30_HD_720p_HIGH_FR24.mp4'

class Layer:
    def __init__(s, name, im, bw, bh, grp=-1, dbl=0, keys=()):
        s.name, s.bw, s.bh, s.grp, s.dbl, s.keys = name, bw, bh, grp, dbl, list(keys)
        W, H = -(-im.width // bw) * bw, -(-im.height // bh) * bh
        c = Image.new('RGBA', (W, H), (0, 0, 0, 0)); c.alpha_composite(im, (0, 0)); s.im, s.W, s.H = c, W, H
        a = np.array(c); s.blocks = []
        for by in range(0, H, bh):
            for bx in range(0, W, bw):
                b = a[by:by + bh, bx:bx + bw]
                if (b[..., 3] >= 128).any(): s.blocks.append(dict(cx=bx + bw // 2 - W // 2, cy=by + bh // 2 - H // 2, px=b))

def quant_group(layers, nbanks):
    """cluster blocks by mean colour into nbanks, quantise each cluster to 15 colours (+ index 0 transparent)"""
    blks = [b for l in layers for b in l.blocks]
    means = np.array([b['px'][..., :3][b['px'][..., 3] >= 128].mean(0) for b in blks], np.float32)
    if nbanks > 1 and len(blks) >= nbanks:
        _, lab, _ = cv2.kmeans(means, nbanks, None, (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, .1), 4, cv2.KMEANS_PP_CENTERS)
        lab = lab.ravel()
    else: lab = np.zeros(len(blks), int); nbanks = 1
    pals = []
    for k in range(nbanks):
        mem = [b for b, l in zip(blks, lab) if l == k]
        if not mem: pals.append([0] * 16); continue
        pix = np.concatenate([b['px'][..., :3][b['px'][..., 3] >= 128] for b in mem]).reshape(-1, 1, 3).astype(np.uint8)
        q = Image.fromarray(pix).quantize(15, Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
        pl = (q.getpalette() or [])[:45]; pl += [0] * (45 - len(pl)); pl = [0, 0, 0] + pl    # slot 0 = transparent
        pim = Image.new('P', (1, 1)); pim.putpalette(pl + [0] * (768 - len(pl)))
        for b in mem:
            rgb = Image.fromarray(np.ascontiguousarray(b['px'][..., :3])).quantize(palette=pim, dither=Image.Dither.FLOYDSTEINBERG)
            idx = np.array(rgb, np.uint8); idx = np.where(idx == 0, 1, idx)                   # never hit slot 0 for solid pixels
            b['idx'] = np.where(b['px'][..., 3] >= 128, idx, 0)
        pals.append([((pl[i*3] >> 3) | ((pl[i*3+1] >> 3) << 5) | ((pl[i*3+2] >> 3) << 10)) for i in range(16)])
    for b, k in zip(blks, lab): b['k'] = int(k)
    return pals

class Scene:
    def __init__(s, name, frames, fin=8, fout=8, bd=0): s.bd = bd; s.name, s.frames, s.fin, s.fout, s.layers, s.pal = name, frames, fin, fout, [], [[0]*16 for _ in range(16)]; s.nb = 0
    def group(s, layers, nbanks=1):
        pals = quant_group(layers, nbanks)
        for i, p in enumerate(pals): s.pal[s.nb + i] = p
        for l in layers:
            for b in l.blocks: b['bank'] = s.nb + b['k']
        s.nb += len(pals); s.layers += layers
    def pack(s):
        words, t = [], 0
        for l in s.layers:
            for b in l.blocks:
                b['tile'] = t; idx = b['idx']
                for ty in range(0, l.bh, 8):
                    for tx in range(0, l.bw, 8):
                        T = idx[ty:ty + 8, tx:tx + 8].astype(np.uint32)
                        for r in range(8): words.append(sum(int(T[r, c]) << (4 * c) for c in range(8)))
                t += l.bw * l.bh // 64
        assert t <= 1024, (s.name, 'tiles', t); s.ntiles = t; s.words = words
        n = sum(len(l.blocks) for l in s.layers) + 0; assert n <= 120, (s.name, 'sprites', n)

def K(t, x, y, sc=256, ang=0, ease=0): return (t, x, y, sc, ang, ease)
scenes = []
# ---------------------------------------------------------------- A: apple zooms out, hand slides in
A = Scene('apple', 150)
ap = Layer('apple', apple(P('20260930_104355.jpg'), (1000, 850, 2700, 2000), (192, 160)), 32, 32, grp=0,
           keys=[K(0, 120, 84, 256, 0, 1), K(110, 92, 88, 150, 4, 1), K(150, 88, 88, 140, 4)])
hf = Layer('hand_flat', cutout('cutout-20260930_105735_B872E6DF2.png', (128, 80)), 32, 32, keys=[K(70, 330, 118, 256, 0, 1), K(150, 176, 118)])
A.group([ap], 3); A.group([hf]); scenes.append(A)
# ---------------------------------------------------------------- B: fist opens, earth appears and swells
B = Scene('earth', 230)
fist = Layer('hand_fist', cutout('cutout-20260930_105750_3C8BE7F3A.png', (104, 80)), 32, 32, keys=[K(0, 330, 112, 256, 0, 1), K(24, 170, 112), K(50, 170, 112)])
opn = Layer('hand_open', cutout('cutout-20260930_105759_1C5578A9F.png', (128, 80)), 32, 32, keys=[K(50, 172, 116), K(120, 172, 116)])
B.group([fist, opn], 2)
ef = [Layer(f'earth{i}', earth(i * 6, (64, 64)), 64, 64, grp=1, dbl=1, keys=[K(56 + i * 16 + (0 if i else 0), 168, 62, 256), K(56 + i * 16 + 15, 168, 62, 256)]) for i in range(4)]
ef[0].keys = [K(54, 168, 66, 40, 0, 1), K(70, 168, 62, 256), K(71, 168, 62, 256)]
for i, l in enumerate(ef): l.keys = l.keys if i == 0 else [K(56 + i * 16, 168, 62, 256), K(71 + i * 16, 168, 62, 256)]
eb = Layer('earthbig', earth(3, (128, 128)), 64, 64, grp=1, dbl=1, keys=[K(120, 168, 62, 128, 0, 1), K(229, 120, 80, 440, 0)])
B.group(ef, 2); B.group([eb], 2); scenes.append(B)
# ---------------------------------------------------------------- C: close-up of the land, brief
Cc = Scene('land', 70)
ld = Layer('land', still(G, 3.3, (240, 160)), 64, 32, keys=[K(0, 120, 80), K(70, 112, 80)]); Cc.group([ld], 3); scenes.append(Cc)
# ---------------------------------------------------------------- D: the man over the city, sinking as the city rises
D = Scene('city', 280, bd=0x739C)      # backdrop = horizon glow, matches the sky's fade
sk = Layer('sky', sky((256, 96)), 64, 32, keys=[K(0, 128, 48), K(280, 128, 48)])
mn = Layer('man', man((160, 96)), 64, 32, keys=[K(0, 120, 70), K(60, 120, 66, 256, 0, 1), K(260, 120, 140), K(280, 120, 150)])
ct = Layer('city', city((240, 64)), 64, 64, keys=[K(0, 120, 146), K(40, 120, 146), K(250, 120, 128, 256, 0, 1), K(280, 120, 128)])
D.group([sk], 2); D.group([mn], 2); D.group([ct], 1); scenes.append(D)
# ---------------------------------------------------------------- E: the eyes
E = Scene('eyes', 70)
ey = Layer('eyes', still(G, 10.2, (256, 128)), 64, 32, keys=[K(0, 122, 80), K(70, 118, 80)]); E.group([ey], 3); scenes.append(E)
# ---------------------------------------------------------------- F: title hand-over (city stays on screen under the title)
F = Scene('title', 2, 0, 0)
ct2 = Layer('city', city((240, 64)), 64, 64, keys=[K(0, 120, 128), K(2, 120, 128)]); F.group([ct2], 1); scenes.append(F)

with open('src/intro_data.c', 'w') as o:
    o.write('// GENERATED by tools/make_intro.py - do not edit\n#include "intro.h"\n')
    o.write('const s16 sin256[256] = {' + ','.join(str(int(round(math.sin(i * math.pi / 128) * 256))) for i in range(256)) + '};\n')
    for si, s in enumerate(scenes):
        s.pack(); n = f's{si}'
        o.write(f'static const u32 {n}_tiles[{max(len(s.words),1)}] __attribute__((aligned(4))) = {{\n' + ',\n'.join(','.join('0x%08x' % w for w in s.words[i:i + 8]) for i in range(0, len(s.words), 8)) + '};\n')
        o.write(f'static const u16 {n}_pal[256] = {{' + ','.join('0x%04x' % v for p in s.pal for v in p) + '};\n')
        for li, l in enumerate(s.layers):
            o.write(f'static const IBlk {n}_b{li}[] = {{' + ','.join('{%d,%d,%d,%d}' % (b['cx'], b['cy'], b['tile'], b['bank']) for b in l.blocks) + '};\n')
            o.write(f'static const IKey {n}_k{li}[] = {{' + ','.join('{%d,%d,%d,%d,%d,%d}' % k for k in l.keys) + '};\n')
        o.write(f'static const ILayer {n}_l[] = {{\n' + ',\n'.join('{%s_b%d,%d,%d,%d,%d,%d,%s_k%d,%d}' % (n, li, len(l.blocks), SHAPES[(l.bw, l.bh)][0], SHAPES[(l.bw, l.bh)][1], l.grp, l.dbl, n, li, len(l.keys)) for li, l in enumerate(s.layers)) + '};\n')
    o.write('const IScene intro_scenes[%d] = {\n' % len(scenes) + ',\n'.join('{s%d_tiles,%d,s%d_pal,s%d_l,%d,%d,%d,%d,0x%04x}' % (i, len(s.words), i, i, len(s.layers), s.frames, s.fin, s.fout, s.bd) for i, s in enumerate(scenes)) + '};\n')
    o.write('const int INTRO_SCENES = %d;\n' % len(scenes))
import pickle; pickle.dump([(s.name, s.words, s.pal, [(l.name, l.bw, l.bh, l.grp, l.dbl, l.keys, [(b['cx'], b['cy'], b['tile'], b['bank']) for b in l.blocks]) for l in s.layers], s.frames) for s in scenes], open('/home/claude/work/out/scenes.pkl', 'wb'))
print([(s.name, s.ntiles, sum(len(l.blocks) for l in s.layers)) for s in scenes])
