#include "story.h"
#include "story_draw.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define A 1u
#define B 2u
#define RIGHT 16u
#define R 256u
static void press(Story *s,unsigned key){
 story_tick(s,0);story_tick(s,(uint16_t)key);story_tick(s,0);
}
static void close_pages(Story *s){
 for(int i=0;i<40&&s->dialogue;i++)press(s,A);
 assert(!s->dialogue);
}
static void stand(Story *s,int x,int y,int facing){
 assert(!story_solid(x,y));s->x=(uint8_t)x;s->y=(uint8_t)y;
 s->prev_x=s->x;s->prev_y=s->y;s->facing=(uint8_t)facing;
 s->cooldown=0;
}
static void at(Story *s,char station){
 switch(station){
 case 'L':stand(s,5,5,2);break;
 case 'C':stand(s,11,8,3);break;
 case 'M':stand(s,19,8,3);break;
 case 'S':stand(s,21,8,3);break;
 case 'W':stand(s,6,12,3);break;
 case 'H':stand(s,7,3,3);break;
 case 'F':stand(s,8,12,3);break;
 case 'B':stand(s,19,4,1);break;
 default:assert(!"bad station");
 }
}
static void bad(Story *s,char station){
 at(s,station);press(s,B);assert(s->risk_menu==station);
 press(s,RIGHT);assert(s->risk_choice==1u);
 press(s,A);assert(!s->risk_menu&&s->dialogue);
}
static void goal(Story *s,int x,int y,const char *label){
 int gx=-1,gy=-1;const char *t=0;
 assert(story_goal(s,&gx,&gy,&t));
 assert(gx==x&&gy==y&&!strcmp(label,t));
}
static void render(Story *s,unsigned n){
 struct {uint8_t before[16],pixels[120*80],after[16];} p;
 story_draw_init();
 for(int view=0;view<2;view++){
  s->view=(uint8_t)view;memset(&p,0xAB,sizeof(p));
  s->finale_frame=(uint16_t)n;
  story_draw(s,(int)n,p.pixels);
  for(int i=0;i<16;i++)assert(p.before[i]==0xAB&&p.after[i]==0xAB);
  for(int i=0;i<120*80;i++)assert(p.pixels[i]<32);
 }
}
static void rescue(Story *s){
 goal(s,6,11,"WITNESS");
 at(s,'F');press(s,A);assert(s->dialogue);close_pages(s);
 press(s,R);assert(s->dialogue);close_pages(s);
 at(s,'W');press(s,A);assert(s->rescue_stage==STORY_RESCUE_WITNESS);
 close_pages(s);goal(s,21,7,"TERMINAL");
 at(s,'S');press(s,A);assert(s->rescue_stage==STORY_RESCUE_TRACE);
 close_pages(s);goal(s,7,2,"SAVE MARA");
 at(s,'H');press(s,A);
 assert(s->risk_menu=='H'&&s->risk_choice==0);
 render(s,75);
 press(s,A);
 assert(s->rescue_stage==STORY_RESCUE_SAVED&&s->dialogue);
 assert(!s->risk_menu);
 close_pages(s);assert(!s->ended);
}
static void setup_three(Story *s){
 story_new(s,0);close_pages(s);s->flags=STORY_ALL;
 /* Backing out of a suspicious menu never adds a bad check. */
 at(s,'L');press(s,B);assert(s->risk_menu=='L');
 render(s,0);press(s,A);assert(!s->risk_menu&&!s->bad_mask);
 bad(s,'L');assert(story_bad_count(s)==1);close_pages(s);
 at(s,'L');press(s,B);assert(!s->risk_menu);
 bad(s,'C');assert(story_bad_count(s)==2);close_pages(s);
 bad(s,'M');assert(story_bad_count(s)==3);
 assert(s->rescue_stage==STORY_RESCUE_MISSING);close_pages(s);
 /* Idle time cannot secretly fail a mission. */
 for(int i=0;i<650;i++)story_tick(s,0);
 assert(s->rescue_stage==STORY_RESCUE_MISSING&&!s->ended);
}
static void bus(Story *s){
 at(s,'B');press(s,A);assert(s->dialogue);close_pages(s);
 assert(s->choosing);press(s,A);
}
int main(void){
 Story s;setup_three(&s);rescue(&s);
 assert(s.bad_mask==(STORY_BAD_LETTER|STORY_BAD_CLERK|STORY_BAD_MUSIC));
 assert(story_bad_count(&s)==3);
 at(&s,'F');press(&s,A);assert(s.dialogue);close_pages(&s);
 /* The good route becomes selectable again after rescue. */
 s.flags=STORY_FULL;bus(&s);
 assert(s.ended&&s.ending==STORY_END_GOOD);
 render(&s,280);
 Story neutral;setup_three(&neutral);rescue(&neutral);
 neutral.flags=STORY_ALL;bus(&neutral);
 assert(!neutral.ended&&neutral.life_stage==STORY_LIFE_WAIT);
 close_pages(&neutral);
 /* Abandoning the door is a clear, explicit failure with an unknown fate. */
 Story lost;setup_three(&lost);
 at(&lost,'W');press(&lost,A);close_pages(&lost);
 at(&lost,'S');press(&lost,A);close_pages(&lost);
 at(&lost,'H');press(&lost,A);assert(lost.risk_menu=='H');
 press(&lost,RIGHT);assert(lost.risk_choice==1u);
 press(&lost,A);assert(lost.rescue_stage==STORY_RESCUE_LOST);
 close_pages(&lost);story_tick(&lost,0);
 assert(lost.ended&&lost.ending==STORY_END_BAD);
 for(unsigned n=0;n<STORY_END_SCORE_FRAMES;n+=311)render(&lost,n);
 lost.finale_frame=STORY_END_SCORE_FRAMES-1;
 story_tick(&lost,0);assert(lost.dialogue);
 {int count=0;const StoryPage *p=story_pages(&lost,&count);
  assert(count==4&&!strcmp(p[0].line1,"There was no"));}
 /* A fourth deliberate check AFTER rescuing Mara causes a distinct bad end
    that never implies she died or vanished. */
 Story after;setup_three(&after);rescue(&after);
 bad(&after,'S');assert(after.rescue_stage==STORY_RESCUE_AFTERSHOCK);
 assert(story_bad_count(&after)==4);
 close_pages(&after);story_tick(&after,0);
 assert(after.ended&&after.ending==STORY_END_BAD);
 after.finale_frame=STORY_END_SCORE_FRAMES-1;
 story_tick(&after,0);assert(after.dialogue);
 {int count=0;const StoryPage *p=story_pages(&after,&count);
  assert(count==4&&!strcmp(p[0].line1,"Mara returned home."));}
 assert(sizeof(Story)<64);
 puts("Mara: 4 intentional checks, cancel, 3-step rescue, optional fail, saved second chances, 2 bad endings and guarded film OK");
 return 0;
}
