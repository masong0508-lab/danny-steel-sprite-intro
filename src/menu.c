#include "menu.h"
#include "music.h"
#include "intro.h"
#define MW 120
#define MH 80
#define REG_BG2PA R16(0x04000020)
#define REG_BG2PB R16(0x04000022)
#define REG_BG2PC R16(0x04000024)
#define REG_BG2PD R16(0x04000026)
#define REG_BG2X  (*(volatile u32 *)0x04000028)
#define REG_BG2Y  (*(volatile u32 *)0x0400002C)
#define FADE_IN 24                              /* frames: picture fades in from black */
#define VOL_FRAMES 90                           /* frames: music glides 100% -> 40% */
#define MUSIC_PCT 40
#define VIDEO_FPS_X2 17                         /* loop: 8.5 fps, in half-frames per second (display = 120 half-frames/s) */

// ---- label / cursor sprites (OBJ tiles 512+ are the only OBJ VRAM that bitmap mode 4 leaves free) ----
#define OAMW     ((volatile u16 *)0x07000000)
#define OBJ_PAL  ((volatile u16 *)0x05000200)
#define UI_VRAM  ((volatile u32 *)0x06014000)
#define UI_TILE  512
#define RGB(r, g, b) ((u16)((r) | ((g) << 5) | ((b) << 10)))
#define BGGREY RGB(12, 12, 12)                  /* about the average colour of the background: the sprites fade toward it */
static const s16 LX[2][3] = { {67, 134, 187}, {65, 124, 178} };   // label centres (measured from the reference video, 240-px screen)
#define LABEL_Y 112                             /* top of the 16-px label sprites */
#define CURSOR_Y 79                             /* top of the 16x32 cursor sprite */

// ---- option-select transition (reference video: labels fade as a dark rounded square dissolves in, zoom/rotate through it, dissolve into the next screen) ----
#define SLIDE 18                                /* frames: cursor glides to the next label (eased) */
#define T_STEP 2                                /* vblanks per transition frame (30 fps) */
#define FO_V 12                                 /* vblanks: labels + cursor fade out (starts with the transition) */
#define FI_V0 20                                /* vblank where the next screen's labels start to fade in (as the dissolve begins) */
#define FI_V 8                                  /* vblanks: ... and how long that takes */
enum { M_IDLE, M_SLIDE, M_TRANS };

static u32 buf[MW * MH / 4] __attribute__((section(".ewram"), aligned(4)));   // decoded frame (EWRAM; falls back to normal RAM if no section)
static int t, frame, page, was_b, was_a, was_l, was_r, acc;
static int st, level, sel, prev_main, mode;      // level 0 = NEW GAME/LOAD/ERASE, level 1 = SLOT 1/SLOT 2/BACK; mode = which main option opened the slots
static int cx, slide_from, slide_t, lift;        // cursor centre x (px), slide state, slide "hop" height
static int dir, tp, tsub, v, switched;           // transition: +1 forward / -1 back, frame, sub-step, vblank counter
static int pend; static const u16 *pend_pal;     // a decoded page waiting for the next vblank to be flipped in (together with its palette)

static void lz77(const void *src, void *dst) {                     // BIOS LZ77UnCompWram (SWI 0x11)
    register const void *r0 __asm__("r0") = src; register void *r1 __asm__("r1") = dst;
    __asm__ volatile("swi 0x11" : "+r"(r0), "+r"(r1) :: "r2", "r3", "memory");
}
static void draw(const u8 *stream, int pg) {                       // decode one stream, copy its 80 rows into page pg (mode-4 stride = 240)
    lz77(stream, buf);
    volatile u32 *vv = (volatile u32 *)(VRAM8 + (pg ? 0xA000 : 0));
    for (int y = 0; y < MH; y++) for (int x = 0; x < MW / 4; x++) vv[y * 60 + x] = buf[y * (MW / 4) + x];
}
static void queue(const u8 *stream, const u16 *pal) {              // draw into the hidden page; it is shown at the start of the NEXT vblank (no tearing, palette swaps in sync)
    page ^= 1; draw(stream, page); pend = 1; pend_pal = pal;
}
static void set_pal(const u16 *p) { for (int i = 0; i < 256; i++) PAL16[i] = p[i]; }
static void flip_pending(void) {
    if (!pend) return;
    if (pend_pal) set_pal(pend_pal);
    REG_DISPCNT = 4 | DCNT_BG2 | OBJ_ON | (page ? 0x10 : 0); pend = 0; pend_pal = 0;
}

// ---- sprites ----
static int smooth(int f) { return ((f * f >> 8) * (768 - 2 * f)) >> 8; }     // f in 0..256, smoothstep
static u16 mix(u16 a, u16 b, int f) {                              // a -> b, f in 0..16
    int r = (a & 31), g = (a >> 5) & 31, bl = (a >> 10) & 31, r2 = (b & 31), g2 = (b >> 5) & 31, b2 = (b >> 10) & 31;
    return RGB(r + (r2 - r) * f / 16, g + (g2 - g) * f / 16, bl + (b2 - bl) * f / 16);
}
static void ui_draw(int fade) {                                    // fade 0 = fully shown, 16 = gone
    static const u8 TXT[4] = {31, 26, 22, 17};                     // white + 3 anti-alias steps (over BGGREY)
    static const u8 BLUE_POS[4] = {1, 0, 1, 2};                    // which chevron is blue (sweeps down and back)
    for (int i = 0; i < 4; i++) OBJ_PAL[240 + 1 + i] = mix(RGB(TXT[i], TXT[i], TXT[i]), BGGREY, fade);
    int bp = BLUE_POS[(t / 12) & 3];
    for (int j = 0; j < 3; j++) OBJ_PAL[224 + 1 + j] = mix(j == bp ? RGB(10, 24, 30) : RGB(31, 17, 2), BGGREY, fade);
    OBJ_PAL[224 + 4] = mix(RGB(4, 4, 5), BGGREY, fade);
    for (int e = 0; e < 7; e++) OAMW[e * 4] = 0x0200;              // hide ours, then place the visible ones
    if (fade >= 16) return;
    for (int k = 0; k < 3; k++) {
        int L = level * 3 + k, x = LX[level][k] - 32;
        OAMW[k * 8 + 0] = LABEL_Y | 0x4000;  OAMW[k * 8 + 1] = (x & 511) | 0x8000;        OAMW[k * 8 + 2] = (UI_TILE + L * 16) | (15 << 12);      // 32x16, left half
        OAMW[k * 8 + 4] = LABEL_Y | 0x4000;  OAMW[k * 8 + 5] = ((x + 32) & 511) | 0x8000;  OAMW[k * 8 + 6] = (UI_TILE + L * 16 + 8) | (15 << 12);  // right half
    }
    int y = CURSOR_Y - lift;
    OAMW[24] = (y & 255) | 0x8000; OAMW[25] = ((cx - 8) & 511) | 0x8000; OAMW[26] = (UI_TILE + 96) | (14 << 12);   // 16x32
}

void menu_enter(void) {
    REG_DISPCNT = DCNT_BLANK; REG_BLDCNT = 0x00FF; REG_BLDY = 16;
    set_pal(menu_pal);
    REG_BG2PA = 128; REG_BG2PD = 128; REG_BG2PB = 0; REG_BG2PC = 0; REG_BG2X = 0; REG_BG2Y = 0;   // 0.5 texel per pixel = 2x zoom
    REG_BGCNT(2) = 0;                                               // priority 0, nothing else needed in bitmap mode
    for (int i = 0; i < ui_tile_words; i++) UI_VRAM[i] = ui_tiles[i];
    for (int e = 0; e < 128; e++) OAMW[e * 4] = 0x0200;             // drop the title's skyline sprites (intro_reload_title() restores them on exit)
    frame = 0; t = 0; page = 0; acc = 0; pend = 0; pend_pal = 0;
    was_a = was_b = was_l = was_r = 1;                              // keys held on entry must be released first
    st = M_IDLE; level = 0; sel = 0; prev_main = 0; mode = 0; cx = LX[0][0]; lift = 0;
    draw(menu_bands[0], 0);
    ui_draw(0);
    REG_DISPCNT = 4 | DCNT_BG2 | OBJ_ON;                            // mode 4, BG2 + the label/cursor sprites, page 0 shown
}

static void start_trans(int d) { dir = d; st = M_TRANS; tp = 0; tsub = 0; v = 0; switched = 0; }
static void go_sel(int n) { slide_from = cx; sel = n; slide_t = 0; st = M_SLIDE; }
static void slot_chosen(int m, int slot) { (void)m; (void)slot; }    // PLACEHOLDER: m = 0 NEW GAME / 1 LOAD / 2 ERASE, slot = 0 or 1 -> start / load / erase here

int menu_update(void) {
    flip_pending();
    if (t < FADE_IN) REG_BLDY = 16 - t * 16 / FADE_IN; else REG_BLDY = 0;
    if (t < VOL_FRAMES) music_set_volume(100 - (100 - MUSIC_PCT) * t / VOL_FRAMES); else music_set_volume(MUSIC_PCT);
    t++;
    u16 k = ~REG_KEYINPUT;
    int a = k & KEY_A, b = k & KEY_B, l = k & KEY_LEFT, r = k & KEY_RIGHT;
    int ea = a && !was_a, eb = b && !was_b, el = l && !was_l, er = r && !was_r;
    was_a = a; was_b = b; was_l = l; was_r = r;
    int fade = 0;

    if (st == M_TRANS) {
        if (tsub == 0) {                                            // a new transition frame every T_STEP vblanks
            if (tp < TRANS_FRAMES) queue(trans_bands[dir > 0 ? tp : TRANS_FRAMES - 1 - tp], tp == 0 ? trans_pal : 0);
            else queue(menu_bands[0], menu_pal);                    // last: the normal background again (palette swaps back in the same flip)
        }
        if (v < FO_V) fade = v * 16 / FO_V;                         // labels + cursor fade out while the square dissolves in
        else {
            if (!switched) {                                        // invisible now: change screens
                switched = 1; level ^= 1;
                sel = dir > 0 ? 0 : prev_main; cx = LX[level][sel]; lift = 0;
            }
            fade = v < FI_V0 ? 16 : 16 - (v - FI_V0) * 16 / FI_V;
            if (fade < 0) fade = 0;
        }
        v++;
        if (++tsub >= T_STEP) { tsub = 0; if (++tp > TRANS_FRAMES) { st = M_IDLE; frame = 0; acc = 0; } }
    } else {
        acc += VIDEO_FPS_X2;
        if (acc >= 120) { acc -= 120; frame = (frame + 1) % MENU_FRAMES; queue(menu_bands[frame], 0); }   // loop: next video frame
        if (st == M_SLIDE) {                                        // cursor glides (eased) and hops while it moves
            int f = ++slide_t * 256 / SLIDE; if (f > 256) f = 256;
            int to = LX[level][sel]; cx = slide_from + (((to - slide_from) * smooth(f)) >> 8);
            lift = f * (256 - f) * 6 / 16384;
            if (slide_t >= SLIDE) { st = M_IDLE; cx = to; lift = 0; }
        }
        if (er && sel < 2) go_sel(sel + 1);
        else if (el && sel > 0) go_sel(sel - 1);
        else if (ea) {
            if (level == 0) { prev_main = mode = sel; start_trans(1); }
            else if (sel == 2) start_trans(-1);                     // BACK
            else slot_chosen(mode, sel);
        }
        else if (eb) {
            if (level == 1) start_trans(-1);
            else { music_set_volume(100); return 1; }               // B on the main menu: back to the title
        }
    }
    ui_draw(fade);
    return 0;
}
