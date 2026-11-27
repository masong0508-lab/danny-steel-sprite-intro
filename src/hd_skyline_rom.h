#pragma once
#include "hd_canvas.h"
#include "hd_skyline_rom.inc"
#include "hd_skyline_dma.inc"
#include "hd_render_copy.h"
enum {HD_SKY_WIDTH=344,HD_SKY_ROWS=48,HD_SKY_TOP=32};
/* Every source crop is word aligned, even during sub-tile panning.
   One small ROM bank per byte alignment avoids shift/merge loops in
   IWRAM. The host and unaligned destination use the same bounded copy. */
static void hd_skyline_blit(uint8_t *p,int pan){
 unsigned offset=(unsigned)(pan+52),shift=offset&3u;
 const uint8_t *bank=shift?hd_sky_shifted[shift-1u]:hd_sky_pixels;
 const uint8_t *start=bank+(offset&~3u);
 for(unsigned y=0;y<HD_SKY_ROWS;y++)
  hd_render_copy(p+(HD_SKY_TOP+y)*HD_W,start+y*HD_SKY_WIDTH,HD_W);
}
static inline void hd_skyline_draw(uint8_t *p,int pan,unsigned phase){
 if(pan< -52||pan>52)return;
 hd_skyline_blit(p,pan);
 phase%=5u;
 for(unsigned i=hd_sky_light_offset[phase];i<hd_sky_light_offset[phase+1u];i++){
  const HdSkyLight *l=&hd_sky_lights[i];
  int x=(int)l->x-52-pan,end=x+(int)l->width;
  if(x<0)x=0;
  if(end>HD_W)end=HD_W;
  uint8_t *row=p+(HD_SKY_TOP+l->row)*HD_W;
  for(;x<end;x++)row[x]=HD_LAMP;
 }
}
