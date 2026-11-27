#pragma once
#include "hd_scene_geometry.h"
#include "hd_wall_shades.inc"
/* One selection per ray. A lossless vertical sampling of the authored art. */
static inline const uint8_t *hd_wall_strip(char type,int tx,int ty,unsigned face,unsigned u,int far){
 unsigned index;
 if(type=='T')index=512u+u;
 else if(story_world_potheads(tx,ty))index=768u+(face*2u+(unsigned)far)*256u+u;
 else index=((face*8u+hd_district(tx,ty,type).variant)*2u+(unsigned)far)*16u+(u>>4);
 return hd_wall_shades[hd_wall_strip_index[index]];
}
static inline const uint8_t *hd_wall_v_samples(unsigned height){
 return hd_wall_scale+hd_wall_scale_offset[height/2u];
}
