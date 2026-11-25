#include "hd_canvas.h"
#include "interior_save.h"
#include "interior_driver.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
extern const uint8_t hd_interior_art[1048320];
static struct {uint8_t pre[32],pixels[HD_BYTES],post[32];} frame;
static const uint32_t hashes[42]={0xe17d383du,0x53f41839u,0x277af7ffu,0x25033459u,0xf01fb9f0u,0xa129bc44u,0x252eed7cu,0xcaaf97c0u,0x5c779d17u,0x4c27a53au,0x523a640cu,0x88f729f0u,0x5b14794du,0x333ba42du,0x3c971754u,0xfef11a37u,0x3d1d70d5u,0xcf1acda2u,0x6a12bb45u,0xcccc7593u,0x853c6e1au,0xe7901281u,0x0e3d13bbu,0x32ef8092u,0x764683a5u,0xec3590ceu,0xa77734a7u,0x61195bf5u,0xe1888114u,0x621089c0u,0x61430a17u,0xabf7bf6du,0x984610bfu,0xbb616716u,0x8492c1bfu,0x5ded9a84u,0x54b046c8u,0x7f03cb7cu,0xfefb806eu,0x0540d45eu,0x0743bbc4u,0x9fb09d5du};
static uint32_t fnv(const uint8_t *p,unsigned n){
 uint32_t h=2166136261u;
 for(unsigned i=0;i<n;i++){assert(p[i]>=32u&&p[i]<64u);h=(h^p[i])*16777619u;}
 return h;
}
static void render(const Story *s){
 memset(&frame,0xA9,sizeof frame);hd_interior_draw(frame.pixels,s,95u);
 for(unsigned i=0;i<32u;i++)assert(frame.pre[i]==0xA9u&&frame.post[i]==0xA9u);
 for(unsigned i=0;i<HD_BYTES;i++)assert(frame.pixels[i]<64u);
}
static void verify_assets(void){
 for(unsigned id=0;id<14u;id++){
  const uint8_t *p=hd_interior_art+74880u*id;
  assert(fnv(p,33600u)==hashes[id*3u]);
  assert(fnv(p+33600u,33600u)==hashes[id*3u+1u]);
  assert(fnv(p+67200u,7680u)==hashes[id*3u+2u]);
  for(unsigned j=0;j<3;j++)for(unsigned k=0;k<id*3u+j;k++)assert(hashes[id*3u+j]!=hashes[k]);
  for(unsigned room=0;room<2;room++){
   unsigned total=0,stations[4]={0},doors=0;
   for(int y=0;y<6;y++)for(int x=0;x<10;x++){
    char c=story_interior_tile(id,room,x,y);
    if(in_walkable(c))++total;
    if(c>='1'&&c<='3')++stations[c-'0'];
    if(c=='C')++stations[0];
    if(c=='D'||c=='>'||c=='<')++doors;
   }
   assert(total>=32&&doors==(room?1u:2u));
   if(room)assert(stations[1]==1&&stations[2]==1&&stations[3]==1);
   else assert(stations[0]==1);
  }
 }
 assert(story_interior_tile(0,0,-1,0)=='#'&&story_interior_tile(0,0,10,0)=='#');
}
static void run_task(Story *s,unsigned id,int retries){
 int ox,oy;assert(story_neighborhood_position(id,&ox,&oy));in_open(s,id);
 render(s);
 in_seek(s,'C');in_press(s,IN_A);assert(s->page==IN_BRIEF);render(s);in_press(s,IN_A);
 in_seek(s,'>');in_press(s,IN_A);assert(s->interior_room==1u);
 render(s);
 if(retries){
  in_seek(s,'3');in_press(s,IN_A);assert(s->page==IN_WRONG&&s->activity_choice==0);
  render(s);in_press(s,IN_B);
 }
 for(unsigned step=0;step<3u;step++){
  char station=(char)('1'+step);in_seek(s,station);in_press(s,IN_A);
  assert(s->page==IN_GAME&&s->activity_choice==step);
  render(s);
  if(retries){
   in_press(s,IN_B);assert(s->page==IN_WALK&&s->activity_choice==step);
   in_press(s,IN_A);assert(s->page==IN_GAME);
   unsigned mode=interior_mode(id,step);
   if(mode==IN_DIAL){
    if(s->choice==interior_dial_target(id,step))in_press(s,IN_RIGHT);
    in_press(s,IN_A);
   }else if(mode==IN_TIMING)in_press(s,IN_A);
   else in_press(s,in_keys[(interior_pattern(id,step,0)+1u)&3u]);
   assert(s->page==IN_RETRY&&s->activity_choice==step);
   render(s);in_press(s,IN_A);assert(s->page==IN_GAME);
  }
  in_solve(s,id,step);render(s);in_press(s,IN_A);assert(s->page==IN_WALK);
 }
 assert(s->interior_done&(1u<<id));
 in_seek(s,'<');in_press(s,IN_A);assert(s->interior_room==0u);
 in_seek(s,'D');in_press(s,IN_A);assert(!s->activity&&s->x==ox&&s->y==oy);
}
int main(void){
 verify_assets();
 assert(sizeof(Story)<64u);
 Story s;story_new(&s,0);s.dialogue=0;s.cinema=CINE_NONE;s.flags=STORY_ALL;
 for(unsigned id=0;id<14u;id++){run_task(&s,id,1);assert(story_interior_count(&s)==id+1u);}
 assert(s.interior_done==STORY_NEIGHBORHOOD_MASK);
 unsigned energy=s.energy,bond=s.bond,spark=s.spark,work=s.work;
 run_task(&s,0,0);run_task(&s,1,0);run_task(&s,3,0);
 assert(s.energy==energy&&s.bond==bond&&s.spark==spark&&s.work==work);
 assert(s.flags==STORY_ALL&&s.life_stage==STORY_LIFE_NONE);
 /* A completed workshop can change later dialogue without replacing the
    original promise, clinic, or bus conditions. */
 s.flags=STORY_ALL;s.x=8;s.y=12;s.facing=3;s.daily_mask|=STORY_PREBUS_MARA;
 in_press(&s,IN_A);int n=0;const StoryPage *pages=story_pages(&s,&n);
 assert(n>0&&!strcmp(pages[0].line1,"People say you helped."));
 in_press(&s,IN_B);
 s.x=11;s.y=8;s.facing=3;in_press(&s,IN_A);
 pages=story_pages(&s,&n);assert(n>0&&!strcmp(pages[0].line1,"The annex is sorted."));
 in_press(&s,IN_B);
 /* Save record is slot-tagged, CRC-protected and backward-compatible. */
 uint8_t save[INTERIOR_EXT_LEN];interior_save_encode(&s,save);
 Story restored;story_new(&restored,0);assert(interior_save_decode(&restored,save));
 assert(restored.interior_done==s.interior_done);
 for(unsigned i=0;i<sizeof save;i++){save[i]^=1u;assert(!interior_save_decode(&restored,save));save[i]^=1u;}
 restored.slot=1;assert(!interior_save_decode(&restored,save));
 memset(save,0xff,sizeof save);assert(!interior_save_decode(&restored,save));
 /* Appointment interruptions and a manual exit return to the same street. */
 s.life_stage=STORY_LIFE_WAIT;s.day_number=1;s.day_frame=STORY_DAY_FRAMES-20u;
 in_open(&s,8);int ox=s.x,oy=s.y;
 for(unsigned i=0;i<30u;i++)story_tick(&s,0);
 assert(s.activity!=STORY_ACT_INTERIOR&&s.life_stage==STORY_LIFE_CLINIC&&s.x==ox&&s.y==oy);
 assert(s.interior_done==STORY_NEIGHBORHOOD_MASK);
 puts("14 buildings, 28 traversed rooms, 42 playable rounds, 42 ROM art hashes, retries, replays, save and interruption OK");
 return 0;
}
