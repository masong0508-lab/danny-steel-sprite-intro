#include "story.h"
#include "story_draw.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#define A 1u
#define B 2u
#define RIGHT 16u
static void press(Story *s,unsigned key){story_tick(s,0);story_tick(s,(uint16_t)key);story_tick(s,0);}
static void done(Story *s){for(int i=0;i<40&&s->dialogue;i++)press(s,A);assert(!s->dialogue);}
static void stand(Story *s,int x,int y,int dir){
 assert(!story_solid(x,y));s->x=(uint8_t)x;s->y=(uint8_t)y;
 s->prev_x=s->x;s->prev_y=s->y;s->facing=(uint8_t)dir;s->cooldown=0;
}
static void choose(Story *s,int station,int option){
 press(s,A);assert(s->activity==station);
 if(option)press(s,RIGHT);
 assert(s->activity_choice==(unsigned)option);
 press(s,A);assert(!s->activity&&s->dialogue);
}
static void guards(Story *s){
 struct {uint8_t a[16],p[120*80],b[16];} frame;
 story_draw_init();
 for(int view=0;view<2;view++){
  s->view=(uint8_t)view;memset(&frame,0xE7,sizeof(frame));
  story_draw(s,47,frame.p);
  for(int i=0;i<16;i++)assert(frame.a[i]==0xE7&&frame.b[i]==0xE7);
  for(int i=0;i<120*80;i++)assert(frame.p[i]<32);
 }
}
int main(void){
 Story s;story_new(&s,0);done(&s);
 s.flags=STORY_ALL;s.life_stage=STORY_LIFE_WAIT;
 assert(s.energy==3&&s.spark==1&&s.bond==1&&s.work==0);
 stand(&s,8,12,3);
 press(&s,A);assert(s.activity=='F'&&s.daily_mask==0);
 guards(&s);press(&s,B);assert(!s.activity&&!s.daily_mask&&s.energy==3);
 choose(&s,'F',1);assert(s.bond==2&&s.energy==4&&s.daily_mask==STORY_DAY_MARA);
 done(&s);
 stand(&s,19,8,3);choose(&s,'M',0);
 assert((s.flags&STORY_MUSIC)!=0&&s.work==2&&s.energy==3&&s.spark==2);
 done(&s);
 stand(&s,14,7,3);choose(&s,'R',0);
 assert(s.work==4&&s.energy==2&&s.spark==3);
 done(&s);
 /* Repeating an activity today never earns progress twice. */
 stand(&s,14,7,3);press(&s,A);assert(s.activity=='r');
 press(&s,B);assert(!s.activity);
 assert(s.work==4&&s.daily_memories==3);
 /* Exhaustion never blocks a choice or ends the game. */
 s.daily_mask=(uint8_t)(s.daily_mask & ~STORY_DAY_ART);s.energy=0;
 choose(&s,'R',0);assert(s.work==5&&s.energy==0&&!s.ended);
 done(&s);
 s.daily_mask=(uint8_t)(s.daily_mask & ~STORY_DAY_ART);
 choose(&s,'R',1);assert(s.energy==2&&s.spark==5&&s.work==5);
 done(&s);
 /* Clock reset restores energy; an appointment takes priority. */
 s.day_frame=STORY_DAY_FRAMES-1;story_tick(&s,0);
 assert(s.day_number==2&&s.energy==4&&s.daily_mask==0);
 assert(s.life_stage==STORY_LIFE_CLINIC&&!s.ended);
 assert(s.activity==0);
 done(&s);stand(&s,15,5,0);press(&s,A);assert(s.life_stage==STORY_LIFE_FOLLOWUP_WAIT);
 done(&s);stand(&s,8,12,3);choose(&s,'F',0);
 assert(s.bond==4&&s.energy==3);
 done(&s);
 /* Bond and real work trigger new reactive, non-gating dialogue. */
 stand(&s,8,12,3);press(&s,A);
 {int count=0;const StoryPage *pages=story_pages(&s,&count);
  assert(count>=2&&!strcmp(pages[0].line1,"You never have to"));}
 done(&s);
 s.work=9;s.daily_mask|=STORY_DAY_MUSIC;stand(&s,19,8,3);press(&s,A);
 assert(s.activity=='m');press(&s,B);
 s.song_steps=STORY_PROJECT_DONE;press(&s,A);
 /* Once mastered, the clear LISTEN / STUDIO menu replaces a redundant
    repeated success popup. The easy listening path remains available. */
 assert(s.activity=='E'&&s.activity_choice==0);
 press(&s,A);
 {int count=0;const StoryPage *pages=story_pages(&s,&count);
  assert(count>=2&&!strcmp(pages[0].line1,"An old BORE theme"));}
 done(&s);
 /* The measured canvas and music sets change as work is completed. */
 {uint8_t before[120*80],after[120*80];
  cinema_draw(before,CINE_ART,250,0,0);
  cinema_draw(after,CINE_ART,250,0,0);
  cinema_moment_detail(after,CINE_ART,12,0);
  int diff=0;for(int y=27;y<38;y++)for(int x=74;x<99;x++)
   diff+=before[y*120+x]!=after[y*120+x];
  assert(diff>=8);
  cinema_draw(before,CINE_MUSIC,250,0,0);
  cinema_draw(after,CINE_MUSIC,250,0,0);
  cinema_moment_detail(after,CINE_MUSIC,12,0);
  diff=0;for(int y=26;y<40;y++)for(int x=76;x<107;x++)
   diff+=before[y*120+x]!=after[y*120+x];
  assert(diff>=2);
 }
 guards(&s);
 assert(sizeof(Story)<64);
 puts("life choices: cancel, five two-option decisions, bounded stats, low-energy safety, 2D/3D, appointments OK");
 return 0;
}
