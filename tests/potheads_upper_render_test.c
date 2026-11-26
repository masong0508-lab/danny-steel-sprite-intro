#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "potheads_upper_render.h"
int main(void){
 PotClothRect r;
 assert(!pot_upper_settled_rect(&r,0,0,0,1));
 assert(!pot_upper_settled_rect(&r,134,239,33u,1));
 assert(!pot_upper_settled_rect(&r,134,239,32u,2));
 assert(!pot_upper_settled_rect(&r,0,0,192u,1));
 assert(pot_upper_settled_rect(&r,130,208,(5u<<5)|31u,5));
 assert(r.x==208&&r.y==130&&r.w==32&&r.h==5);
#if POTHEADS_UPPER_ENABLED
 assert(POT_UPPER_FRAMES==548u&&POT_UPPER_BYTES==217768u);
 assert(pot_upper_tick[0]==0&&pot_upper_tick[547]==1093);
 assert(pot_upper_off[0]==0&&pot_upper_off[548]==POT_UPPER_BYTES);
 static unsigned char seen[240*135];
 unsigned pixels=0,skin=0,rects=0,rows=0,peak=0,active=0;
 for(unsigned i=0;i<POT_UPPER_FRAMES;i++){
  memset(seen,0,sizeof seen);
  assert(pot_upper_tick[i]==pot_cloth_tick[i]&&pot_upper_tick[i]==pot_leg_tick[i]);
  if(i)assert(pot_upper_tick[i]>pot_upper_tick[i-1]);
  unsigned a=pot_upper_off[i],b=pot_upper_off[i+1];
  assert(b>=a&&b<=POT_UPPER_BYTES&&(b-a)%4u==0u);
  if((b-a)/4u>peak)peak=(b-a)/4u;
  int cf=pot_source_cloth_frame(pot_upper_tick[i]),lf=pot_source_leg_frame(pot_upper_tick[i]);
  int uf=pot_source_upper_frame(pot_upper_tick[i],cf,lf);
  assert(uf==(cf>=0&&lf>=0&&b>a?(int)i:-1));
  if(uf>=0){active++;assert(pot_upper_pose_tick(pot_upper_tick[i]+1u,uf)==pot_upper_tick[i]);}
  for(unsigned k=pot_cloth_off[i];k<pot_cloth_off[i+1];k+=3u){
   unsigned y=pot_cloth_spans[k],x=pot_cloth_spans[k+1],n=pot_cloth_spans[k+2]&127u;
   assert(y<135u&&x+n<=240u);for(unsigned j=0;j<n;j++)seen[y*240u+x+j]=1;
  }
  for(unsigned k=pot_leg_off[i];k<pot_leg_off[i+1];k+=3u){
   unsigned y=pot_leg_spans[k],x=pot_leg_spans[k+1],n=(pot_leg_spans[k+2]&31u)+1u;
   assert(y<135u&&x+n<=240u);for(unsigned j=0;j<n;j++)seen[y*240u+x+j]=1;
  }
  for(unsigned k=a;k<b;k+=4u){
   unsigned y=pot_upper_rects[k],x=pot_upper_rects[k+1],packed=pot_upper_rects[k+2],h=pot_upper_rects[k+3];
   assert(pot_upper_settled_rect(&r,(int)y,(int)x,packed,(int)h));
   unsigned n=(packed&31u)+1u;rects++;rows+=h;
   for(unsigned j=0;j<h;j++){
    for(unsigned u=0;u<n;u++){unsigned at=(y+j)*240u+x+u;assert(!seen[at]);seen[at]=1;pixels++;if((packed>>5)>=4u)skin++;}
    PotClothRect q;assert(pot_cloth_rect(&q,(int)x,(int)(y+j),(int)n,256,48,256,0,128));
    assert(q.x==r.x&&q.y==r.y+(int)j&&q.w==r.w&&q.h==1);
    if(pot_cloth_rect(&q,(int)x,(int)(y+j),(int)n,160,225,56,40,88))assert(q.x>=0&&q.y>=0&&q.x+q.w<=240&&q.y+q.h<=135);
   }
  }
 }
 assert(rects==54442u&&rows==71580u&&peak==194u&&pixels==288727u&&skin==22258u&&active==546u);
 printf("548 frames: %u active, %u exact rectangles vs %u rows, peak %u; %u pixels (%u skin), no overlaps\n",active,rects,rows,peak,pixels,skin);
#else
 assert(pot_source_upper_frame(50u,25,25)==-1);
 assert(pot_upper_pose_tick(51u,-1)==51u);
 puts("Source-only fallback: authored upper body remains active");
#endif
 return 0;
}
