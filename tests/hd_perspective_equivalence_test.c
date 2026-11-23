#include "hd_canvas.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Host-only validator is deliberately excluded from the cartridge. */
int hd_first_person_anchor_map_valid(void);
unsigned hd_debug_road_rom_category(int tx,int ty,unsigned cx,unsigned cy);
static unsigned original_road_texel(unsigned x,unsigned y){
 uint32_t n=(x+31u)*1664525u+(y+79u)*1013904223u;
 n^=n>>13;n*=2246822519u;return (unsigned)(n^(n>>11));
}
static void verify_original_road_material_rom(void){
 unsigned checks=0u,nonroad=0u;
 for(int ty=0;ty<STORY_H;ty++)for(int tx=0;tx<STORY_W;tx++){
  int road=story_tile(tx,ty)=='.';
  for(unsigned cy=0;cy<16u;cy++)for(unsigned cx=0;cx<16u;cx++){
   unsigned got=hd_debug_road_rom_category(tx,ty,cx,cy);
   if(!road){assert(got==255u);nonroad++;continue;}
   unsigned n=original_road_texel((unsigned)tx*16u+cx,
                                  (unsigned)ty*16u+cy);
   unsigned want=(n&31u)==0u?0u:(n&7u)==1u?1u:2u;
   assert(got==want);checks++;
  }
 }
 assert(checks==240u*256u&&nonroad==192u*256u);
 assert(hd_debug_road_rom_category(-1,0,0,0)==255u);
 assert(hd_debug_road_rom_category(24,0,0,0)==255u);
 puts("packed road ROM: 61440 exact original shade categories, every map tile guarded");
}
void hd_debug_wall_store_span(uint8_t *canvas,int x,int y,int width,uint8_t ink);
void hd_debug_invalidate_wall_cache(void);

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

/* The production wall-row writer packs matching pixels into aligned
   16-bit EWRAM stores. Compare the entire guarded canvas, including
   other rows and the head/tail bytes, against an independent byte loop. */
static void verify_halfword_wall_spans(void){
 static uint8_t expected[HD_BYTES+132u],actual[HD_BYTES+132u];
 const int xs[]={0,1,2,3,22,119,120,231,237,238,239};
 const int ys[]={0,1,79,139,159};
 const unsigned inks[]={0u,29u,32u,43u,63u};
 unsigned cases=0u;
 for(unsigned align=0;align<4u;align++)
  for(unsigned xi=0;xi<sizeof xs/sizeof xs[0];xi++)
   for(unsigned yi=0;yi<sizeof ys/sizeof ys[0];yi++)
    for(int width=1;width<=11;width++){
     int x=xs[xi],y=ys[yi];
     if(x+width>HD_W)continue;
     for(unsigned k=0;k<sizeof inks/sizeof inks[0];k++){
      memset(expected,0xA7,sizeof expected);
      memset(actual,0xA7,sizeof actual);
      uint8_t *want=expected+64u+align,*got=actual+64u+align;
      memset(want,HD_ROAD,HD_BYTES);
      memset(got,HD_ROAD,HD_BYTES);
      for(int col=0;col<width;col++)
       want[y*HD_W+x+col]=(uint8_t)inks[k];
      hd_debug_wall_store_span(got,x,y,width,(uint8_t)inks[k]);
      assert(memcmp(expected,actual,sizeof expected)==0);
      cases++;
     }
    }
 assert(cases>6000u);
 printf("aligned wall spans: %u exact guarded pixel/alignment cases\\n",cases);
}
/* Every warm ray cache must reproduce the original uncached image,
   including when rain, clouds, animation and facing change after a draw.
   Source graphics and actor overlays remain the authority. */
static void verify_camera_shade_cache(void){
 static uint8_t cold[HD_BYTES];
 const int places[][2]={{20,11},{12,13},{11,8},{18,13},{7,7}};
 const unsigned moments[]={0u,71u,433u};
 Story s;story_new(&s,0);
 s.dialogue=0;s.cinema=CINE_NONE;s.battle=0;s.scene=STORY_SCENE_NONE;
 s.anim=0;s.activity=0;s.ended=0;
 unsigned cases=0;
 for(unsigned place=0;place<sizeof places/sizeof places[0];place++){
  s.x=(uint8_t)places[place][0];s.y=(uint8_t)places[place][1];
  assert(!story_solid(s.x,s.y));
  s.prev_x=s.x;s.prev_y=s.y;
  for(unsigned facing=0;facing<4u;facing++){
   s.facing=(uint8_t)facing;
   for(unsigned k=0;k<sizeof moments/sizeof moments[0];k++){
    unsigned tick=moments[k];
    hd_debug_invalidate_wall_cache();
    memset(&canvas,0xA6,sizeof canvas);
    hd_first_person_draw(canvas.pixels,&s,tick);
    memcpy(cold,canvas.pixels,HD_BYTES);
    memset(&canvas,0xA6,sizeof canvas);
    hd_first_person_draw(canvas.pixels,&s,tick); /* warm, same clock */
    assert(!memcmp(cold,canvas.pixels,HD_BYTES));
    cases++;
    memset(&canvas,0xA6,sizeof canvas);
    hd_first_person_draw(canvas.pixels,&s,tick+13u); /* warm, animated */
    memcpy(cold,canvas.pixels,HD_BYTES);
    hd_debug_invalidate_wall_cache();
    memset(&canvas,0xA6,sizeof canvas);
    hd_first_person_draw(canvas.pixels,&s,tick+13u); /* cold, animated */
    assert(!memcmp(cold,canvas.pixels,HD_BYTES));
    cases++;
    for(unsigned sentinel=0;sentinel<32u;sentinel++)
     assert(canvas.before[sentinel]==0xA6u&&canvas.after[sentinel]==0xA6u);
   }
  }
 }
 assert(cases==120u);
 printf("camera wall-shade cache: %u cold/warm native full-frame matches\\n",cases);
}
int main(void){
 verify_original_road_material_rom();
 verify_halfword_wall_spans();
 verify_camera_shade_cache();
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
