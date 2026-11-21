#include "story.h"
#include "story_draw.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
/* Render every shot, dialogue camera beat, dramatic finale moment and viewport
   edge in an instrumentable host build; no off-screen or palette writes. */
static struct {
    uint8_t guard[32];
    uint8_t pixels[120*80];
    uint8_t tail[32];
} frame;
static uint8_t last[120*80];
static void verify(void){
 for(int i=0;i<32;i++)assert(frame.guard[i]==0xA5&&frame.tail[i]==0xA5);
 for(int i=0;i<120*80;i++)assert(frame.pixels[i]<32);
}
static void press(Story *s,unsigned key){story_tick(s,0);story_tick(s,(uint16_t)key);story_tick(s,0);}
static void close_script(Story *s){for(int i=0;i<30&&s->dialogue;i++)press(s,1);assert(!s->dialogue);}
int main(void){
 Story s;
 story_new(&s,0);assert(s.cinema==CINE_OPEN);assert(sizeof(Story)<64);
 story_draw_init();
 for(int scene=1;scene<CINE_COUNT;scene++){
  int changes=0;
  for(unsigned pg=0;pg<6;pg++){
   for(int i=0;i<6;i++){
    static const unsigned frames[6]={0,7,35,83,190,449};
    unsigned t=frames[i];
    memset(&frame,0xA5,sizeof(frame));
    s.cinema=(uint8_t)scene;s.cinema_frame=(uint16_t)t;
    s.page=(uint8_t)pg;s.dialogue=1;
    s.life_stage=scene==CINE_FINALE?STORY_LIFE_FINALE:0;
    s.finale_frame=(uint16_t)t;
    story_draw(&s,(int)t,frame.pixels);
    verify();
    if(i&&memcmp(last,frame.pixels,sizeof(last))!=0)changes++;
    memcpy(last,frame.pixels,sizeof(last));
   }
  }
  assert(changes>5);
 }
 story_new(&s,0);
 close_script(&s);assert(s.cinema==CINE_NONE);
 s.x=5;s.y=5;s.facing=2;press(&s,1);
 assert(s.cinema==CINE_LETTER);close_script(&s);
 s.x=11;s.y=8;s.facing=3;press(&s,1);
 assert(s.cinema==CINE_BORE);close_script(&s);
 s.x=8;s.y=12;s.facing=0;press(&s,1);
 assert(s.cinema==CINE_MARA);close_script(&s);
 s.x=19;s.y=4;s.facing=1;press(&s,1);
 assert(s.cinema==CINE_BUS);close_script(&s);
 assert(s.choosing&&s.cinema==CINE_BUS);
 press(&s,2);assert(!s.choosing&&s.cinema==CINE_NONE);
 puts("cinematics: 20 shot types, animated beats, palette and viewport guards OK");
 return 0;
}
