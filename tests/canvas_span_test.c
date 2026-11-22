#include "hd_canvas.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Word-wide canvas fills must retain the exact byte contract: clipping,
   unaligned caller buffers, one-pixel spans, and arbitrary palette bytes.
   Padding around every canvas also detects writes into neighboring rows
   or into either allocation guard. Expected output uses byte stores only. */
static uint8_t expected[HD_BYTES+132] __attribute__((aligned(4)));
static uint8_t actual[HD_BYTES+132] __attribute__((aligned(4)));
static void byte_rect(uint8_t *p,int x,int y,int w,int h,unsigned ink){
 for(int yy=y;yy<y+h;yy++){
  if(yy<0||yy>=HD_H)continue;
  for(int xx=x;xx<x+w;xx++)
   if(xx>=0&&xx<HD_W)p[yy*HD_W+xx]=(uint8_t)ink;
 }
}
int main(void){
 const int xs[]={-17,-3,-1,0,1,2,3,4,7,237,238,239,240};
 const int ys[]={-7,-1,0,1,158,159,160};
 const int widths[]={-3,0,1,2,3,4,5,7,8,15,16,17,31,32,33,239,240,241,480};
 const int heights[]={-1,0,1,2,9,160,300};
 unsigned cases=0;
 for(unsigned align=0;align<4u;align++)
  for(unsigned xi=0;xi<sizeof xs/sizeof xs[0];xi++)
   for(unsigned yi=0;yi<sizeof ys/sizeof ys[0];yi++)
    for(unsigned wi=0;wi<sizeof widths/sizeof widths[0];wi++)
     for(unsigned hi=0;hi<sizeof heights/sizeof heights[0];hi++){
      memset(expected,0xA5,sizeof expected);
      memset(actual,0xA5,sizeof actual);
      unsigned ink=cases*37u; /* includes values above255, just like memset */
      byte_rect(expected+64u+align,xs[xi],ys[yi],widths[wi],heights[hi],ink);
      hd_rect(actual+64u+align,xs[xi],ys[yi],widths[wi],heights[hi],ink);
      assert(memcmp(expected,actual,sizeof expected)==0);
      cases++;
     }
 printf("canvas spans: %u exact byte matches across clipping, width and alignment cases\n",cases);
 return 0;
}
