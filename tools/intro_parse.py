"""Parses a generated intro_data.c back into dicts (used by make_intro_v2.py to carry untouched scenes)."""
import re
def load(path):
    t = open(path).read(); out = []
    sc = re.search(r'intro_scenes\[\d+\] = \{(.*?)\};\nconst int', t, re.S).group(1)
    rows = re.findall(r'\{(s\d+_tiles[^}]*)\}', sc)
    for i, r in enumerate(rows):
        f = r.split(','); n = f's{i}'
        nwords, nl, frames, fin, fout, bd = int(f[1]), int(f[4]), int(f[5]), int(f[6]), int(f[7]), int(f[8], 16)
        body = re.search(rf'{n}_tiles\[\d+\][^=]*= \{{(.*?)\}};', t, re.S).group(1)
        words = [int(x, 16) for x in re.findall(r'0x[0-9a-fA-F]+', body)]
        pal = [int(x, 16) for x in re.findall(r'0x[0-9a-fA-F]+', re.search(rf'{n}_pal\[256\][^=]*= \{{(.*?)\}};', t, re.S).group(1))]
        lrows = re.findall(r'\{(%s_b\d+,[^}]*)\}' % n, re.search(rf'{n}_l\[\] = \{{(.*?)\}};\n', t + '\n', re.S).group(1) + '}')
        layers = []
        for lr in lrows:
            g = lr.split(','); bi, nb, shape, size, grp, dbl, ki = g[0], int(g[1]), int(g[2]), int(g[3]), int(g[4]), int(g[5]), g[6]
            bl = re.search(rf'{bi}\[\] = \{{(.*?)\}};', t, re.S).group(1)
            blks = [tuple(map(int, x.split(','))) for x in re.findall(r'\{([-\d,]+)\}', bl)]
            kl = re.search(rf'{ki}\[\] = \{{(.*?)\}};', t, re.S).group(1)
            keys = [tuple(map(int, x.split(','))) for x in re.findall(r'\{([-\d,]+)\}', kl)]
            layers.append(dict(shape=shape, size=size, grp=grp, dbl=dbl, blks=blks, keys=keys))
        out.append(dict(words=words, pal=pal, frames=frames, fin=fin, fout=fout, bd=bd, layers=layers))
    return out
