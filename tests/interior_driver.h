#pragma once
#include "story.h"
#include "interior_tasks.h"
#include <assert.h>
#include <string.h>
enum { IN_A=1,IN_B=2,IN_SELECT=4,IN_START=8,IN_RIGHT=16,IN_LEFT=32,IN_UP=64,IN_DOWN=128,IN_R=256 };
static const uint16_t in_keys[4]={IN_RIGHT,IN_DOWN,IN_LEFT,IN_UP};
static const int8_t in_dx[4]={1,0,-1,0},in_dy[4]={0,1,0,-1};
static inline void in_press(Story *s,unsigned key){story_tick(s,0);story_tick(s,(uint16_t)key);story_tick(s,0);}
static inline int in_walkable(char c){return c=='.'||c=='D'||c=='>'||c=='<';}
static inline void in_move(Story *s,unsigned d){
 while(s->cooldown)story_tick(s,0);
 in_press(s,in_keys[d]);
}
/* Reach an actual door tile or stand adjacent to a solid workstation.
   Recompute a shortest path over the production collision query. */
static inline void in_seek(Story *s,char target){
 int id=story_interior_id(s);
 assert(id>=0&&s->page==IN_WALK);
 for(unsigned attempt=0;attempt<60u;attempt++){
  unsigned start=s->interior_pos,room=s->interior_room;
  int tx=-1,ty=-1;
  for(int y=0;y<6;y++)for(int x=0;x<10;x++)
   if(story_interior_tile((unsigned)id,room,x,y)==target){tx=x;ty=y;}
  assert(tx>=0&&ty>=0);
  int isdoor=in_walkable(target);
  int x=(int)(start%10u),y=(int)(start/10u);
  if(isdoor&&x==tx&&y==ty){assert(story_interior_action(s)==target);return;}
  if(!isdoor&&!in_walkable(story_interior_tile((unsigned)id,room,tx,ty))){
   for(unsigned d=0;d<4u;d++)if(x+in_dx[d]==tx&&y+in_dy[d]==ty&&
       story_interior_tile((unsigned)id,room,x,y)=='.'){
    in_move(s,d); /* turn against the physical counter, never through it */
    assert(s->interior_pos==start&&story_interior_action(s)==target);
    return;
   }
  }
  uint8_t q[60],first[60];
  memset(first,255,sizeof first);first[start]=4u;q[0]=(uint8_t)start;
  unsigned head=0,tail=1;int direction=-1;
  while(head<tail&&direction<0){
   unsigned pos=q[head++];
   int ax=(int)(pos%10u),ay=(int)(pos/10u);
   for(unsigned d=0;d<4u;d++){
    int nx=ax+in_dx[d],ny=ay+in_dy[d];
    char c=story_interior_tile((unsigned)id,room,nx,ny);
    if(!in_walkable(c))continue;
    unsigned next=(unsigned)(ny*10+nx);
    if(first[next]!=255u)continue;
    first[next]=(uint8_t)(pos==start?d:first[pos]);q[tail++]=(uint8_t)next;
    int distance=(nx>tx?nx-tx:tx-nx)+(ny>ty?ny-ty:ty-ny);
    if((isdoor&&distance==0)||(!isdoor&&c=='.'&&distance==1)){
     direction=first[next];break;
    }
   }
  }
  assert(direction>=0&&direction<4);
  in_move(s,(unsigned)direction);
  assert(s->interior_pos!=start);
 }
 assert(!"indoor path did not converge");
}
static inline void in_solve(Story *s,unsigned id,unsigned step){
 assert(s->page==IN_GAME&&s->activity_choice==step);
 unsigned mode=interior_mode(id,step);
 if(mode==IN_DIAL){
  for(unsigned i=0;i<8u&&s->choice!=interior_dial_target(id,step);i++)in_press(s,IN_RIGHT);
  assert(s->choice==interior_dial_target(id,step));in_press(s,IN_A);
 }else if(mode==IN_TIMING){
  for(unsigned i=0;i<124u&&interior_timing_position(s)!=interior_timing_target(id,step);i++)
   story_tick(s,0u);
  assert(interior_timing_position(s)==interior_timing_target(id,step));
  story_tick(s,IN_A);story_tick(s,0u);
 }else{
  for(unsigned i=0;i<3u;i++)in_press(s,in_keys[interior_pattern(id,step,i)]);
 }
 assert(s->page==IN_RESULT&&s->activity_choice==step+1u);
}
static inline void in_open(Story *s,unsigned id){
 int x,y;assert(story_neighborhood_position(id,&x,&y));
 s->x=(uint8_t)x;s->y=(uint8_t)y;s->prev_x=s->x;s->prev_y=s->y;
 s->anim=0;s->cooldown=0;s->facing=0;
 in_press(s,IN_R);
 assert(story_interior_id(s)==(int)id&&s->interior_room==0u&&s->interior_pos==54u);
}
