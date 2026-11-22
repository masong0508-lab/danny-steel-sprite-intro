#include "story.h"
#include "story_checks.h"
#include "story_draw.h"
#include "hd_canvas.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Independently check shortest legal walking distance to an interaction.
   For the letter the target is a walkable standing tile; every solid NPC
   uses a reachable neighboring tile. */
static unsigned shortest(int sx,int sy,int gx,int gy){
 unsigned distances[STORY_H][STORY_W];
 int queue_x[STORY_W*STORY_H],queue_y[STORY_W*STORY_H];
 for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++)
  distances[y][x]=999u;
 if(story_solid(sx,sy))return 999u;
 unsigned a=0,b=0;queue_x[b]=sx;queue_y[b++]=sy;
 distances[sy][sx]=0;
 while(a<b){
  int x=queue_x[a],y=queue_y[a++];
  int delta=x>gx?x-gx:gx-x;delta+=y>gy?y-gy:gy-y;
  if((!story_solid(gx,gy)&&delta==0)||
     (story_solid(gx,gy)&&delta<=1))return distances[y][x];
  static const int dx[]={1,0,-1,0},dy[]={0,1,0,-1};
  for(unsigned d=0;d<4;d++){
   int xx=x+dx[d],yy=y+dy[d];
   if(xx<0||yy<0||xx>=STORY_W||yy>=STORY_H||
      story_solid(xx,yy)||distances[yy][xx]!=999u)continue;
   distances[yy][xx]=distances[y][x]+1u;
   queue_x[b]=xx;queue_y[b++]=yy;
  }
 }
 return 999u;
}
static void press(Story *s,uint16_t key){
 story_tick(s,0u);story_tick(s,key);story_tick(s,0u);
}
static void check_route(Story *s,unsigned *seen){
 int gx=0,gy=0;const char *label=0;
 if(!story_goal(s,&gx,&gy,&label)){
  unsigned dir=99u,steps=999u;
  assert(!story_route(s,&dir,&steps));return;
 }
 unsigned expected=shortest((int)s->x,(int)s->y,gx,gy);
 unsigned dir=99u,steps=999u;
 int found=story_route(s,&dir,&steps);
 if(expected==999u){assert(!found);return;}
 assert(found&&dir<=4u&&steps==expected);
 unsigned original_x=s->x,original_y=s->y;
 unsigned previous_steps=steps;
 for(unsigned k=0;k<expected;k++){
  assert(dir<4u);
  static const int dx[]={1,0,-1,0},dy[]={0,1,0,-1};
  int xx=(int)s->x+dx[dir],yy=(int)s->y+dy[dir];
  assert(!story_solid(xx,yy));
  s->x=(uint8_t)xx;s->y=(uint8_t)yy;
  assert(story_route(s,&dir,&steps));
  assert(steps+1u==previous_steps);
  previous_steps=steps;
 }
 assert(steps==0u&&dir==4u);
 s->x=(uint8_t)original_x;s->y=(uint8_t)original_y;
 ++*seen;
}
static void guards(Story *s){
 struct {uint8_t first[16],pixels[120*80],last[16];} legacy;
 struct {uint8_t first[16],pixels[HD_BYTES],last[16];} high;
 memset(&legacy,0xAC,sizeof legacy);
 story_draw(s,11,legacy.pixels);
 for(unsigned i=0;i<sizeof legacy.first;i++)
  assert(legacy.first[i]==0xAC && legacy.last[i]==0xAC);
 for(unsigned i=0;i<sizeof legacy.pixels;i++)assert(legacy.pixels[i]<32u);
 memset(&high,0xBC,sizeof high);
 hd_rect(high.pixels,0,0,HD_W,HD_H,HD_STONE_DARK);
 hd_game_overlay(high.pixels,s);
 for(unsigned i=0;i<sizeof high.first;i++)
  assert(high.first[i]==0xBC && high.last[i]==0xBC);
 for(unsigned i=0;i<sizeof high.pixels;i++)assert(high.pixels[i]<64u);
}
static void test_guided_mainline(void){
 Story s;story_new(&s,1);s.dialogue=0;s.cinema=CINE_NONE;
 static const uint16_t key[]={16u,128u,32u,64u};
 static const char *const goal[]={"LETTER","STAMP","MARA","MUSIC","CANVAS","LAST BUS"};
 for(unsigned stage=0;stage<6u;stage++){
  unsigned route_dir=4u,route_length=0u;
  int gx=0,gy=0;const char *title=0;
  assert(story_goal(&s,&gx,&gy,&title));
  assert(!strcmp(title,goal[stage]));
  for(unsigned steps=0;steps<STORY_W*STORY_H;steps++){
   assert(story_route(&s,&route_dir,&route_length));
   if(!route_length)break;
   assert(route_dir<4u);
   s.cooldown=0;s.old_keys=0;
   uint8_t before_x=s.x,before_y=s.y;
   story_tick(&s,key[route_dir]);story_tick(&s,0);
   assert(s.x!=before_x||s.y!=before_y);
   assert(!story_solid(s.x,s.y));
  }
  assert(story_route(&s,&route_dir,&route_length)&&route_length==0u);
  assert(route_dir==4u);
  /* A on the reached tile must interact with the intended goal, not
     another actor who happens to stand on the opposite side of the street. */
  const char *hint=story_action_hint(&s);
  switch(stage){
   case 0:assert(!strcmp(hint,"A LETTER"));break;
   case 1:assert(!strcmp(hint,"A BORE"));break;
   case 2:assert(!strcmp(hint,"A MARA"));break;
   case 3:assert(!strcmp(hint,"A MUSIC"));break;
   case 4:assert(!strcmp(hint,"A CANVAS"));break;
   case 5:assert(!strcmp(hint,"A BUS"));break;
  }
  press(&s,1u);
  if(stage==0)assert(s.flags&STORY_HOME);
  if(stage==1)assert(s.flags&STORY_STAMP);
  if(stage==2){
   while(s.dialogue)press(&s,1u);
   assert(s.flags&STORY_MARA);
  }else if(stage==3)assert(s.flags&STORY_MUSIC);
  else if(stage==4)assert(s.flags&STORY_ART);
  if(s.dialogue){
   for(int i=0;i<12&&s.dialogue;i++)press(&s,1u);
  }
  assert(!s.dialogue);
  if(stage==5){assert(s.choosing);press(&s,2u);assert(!s.choosing);}
 }
 assert((s.flags&STORY_FULL)==STORY_FULL);
}
int main(void){
 test_guided_mainline();
 Story s;story_new(&s,0);s.dialogue=0;
 const char *reason=story_goal_reason(&s);
 assert(strstr(reason,"LETTER")!=0);
 assert(!strcmp(story_action_hint(&s),"A SHIFT"));
 s.x=5;s.y=5;s.facing=2;
 assert(!strcmp(story_action_hint(&s),"A LETTER"));
 assert(story_risk_available(&s));
 s.bad_mask=STORY_BAD_LETTER;
 assert(!story_risk_available(&s));
 s.bad_mask=0;s.x=12;s.y=12;
 assert(!strcmp(story_action_hint(&s),"A LOOK"));
 press(&s,1u);
 assert(s.dialogue);int count=0;
 const StoryPage *pages=story_pages(&s,&count);
 assert(count==2 && strcmp(pages[0].speaker,"DANNY")==0);
 press(&s,2u);assert(!s.dialogue);
 s.x=20;s.y=11;s.facing=1;press(&s,1u);
 pages=story_pages(&s,&count);
 assert(count==2&&strstr(pages[0].line1,"Potheads")!=0);
 press(&s,2u);assert(!s.dialogue);
 /* The field guide is an actual interactive pause page, not a dummy
    menu hint. It never changes story flags or calls into SRAM. */
 s.x=12;s.y=13;s.choice=0;s.flags=0;
 Story unchanged=s;
 press(&s,8u);assert(s.paused);
 press(&s,512u);assert(s.choice&STORY_FIELD_GUIDE_ACTIVE);
 guards(&s);
 press(&s,16u);assert(s.choice&STORY_FIELD_GUIDE_DETAIL);
 guards(&s);
 press(&s,32u);assert(!(s.choice&STORY_FIELD_GUIDE_DETAIL));
 guards(&s);
 press(&s,256u);assert(s.choice&STORY_CHECK_PAGE_ACTIVE);
 assert(!(s.choice&STORY_FIELD_GUIDE_ACTIVE));guards(&s);
 press(&s,512u);assert(s.choice&STORY_FIELD_GUIDE_ACTIVE);
 assert(!(s.choice&STORY_CHECK_PAGE_ACTIVE));
 assert(!(s.choice&STORY_FIELD_GUIDE_DETAIL));guards(&s);
 press(&s,2u);assert(!(s.choice&STORY_FIELD_GUIDE_ACTIVE));
 assert(s.paused&&!s.quit);guards(&s);
 press(&s,8u);assert(!s.paused&&!s.quit);
 assert(s.flags==unchanged.flags&&s.energy==unchanged.energy&&
        s.bond==unchanged.bond&&s.work==unchanged.work);
 /* All normal quest types, rescue leads, day and clinic states. */
 unsigned count_routes=0;
 for(unsigned stage=0;stage<15u;stage++){
  Story t;story_new(&t,0);t.dialogue=0;
  if(stage==1)t.flags=STORY_HOME;
  if(stage==2)t.flags=STORY_HOME|STORY_STAMP;
  if(stage==3)t.flags=STORY_ALL;
  if(stage==4)t.flags=STORY_ALL|STORY_MUSIC;
  if(stage>=5)t.flags=STORY_FULL;
  if(stage==6)t.rescue_stage=STORY_RESCUE_MISSING;
  if(stage==7)t.rescue_stage=STORY_RESCUE_WITNESS;
  if(stage==8)t.rescue_stage=STORY_RESCUE_TRACE;
  if(stage==9)t.rescue_stage=STORY_RESCUE_LOST;
  if(stage==10)t.life_stage=STORY_LIFE_CLINIC;
  if(stage==11)t.life_stage=STORY_LIFE_FINAL_CLINIC;
  if(stage==12)t.life_stage=STORY_LIFE_LAST_WORK;
  if(stage==13)t.life_stage=STORY_LIFE_WAIT;
  if(stage==14)t.life_stage=STORY_LIFE_FOLLOWUP_WAIT;
  for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++){
   if(story_solid(x,y))continue;
   t.x=(uint8_t)x;t.y=(uint8_t)y;
   check_route(&t,&count_routes);
  }
 }
 s.ended=1;assert(!story_route(&s,0,0));
 assert(sizeof(Story)<64u);
 printf("navigation and field guide: %u shortest walkable route cases, ambient interactions, A/B contextual prompts, both resolution guards, and pause navigation passed\n",count_routes);
 return 0;
}
