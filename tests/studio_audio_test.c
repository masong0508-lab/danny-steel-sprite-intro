#include "music.h"
#include "studio.h"
#include "data.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
volatile uint16_t test_sound_regs[20];
volatile uint16_t test_wave_regs[8];
volatile uint16_t test_timer;
const int MUSIC_COUNT=1,MUSIC_LOOP_IDX=0;
const u32 MUSIC_LOOP_UNITS=4000000u;
const MusEv music_events[]={{2000000,800,0,10}};
static void advance(void){test_timer=(u16)(test_timer+30000);music_poll();}
int main(void){
 studio_reset();music_init();music_start();
 music_set_cue(MUSIC_CUE_STUDIO);
 assert(music_get_cue()==MUSIC_CUE_STUDIO);
 for(int i=0;i<6;i++)advance();
 assert(studio_playhead()<STUDIO_STEPS);
 uint16_t note1=test_sound_regs[2];
 /* Changes to the musical sequence are not decorations. */
 studio_change(STUDIO_NOTES,0u,1);
 studio_motif_step(0u,&(MotifBeat){0}); /* bound and playable */
 for(int i=0;i<50;i++)advance();
 assert(test_sound_regs[2]!=0u);
 assert(music_get_cue()==MUSIC_CUE_STUDIO);
 assert(note1!=0u);
 music_set_cue(MUSIC_CUE_STREETS);
 for(int i=0;i<50&&music_get_cue()!=MUSIC_CUE_STREETS;i++)advance();
 assert(music_get_cue()==MUSIC_CUE_STREETS);
 /* The original title and ending still have absolute cue priority. */
 music_set_cue(MUSIC_CUE_TITLE);
 assert(music_get_cue()==MUSIC_CUE_TITLE);
 music_begin_ending(2u);
 music_set_cue(MUSIC_CUE_STUDIO);
 assert(music_get_cue()==MUSIC_CUE_HERE_TODAY);
 puts("studio PSG preview: selected live cue, sequenced notes, transport exit and non-interruption of ending score OK");
 return 0;
}
