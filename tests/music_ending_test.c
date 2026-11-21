#include "music.h"
#include "data.h"
#include <assert.h>
#include <stdio.h>
volatile uint16_t test_sound_regs[20];
volatile uint16_t test_wave_regs[8];
volatile uint16_t test_timer;
const int MUSIC_COUNT=1,MUSIC_LOOP_IDX=0;
const u32 MUSIC_LOOP_UNITS=4000000u;
const MusEv music_events[]={{2000000,800,0,10}};
static void advance(uint16_t units){test_timer=(uint16_t)(test_timer+units);music_poll();}
static void advance_sec(unsigned seconds){for(unsigned i=0;i<seconds*9;i++)advance(29127);}
int main(void){
 music_init();music_start();
 music_set_cue(MUSIC_CUE_STREETS);advance(15000);
 music_begin_ending(1);assert(music_ending_phase()==1);
 music_begin_ending(1); /* no restart */
 music_set_cue(MUSIC_CUE_ORCHARD); /* switching shots must NOT switch the score */
 advance_sec(60);
 assert(music_ending_phase()==1);
 assert(music_get_cue()==MUSIC_CUE_HERE_TODAY);
 music_begin_ending(1);
 advance_sec(14);
 assert(music_ending_phase()==2);
 assert(music_get_cue()==MUSIC_CUE_LEGACY);
 advance_sec(18);
 assert(music_ending_phase()==3);
 music_set_cue(MUSIC_CUE_TITLE);
 assert(music_ending_phase()==0);
 for(int ending=2;ending<=3;ending++){
   music_begin_ending(ending);advance_sec(75);
   assert(music_ending_phase()==3);
   assert(music_get_cue()==MUSIC_CUE_HERE_TODAY);
   music_set_cue(MUSIC_CUE_TITLE);
   assert(music_ending_phase()==0);
 }
 puts("HERE TODAY: full 72s ending score, good-only 16s coda, neutral/bad silence, restart safety OK");
 return 0;
}
