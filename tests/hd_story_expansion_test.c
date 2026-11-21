#include "hd_canvas.h"
#include "studio.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static struct {
 uint8_t guard[64];
 uint8_t pixels[HD_BYTES];
 uint8_t tail[64];
} frame;
static void clear(void){memset(&frame,0xA7,sizeof(frame));}
static unsigned checksum(void){
 unsigned hash=2166136261u,changed=0;
 for(int i=0;i<64;i++)
  assert(frame.guard[i]==0xA7&&frame.tail[i]==0xA7);
 for(unsigned i=0;i<HD_BYTES;i++){
  uint8_t p=frame.pixels[i];
  assert(p<64u);
  if(p!=HD_INK && p!=HD_SHADOW)changed++;
  hash=(hash^p)*16777619u;
 }
 assert(changed>1600u);
 return hash;
}
int main(void){
 Story s;
 story_new(&s,0);studio_reset();
 s.dialogue=0;s.battle=0;s.activity=0;s.ended=0;
 s.life_stage=STORY_LIFE_NONE;
 /* The native renderer must support every scene, rather than allowing a
    120x80 cutscene to reappear midway through major story chapters. */
 unsigned count=0;
 for(unsigned scene=1;scene<CINE_COUNT;scene++){
  assert(hd_cinema_supports(scene));
  s.cinema=(uint8_t)scene;
  for(int page=0;page<4;page++){
   s.page=(uint8_t)page;
   clear();hd_cinema_draw(frame.pixels,&s,55u+(unsigned)page*37u);
   (void)checksum();count++;
  }
 }
 assert(count>=96u);
 /* The final tribute is not a blank stage: recordshop and concert
    must be different real sets with light, albums and a performer. */
 s.cinema=CINE_GOOD;s.page=3;s.ended=0;
 clear();hd_cinema_draw(frame.pixels,&s,2910u);
 unsigned record_store=checksum(),warm_record=0;
 /* At the actual held album sleeve the printed cover has a material
    color, not the black turntable that occupied its old floating spot. */
 assert(frame.pixels[86u*HD_W+69u]==HD_GLASS_LIT);
 assert(frame.pixels[91u*HD_W+75u]==HD_GOLD);
 for(int y=20;y<99;y++)for(int x=133;x<231;x++){
  unsigned c=frame.pixels[y*HD_W+x];
  if(c==HD_GOLD||c==HD_COPPER||c==HD_PAPER)warm_record++;
 }
 assert(warm_record>80u);
 s.page=6;
 clear();hd_cinema_draw(frame.pixels,&s,4760u);
 unsigned memorial_stage=checksum(),stage_light=0;
 /* Mara's actual extended hand meets the fixed microphone at x140. */
 assert(frame.pixels[86u*HD_W+140u]==HD_INK);
 for(int y=20;y<120;y++)for(int x=18;x<223;x++){
  unsigned c=frame.pixels[y*HD_W+x];
  if(c==HD_GOLD||c==HD_LAMP||c==HD_GLASS_LIT)stage_light++;
 }
 assert(memorial_stage!=record_store&&stage_light>70u);
 /* The stage remains animated instead of becoming a static victory card. */
 clear();hd_cinema_draw(frame.pixels,&s,4830u);
 assert(checksum()!=memorial_stage);
 /* The endings must follow the original director's shot mapping; rescuing
    Mara changes the visible scene, but not the timing or score length. */
 unsigned endings[3]={STORY_END_GOOD,STORY_END_NEUTRAL,STORY_END_BAD};
 unsigned frames[]={0u,360u,790u,1120u,1550u,2040u,2480u,
                   2780u,3200u,3640u,4000u,4330u,4510u,4880u,
                   5100u,5310u};
 for(unsigned e=0;e<3;e++)for(unsigned k=0;k<16;k++){
  s.ended=1;s.ending=(uint8_t)endings[e];s.dialogue=0;
  s.rescue_stage=STORY_RESCUE_LOST;
  s.finale_frame=(uint16_t)frames[k];
  if(endings[e]!=STORY_END_GOOD && frames[k]>4350u)continue;
  clear();hd_ending_draw(frame.pixels,&s,frames[k]);
  (void)checksum();
 }
 /* A rescued Mara MUST lead to different art at the door and cafe. */
 s.ending=STORY_END_BAD;s.ended=1;s.dialogue=0;
 s.finale_frame=1080u;s.rescue_stage=STORY_RESCUE_LOST;
 clear();hd_ending_draw(frame.pixels,&s,s.finale_frame);
 unsigned missing_cafe=checksum();
 s.rescue_stage=STORY_RESCUE_AFTERSHOCK;
 clear();hd_ending_draw(frame.pixels,&s,s.finale_frame);
 unsigned safe_cafe=checksum();
 assert(missing_cafe!=safe_cafe);
 s.finale_frame=2000u;s.rescue_stage=STORY_RESCUE_LOST;
 clear();hd_ending_draw(frame.pixels,&s,s.finale_frame);
 unsigned closed_door=checksum();
 s.rescue_stage=STORY_RESCUE_AFTERSHOCK;
 clear();hd_ending_draw(frame.pixels,&s,s.finale_frame);
 unsigned opened_door=checksum();
 assert(closed_door!=opened_door);
 /* Different actors share a body skeleton but are visually identifiable. */
 unsigned people[5];
 for(int who=1;who<=5;who++){
  clear();hd_rect(frame.pixels,0,0,HD_W,HD_H,HD_ROAD);
  hd_actor_stage(frame.pixels,120,147,who,110u,3);
  people[who-1]=checksum();
  for(int prev=0;prev<who-1;prev++)assert(people[who-1]!=people[prev]);
 }
 puts("native 240x160 cinema: all 24 scenes and 3 scored endings render safely");
 puts("rescue-specific cafe and door scenes remain different; five characters distinct");
 return 0;
}
