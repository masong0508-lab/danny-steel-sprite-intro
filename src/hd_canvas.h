#pragma once
#include <stdint.h>
#include "story.h"

/* Mode-4, one physical GBA pixel per painted pixel, double-buffered.
   The original story logic/8-bit frame remains available as a fallback. */
#define HD_W 240
#define HD_H 160
#define HD_BYTES (HD_W*HD_H)
enum {
 HD_ROAD=32,HD_ROAD_LIGHT,HD_ROAD_COOL,HD_WET,HD_MORTAR,HD_STONE,
 HD_STONE_LIT,HD_STONE_DARK,HD_BRICK_DARK,HD_BRICK,HD_BRICK_LIT,
 HD_PLASTER,HD_PLASTER_LIT,HD_SHADOW,HD_LAMP,HD_GLASS,HD_GLASS_LIT,
 HD_REFLECT,HD_COPPER,HD_LEAF_DARK,HD_LEAF,HD_LEAF_LIT,HD_BARK,
 HD_PAPER,HD_INK,HD_CHALK,HD_RED,HD_RED_SHADE,HD_SKIN,
 HD_SKIN_LIT,HD_HAIR,HD_GOLD
};
extern const uint16_t hd_extra_palette[32];
void hd_rect(uint8_t *p,int x,int y,int w,int h,unsigned c);
void hd_pixel(uint8_t *p,int x,int y,unsigned c);
void hd_line(uint8_t *p,int x,int y,int xx,int yy,unsigned c);
void hd_ellipse(uint8_t *p,int cx,int cy,int rx,int ry,unsigned c);
void hd_text(uint8_t *p,int x,int y,const char *s,unsigned ink,int scale,int track);
int hd_text_width(const char *s,int scale,int track);
void hd_copy_120_to_240(uint8_t *out,const uint8_t *in);
void hd_upscale_inplace(uint8_t *pixels);
void hd_menu_draw(uint8_t *p,unsigned page,unsigned mode,unsigned selection,
                  unsigned frame,unsigned feedback,unsigned erase_hold,
                  unsigned slot0_valid,unsigned slot1_valid);
void hd_world_draw(uint8_t *p,const Story *s,unsigned frame);
void hd_game_overlay(uint8_t *p,const Story *s);
