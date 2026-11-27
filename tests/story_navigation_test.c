#include "story.h"
#include "story_draw.h"
#include "story_checks.h"
#include "city_furniture.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Independent reverse flood fill from every WALKABLE approach square.
   It does not call the game's pathfinder and makes no assumptions about
   the heuristic or tie-breaking in story_route. */
static int distance[STORY_H][STORY_W];
static int qx[STORY_W*STORY_H],qy[STORY_W*STORY_H];
static const int dX[4]={1,0,-1,0},dY[4]={0,1,0,-1};
static int abs_i(int x){return x<0?-x:x;}
static void reference(int gx,int gy){
 for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++)distance[y][x]=-1;
 int count=0,read=0,solid=story_solid(gx,gy);
 for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++){
  if(story_solid(x,y))continue;
  int d=abs_i(x-gx)+abs_i(y-gy);
  if((solid&&d==1)||(!solid&&d==0)){
   distance[y][x]=0;qx[count]=x;qy[count++]=y;
  }
 }
 while(read<count){
  int cx=qx[read],cy=qy[read++];
  for(int d=0;d<4;d++){
   int nx=cx+dX[d],ny=cy+dY[d];
   if((unsigned)nx>=STORY_W||(unsigned)ny>=STORY_H||
       story_solid(nx,ny)||distance[ny][nx]>=0)continue;
   distance[ny][nx]=distance[cy][cx]+1;
   qx[count]=nx;qy[count++]=ny;
  }
 }
}
static unsigned route_checked=0,route_walks=0;
static void check_stage(Story *s){
 int gx,gy;const char *title;
 assert(story_goal(s,&gx,&gy,&title));
 assert(strlen(story_goal_reason(s))<=27u);
 reference(gx,gy);
 for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++){
  if(story_solid(x,y))continue;
  s->x=(uint8_t)x;s->y=(uint8_t)y;s->prev_x=s->x;s->prev_y=s->y;
  Story snapshot=*s;
  int dir=42,steps=42;
  int route=story_route(s,&dir,&steps);
  assert(!memcmp(s,&snapshot,sizeof(*s)));
  assert(!!route==(distance[y][x]>=0));
  if(route){
   assert(steps==distance[y][x]);
   if(!steps)assert(dir==-1);
   else{
    assert(dir>=0&&dir<4);
    int nx=x+dX[dir],ny=y+dY[dir];
    assert(!story_solid(nx,ny));
    assert(distance[ny][nx]==steps-1);
   }
   route_walks++;
  }else assert(dir==-1&&steps==-1);
  route_checked++;
 }
}
static void click(Story *s,unsigned k){story_tick(s,0u);story_tick(s,(uint16_t)k);story_tick(s,0u);}
static void render_guarded(const Story *s){
 struct{uint8_t guard[32],pixels[120*80],tail[32];} a;
 memset(&a,0xB3,sizeof(a));story_draw(s,13,a.pixels);
 for(int i=0;i<32;i++)assert(a.guard[i]==0xB3&&a.tail[i]==0xB3);
 for(unsigned i=0;i<sizeof a.pixels;i++)assert(a.pixels[i]<32);
}
static void expanded_city(void){
 Story s;story_new(&s,0);s.dialogue=0;s.cinema=CINE_NONE;story_draw_init();
 static const unsigned key[]={16u,128u,32u,64u};
 static const int places[][2]={{35,6},{31,15},{9,25},{22,26},{32,22},{42,13},{56,11},{57,25}};
 static const char *const district[]={"EAST AVENUE","CIVIC ROW","SOUTH PARK","CANAL WALK","RIVER PLAZA","MEADOWS","HIGH RIDGE","FOOTHILLS"};
 /* All new destinations are genuinely connected to the Potheads spawn.
    The two sealed archive rooms already existed in the old map. */
 reference(20,11);unsigned count=0;
 for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++){
  if(story_solid(x,y))continue;
  if(y==5&&(x==11||x==12))continue;
  assert(distance[y][x]>=0);count++;
 }
 assert(count==1351u);
 assert(!story_solid(23,7)&&!story_solid(24,7));
 assert(!story_solid(12,17)&&!story_solid(12,18));
 assert(story_solid(39,10)&&story_solid(10,29)&&story_solid(-1,10)&&story_solid(64,10));
 assert(!story_solid(39,8)&&!story_solid(40,8));
 assert(story_tile(19,7)=='M'&&story_tile(8,11)=='F');
 for(unsigned i=0;i<sizeof city_furniture/sizeof city_furniture[0];i++)
  assert(!story_solid(city_furniture[i].x,city_furniture[i].y));
 unsigned moved=0;
 for(unsigned i=0;i<sizeof places/sizeof places[0];i++){
  int tx=places[i][0],ty=places[i][1];reference(tx,ty);
  for(unsigned n=0;n<STORY_W*STORY_H&&distance[s.y][s.x]>0;n++){
   int before=distance[s.y][s.x],d=-1;
   for(int j=0;j<4;j++){int x=s.x+dX[j],y=s.y+dY[j];
    if(!story_solid(x,y)&&distance[y][x]==before-1){d=j;break;}}
   assert(d>=0);s.cooldown=0;s.old_keys=0;
   click(&s,key[d]);assert(distance[s.y][s.x]==before-1);moved++;
  }
  assert(s.x==tx&&s.y==ty&&!strcmp(story_district_name(tx,ty),district[i]));
  click(&s,1u);assert(s.dialogue&&s.cinema==CINE_NONE);click(&s,2u);
  assert(!s.dialogue);
  for(int v=0;v<2;v++){s.view=(uint8_t)v;render_guarded(&s);}
  s.view=0;
 }
 assert(moved>50u);
 /* Core campaign routing still works after walking far outside the old city. */
 s.flags=0;int dir,steps;
 assert(story_route(&s,&dir,&steps)&&steps>30);
 while(steps){
  int before=steps;s.cooldown=0;s.old_keys=0;
  click(&s,key[dir]);assert(story_route(&s,&dir,&steps)&&steps==before-1);
 }
 printf("city: %u connected tiles, eight districts, live walking, return route and both render guards passed\n",count);
}
int main(void){
 expanded_city();
 Story s;story_new(&s,0);s.dialogue=0;story_draw_init();
 /* The original main story, 24x18 world and optional days, with no new
    collision or hidden plot requirements. */
 static const unsigned masks[]={0u,STORY_HOME,STORY_HOME|STORY_STAMP,
    STORY_ALL,STORY_ALL|STORY_MUSIC,STORY_FULL};
 for(unsigned i=0;i<sizeof masks/sizeof masks[0];i++){
  s.flags=(uint8_t)masks[i];s.life_stage=STORY_LIFE_NONE;
  s.rescue_stage=STORY_RESCUE_NONE;check_stage(&s);
 }
 s.flags=STORY_FULL;
 const unsigned life[]={STORY_LIFE_WAIT,STORY_LIFE_CLINIC,
   STORY_LIFE_FOLLOWUP_WAIT,STORY_LIFE_FINAL_CLINIC,STORY_LIFE_LAST_WORK};
 for(unsigned i=0;i<sizeof life/sizeof life[0];i++){
  s.life_stage=(uint8_t)life[i];check_stage(&s);
 }
 s.life_stage=STORY_LIFE_NONE;
 const unsigned rescue[]={STORY_RESCUE_MISSING,STORY_RESCUE_WITNESS,
   STORY_RESCUE_TRACE,STORY_RESCUE_LOST};
 for(unsigned i=0;i<sizeof rescue/sizeof rescue[0];i++){
  s.rescue_stage=(uint8_t)rescue[i];check_stage(&s);
 }
 s.rescue_stage=STORY_RESCUE_NONE;
 s.x=20;s.y=11;s.flags=0;s.facing=0;
 assert(!strcmp(story_action_hint(&s),"A SHIFT"));
 assert(!strcmp(story_goal_reason(&s),"LETTER / WEST OLD QUARTER"));
 int dir,steps;
 assert(story_route(&s,&dir,&steps)&&steps>0);
 s.x=11;s.y=8;s.facing=3;
 assert(!strcmp(story_action_hint(&s),"A CLERK"));
 s.x=8;s.y=12;s.facing=3;
 assert(!strcmp(story_action_hint(&s),"A MARA"));
 s.rescue_stage=STORY_RESCUE_MISSING;
 assert(!strcmp(story_action_hint(&s),"A EMPTY CAFE"));
 s.rescue_stage=STORY_RESCUE_LOST;
 assert(!strcmp(story_action_hint(&s),"A PHONE"));
 s.rescue_stage=STORY_RESCUE_SAVED;
 assert(!strcmp(story_action_hint(&s),"A MARA"));
 s.rescue_stage=STORY_RESCUE_NONE;
 s.x=12;s.y=13;s.facing=0;
 assert(!strcmp(story_action_hint(&s),"A LOOK"));
 /* The fourth page is a genuine live field guide in BOTH camera modes.
    It must remain reversible and save nothing. */
 for(int view=0;view<2;view++){
  s.view=(uint8_t)view;s.paused=0;s.choice=0;
  click(&s,8);assert(s.paused);
  for(int i=0;i<4;i++)click(&s,256);
  assert(story_check_page(&s)==3);
  render_guarded(&s);
  click(&s,2);assert(!(s.choice&STORY_CHECK_PAGE_ACTIVE));
  click(&s,8);assert(!s.paused&&!s.quit);
 }
 s.ended=1;
 assert(!story_route(&s,&dir,&steps)&&dir==-1&&steps==-1);
 printf("pathfinder: %u walkable origin/goal combinations, %u valid routes; live prompts, fourth journal page, two cameras and frame guards OK\n",route_checked,route_walks);
 return 0;
}
