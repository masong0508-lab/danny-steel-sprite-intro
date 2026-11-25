#include "hd_canvas.h"
#include "neighborhood_save.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
enum {A=1,B=2,SELECT=4,START=8,RIGHT=16,LEFT=32,UP=64,DOWN=128};
static struct {uint8_t before[32],pixels[HD_BYTES],after[32];} frame;
static const uint32_t expected[42]={0x6cc5e718u,0x67b1e486u,0x4538944au,0xf4dcb912u,0xeda58ceeu,0x9cf17cf9u,0xd16043a2u,0x94b58ac3u,0x56dcb2d3u,0x4327b9f2u,0x5abfec0au,0xefc6e7dau,0xed3d29d9u,0xf4ef2be9u,0x13b49d61u,0x6329e719u,0x1f114d87u,0x899ab820u,0xf8fc8c40u,0x39213a9du,0xbba760ecu,0xc4d6fec3u,0x3a64be30u,0x067dee6au,0xebae1bf6u,0x6e9559d8u,0x277e6c81u,0xc8632edcu,0xf7ef860du,0x315dbc80u,0x7ace5ca4u,0x21d958c1u,0xb649faccu,0xf7c4eac7u,0x2c605bd1u,0xf506f15au,0x81fdae4eu,0x89b1a44au,0xd29ee1f5u,0x97c01747u,0xb1274c82u,0xec0b816fu};
static void press(Story *s,unsigned k){story_tick(s,0);story_tick(s,(uint16_t)k);story_tick(s,0);}
static void done(Story *s){for(unsigned i=0;i<16&&s->dialogue;i++)press(s,A);assert(!s->dialogue);}
static void render(Story *s,unsigned id){
 memset(&frame,0xA9,sizeof frame);hd_cinema_draw(frame.pixels,s,93u);
 uint32_t hash=2166136261u;
 for(unsigned i=0;i<240u*104u;i++){
  assert(frame.pixels[i]>=32u&&frame.pixels[i]<64u);
  hash=(hash^frame.pixels[i])*16777619u;
 }
 assert(hash==expected[id]);hd_game_overlay(frame.pixels,s);
 for(unsigned i=0;i<32u;i++)assert(frame.before[i]==0xA9&&frame.after[i]==0xA9);
 for(unsigned i=0;i<HD_BYTES;i++)assert(frame.pixels[i]<64u);
}
static void stand(Story *s,unsigned id){
 int x=-1,y=-1;assert(story_neighborhood_position(id,&x,&y));assert(!story_solid(x,y));
 s->x=(uint8_t)x;s->y=(uint8_t)y;s->prev_x=s->x;s->prev_y=s->y;s->anim=0;s->cooldown=0;s->facing=0;
}
static void walk_to(Story *s,unsigned id){
 static const unsigned key[4]={RIGHT,DOWN,LEFT,UP};
 int gx,gy;assert(story_neighborhood_position(id,&gx,&gy));
 s->neighborhood_target=(uint8_t)(id+1u);
 for(unsigned i=0;i<STORY_W*STORY_H;i++){
  int dir=-1,steps=-1;assert(story_route(s,&dir,&steps));
  if(steps==0)break;
  assert(dir>=0&&dir<4);s->cooldown=0;s->old_keys=0;
  int before=s->x+STORY_W*s->y;
  story_tick(s,(uint16_t)key[dir]);story_tick(s,0);
  assert(before!=s->x+STORY_W*s->y&&!story_solid(s->x,s->y));
 }
 assert(s->x==gx&&s->y==gy);
}
int main(void){
 Story s;story_new(&s,0);done(&s);stand(&s,0);assert(story_neighborhood_at(&s)<0);
 s.flags=STORY_ALL;
 uint8_t flags=s.flags,energy=s.energy,bond=s.bond,spark=s.spark,work=s.work;
 press(&s,START);press(&s,SELECT);assert(s.paused&&(s.choice&STORY_NEIGHBORHOOD_JOURNAL));
 memset(frame.pixels,HD_ROAD,HD_BYTES);hd_game_overlay(frame.pixels,&s);
 press(&s,DOWN);press(&s,A);assert(!s.paused&&s.neighborhood_target==2u);
 int gx,gy,dir,steps;const char *label=0;
 assert(story_goal(&s,&gx,&gy,&label)&&!strcmp(label,"PHOTO"));
 assert(story_route(&s,&dir,&steps)&&steps>0);
 for(unsigned i=0;i<14u;i++){
  walk_to(&s,i);
  assert(story_neighborhood_at(&s)==(int)i);
  assert(!strcmp(story_action_hint(&s),"A STORY"));
  press(&s,A);assert(story_neighborhood_panel(&s)==(int)i*3);
  render(&s,i*3u);press(&s,A|B);assert(!s.dialogue&&!s.activity);
  assert(!(s.neighborhood_seen&(1u<<i)));
  press(&s,A);done(&s);assert(s.activity==STORY_ACT_NEIGHBORHOOD);
  press(&s,B);assert(!s.activity&&!s.dialogue);
  for(unsigned b=0;b<2u;b++){
   press(&s,A);done(&s);assert(s.activity==STORY_ACT_NEIGHBORHOOD);
   if(b)press(&s,RIGHT);
   press(&s,A);assert(story_neighborhood_panel(&s)==(int)(i*3+1+b));render(&s,i*3u+1u+b);
   if(b==0){press(&s,B);assert(!(s.neighborhood_seen&(1u<<i)));press(&s,A);done(&s);press(&s,A);}
   done(&s);assert(!s.activity&&!s.dialogue&&s.cinema==CINE_NONE);
   assert(s.neighborhood_seen&(1u<<i));assert(!!(s.neighborhood_right&(1u<<i))==b);
  }
  assert(story_neighborhood_count(&s)==i+1u);
  assert(s.neighborhood_target==0u);
 }
 assert(s.neighborhood_seen==STORY_NEIGHBORHOOD_MASK&&s.neighborhood_right==STORY_NEIGHBORHOOD_MASK);
 assert(s.flags==flags&&s.energy==energy&&s.spark==spark&&s.bond==bond&&s.work==work);
 uint8_t raw[NEIGHBORHOOD_EXT_LEN];neighborhood_save_encode(&s,raw);
 Story loaded;story_new(&loaded,0);assert(neighborhood_save_decode(&loaded,raw));
 assert(loaded.neighborhood_seen==s.neighborhood_seen&&loaded.neighborhood_right==s.neighborhood_right);
 for(unsigned i=0;i<sizeof raw;i++){raw[i]^=1u;assert(!neighborhood_save_decode(&loaded,raw));raw[i]^=1u;}
 loaded.slot=1;assert(!neighborhood_save_decode(&loaded,raw));
 memset(raw,0xff,sizeof raw);assert(!neighborhood_save_decode(&loaded,raw));
 s.neighborhood_target=3;s.rescue_stage=STORY_RESCUE_MISSING;
 assert(!story_neighborhood_enabled(&s)&&story_goal(&s,&gx,&gy,&label)&&!strcmp(label,"WITNESS"));
 s.rescue_stage=STORY_RESCUE_NONE;s.life_stage=STORY_LIFE_CLINIC;
 assert(!story_neighborhood_enabled(&s)&&story_goal(&s,&gx,&gy,&label)&&!strcmp(label,"CLINIC"));
 /* Simultaneously canceling the final promise can never grant it. */
 story_new(&s,0);done(&s);s.flags=STORY_HOME|STORY_STAMP;
 s.x=8;s.y=12;s.facing=3;s.old_keys=0;press(&s,A);
 int n=0;story_pages(&s,&n);assert(n>1);
 for(int i=1;i<n;i++)press(&s,A);
 press(&s,A|B);assert(!(s.flags&STORY_MARA)&&!s.dialogue);
 puts("14 walkable sites, 28 outcomes, 42 golden ROM panels, route, cancel and save checks passed");return 0;
}
