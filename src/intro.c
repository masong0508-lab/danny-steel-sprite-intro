#include "intro.h"
static const u8 SZ[3][4][2] = {{{8,8},{16,16},{32,32},{64,64}}, {{16,8},{32,8},{32,16},{64,32}}, {{8,16},{8,32},{16,32},{32,64}}};
#define RESERVED 8                     /* OAM entries 0-7 hold the two affine matrices (their attr3 slots), real sprites start at 8 */

static int smooth(int f) { return ((f * f >> 8) * (768 - 2 * f)) >> 8; }     // f in 0..256
static int track(const ILayer *L, int t, int *x, int *y, int *sc, int *ang) {
    const IKey *k = L->keys;
    if (t < k[0].t || t > k[L->nkeys - 1].t) return 0;
    int i = 0; while (i + 1 < L->nkeys - 0 && t > k[i + 1].t) i++;
    if (i >= L->nkeys - 1) i = L->nkeys - 2;
    const IKey *a = &k[i], *b = &k[i + 1]; int span = b->t - a->t, f = span ? (t - a->t) * 256 / span : 0;
    if (a->ease) f = smooth(f);
    *x = a->x + ((b->x - a->x) * f >> 8); *y = a->y + ((b->y - a->y) * f >> 8);
    *sc = a->sc + ((b->sc - a->sc) * f >> 8); *ang = a->ang + ((b->ang - a->ang) * f >> 8);
    return 1;
}

void intro_build(const IScene *S, int t, u16 *oam) {
    for (int i = 0; i < 512; i++) oam[i] = 0;
    for (int e = 0; e < 128; e++) oam[e * 4] = 0x0200;                   // every entry hidden
    int n = RESERVED;
    for (int li = S->nlayers - 1; li >= 0; li--) {                        // last layer = front
        const ILayer *L = &S->layers[li]; int X, Y, sc, ang;
        if (!track(L, t, &X, &Y, &sc, &ang)) continue;
        int w = SZ[L->shape][L->size][0], h = SZ[L->shape][L->size][1], aff = L->grp >= 0;
        int c = 256, s = 0;
        if (aff) {
            c = sin256[(ang + 64) & 255]; s = sin256[ang & 255];
            int pa = c * 256 / sc * 246 >> 8, pb = s * 256 / sc * 246 >> 8;   // 4% over-draw hides seams between blocks
            u16 *m = &oam[L->grp * 16];
            m[3] = (u16)pa; m[7] = (u16)pb; m[11] = (u16)-pb; m[15] = (u16)pa;
        }
        for (int bi = 0; bi < L->n && n < 128; bi++) {
            const IBlk *B = &L->blk[bi]; int dx = B->cx, dy = B->cy;
            if (aff) { dx = ((c * B->cx - s * B->cy) * sc + 0x8000) >> 16; dy = ((s * B->cx + c * B->cy) * sc + 0x8000) >> 16; }
            int bw = aff && L->dbl ? w * 2 : w, bh = aff && L->dbl ? h * 2 : h;
            int x = X + dx - bw / 2, y = Y + dy - bh / 2;
            if (x + bw <= 0 || x >= 240 || y + bh <= 0 || y >= 160) continue;
            oam[n * 4 + 0] = (y & 255) | (aff ? (L->dbl ? 0x300 : 0x100) : 0) | (L->shape << 14);
            oam[n * 4 + 1] = (x & 511) | (aff ? L->grp << 9 : 0) | (L->size << 14);
            oam[n * 4 + 2] = B->tile | (2 << 10) | (B->bank << 12);      // priority 2: above the painting, below the title text
            n++;
        }
    }
}

#ifndef INTRO_HOST
#include "music.h"
#define OAMW      ((volatile u16 *)0x07000000)
#define OBJ_VRAM  ((volatile u32 *)0x06010000)
#define OBJ_PAL   ((volatile u16 *)0x05000200)
static u16 shadow[512];
static void wait_vblank(void) { while (REG_VCOUNT >= 160) music_poll(); while (REG_VCOUNT < 160) music_poll(); }
static void push_oam(void) { for (int i = 0; i < 512; i++) OAMW[i] = shadow[i]; }
#define DMA3SAD (*(volatile u32 *)0x040000D4)
#define DMA3DAD (*(volatile u32 *)0x040000D8)
#define DMA3CNT (*(volatile u32 *)0x040000DC)
static void dma32(const void *src, volatile void *dst, u32 words) { DMA3SAD = (u32)src; DMA3DAD = (u32)dst; DMA3CNT = 0x84000000u | words; }   // 32-bit copy, CPU halts until done
static int fade_level(const IScene *S, int t) {                       // 0..16 = how far into the fade-in / fade-out we are
    if (t < S->fin) return 16 - t * 16 / S->fin;
    if (S->fout && t >= S->frames - S->fout) return (S->flags & 2) ? 16 : (t - (S->frames - S->fout) + 1) * 16 / S->fout;   // flags bit1: hard flash (full level at once)
    return 0;
}
static void show(const IScene *S, int t, int white) {                 // build frame t in RAM, then flip it onto the screen at the next vblank
    intro_build(S, t, shadow);
    int fading_out = S->fout && t >= S->frames - S->fout, bl = fade_level(S, t);
    wait_vblank(); push_oam();
    REG_BLDCNT = (fading_out && (S->flags & 1)) || (white && t < S->fin) ? 0x00BF : 0x00FF;   // 0xBF = to white, 0xFF = to black
    REG_BLDY = bl > 16 ? 16 : bl; REG_DISPCNT = OBJ_ON;
}
static void load(const IScene *S, int white) {                        // hard cut: the whole swap (tiles, palette, first frame) happens inside ONE vblank, so there is no black gap
    intro_build(S, 0, shadow);
    wait_vblank();                                                    // returns at the start of vblank (~83k cycles available)
    PAL16[0] = S->backdrop;
    dma32(S->tiles, OBJ_VRAM, S->nwords); dma32(S->pal, OBJ_PAL, 128);
    push_oam();
    REG_BLDCNT = white ? 0x00BF : 0x00FF; REG_BLDY = (white && !S->fin) ? 16 : fade_level(S, 0); REG_DISPCNT = OBJ_ON;   // after a white flash, stay white until the title takes over
}
void intro_play(void) {
    *(volatile u16 *)0x04000204 = 0x4317;                             // WAITCNT: faster ROM (3/1 wait states + prefetch) so the big tile swaps fit inside one vblank
    int white = 0;                                                    // previous scene ended on a white flash -> this scene starts from white too
    music_start();
    for (int si = 0; si < INTRO_SCENES; si++) {
        const IScene *S = &intro_scenes[si];
        load(S, white);
        if (S->frames <= 2) break;                                    // last scene: stays on OAM for the title
        for (int t = 1; t < S->frames; t++) show(S, t, white);
        white = S->flags & 1;
    }
}
static int last_step = -1;
void intro_title_tick(int frame) {
    int step = (frame % INTRO_TITLE_CYCLE) / INTRO_TITLE_HOLD;
    if (step == last_step) return;                                     // same pose: OAM already correct
    last_step = step;
    intro_build(&intro_scenes[INTRO_SCENES - 1], frame % INTRO_TITLE_CYCLE, shadow); push_oam();
}
void intro_reload_title(void) {
    const IScene *S = &intro_scenes[INTRO_SCENES - 1];
    for (int i = 0; i < S->nwords; i++) OBJ_VRAM[i] = S->tiles[i];
    for (int i = 0; i < 256; i++) OBJ_PAL[i] = S->pal[i];
    last_step = -1; intro_build(S, 0, shadow); push_oam();             // the menu drew its own sprites into OAM: put the skyline back
}
#endif
