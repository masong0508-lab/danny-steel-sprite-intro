#include "frame_runtime.h"
#include "story.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static unsigned drain(Story *s){
 uint16_t keys;
 unsigned count=0;
 while(frame_runtime_next(&keys)){
  if(s)story_tick(s,keys);
  ++count;
 }
 return count;
}
static void clocks_survive_draws(void){
 Story game;story_new(&game,0);
 unsigned initial=game.cinema_frame,ticks=0;
 frame_runtime_init();frame_runtime_start(0u);
 for(unsigned frame=1;frame<=600u;frame++){
  frame_runtime_capture(0u);
  /* A six-frame blocking painter must delay presentation, not turn ten
     seconds of emulated video into only a few seconds of story time. */
  if(frame%6u==0u)ticks+=drain(&game);
  assert(ticks+(uint32_t)(frame_runtime_write-frame_runtime_read)==frame);
 }
 assert(ticks==600u&&game.cinema_frame==initial+600u);
 assert(frame_runtime_video_frames==600u&&frame_runtime_dropped==0u);
}
static void chronological_edges(void){
 Story expected,actual;
 story_new(&expected,0);actual=expected;
 frame_runtime_init();frame_runtime_start(0u);
 for(unsigned frame=1;frame<=180u;frame++){
  /* Two presses during the same busy draw must remain two presses.
     Their intervening releases cannot be collapsed into an OR latch. */
  uint16_t keys=(frame==3u||frame==4u||frame==7u)?1u:
                frame==33u?2u:
                frame>=40u&&frame<72u?32u:
                frame>=91u&&frame<97u?512u:
                frame==121u?512u:0u;
  story_tick(&expected,keys);
  frame_runtime_capture(keys);
  if(frame%12u==0u){
   assert(drain(&actual)==12u);
   assert(memcmp(&actual,&expected,sizeof actual)==0);
   if(frame==12u)assert(actual.page==2u);
  }
 }
 assert(frame_runtime_dropped==0u);
 /* Entry-held A belongs to the menu, and must not skip the first line. */
 story_new(&actual,0);actual.old_keys=1u;
 frame_runtime_start(0u);
 for(unsigned i=0;i<12u;i++)frame_runtime_capture(1u);
 assert(drain(&actual)==12u&&actual.page==0u);
 frame_runtime_capture(0u);frame_runtime_capture(1u);
 assert(drain(&actual)==2u&&actual.page==1u);
}
static void publication_and_budget(void){
 unsigned page=99u;
 frame_runtime_init();frame_runtime_start(0u);
 assert(!frame_runtime_begin_render(&page));
 frame_runtime_capture(0u);assert(drain(NULL)==1u);
 assert(!frame_runtime_begin_render(&page));
 frame_runtime_capture(0u);assert(drain(NULL)==1u);
 assert(frame_runtime_begin_render(&page)&&page==1u);
 frame_runtime_submit(page);
 assert(!frame_runtime_begin_render(&page));
 frame_runtime_capture(0u);assert(drain(NULL)==1u);
 assert(!frame_runtime_begin_render(&page));
 frame_runtime_capture(0u);assert(drain(NULL)==1u);
 assert(frame_runtime_begin_render(&page)&&page==0u);
 /* Painting takes six video frames. No page is eligible to publish
    until the complete copy is explicitly submitted. */
 for(unsigned i=0;i<6u;i++)frame_runtime_capture(0u);
 assert(!frame_runtime_begin_render(&page)); /* input still pending */
 assert(drain(NULL)==6u);
 frame_runtime_submit(0u);
 assert(!frame_runtime_begin_render(&page));
 frame_runtime_capture(0u);assert(drain(NULL)==1u);
 /* It can start immediately after publication. There is no artificial
    alternating-call idle frame after an over-budget draw. */
 assert(frame_runtime_begin_render(&page)&&page==1u);
 frame_runtime_submit(page);
 frame_runtime_stop();
 for(unsigned i=0;i<6u;i++)frame_runtime_capture(1u);
 assert(drain(NULL)==0u&&!frame_runtime_begin_render(&page));
 frame_runtime_start(0u);
 frame_runtime_capture(0u);frame_runtime_capture(0u);drain(NULL);
 assert(frame_runtime_begin_render(&page)&&page==1u);
}
static void overflow_and_wrap(void){
 uint16_t keys;
 unsigned page=99u;
 frame_runtime_init();frame_runtime_start(0u);
 for(unsigned i=0;i<129u;i++)frame_runtime_capture((uint16_t)i);
 assert(frame_runtime_dropped==65u);
 assert((uint32_t)(frame_runtime_write-frame_runtime_read)==64u);
 for(unsigned i=65u;i<129u;i++){
  assert(frame_runtime_next(&keys));assert(keys==i);
 }
 assert(!frame_runtime_next(&keys));
 frame_runtime_video_frames=UINT32_MAX-1u;
 frame_runtime_start(0u);
 frame_runtime_capture(0u);frame_runtime_capture(0u);drain(NULL);
 assert(frame_runtime_video_frames==0u);
 assert(frame_runtime_begin_render(&page)&&page==1u);
 frame_runtime_start(0u);
 frame_runtime_read=frame_runtime_write=UINT32_MAX-2u;
 for(unsigned i=1;i<=4u;i++)frame_runtime_capture((uint16_t)i);
 for(unsigned i=1;i<=4u;i++){
  assert(frame_runtime_next(&keys));assert(keys==i);
 }
 assert(frame_runtime_read==1u&&frame_runtime_write==1u);
 assert(frame_runtime_dropped==0u&&!frame_runtime_next(&keys));
}
static void transition_publication(void){
 unsigned page=99u;
 frame_runtime_init();frame_runtime_start(0u);
 frame_runtime_capture(0u);frame_runtime_capture(0u);assert(drain(NULL)==2u);
 assert(frame_runtime_begin_render(&page)&&page==1u);
 for(unsigned i=0;i<5u;i++){frame_runtime_capture(0u);assert(frame_runtime_dim==0u);}
 assert(drain(NULL)==5u);
 frame_runtime_submit_transition(page,FRAME_RUNTIME_SOFTEN);
 assert(frame_runtime_dim==0u);
 frame_runtime_capture(1u);assert(frame_runtime_dim==FRAME_RUNTIME_DIM_LEVEL);
 uint16_t key=0;assert(frame_runtime_next(&key)&&key==1u);
 for(unsigned n=FRAME_RUNTIME_DIM_LEVEL;n>0u;n--){
  frame_runtime_capture(0u);assert(frame_runtime_dim==n-1u);
  assert(drain(NULL)==1u);
 }
 assert(frame_runtime_begin_render(&page)&&page==0u);
 frame_runtime_submit_transition(page,FRAME_RUNTIME_SOFTEN);
 frame_runtime_capture(0u);assert(drain(NULL)==1u);
 frame_runtime_capture(0u);assert(drain(NULL)==1u);
 assert(frame_runtime_dim==FRAME_RUNTIME_DIM_LEVEL-1u);
 assert(frame_runtime_begin_render(&page)&&page==1u);
 frame_runtime_submit_transition(page,FRAME_RUNTIME_CLEAR);
 frame_runtime_capture(0u);assert(drain(NULL)==1u&&frame_runtime_dim==0u);
 assert(!frame_runtime_dropped);
 frame_runtime_stop();assert(frame_runtime_dim==0u);
}
int main(void){
 transition_publication();
 clocks_survive_draws();chronological_edges();
 publication_and_budget();overflow_and_wrap();
 puts("Video-time runtime: 600 story ticks through blocked draws, chronological taps/holds, completed-page publication, overflow and counter-wrap checks passed");
 return 0;
}
