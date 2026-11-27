#include "hd_stone_rain.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint8_t a[HD_BYTES+16],b[HD_BYTES+16];
static uint32_t hash(unsigned x,unsigned y){
 uint32_t n=(x+31u)*1664525u+(y+79u)*1013904223u;
 n^=n>>13;n*=2246822519u;return n^(n>>11);
}
static unsigned ref_stone(unsigned x,unsigned y){
 if(x<8u||y<7u)return HD_MORTAR;
 if(x>244u||y>244u)return HD_STONE_DARK;
 return ((x>>5)^(y>>6))&1u?HD_STONE_LIT:HD_STONE;
}
static void compare(unsigned t){
 memset(a,0xA6,sizeof a);memset(b,0xA6,sizeof b);
 memset(a+8,HD_ROAD,HD_BYTES);memset(b+8,HD_ROAD,HD_BYTES);
 for(unsigned i=0;i<24u;i++){
  uint32_t v=hash(i+31u,i*7u+41u);
  int x=(int)((v+t*2u)%252u)-8;
  int y=20+(int)(((v>>9)+t*3u)%116u);
  hd_line(a+8,x,y,x-2,y+4+(int)(i&3u),
          (i&3u)==0u?HD_STONE_LIT:HD_GLASS_LIT);
 }
 hd_rain_draw(b+8,t);
 assert(!memcmp(a,b,sizeof a));
 for(unsigned i=0;i<8u;i++)assert(b[i]==0xA6&&b[HD_BYTES+8+i]==0xA6);
}
int main(void){
 unsigned count=0;
 for(unsigned y=0;y<256u;y++)for(unsigned x=0;x<256u;x++){
  unsigned c=ref_stone(x,y);
  assert(hd_stone_strip(1,y)[x]==c&&hd_stone_strip(0,x)[y]==c);
 }
 for(unsigned t=0;t<5000u;t+=7u){compare(t);count++;}
 for(unsigned i=0;i<256u;i++){
  compare(UINT32_MAX-i);compare(0x7fffffffu+i);compare(0x55555555u+i);count+=3u;
 }
 printf("Flagstone: 131072 exact shade samples. Rain: %u guarded full frames including unsigned wraps\n",count);
 return 0;
}
