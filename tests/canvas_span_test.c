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
static void byte_pixel(uint8_t *p,int x,int y,unsigned ink){
 if(x>=0&&x<HD_W&&y>=0&&y<HD_H)p[y*HD_W+x]=(uint8_t)ink;
}
static int abs_i(int n){return n<0?-n:n;}
static void byte_line(uint8_t *p,int x,int y,int xx,int yy,unsigned ink){
 int dx=abs_i(xx-x),dy=abs_i(yy-y),sx=x<xx?1:-1,sy=y<yy?1:-1,err=dx-dy;
 for(;;){
  byte_pixel(p,x,y,ink);
  if(x==xx&&y==yy)break;
  int twice=2*err;
  if(twice>-dy){err-=dy;x+=sx;}
  if(twice<dx){err+=dx;y+=sy;}
 }
}
static void byte_ellipse(uint8_t *p,int cx,int cy,int rx,int ry,unsigned ink){
 if(rx<1||ry<1)return;
 int rx2=rx*rx,ry2=ry*ry;
 for(int y=-ry;y<=ry;y++){
  int w=0,n=rx2*(ry2-y*y);
  while((w+1)*(w+1)*ry2<=n)w++;
  byte_rect(p,cx-w,cy+y,w*2+1,1,ink);
 }
}
static uint32_t rng=0x923d98bu;
static int random_coord(unsigned limit,int offset){
 rng=rng*1664525u+1013904223u;return (int)((rng>>8)%limit)+offset;
}
static void verify_lines_ellipses(void){
 for(unsigned i=0;i<18000u;i++){
  unsigned alignment=i&3u,ink=i*31u;
  int x=random_coord(400u,-80),y=random_coord(320u,-80);
  int xx=random_coord(400u,-80),yy=random_coord(320u,-80);
  if(i%3u==0u){xx=x+random_coord(300u,-150);yy=y+random_coord(30u,-15);}
  if(i%3u==1u){xx=x+random_coord(30u,-15);yy=y+random_coord(300u,-150);}
  memset(expected,0xA5,sizeof expected);memset(actual,0xA5,sizeof actual);
  byte_line(expected+64u+alignment,x,y,xx,yy,ink);
  hd_line(actual+64u+alignment,x,y,xx,yy,ink);
  assert(!memcmp(expected,actual,sizeof expected));
 }
 for(unsigned i=0;i<4000u;i++){
  unsigned alignment=i&3u,ink=i*59u;
  int x=random_coord(340u,-50),y=random_coord(260u,-50);
  int rx=random_coord(51u,0),ry=random_coord(51u,0);
  memset(expected,0xA5,sizeof expected);memset(actual,0xA5,sizeof actual);
  byte_ellipse(expected+64u+alignment,x,y,rx,ry,ink);
  hd_ellipse(actual+64u+alignment,x,y,rx,ry,ink);
  assert(!memcmp(expected,actual,sizeof expected));
 }
 puts("22000 exact line and ellipse raster matches, all octants, crop and alignment guards");
}
int main(void){
 verify_lines_ellipses();
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
