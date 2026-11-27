#include "hd_canvas.h"
#include "neighborhood_save.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
enum {A=1,B=2,SELECT=4,START=8,RIGHT=16,LEFT=32,UP=64,DOWN=128,R=256};
static struct {uint8_t before[32],pixels[HD_BYTES],after[32];} frame;
static const uint32_t expected[42]={0x056c5df2u,0x8449fa74u,0x8d74dd3eu,0xc3f67fa4u,0x2a18805cu,0xfe1cf3ffu,0x4b9beeb3u,0xf123cef3u,0x3bab5b09u,0x8acf5cb8u,0x9f763248u,0xe12023aau,0x69ba85bdu,0xc6b39afdu,0xd3a9314fu,0xa0101a89u,0x3758c266u,0x63986615u,0x10042bc7u,0x6d29b22du,0x665b4936u,0x98f498f5u,0x1dd1e0c6u,0x723bf0b6u,0x89812402u,0x9ebc31f2u,0x225b169eu,0x8217b567u,0x4825199bu,0x4211abbeu,0xab41d88du,0x199ff238u,0x325110afu,0x0abe555eu,0x4aa9f709u,0xb2ff2d7eu,0x8c83a74cu,0x4d2ec278u,0x79576d30u,0x58715a3bu,0xdca0c072u,0x25dcd425u};
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
  /* Both outcome-specific follow-ups reuse the actual matching panel,
     never overwrite the selected outcome, and link to a completed site. */
  static const unsigned pair[14]={9,7,12,13,11,10,8,1,6,0,5,4,2,3};
  for(unsigned b=0;b<2u;b++){
   Story v=s;v.neighborhood_seen=(uint16_t)(1u<<i);
   v.neighborhood_right=b?v.neighborhood_seen:0u;
   assert(story_neighborhood_revisit(&v)==(int)i);
   press(&v,R);int n=0;const StoryPage *p=story_pages(&v,&n);
   assert(n==2&&p&&p[0].line1[0]);unsigned dialogue=v.dialogue;
   assert(story_neighborhood_panel(&v)==(int)(i*3+1+b));render(&v,i*3u+1u+b);
   done(&v);assert(v.neighborhood_seen==(1u<<i)&&v.neighborhood_right==(b?(1u<<i):0u));
   v.neighborhood_seen|=(uint16_t)(1u<<pair[i]);
   press(&v,R);p=story_pages(&v,&n);
   assert(v.dialogue==dialogue&&n==3&&p[2].line1[0]);
   press(&v,B);assert(!v.dialogue&&!v.activity);
   assert(v.neighborhood_seen==((1u<<i)|(1u<<pair[i]))&&v.energy==energy&&v.bond==bond&&v.work==work);
   v.life_stage=STORY_LIFE_CLINIC;assert(story_neighborhood_revisit(&v)<0);
   v.life_stage=STORY_LIFE_NONE;v.rescue_stage=STORY_RESCUE_MISSING;
   assert(story_neighborhood_revisit(&v)<0);
  }
  assert(story_neighborhood_count(&s)==i+1u);
  assert(s.neighborhood_target==0u);
 }
 assert(s.neighborhood_seen==STORY_NEIGHBORHOOD_MASK&&s.neighborhood_right==STORY_NEIGHBORHOOD_MASK);
 assert(s.flags==flags&&s.energy==energy&&s.spark==spark&&s.bond==bond&&s.work==work);
 uint8_t raw[NEIGHBORHOOD_EXT_LEN];neighborhood_save_encode(&s,raw);
 Story loaded;story_new(&loaded,0);assert(neighborhood_save_decode(&loaded,raw));
 assert(loaded.neighborhood_seen==s.neighborhood_seen&&loaded.neighborhood_right==s.neighborhood_right);
 loaded.flags=STORY_ALL;done(&loaded);stand(&loaded,4);
 assert(story_neighborhood_revisit(&loaded)==4);press(&loaded,R);
 {int n=0;story_pages(&loaded,&n);assert(n==3&&story_neighborhood_panel(&loaded)==14);}
 done(&loaded);
 for(unsigned i=0;i<sizeof raw;i++){raw[i]^=1u;assert(!neighborhood_save_decode(&loaded,raw));raw[i]^=1u;}
 for(unsigned id=0;id<=STORY_TRACK_MAX;id++){
  s.neighborhood_target=(uint8_t)id;neighborhood_save_encode(&s,raw);
  assert(raw[2]==2u&&raw[8]==id&&neighborhood_save_decode(&loaded,raw));
  assert(loaded.neighborhood_target==id);
 }
 raw[8]=STORY_TRACK_MAX+1u;raw[9]=neighborhood_crc(raw);
 Story unchanged=loaded;assert(!neighborhood_save_decode(&loaded,raw));assert(!memcmp(&loaded,&unchanged,sizeof loaded));
 raw[2]=1u;raw[8]=0xB6u;raw[9]=neighborhood_crc(raw);
 assert(neighborhood_save_decode(&loaded,raw)&&!loaded.neighborhood_target);
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
