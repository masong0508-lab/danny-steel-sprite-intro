#include "story.h"
#include "story_draw.h"
#include "story_save.h"
#include "rescue_save.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define A 1u
#define B 2u
#define START 8u
#define RIGHT 16u
#define R 256u
static void press(Story *s,unsigned key){
 story_tick(s,0);story_tick(s,(uint16_t)key);story_tick(s,0);
}
static void close_pages(Story *s){
 for(int i=0;i<50&&s->dialogue;i++)press(s,A);
 assert(!s->dialogue);
}
static void at(Story *s,int x,int y,int facing){
 assert(!story_solid(x,y));
 s->x=(uint8_t)x;s->y=(uint8_t)y;s->facing=(uint8_t)facing;
 s->cooldown=0;s->prev_x=s->x;s->prev_y=s->y;
}
static void assert_line(Story *s,int page,const char *line){
 int count=0;const StoryPage *p=story_pages(s,&count);
 assert(s->dialogue&&p&&count>page);
 assert(!strcmp(p[page].line1,line));
}
static void guard(Story *s){
 struct {uint8_t left[16],pixels[120*80],right[16];} out;
 story_draw_init();
 for(int view=0;view<2;view++){
  s->view=(uint8_t)view;memset(&out,0xB3,sizeof(out));
  story_draw(s,0,out.pixels);
  for(int j=0;j<16;j++)assert(out.left[j]==0xB3&&out.right[j]==0xB3);
  for(int j=0;j<120*80;j++)assert(out.pixels[j]<32u);
 }
}
static void create_wait(Story *s){
 story_new(s,0);close_pages(s);
 s->flags=STORY_FULL;
 s->bad_mask=STORY_BAD_LETTER|STORY_BAD_CLERK|STORY_BAD_MUSIC;
 s->rescue_stage=STORY_RESCUE_TRACE;
 at(s,7,3,3);press(s,A);assert(s->risk_menu=='H');
 press(s,RIGHT);assert(s->risk_choice);
 press(s,A);assert(s->rescue_stage==STORY_RESCUE_LOST);
 assert(!s->ended&&s->life_stage==STORY_LIFE_NONE);
 assert(s->day_number==0u&&s->day_frame==0u);
 assert_line(s,1,"She will call you.");
}
static void visit(Story *s,int x,int y,int facing,const char *line){
 at(s,x,y,facing);press(s,A);assert_line(s,0,line);
 guard(s);close_pages(s);
}
static void persist(Story *s){
 uint8_t base[STORY_SAVE_BYTES],norm[STORY_SAVE_BYTES],extra[8];
 story_save_encode(s,base);
 assert(story_save_decode(base,norm));
 assert(norm[16]==s->day_number);
 assert((unsigned)norm[17]+((unsigned)norm[18]<<8)==s->day_frame);
 rescue_save_encode(s->slot,s->bad_mask,s->rescue_stage,extra);
 uint8_t mask=0,stage=0;
 assert(rescue_save_decode(s->slot,extra,&mask,&stage));
 assert(mask==s->bad_mask&&stage==STORY_RESCUE_LOST);
}
int main(void){
 Story s;create_wait(&s);
 guard(&s);close_pages(&s);
 for(int n=0;n<650;n++)story_tick(&s,0);
 assert(!s.ended&&s.day_number==0u);
 int x=0,y=0;const char *label=0;
 assert(story_goal(&s,&x,&y,&label));
 assert(x==8&&y==11&&!strcmp(label,"PHONE"));
 press(&s,R);assert_line(&s,0,"AWAIT MARA'S CALL.");
 close_pages(&s);
 visit(&s,8,12,3,"CALL STILL PENDING.");
 visit(&s,6,12,3,"They promised news.");
 visit(&s,21,8,3,"LOCATION UNKNOWN.");
 visit(&s,7,3,3,"The door is shut.");
 at(&s,19,4,1);press(&s,A);
 assert_line(&s,0,"AWAIT MARA'S CALL.");
 close_pages(&s);
 assert(!s.choosing&&!s.ended);
 persist(&s);
 press(&s,START);assert(s.paused);
 unsigned paused_frame=s.day_frame;
 for(int n=0;n<80;n++)story_tick(&s,0);
 assert(s.day_frame==paused_frame);
 press(&s,START);assert(!s.paused);
 assert(!s.ended);
 s.day_frame=STORY_DAY_FRAMES-1u;
 story_tick(&s,0);
 assert(s.day_number==STORY_HOPE_REALIZE_DAY);
 assert(s.day_frame==0u&&!s.ended);
 assert_line(&s,2,"Mara still has not");
 persist(&s);guard(&s);
 unsigned held=s.day_frame;
 for(int n=0;n<300;n++)story_tick(&s,0);
 assert(s.day_frame==held&&s.dialogue);
 close_pages(&s);
 assert(!s.ended&&s.rescue_stage==STORY_RESCUE_LOST);
 press(&s,R);assert_line(&s,0,"MARA NOT RECOVERED.");
 close_pages(&s);
 visit(&s,8,12,3,"NO VERIFIED UPDATE.");
 persist(&s);
 s.day_frame=STORY_DAY_FRAMES-3u;
 story_tick(&s,0);
 assert(!s.ended);
 story_tick(&s,0);
 assert(!s.ended);
 story_tick(&s,0);
 assert(s.ended&&s.ending==STORY_END_BAD&&s.day_number==STORY_HOPE_END_DAY);
 guard(&s);
 assert(s.rescue_stage==STORY_RESCUE_LOST);
 Story fourth;story_new(&fourth,1);close_pages(&fourth);
 fourth.bad_mask=STORY_BAD_LETTER|STORY_BAD_CLERK|STORY_BAD_MUSIC;
 fourth.rescue_stage=STORY_RESCUE_MISSING;
 at(&fourth,21,8,3);press(&fourth,B);
 assert(fourth.risk_menu=='S');press(&fourth,RIGHT);press(&fourth,A);
 assert(fourth.rescue_stage==STORY_RESCUE_LOST&&!fourth.ended);
 assert_line(&fourth,1,"She will call you.");
 close_pages(&fourth);
 assert(fourth.day_number==0u);
 Story safe;story_new(&safe,0);close_pages(&safe);
 safe.rescue_stage=STORY_RESCUE_SAVED;
 safe.bad_mask=STORY_BAD_LETTER|STORY_BAD_CLERK|STORY_BAD_MUSIC;
 at(&safe,21,8,3);press(&safe,B);
 assert(safe.risk_menu=='S');press(&safe,RIGHT);press(&safe,A);
 assert(safe.rescue_stage==STORY_RESCUE_AFTERSHOCK);
 close_pages(&safe);story_tick(&safe,0);
 assert(safe.ended&&safe.ending==STORY_END_BAD);
 puts("hope: false reassurance, delayed nonfatal realization, two saved days, phone/witness/terminal, guarded 2D/3D, alternate rescue outcomes OK");
 return 0;
}
