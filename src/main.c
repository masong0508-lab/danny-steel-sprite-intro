// THE LAST DAYS of danny steel - title screen (placeholder logic)
//   * drifting, digitised painting as background (streamed from ROM)
//   * grunge title + blinking PRESS START
//   * START -> fade out -> the title screen restarts (music too). Replace title_finished() later.
#include "gba.h"
#include "data.h"
#include "bgstream.h"
#include "music.h"
#include "logo.h"
#include "intro.h"
#include "menu.h"
#include "play.h"

#define TITLE_ROW 1                       // map rows for the text layers
#define PRESS_ROW 16

enum { ST_FADE_IN, ST_RUN, ST_FADE_OUT };
static int in_menu, in_game, state, state_t, frames, was_down, keep_music, white_in, city_t;   // keep_music: the intro already started the song
static u32 ph_x, ph_y;                    // scroll phases, 16.16 turns
static s16 sin_lut[256];                  // Q12

static void build_sin(void) {             // integer sine (Bhaskara approximation), no libm
    for (int a = 0; a < 256; a++) {
        int h = a & 127, p = h * (128 - h), v = 65536 * p / (81920 - 4 * p);
        sin_lut[a] = (s16)(a >= 128 ? -v : v);
    }
}
static int sin12(u32 ph) {
    int i = (ph >> 8) & 255, f = ph & 255, a = sin_lut[i], b = sin_lut[(i + 1) & 255];
    return a + (((b - a) * f) >> 8);
}
static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }
/* Filmic exposure curves avoid abrupt 0.4-second linear brightness ramps. */
static int exposure_ease(int elapsed, int length) {
    int f = clampi(elapsed * 256 / length,0,256);
    int smooth = ((f * f >> 8) * (768 - 2 * f)) >> 8;
    return smooth * 16 >> 8;
}
static u16 warm_mix(u16 light,u16 shadow,int amount) {
    int r=(light&31),g=(light>>5)&31,b=(light>>10)&31;
    return (u16)((r+(((shadow&31)-r)*amount)/16) |
          ((g+((((shadow>>5)&31)-g)*amount)/16)<<5) |
          ((b+((((shadow>>10)&31)-b)*amount)/16)<<10));
}
static void title_prompt_breathe(int tick) {
    /* Only BG1's bank 13 is modulated; the distressed title in bank 14
       remains steady. Slower, always-readable breathing replaces blinking. */
    int phase=tick%144;
    if(phase>72)phase=144-phase;
    int dark=7-phase*6/72;
    PAL16[209]=text_pal[1];
    PAL16[210]=warm_mix(text_pal[2],0x294a,dark);
    PAL16[211]=warm_mix(text_pal[3],0x294a,dark/2);
}
static void scroll_pos(int *sx, int *sy) {
    *sx = clampi(BGS_MAX_X / 2 + ((BGS_MAX_X / 2 * sin12(ph_x)) >> 12), 0, BGS_MAX_X);
    *sy = clampi(BGS_MAX_Y / 2 + ((BGS_MAX_Y / 2 * sin12(ph_y)) >> 12), 0, BGS_MAX_Y);
}

// Build the whole title screen from scratch (used at boot and every time we "loop back").
static void title_setup(void) {
    REG_BLDCNT = keep_music ? 0x00BF : 0x00FF; REG_BLDY = 16; REG_DISPCNT = 0;   // no forced blank (it shows white): white after the eyes' flash, black otherwise
    if (!keep_music) music_stop();
    for (int i = 0; i < 224; i++) PAL16[i] = bg_pal[i];
    for (int i = 0; i < 16; i++) {
        PAL16[224 + i] = text_pal[i];      // bank 14: steady hand-distressed title
        PAL16[208 + i] = text_pal[i];      // bank 13: slowly breathing invitation
    }
    volatile u32 *tt = (volatile u32 *)(VRAM8 + 3 * 0x4000);              // charblock 3 = text tiles
    for (int i = 0; i < text_tile_count * 8; i++) tt[i] = text_tiles[i];
    volatile u16 *m0 = (volatile u16 *)(VRAM8 + 30 * 0x800), *m1 = (volatile u16 *)(VRAM8 + 31 * 0x800);
    for (int i = 0; i < 1024; i++) { m0[i] = 0; m1[i] = 0; }
    for (int r = 0; r < TITLE_TH; r++) for (int c = 0; c < TITLE_TW; c++) m0[(TITLE_ROW + r) * 32 + c] = title_map[r * TITLE_TW + c];
    for (int r = 0; r < PRESS_TH; r++) for (int c = 0; c < PRESS_TW; c++) {
        u16 tile=press_map[r * PRESS_TW + c];
        m1[(PRESS_ROW + r) * 32 + c]= tile ? (u16)((tile & 0x0FFF) | (13 << 12)) : 0;
    }
    ph_x = ph_y = 0; frames = 0; was_down = 1;
    int sx, sy; scroll_pos(&sx, &sy);
    bgs_init(sx, sy);
    REG_BGCNT(0) = BGCNT(0, 3, 30);                                       // title text
    REG_BGCNT(1) = BGCNT(0, 3, 31);                                       // press start
    REG_BGCNT(2) = BGCNT(3, 0, BGS_SB) | BGCNT_8BPP;                      // painting
    REG_BGHOFS(0) = REG_BGVOFS(0) = REG_BGHOFS(1) = REG_BGVOFS(1) = 0;
    REG_BGHOFS(2) = sx & 255; REG_BGVOFS(2) = sy & 255;
    white_in = keep_music;                                                // coming from the eyes' white flash -> fade in from white
    REG_BLDCNT = white_in ? 0x00BF : 0x00FF;
    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | OBJ_ON;              // OBJ = the intro's city, kept under the title text
    if (!keep_music) music_start();
    keep_music = 0;
    state = ST_FADE_IN; state_t = 0;
}

/* The title belongs to the chapter's physical painting, not a debug splash. */
static void title_finished(void) { in_menu = 1; menu_enter(); }

static void frame_update(void) {
    frames++; intro_title_tick(city_t++);                                 // skyline keeps turning under the title
    ph_x += 22; ph_y += 33;                                               // ~50 s and ~33 s per loop
    int sx, sy; scroll_pos(&sx, &sy);
    bgs_update(sx, sy);
    REG_BGHOFS(2) = sx & 255; REG_BGVOFS(2) = sy & 255;

    int down = !(REG_KEYINPUT & KEY_START);
    int pressed = down && !was_down; was_down = down;
    title_prompt_breathe(frames);
    switch (state) {
    case ST_FADE_IN:
        { int hold = white_in ? 4 : 0, len = white_in ? 22 : 24;            // reference: ~4 frames of pure white, then a ~0.37 s fade down to the title
            REG_BLDY = (u16)(16 - exposure_ease(state_t-hold,len));
            if (++state_t >= hold + len) { state = ST_RUN; state_t = 0; REG_BLDCNT = 0x00FF; } }
        break;
    case ST_RUN:
        REG_BLDY = 0;
        if (pressed) { state = ST_FADE_OUT; state_t = 0; }
        break;
    default:                                                              // ST_FADE_OUT
        REG_BLDY = (u16)exposure_ease(state_t,27);
        if (++state_t >= 27) { title_finished(); return; }
    }
    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | OBJ_ON;
}

int main(void) {
    logo_play();                                                          // boot logo (~8 s)
    build_sin();
    music_init();
    intro_play();                                                         // sprite-only intro (~13 s, song starts with it)
    keep_music = 1;
    title_setup();
    int in_vblank = 0;
    for (;;) {
        music_poll();                                                     // keeps the song on time
        if (REG_VCOUNT >= 160) {
            if (!in_vblank) {
                in_vblank = 1;
                if (in_menu) {
                    int action = menu_update();
                    if (action == 1) { in_menu = 0; intro_reload_title(); title_setup(); }
                    else if (action >= 2) { in_menu = 0; in_game = 1; play_enter(action >= 4, (action - 2) & 1); }
                } else if (in_game) {
                    if (play_update()) { in_game = 0; in_menu = 1; menu_enter(); }
                } else frame_update();
            }
        }
        else in_vblank = 0;
    }
}
