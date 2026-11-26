#include "hd_canvas.h"
#include "potheads_motion.h"
#include "potheads_upper_render.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { OPENING_DESCENT=180u, FILM_HEIGHT=135u };

/* Exercise the production renderer throughout the source performance,
   including the inverted and cropped poses at the end of the film. */
static struct {
 uint8_t before[64];
 uint8_t pixels[HD_BYTES];
 uint8_t after[64];
} frame;
static uint8_t caption_before[HD_BYTES];

static unsigned paint(unsigned t,unsigned page,int overlay){
 Story s;
 story_new(&s,0);
 assert(s.cinema==CINE_OPEN);
 s.cinema_frame=(uint16_t)t;
 s.page=(uint8_t)page;
 memset(&frame,0xB7,sizeof frame);
 hd_cinema_draw(frame.pixels,&s,t);
 if(overlay)hd_game_overlay(frame.pixels,&s);
 unsigned checksum=2166136261u;
 for(unsigned i=0;i<64u;i++)
  assert(frame.before[i]==0xB7&&frame.after[i]==0xB7);
 for(unsigned y=0;y<HD_H;y++)for(unsigned x=0;x<HD_W;x++){
  unsigned pixel=frame.pixels[y*HD_W+x];
  assert(pixel<64u);
  if(y<FILM_HEIGHT)checksum=(checksum^pixel)*16777619u;
  else if(!overlay)assert(pixel==HD_INK);
 }
 return checksum;
}
static unsigned film_ink(unsigned ink){
 unsigned count=0u;
 for(unsigned i=0;i<HD_W*FILM_HEIGHT;i++)count+=frame.pixels[i]==ink;
 return count;
}
static void tap(Story *s,unsigned key){
 story_tick(s,0u);story_tick(s,(uint16_t)key);story_tick(s,0u);
}
static void finish_dialogue(Story *s){
 for(unsigned i=0;i<24u&&s->dialogue;i++)tap(s,1u);
 assert(!s->dialogue);
}
static void verify_full_source_track(void){
 /* Reject a preview track or the former standing-only cut. The reference
    reaches its right-edge departure at 18.2 seconds after its first frame. */
 assert(POT_TRACE_COMPLETE==1);
 assert(POT_TRACE_COUNT>1u);
 assert(POT_TRACE_LAST_TICK>=1093u&&POT_TRACE_LAST_TICK<=1094u);
 assert(POT_TRACE_LAST_SOURCE_FRAME==547u);
 for(unsigned i=1;i<POT_TRACE_COUNT;i++)
  assert(pot_trace[i].tick>pot_trace[i-1u].tick);

 PotPose first={0},last={0},clamped={0};
 pot_trace_sample(0u,&first);
 pot_trace_sample(POT_TRACE_LAST_TICK,&last);
 pot_trace_sample(POT_TRACE_LAST_TICK+600u,&clamped);
 assert(memcmp(&last,&clamped,sizeof last)==0);
 assert(memcmp(&first,&last,sizeof first)!=0);
 assert((first.flags&3u)==POT_SIGN_HELD);
 assert((last.flags&3u)==0u);

 /* Missing anatomical landmarks and a visible contour at the frame edge
    are different cases. Preserve that distinction through interpolation. */
 for(unsigned tick=0;tick<=POT_TRACE_LAST_TICK;tick++){
  PotPose pose={0};pot_trace_sample(tick,&pose);
  assert(!(pose.visible&pose.inferred));
  assert(!(pose.visible&pose.boundary));
  assert(!(pose.inferred&pose.boundary));
  assert(pose.located==(pose.visible|pose.inferred|pose.boundary));
  assert(!(pose.located&~0xffffu));
  assert(!(pose.contact_mask&~15u));
 }
}
static void near_point(PotJoint point,int x,int y){
 /* One packed unit is three reference pixels; allow one unit of tracing
    refinement while protecting the observed palm and final head anchors. */
 assert(point.x>=x-1&&point.x<=x+1);
 assert(point.y>=y-1&&point.y<=y+1);
}
static void verify_observed_floor_poses(void){
 /* Independently read from source frames 305, 320, 340, 425 and 547.
    Contacts are only assigned where the source actually shows the palm;
    cropped sole contacts are not invented for the bridge or final exit. */
 static const struct {
  unsigned tick,contacts,facing;
  int left_x,left_y,right_x,right_y;
 } support[]={
  {610u,12u,1u,-47,2,-51,-2},
  {640u,12u,3u,-49,3,-19,-2},
  {680u, 8u,0u,  0,0,-21, 0},
  {850u,12u,3u,  0,0, 24, 0},
  {1093u,8u,1u,  0,0, 96, 3}
 };
 for(unsigned i=0;i<sizeof support/sizeof support[0];i++){
  PotPose pose={0};pot_trace_sample(support[i].tick,&pose);
  assert(pose.contact_mask==support[i].contacts);
  assert(((pose.flags>>POT_FACING_SHIFT)&3u)==support[i].facing);
  assert(pose.flags&POT_GROUNDED);
  assert((pose.flags&3u)==0u);
  if(pose.contact_mask&POT_LEFT_PALM)
   near_point(pose.contact[2],support[i].left_x,support[i].left_y);
  if(pose.contact_mask&POT_RIGHT_PALM)
   near_point(pose.contact[3],support[i].right_x,support[i].right_y);
 }

 PotPose scissor={0},turn={0},sweep={0},last={0},sign={0};
 pot_trace_sample(610u,&scissor);
 pot_trace_sample(640u,&turn);
 pot_trace_sample(850u,&sweep);
 pot_trace_sample(POT_TRACE_LAST_TICK,&last);
 assert(scissor.joint[POT_LHIP].y+scissor.joint[POT_RHIP].y<
        2*scissor.joint[POT_NECK].y);
 assert(turn.joint[POT_LHIP].y+turn.joint[POT_RHIP].y<
        2*turn.joint[POT_NECK].y);
 assert(scissor.boundary&(1u<<POT_RTOE));
 assert(turn.boundary&(1u<<POT_LANKLE));
 assert(!(turn.located&(1u<<POT_LTOE)));
 assert(scissor.flags&POT_NEAR_LEFT_LEG);
 assert(!(turn.flags&POT_NEAR_LEFT_LEG));
 assert(sweep.flags&POT_NEAR_LEFT_LEG);

 unsigned legs=(1u<<POT_LKNEE)|(1u<<POT_LANKLE)|(1u<<POT_LTOE)|
               (1u<<POT_RKNEE)|(1u<<POT_RANKLE)|(1u<<POT_RTOE);
 unsigned hips=(1u<<POT_LHIP)|(1u<<POT_RHIP);
 assert(!(last.located&legs));
 assert((last.boundary&hips)==hips);
 assert(last.visible&(1u<<POT_HEAD));
 near_point(last.joint[POT_HEAD],90,-40);

 pot_trace_sample(428u,&sign);
 assert((sign.flags&3u)==POT_SIGN_FALLING);
 pot_trace_sample(452u,&sign);
 assert((sign.flags&3u)==0u);
 for(unsigned tick=770u;tick<=POT_TRACE_LAST_TICK;tick++){
  PotPose pose={0};pot_trace_sample(tick,&pose);
  assert(pose.bag_state==POT_BAG_RESTING);
  near_point(pose.bag,-42,-2);
 }
}
static void verify_caption_boundary(void){
 /* Caption placement must not cover a hand plant, a foreground shoe or
    the final cropped arm. Check every dialogue page at each major phase. */
 static const unsigned beats[]={0u,180u,600u,790u,960u,1188u,1274u};
 for(unsigned i=0;i<sizeof beats/sizeof beats[0];i++){
  for(unsigned page=0;page<7u;page++){
   unsigned film=paint(beats[i],page,0);
   memcpy(caption_before,frame.pixels,sizeof caption_before);
   assert(paint(beats[i],page,1)==film);
   assert(memcmp(caption_before,frame.pixels,HD_W*FILM_HEIGHT)==0);
   assert(memcmp(caption_before+HD_W*FILM_HEIGHT,
                 frame.pixels+HD_W*FILM_HEIGHT,
                 HD_W*(HD_H-FILM_HEIGHT))!=0);
  }
 }
}
static void verify_full_film(void){
 unsigned end=OPENING_DESCENT+POT_TRACE_LAST_TICK;
 /* Clipped lines and polygons must remain safe at every intervening tick,
    not only at the sparse source landmarks or selected poster frames. */
 for(unsigned t=0;t<=end;t++)paint(t,0u,0);
 assert(paint(0u,0u,0)!=paint(90u,0u,0));
 assert(paint(90u,0u,0)!=paint(180u,0u,0));

 paint(OPENING_DESCENT,0u,0);
 unsigned held_paper=film_ink(HD_PAPER);
 paint(OPENING_DESCENT+480u,0u,0);
 unsigned released_paper=film_ink(HD_PAPER);
 assert(held_paper>released_paper+100u);

 /* The camera is settled for these beats: changing film pixels therefore
    cover the kick-up, hand-supported turns and travelling floor exit. */
 static const unsigned source_beats[]={
  570u,610u,640u,680u,720u,770u,810u,850u,890u,930u,
  970u,1010u,1050u,1093u
 };
 unsigned previous=paint(OPENING_DESCENT+source_beats[0],0u,0);
 for(unsigned i=1;i<sizeof source_beats/sizeof source_beats[0];i++){
  unsigned next=paint(OPENING_DESCENT+source_beats[i],0u,0);
  assert(next!=previous);
  previous=next;
 }
 unsigned final=paint(end,0u,0);
 assert(paint(end+600u,0u,0)==final);
 assert(paint(OPENING_DESCENT+850u,3u,1)==
        paint(OPENING_DESCENT+850u,3u,1));
}
static void verify_long_reading_clock(void){
 Story slow;story_new(&slow,0);
 unsigned reading=OPENING_DESCENT+POT_TRACE_LAST_TICK+120u;
 for(unsigned i=0;i<reading;i++){
  story_tick(&slow,0u);
  assert(slow.cinema==CINE_OPEN&&slow.dialogue&&slow.page==0u);
  assert(slow.cinema_frame==i+1u);
 }
 /* A long read plays the entire performance without input. Later page
    cues cannot rewind that clock or restart the standing sign pose. */
 for(unsigned page=1;page<7u;page++){
  unsigned before=slow.cinema_frame;
  tap(&slow,1u);
  assert(slow.page==page&&slow.cinema==CINE_OPEN);
  assert(slow.cinema_frame==before+3u);
 }
 finish_dialogue(&slow);
 assert(slow.cinema==CINE_NONE);

 static const unsigned skip_beats[]={0u,179u,180u,790u,1188u,1274u};
 for(unsigned i=0;i<sizeof skip_beats/sizeof skip_beats[0];i++){
  Story skip;story_new(&skip,1);
  skip.cinema_frame=(uint16_t)skip_beats[i];
  story_tick(&skip,2u);
  assert(!skip.dialogue&&skip.cinema==CINE_NONE);
 }
}
static void verify_work_not_wishful_thinking(void){
 Story s;story_new(&s,0);
 assert(s.x==20u&&s.y==11u&&s.facing==0u);
 assert(!story_solid(s.x,s.y));
 assert(story_tile(21,11)=='#');
 int count=0;
 const StoryPage *pages=story_pages(&s,&count);
 assert(count==7);
 assert(strstr(pages[1].line1,"don't love")!=NULL);
 assert(strstr(pages[2].line2,"grateful")!=NULL);
 /* A remains freely advancing. Its cues reach the floor sequence and
    exit, while leaving the continuously playing source clock monotonic. */
 static const unsigned anchors[7]={0u,90u,180u,450u,780u,990u,1188u};
 for(unsigned page=1;page<7;page++){
  tap(&s,1u);
  assert(s.page==page);
  assert(s.cinema_frame==anchors[page]+1u);
 }
 finish_dialogue(&s);
 /* B remains a genuine skip for players who want to get to gameplay. */
 Story skip;story_new(&skip,1);
 tap(&skip,2u);
 assert(skip.dialogue==0u&&skip.cinema==CINE_NONE);
 s.x=20;s.y=11;s.prev_x=s.x;s.prev_y=s.y;s.facing=0;
 s.life_stage=STORY_LIFE_WAIT;s.energy=3;
 tap(&s,1u);
 assert(s.activity=='J'&&s.cinema==CINE_OPEN);
 assert(s.work==0u&&s.energy==3u);
 tap(&s,2u); /* B CANCEL */
 assert(!s.activity&&s.work==0u&&s.energy==3u);
 tap(&s,1u);
 assert(s.activity=='J');
 tap(&s,16u); /* RIGHT LEAVE */
 assert(s.activity_choice==1u);
 tap(&s,1u);
 assert(!s.activity&&!s.dialogue&&s.work==0u&&s.energy==3u);
 tap(&s,1u);
 assert(s.activity=='J'&&s.activity_choice==0u);
 tap(&s,1u); /* explicit WORK confirmation */
 assert(s.dialogue!=0u&&s.cinema==CINE_OPEN);
 assert(s.cinema_frame>=184u);
 assert(s.work==2u&&s.energy==2u);
 assert(s.daily_mask&STORY_DAY_SHIFT);
 finish_dialogue(&s);
 tap(&s,1u);
 assert(s.work==2u&&s.energy==2u); /* no same-day work exploit */
 finish_dialogue(&s);
 s.rescue_stage=STORY_RESCUE_MISSING;
 tap(&s,1u);
 assert(s.cinema==CINE_NOTICE);
 assert(s.work==2u&&s.energy==2u); /* rescue takes precedence */
}
static void verify_exact_short_division(void){
 /* Check the exact prior rounded signed expression at every int8
    coordinate pair, every short duration and every intermediate tick. */
 for(int a=-128;a<=127;a++)for(int b=-128;b<=127;b++)
  for(unsigned d=1u;d<=16u;d++)for(unsigned n=0u;n<=d;n++){
   int v=a*(int)d+(b-a)*(int)n;
   int expected=(v+(v<0?-(int)(d/2u):(int)(d/2u)))/(int)d;
   assert(pot_trace_lerp(a,b,n,d)==expected);
  }
 for(int a=-5000;a<=5000;a+=499)for(int b=-5000;b<=5000;b+=501)
  for(unsigned d=17u;d<=63u;d+=7u)for(unsigned n=0u;n<=d;n++){
   int v=a*(int)d+(b-a)*(int)n;
   assert(pot_trace_lerp(a,b,n,d)==(v+(v<0?-(int)(d/2u):(int)(d/2u)))/(int)d);
  }
 puts("Exact signed reciprocal interpolation and long-interval fallback OK");
}
static void verify_pose_clock_and_rounding(void){
 assert(pot_trace_lerp(-10,-7,1,2)==pot_trace_lerp(-7,-10,1,2));
 assert(pot_trace_lerp(0,3,1,2)==2);
 assert(pot_trace_lerp(0,-3,1,2)==-2);
 for(int a=-113;a<=113;a+=7)for(int b=-113;b<=113;b+=7)
  for(unsigned n=0;n<=10u;n++){
   int v=pot_trace_lerp(a,b,n,10u);
   assert(v==pot_trace_lerp(b,a,10u-n,10u));
   assert(v>=(a<b?a:b)&&v<=(a>b?a:b));
  }
 for(unsigned t=0;t<=POT_TRACE_LAST_TICK+1u;t++){
  assert(pot_source_pose_tick(t,-1,-1,-1)==t);
  int c=pot_source_cloth_frame(t),l=pot_source_leg_frame(t);
  int u=pot_source_upper_frame(t,c,l);
#if POTHEADS_CLOTH_ENABLED
  if(c>=0)assert(pot_source_pose_tick(t,c,-1,-1)==pot_cloth_tick[c]);
#endif
#if POTHEADS_LEG_ENABLED
  if(l>=0)assert(pot_source_pose_tick(t,-1,l,-1)==pot_leg_tick[l]);
#endif
  unsigned pose=pot_source_pose_tick(t,c,l,u);
  assert(pose<=t);
  if(c<0&&l<0&&u<0)assert(pose==t);
#if POTHEADS_UPPER_ENABLED
  if(u>=0)assert(pose==pot_upper_tick[u]);
#endif
 }
}
int main(void){
 verify_exact_short_division();
 verify_pose_clock_and_rounding();
 verify_full_source_track();
 verify_observed_floor_poses();
 verify_caption_boundary();
 verify_full_film();
 verify_long_reading_clock();
 verify_work_not_wishful_thinking();
 puts("Potheads opening: complete source film, floor motion, caption isolation, monotonic clock, optional paid shift, bounds and rescue priority OK");
 return 0;
}

