#include "hd_canvas.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {uint8_t before[32],pixels[HD_BYTES],after[32];} buf;
static uint8_t comparison[HD_BYTES];
static void stand(Story *s,int x,int y,unsigned f){
 assert(!story_solid(x,y));s->x=(uint8_t)x;s->y=(uint8_t)y;
 s->prev_x=s->x;s->prev_y=s->y;s->anim=0;s->facing=(uint8_t)f;
}
static void render(const Story *s,unsigned tick,int perspective){
 memset(&buf,0xA7,sizeof buf);
 if(perspective)hd_first_person_draw(buf.pixels,s,tick);
 else hd_isometric_draw(buf.pixels,s,tick);
 for(unsigned i=0;i<32u;i++)assert(buf.before[i]==0xA7&&buf.after[i]==0xA7);
 for(unsigned i=0;i<HD_BYTES;i++)assert(buf.pixels[i]<64u);
}
static unsigned diff(int x,int y,int w,int h){
 unsigned n=0;
 for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)
  n+=comparison[j*HD_W+i]!=buf.pixels[j*HD_W+i];
 return n;
}
static unsigned region_hash(int x,int y,int w,int h){
 unsigned n=2166136261u;
 for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)
  n=(n^buf.pixels[j*HD_W+i])*16777619u;
 return n;
}
int main(void){
 Story s;story_new(&s,0);
 s.dialogue=0;s.cinema=CINE_NONE;s.flags=STORY_FULL;
 stand(&s,8,12,3);
 for(int view=0;view<=1;view++){
  s.rescue_stage=STORY_RESCUE_NONE;render(&s,155u,view);
  memcpy(comparison,buf.pixels,HD_BYTES);
  s.rescue_stage=STORY_RESCUE_MISSING;render(&s,155u,view);
  assert(hd_mara_absent(&s));
  unsigned changed=view?diff(80,48,80,100):diff(119,42,34,37);
  assert(changed>20u);
  unsigned absent=region_hash(80,40,100,100);
  s.rescue_stage=STORY_RESCUE_LOST;render(&s,155u,view);
  if(view)assert(absent==region_hash(80,40,100,100));
  s.rescue_stage=STORY_RESCUE_SAVED;render(&s,155u,view);
  assert(!hd_mara_absent(&s));
  if(view)assert(!memcmp(comparison,buf.pixels,HD_BYTES));
  else assert(diff(126,48,21,21)==0u);
 }
 s.rescue_stage=STORY_RESCUE_NONE;
 stand(&s,14,7,3);
 unsigned stage_hash[5];
 for(unsigned phase=0;phase<=4u;phase++){
  s.art_steps=(uint8_t)phase;render(&s,155u,0);
  stage_hash[phase]=region_hash(125,51,22,23);
  for(unsigned prior=0;prior<phase;prior++)assert(stage_hash[prior]!=stage_hash[phase]);
 }
 s.project_style=STORY_STYLE_ART_WARM;render(&s,155u,0);
 assert(region_hash(125,51,22,23)!=stage_hash[4]);
 stand(&s,19,8,3);
 for(unsigned phase=0;phase<=4u;phase++){
  s.song_steps=(uint8_t)phase;render(&s,155u,0);
  stage_hash[phase]=region_hash(125,60,26,22);
  for(unsigned prior=0;prior<phase;prior++)assert(stage_hash[prior]!=stage_hash[phase]);
 }
 puts("landmarks: guarded street art, both camera absence states and five real creative stages");
 return 0;
}
