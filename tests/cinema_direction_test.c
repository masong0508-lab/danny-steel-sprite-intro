#include "cinema.h"
#include "story.h"
#include "story_draw.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/* Directly exercises actual 120x80 film frames; no mocks or screenshots
   substituted for the authored source renderer. */
static struct {
 uint8_t a[32],pixels[120*80],b[32];
} out;
static uint8_t earlier[120*80];
static void verify(void){
 for(int i=0;i<32;i++)assert(out.a[i]==0xF3&&out.b[i]==0xF3);
 for(int i=0;i<120*80;i++)assert(out.pixels[i]<32);
}
static void draw_scene(unsigned scene,unsigned tick,unsigned pg,unsigned fin){
 memset(&out,0xF3,sizeof(out));
 cinema_draw(out.pixels,scene,tick,pg,fin);
 verify();
}
static unsigned difference(const uint8_t *a,const uint8_t *b,
                          int x0,int y0,int x1,int y1){
 unsigned n=0;
 for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++)
  n+=(a[y*120+x]!=b[y*120+x]);
 return n;
}
static void direction(unsigned scene,unsigned pg,int x0,int y0,int x1,int y1){
 draw_scene(scene,20,pg,20);memcpy(earlier,out.pixels,sizeof(earlier));
 draw_scene(scene,160,pg,160);
 assert(difference(earlier,out.pixels,x0,y0,x1,y1)>=9u);
 draw_scene(scene,160,pg,160);
 assert(!memcmp(earlier,out.pixels,sizeof(earlier))==0); /* not a frozen scene */
}
int main(void){
 story_draw_init();
 /* Walking and hand-to-prop animation actually change image regions. */
 direction(CINE_LETTER,1,35,29,102,77);
 direction(CINE_BORE,2,22,29,114,77);
 direction(CINE_MARA,2,22,23,110,77);
 direction(CINE_RESCUE_DOOR,0,28,30,103,77);
 direction(CINE_RESCUE_SAVED,1,25,25,113,77);
 direction(CINE_ART,1,25,20,111,77);
 direction(CINE_NOTICE,0,28,20,93,77);
 direction(CINE_CLINIC,1,20,25,113,77);
 /* Café architecture persists; Mara is not inadvertently on screen. */
 draw_scene(CINE_MARA,168,2,0);
 uint8_t cafe[120*80];memcpy(cafe,out.pixels,sizeof(cafe));
 draw_scene(CINE_MARA_EMPTY,168,2,0);
 for(int y=18;y<47;y++)for(int x=7;x<28;x++)
  assert(out.pixels[y*120+x]==cafe[y*120+x]);
 assert(difference(cafe,out.pixels,72,37,105,75)>50u);
 /* Finishing the neutral canvas cannot make brushwork vanish later. */
 draw_scene(CINE_FINALE,420,0,420);
 memcpy(earlier,out.pixels,sizeof(earlier));
 draw_scene(CINE_FINALE,2200,0,420);
 for(int y=21;y<43;y++)for(int x=72;x<98;x++)
  assert(out.pixels[y*120+x]==earlier[y*120+x]);
 /* The sky must continue animating, even after the character pose holds. */
 assert(difference(earlier,out.pixels,2,6,69,42)>10u);
 /* Explicit memory context: the two bad endings must not confuse Mara's fate. */
 assert(cinema_ending_scene(STORY_END_BAD,800,0)==CINE_MARA_EMPTY);
 assert(cinema_ending_scene(STORY_END_BAD,800,1)==CINE_MARA);
 assert(cinema_ending_scene(STORY_END_BAD,1700,0)==CINE_RESCUE_DOOR);
 assert(cinema_ending_scene(STORY_END_BAD,1700,1)==CINE_RESCUE_SAVED);
 assert(cinema_ending_scene(STORY_END_GOOD,800,0)==
        cinema_ending_scene(STORY_END_GOOD,800,1));
 assert(cinema_ending_shot(STORY_END_BAD,4349)==10u);
 const unsigned cuts[]={0,350,780,1190,1650,2070,2490,2900,
                        3320,3670,4030,4349};
 for(unsigned is_saved=0;is_saved<2;is_saved++){
  for(unsigned i=0;i<sizeof(cuts)/sizeof(cuts[0]);i++){
   memset(&out,0xF3,sizeof(out));
   cinema_ending_draw_context(out.pixels,STORY_END_BAD,cuts[i],is_saved);
   verify();
   memcpy(earlier,out.pixels,sizeof(earlier));
   memset(&out,0xF3,sizeof(out));
   cinema_ending_draw_context(out.pixels,STORY_END_BAD,cuts[i],is_saved);
   verify();assert(!memcmp(earlier,out.pixels,sizeof(earlier)));
  }
 }
 /* The old public API remains usable by all prior host tests. */
 memset(&out,0xF3,sizeof(out));
 cinema_ending_draw(out.pixels,STORY_END_GOOD,4900);
 verify();
 puts("cinema direction: staged movement, common set, durable painting, honest bad montage and guarded deterministic frames OK");
 return 0;
}
