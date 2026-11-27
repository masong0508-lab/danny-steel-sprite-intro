#include "hd_canvas.h"
#include "hd_floor_runs.inc"
#include <assert.h>
#include <stdio.h>
static uint32_t ref_texel(unsigned x,unsigned y){
 uint32_t v=(x+31u)*1664525u+(y+79u)*1013904223u;
 v^=v>>13;v*=2246822519u;return v^(v>>11);
}
int main(void){
 unsigned checks=0,windows=0,dither=0,coverage=0;
 for(int negative=0;negative<2;negative++)
  for(int y=80;y<140;y++){
   int dist=13500/(y-75);if(dist>2304)dist=2304;
   int unit=dist*768/480,step=y<98?2:1,sign=negative?-1:1;
   for(int phase=-4;phase<=4;phase++)for(int x=0;x<240;x+=step){
    int origin=phase*64;
    int old=origin+sign*(((x-120)*unit)>>8);
    int end=x+step;
    while(end<240&&((origin+sign*(((end-120)*unit)>>8))>>4)==(old>>4))end+=step;
    assert(hd_floor_run_next[negative][y-80][x]==end);checks++;
   }
  }
 for(unsigned far=0;far<2u;far++)for(unsigned n=0;n<400u;n++){
  unsigned k=0,step=far?2u:1u;
  while((n+k)%17u)k+=step;
  assert(hd_floor_dither_skip[far][n]==k);dither++;
 }
 for(int i=-2;i<19;i++){
  int ht=17+(int)(ref_texel((unsigned)(i+33),23u)%22u);
  assert(hd_skyline_material[i+2][0]==ht);
  for(int col=0;col<3;col++){
   int w=3+col*7;unsigned bits=hd_skyline_material[i+2][col+1];
   for(int j=0;j<4;j++){
    int h=7+j*9;
    unsigned lit=(h<ht-2)&&((ref_texel((unsigned)(i+41+w),(unsigned)(h+35))&3u)!=0u);
    assert(((bits>>j)&1u)==lit);windows++;
   }
   for(unsigned phase=0;phase<5u;phase++)
    assert((phase==(bits>>4))==((phase+(unsigned)(i+w))%5u==0u));
  }
 }
 /* Wall paint and floor paint cover complementary regions for every
    possible original hit depth, so the lower sky clear is dead work. */
 for(unsigned depth=32;depth<=2304;depth+=32){
  unsigned d=depth>2300?2300:depth,h=20800/(d+60u);
  if(h>150u)h=150u;
  for(unsigned y=80;y<140;y++){
   int wall=y<79u+h/2u;
   int floor=(int)d>(int)(10400u/(y-78u))-60;
   assert(wall!=floor);coverage++;
  }
 }
 printf("Floor runs: %u exact camera-phase spans, %u dither offsets, %u skyline cells, %u complementary wall/floor rows\n",checks,dither,windows,coverage);
 return 0;
}
