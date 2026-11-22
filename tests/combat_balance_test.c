#include "story.h"
#include "hd_canvas.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* The two combat choices must actually differ beyond their menu labels.
   FACTS gives a quick, skillful counter; CARE absorbs one missed attack.
   All time windows are simulation ticks, independent of frame cadence. */
static struct{uint8_t before[24],pixels[HD_BYTES],after[24];} screen;
static void render(Story *s){
 memset(&screen,0xBC,sizeof screen);
 hd_arena_draw(screen.pixels,s,0u);
 for(unsigned i=0;i<24u;i++)
  assert(screen.before[i]==0xBC&&screen.after[i]==0xBC);
 for(unsigned i=0;i<HD_BYTES;i++)
  assert(screen.pixels[i]>=32u&&screen.pixels[i]<64u);
}
static uint32_t hash(void){
 uint32_t h=2166136261u;
 for(unsigned i=0;i<HD_BYTES;i++)h=(h^screen.pixels[i])*16777619u;
 return h;
}
static void base(Story *s,unsigned mode){
 story_new(s,0);s->dialogue=0;s->cinema=CINE_NONE;
 s->battle=BATTLE_DODGE;s->battle_choice=(uint8_t)mode;
 s->battle_focus=3;s->battle_timer=0;s->battle_progress=0;
 s->battle_parry=s->battle_parry_cd=0;
 s->battle_lane=1;s->battle_guard=mode==1u;
 s->old_keys=0;
}
static void timing(unsigned mode,unsigned window,unsigned cooldown){
 Story s;base(&s,mode);
 story_tick(&s,1u); /* a single sampled A press */
 assert(s.battle_parry==window&&s.battle_parry_cd==cooldown);
 for(unsigned t=1;t<window;t++){
  story_tick(&s,0u);
  assert(s.battle_parry==window-t);
 }
 story_tick(&s,0u);assert(s.battle_parry==0u);
 assert(s.battle_parry_cd==cooldown-window);
}
static void collision(unsigned mode){
 Story s;base(&s,mode);
 int target=story_attack_lane(&s,0);
 s.battle_lane=(uint8_t)target;s.battle_timer=STORY_BATTLE_BEAT-1u;
 s.battle_parry=2;s.battle_focus=2;
 story_tick(&s,0u);
 assert(s.battle_timer==STORY_BATTLE_BEAT&&s.battle_focus==(mode==0u?3u:2u));
 /* A dodge outside the target doesn't restore focus. */
 base(&s,mode);s.battle_timer=STORY_BATTLE_BEAT-1u;s.battle_parry=2;s.battle_focus=2;
 s.battle_lane=(uint8_t)((story_attack_lane(&s,0)+1)%3);
 story_tick(&s,0u);
 assert(s.battle_focus==2u);
 /* No counter may overfill focus. */
 base(&s,mode);s.battle_timer=STORY_BATTLE_BEAT-1u;s.battle_parry=2;s.battle_focus=4;
 s.battle_lane=(uint8_t)story_attack_lane(&s,0);
 story_tick(&s,0u);assert(s.battle_focus==4u);
 /* A missed hit costs focus for FACTS; CARE spends its shield first. */
 base(&s,mode);s.battle_timer=STORY_BATTLE_BEAT-1u;s.battle_focus=3;
 s.battle_lane=(uint8_t)story_attack_lane(&s,0);
 story_tick(&s,0u);
 assert(s.battle_focus==(mode==1u?3u:2u));
 assert(s.battle_guard==0u);
 if(mode==1u){
  s.battle_timer=STORY_BATTLE_BEAT*2u-1u;s.battle_lane=(uint8_t)story_attack_lane(&s,1);
  story_tick(&s,0u);assert(s.battle_focus==2u);
 }
}
int main(void){
 assert(STORY_FACTS_PARRY_TICKS>STORY_CARE_PARRY_TICKS);
 assert(STORY_FACTS_PARRY_COOLDOWN<STORY_CARE_PARRY_COOLDOWN);
 timing(0u,STORY_FACTS_PARRY_TICKS,STORY_FACTS_PARRY_COOLDOWN);
 timing(1u,STORY_CARE_PARRY_TICKS,STORY_CARE_PARRY_COOLDOWN);
 collision(0u);collision(1u);
 Story s;base(&s,0u);s.battle=BATTLE_MENU;
 render(&s);uint32_t facts=hash();
 s.battle_choice=1u;render(&s);uint32_t care=hash();
 s.battle_choice=2u;render(&s);assert(hash()!=facts);
 assert(facts!=care);
 base(&s,0u);render(&s);uint32_t ready=hash();
 s.battle_parry_cd=5;render(&s);assert(hash()!=ready);
 /* The tactic remains transient, both character and saves stay compact. */
 assert(sizeof(Story)<64u);
 puts("combat: facts quick parry/counter, care guard, cooldown, max focus, both HUD presentations and frame guards OK");
 return 0;
}
