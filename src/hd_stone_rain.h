#pragma once
#include "hd_canvas.h"
#include "hd_stone_rain.inc"
/* Cardinal flagstone rows have only four fixed-coordinate states. */
static inline const uint8_t *hd_stone_strip(int along_x,unsigned fixed){
 unsigned state=fixed<(along_x?7u:8u)?0u:fixed>244u?1u:
                ((fixed>>(along_x?6u:5u))&1u)?2u:3u;
 return hd_stone_strips[(unsigned)along_x*4u+state];
}
/* Modular sum with the original unsigned 32-bit addition wrap. */
static inline unsigned hd_rain_mod(unsigned clockmod,unsigned seedmod,
                                   uint32_t clock,uint32_t seed,
                                   unsigned modulus,unsigned wrap){
 unsigned n=clockmod+seedmod;
 if(clock>UINT32_MAX-seed){
  if(n<wrap)n+=modulus;
  n-=wrap;
 }
 if(n>=modulus)n-=modulus;
 return n;
}
/* Identical short diagonal strokes, with no generic line calls and only
   two modulo operations for the whole weather layer. */
static void hd_rain_draw(uint8_t *p,unsigned tick){
 uint32_t tx=(uint32_t)tick*2u,ty=(uint32_t)tick*3u;
 unsigned xm=tx%252u,ym=ty%116u;
 for(unsigned i=0;i<24u;i++){
  const HdRainSeed *s=&hd_rain_seeds[i];
  int x=(int)hd_rain_mod(xm,s->xmod,tx,s->seed,252u,4u)-8;
  int y=20+(int)hd_rain_mod(ym,s->ymod,ty,s->seed>>9,116u,16u);
  unsigned c=(i&3u)==0u?HD_STONE_LIT:HD_GLASS_LIT;
  unsigned len=4u+(i&3u);
  for(unsigned k=0;k<=len;k++){
   int xx=x-hd_rain_dx[i&3u][k],yy=y+(int)k;
   if((unsigned)xx<HD_W&&(unsigned)yy<HD_H)
    p[yy*HD_W+xx]=(uint8_t)c;
  }
 }
}
