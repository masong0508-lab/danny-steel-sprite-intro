#include "hd_skyline_rom.h"
#include "hd_extra_floor_atlas.h"
#include "hd_floor_runs.inc"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t ref[HD_BYTES],actual[HD_BYTES+12];
static void original_skyline(uint8_t *p,int pan,unsigned phase){
 hd_rect(p,0,32,HD_W,22,17);
 hd_rect(p,0,54,HD_W,20,HD_GLASS);
 hd_rect(p,0,74,HD_W,6,HD_GLASS_LIT);
 for(int i=-2;i<19;i++){
  int x=i*23-pan,ht=hd_skyline_material[i+2][0];
  hd_rect(p,x,79-ht,23,ht,HD_STONE_DARK);
  hd_rect(p,x+3,79-ht-2,18,2,HD_ROAD);
  hd_line(p,x-2,79-ht,x+10,73-ht,HD_STONE_DARK);
  hd_line(p,x+10,73-ht,x+25,79-ht,HD_STONE_DARK);
  hd_line(p,x+1,79-ht,x+21,79-ht,HD_STONE);
  if((i&3)==0){
   hd_rect(p,x+14,69-ht,3,10,HD_ROAD_COOL);
   hd_rect(p,x+13,68-ht,6,2,HD_STONE_DARK);
  }
  for(unsigned col=0;col<3u;col++){
   unsigned bits=hd_skyline_material[i+2][col+1u],mask=bits&15u;
   for(int h=7;mask;h+=9,mask>>=1)
    if(mask&1u)hd_rect(p,x+3+(int)col*7,79-ht+h,2,3,
                      phase==(bits>>4)?HD_LAMP:HD_GLASS);
  }
 }
 hd_line(p,0,79,239,79,HD_STONE);
}
static uint32_t hash(unsigned x,unsigned y){
 uint32_t n=(x+31u)*1664525u+(y+79u)*1013904223u;
 n^=n>>13;n*=2246822519u;return n^(n>>11);
}
int main(void){
 unsigned frames=0,road=0,soft=0;
 for(int pan=-52;pan<=52;pan++)for(unsigned phase=0;phase<5u;phase++){
  memset(ref,0xA6,sizeof ref);original_skyline(ref,pan,phase);
  for(unsigned a=0;a<4u;a++){
   memset(actual,0xA6,sizeof actual);uint8_t *p=actual+4+a;
   hd_skyline_draw(p,pan,phase);
   assert(!memcmp(ref,p,HD_BYTES));
   for(unsigned k=0;k<4u+a;k++)assert(actual[k]==0xA6);
   for(unsigned k=4u+a+HD_BYTES;k<sizeof actual;k++)assert(actual[k]==0xA6);
   frames++;
  }
 }
 for(int y=0;y<18;y++)for(int x=0;x<24;x++){
  char t=story_tile(x,y);
  unsigned id=hd_landmark_floor_id(x,y),sid=hd_soft_floor_id(x,y);
  if(!strchr(".#ATg~,=",t)){
   assert(id!=255u&&sid==255u);
   for(unsigned b=0;b<16u;b++)for(unsigned a=0;a<16u;a++){
    unsigned n=hash((unsigned)x*16u+a,(unsigned)y*16u+b);
    unsigned want=(n&31u)==0u?0u:(n&7u)==1u?1u:2u;
    assert(((hd_landmark_floor_word(id,1,b)>>(a*2u))&3u)==want);
    assert(((hd_landmark_floor_word(id,0,a)>>(b*2u))&3u)==want);
    road+=2u;
   }
  }else{
   assert(id==255u);
   if(t=='T'||t=='g'||t=='~'){
    assert(sid!=255u);
    for(unsigned b=0;b<8u;b++)for(unsigned a=0;a<8u;a++){
     unsigned want=(hash((unsigned)x*8u+a,(unsigned)y*8u+b)&3u)==0u;
     assert(((hd_soft_floor_word(sid,1,b)>>a)&1u)==want);
     assert(((hd_soft_floor_word(sid,0,a)>>b)&1u)==want);
     soft+=2u;
    }
   }else assert(sid==255u);
  }
 }
 assert(hd_landmark_floor_id(-1,0)==255u&&hd_soft_floor_id(24,0)==255u);
 printf("Scenery atlas: %u guarded exact panorama crops, %u landmark and %u soft-terrain samples\n",frames,road,soft);
 return 0;
}
