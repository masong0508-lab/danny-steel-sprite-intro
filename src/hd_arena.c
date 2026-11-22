#include "hd_canvas.h"
#include "hd_arena_art.h"
#include <string.h>

/* Mirror Orchard was the only major game screen still enlarged from 120x80.
   A ROM-only 240x160 authored illustration for each rival replaces the
   doubled-pixel graphics; story's original three-lane combat logic remains
   sole authority on focus, parry windows and attacks.

   ROM: 3 * 38,400 scene pixels + 3 * 6,720 enemy pixels, deliberately
   trading cartridge space for a predictable full-screen copy.
   EWRAM: ZERO new framebuffers, no heap, no decompression scratch. */

static void enemy(uint8_t *p,unsigned rival,unsigned tick){
 const unsigned char *asset=hd_arena_enemies[rival%3u];
 /* Two-pixel coat motion, with grounded boot shadow inside the art.
    All six enemy-state variants share the same actual visible character
    geometry rather than changing proportions mid-battle. */
 int y0=29+(int)((tick/23u)&1u);
 int x0=80;
 for(int y=0;y<ARENA_FIG_H;y++){
  const unsigned char *src=asset+y*ARENA_FIG_W;
  uint8_t *dst=p+(y0+y)*HD_W+x0;
  for(int x=0;x<ARENA_FIG_W;x++)
   if(src[x]!=64u)dst[x]=src[x];
 }
}
static void telegraph(uint8_t *p,const Story *s,unsigned frame){
 (void)frame;
 if(s->battle!=BATTLE_DODGE)return;
 unsigned beat=s->battle_timer/15u;
 unsigned phase=s->battle_timer%15u;
 if(beat>=4u)return;
 unsigned lane=(unsigned)story_attack_lane(s,(int)beat);
 int target=57+(int)lane*63;
 /* Physical danger indication agrees EXACTLY with collision lane,
    using story_attack_lane() in both the renderer and story_tick(). */

 hd_ellipse(p,target,129,21,4,HD_RED_SHADE);
 hd_ellipse(p,target,129,16,2,HD_COPPER);
 /* A crumpled paper warning travels down the appropriate attack
    corridor. On the fifteenth story frame the game checks this lane. */
 int x=120+(target-120)*(int)phase/14;
 int y=82+(int)phase*44/14;
 int size=5+(int)phase*9/14;
 hd_ellipse(p,x+2,y+2,size/2+1,size/2+1,HD_SHADOW);
 hd_ellipse(p,x,y,size/2+1,size/2+1,HD_PAPER);
 hd_line(p,x-size/3,y-size/3,x+size/3,y+size/3,HD_STONE);
 hd_line(p,x-size/3,y+size/3,x+size/3,y-size/3,HD_COPPER);
 if(phase>=8u)hd_pixel(p,x,y,HD_RED);

}
static void badge(uint8_t *p,int x,int y,const char *name,unsigned active){
 hd_rect(p,x,y,71,17,active?HD_BARK:HD_STONE_DARK);
 hd_rect(p,x+1,y+1,69,15,active?HD_COPPER:HD_INK);
 hd_rect(p,x+2,y+2,67,13,active?HD_BRICK_DARK:HD_STONE_DARK);
 hd_text(p,x+17,y+5,name,active?HD_PAPER:HD_STONE_LIT,1,0);
 if(active){
  hd_rect(p,x+4,y+5,3,7,HD_GOLD);
  hd_rect(p,x+61,y+5,3,7,HD_GOLD);
 }
}
static void huddle(uint8_t *p,const Story *s,unsigned tick){
 /* Danny moves exactly to the logical lane's end point; grounded shadow
    and consistent red coat replace the tiny red rectangle in old arena. */
 int x=57+(int)s->battle_lane*63;
 int move=(int)((tick/16u)&1u);
 hd_ellipse(p,x,130,19,4,HD_INK);
 hd_rect(p,x-16,131,32,1,HD_STONE_LIT);
 hd_actor_stage(p,x,134,1,tick,s->battle_parry?3:move?2:0);
 if(s->battle_parry){
  /* The painted guard only appears during the REAL strategy-dependent
     parry window. Neither a completed redraw nor low FPS prolongs it. */
  hd_line(p,x-26,86,x-20,79,HD_GOLD);
  hd_line(p,x-20,79,x-10,75,HD_COPPER);
  hd_line(p,x+26,86,x+20,79,HD_GOLD);
  hd_line(p,x+20,79,x+10,75,HD_COPPER);
  hd_ellipse(p,x-21,84,3,3,HD_LAMP);
  hd_ellipse(p,x+21,84,3,3,HD_LAMP);
 }
}
static void hud(uint8_t *p,const Story *s){
 hd_rect(p,0,0,HD_W,26,HD_INK);
 hd_rect(p,0,25,HD_W,2,HD_COPPER);
 hd_rect(p,7,5,3,15,HD_GOLD);
 hd_text(p,15,4,"MIRROR ORCHARD",HD_LAMP,1,1);
 hd_text(p,15,15,story_enemy_name(s),HD_PAPER,1,0);
 const char *era=s->cult_stage>=CULT_RESOLVED?"AFTER":
                 s->cult_stage?"DURING":"BEFORE";
 hd_text(p,196,4,era,HD_STONE_LIT,1,0);
 hd_text(p,155,15,"FOCUS",HD_STONE_LIT,1,0);
 for(int i=0;i<4;i++){
  int x=192+i*11;
  hd_rect(p,x,17,8,5,i<s->battle_focus?HD_GOLD:HD_STONE_DARK);
  if(i<s->battle_focus)hd_rect(p,x,17,8,1,HD_PAPER);
 }
 hd_rect(p,0,136,HD_W,24,HD_INK);
 hd_rect(p,0,136,HD_W,2,HD_COPPER);
 if(s->battle==BATTLE_MENU){
  badge(p,5,141,"FACTS",s->battle_choice==0);
  badge(p,84,141,"CARE",s->battle_choice==1);
  badge(p,163,141,"EXIT",s->battle_choice==2);
  /* Separate analog placards live beside the subject instead of
     a full-width banner cropping the rival's hair, hat or costume ears. */
  hd_rect(p,4,34,72,12,HD_INK);
  hd_rect(p,4,34,72,1,HD_COPPER);
  hd_text(p,8,37,"LEFT/RIGHT",HD_STONE_LIT,1,0);
  hd_rect(p,164,34,72,12,HD_INK);
  hd_rect(p,164,34,72,1,HD_COPPER);
  hd_text(p,168,37,"A PICK B EXIT",HD_STONE_LIT,1,0);
  hd_rect(p,34,121,180,12,HD_INK);
  hd_rect(p,34,121,180,1,HD_COPPER);
  hd_text(p,42,124,s->battle_choice==0u?
          "FACTS: QUICK PARRY / COUNTER":s->battle_choice==1u?
          "CARE: SHIELD ONE MISTAKE":"EXIT: NO PENALTY",
          HD_PAPER,1,0);
 }else{
  char wave[]="ROUND 1/2";
  wave[6]=(char)('1'+(s->battle_progress&1u));
  hd_text(p,8,143,"LEFT RIGHT / MOVE",HD_PAPER,1,0);
  hd_text(p,151,143,s->battle_parry_cd?"PARRY / WAIT":"A / PARRY READY",
          s->battle_parry_cd?HD_STONE_LIT:HD_GOLD,1,0);
  hd_text(p,8,151,wave,HD_STONE_LIT,1,0);
  char hit[]="BEAT 1/4";
  hit[5]=(char)('1'+(s->battle_timer/15u)%4u);
  hd_text(p,137,151,hit,HD_STONE_LIT,1,0);
  char incoming[]="IN 00";
  unsigned gap=15u-(s->battle_timer%15u);
  incoming[3]=(char)('0'+gap/10u);
  incoming[4]=(char)('0'+gap%10u);
  hd_text(p,203,151,incoming,HD_GOLD,1,0);
 }
}
void hd_arena_draw(uint8_t *p,const Story *s,unsigned frame){
 unsigned idx=(unsigned)s->cult_enemy%CULT_ENCOUNTERS;
 /* Direct ROM->shared EWRAM copy, never a second drawing page. */
 memcpy(p,hd_arena_backdrops[idx],HD_BYTES);
 enemy(p,idx,frame);
 if(s->battle==BATTLE_DODGE)telegraph(p,s,frame);
 huddle(p,s,frame);
 hud(p,s);
}
