#pragma once
// Main menu (NEW GAME / LOAD / ERASE -> SLOT 1 / SLOT 2 / BACK, zoom-through transition between them): a looping 120x80 video (shown 2x by the affine mode-4 background), LZ77-streamed at 8.5 fps.
// The loop crossfade is baked into the data (tools/make_menu.py), so playback simply wraps.
#include "gba.h"
extern const u16 menu_pal[256];
extern const u8 *const menu_bands[];          // one BIOS-LZ77 stream per frame
extern const int MENU_FRAMES;
extern const u16 trans_pal[256];                // option-select transition (tools/make_menu_trans.py): its own palette + frames
extern const u8 *const trans_bands[];
extern const int TRANS_FRAMES;
extern const int ui_tile_words;                 // label + cursor sprite tiles (tools/make_menu_ui.py)
extern const u32 ui_tiles[];
void menu_enter(void);                        // call after the title's fade-out; fades the picture in and the music down to 40%
int  menu_update(void);                       // call once per vblank; returns 1 when the player backs out (B) -> caller rebuilds the title
