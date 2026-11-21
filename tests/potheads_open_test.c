#include "hd_canvas.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Draw the production opening into a guarded native-resolution screen.
   All eight dance beats are tested, not just the poster frame. */
static struct {
 uint8_t before[64];
 uint8_t pixels[HD_BYTES];
 uint8_t after[64];
} frame;
static unsigned paint(unsigned t,unsigned page,int overlay){
 Story s;
 story_new(&s,0);
 assert(s.cinema==CINE_OPEN);
 s.cinema_frame=(uint16_t)t;
 s.page=(uint8_t)page;
 memset(&frame,0xB7,sizeof frame);
 hd_cinema_draw(frame.pixels,&s,t);
 if(overlay)hd_game_overlay(frame.pixels,&s);
 unsigned checksum=2166136261u,colors=0u,paper=0u,gold=0u;
 for(unsigned i=0;i<64u;i++)
  assert(frame.before[i]==0xB7&&frame.after[i]==0xB7);
 for(unsigned y=0;y<HD_H;y++)for(unsigned x=0;x<HD_W;x++){
  unsigned pixel=frame.pixels[y*HD_W+x];
  assert(pixel<64u);
  checksum=(checksum^pixel)*16777619u;
  if(x>=80u&&x<214u&&y>=20u&&y<103u){
   colors|=1u<<(pixel&31u);
   if(pixel==HD_PAPER)paper++;
   if(pixel==HD_GOLD)gold++;
  }
 }
 assert(colors!=0u);
 if(t>=144u){assert(paper>55u);assert(gold>35u);}
 return checksum;
}
int main(void){
 assert(paint(0u,0u,1)!=paint(54u,0u,1));
 assert(paint(54u,0u,1)!=paint(132u,0u,1));
 assert(paint(145u,0u,1)!=paint(181u,0u,1));
 for(unsigned pose=0;pose<8;pose++){
  unsigned t=156u+pose*12u;
  assert(paint(t,1u,1)!=paint(t+12u,1u,1));
 }
 assert(paint(180u,1u,0)!=paint(180u,1u,1));
 assert(paint(320u,3u,1)==paint(320u,3u,1));
 puts("Potheads opening: descending camera, connected pose cycle, paper sign, dialog visibility, bounds and palette OK");
 return 0;
}
