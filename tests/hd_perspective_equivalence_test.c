#include "hd_canvas.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Host-only validator is deliberately excluded from the cartridge. */
int hd_first_person_anchor_map_valid(void);
unsigned hd_debug_next_tile_sample(int position,int velocity);

/* Captured from the original shipped PR14 painter before changing its
   ray reuse, floor ordering or instruction placement. Each value covers
   every walkable town tile in a fixed facing/animation/weather state.
   The expected image never calls the new fast path to construct itself. */
static const uint32_t original_hash[4][3]={
 {0x6da7864fu,0x1c071308u,0x7d077ed6u},
 {0x5d126328u,0xbdbe6053u,0x3ed5b219u},
 {0x24bf7f71u,0x264f1c4bu,0x736dae2eu},
 {0x095fe454u,0x989838bdu,0xdc05d821u}
};
static struct {
 uint8_t before[32],pixels[HD_BYTES],after[32];
} canvas;

/* The production raycaster jumps to a tile crossing using reciprocal
   multiplication. For every relevant velocity and signed sub-tile phase,
   verify that the answer is the FIRST original 32-unit sample in another
   tile, not a geometric intersection between the original samples. */
static void verify_exact_tile_jumps(void){
 unsigned cases=0u;
 for(int position=-257;position<=511;position++){
  int original_tile=position>>8;
  for(int velocity=-32;velocity<=32;velocity++){
   unsigned actual=hd_debug_next_tile_sample(position,velocity);
   if(!velocity){assert(actual==76u);continue;}
   unsigned expected=1u;
   while(((position+(int)expected*velocity)>>8)==original_tile)
    expected++;
   assert(actual==expected);
   cases++;
  }
 }
 assert(cases==769u*64u);
 printf("exact ray jumps: %u signed sub-tile phases and velocities verified\\n",cases);
}
int main(void){
 verify_exact_tile_jumps();
 assert(hd_first_person_anchor_map_valid());
 Story s;story_new(&s,0);
 const unsigned ticks[3]={0,71,433};
 const int dx[4]={1,0,-1,0},dy[4]={0,1,0,-1};
 unsigned cases=0;
 for(unsigned facing=0;facing<4;facing++)
  for(unsigned mode=0;mode<3;mode++){
   uint32_t hash=2166136261u;
   for(int y=1;y<STORY_H-1;y++)for(int x=1;x<STORY_W-1;x++){
    if(story_solid(x,y))continue;
    s.x=(uint8_t)x;s.y=(uint8_t)y;s.facing=(uint8_t)facing;
    s.prev_x=(uint8_t)(x-dx[facing]);
    s.prev_y=(uint8_t)(y-dy[facing]);
    s.anim=(uint8_t)(mode?mode+1u:0u);
    s.view=1;s.cinema=CINE_NONE;
    memset(&canvas,0xA6,sizeof canvas);
    hd_first_person_draw(canvas.pixels,&s,ticks[mode]);
    for(unsigned i=0;i<32u;i++)
     assert(canvas.before[i]==0xA6u&&canvas.after[i]==0xA6u);
    for(unsigned i=0;i<HD_BYTES;i++){
     assert(canvas.pixels[i]<64u);
     hash=(hash^canvas.pixels[i])*16777619u;
    }
    cases++;
   }
   assert(hash==original_hash[facing][mode]);
  }
 assert(cases==3456u);

 /* Independently check the depth inequality at every possible sampled
    wall distance and floor row, including original height clamp edges. */
 unsigned depth_cases=0;
 for(int depth=32;depth<=2300;depth++)for(int y=80;y<140;y++){
  int height=20800/(depth+60);
  if(height>150)height=150;
  if(height<3)height=3;
  int covered=y<79+height/2;
  assert(covered==(depth<=10400/(y-78)-60));
  depth_cases++;
 }
 printf("native perspective: %u shipped-image golden frames, all town positions, four facings and guarded pixels\n",cases);
 printf("floor occlusion: %u exact original wall-bottom comparisons\n",depth_cases);
 return 0;
}
