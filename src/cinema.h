#pragma once
#include <stdint.h>
/* A shot-driven, palette-indexed cinematic system; no framebuffer or heap. */
enum {
 CINE_NONE=0, CINE_OPEN, CINE_LETTER, CINE_BORE, CINE_MARA,
 CINE_BUS, CINE_STAY, CINE_MUSIC, CINE_ART, CINE_SERVER,
 CINE_WITNESS, CINE_SAFE, CINE_BACKUP, CINE_NOTICE,
 CINE_CLINIC, CINE_FOLLOWUP, CINE_LAST_CLINIC, CINE_GOOD,
 CINE_FINALE, CINE_DAY, CINE_REBRAND
};
#define CINE_COUNT 21
void cinema_draw(uint8_t *pixels, unsigned scene, unsigned tick,
                 unsigned page, unsigned finale_frame);
