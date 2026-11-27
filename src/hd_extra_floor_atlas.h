#pragma once
#include "hd_canvas.h"
#include "hd_extra_floor_atlas.inc"
/* Out-of-town samples retain the original solid-border material. */
static inline unsigned hd_landmark_floor_id(int tx,int ty){
 return (unsigned)tx<24u&&(unsigned)ty<18u?hd_landmark_floor_index[ty*24+tx]:255u;
}
static inline unsigned hd_soft_floor_id(int tx,int ty){
 return (unsigned)tx<24u&&(unsigned)ty<18u?hd_soft_floor_index[ty*24+tx]:255u;
}
static inline uint32_t hd_landmark_floor_word(unsigned id,int along_x,unsigned sub){
 return along_x?hd_landmark_floor_rows[id*16u+sub]:hd_landmark_floor_cols[id*16u+sub];
}
static inline unsigned hd_soft_floor_word(unsigned id,int along_x,unsigned sub){
 return along_x?hd_soft_floor_rows[id*8u+sub]:hd_soft_floor_cols[id*8u+sub];
}
