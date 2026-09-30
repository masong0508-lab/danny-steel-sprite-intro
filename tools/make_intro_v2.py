"""Builds src/intro_data.c (v2).  Run from the project root:  python3 tools/make_intro_v2.py
Untouched scenes (hand, earth, land, man, sky) are carried over verbatim from tools/intro_data_base.c;
the apple, eyes and skyline scenes are rebuilt from assets/anim/*.png.   Needs Pillow, numpy, opencv-python."""
import sys, os, math, pickle, numpy as np, cv2
from PIL import Image
sys.path.insert(0, os.path.dirname(__file__))
from intro_parse import load

SZ = [[(8,8),(16,16),(32,32),(64,64)], [(16,8),(32,8),(32,16),(64,32)], [(8,16),(8,32),(16,32),(32,64)]]
SHAPE_OF = {wh: (s, z) for s in range(3) for z, wh in enumerate(SZ[s])}
AN = 'assets/anim/'
def img(n): return Image.open(AN + n).convert('RGBA')

class G:
    def __init__(s, shape, size, blks): s.shape, s.size, s.blks = shape, size, blks
class L:
    def __init__(s, g, keys, grp=-1, dbl=0): s.g, s.keys, s.grp, s.dbl = g, keys, grp, dbl
class S:
    def __init__(s, name, frames, fin, fout, bd, flags=0): s.name, s.frames, s.fin, s.fout, s.bd, s.flags = name, frames, fin, fout, bd, flags; s.layers = []; s.pal = [0] * 256
def K(t, x, y, sc=256, ang=0, ease=0): return (t, x, y, sc, ang, ease)

def carry(base, li):
    b = base['layers'][li]; w, h = SZ[b['shape']][b['size']]; tc = w * h // 64
    return G(b['shape'], b['size'], [dict(cx=cx, cy=cy, words=base['words'][t * 8:(t + tc) * 8], bank=bk) for cx, cy, t, bk in b['blks']])

def blockify(im, bw, bh):
    W, H = im.width, im.height; assert W % bw == 0 and H % bh == 0, (W, H, bw, bh)
    a = np.array(im); out = []
    for by in range(0, H, bh):
        for bx in range(0, W, bw):
            p = a[by:by + bh, bx:bx + bw]
            if (p[..., 3] >= 128).any(): out.append(dict(cx=bx + bw // 2 - W // 2, cy=by + bh // 2 - H // 2, px=p))
    return out

def quantise(blocks, banks, pal):
    means = np.array([b['px'][..., :3][b['px'][..., 3] >= 128].mean(0) for b in blocks], np.float32); nb = min(len(banks), len(blocks))
    if nb > 1:
        _, lab, _ = cv2.kmeans(means, nb, None, (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, .1), 4, cv2.KMEANS_PP_CENTERS); lab = lab.ravel()
    else: lab = np.zeros(len(blocks), int)
    for k in range(nb):
        mem = [b for b, l in zip(blocks, lab) if l == k]
        if not mem: continue
        pix = np.concatenate([b['px'][..., :3][b['px'][..., 3] >= 128] for b in mem]).reshape(-1, 1, 3).astype(np.uint8)
        q = Image.fromarray(pix).quantize(15, Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
        pl = (q.getpalette() or [])[:45]; pl += [0] * (45 - len(pl)); pl = [0, 0, 0] + pl
        pim = Image.new('P', (1, 1)); pim.putpalette(pl + [0] * (768 - len(pl)))
        for b in mem:
            idx = np.array(Image.fromarray(np.ascontiguousarray(b['px'][..., :3])).quantize(palette=pim, dither=Image.Dither.FLOYDSTEINBERG), np.uint8)
            idx = np.where(idx == 0, 1, idx); idx = np.where(b['px'][..., 3] >= 128, idx, 0); b['idx'] = idx; b['bank'] = banks[k]
        for i in range(16): pal[banks[k] * 16 + i] = ((pl[i*3] >> 3) | ((pl[i*3+1] >> 3) << 5) | ((pl[i*3+2] >> 3) << 10)) if i else 0
    for b in blocks:
        idx = b['idx']; bh, bw = idx.shape; words = []
        for ty in range(0, bh, 8):
            for tx in range(0, bw, 8):
                T = idx[ty:ty + 8, tx:tx + 8].astype(np.uint32)
                for r in range(8): words.append(int(sum(int(T[r, c]) << (4 * c) for c in range(8))))
        b['words'] = words

def new_group(ims, bw, bh, banks, pal):
    shape, size = SHAPE_OF[(bw, bh)]; per = [blockify(i, bw, bh) for i in ims]; quantise([b for p in per for b in p], banks, pal)
    return [G(shape, size, p) for p in per]

def used_banks(layers): return {b['bank'] for l in layers for b in l.g.blks}

base = load('tools/intro_data_base.c')
def from_base(i, name, keep=None):
    b = base[i]; s = S(name, b['frames'], b['fin'], b['fout'], b['bd']); s.pal = list(b['pal'])
    s.layers = [L(carry(b, li), l['keys'], l['grp'], l['dbl']) for li, l in enumerate(b['layers']) if keep is None or li in keep]; return s

scenes = []
# ---- A: spinning apple, hand slides in
A = from_base(0, 'apple', keep=[1]); hand = A.layers[0]
free = [k for k in range(16) if k not in used_banks([hand])]
def norm(im, H=112):                        # every pose -> same apple height, centred on its bbox, bottom-aligned (no size pulsing while it turns)
    bb = im.getchannel('A').point(lambda v: 255 if v >= 128 else 0).getbbox(); c = im.crop(bb); w = max(1, round(c.width * H / c.height))
    c = c.resize((w, H), Image.NEAREST); o = Image.new('RGBA', (128, 128), (0, 0, 0, 0)); o.alpha_composite(c, ((128 - w) // 2, 120 - H)); return o
poses = new_group([norm(img(f'apple_{k}.png')) for k in range(4)], 16, 16, free[:8], A.pal)
def smooth(f): return f * f * (3 - 2 * f)
START, LAND = (120, 80, 256), (158, 72, 140)       # apple centre x, y and scale (Q8); LAND = resting in the palm (hand is drawn in front, so its thumb/fingers cradle it)
def master(t):                                    # one smoothstep glide, arriving exactly as the hand does (t=150), so it settles softly
    f = smooth(min(1, t / (A.frames - 1)))
    return tuple(round(a + (b - a) * f) for a, b in zip(START, LAND))
seg, t, i = [], 0, 0
while t < A.frames:
    d = max(3, round([8, 5, 8, 5][i % 4] * (1.0 if t < 70 else 1.0 + (t - 70) / 80 * 1.6))); seg.append([i % 4, t, min(A.frames, t + d)]); t += d; i += 1
last0 = max(k for k, sg in enumerate(seg) if sg[0] == 0); seg = seg[:last0 + 1]; seg[-1][2] = A.frames
apple_layers = []
for pose, t0, t1 in seg:
    (xa, ya, sa), (xb, yb, sb) = master(t0), master(t1 - 1)
    apple_layers.append(L(poses[pose], [K(t0, xa, ya, sa), K(t1 - 1, xb, yb, sb)], grp=0, dbl=0))
A.layers = apple_layers + [hand]; scenes.append(A)
import colorsys
def alienise(sc, layers, dh=-0.22, ds=1.25):         # recolour the globe's palette banks only (hand banks untouched): blue seas -> toxic green, land -> amber, clouds tinted
    banks = {b['bank'] for l in layers for b in l.g.blks}
    for bk in banks:
        for i in range(1, 16):
            c = sc.pal[bk * 16 + i]; r, g, b = (c & 31) / 31, ((c >> 5) & 31) / 31, ((c >> 10) & 31) / 31
            h, sat, v = colorsys.rgb_to_hsv(r, g, b); h = (h + dh) % 1; sat = min(1, sat * ds + (0.12 if v > .7 else 0)); v = v ** 1.15
            r, g, b = colorsys.hsv_to_rgb(h, sat, v); sc.pal[bk * 16 + i] = round(r * 31) | (round(g * 31) << 5) | (round(b * 31) << 10)
B = from_base(1, 'earth'); hb = used_banks(B.layers[:2]); globe = [l for l in B.layers[2:] if not (used_banks([l]) & hb)]
print('globe layers', len(globe), 'of', len(B.layers) - 2); alienise(B, globe); scenes.append(B)
C = from_base(2, 'land', keep=[]); free = [k for k in range(16)]                  # close-up shot: now the alien world's Japan (assets/anim/land_japan.png, made by tools/gen_japan.py), same 8 px pan as before
C.pal = [0] * 256; C.layers = [L(new_group([img('land_japan.png')], 64, 32, free[:8], C.pal)[0], base[2]['layers'][0]['keys'])]; scenes.append(C)
# ---- D: man over the city (full-bleed, bottom-aligned like the title)
D = from_base(3, 'city', keep=[0, 1]); ck = [(t, x, 128, sc, a, e) for t, x, y, sc, a, e in base[3]['layers'][2]['keys']]
free = [k for k in range(16) if k not in used_banks(D.layers)]
city_d = new_group([img('city_0.png')], 64, 32, free[:2], D.pal)[0]
D.layers.append(L(city_d, ck)); D.fout = 0; scenes.append(D)       # man scene ends on a HARD CUT to the eyes (reference video)
# ---- E: eyes (top 256x48 + swappable strip 256x48 + bottom 256x64), ending in a white flash
E = S('eyes', 23, 0, 1, 0x0000, flags=1)      # reference timing @60fps: hard cut in, eyes closed 10f, half 2f, open 10f, then ONE white frame (hard cut out)
def feather(strip, ref, m=8):
    h, w = strip.shape[:2]; y = np.minimum(np.arange(h), np.arange(h)[::-1])[:, None]; x = np.minimum(np.arange(w), np.arange(w)[::-1])[None, :]
    r = np.clip(np.minimum(y, x) / m, 0, 1)[..., None]; r = r * r * (3 - 2 * r)
    return (strip * r + ref * (1 - r)).astype(np.uint8)
op = np.array(img('eye_open.png').convert('RGB')).astype(np.float32)
def strip_im(n): return Image.fromarray(feather(np.array(img(n).convert('RGB')).astype(np.float32), op), 'RGB').convert('RGBA')
ft, fb = img('face_top.png'), img('face_bot.png')
g_top, g_bot, g_closed, g_half, g_open = new_group([ft, fb, strip_im('eye_closed.png'), strip_im('eye_half.png'), strip_im('eye_open.png')], 32, 16, list(range(8)), E.pal)
def win(gr, t0, t1, x, y): return L(gr, [K(t0, x, y), K(t1, x, y)])
TL = [(g_closed, 0, 9), (g_half, 10, 11), (g_open, 12, 22)]
E.layers = [win(g_top, 0, 22, 120, 24), win(g_bot, 0, 22, 120, 128)] + [win(gr, a, b, 120, 72) for gr, a, b in TL]; scenes.append(E)
# ---- F: title hand-over + looping skyline (ping-pong over 4 frames, stepped by intro_title_tick)
HOLD = 6; SEQ = [0, 1, 2, 3, 2, 1]
F = S('title', 2, 0, 0, 0x0000)
cf = new_group([img(f'city_{k}.png') for k in range(4)], 64, 32, [0, 1], F.pal)
F.layers = [L(cf[fr], [K(s * HOLD, 120, 128), K(s * HOLD + HOLD - 1, 120, 128)]) for s, fr in enumerate(SEQ)]; scenes.append(F)

def pack(s):
    words, seen = [], {}
    for l in s.layers:
        if id(l.g) in seen: continue
        seen[id(l.g)] = len(seen)
        for b in l.g.blks: b['tile'] = len(words) // 8; words += b['words']
    assert len(words) // 8 <= 1024, (s.name, 'tiles', len(words) // 8)
    s.words = words; s.gid = seen
    live = {}
    for t in range(max(s.frames, 40)):
        live[t] = sum(len(l.g.blks) for l in s.layers if l.keys[0][0] <= t <= l.keys[-1][0])
    assert max(live.values()) <= 120, (s.name, 'sprites', max(live.values()))
    return len(words) // 8, max(live.values())
info = [(s.name,) + pack(s) for s in scenes]; print(info)

with open('src/intro_data.c', 'w') as o:
    o.write('// GENERATED by tools/make_intro_v2.py - do not edit\n#include "intro.h"\n')
    o.write('const s16 sin256[256] = {' + ','.join(str(int(round(math.sin(i * math.pi / 128) * 256))) for i in range(256)) + '};\n')
    for si, s in enumerate(scenes):
        n = f's{si}'
        o.write(f'static const u32 {n}_tiles[{max(len(s.words), 1)}] __attribute__((aligned(4))) = {{\n' + ',\n'.join(','.join('0x%08x' % w for w in s.words[i:i + 8]) for i in range(0, len(s.words), 8)) + '};\n')
        o.write(f'static const u16 {n}_pal[256] __attribute__((aligned(4))) = {{' + ','.join('0x%04x' % v for v in s.pal) + '};\n')
        done = set()
        for l in s.layers:
            gi = s.gid[id(l.g)]
            if gi in done: continue
            done.add(gi); o.write(f'static const IBlk {n}_b{gi}[] = {{' + ','.join('{%d,%d,%d,%d}' % (b['cx'], b['cy'], b['tile'], b['bank']) for b in l.g.blks) + '};\n')
        for li, l in enumerate(s.layers): o.write(f'static const IKey {n}_k{li}[] = {{' + ','.join('{%d,%d,%d,%d,%d,%d}' % k for k in l.keys) + '};\n')
        o.write(f'static const ILayer {n}_l[] = {{\n' + ',\n'.join('{%s_b%d,%d,%d,%d,%d,%d,%s_k%d,%d}' % (n, s.gid[id(l.g)], len(l.g.blks), l.g.shape, l.g.size, l.grp, l.dbl, n, li, len(l.keys)) for li, l in enumerate(s.layers)) + '};\n')
    o.write('const IScene intro_scenes[%d] = {\n' % len(scenes) + ',\n'.join('{s%d_tiles,%d,s%d_pal,s%d_l,%d,%d,%d,%d,0x%04x,%d}' % (i, len(s.words), i, i, len(s.layers), s.frames, s.fin, s.fout, s.bd, s.flags) for i, s in enumerate(scenes)) + '};\n')
    o.write('const int INTRO_SCENES = %d;\nconst int INTRO_TITLE_HOLD = %d, INTRO_TITLE_CYCLE = %d;\n' % (len(scenes), HOLD, HOLD * len(SEQ)))
pickle.dump([(s.name, s.words, s.pal, s.frames, s.bd) for s in scenes], open('/tmp/scenes_v2.pkl', 'wb'))
