#pragma once
#include <stdint.h>

/* Quiet photographic intertitles composited into the existing 120x80
   mode-4 video page. No second framebuffer or runtime font allocation. */
#define MENU_INK_W 120u
#define MENU_INK_H 80u
#define MENU_ERASE_HOLD 40u
enum { MENU_NOTE_NONE, MENU_NOTE_EMPTY, MENU_NOTE_ERASED };
/* Pure erase-hold state: requires a fresh A press, never a carried hold. */
unsigned menu_erase_step(unsigned held, int new_press, int down);
void menu_ink_draw(uint8_t *pixels, unsigned level, unsigned mode,
                   unsigned slot, unsigned valid, unsigned held,
                   unsigned feedback);
