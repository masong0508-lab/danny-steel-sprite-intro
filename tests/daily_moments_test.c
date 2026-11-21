#include "story.h"
#include "story_draw.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define A 1u
static void press(Story *s,unsigned key){story_tick(s,0);story_tick(s,(uint16_t)key);story_tick(s,0);}
static void done(Story *s){for(int i=0;i<48&&s->dialogue;i++)press(s,A);assert(!s->dialogue);}
static void stand(Story *s,int x,int y){
 assert(!story_solid(x,y));s->x=(uint8_t)x;s->y=(uint8_t)y;
 s->prev_x=s->x;s->prev_y=s->y;s->cooldown=0;s->facing=3;
}
static void goal(Story *s,int x,int y,const char *name){
 int gx=-1,gy=-1;const char *label=0;
 assert(story_goal(s,&gx,&gy,&label));assert(gx==x&&gy==y&&!strcmp(label,name));
}
static void render(Story *s){
 struct {uint8_t before[16],pixels[120*80],after[16];} b;
 story_draw_init();
 for(int mode=0;mode<2;mode++){
  memset(&b,0xCC,sizeof(b));s->view=(uint8_t)mode;story_draw(s,187,b.pixels);
  for(int i=0;i<16;i++)assert(b.before[i]==0xCC&&b.after[i]==0xCC);
  for(int i=0;i<120*80;i++)assert(b.pixels[i]<32);
 }
}
int main(void){
 Story s;story_new(&s,0);done(&s);s.flags=STORY_ALL;
 s.life_stage=STORY_LIFE_WAIT;s.day_number=1;s.day_frame=0;
 goal(&s,8,11,"MARA");
 stand(&s,8,12);press(&s,A);
 assert(s.daily_mask==STORY_DAY_MARA&&s.daily_memories==1&&s.cinema==CINE_MARA);
 render(&s);done(&s);
 stand(&s,8,12);press(&s,A);done(&s);
 assert(s.daily_memories==1);
 goal(&s,5,5,"LETTER");
 stand(&s,5,5);s.facing=2;press(&s,A);
 assert(s.cinema==CINE_LETTER);done(&s);
 assert(s.daily_mask==(STORY_DAY_MARA|STORY_DAY_LETTER));
 goal(&s,11,7,"BORE");
 stand(&s,11,8);press(&s,A);assert(s.cinema==CINE_BORE);done(&s);
 goal(&s,14,6,"PAINT");
 stand(&s,14,7);press(&s,A);assert(s.cinema==CINE_ART);done(&s);
 goal(&s,19,7,"MUSIC");
 stand(&s,19,8);press(&s,A);assert(s.cinema==CINE_MUSIC);
 assert(s.flags&STORY_MUSIC);done(&s);
 assert((s.daily_mask&STORY_DAY_ALL)==STORY_DAY_ALL);
 assert(s.daily_mask&STORY_DAY_RECAP&&s.daily_memories==5);
 goal(&s,14,6,"REST");
 s.day_frame=STORY_DAY_FRAMES-1;story_tick(&s,0);
 assert(s.day_number==2&&s.life_stage==STORY_LIFE_CLINIC&&s.daily_mask==0);
 goal(&s,16,5,"CLINIC");done(&s);
 stand(&s,15,5);s.facing=0;press(&s,A);done(&s);
 assert(s.life_stage==STORY_LIFE_FOLLOWUP_WAIT);goal(&s,8,11,"MARA");
 /* Ordinary painting remains possible BETWEEN the medical appointments. */
 stand(&s,14,7);press(&s,A);
 assert(s.daily_mask==STORY_DAY_ART&&s.cinema==CINE_ART);
 int n;const StoryPage *pages=story_pages(&s,&n);
 assert(n==3&&!strcmp(pages[1].line1,"Leave some space"));
 render(&s);done(&s);assert(s.daily_memories==6);
 s.day_frame=STORY_DAY_FRAMES-1;story_tick(&s,0);
 assert(s.day_number==3&&s.life_stage==STORY_LIFE_FINAL_CLINIC);
 goal(&s,16,5,"CLINIC");done(&s);
 stand(&s,15,5);s.facing=0;press(&s,A);done(&s);
 assert(s.life_stage==STORY_LIFE_LAST_WORK&&!s.ended);
 goal(&s,14,6,"CANVAS");
 Story t;story_new(&t,1);done(&t);t.flags=STORY_ALL;
 t.life_stage=STORY_LIFE_WAIT;t.day_frame=STORY_DAY_FRAMES-1;
 story_tick(&t,0);
 assert(t.life_stage==STORY_LIFE_CLINIC&&t.daily_mask==0&&t.daily_memories==0&&!t.ended);
 assert(sizeof(Story)<64);
 puts("daily memories: 5 optional scenes, changing scripts, 2D/3D guards, day reset, appointments and endings accessible");
 return 0;
}
