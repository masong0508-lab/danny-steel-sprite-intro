#include "cinema.h"
#include "story.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/* Temporal-regression tests sample actual rendered pixels. The staging
   is deliberately free of heap, extra framebuffers and random state. */
static struct {uint8_t left[16],image[120*80],right[16];} a,b;
static void frame(unsigned scene,unsigned t,unsigned page,
                  void *destination){
 struct {uint8_t left[16],image[120*80],right[16];} *f=destination;
 memset(f,0xD3,sizeof(*f));
 cinema_draw(f->image,scene,t,page,t);
 for(int i=0;i<16;i++)assert(f->left[i]==0xD3&&f->right[i]==0xD3);
 for(int i=0;i<120*80;i++)assert(f->image[i]<32u);
}
static unsigned changed(int x0,int y0,int x1,int y1){
 unsigned count=0;
 for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++)
  count+=(a.image[y*120+x]!=b.image[y*120+x]);
 return count;
}
int main(void){
 /* The 160-pixel sky loop must wrap off-screen instead of jumping
    41 pixels while the audience is looking at it. */
 frame(CINE_MARA_EMPTY,3839u,0,&a);
 frame(CINE_MARA_EMPTY,3840u,0,&b);
 assert(changed(0,9,120,15)<55u);
 /* The distant buildings loop after exactly 160px: their heights,
    walls and windows stay coherent at the seam. */
 frame(CINE_MARA_EMPTY,13599u,0,&a);
 frame(CINE_MARA_EMPTY,13600u,0,&b);
 assert(changed(38,22,118,39)<1200u);
 /* The nearer district loops after 184px without a frame teleport. */
 frame(CINE_MARA_EMPTY,20239u,0,&a);
 frame(CINE_MARA_EMPTY,20240u,0,&b);
 assert(changed(39,43,117,57)<900u);
 /* Inside the archive, buildings across the window no longer move
    randomly sideways. The actors and desk are below this region. */
 frame(CINE_BORE,0,0,&a);
 frame(CINE_BORE,475,0,&b);
 assert(changed(12,14,51,26)==0u);
 /* The physical tape reel and LP reflect different rotation phases. */
 frame(CINE_MUSIC,0,0,&a);
 frame(CINE_MUSIC,18,0,&b);
 assert(changed(38,44,53,55)>5u);
 frame(CINE_GOOD,0,3,&a);
 frame(CINE_GOOD,34,3,&b);
 assert(changed(53,31,68,45)>4u);
 /* Neither parked bus may continue moving or return to its entrance. */
 frame(CINE_OPEN,220,0,&a);
 frame(CINE_OPEN,400,0,&b);
 /* Compare the parked bus's second windshield, not Danny's blinking
    face in front of the first window. */
 assert(changed(60,31,74,39)==0u);
 frame(CINE_BUS,220,0,&a);
 frame(CINE_BUS,400,0,&b);
 assert(changed(27,30,55,40)==0u);
 /* Earlier than arrival, the bus and Danny do move on the set. */
 frame(CINE_BUS,0,0,&a);
 frame(CINE_BUS,91,0,&b);
 assert(changed(17,26,95,61)>200u);
 /* Extra animation remains deterministic in all five ending sequences. */
 for(unsigned scene=CINE_GOOD;scene<=CINE_BAD;scene++){
  frame(scene,123u,0,&a);
  frame(scene,123u,0,&b);
  assert(!memcmp(a.image,b.image,sizeof(a.image)));
 }
 puts("temporal cinema: seamless cloud/city loops, static window geometry, moving reels/vinyl and stable bus arrival OK");
 return 0;
}
