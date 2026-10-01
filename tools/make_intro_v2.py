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

def extend_arm(g, rows=(-64, -32, 0, 32, 64), cols=(-48, -16, 16, 48, 80)):
    """Generic version of extend_hand: finds the rightmost opaque column of a 32x32-block arm sprite, regenerates the last 12 columns and everything
    to the right of them from the clean forearm texture (mirrored), with flat/slightly sloped edges, so the arm runs off the right edge of the screen."""
    import cv2
    CW = CH = 224; OX = OY = 112; cv = np.zeros((CH, CW), np.uint8); bank = g.blks[0]['bank']
    for b in g.blks:
        for ty in range(4):
            for tx in range(4):
                for r in range(8):
                    w = b['words'][(ty * 4 + tx) * 8 + r]
                    for c in range(8): cv[b['cy'] - 16 + ty * 8 + r + OY, b['cx'] - 16 + tx * 8 + c + OX] = (w >> (4 * c)) & 15
    xr = int(np.where((cv > 0).any(0))[0].max()) - OX; X0 = xr - 11; XR = X0 - 1; XL = XR - 24
    med = cv2.medianBlur(cv, 3)
    for x in range(XL, xr + 1):                      # despeckle the pale dither dots on the lower edge
        col = np.where(cv[:, x + OX] > 0)[0]; bot = col.max()
        for y in range(bot - 9, bot + 1):
            if 0 < cv[y, x + OX] <= 4 and med[y, x + OX] > 0: cv[y, x + OX] = med[y, x + OX]
    def edges(x):
        c = np.where(cv[:, x + OX] > 0)[0]; return float(c.min()), float(c.max())
    ed = {x: edges(x) for x in range(XL - 4, XR + 1)}
    def sm(x, k): return float(np.median([ed[min(XR, max(XL - 4, x + d))][k] for d in range(-2, 3)]))
    Ta = np.median([ed[x][0] for x in range(XR - 4, XR + 1)]); Ba = np.median([ed[x][1] for x in range(XR - 4, XR + 1)])
    ts = float(np.clip((Ta - np.median([ed[x][0] for x in range(XR - 24, XR - 19)])) / 20, -0.3, 0.3))
    out = cv.copy(); out[:, X0 + OX:] = 0; n = XR - XL
    for x in range(X0, CW - OX):
        u = (x - XR) % (2 * n); sx = XR - u if u <= n else XL + (u - n)
        Tn = Ta + ts * (x - XR); Bn = Ba; Ts, Bs = sm(sx, 0), sm(sx, 1)
        for y in range(int(round(Tn)), int(round(Bn)) + 1):
            f = (y - Tn) / (Bn - Tn); ys = int(round(Ts + f * (Bs - Ts)))
            for d in (0, 1, -1, 2, -2, 3, -3):
                v = cv[ys + d, sx + OX]
                if v: out[y, x + OX] = v; break
    blks = []
    for cy in rows:
        for cx in cols:
            blk = out[cy - 16 + OY:cy + 16 + OY, cx - 16 + OX:cx + 16 + OX]
            if not blk.any(): continue
            words = []
            for ty in range(0, 32, 8):
                for tx in range(0, 32, 8):
                    T = blk[ty:ty + 8, tx:tx + 8].astype(np.uint32)
                    for r in range(8): words.append(int(sum(int(T[r, c]) << (4 * c) for c in range(8))))
            blks.append(dict(cx=cx, cy=cy, words=words, bank=bank))
    print('arm art ended at', xr, '- blocks', len(g.blks), '->', len(blks)); g.blks = blks

base = load('tools/intro_data_base.c')
def from_base(i, name, keep=None):
    b = base[i]; s = S(name, b['frames'], b['fin'], b['fout'], b['bd']); s.pal = list(b['pal'])
    s.layers = [L(carry(b, li), l['keys'], l['grp'], l['dbl']) for li, l in enumerate(b['layers']) if keep is None or li in keep]; return s

scenes = []
# ---- A: spinning apple, hand slides in

def extend_hand(g, bank_fallback=3):
    """The arm art was clipped by its own sprite canvas (top edge hit the top row, nothing past the right edge).
    Decode the 9 blocks into an index canvas, despeckle the dither noise on the lower edge, regenerate everything right of
    X0 by mirroring the clean forearm texture (edges extrapolated), and re-cut into 32x32 blocks incl. a new top row and right column.
    Only palette indices are moved around, so no new colours / palette banks are needed."""
    import cv2
    X0, XL, XR = 52, 28, 51                         # regenerate x >= X0, mirror texture taken from x in [XL, XR] (pure forearm)
    CW, CH, OX, OY = 160, 128, 64, 80               # canvas x in [-64,96), y in [-80,48)
    cv = np.zeros((CH, CW), np.uint8); bank = g.blks[0]['bank']
    for b in g.blks:
        for ty in range(4):
            for tx in range(4):
                for r in range(8):
                    w = b['words'][(ty * 4 + tx) * 8 + r]
                    for c in range(8): cv[b['cy'] - 16 + ty * 8 + r + OY, b['cx'] - 16 + tx * 8 + c + OX] = (w >> (4 * c)) & 15
    # despeckle: the pale checker dots along the lower edge are quantiser noise -> 3x3 median in index space (indices are ordered by brightness)
    med = cv2.medianBlur(cv, 3)
    for x in range(36, 64):
        col = np.where(cv[:, x + OX] > 0)[0]; bot = col.max()
        for y in range(bot - 9, bot + 1):
            if 0 < cv[y, x + OX] <= 4 and med[y, x + OX] > 0: cv[y, x + OX] = med[y, x + OX]
    def edges(x):
        c = np.where(cv[:, x + OX] > 0)[0]; return float(c.min()), float(c.max())
    ed = {x: edges(x) for x in range(XL - 4, XR + 1)}
    def sm(x, k):                                   # 5-col median of an edge
        return float(np.median([ed[min(XR, max(XL - 4, x + d))][k] for d in range(-2, 3)]))
    Ta, Ba = np.median([ed[x][0] for x in range(XR - 4, XR + 1)]), np.median([ed[x][1] for x in range(XR - 4, XR + 1)])
    ts = (Ta - np.median([ed[x][0] for x in range(36, 41)])) / (XR - 38)              # top edge keeps rising (forearm widens); bottom stays flat
    out = cv.copy(); out[:, X0 + OX:] = 0; n = XR - XL
    for x in range(X0, CW - OX):
        u = (x - XR) % (2 * n); sx = XR - u if u <= n else XL + (u - n)
        Tn = Ta + ts * (x - XR); Bn = Ba; Ts, Bs = sm(sx, 0), sm(sx, 1)
        for y in range(int(round(Tn)), int(round(Bn)) + 1):
            f = (y - Tn) / (Bn - Tn); ys = int(round(Ts + f * (Bs - Ts)))
            for d in (0, 1, -1, 2, -2, 3, -3):
                v = cv[ys + d, sx + OX]
                if v: out[y, x + OX] = v; break
    blks = []
    for cy in (-64, -32, 0, 32):
        for cx in (-48, -16, 16, 48, 80):
            blk = out[cy - 16 + OY:cy + 16 + OY, cx - 16 + OX:cx + 16 + OX]
            if not blk.any(): continue
            words = []
            for ty in range(0, 32, 8):
                for tx in range(0, 32, 8):
                    T = blk[ty:ty + 8, tx:tx + 8].astype(np.uint32)
                    for r in range(8): words.append(int(sum(int(T[r, c]) << (4 * c) for c in range(8))))
            blks.append(dict(cx=cx, cy=cy, words=words, bank=bank))
    print('hand blocks', len(g.blks), '->', len(blks)); g.blks = blks
A = from_base(0, 'apple', keep=[1]); hand = A.layers[0]; extend_hand(hand.g)
free = [k for k in range(16) if k not in used_banks([hand])]
def solid(im):                              # close pin-holes / notches in the apple's silhouette (alpha dither in the source art showed up as black specks on its edge)
    a = np.array(im); m = (a[..., 3] >= 128).astype(np.uint8); cl = cv2.morphologyEx(m, cv2.MORPH_CLOSE, np.ones((5, 5), np.uint8))
    ff = (1 - cl).copy(); h, w = ff.shape; msk = np.zeros((h + 2, w + 2), np.uint8); cv2.floodFill(ff, msk, (0, 0), 2); cl = np.where(ff == 2, 0, 1).astype(np.uint8)   # + enclosed holes
    wgt = m.astype(np.float32); rgb = a[..., :3].astype(np.float32) * wgt[..., None]
    fill = cv2.blur(rgb, (7, 7)) / np.maximum(cv2.blur(wgt, (7, 7)), 1e-3)[..., None]
    new = (cl == 1) & (m == 0); a[..., :3] = np.where(new[..., None], fill, a[..., :3]).astype(np.uint8); a[..., 3] = np.where(cl == 1, 255, 0); return Image.fromarray(a, 'RGBA')
def norm(im, T=96.0):                       # every pose -> same silhouette AREA and centroid on the canvas centre: the apple neither pulses in size nor hops sideways/up when the pose changes
    a = np.array(im.getchannel('A')) >= 128; ys, xs = np.where(a); k = T / a.sum() ** .5
    r = im.resize((round(im.width * k), round(im.height * k)), Image.NEAREST); o = Image.new('RGBA', (128, 128), (0, 0, 0, 0))
    o.paste(r, (round(64 - xs.mean() * k), round(64 - ys.mean() * k))); return o
poses = new_group([norm(solid(img(f'apple_{k}.png'))) for k in range(4)], 16, 16, free[:8], A.pal)
def smooth(f): return f * f * (3 - 2 * f)
START, LAND = (120, 80, 256), (158, 72, 140)       # apple centre x, y and scale (Q8); LAND = resting in the palm (hand is drawn in front, so its thumb/fingers cradle it)
def master(t):                                    # one smoothstep glide, arriving exactly as the hand does (t=150); scale is interpolated geometrically so the shrink looks even
    f = smooth(min(1, t / (A.frames - 1)))
    return (round(START[0] + (LAND[0] - START[0]) * f), round(START[1] + (LAND[1] - START[1]) * f), round(START[2] * (LAND[2] / START[2]) ** f))
seg, t, i = [], 0, 0
while t < A.frames:
    d = max(3, round([8, 5, 8, 5][i % 4] * (1.0 if t < 70 else 1.0 + (t - 70) / 80 * 1.6))); seg.append([i % 4, t, min(A.frames, t + d)]); t += d; i += 1
last0 = max(k for k, sg in enumerate(seg) if sg[0] == 0); seg = seg[:last0 + 1]; seg[-1][2] = A.frames
apple_layers = []
for pose, t0, t1 in seg:
    ks = [K(t, *master(t)) for t in range(t0, t1)]       # a key on EVERY frame: no piecewise-linear speed changes at pose switches
    if len(ks) == 1: ks.append(ks[0])
    apple_layers.append(L(poses[pose], ks, grp=0, dbl=0))
A.layers = [hand] + apple_layers; A.fout = 0; scenes.append(A)       # hard cut to the fist (guide video)
A_ = None;  # (scene appended above)       # last layer = front, so the apple now sits IN FRONT of the hand
import colorsys
def alienise(sc, layers, dh=-0.22, ds=1.25):         # recolour the globe's palette banks only (hand banks untouched): blue seas -> toxic green, land -> amber, clouds tinted
    banks = {b['bank'] for l in layers for b in l.g.blks}
    for bk in banks:
        for i in range(1, 16):
            c = sc.pal[bk * 16 + i]; r, g, b = (c & 31) / 31, ((c >> 5) & 31) / 31, ((c >> 10) & 31) / 31
            h, sat, v = colorsys.rgb_to_hsv(r, g, b); h = (h + dh) % 1; sat = min(1, sat * ds + (0.12 if v > .7 else 0)); v = v ** 1.15
            r, g, b = colorsys.hsv_to_rgb(h, sat, v); sc.pal[bk * 16 + i] = round(r * 31) | (round(g * 31) << 5) | (round(b * 31) << 10)
# ---- A2: fist. Guide video @24fps: hard cut to the fist (arm runs off the right edge), ~0.32 s, then a hard WHITE flash (~0.12 s) and a quick fade into the palm shot
FI = from_base(1, 'fist', keep=[0]); extend_arm(FI.layers[0].g)
FI.frames, FI.fin, FI.fout, FI.flags = 26, 0, 7, 3                 # flags bit0 = flash to white, bit1 = hard flash (full white for all fout frames)
FI.layers[0].keys = [K(0, 170, 112), K(18, 170, 112)]; scenes.append(FI)         # layer is hidden after t=18 -> 7 white frames
# ---- B: palm + globe, timed to the guide: globe 49 px -> 236 px between 0.54 s and 1.33 s (measured on the recording), moving from the palm to centre screen
B = from_base(1, 'earth', keep=[1, 2, 3, 4, 5, 6]); extend_arm(B.layers[0].g)
Dt = [(0, 49), (11, 107), (14, 118), (19, 142), (24, 166), (29, 187), (34, 204), (41, 221), (44, 230)]       # (frame @60fps after the flash, globe diameter px)
Pt = [(0, 160, 87), (11, 162, 86), (24, 151, 81), (34, 136, 82), (44, 120, 81)]
def lerp(tab, t, k): 
    for a, b in zip(tab, tab[1:]):
        if a[0] <= t <= b[0]: f = (t - a[0]) / (b[0] - a[0]); return a[k] + (b[k] - a[k]) * f
    return tab[-1][k]
B.frames, B.fin, B.fout = 44, 4, 0
palm = B.layers[0]; palm.keys = [K(0, 172, 116), K(15, 172, 116)]       # palm leaves once the globe covers it (keeps the per-scanline sprite cost under the 1210 limit)
def gkeys(a, b, mul): return [K(t, round(lerp(Pt, t, 1)), round(lerp(Pt, t, 2)), round(lerp(Dt, t, 1) * mul)) for t in range(a, b + 1)]
spans = [(0, 3), (4, 7), (8, 11), (12, 15)]                                       # 64 px spin poses (native size at D=64 -> scale = 4*D)
for l, (a, b) in zip(B.layers[1:5], spans): l.keys = gkeys(a, b, 4)
B.layers[5].keys = gkeys(16, 43, 2)                                              # 128 px globe (native at D=128 -> scale = 2*D)
hb = used_banks(B.layers[:1]); globe = [l for l in B.layers[1:] if not (used_banks([l]) & hb)]
print('globe layers', len(globe), 'of', len(B.layers) - 1); alienise(B, globe); scenes.append(B)
C = from_base(2, 'land', keep=[]); free = [k for k in range(16)]                  # close-up shot: now the alien world's Japan (assets/anim/land_japan.png, made by tools/gen_japan.py), same 8 px pan as before
C.pal = [0] * 256; C.layers = [L(new_group([img('land_japan.png')], 64, 32, free[:8], C.pal)[0], base[2]['layers'][0]['keys'])]; C.fin = 0; scenes.append(C)
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
