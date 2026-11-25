#pragma once
#include "hd_canvas.h"
/* 0..3 wet road, 4..5 stone, 6..7 grass, 8..9 canal,
   10..11 shingled roof, 12..13 walkway. ROM only. */
HD_CANVAS_CODE void hd_iso_tile(uint8_t *p,int px,int py,unsigned material);
/* 14..17 quiet asphalt, 18..19 flagstones, 20..23 slate/metal roofs,
   24..25 flat civic/industrial roofs. Same diamond and clipped copier. */
HD_CANVAS_CODE void hd_iso_tile_detail(uint8_t *p,int px,int py,unsigned material);

