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

static u32 buf[MW * MH / 4] __attribute__((section(".ewram"), aligned(4)));   // decoded frame (EWRAM; falls back to normal RAM if no section)
#define VIDEO_FPS_X2 17                           /* 8.5 fps, in half-frames per second (display = 120 half-frames/s) */
static int t, frame, page, was_b, acc;

static void lz77(const void *src, void *dst) {                     // BIOS LZ77UnCompWram (SWI 0x11)
    register const void *r0 __asm__("r0") = src; register void *r1 __asm__("r1") = dst;
    __asm__ volatile("swi 0x11" : "+r"(r0), "+r"(r1) :: "r2", "r3", "memory");
}
static void draw(int f, int pg) {                                  // decode frame f, copy its 80 rows into page pg (mode-4 stride = 240)
    lz77(menu_bands[f], buf);
    volatile u32 *v = (volatile u32 *)(VRAM8 + (pg ? 0xA000 : 0));
    for (int y = 0; y < MH; y++) for (int x = 0; x < MW / 4; x++) v[y * 60 + x] = buf[y * (MW / 4) + x];
}
void menu_enter(void) {
    REG_DISPCNT = DCNT_BLANK; REG_BLDCNT = 0x00FF; REG_BLDY = 16;
    for (int i = 0; i < 256; i++) PAL16[i] = menu_pal[i];
    REG_BG2PA = 128; REG_BG2PD = 128; REG_BG2PB = 0; REG_BG2PC = 0; REG_BG2X = 0; REG_BG2Y = 0;   // 0.5 texel per pixel = 2x zoom
    REG_BGCNT(2) = 0;                                               // priority 0, nothing else needed in bitmap mode
    frame = 0; t = 0; page = 0; was_b = 1; acc = 0;
    draw(0, 0);
    REG_DISPCNT = 4 | DCNT_BG2;                                     // mode 4, BG2 only (OBJ off), page 0 shown
}
int menu_update(void) {
    if (t < FADE_IN) REG_BLDY = 16 - t * 16 / FADE_IN; else REG_BLDY = 0;
    if (t < VOL_FRAMES) music_set_volume(100 - (100 - MUSIC_PCT) * t / VOL_FRAMES); else music_set_volume(MUSIC_PCT);
    t++;
    acc += VIDEO_FPS_X2;
    if (acc >= 120) {                                               // new video frame every ~7.06 vblanks: draw into the hidden page, then flip
        acc -= 120;
        frame = (frame + 1) % MENU_FRAMES; page ^= 1;
        draw(frame, page);
        REG_DISPCNT = 4 | DCNT_BG2 | (page ? 0x10 : 0);
    }
    int b = !(REG_KEYINPUT & KEY_B), pressed = b && !was_b; was_b = b;
    if (pressed) { music_set_volume(100); return 1; }
    return 0;
}
