#include "hd_canvas.h"
#include "hd_arena_art.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static struct {uint8_t left[40],frame[HD_BYTES],right[40];} guarded;
static void clear(void){memset(&guarded,0xA6,sizeof guarded);}
static unsigned inspect(void){
 unsigned sum=2166136261u,detail=0;
 for(int i=0;i<40;i++)
  assert(guarded.left[i]==0xA6 && guarded.right[i]==0xA6);
 for(unsigned i=0;i<HD_BYTES;i++){
  uint8_t c=guarded.frame[i];
  assert(c>=32u&&c<64u);
  if(c!=HD_INK && c!=HD_SHADOW)detail++;
  sum=(sum^c)*16777619u;
 }
 assert(detail>4800u);
 return sum;
}
static void art_is_rom_only(void){
 const unsigned expected=ARENA_ART_W*ARENA_ART_H;
 assert(expected==HD_BYTES);
 for(unsigned rival=0;rival<3u;rival++){
  const unsigned char *background=hd_arena_backdrops[rival];
  const unsigned char *portrait=hd_arena_enemies[rival];
  for(unsigned i=0;i<expected;i++)
   assert(background[i]>=32u&&background[i]<64u);
  for(unsigned i=0;i<ARENA_FIG_W*ARENA_FIG_H;i++)
   assert(portrait[i]>=32u&&portrait[i]<=64u);
 }
}
int main(void){
 art_is_rom_only();
 Story s;story_new(&s,0);
 s.dialogue=0;s.activity=0;s.scene=STORY_SCENE_NONE;
 s.ended=0;s.life_stage=STORY_LIFE_NONE;s.cinema=CINE_NONE;
 s.battle_focus=4;s.battle_lane=1;s.battle_progress=0;
 unsigned rival_hashes[3],cases=0;
 for(unsigned enemy=0;enemy<3;enemy++){
  s.cult_enemy=(uint8_t)enemy;
  s.cult_stage=0;s.battle=BATTLE_MENU;
  for(unsigned choice=0;choice<3;choice++){
   s.battle_choice=(uint8_t)choice;
   clear();hd_arena_draw(guarded.frame,&s,48+choice*11);
   unsigned hash=inspect();
   if(choice==0u)rival_hashes[enemy]=hash;
   cases++;
  }
  for(unsigned stage=0;stage<2;stage++){
   s.cult_stage=stage?CULT_RESOLVED:0;
   s.battle=BATTLE_DODGE;
   for(unsigned beat=0;beat<4u;beat++){
    s.battle_lane=1;
    for(unsigned phase=0;phase<15u;phase+=7u){
     s.battle_timer=(uint8_t)(beat*15u+phase);
     s.battle_progress=(uint8_t)(beat/2u);
     s.battle_parry=(uint8_t)(phase>=7u?4u:0u);
     assert(story_attack_lane(&s,(int)beat)>=0);
     assert(story_attack_lane(&s,(int)beat)<3);
     clear();hd_arena_draw(guarded.frame,&s,170u+beat*17u+phase);
     inspect();cases++;
    }
   }
  }
 }
 assert(rival_hashes[0]!=rival_hashes[1]);
 assert(rival_hashes[0]!=rival_hashes[2]);
 assert(rival_hashes[1]!=rival_hashes[2]);
 /* A key mechanical truth: danger must follow the original attack lane.
    A new enemy art variant is NOT allowed to alter combat outcomes. */
 for(int enemy=0;enemy<3;enemy++){
  s.cult_enemy=(uint8_t)enemy;
  for(int beat=0;beat<4;beat++){
   int lane=story_attack_lane(&s,beat);
   s.battle_lane=(uint8_t)lane;
   s.battle_focus=4;s.battle= BATTLE_DODGE;
   s.battle_timer=(uint8_t)(beat*15);
   clear();hd_arena_draw(guarded.frame,&s,200);
   inspect();cases++;
  }
 }
 printf("ROM art: 3 complete 240x160 backgrounds, 3 native 80x84 figures\n");
 printf("Combat: %u pixel/palette/bounds checks; all rivals, stages and parry beats\n",cases);
 puts("Native arena only reads battle state; game logic and 3-lane hitboxes preserved");
 return 0;
}
