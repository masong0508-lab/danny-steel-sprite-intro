#pragma once
/* Photographic, 120x80 looping video and in-engine exposure transitions.
   The two-column menu logic is NEW GAME / LOAD / ERASE, then
   SLOT 1 / SLOT 2 / BACK. Input is LEFT/RIGHT, A, B; destructive
   erasure additionally requires a displayed uninterrupted hold. */
#include "gba.h"
extern const u16 menu_pal[256];
extern const u8 *const menu_bands[];          /* BIOS LZ77 frames, 8.5 fps */
extern const int MENU_FRAMES;
extern const int ui_tile_words;              /* compact OBJ label/cursor art */
extern const u32 ui_tiles[];
void menu_enter(void);
int menu_update(void);                       /* 0 continue, 1 title, 2/3 new slot, 4/5 load slot */
