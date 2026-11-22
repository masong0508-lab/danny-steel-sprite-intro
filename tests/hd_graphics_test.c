#include "hd_canvas.h"
#include "menu_ink.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {
 uint8_t guard[64];
 uint8_t pixels[HD_BYTES];
 uint8_t tail[64];
} view;
static void prepare(void){
 memset(&view,0xA5,sizeof(view));
}
static void check(void){
 for(int i=0;i<64;i++)assert(view.guard[i]==0xA5&&view.tail[i]==0xA5);
 for(unsigned i=0;i<HD_BYTES;i++)assert(view.pixels[i]<64u);
}
int main(void){
 Story s;story_new(&s,0);
 for(unsigned page=0;page<2;page++)for(unsigned mode=0;mode<3;mode++)
  for(unsigned selection=0;selection<3;selection++)for(unsigned feedback=0;feedback<3;feedback++){
   prepare();
   hd_menu_draw(view.pixels,page,mode,selection,127,feedback,19,0,1);
   check();
   assert(view.pixels[0]!=0xA5);
   assert(view.pixels[24*HD_W+15]==HD_PAPER);
  }
 for(int x=0;x<STORY_W;x++)for(int y=0;y<STORY_H;y++){
  s.x=(uint8_t)x;s.y=(uint8_t)y;
  if(story_solid(x,y))continue;
  for(unsigned f=0;f<3;f++){
   prepare();hd_world_draw(view.pixels,&s,f*49);
   check();
  }
 }
 s.x=12;s.y=12;s.view=0;s.dialogue=0;
 prepare();hd_world_draw(view.pixels,&s,50);
 check();
 assert(view.pixels[1*HD_W+2]==HD_INK);
 assert(view.pixels[17*HD_W+25]==HD_COPPER);
 unsigned changes=0;
 for(unsigned i=18u*HD_W;i<140u*HD_W;i++)
  if(view.pixels[i]!=HD_ROAD)changes++;
 assert(changes>HD_W*20u); /* world isn't a monochrome checker */
 /* Verify in-place reverse expansion does not overwrite source pixels. */
 prepare();
 for(unsigned y=0;y<80u;y++)for(unsigned x=0;x<120u;x++)
  view.pixels[y*120u+x]=(uint8_t)((x+y*3u)%32u);
 hd_upscale_inplace(view.pixels);
 for(unsigned y=0;y<80u;y++)for(unsigned x=0;x<120u;x++){
  uint8_t expected=(uint8_t)((x+y*3u)%32u);
  assert(view.pixels[(2*y)*HD_W+2*x]==expected);
  assert(view.pixels[(2*y+1)*HD_W+2*x+1]==expected);
 }
 check();
 story_new(&s,0);
 prepare();memset(view.pixels,HD_ROAD,HD_BYTES);
 hd_game_overlay(view.pixels,&s);
 check();
 assert(view.pixels[135*HD_W]==HD_GOLD);
 assert(view.pixels[103*HD_W+8]==HD_ROAD); /* opening leaves full source crop clear */
 s.cinema=CINE_LETTER; /* ordinary conversations retain the larger native panel */
 hd_game_overlay(view.pixels,&s);
 check();
 assert(view.pixels[103*HD_W+8]==HD_COPPER);
 assert(menu_erase_step(0,0,1)==0);
 puts("native 240x160 scenes: bounded drawing, all menus, traversal, overlays and in-place upscale OK");
 return 0;
}

