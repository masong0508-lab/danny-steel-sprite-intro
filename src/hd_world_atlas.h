#pragma once
#include "hd_canvas.h"
/* Authored native-resolution ROM-backed Old Quarter pixel artwork.
   64 is a TRANSPARENT sprite-key only; palette indices stay 32..63.
   No decoded textures, heap, or additional EWRAM framebuffer. */
void hd_world_tile_art(uint8_t *frame,int sx,int sy,char tile,int wx,int wy);
/* 0 is the established idle cel; 2..5 are the four gait cels. The
   committed movement phase selects the cel, not the presentation clock.
   Alternating destination parity changes the leading leg on each step. */
static inline unsigned hd_world_danny_gait(const Story *s){
 if(!s->anim)return 0u;
 unsigned phase=s->anim>=4u?0u:4u-s->anim;
 return 2u+((phase+(((unsigned)s->x+(unsigned)s->y)&1u)*2u)&3u);
}
HD_CANVAS_CODE void hd_world_person_art(uint8_t *dst,int center_x,int foot_y,
                         unsigned role,unsigned facing,unsigned gait);
HD_CANVAS_CODE void hd_world_tree_art(uint8_t *dst,int center_x,int foot_y,
                       unsigned kind,unsigned phase);
/* Nearest-neighbor projected actor preserves the exact same hair,
   silhouette and clothing in first-person. No heap or scaling buffer. */
void hd_world_person_projected(uint8_t *dst,int center_x,int foot_y,int height,
                               unsigned role,unsigned facing,unsigned gait);

