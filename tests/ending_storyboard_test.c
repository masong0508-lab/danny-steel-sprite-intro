#include "cinema.h"
#include "story.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Exercises real 120x80 indexed output and the entire edited montage;
   guard bytes catch writes outside the original single framebuffer. */
typedef struct {
 uint8_t before[24],pixels[120*80],after[24];
} Film;
static Film a,b,reference;
static void checked(const Film *v){
 for(int i=0;i<24;i++)assert(v->before[i]==0xE7&&v->after[i]==0xE7);
 for(int i=0;i<120*80;i++)assert(v->pixels[i]<32u);
}
static void ending(Film *out,unsigned route,unsigned frame,unsigned saved){
 memset(out,0xE7,sizeof(*out));
 cinema_ending_draw_context(out->pixels,route,frame,saved);
 checked(out);
}
static void raw(Film *out,unsigned scene,unsigned tick,unsigned page,unsigned finale){
 memset(out,0xE7,sizeof(*out));
 cinema_draw(out->pixels,scene,tick,page,finale);
 checked(out);
}
static unsigned difference(const Film *x,const Film *y,
                           int x0,int y0,int x1,int y1){
 unsigned n=0;
 for(int j=y0;j<y1;j++)for(int i=x0;i<x1;i++)
  n+=x->pixels[j*120+i]!=y->pixels[j*120+i];
 return n;
}
int main(void){
 /* Returning to the roof after memory scenes is no longer a frozen still:
    each of the five score-timed sections has an observable action. */
 const unsigned neutral_frames[]={2680u,3060u,3470u,3880u,4200u};
 for(unsigned i=0;i<5;i++){
  unsigned f=neutral_frames[i];
  assert(cinema_ending_scene(STORY_END_NEUTRAL,f,0u)==CINE_FINALE);
  ending(&a,STORY_END_NEUTRAL,f,0u);
  raw(&reference,CINE_FINALE,f,0u,420u);
  /* Mara, the note or roof railing appears on the left side. */
  assert(difference(&a,&reference,0,34,67,74)>16u);
  if(i){
   /* New actions are genuinely different, not identically timed loops. */
   assert(difference(&a,&b,0,34,67,74)>20u);
  }
  /* Canvas itself remains identical and physically anchored on return. */
  if(i==0)memcpy(&b,&a,sizeof(a));
  else{
   for(int y=21;y<43;y++)for(int x=72;x<98;x++)
    assert(a.pixels[y*120+x]==b.pixels[y*120+x]);
   memcpy(&b,&a,sizeof(a));
  }
 }
 /* In the good ending, discovery, the growing audience and final listen
    are distinct record-store compositions around the SAME turntable. */
 const unsigned good_frames[]={2350u,3470u,4530u,5230u};
 const unsigned good_shots[]={4u,6u,8u,11u};
 for(unsigned i=0;i<4;i++){
  unsigned f=good_frames[i];
  assert(cinema_ending_shot(STORY_END_GOOD,f)==good_shots[i]);
  ending(&a,STORY_END_GOOD,f,0u);
  raw(&reference,CINE_GOOD,100u+f,3u,0u);
  assert(difference(&a,&reference,0,35,43,74)>10u);
 }
 /* A verified rescue must change even the non-dialogue bad-ending
    rooftop: no unanswered-Mara phone in the explicitly survived path. */
 const unsigned bad_frames[]={230u,2340u,3410u,4210u};
 for(unsigned i=0;i<4;i++){
  ending(&a,STORY_END_BAD,bad_frames[i],0u);
  ending(&b,STORY_END_BAD,bad_frames[i],1u);
  assert(difference(&a,&b,12,52,47,74)>8u);
  ending(&reference,STORY_END_BAD,bad_frames[i],0u);
  assert(!memcmp(reference.pixels,a.pixels,sizeof(a.pixels)));
 }
 /* Timed story and coda remain intact through the last valid frames. */
 assert(cinema_ending_shot(STORY_END_GOOD,5309u)==11u);
 assert(cinema_ending_shot(STORY_END_NEUTRAL,4349u)==10u);
 assert(cinema_ending_shot(STORY_END_BAD,4349u)==10u);
 for(unsigned f=0;f<5310u;f+=233u){
  ending(&a,STORY_END_GOOD,f,0u);
  if(f<4350u){
   ending(&a,STORY_END_NEUTRAL,f,0u);
   ending(&a,STORY_END_BAD,f,0u);
   ending(&a,STORY_END_BAD,f,1u);
  }
 }
 puts("ending storyboard: five rooftop story beats, four shop coverage shots, Mara survival continuity, palette and canary guards OK");
 return 0;
}
