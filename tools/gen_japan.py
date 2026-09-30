"""Paints assets/anim/land_japan.png (256x160): a satellite-style close-up of Japan on the ALIEN world (green seas, amber land, tinted clouds).
Coastlines are hand-placed lon/lat outlines -> supersampled mask -> noise-warped; relief/colour/clouds are procedural.  Run: python3 tools/gen_japan.py"""
import numpy as np, cv2
from PIL import Image
W, H, SS = 256, 160, 4
rng = np.random.default_rng(7)
ISL = {
 'honshu': [(130.9,34.0),(132.5,35.5),(134.5,35.6),(136,36),(136.7,37.3),(137.3,37.5),(138.5,37.8),(139.8,39.5),(140,40.6),(140.3,41.0),(141.3,41.4),(141.5,40.5),(142,39.5),(141.5,38.3),(141,37),(140.6,36),(140.9,35.7),(140,35),(139.8,35.2),(139.6,35.3),(139.1,35.2),(138.8,34.6),(138,34.7),(137,34.6),(136.9,34.7),(136.3,34.3),(135.7,33.5),(135.1,34.0),(135.3,34.6),(134.5,34.6),(133,34.3),(132.3,34.2),(131.4,34.0)],
 'hokkaido': [(140,41.5),(140.3,42.5),(140.5,43.3),(141.5,43.3),(141.3,45.2),(141.9,45.5),(143.5,44.3),(145.3,43.3),(144,42.9),(143.3,42),(141.8,42.6),(141,42.3),(140.5,41.8)],
 'kyushu': [(130.9,33.9),(131.8,33.6),(131.9,32.8),(131.4,31.4),(130.7,31.0),(130.2,31.3),(130.2,32.5),(129.7,33.2),(130.3,33.6)],
 'shikoku': [(132.5,33.9),(134.3,34.2),(134.7,33.8),(134.1,33.3),(133,32.8),(132.5,33.4)],
 'sado': [(138.2,38.0),(138.5,38.3),(138.5,37.9),(138.3,37.8)],
}
CLON, CLAT, PPD = 137.2, 38.0, 11.0                       # map centre (shifted so the pan shows the whole arc), pixels per degree of latitude
def proj(lo, la): return ((lo - CLON) * PPD * np.cos(np.radians(37.4)) + W / 2, (CLAT - la) * PPD + H / 2 + 6)
def fbm(h, w, oct=5, base=4, seed=0):
    r = np.random.default_rng(seed); out = np.zeros((h, w), np.float32); a = 1.0; tot = 0
    for o in range(oct):
        n = base * 2 ** o; g = r.random((n + 2, int(n * w / h) + 2)).astype(np.float32)
        out += a * cv2.resize(g, (w, h), interpolation=cv2.INTER_CUBIC); tot += a; a *= .55
    out /= tot; return (out - out.min()) / (out.max() - out.min())
mask = np.zeros((H * SS, W * SS), np.uint8)
for pts in ISL.values(): cv2.fillPoly(mask, [np.array([[x * SS, y * SS] for x, y in (proj(*p) for p in pts)], np.int32)], 255)
warp = (fbm(H * SS, W * SS, 6, 6, 1) - .5) * 14; warp2 = (fbm(H * SS, W * SS, 6, 6, 2) - .5) * 14         # ragged, fractal coasts
yy, xx = np.mgrid[0:H * SS, 0:W * SS].astype(np.float32)
mask = cv2.remap(cv2.GaussianBlur(mask, (0, 0), 3), xx + warp, yy + warp2, cv2.INTER_LINEAR)
land = cv2.resize((mask > 127).astype(np.uint8) * 255, (W, H), interpolation=cv2.INTER_AREA).astype(np.float32) / 255
ld = cv2.distanceTransform((land > .5).astype(np.uint8), cv2.DIST_L2, 5); od = cv2.distanceTransform((land <= .5).astype(np.uint8), cv2.DIST_L2, 5)
n1, n2 = fbm(H, W, 6, 5, 3), fbm(H, W, 5, 10, 4)
elev = np.clip(ld / 9, 0, 1) * .55 + n1 * .45                                                 # interior ridge + noise = mountain spine
gy, gx = np.gradient(cv2.GaussianBlur(elev, (0, 0), 1.0)); shade = np.clip(.5 + (-gx * .8 - gy * .6) * 9, 0, 1)
def lerp(a, b, t): return np.array(a, np.float32) * (1 - t[..., None]) + np.array(b, np.float32) * t[..., None]
depth = np.clip(od / 26, 0, 1)
sea = lerp((70, 190, 140), (8, 78, 70), depth ** .7) * (.92 + .16 * n2[..., None])
sea = sea + 14 * (np.abs(od - 3) < .7)[..., None]                                             # bright surf line along the coast
low, mid, hi = (186, 158, 66), (160, 96, 50), (204, 150, 104)
t = np.clip(elev * 1.5 - .25, 0, 1)
ter = np.where((t < .5)[..., None], lerp(low, mid, np.clip(t * 2, 0, 1)), lerp(mid, hi, np.clip((t - .5) * 2.4, 0, 1))) * (.62 + .7 * shade[..., None]) * (.9 + .2 * n2[..., None])
a = np.clip((land - .3) / .4, 0, 1)[..., None]; img = sea * (1 - a) + ter * a
cm = np.clip((fbm(H, W, 6, 3, 9) - .58) * 6, 0, 1) * np.clip(fbm(H, W, 3, 2, 5) * 1.6 - .25, 0, 1)   # drifting cloud banks
big = np.exp(-(((xx[::SS, ::SS] - 40) / 44) ** 2 + ((yy[::SS, ::SS] - 128) / 30) ** 2)) * np.clip(fbm(H, W, 6, 5, 11) * 1.6, 0, 1)  # the big storm cloud, bottom-left like the original shot
cl = np.clip(cm * .8 + big * 1.4, 0, 1)[..., None]; img = img * (1 - cl) + np.array((226, 244, 232), np.float32) * cl
Image.fromarray(np.clip(img, 0, 255).astype(np.uint8)).convert('RGBA').save('assets/anim/land_japan.png')
