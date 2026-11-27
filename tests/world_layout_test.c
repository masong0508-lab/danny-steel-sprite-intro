#include "story.h"
#include "world_save.h"
#include "brush_save.h"
#include "hd_canvas.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const int dx[]={1,0,-1,0},dy[]={0,1,0,-1};
static const unsigned keys[]={16u,128u,32u,64u};
static uint8_t reached[STORY_H][STORY_W];
static uint16_t queue[STORY_W*STORY_H];
static struct {uint8_t pre[32],p[HD_BYTES],post[32];} frame;
static uint8_t prior[HD_BYTES];
static unsigned canonical_count[256];
/* Immutable layout hashes captured from the original 96 seed recipes.
   A later generator must never silently relocate an existing save. */
static const uint32_t legacy_hash[STORY_WORLD_BASE_LAYOUTS]={
 0x0a399113u,0xaf659447u,0xf8140befu,0xd2ebc3c7u,0x3ccdc95fu,0x23194643u,
 0x73a1cfabu,0x469f75c3u,0x7b00627fu,0x7f9ef4dbu,0x8266f667u,0xa353414bu,
 0xa5a25ebfu,0xdfd52b7bu,0x422ef8c7u,0x5745346bu,0xd5a7d8c3u,0x2bcce0a7u,
 0x72bba0bbu,0x0972a757u,0xa25863dfu,0x99b65933u,0x5a997477u,0x9df89563u,
 0xd25779efu,0x4c0202cfu,0xd289806bu,0xbdeb9ed3u,0x4de53dabu,0xfe93420bu,
 0x005d6837u,0xc495641fu,0xd4323f2bu,0x3d253193u,0xf727c53bu,0x8a72a863u,
 0xab7d4f8bu,0x367dd613u,0x4c2ec05bu,0xc03329e3u,0xe459af1fu,0xd5daba5fu,
 0x91d5951fu,0x98944ae7u,0xadcdb833u,0x4f636e73u,0x5b499e33u,0xd82b3d3bu,
 0xa52e35dbu,0x56686967u,0xa6c9ccc7u,0xc1dbefe7u,0x05646c7bu,0x16306bc7u,
 0x69f3eaa7u,0x81a3f247u,0x12958333u,0x3bc71aefu,0xcbaf4e7fu,0xdf85e853u,
 0xfdfa7bb3u,0xf33dcc4fu,0x9b79245fu,0x3a5c8bd3u,0xfddc74ffu,0x7c8c249bu,
 0xbda8474fu,0x4974fcf7u,0x75da416fu,0xdceb3fcbu,0xb35f3cffu,0xe1ab5fc7u,
 0x64aa3903u,0xcac87667u,0xf28390d3u,0xbc1a6303u,0xdaad951fu,0xf075aef3u,
 0xb7a5506fu,0x321dbf1fu,0x41a856bbu,0xb516dd6fu,0xbbc6cbbbu,0x47b6c4bfu,
 0x62120c7fu,0x4f8689c3u,0xdc30817fu,0x0e332ad3u,0x25939bcbu,0xf327bbd7u,
 0x7b1d6d0fu,0x486d3597u,0xed26fdbbu,0x7da0fca7u,0x8cac04bfu,0x0363b867u,
};
extern int hd_first_person_anchor_map_valid(void);
static void tap(Story *s,unsigned key){
 story_tick(s,0);story_tick(s,(uint16_t)key);story_tick(s,0);
}
static void done(Story *s){
 for(unsigned i=0;i<40u&&s->dialogue;i++)tap(s,1u);
 assert(!s->dialogue);
}
static void stand(Story *s,int x,int y,unsigned facing){
 story_world_place(&x,&y);
 assert(!story_solid(x,y));s->x=s->prev_x=(uint8_t)x;s->y=s->prev_y=(uint8_t)y;
 s->facing=(uint8_t)story_world_facing(facing);s->anim=s->cooldown=0;s->old_keys=0;
}
static unsigned flood(const Story *s){
 memset(reached,0,sizeof reached);
 unsigned read=0,write=0;
 reached[s->y][s->x]=1u;queue[write++]=(uint16_t)(s->y*STORY_W+s->x);
 while(read<write){
  unsigned id=queue[read++];int x=(int)(id%STORY_W),y=(int)(id/STORY_W);
  for(unsigned d=0;d<4u;d++){
   int nx=x+dx[d],ny=y+dy[d];
   if(story_solid(nx,ny)||reached[ny][nx])continue;
   reached[ny][nx]=1u;queue[write++]=(uint16_t)(ny*STORY_W+nx);
  }
 }
 return write;
}
static void goal_walk(Story *s,int canonical_x,int canonical_y,const char *name){
 int x,y,dir,n;const char *label=0;
 story_world_place(&canonical_x,&canonical_y);
 assert(story_goal(s,&x,&y,&label)&&x==canonical_x&&y==canonical_y&&!strcmp(label,name));
 assert(story_route(s,&dir,&n));
 for(unsigned steps=0;n;steps++){
  assert(dir>=0&&dir<4&&steps<STORY_W*STORY_H);
  int before=n;s->cooldown=0;s->old_keys=0;
  tap(s,keys[dir]);assert(story_route(s,&dir,&n)&&n==before-1);
 }
 if(story_solid(x,y))assert((s->x==x&&(s->y==y-1||s->y==y+1))||(s->y==y&&(s->x==x-1||s->x==x+1)));
 else assert(s->x==x&&s->y==y);
}
static void draw(Story *s,unsigned view){
 memset(&frame,0xA5,sizeof frame);
 if(view)hd_first_person_draw(frame.p,s,121u);
 else hd_isometric_draw(frame.p,s,121u);
 for(unsigned i=0;i<32u;i++)assert(frame.pre[i]==0xA5&&frame.post[i]==0xA5);
 for(unsigned i=0;i<HD_BYTES;i++)assert(frame.p[i]<64u);
}
static uint32_t layout_hash(unsigned seed){
 uint32_t hash=2166136261u;
 for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++)
  hash=(hash^(uint8_t)story_world_tile_for_seed(seed,x,y))*16777619u;
 return hash;
}
static void verify_random_pool(void){
 WorldPool a,b,resumed;
 world_pool_init(&a,12345u,255u);world_pool_init(&b,54321u,255u);
 unsigned prior_seed=255u,different=0;
 uint8_t raw[WORLD_POOL_LEN];
 for(unsigned cycle=0;cycle<3u;cycle++){
  uint8_t seen[STORY_WORLD_LAYOUTS]={0};
  for(unsigned i=0;i<STORY_WORLD_LAYOUTS;i++){
   unsigned seed=world_pool_pick(&a,i+cycle*31u);
   unsigned other=world_pool_pick(&b,i+cycle*31u);
   assert(seed<STORY_WORLD_LAYOUTS&&seed!=prior_seed&&!seen[seed]);
   seen[seed]=1u;prior_seed=seed;different+=(seed!=other);
   world_pool_encode(&a,raw);assert(world_pool_decode(&resumed,raw));
   assert(a.last==resumed.last&&a.rng==resumed.rng&&!memcmp(a.used,resumed.used,sizeof a.used));
   a=resumed;
  }
 }
 assert(different>STORY_WORLD_LAYOUTS);
 for(unsigned i=0;i<WORLD_POOL_LEN;i++){
  raw[i]^=1u;assert(!world_pool_decode(&resumed,raw));raw[i]^=1u;
 }
 world_pool_init(&a,123u,37u);assert(world_pool_pick(&a,456u)!=37u);
 puts("Random New Game: three full shuffled pools without repetitions, per-draw save/load, migration and CRC rejection");
}
int main(void){
 verify_random_pool();
 Story s;story_new(&s,0);done(&s);
 for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++)canonical_count[(uint8_t)story_tile(x,y)]++;
 unsigned next=0;uint8_t used[STORY_WORLD_LAYOUTS]={0};
 uint32_t hashes[STORY_WORLD_LAYOUTS];
 for(unsigned seed=0;seed<STORY_WORLD_LAYOUTS;seed++){
  assert(!used[next]);used[next]=1;next=world_seed_next(next);
  story_new_seeded(&s,0,seed);done(&s);
  assert(story_world_seed()==seed&&hd_first_person_anchor_map_valid());
  assert(!strcmp(story_action_hint(&s),"A SHIFT"));
  int bx=21,by=11;story_world_place(&bx,&by);
  assert(story_world_potheads(bx,by)&&story_tile(bx,by)=='#');
  assert(flood(&s)==1351u);
  unsigned tally[256]={0};
  for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++){
   char t=story_tile(x,y);
   assert(t==story_world_tile_for_seed(seed,x,y));tally[(uint8_t)t]++;
   int sx=x,sy=y;story_world_source(&sx,&sy);story_world_place(&sx,&sy);
   assert(sx==x&&sy==y);
  }
  assert(!memcmp(tally,canonical_count,sizeof tally));
  hashes[seed]=layout_hash(seed);
  if(seed<STORY_WORLD_BASE_LAYOUTS)assert(hashes[seed]==legacy_hash[seed]);
  for(unsigned i=0;i<seed;i++)assert(hashes[i]!=hashes[seed]);
  int px=9,py=25;story_world_place(&px,&py);
  assert(px==(seed>=STORY_WORLD_BASE_LAYOUTS?34:9)&&py==25);
  assert(!strcmp(story_district_name(px,py),"SOUTH PARK"));
  assert(!story_solid(px,py)&&reached[py][px]);
  assert(story_tile(px,py)==story_world_tile_for_seed(0u,9,25));
  /* Verify the main path with actual movement and interactions on
     each generated arrangement, including the promise and bus gate. */
  goal_walk(&s,5,5,"LETTER");s.facing=(uint8_t)story_world_facing(2u);
  tap(&s,1u);done(&s);assert(s.flags&STORY_HOME);
  goal_walk(&s,11,7,"STAMP");
  stand(&s,11,8,3);tap(&s,1u);done(&s);assert(s.flags&STORY_STAMP);
  goal_walk(&s,8,11,"MARA");
  stand(&s,8,12,3);tap(&s,1u);done(&s);assert(s.flags&STORY_MARA);
  s.flags=STORY_FULL;
  goal_walk(&s,19,5,"LAST BUS");
  stand(&s,19,4,1);tap(&s,1u);done(&s);assert(s.choosing);
  tap(&s,2u);assert(!s.choosing);
  for(unsigned id=0;id<STORY_NEIGHBORHOOD_COUNT;id++){
   int x,y;assert(story_neighborhood_position(id,&x,&y)&&!story_solid(x,y));
   assert(reached[y][x]);
   s.x=s.prev_x=(uint8_t)x;s.y=s.prev_y=(uint8_t)y;s.anim=0;
   assert(story_neighborhood_at(&s)==(int)id);
  }
  /* Restore the exact layout before validating the physical saved tile. */
  uint8_t raw[WORLD_EXT_LEN];unsigned restored=999u;
  world_save_encode(0,seed,raw);assert(world_save_decode(0,raw,&restored)&&restored==seed);
  assert(!world_save_decode(1,raw,&restored));
  for(unsigned k=0;k<WORLD_EXT_LEN;k++){
   raw[k]^=1u;assert(!world_save_decode(0,raw,&restored));raw[k]^=1u;
  }
  int sx=s.x,sy=s.y;story_world_set(world_seed_next(seed));
  assert(world_save_decode(0,raw,&restored));
  story_resume_seeded(&s,0,sx,sy,0,0,STORY_FULL,0,0,restored);
  assert(s.x==sx&&s.y==sy&&story_world_seed()==seed);
  done(&s);
  /* Draw each moved storefront and a city parcel in both cameras. */
  stand(&s,20,11,0);
  draw(&s,0);draw(&s,1);
  memcpy(prior,frame.p,HD_BYTES);draw(&s,1);assert(!memcmp(prior,frame.p,HD_BYTES));
  stand(&s,9,25,0);draw(&s,0);draw(&s,1);
 }
 assert(next==0u);
 /* Release lock: ordinary input can't modify either legacy plots or
    the brush. Hiking, artwork and saved terrain data still exist. */
 assert(!story_land_tools_enabled());
 story_new_seeded(&s,0,37u);done(&s);
 unsigned before=story_world_seed();tap(&s,12u);
 assert(!story_debug_menu(&s)&&s.paused&&story_world_seed()==before);
 tap(&s,8u);assert(!s.paused);
 s.choice=STORY_DEBUG_MENU|STORY_DEBUG_CONFIRM;s.paused=1;
 assert(!story_debug_menu(&s));s.choice=0;s.paused=0;
 static const int patches[][2]={{44,8},{54,24},{50,7}};
 for(unsigned i=0;i<3u;i++){
  stand(&s,patches[i][0],patches[i][1],0);
  unsigned style=s.terrain_style,rev=story_brush_revision(0);
  tap(&s,4u);assert(!story_brush_mode(&s)&&!story_brush_editable(&s));
  tap(&s,1u);done(&s);tap(&s,2u);
  assert(s.terrain_style==style&&story_brush_revision(0)==rev);
 }
 stand(&s,42,7,0);tap(&s,1u);done(&s);assert(story_hike_stage(0)==1u);
 printf("Generated city: %u unique arrangements, conserved tile inventory, reachable story and neighborhood sites, live quest routes, guarded two-camera draws, persistent seeds and release terrain lock\n",STORY_WORLD_LAYOUTS);
 return 0;
}
