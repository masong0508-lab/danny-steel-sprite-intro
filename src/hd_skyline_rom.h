#pragma once
#include "hd_canvas.h"
#include "hd_skyline_rom.inc"
enum {HD_SKY_WIDTH=344,HD_SKY_ROWS=48,HD_SKY_TOP=32};
typedef uint32_t HdSkyWord __attribute__((may_alias));
/* Exact crop for any pan in [-52,52], including unaligned source
   offsets. Real frame is word-aligned. The portable fallback lets
   guarded host tests exercise every destination alignment too. */
static HD_CANVAS_CODE __attribute__((noinline)) void hd_skyline_blit(uint8_t *p,int pan){
 unsigned offset=(unsigned)(pan+52);
 unsigned shift=(offset&3u)*8u;
 const uint8_t *start=hd_sky_pixels+(offset&~3u);
 for(unsigned y=0;y<HD_SKY_ROWS;y++){
  uint8_t *dst=p+(HD_SKY_TOP+y)*HD_W;
  const uint8_t *src=start+y*HD_SKY_WIDTH;
  if((uintptr_t)dst&3u){
   for(unsigned x=0;x<HD_W;x++)dst[x]=src[x+(offset&3u)];
   continue;
  }
  const HdSkyWord *in=(const HdSkyWord *)(const void *)src;
  HdSkyWord *out=(HdSkyWord *)(void *)dst;
  if(!shift){
   for(unsigned x=0;x<HD_W/4u;x+=4u){
    out[x]=in[x];out[x+1u]=in[x+1u];out[x+2u]=in[x+2u];out[x+3u]=in[x+3u];
   }
  }else{
   uint32_t lo=in[0];
   for(unsigned x=0;x<HD_W/4u;x++){
    uint32_t hi=in[x+1u];
    out[x]=(lo>>shift)|(hi<<(32u-shift));lo=hi;
   }
  }
 }
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
