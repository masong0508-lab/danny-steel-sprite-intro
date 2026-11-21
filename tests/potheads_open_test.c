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
static void tap(Story *s,unsigned key){
 story_tick(s,0u);story_tick(s,(uint16_t)key);story_tick(s,0u);
}
static void finish_dialogue(Story *s){
 for(unsigned i=0;i<24u&&s->dialogue;i++)tap(s,1u);
 assert(!s->dialogue);
}
static void verify_work_not_wishful_thinking(void){
 Story s;story_new(&s,0);
 assert(s.x==20u&&s.y==11u&&s.facing==0u);
 assert(!story_solid(s.x,s.y));
 assert(story_tile(21,11)=='#');
 int count=0;
 const StoryPage *pages=story_pages(&s,&count);
 assert(count==7);
 assert(strstr(pages[1].line1,"don't love")!=NULL);
 assert(strstr(pages[2].line2,"grateful")!=NULL);
 /* Dialogue frames are authored camera beats, not mandatory delays.
    Even instant A presses show the descent and full-costume dance. */
 static const unsigned anchors[7]={0u,36u,76u,132u,178u,218u,254u};
 for(unsigned page=1;page<7;page++){
  tap(&s,1u);
  assert(s.page==page);
  assert(s.cinema_frame>=anchors[page]);
 }
 finish_dialogue(&s);
 /* B remains a genuine skip for players who want to get to gameplay. */
 Story skip;story_new(&skip,1);
 tap(&skip,2u);
 assert(skip.dialogue==0u&&skip.cinema==CINE_NONE);
 s.x=20;s.y=11;s.prev_x=s.x;s.prev_y=s.y;s.facing=0;
 s.life_stage=STORY_LIFE_WAIT;s.energy=3;
 tap(&s,1u);
 assert(s.activity=='J'&&s.cinema==CINE_OPEN);
 assert(s.work==0u&&s.energy==3u);
 tap(&s,2u); /* B CANCEL */
 assert(!s.activity&&s.work==0u&&s.energy==3u);
 tap(&s,1u);
 assert(s.activity=='J');
 tap(&s,16u); /* RIGHT LEAVE */
 assert(s.activity_choice==1u);
 tap(&s,1u);
 assert(!s.activity&&!s.dialogue&&s.work==0u&&s.energy==3u);
 tap(&s,1u);
 assert(s.activity=='J'&&s.activity_choice==0u);
 tap(&s,1u); /* explicit WORK confirmation */
 assert(s.dialogue!=0u&&s.cinema==CINE_OPEN);
 assert(s.cinema_frame>=136u);
 assert(s.work==2u&&s.energy==2u);
 assert(s.daily_mask&STORY_DAY_SHIFT);
 finish_dialogue(&s);
 tap(&s,1u);
 assert(s.work==2u&&s.energy==2u); /* no same-day work exploit */
 finish_dialogue(&s);
 s.rescue_stage=STORY_RESCUE_MISSING;
 tap(&s,1u);
 assert(s.cinema==CINE_NOTICE);
 assert(s.work==2u&&s.energy==2u); /* rescue takes precedence */
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
 verify_work_not_wishful_thinking();
 puts("Potheads opening: camera, silhouette, animation, grateful dialogue, optional paid shift, bounds and rescue priority OK");
 return 0;
}
