#include "story.h"
#include "story_draw.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define A 1u
#define B 2u
#define RIGHT 16u
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
 stand(&s,8,12);press(&s,A);assert(s.activity=='F');press(&s,A);
 assert(s.daily_mask==STORY_DAY_MARA&&s.daily_memories==1&&s.cinema==CINE_MARA);
 render(&s);done(&s);
 stand(&s,8,12);press(&s,A);done(&s);
 assert(s.daily_memories==1);
 goal(&s,5,5,"LETTER");
 stand(&s,5,5);s.facing=2;press(&s,A);assert(s.activity=='L');press(&s,A);
 assert(s.cinema==CINE_LETTER);done(&s);
 assert(s.daily_mask==(STORY_DAY_MARA|STORY_DAY_LETTER));
 goal(&s,11,7,"BORE");
 stand(&s,11,8);press(&s,A);assert(s.activity=='C');press(&s,A);assert(s.cinema==CINE_BORE);done(&s);
 goal(&s,14,6,"PAINT");
 stand(&s,14,7);press(&s,A);assert(s.activity=='R');press(&s,A);assert(s.cinema==CINE_ART);done(&s);
 goal(&s,19,7,"MUSIC");
 stand(&s,19,8);press(&s,A);assert(s.activity=='M');press(&s,A);assert(s.cinema==CINE_MUSIC);
 assert(s.flags&STORY_MUSIC);done(&s);
 assert((s.daily_mask&STORY_DAY_ALL)==STORY_DAY_ALL);
 assert(s.daily_mask&STORY_DAY_RECAP&&s.daily_memories==5);
 goal(&s,14,6,"ART WORK");
 s.day_frame=STORY_DAY_FRAMES-1;story_tick(&s,0);
 assert(s.day_number==2&&s.life_stage==STORY_LIFE_CLINIC&&s.daily_mask==0);
 goal(&s,16,5,"CLINIC");done(&s);
 stand(&s,15,5);s.facing=0;press(&s,A);done(&s);
 assert(s.life_stage==STORY_LIFE_FOLLOWUP_WAIT);goal(&s,8,11,"MARA");
 /* Ordinary painting remains possible BETWEEN the medical appointments. */
 stand(&s,14,7);press(&s,A);assert(s.activity=='R');press(&s,A);
 assert(s.daily_mask==STORY_DAY_ART&&s.cinema==CINE_ART);
 int n;const StoryPage *pages=story_pages(&s,&n);
 assert(n==2&&!strcmp(pages[0].line1,"Buildings rise over"));
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
 /* Finished projects must not block the last daily objective or recap.
    Start with all but art and music, and visit the real world stations. */
 Story master;story_new(&master,0);done(&master);
 master.flags=STORY_FULL;master.life_stage=STORY_LIFE_WAIT;
 master.day_number=1;master.day_frame=0;
 master.song_steps=master.art_steps=STORY_PROJECT_DONE;
 master.daily_mask=STORY_DAY_MARA|STORY_DAY_LETTER|STORY_DAY_CLERK;
 master.daily_memories=3;master.energy=1;master.spark=1;master.work=12;
 goal(&master,14,6,"PAINT");
 stand(&master,14,7);press(&master,A);assert(master.activity=='R');
 render(&master);press(&master,A);done(&master);
 assert(master.art_steps==4&&master.daily_memories==4&&master.work==12);
 goal(&master,19,7,"MUSIC");
 stand(&master,19,8);press(&master,A);assert(master.activity=='E');
 render(&master);press(&master,RIGHT);press(&master,B);
 assert(!master.activity&&master.daily_memories==4&&master.energy==1);
 press(&master,A);press(&master,RIGHT);press(&master,A);
 assert(master.activity==STORY_ACT_STUDIO);press(&master,B);
 assert(master.daily_memories==4&&!(master.daily_mask&STORY_DAY_MUSIC));
 press(&master,A);assert(master.activity=='E');press(&master,A);done(&master);
 assert(master.daily_memories==5&&master.daily_mask&STORY_DAY_RECAP);
 assert(master.song_steps==4&&master.work==12&&master.energy==2&&master.spark==2);
 press(&master,A);press(&master,A);done(&master);
 assert(master.daily_memories==5&&master.energy==2&&master.spark==2);
 goal(&master,14,6,"REST");
 assert(!strcmp(story_goal_reason(&master),"REST OR EXPLORE UNTIL CALL"));
 stand(&master,14,7);press(&master,A);assert(master.activity==STORY_ACT_ROOF_REST);
 render(&master);press(&master,RIGHT);press(&master,A);
 assert(!master.activity&&master.energy==2);
 for(unsigned i=0;i<5u;i++){
  press(&master,A);assert(master.activity==STORY_ACT_ROOF_REST);
  press(&master,A);done(&master);
 }
 assert(master.energy==STORY_STAT_MAX&&master.spark==2&&master.work==12);
 assert(master.daily_memories==5&&master.art_steps==4);
 master.day_frame=STORY_DAY_FRAMES-1;story_tick(&master,0);
 assert(master.life_stage==STORY_LIFE_CLINIC);done(&master);
 goal(&master,16,5,"CLINIC");
 stand(&master,14,7);press(&master,A);
 assert(!master.activity&&master.dialogue);done(&master);
 assert(sizeof(Story)<64);
 puts("daily memories: 5 optional scenes, changing scripts, 2D/3D guards, day reset, appointments and endings accessible");
 return 0;
}
