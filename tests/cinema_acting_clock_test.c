#include "cinema.h"
#include "story.h"
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
/* These samples are produced by the actual 120x80 game renderer, not
   generated reference art. Surrounding canaries detect OOB writes. */
static struct {
 uint8_t left[24],pixels[120*80],right[24];
} a,b;
static void check(const uint8_t *left,const uint8_t *pixels,
                  const uint8_t *right){
 for(int i=0;i<24;i++)assert(left[i]==0xCC&&right[i]==0xCC);
 for(int i=0;i<120*80;i++)assert(pixels[i]<32u);
}
static void scene(unsigned id,unsigned tick,unsigned page,int which){
 if(which==0){
  memset(&a,0xCC,sizeof(a));
  cinema_draw(a.pixels,id,tick,page,tick);
  check(a.left,a.pixels,a.right);
 }else{
  memset(&b,0xCC,sizeof(b));
  cinema_draw(b.pixels,id,tick,page,tick);
  check(b.left,b.pixels,b.right);
 }
}
static void ending(unsigned end,unsigned frame,unsigned safe,int which){
 if(which==0){
  memset(&a,0xCC,sizeof(a));
  cinema_ending_draw_context(a.pixels,end,frame,safe);
  check(a.left,a.pixels,a.right);
 }else{
  memset(&b,0xCC,sizeof(b));
  cinema_ending_draw_context(b.pixels,end,frame,safe);
  check(b.left,b.pixels,b.right);
 }
}
static unsigned diff(int x0,int y0,int x1,int y1){
 unsigned count=0;
 for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++)
  count+=a.pixels[y*120+x]!=b.pixels[y*120+x];
 return count;
}
int main(void){
 /* A listening character does not use the same face/arm pose as a
    speaker even while standing at exactly the same café position. */
 scene(CINE_MARA,180u,0u,0);scene(CINE_MARA,180u,1u,1);
 assert(diff(31,31,55,72)>6u);
 scene(CINE_LAST_CLINIC,180u,0u,0);
 scene(CINE_LAST_CLINIC,180u,2u,1);
 assert(diff(19,30,72,73)>8u);
 /* After the safe-house door has opened, relief differs from a
    mid-rescue approach; characters remain on the original set. */
 scene(CINE_RESCUE_SAVED,100u,1u,0);
 scene(CINE_RESCUE_SAVED,150u,1u,1);
 assert(diff(26,32,112,75)>25u);
 /* Score-boundary 2600 moves from record-shop coverage to Mara's
    arrival. Both shots share the same absolute vinyl/world clock.
    The near-center grooves MUST NOT reset or jump even as blocking
    begins again in the second shot. */
 assert(cinema_ending_scene(STORY_END_GOOD,2599u,0u)==CINE_GOOD);
 assert(cinema_ending_scene(STORY_END_GOOD,2600u,0u)==CINE_GOOD);
 ending(STORY_END_GOOD,2599u,0u,0);
 ending(STORY_END_GOOD,2600u,0u,1);
 assert(diff(53,30,68,45)==0u);
 /* Contrasting fates remain visually distinct without implying explicit
    violence or reintroducing Mara into an unresolved disappearance. */
 assert(cinema_ending_scene(STORY_END_BAD,800u,0u)==CINE_MARA_EMPTY);
 assert(cinema_ending_scene(STORY_END_BAD,800u,1u)==CINE_MARA);
 ending(STORY_END_BAD,800u,0u,0);
 ending(STORY_END_BAD,800u,1u,1);
 assert(diff(70,36,111,75)>18u);
 /* Determinism and indexed-palette safety for every scene and
    representative animated beats, including the new emotions. */
 for(unsigned scene_id=0;scene_id<CINE_COUNT;scene_id++){
  for(unsigned beat=0;beat<300u;beat+=41u){
   scene(scene_id,beat,2u,0);
   scene(scene_id,beat,2u,1);
   assert(memcmp(a.pixels,b.pixels,sizeof(a.pixels))==0);
  }
 }
 puts("cinema acting: expressive reactions, rescue continuity, absolute world clock across score cuts, deterministic bounded frames OK");
 return 0;
}
