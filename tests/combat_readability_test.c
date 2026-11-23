#include "story.h"
#include "hd_canvas.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void press(Story *s,unsigned key){story_tick(s,0);story_tick(s,(uint16_t)key);story_tick(s,0);}
static struct {uint8_t before[24],p[HD_BYTES],after[24];} frame;
static uint32_t render(Story *s,unsigned ticks){
 memset(&frame,0xA1,sizeof frame);
 hd_arena_draw(frame.p,s,ticks);
 uint32_t hash=2166136261u;
 for(unsigned i=0;i<HD_BYTES;i++){
  assert(frame.p[i]>=32u&&frame.p[i]<64u);
  hash=(hash^frame.p[i])*16777619u;
 }
 for(int i=0;i<24;i++)assert(frame.before[i]==0xA1&&frame.after[i]==0xA1);
 return hash;
}
/* FACTS and CARE now offer different, fully VBlank-timed tactics.
   These checks use normal story_tick input, not a fake combat clock. */
static void verify_tactics(void){
 assert(STORY_FACTS_PARRY_TICKS==15u);
 assert(STORY_CARE_PARRY_TICKS==12u);
 assert(STORY_FACTS_PARRY_COOLDOWN==24u);
 assert(STORY_CARE_PARRY_COOLDOWN==30u);
 for(unsigned mode=0;mode<2u;mode++){
  Story s;story_new(&s,0);s.dialogue=0;s.cinema=CINE_NONE;
  s.battle=BATTLE_DODGE;s.battle_choice=(uint8_t)mode;
  s.battle_focus=4;s.battle_lane=1;s.battle_timer=0;
  s.battle_parry=s.battle_parry_cd=0;s.battle_guard=(uint8_t)(mode==1u);
  s.old_keys=0;
  unsigned window=mode?STORY_CARE_PARRY_TICKS:STORY_FACTS_PARRY_TICKS;
  unsigned cooldown=mode?STORY_CARE_PARRY_COOLDOWN:STORY_FACTS_PARRY_COOLDOWN;
  story_tick(&s,1u); /* true one-frame A button press */
  assert(s.battle_parry==window && s.battle_parry_cd==cooldown);
  uint32_t active=render(&s,72u);
  for(unsigned n=0;n<window;n++)story_tick(&s,0u);
  assert(s.battle_parry==0u);
  assert(s.battle_parry_cd==cooldown-window);
  uint32_t waiting=render(&s,72u);
  assert(waiting!=active);
  for(unsigned n=window;n<cooldown;n++)story_tick(&s,0u);
  assert(!s.battle_parry_cd);
  s.battle_timer=0;s.old_keys=0;
  uint32_t ready=render(&s,72u);
  assert(ready!=waiting);
  story_tick(&s,1u);
  assert(s.battle_parry==window && s.battle_parry_cd==cooldown);
 }
 /* FACTS counters restore one focus, but never exceed the normal cap.
    CARE shield remains independent and is spent on the first missed hit. */
 Story s;story_new(&s,0);s.dialogue=0;s.cinema=CINE_NONE;
 s.battle=BATTLE_DODGE;s.battle_choice=0;s.battle_focus=3;
 s.battle_timer=STORY_BATTLE_BEAT-1u;
 s.battle_lane=(uint8_t)story_attack_lane(&s,0);s.battle_parry=2u;
 story_tick(&s,0u);assert(s.battle_focus==4u);
 s.battle_choice=1u;s.battle_guard=1u;s.battle_focus=3u;
 s.battle_parry=0u;s.battle_timer=STORY_BATTLE_BEAT-1u;
 s.battle_lane=(uint8_t)story_attack_lane(&s,0);
 story_tick(&s,0u);
 assert(!s.battle_guard&&s.battle_focus==3u);
 assert(sizeof(Story)<64u);
}
int main(void){
 verify_tactics();
 Story s;story_new(&s,0);s.dialogue=0;s.x=2;s.y=7;s.facing=0;
 press(&s,1);assert(s.battle==BATTLE_MENU&&s.battle_focus==4);
 assert(STORY_BATTLE_BEAT==30u&&STORY_BATTLE_PARRY>=10u);
 uint32_t menu0=render(&s,0);
 press(&s,16);assert(s.battle_choice==1u); /* CARE shields one hit. */
 uint32_t menu_care=render(&s,0);assert(menu0!=menu_care);
 press(&s,1);assert(s.battle==BATTLE_DODGE&&s.battle_guard==1u);
 /* press() also delivers a release tick; start this exact beat at zero. */
 s.battle_timer=0;
 s.battle_lane=(uint8_t)story_attack_lane(&s,0);
 /* First full half-second contains no damage before the stated tell. */
 for(unsigned i=0;i<STORY_BATTLE_BEAT-1u;i++){
  story_tick(&s,0);assert(s.battle_focus==4u&&s.battle_guard==1u);
 }
 assert(s.battle_timer==STORY_BATTLE_BEAT-1u);
 uint32_t telegraph=render(&s,STORY_BATTLE_BEAT-1u);
 story_tick(&s,0); /* The first paper hits, the CARE shield absorbs it. */
 assert(s.battle_focus==4u&&!s.battle_guard);
 assert(telegraph!=render(&s,STORY_BATTLE_BEAT));
 /* The next lane is the one drawn by the shared attack function. A timed
    parry protects it. Merely holding A does not sustain the parry. */
 s.battle_lane=(uint8_t)story_attack_lane(&s,1);
 s.battle_timer=(uint8_t)(STORY_BATTLE_BEAT*2u-5u);
 s.battle_parry=0;s.battle_parry_cd=0;s.old_keys=0;
 story_tick(&s,1); /* five ticks before the strike */
 assert(s.battle_parry>0u && s.battle_parry_cd>0u);
 for(int k=0;k<4;k++)story_tick(&s,0);
 assert(s.battle_timer==STORY_BATTLE_BEAT*2u);
 assert(s.battle_focus==4u);
 /* With guard/parry both absent, standing in the warned lane loses one. */
 s.battle_timer=(uint8_t)(STORY_BATTLE_BEAT*3u-1u);
 s.battle_lane=(uint8_t)story_attack_lane(&s,2);
 s.battle_parry=0;s.battle_guard=0;
 story_tick(&s,0);
 assert(s.battle_focus==3u);
 /* The final strike ends wave one, not the entire fight. */
 s.battle_lane=(uint8_t)((story_attack_lane(&s,3)+1)%3);
 s.battle_timer=(uint8_t)(STORY_BATTLE_BEAT*4u-1u);
 story_tick(&s,0);
 assert(s.battle==BATTLE_MENU&&s.battle_progress==1u);
 /* Nonviolent exit stays an explicit, nonpunishing option. */
 press(&s,2);
 assert(s.battle==BATTLE_NONE && s.dialogue);
 puts("combat: FACTS 15/24 quick counter, CARE 12/30 guarded parry, 30-tick warnings, dynamic native HUD, focus cap, two waves and safe exit OK");
 return 0;
}
