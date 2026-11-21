#pragma once
#include "hd_canvas.h"
/* Authored native-resolution ROM-backed Old Quarter pixel artwork.
   64 is a TRANSPARENT sprite-key only; palette indices stay 32..63.
   No decoded textures, heap, or additional EWRAM framebuffer. */
void hd_world_tile_art(uint8_t *frame,int sx,int sy,char tile,int wx,int wy);
void hd_world_person_art(uint8_t *dst,int center_x,int foot_y,
                         unsigned role,unsigned facing,unsigned gait);
void hd_world_tree_art(uint8_t *dst,int center_x,int foot_y,
                       unsigned kind,unsigned phase);
