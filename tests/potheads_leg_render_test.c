#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "potheads_leg_render.h"
int main(void){
 PotClothRect r;unsigned ink;
 assert(!pot_leg_rect(&r,&ink,0,0,0,256,48,256,0,128));
 assert(!pot_leg_rect(&r,&ink,239,134,(5u<<5)|1u,256,48,256,0,128));
 assert(!pot_leg_rect(&r,&ink,0,135,1u<<5,256,48,256,0,128));
 assert(pot_leg_rect(&r,&ink,0,0,(3u<<5)|31u,256,48,256,0,128));
 assert(r.x==0&&r.y==0&&r.w==32&&r.h==1&&ink==3);
 /* Every possible valid span at the settled camera is exact. */
 for(int y=0;y<135;y++)for(int x=0;x<240;x++)for(int n=1;n<=32&&x+n<=240;n++){
  assert(pot_leg_rect(&r,&ink,x,y,(2u<<5)|(unsigned)(n-1),256,48,256,0,128));
  assert(r.x==x&&r.y==y&&r.w==n&&r.h==1&&ink==2);
 }
#if POTHEADS_LEG_ENABLED
 assert(POT_LEG_FRAMES==548u&&pot_leg_tick[0]==0u&&pot_leg_tick[547]==1093u);
 assert(pot_leg_off[0]==0u&&pot_leg_off[548]==POT_LEG_BYTES);
 unsigned pixels=0,peak=0,visible=0;
 static unsigned char seen[240*135];
 for(unsigned f=0;f<POT_LEG_FRAMES;f++){
  if(f)assert(pot_leg_tick[f]>pot_leg_tick[f-1]);
  assert(pot_source_leg_frame(pot_leg_tick[f])==(int)f);
  unsigned a=pot_leg_off[f],b=pot_leg_off[f+1];
  assert(b>=a&&b<=POT_LEG_BYTES&&(b-a)%3u==0u);
  if(a!=b)visible++;
  if((b-a)/3u>peak)peak=(b-a)/3u;
  memset(seen,0,sizeof seen);
  for(unsigned i=a;i<b;i+=3u){
   int y=pot_leg_spans[i],x=pot_leg_spans[i+1];unsigned k=pot_leg_spans[i+2];
   assert(pot_leg_rect(&r,&ink,x,y,k,256,48,256,0,128));
   for(int j=0;j<r.w;j++){unsigned q=(unsigned)y*240u+(unsigned)(x+j);assert(!seen[q]);seen[q]=1;pixels++;}
   if(pot_leg_rect(&r,&ink,x,y,k,160,225,56,40,88))assert(r.x>=0&&r.y>=0&&r.x+r.w<=240&&r.y+r.h<=135);
  }
 }
 assert(visible==545u&&pixels==866440u&&peak==402u);
 for(unsigned i=545;i<548;i++)assert(pot_leg_off[i]==pot_leg_off[i+1]);
 assert(pot_source_leg_frame(65535u)==547);
 printf("548 frames, %u visible, 3 empty cropped, %u spans max, %u pixels\n",visible,peak,pixels);
#else
 assert(pot_source_leg_frame(0u)==-1&&pot_source_leg_frame(65535u)==-1);
 puts("No private trace: authored legs retained");
#endif
 return 0;
}
