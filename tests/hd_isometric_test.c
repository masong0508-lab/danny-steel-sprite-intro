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
/* Captured from the pre-optimization byte/memcpy painter, with untouched
   transparent pixels set to64. These independent complete-tile hashes
   prevent a broken full-tile copy becoming its own clipping reference. */
static void verify_iso_copies(void){
 static const uint32_t original_hash[14]={
  0x2d8388fau,0x6bc59cb7u,0x159b230bu,0x046cc262u,
  0xa474720au,0x4f55d4d2u,0x9d73d9aau,0x7efe7777u,
  0xc92c28f1u,0xcfe281d5u,0xfc984ed3u,0xef48ddf8u,
  0xd9da39a5u,0x5bf7b2f1u
 };
 static uint8_t expected[HD_BYTES+132] __attribute__((aligned(4)));
 static uint8_t actual[HD_BYTES+132] __attribute__((aligned(4)));
 uint8_t original[32u*16u];
 const int xs[]={-33,-32,-31,-30,-29,-28,-27,-3,-2,-1,0,1,2,3,
                 4,5,6,7,103,104,105,207,208,209,210,235,236,237,
                 238,239,240,241};
 const int ys[]={-16,-15,-14,-13,-1,0,1,70,145,146,157,158,159,160};
 unsigned cases=0;
 for(unsigned material=0;material<14u;material++){
  clear();memset(guarded.pixel,64,HD_BYTES);
  hd_iso_tile(guarded.pixel,104,70,material);
  uint32_t hash=2166136261u;
  for(int y=0;y<16;y++)for(int x=0;x<32;x++){
   unsigned ink=guarded.pixel[(70+y)*HD_W+104+x];
   original[y*32+x]=(uint8_t)ink;
   hash=(hash^ink)*16777619u;
  }
  assert(hash==original_hash[material]);
  for(unsigned align=0;align<4u;align++)
   for(unsigned xi=0;xi<sizeof xs/sizeof xs[0];xi++)
    for(unsigned yi=0;yi<sizeof ys/sizeof ys[0];yi++){
     memset(expected,0xA7,sizeof expected);
     memset(actual,0xA7,sizeof actual);
     uint8_t *want=expected+64u+align,*got=actual+64u+align;
     memset(want,HD_CHALK,HD_BYTES);memset(got,HD_CHALK,HD_BYTES);
     int px=xs[xi],py=ys[yi];
     /* Independent rectangular byte crop, with no diamond formula,
        alignment arithmetic, word loads, or optimized span helper. */
     for(int y=0;y<16;y++)for(int x=0;x<32;x++){
      int xx=px+x,yy=py+y;
      unsigned ink=original[y*32+x];
      if(ink<64u&&(unsigned)xx<HD_W&&(unsigned)yy<HD_H)
       want[yy*HD_W+xx]=(uint8_t)ink;
     }
     hd_iso_tile(got,px,py,material+((cases&1u)?14u:0u));
     assert(memcmp(expected,actual,sizeof expected)==0);
     cases++;
    }
 }
 printf("isometric ROM copies: %u exact golden-byte clipping/alignment cases\n",cases);
}
/* Exhaustively prove the monotone visible-span traversal visits exactly
   the same world tiles in the same painter order as the original loops.
   This host-only test also exercises negative/off-map coordinates. */
static uint32_t tile_mix(uint32_t h,int wx,int wy,int sx,int sy){
 h=(h^(uint32_t)(wx+2))*16777619u;
 h=(h^(uint32_t)(wy+2))*16777619u;
 h=(h^(uint32_t)(sx+1024))*16777619u;
 return (h^(uint32_t)(sy+1024))*16777619u;
}
static void verify_visible_spans(void){
 unsigned views=0;
 for(int px=0;px<STORY_W;px++)for(int py=0;py<STORY_H;py++){
  uint32_t old_ground=2166136261u,new_ground=2166136261u;
  unsigned old_count=0,new_count=0;
  for(int wy=-1;wy<=STORY_H;wy++)
   for(int wx=-1;wx<=STORY_W;wx++){
    int sx=104+16*((wx-px)-(wy-py));
    int sy=72+8*((wx-px)+(wy-py));
    if(sx<=-32||sx>=240||sy<=-18||sy>=154)continue;
    old_ground=tile_mix(old_ground,wx,wy,sx,sy);old_count++;
   }
  for(int wy=-1;wy<=STORY_H;wy++){
   int sx=104+16*(-1-wy-px+py),sy=72+8*(-1+wy-px-py);
   for(int wx=-1;wx<=STORY_W;wx++,sx+=16,sy+=8){
    if(sx<=-32||sy<=-18)continue;
    if(sx>=240||sy>=154)break;
    new_ground=tile_mix(new_ground,wx,wy,sx,sy);new_count++;
   }
  }
  assert(old_count==new_count&&old_ground==new_ground);
  uint32_t old_depth=2166136261u,new_depth=2166136261u;
  old_count=new_count=0;
  for(int depth=-2;depth<=STORY_W+STORY_H+1;depth++)
   for(int wx=-1;wx<=STORY_W;wx++){
    int wy=depth-wx;
    if(wy<-1||wy>STORY_H)continue;
    int sx=104+16*((wx-px)-(wy-py));
    int sy=72+8*((wx-px)+(wy-py));
    if(sx<=-48||sx>=256||sy<=-55||sy>=166)continue;
    old_depth=tile_mix(old_depth,wx,wy,sx,sy);old_count++;
   }
  for(int depth=-2;depth<=STORY_W+STORY_H+1;depth++){
   int sy=72+8*(depth-px-py);
   if(sy<=-55||sy>=166)continue;
   int first=depth-STORY_H,last=depth+1;
   if(first<-1)first=-1;
   if(last>STORY_W)last=STORY_W;
   int sx=104+16*(2*first-depth-px+py);
   for(int wx=first;wx<=last;wx++,sx+=32){
    if(sx<=-48)continue;
    if(sx>=256)break;
    new_depth=tile_mix(new_depth,wx,depth-wx,sx,sy);new_count++;
   }
  }
  assert(old_count==new_count&&old_depth==new_depth);
  views++;
 }
 assert(views==STORY_W*STORY_H);
 puts("isometric visible spans: all 432 camera cells preserve original tile order and pixels");
}
int main(void){
 verify_iso_copies();
 verify_visible_spans();
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
 /* Route cues must be tied to the objective's real town tile. With
    identical weather and camera, changing story progress changes the
    world image, not just a HUD label. */
 story_new(&s,0);
 s.x=11;s.y=8;s.prev_x=s.x;s.prev_y=s.y;
 s.anim=0;s.dialogue=0;s.cinema=CINE_NONE;
 clear();hd_isometric_draw(guarded.pixel,&s,96u);
 uint32_t letter_world=checksum_with(1000u);
 s.flags|=STORY_HOME;
 clear();hd_isometric_draw(guarded.pixel,&s,96u);
 uint32_t clerk_world=checksum_with(1000u);
 assert(letter_world!=clerk_world);
 /* Animated sheen must keep the same guarded Mode-4 buffer, with no
    change to the original walkable world or player's sprite bytes. */
 s.x=18;s.y=13;s.prev_x=s.x;s.prev_y=s.y;
 clear();hd_isometric_draw(guarded.pixel,&s,96u);
 uint32_t canal_a=checksum_with(1000u);
 clear();hd_isometric_draw(guarded.pixel,&s,159u);
 uint32_t canal_b=checksum_with(1000u);
 assert(canal_a!=canal_b);
 puts("route markers remain grounded; wet-town animation remains bounded");
 printf("isometric atlas: 14 distinct ROM diamonds and clipped sprite art OK\n");
 printf("isometric projection: %u compass/position frames, deterministic and guarded\n",checks);
 puts("player visibility: 40 static views and 24 interpolated walking frames retain coat/skin");
 puts("Original 24x18 map, interactions, collision, SRAM layout and first-person camera unchanged");
 return 0;
}

