#include "story.h"
#include <assert.h>
#include <stdio.h>
static void hit(Story *s){
 s->battle=BATTLE_DODGE;s->battle_timer=STORY_BATTLE_BEAT-1u;
 s->battle_lane=(uint8_t)story_attack_lane(s,0);
 s->battle_focus=2u;s->battle_parry=2u;s->old_keys=0u;
 story_tick(s,0u);
}
int main(void){
 Story s;story_new(&s,0);s.dialogue=0;s.cinema=CINE_NONE;
 s.battle_choice=0u;hit(&s);
 assert(s.battle_timer==STORY_BATTLE_BEAT);
 assert(s.battle_focus==3u);
 s.battle_choice=1u;hit(&s);
 assert(s.battle_focus==2u);
 s.battle_guard=1u;s.battle_parry=0u;
 s.battle_timer=STORY_BATTLE_BEAT-1u;
 story_tick(&s,0u);
 assert(s.battle_guard==0u&&s.battle_focus==2u);
 s.battle_choice=0u;s.battle_focus=4u;s.battle_parry=2u;
 s.battle_timer=STORY_BATTLE_BEAT-1u;
 story_tick(&s,0u);
 assert(s.battle_focus==4u);
 puts("FACTS timing restores focus, CARE guard blocks one strike, no focus overfill passed");
 return 0;
}
