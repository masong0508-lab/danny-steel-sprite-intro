#pragma once
/* Three original 240x160 backgrounds + three original 80x84 figures live
   solely in cartridge ROM. Index values 32..63 are existing GBA colors.
   Character 'at sign' 64 marks transparent sprite pixels only. */
#define ARENA_ART_W 240
#define ARENA_ART_H 160
#define ARENA_FIG_W 80
#define ARENA_FIG_H 84
extern const unsigned char *const hd_arena_backdrops[3];
extern const unsigned char *const hd_arena_enemies[3];
