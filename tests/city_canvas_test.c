#include "hd_city_canvas.h"
#include "hd_guide_map.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {uint8_t before[32],pixels[HD_BYTES],after[32];} f;
static uint8_t prev[HD_BYTES];
static void reset(void){memset(&f,0xa5,sizeof f);memset(f.pixels,HD_INK,HD_BYTES);}
static void bounds(int x,int y,int w,int h){
 for(unsigned i=0;i<32u;i++)assert(f.before[i]==0xa5&&f.after[i]==0xa5);
 for(int yy=0;yy<HD_H;yy++)for(int xx=0;xx<HD_W;xx++){
  unsigned v=f.pixels[yy*HD_W+xx];assert(v<64u);
  if(xx<x||xx>=x+w||yy<y||yy>=y+h)assert(v==HD_INK);
 }
}
int main(void){
 static const int sizes[3][2]={{104,64},{53,38},{41,26}};
 static const unsigned masks[]={0x0f,0xf0,0x66,0x99};
 unsigned checks=0;
 for(unsigned scale=0;scale<3u;scale++)for(unsigned warm=0;warm<2u;warm++)
  for(unsigned phase=0;phase<4u;phase++){
   int x=12,y=22,w=sizes[scale][0],h=sizes[scale][1];
   reset();hd_city_painting(f.pixels,x,y,w,h,phase,warm,0u);bounds(x,y,w,h);
   memcpy(prev,f.pixels,HD_BYTES);
   for(unsigned k=0;k<8u;k++)if(masks[phase]&(1u<<k)){
    reset();hd_city_painting(f.pixels,x,y,w,h,phase,warm,1u<<k);bounds(x,y,w,h);
    assert(memcmp(prev,f.pixels,HD_BYTES)!=0);
    for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++){
     int l=x+(int)(k&3u)*16*w/64,t=y+(int)(k>>2)*20*h/40;
     int rr=x+((int)(k&3u)+1)*16*w/64,bb=y+((int)(k>>2)+1)*20*h/40;
     if(xx<l||xx>=rr||yy<t||yy>=bb)assert(prev[yy*HD_W+xx]==f.pixels[yy*HD_W+xx]);
    }
    checks++;
   }
   reset();hd_city_painting(f.pixels,x,y,w,h,phase,warm,masks[phase]);
   memcpy(prev,f.pixels,HD_BYTES);reset();hd_city_painting(f.pixels,x,y,w,h,phase+1u,warm,0u);
   assert(!memcmp(prev,f.pixels,HD_BYTES));checks++;
  }
 Story s;story_new(&s,0);s.dialogue=0;s.cinema=CINE_NONE;
 for(unsigned v=0;v<2u;v++){
  s.view=(uint8_t)v;Story snapshot=s;reset();hd_guide_map(f.pixels,&s,144,50);
  bounds(142,48,STORY_W+4,STORY_H+4);assert(!memcmp(&s,&snapshot,sizeof s));
  int gx,gy;const char *label;assert(story_goal(&s,&gx,&gy,&label));
  assert(f.pixels[(50+gy)*HD_W+144+gx]==HD_GOLD);
  assert(f.pixels[(50+s.y)*HD_W+144+s.x]==HD_PAPER);
  checks++;
 }
 printf("Shared painting and guide: %u raster/commit/clip cases, exact progress, three real scales and read-only map\n",checks);
 return 0;
}
