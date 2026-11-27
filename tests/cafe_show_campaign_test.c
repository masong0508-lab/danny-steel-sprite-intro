#include "story.h"
#include "studio.h"
#include "story_save.h"
#include "neighborhood_save.h"
#include "hd_canvas.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
enum {A=1,B=2,SELECT=4,START=8,RIGHT=16,LEFT=32,UP=64,DOWN=128};
static void tap(Story *s,unsigned key){story_tick(s,0);story_tick(s,(uint16_t)key);story_tick(s,0);}
static void done(Story *s){for(unsigned i=0;i<40u&&s->dialogue;i++)tap(s,A);assert(!s->dialogue);}
static void stand(Story *s,int x,int y,int facing){assert(!story_solid(x,y));s->x=s->prev_x=(uint8_t)x;s->y=s->prev_y=(uint8_t)y;s->facing=(uint8_t)facing;s->cooldown=0;}
static void track(Story *s,unsigned which){tap(s,START);tap(s,A);assert(s->paused);for(unsigned i=0;i<which;i++)tap(s,DOWN);tap(s,A);assert(!s->paused);}
static unsigned walks;
static void arrive(Story *s){
 static const unsigned keys[]={RIGHT,DOWN,LEFT,UP};
 for(unsigned i=0;i<STORY_W*STORY_H;i++){int d,n;assert(story_route(s,&d,&n));if(!n)return;s->cooldown=0;tap(s,keys[d]);walks++;}
 assert(0);
}
static struct {uint8_t before[32],pixels[HD_BYTES],after[32];} frame;
static void paint(Story *s){
 memset(&frame,0xA5,sizeof frame);memset(frame.pixels,HD_ROAD,HD_BYTES);
 hd_cinema_draw(frame.pixels,s,100u);hd_game_overlay(frame.pixels,s);
 for(unsigned i=0;i<32u;i++)assert(frame.before[i]==0xA5&&frame.after[i]==0xA5);
 for(unsigned i=0;i<HD_BYTES;i++)assert(frame.pixels[i]<64);
}
static void stage(Story *s,unsigned n){
 assert(story_show_stage(s)==n);
 uint8_t raw[STORY_SAVE_BYTES],out[STORY_SAVE_BYTES],extra[NEIGHBORHOOD_EXT_LEN];
 story_save_encode(s,raw);assert(story_save_decode(raw,out));assert(out[29]==s->project_style);
 neighborhood_save_encode(s,extra);Story reload=*s;reload.neighborhood_target=0;
 assert(neighborhood_save_decode(&reload,extra)&&reload.neighborhood_target==s->neighborhood_target);
}
int main(void){
 Story s;story_new(&s,0);done(&s);assert(!story_track_available(&s,STORY_TRACK_SHOW_INDEX));
 stand(&s,5,5,2);tap(&s,A);done(&s);
 stand(&s,11,8,3);tap(&s,A);done(&s);
 stand(&s,8,12,3);tap(&s,A);done(&s);
 assert(story_track_available(&s,STORY_TRACK_SHOW_INDEX));
 track(&s,STORY_TRACK_SHOW_INDEX);arrive(&s);tap(&s,A);assert(s.activity==STORY_ACT_SHOW);paint(&s);
 tap(&s,B);stage(&s,0);tap(&s,A);tap(&s,A);done(&s);stage(&s,1);
 arrive(&s);tap(&s,A);assert(s.activity==STORY_ACT_SHOW);paint(&s);tap(&s,RIGHT);tap(&s,A);done(&s);
 stage(&s,2);assert(s.project_style&STORY_SHOW_WARM_POSTER);
 arrive(&s);tap(&s,A);done(&s);assert(s.flags&STORY_MUSIC);
 for(unsigned i=0;i<4;i++){tap(&s,A);assert(s.activity=='m');tap(&s,A);done(&s);assert(s.song_steps==i+1u);}
 tap(&s,A);assert(s.activity==STORY_ACT_REHEARSAL);paint(&s);assert(story_master_showcase(&s));
 tap(&s,RIGHT);tap(&s,A);assert(s.studio_tab==0&&s.studio_cursor==1);paint(&s);
 tap(&s,B);stage(&s,2);
 tap(&s,A);
 for(unsigned i=0;i<4u;i++){
  assert(s.studio_tab==i);while(s.activity_choice!=story_show_expected(&s))tap(&s,RIGHT);
  tap(&s,A);
 }
 done(&s);stage(&s,3);
 arrive(&s);tap(&s,A);done(&s);assert(s.flags&STORY_ART);
 for(unsigned i=0;i<4;i++){
  tap(&s,A);assert(s.activity=='r');tap(&s,LEFT);assert(s.activity_choice==2u);tap(&s,A);
  assert(s.activity==STORY_ACT_PAINT);paint(&s);
  tap(&s,START);assert(s.activity==STORY_ACT_PAINT&&s.art_steps==i&&s.studio_cursor==2u);
  if(!i){tap(&s,B|START);assert(!s.activity&&!s.art_steps);tap(&s,A);tap(&s,LEFT);tap(&s,A);}
  if(!story_paint_warm(&s))tap(&s,SELECT);
  for(unsigned k=0;k<8u;k++)if(story_paint_targets(&s)&(1u<<k)){
   while((story_paint_brush(&s)&3u)<(k&3u))tap(&s,RIGHT);
   while((story_paint_brush(&s)&3u)>(k&3u))tap(&s,LEFT);
   if(story_paint_brush(&s)/4u<k/4u)tap(&s,DOWN);
   if(story_paint_brush(&s)/4u>k/4u)tap(&s,UP);
   tap(&s,A);paint(&s);
  }
  assert(story_paint_ready(&s));tap(&s,START);done(&s);
  assert(s.art_steps==i+1u&&s.project_style&STORY_STYLE_ART_WARM);
 }
 tap(&s,A);assert(s.activity==STORY_ACT_SHOW);tap(&s,RIGHT);tap(&s,A);done(&s);stage(&s,4);
 assert(s.project_style&STORY_SHOW_WARM_FRAME);
 arrive(&s);tap(&s,A);paint(&s);tap(&s,A);done(&s);stage(&s,5);
 arrive(&s);tap(&s,A);tap(&s,A);assert(story_master_showcase(&s)&&story_show_scene(&s));paint(&s);
 tap(&s,B);stage(&s,5);tap(&s,A);tap(&s,A);done(&s);stage(&s,6);
 assert(s.project_style&STORY_STYLE_SONG_SHARED);
 arrive(&s);tap(&s,A);tap(&s,A);done(&s);stage(&s,7);
 assert(!s.neighborhood_target&&!story_track_available(&s,STORY_TRACK_SHOW_INDEX));
 assert(walks>=40u&&s.flags==STORY_FULL&&!s.ended);
 /* Even a partially planned show never competes with urgent story beats. */
 s.project_style=(uint8_t)((s.project_style&~STORY_SHOW_STAGE_MASK)|(2u<<3));
 s.neighborhood_target=STORY_TRACK_SHOW;s.life_stage=STORY_LIFE_CLINIC;
 int x,y;const char *label;assert(story_goal(&s,&x,&y,&label)&&!strcmp(label,"CLINIC"));
 s.rescue_stage=STORY_RESCUE_MISSING;assert(story_goal(&s,&x,&y,&label)&&!strcmp(label,"WITNESS"));
 printf("Cafe exhibition: seven saved stages, %u actual walked steps, eight creative sessions, 16 brush marks, four cue cards, cancel/retry, real master and urgent priority OK\n",walks);
 return 0;
}
