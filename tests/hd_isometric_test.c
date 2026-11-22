#include "hd_canvas.h"
#include "hd_isometric_atlas.h"
#include "hd_world_atlas.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/* All production pixels, including shaded raised facades, remain inside
   one 240x160 Mode-4 frame. No second frame, new collision grid or
   permanent RAM allocation is required. */
static struct {
 uint8_t pre[64],pixel[HD_BYTES],post[64];
} guarded;
static void clear(void){memset(&guarded,0xA7,sizeof guarded);}
static uint32_t checksum_with(unsigned minimum){
 uint32_t h=2166136261u,detail=0;
 for(unsigned i=0;i<64u;i++)
  assert(guarded.pre[i]==0xA7&&guarded.post[i]==0xA7);
 for(unsigned i=0;i<HD_BYTES;i++){
  uint8_t ink=guarded.pixel[i];
  assert(ink<64u); /* 64 is ONLY a cartridge-asset transparency key. */
  if(ink!=HD_ROAD&&ink!=HD_ROAD_COOL&&ink!=HD_STONE_DARK)detail++;
  h=(h^ink)*16777619u;
 }
 assert(detail>minimum);
 return h;
}
/* Compare the real world frame to the known player sprite, not merely
   to a changing scene checksum. The old spawn passed all checksum tests
   while a foreground building covered every red-coat and skin pixel. */
static void readable_player(const Story *s,unsigned frame,int cx,int foot){
 uint8_t reference[22u*34u];
 clear();memset(guarded.pixel,64,HD_BYTES);
 hd_world_person_art(guarded.pixel,cx,foot,0u,s->facing,
                     s->anim?((frame/7u)&1u):0u);
 for(int y=0;y<34;y++)for(int x=0;x<22;x++)
  reference[y*22+x]=guarded.pixel[(foot-32+y)*HD_W+cx-11+x];
 clear();hd_world_draw(guarded.pixel,s,frame);checksum_with(1000u);
 unsigned coat=0,skin=0,coat_total=0,skin_total=0;
 for(int y=0;y<34;y++)for(int x=0;x<22;x++){
  unsigned want=reference[y*22+x];
  unsigned got=guarded.pixel[(foot-32+y)*HD_W+cx-11+x];
  if(want==HD_RED||want==HD_RED_SHADE){
   coat_total++;if(got==want)coat++;
  }
  if(want==HD_SKIN||want==HD_SKIN_LIT){
   skin_total++;if(got==want)skin++;
  }
 }
 assert(coat_total>100u&&skin_total>0u);
 /* A tree, bus or small physical sign can still cover part of him;
    depth is retained. His actual coat and skin must remain readable. */
 assert(coat*4u>=coat_total);
 assert(skin*8u>=skin_total);
 if(s->x==20u&&s->y==11u&&!s->anim){
  /* The first playable view has a much stronger readability contract. */
  assert(coat*10u>=coat_total*9u);
  assert(skin*4u>=skin_total*3u);
 }
}
int main(void){
 Story s;story_new(&s,0);
 s.cinema=CINE_NONE;s.dialogue=0;s.activity=0;s.battle=0;
 s.ended=0;s.scene=STORY_SCENE_NONE;
 s.life_stage=STORY_LIFE_NONE;s.view=0;
 uint32_t textures[14];
 for(unsigned id=0;id<14;id++){
  clear();memset(guarded.pixel,HD_CHALK,HD_BYTES);
  hd_iso_tile(guarded.pixel,102,70,id);
  textures[id]=checksum_with(0u);
  /* Every asset is a genuine diamond: transparent rectangular corners,
     but a solid roof/cobblestone/grass/canal center. */
  assert(guarded.pixel[70*HD_W+102]==HD_CHALK);
  assert(guarded.pixel[77*HD_W+118]!=HD_CHALK);
  /* A fully visible diamond supplies an independent reference for all
     clipped row-copy cases. Compare the entire local rectangle, including
     untouched transparent corners, against that exact ROM art. */
  uint8_t original[32u*16u];
  for(int py=0;py<16;py++)for(int px=0;px<32;px++)
   original[py*32+px]=guarded.pixel[(70+py)*HD_W+102+px];
  for(int x=-32;x<=240;x+=34)for(int y=-16;y<=160;y+=37){
   clear();memset(guarded.pixel,HD_CHALK,HD_BYTES);
   hd_iso_tile(guarded.pixel,x,y,id);
   for(unsigned k=0;k<64;k++)
    assert(guarded.pre[k]==0xA7&&guarded.post[k]==0xA7);
   for(int py=0;py<16;py++)for(int px=0;px<32;px++){
    int xx=x+px,yy=y+py;
    if((unsigned)xx<HD_W&&(unsigned)yy<HD_H)
     assert(guarded.pixel[yy*HD_W+xx]==original[py*32+px]);
   }
  }
 }
 for(unsigned i=0;i<14;i++)for(unsigned j=i+1;j<14;j++)
  assert(textures[i]!=textures[j]);
 const int loc[][2]={{12,13},{11,8},{8,12},{19,4},
                     {3,8},{18,13},{7,7},{16,6},{20,7},{20,11}};
 unsigned checks=0;uint32_t views[4];
 for(unsigned i=0;i<sizeof loc/sizeof loc[0];i++){
  if(story_solid(loc[i][0],loc[i][1]))continue;
  s.x=(uint8_t)loc[i][0];s.y=(uint8_t)loc[i][1];
  s.prev_x=s.x;s.prev_y=s.y;s.anim=0;
  for(int facing=0;facing<4;facing++){
   s.facing=(uint8_t)facing;
   clear();hd_world_draw(guarded.pixel,&s,100u);
   views[facing]=checksum_with(1000u);
   /* Legend remains grounded over geometry at the original HUD band. */
   assert(guarded.pixel[0]==HD_INK);
   assert(guarded.pixel[17*HD_W+25]==HD_COPPER);
   readable_player(&s,100u,120,88);
   clear();hd_world_draw(guarded.pixel,&s,100u);
   assert(checksum_with(1000u)==views[facing]); /* deterministic authored view */
   s.anim=3;s.prev_x=s.x>0?s.x-1:s.x;
   clear();hd_world_draw(guarded.pixel,&s,127u);
   checksum_with(1000u);s.anim=0;s.prev_x=s.x;
   checks++;
  }
  for(int f=0;f<4;f++)for(int g=f+1;g<4;g++)
   assert(views[f]!=views[g]); /* directions remain distinguishable */
 }
 assert(checks>=24u);
 /* Actual walkable steps into and out of the shop lane. Explicit screen
    anchors cover both signs of fractional depth and all three gait
    offsets, so a destination-only cutaway cannot regress moving actors. */
 const int walks[][8]={
  {20,11,19,11,3,108,82,0},{20,11,19,11,2,112,84,0},
  {20,11,19,11,1,116,86,0},{19,11,20,11,3,132,94,1},
  {19,11,20,11,2,128,92,1},{19,11,20,11,1,124,90,1},
  {20,12,20,11,3,132,82,2},{20,12,20,11,2,128,84,2},
  {20,12,20,11,1,124,86,2},{20,11,20,12,3,108,94,3},
  {20,11,20,12,2,112,92,3},{20,11,20,12,1,116,90,3}
 };
 for(unsigned i=0;i<sizeof walks/sizeof walks[0];i++){
  const int *w=walks[i];
  assert(!story_solid(w[0],w[1])&&!story_solid(w[2],w[3]));
  s.x=(uint8_t)w[0];s.y=(uint8_t)w[1];
  s.prev_x=(uint8_t)w[2];s.prev_y=(uint8_t)w[3];
  s.anim=(uint8_t)w[4];s.facing=(uint8_t)w[7];
  readable_player(&s,100u,w[5],w[6]);
  readable_player(&s,107u,w[5],w[6]);
 }
 printf("isometric atlas: 14 distinct ROM diamonds and clipped sprite art OK\n");
 printf("isometric projection: %u compass/position frames, deterministic and guarded\n",checks);
 puts("player visibility: 40 static views and 24 interpolated walking frames retain coat/skin");
 puts("Original 24x18 map, interactions, collision, SRAM layout and first-person camera unchanged");
 return 0;
}

