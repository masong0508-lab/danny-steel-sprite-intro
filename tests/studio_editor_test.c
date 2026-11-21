#include "story.h"
#include "story_draw.h"
#include "studio.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define A 1u
#define B 2u
#define RIGHT 16u
#define UP 64u
#define DOWN 128u
#define R 256u
#define L 512u
#define SELECT 4u
static void press(Story *s,unsigned k){
 story_tick(s,0);story_tick(s,(uint16_t)k);story_tick(s,0);
}
static void done(Story *s){
 for(int i=0;i<45&&s->dialogue;i++)press(s,A);
 assert(!s->dialogue);
}
static void guard(Story *s){
 struct {uint8_t before[24],pixels[120*80],after[24];} p;
 for(int view=0;view<2;view++){
  s->view=(uint8_t)view;
  memset(&p,0xCC,sizeof(p));
  story_draw(s,50,p.pixels);
  for(int i=0;i<24;i++)assert(p.before[i]==0xCC&&p.after[i]==0xCC);
  for(int i=0;i<120*80;i++)assert(p.pixels[i]<32u);
 }
}
static void enter(Story *s){
 s->x=19;s->y=8;s->facing=3;s->cooldown=0;
 s->prev_x=s->x;s->prev_y=s->y;
 press(s,A);assert(s->activity=='E');guard(s);
 press(s,RIGHT);assert(s->activity_choice==1u);
 press(s,A);assert(s->activity==STORY_ACT_STUDIO);
 assert(s->studio_tab==STUDIO_SONG);
 assert(s->studio_cursor==0u&&!s->activity_choice);
}
int main(void){
 Story s;story_new(&s,0);done(&s);
 assert(s.song_steps==0u);
 s.flags=STORY_FULL;s.life_stage=STORY_LIFE_WAIT;
 s.song_steps=STORY_PROJECT_DONE;
 enter(&s);guard(&s);
 /* Time cannot expire and send a player to the doctor while they edit. */
 s.day_frame=STORY_DAY_FRAMES-1u;
 for(int i=0;i<120;i++)story_tick(&s,0);
 assert(s.day_frame==STORY_DAY_FRAMES-1u);
 /* Subsequent menu tests should not artificially hit the next morning
    on exit; clinic/day rollover is already covered separately. */
 s.day_frame=120u;
 /* SONG setup actually changes tempo and feel. */
 uint32_t oldticks=studio_ticks();
 press(&s,UP);assert(studio_ticks()!=oldticks);
 guard(&s);
 /* R changes tab, not first-person facing or camera mode. */
 uint8_t view=s.view;
 press(&s,R);assert(s.studio_tab==STUDIO_NOTES&&s.view==view);
 press(&s,RIGHT);assert(s.studio_cursor==1u);
 assert(studio_patch()->notes[1]==0u);
 press(&s,UP);assert(studio_patch()->notes[1]==4u);
 press(&s,A);assert(studio_patch()->notes[1]==0u);
 press(&s,A);assert(studio_patch()->notes[1]==4u);guard(&s);
 /* Ordered sections alter the real melody, not only a drawn block. */
 press(&s,R);assert(s.studio_tab==STUDIO_ORDER);
 press(&s,RIGHT);assert(s.studio_cursor==1u);
 MotifBeat before,after;
 unsigned order_before=studio_patch()->order[1];
 studio_motif_step(8u,&before);
 press(&s,UP);studio_motif_step(8u,&after);
 assert(studio_patch()->order[1]!=order_before);
 assert(before.lead!=after.lead);
 guard(&s);
 press(&s,R);assert(s.studio_tab==STUDIO_LYRICS);
 const char *line=studio_lyric(studio_patch()->lyric);
 press(&s,UP);assert(strcmp(line,studio_lyric(studio_patch()->lyric)));
 guard(&s);
 press(&s,R);assert(s.studio_tab==STUDIO_CHOIR);
 studio_motif_step(0,&before);press(&s,UP);
 studio_motif_step(0,&after);assert(before.comp!=after.comp);
 guard(&s);
 press(&s,R);assert(s.studio_tab==STUDIO_PERFORM);
 press(&s,SELECT);assert(s.activity_choice==1u);
 guard(&s);
 press(&s,A);assert(s.activity_choice==0u);
 press(&s,L);assert(s.studio_tab==STUDIO_CHOIR);
 press(&s,SELECT);assert(s.activity_choice==1u);
 press(&s,B);assert(s.activity==STORY_ACT_NONE);
 assert(!s.activity_choice);assert(s.cinema==CINE_NONE);
 assert(s.song_steps==STORY_PROJECT_DONE);
 /* Full pattern persists per slot, with corruption and cross-slot
    rejection. A corrupt studio patch never damages the story save. */
 uint8_t serialized[STUDIO_EXT_LEN];
 studio_save_encode(0,serialized);
 uint8_t saved_note=studio_patch()->notes[1];
 uint8_t saved_order=studio_patch()->order[1];
 unsigned saved_lyric=studio_patch()->lyric;
 assert(!studio_save_decode(1,serialized));
 serialized[5]^=7u;
 assert(!studio_save_decode(0,serialized));
 serialized[5]^=7u;
 studio_reset();
 assert(studio_patch()->notes[1]==0u);
 assert(studio_save_decode(0,serialized));
 assert(studio_patch()->notes[1]==saved_note);
 assert(studio_patch()->order[1]==saved_order);
 assert(studio_patch()->lyric==saved_lyric);
 assert(sizeof(Story)<64u);
 puts("studio editor: unlock, visible 6-tab UI, safe 2D/3D, editable real notes, clock freeze, preview, exit and checksum slots OK");
 return 0;
}
