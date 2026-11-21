#include "story.h"
#include "story_draw.h"
#include "cinema.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define A 1u
#define B 2u
#define RIGHT 16u
static void press(Story *s,unsigned key){
 story_tick(s,0);story_tick(s,(uint16_t)key);story_tick(s,0);
}
static void done(Story *s){
 for(int i=0;i<48&&s->dialogue;i++)press(s,A);
 assert(!s->dialogue);
}
static void stand(Story *s,int x,int y,int facing){
 assert(!story_solid(x,y));s->x=(uint8_t)x;s->y=(uint8_t)y;
 s->prev_x=s->x;s->prev_y=s->y;s->cooldown=0;s->facing=(uint8_t)facing;
}
static void begin(Story *s,int station,int alternative){
 press(s,A);assert(s->activity==station);
 if(alternative)press(s,RIGHT);
 press(s,A);assert(!s->activity&&s->dialogue);
}
static void opening(Story *s,const char *expected){
 int n=0;const StoryPage *p=story_pages(s,&n);
 assert(n>0&&p&&!strcmp(p[0].line1,expected));
}
static void render(Story *s){
 struct {uint8_t left[16],pixels[120*80],right[16];} out;
 story_draw_init();
 for(int view=0;view<2;view++){
   s->view=(uint8_t)view;memset(&out,0xC7,sizeof(out));
   story_draw(s,172,out.pixels);
   for(int i=0;i<16;i++)assert(out.left[i]==0xC7&&out.right[i]==0xC7);
   for(int i=0;i<120*80;i++)assert(out.pixels[i]<32);
 }
}
static void compare_scene(unsigned scene,unsigned music,unsigned art,unsigned style){
 uint8_t old[120*80],newpic[120*80];
 cinema_draw(old,scene,153,0,0);
 memcpy(newpic,old,sizeof(old));
 cinema_project_detail(newpic,scene,music,art,style,2);
 int changed=0;
 for(int k=0;k<120*80;k++){assert(newpic[k]<32);changed+=newpic[k]!=old[k];}
 assert(changed>=4);
}
int main(void){
 Story s;story_new(&s,0);done(&s);
 s.flags=STORY_ALL;s.life_stage=STORY_LIFE_WAIT;
 assert(s.song_steps==0&&s.art_steps==0&&s.project_style==0);
 /* Times of day affect conversations but never lock an accessible NPC. */
 s.daily_mask=STORY_DAY_MARA|STORY_DAY_CLERK;s.bond=1;
 stand(&s,8,12,3);s.day_frame=60;
 press(&s,A);opening(&s,"The cafe's opening.");done(&s);
 s.day_frame=STORY_DAY_FRAMES-600;
 press(&s,A);opening(&s,"Evening's here.");done(&s);
 stand(&s,11,8,3);press(&s,A);
 opening(&s,"Five minutes until");done(&s);
 s.day_frame=0;s.daily_mask=0;
 stand(&s,19,8,3);begin(&s,'M',0);
 assert(s.song_steps==1&&s.daily_memories==1);done(&s);
 begin(&s,'m',1);assert(s.song_steps==2);
 assert(s.project_style&STORY_STYLE_SONG_SOFT);
 opening(&s,"The bass finds the");done(&s);
 begin(&s,'m',0);assert(s.song_steps==3);
 opening(&s,"The rhythm is");done(&s);
 begin(&s,'m',0);assert(s.song_steps==4);
 opening(&s,"MASTER COPY SAVED.");done(&s);
 unsigned memories=s.daily_memories,work=s.work;
 press(&s,A);assert(s.activity=='E');render(&s);
 /* After completing the master, LISTEN stays one easy choice away.
    Advanced editor is optional, never a forced progression screen. */
 press(&s,A);opening(&s,"An old BORE theme");done(&s);
 assert(s.song_steps==4&&s.daily_memories==memories&&s.work==work);
 stand(&s,11,8,3);s.daily_mask|=STORY_DAY_CLERK;
 press(&s,A);opening(&s,"This recording");done(&s);
 stand(&s,14,7,3);begin(&s,'R',0);assert(s.art_steps==1);done(&s);
 begin(&s,'r',1);assert(s.art_steps==2);
 assert(s.project_style&STORY_STYLE_ART_WARM);
 opening(&s,"Buildings rise over");done(&s);
 begin(&s,'r',0);assert(s.art_steps==3);
 opening(&s,"A thin brush adds");done(&s);
 begin(&s,'r',0);assert(s.art_steps==4);
 opening(&s,"THE OLD QUARTER");done(&s);
 stand(&s,8,12,3);s.daily_mask|=STORY_DAY_MARA;
 press(&s,A);opening(&s,"You finished the");done(&s);
 compare_scene(CINE_ART,0,s.art_steps,s.project_style);
 compare_scene(CINE_MUSIC,s.song_steps,0,s.project_style);
 compare_scene(CINE_MARA,0,0,0);
 render(&s);
 /* Completed creations remain optional and appointment sequence is intact. */
 s.life_stage=STORY_LIFE_CLINIC;s.day_number=2;
 {int x,y;const char *label=0;
  assert(story_goal(&s,&x,&y,&label));assert(x==16&&y==5);
  assert(!strcmp(label,"CLINIC"));}
 stand(&s,15,5,0);press(&s,A);
 assert(s.life_stage==STORY_LIFE_FOLLOWUP_WAIT);done(&s);
 assert(s.song_steps==4&&s.art_steps==4);
 assert(sizeof(Story)<64);
 puts("projects: 2x4 stages, choices, persistent styles, NPC time-of-day, studio/canvas rendering, clinic priority OK");
 return 0;
}
