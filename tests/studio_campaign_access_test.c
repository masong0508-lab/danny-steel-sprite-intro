#include "story.h"
#include "studio.h"
#include "story_save.h"
#include "story_checks.h"
#include "hd_canvas.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
enum {A=1,B=2,SELECT=4,START=8,RIGHT=16,LEFT=32,UP=64,DOWN=128,R=256,L=512};
static void tap(Story *s,unsigned key){story_tick(s,0);story_tick(s,(uint16_t)key);story_tick(s,0);}
static void done(Story *s){for(unsigned i=0;i<40u&&s->dialogue;i++)tap(s,A);assert(!s->dialogue);}
static void stand(Story *s,int x,int y,int facing){assert(!story_solid(x,y));s->x=s->prev_x=(uint8_t)x;s->y=s->prev_y=(uint8_t)y;s->facing=(uint8_t)facing;s->cooldown=0;}
static void goal(Story *s,int x,int y,const char *name){int gx,gy;const char *label;assert(story_goal(s,&gx,&gy,&label)&&gx==x&&gy==y&&!strcmp(name,label));}
static void track(Story *s,unsigned which){tap(s,START);tap(s,L);tap(s,RIGHT);assert(s->paused);for(unsigned i=0;i<which;i++)tap(s,DOWN);tap(s,A);assert(!s->paused);}
static void arrive(Story *s){static const unsigned keys[]={RIGHT,DOWN,LEFT,UP};for(unsigned i=0;i<STORY_W*STORY_H;i++){int d,n;assert(story_route(s,&d,&n));if(!n)return;s->cooldown=0;tap(s,keys[d]);}assert(0);}
static struct {uint8_t left[32],pixels[HD_BYTES],right[32];} frame;
static void paint(Story *s){memset(&frame,0xA5,sizeof frame);memset(frame.pixels,HD_ROAD,HD_BYTES);if(s->activity==STORY_ACT_STUDIO)hd_studio_draw(frame.pixels,s,10);else hd_game_overlay(frame.pixels,s);for(unsigned i=0;i<32;i++)assert(frame.left[i]==0xA5&&frame.right[i]==0xA5);for(unsigned i=0;i<HD_BYTES;i++)assert(frame.pixels[i]<64);}
int main(void){
 Story s;story_new(&s,0);done(&s);goal(&s,5,5,"LETTER");
 track(&s,0);goal(&s,19,7,"STUDIO");arrive(&s);
 int dx=19-s.x,dy=7-s.y;s.facing=dx>0?0:dx<0?2:dy>0?1:3;
 tap(&s,A);assert(s.flags&STORY_MUSIC);done(&s);
 assert(!s.song_steps&&!strcmp(story_action_hint(&s),"A STUDIO"));
 for(unsigned take=0;take<4;take++){
  tap(&s,A);assert(s.activity=='m');
  tap(&s,LEFT);assert(s.activity_choice==2u);paint(&s);
  tap(&s,A);assert(s.activity==STORY_ACT_STUDIO&&s.studio_tab==STUDIO_SONG);
  paint(&s);tap(&s,R);assert(s.studio_tab==STUDIO_NOTES);
  unsigned before=studio_patch()->notes[0];tap(&s,UP);assert(studio_patch()->notes[0]!=before||before==7u);
  tap(&s,R);assert(s.studio_tab==STUDIO_PERFORM);tap(&s,SELECT);assert(s.activity_choice);paint(&s);
  if(take==0){tap(&s,B|START);assert(!s.activity&&!s.song_steps);tap(&s,A);tap(&s,LEFT);tap(&s,A);}
  tap(&s,START);assert(!s.activity&&s.song_steps==take+1u&&s.dialogue);done(&s);
 }
 tap(&s,A);assert(s.activity=='E');tap(&s,RIGHT);tap(&s,A);
 tap(&s,R);assert(s.studio_tab==STUDIO_NOTES);tap(&s,R);assert(s.studio_tab==STUDIO_ORDER);tap(&s,B);
 track(&s,3);goal(&s,5,5,"LETTER");
 stand(&s,5,5,2);tap(&s,A);done(&s);
 stand(&s,11,8,3);tap(&s,A);done(&s);
 stand(&s,8,12,3);tap(&s,A);done(&s);assert((s.flags&STORY_ALL)==STORY_ALL);
 tap(&s,A);tap(&s,A);done(&s);
 track(&s,0);goal(&s,8,11,"SHARE SONG");
 unsigned bond=s.bond;tap(&s,A);assert(s.dialogue);tap(&s,B);assert(!(s.project_style&STORY_STYLE_SONG_SHARED));
 tap(&s,A);done(&s);assert(s.project_style&STORY_STYLE_SONG_SHARED);
 assert(s.bond==(bond>=STORY_STAT_MAX?STORY_STAT_MAX:bond+1u));
 unsigned saved=s.bond;tap(&s,A);done(&s);assert(s.bond==saved);
 uint8_t raw[STORY_SAVE_BYTES],normal[STORY_SAVE_BYTES];story_save_encode(&s,raw);assert(story_save_decode(raw,normal)&&(normal[29]&STORY_STYLE_SONG_SHARED));
 track(&s,1);goal(&s,14,6,"PAINT");track(&s,2);goal(&s,21,7,"TERMINAL");
 s.rescue_stage=STORY_RESCUE_MISSING;goal(&s,6,11,"WITNESS");
 s.rescue_stage=STORY_RESCUE_NONE;s.life_stage=STORY_LIFE_CLINIC;goal(&s,16,5,"CLINIC");
 assert(!story_track_available(&s,0)&&story_track_available(&s,3));
 puts("studio campaign: route, basic editor, four takes, six-tab unlock, share reward, save and urgent priorities OK");
 return 0;
}
