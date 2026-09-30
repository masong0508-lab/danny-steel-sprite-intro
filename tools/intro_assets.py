"""Source-image prep for the sprite-only intro (used by tools/make_intro.py). Everything returns RGBA PIL images."""
import numpy as np, cv2, subprocess, os
from PIL import Image
UP = os.environ.get('INTRO_SRC', '/mnt/user-data/uploads/')
def P(n): return UP + n
def frame(mp4, t, out='/tmp/_f.png'):
    subprocess.run(['ffmpeg', '-v', 'error', '-y', '-ss', str(t), '-i', P(mp4), '-frames:v', '1', out], check=True)
    return Image.open(out).convert('RGB')
def fit(im, size):                     # shrink (keep aspect) into size, centred on a transparent canvas
    im = im.copy(); im.thumbnail(size, Image.LANCZOS)
    c = Image.new('RGBA', size, (0, 0, 0, 0)); c.alpha_composite(im, ((size[0] - im.width) // 2, (size[1] - im.height) // 2)); return c
def hard(im, th=128):
    a = np.array(im); a[..., 3] = np.where(a[..., 3] >= th, 255, 0); return Image.fromarray(a, 'RGBA')

def apple(path, box, size):
    """colour mask: bed/wall are grey, the apple is red/yellow"""
    im = Image.open(path).convert('RGB').crop(box); im = im.resize((im.width // 4, im.height // 4), Image.LANCZOS)
    a = np.array(im); hsv = cv2.cvtColor(a, cv2.COLOR_RGB2HSV)
    h, sa, v = hsv[..., 0], hsv[..., 1], hsv[..., 2]
    m = (((sa > 62) & (v > 70) & ((h < 35) | (h > 165))) | ((sa > 36) & (v > 110) & (h > 14) & (h < 48))).astype(np.uint8)
    m = cv2.morphologyEx(m, cv2.MORPH_CLOSE, np.ones((21, 21), np.uint8)); m = cv2.morphologyEx(m, cv2.MORPH_OPEN, np.ones((11, 11), np.uint8))
    n, lab, st, _ = cv2.connectedComponentsWithStats(m); k = 1 + np.argmax(st[1:, 4]); m = (lab == k).astype(np.uint8)
    cnt, _ = cv2.findContours(m, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE); f = np.zeros_like(m); cv2.drawContours(f, cnt, -1, 1, -1)
    f = cv2.erode(f, np.ones((3, 3), np.uint8))
    out = Image.fromarray(np.dstack([a, f * 255]), 'RGBA'); out = out.crop(out.getchannel('A').getbbox())
    return hard(fit(out, size))

def cutout(name, size):
    im = Image.open(P(name)).convert('RGBA'); bb = im.getchannel('A').point(lambda v: 255 if v > 40 else 0).getbbox()
    return hard(fit(im.crop(bb), size))

def earth(i, size):
    g = Image.open(P('earth-spinning-fast-rotate-olxitbpmxtf60lcf.gif')); g.seek(i); im = g.convert('RGB')
    m = np.zeros((im.height, im.width), np.uint8); cv2.circle(m, (im.width // 2, im.height // 2), min(im.size) // 2 - 6, 255, -1, cv2.LINE_AA)
    im = Image.fromarray(np.dstack([np.array(im), m]), 'RGBA').crop((6, 4, im.width - 6, im.height - 4))
    return hard(im.resize(size, Image.LANCZOS))

def man(size):
    a = np.array(Image.open(P('Screenshot_20260930_112847_Chrome.jpg')).convert('RGB')).astype(int); r, g, b = a[..., 0], a[..., 1], a[..., 2]
    sky = ((b > 100) & (b - r > 35) & (b - g > 12)).astype(np.uint8); sky[:, -10:] = 1
    n, lab = cv2.connectedComponents(sky); keep = np.zeros_like(sky)
    for e in (set(lab[0, :]) | set(lab[:, 0]) | set(lab[:, -1])) - {0}: keep[lab == e] = 1
    alpha = (1 - cv2.dilate(keep, np.ones((3, 3), np.uint8))) * 255; alpha[:, -12:] = 0; alpha[:, :4] = 0
    alpha = cv2.morphologyEx(alpha.astype(np.uint8), cv2.MORPH_OPEN, np.ones((3, 3), np.uint8))
    im = Image.fromarray(np.dstack([a.astype(np.uint8), alpha]), 'RGBA'); im = im.crop(im.getchannel('A').getbbox())
    return hard(fit(im, size))

def city(size, t=6.0):
    im = frame('472986863.mp4', t); W, H = im.size; im = im.crop((0, int(H * .38), W, H)); a = np.array(im)
    gray = cv2.GaussianBlur(cv2.cvtColor(a, cv2.COLOR_RGB2GRAY), (0, 0), 1.2).astype(np.float32)
    tex = cv2.GaussianBlur(np.hypot(cv2.Sobel(gray, cv2.CV_32F, 1, 0), cv2.Sobel(gray, cv2.CV_32F, 0, 1)), (0, 0), 2.0)
    sm = cv2.morphologyEx((tex < 6.0).astype(np.uint8), cv2.MORPH_OPEN, np.ones((5, 5), np.uint8))
    n, lab = cv2.connectedComponents(sm); bg = np.zeros_like(sm)
    for l in set(lab[0, :]) - {0}: bg[lab == l] = 1
    bg[int(a.shape[0] * .8):] = 0
    al = cv2.morphologyEx(((1 - bg) * 255).astype(np.uint8), cv2.MORPH_CLOSE, np.ones((9, 9), np.uint8)); al[:, :4] = 0; al[:, -4:] = 0
    out = Image.fromarray(np.dstack([a, al]), 'RGBA').resize(size, Image.LANCZOS); return hard(out)

def sky(size):
    im = Image.open('assets/painting_stitched.png').convert('RGB'); W, H = im.size
    cw = int(H * .34 * size[0] / size[1]); x0 = int(W * .10)
    a = np.array(im.crop((x0, int(H * .08), x0 + cw, int(H * .08) + int(H * .34))).resize(size, Image.LANCZOS)).astype(np.float32)
    t = np.clip((np.arange(size[1]) - size[1] * .55) / (size[1] * .45), 0, 1)[:, None, None] ** 1.2
    a = a * (1 - t) + np.array((226, 226, 230), np.float32) * t
    return Image.fromarray(a.clip(0, 255).astype(np.uint8), 'RGB').convert('RGBA')

def still(mp4, t, size, box=None):
    im = frame(mp4, t)
    if box: im = im.crop(box)
    return im.resize(size, Image.LANCZOS).convert('RGBA')
