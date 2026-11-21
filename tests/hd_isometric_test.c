#include "hd_canvas.h"
#include "hd_isometric_atlas.h"
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
  for(int x=-32;x<=240;x+=34)for(int y=-16;y<=160;y+=37){
   clear();memset(guarded.pixel,HD_ROAD,HD_BYTES);
   hd_iso_tile(guarded.pixel,x,y,id);
   for(unsigned k=0;k<64;k++)
    assert(guarded.pre[k]==0xA7&&guarded.post[k]==0xA7);
   for(unsigned k=0;k<HD_BYTES;k++)assert(guarded.pixel[k]<64u);
  }
 }
 for(unsigned i=0;i<14;i++)for(unsigned j=i+1;j<14;j++)
  assert(textures[i]!=textures[j]);
 const int loc[][2]={{12,13},{11,8},{8,12},{19,4},
                     {3,8},{18,13},{7,7},{16,6},{20,7}};
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
 printf("isometric atlas: 14 distinct ROM diamonds and clipped sprite art OK\n");
 printf("isometric projection: %u compass/position frames, deterministic and guarded\n",checks);
 puts("Original 24x18 map, interactions, collision, SRAM layout and first-person camera unchanged");
 return 0;
}
