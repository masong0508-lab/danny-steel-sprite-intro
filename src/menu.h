#pragma once
// Main menu: a looping 120x80 video (shown 2x by the affine mode-4 background), LZ77-streamed at 8.5 fps.
// The loop crossfade is baked into the data (tools/make_menu.py), so playback simply wraps.
#include "gba.h"
extern const u16 menu_pal[256];
extern const u8 *const menu_bands[];          // one BIOS-LZ77 stream per frame
extern const int MENU_FRAMES;
void menu_enter(void);                        // call after the title's fade-out; fades the picture in and the music down to 40%
int  menu_update(void);                       // call once per vblank; returns 1 when the player backs out (B) -> caller rebuilds the title
