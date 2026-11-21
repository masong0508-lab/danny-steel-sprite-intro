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
static void advance(void){test_timer=(u16)(test_timer+30000);music_poll();}
int main(void){
 music_init();music_start();
 assert(music_get_cue()==MUSIC_CUE_TITLE);
 music_set_cue(MUSIC_CUE_STREETS);
 assert(music_get_cue()==MUSIC_CUE_STREETS);
 advance();assert(test_sound_regs[2]&0x8000); // audible lead
 unsigned before=test_sound_regs[2];
 for(int i=0;i<5;i++)advance();
 assert(test_sound_regs[2]!=before);
 music_set_cue(MUSIC_CUE_ORCHARD);
 assert(music_get_cue()==MUSIC_CUE_STREETS);
 for(int i=0;i<40 && music_get_cue()!=MUSIC_CUE_ORCHARD;i++)advance();
 assert(music_get_cue()==MUSIC_CUE_ORCHARD);
 for(int i=0;i<30;i++)advance();
 music_set_cue(MUSIC_CUE_CLINIC);
 for(int i=0;i<40 && music_get_cue()!=MUSIC_CUE_CLINIC;i++)advance();
 assert(music_get_cue()==MUSIC_CUE_CLINIC);
 music_set_cue(MUSIC_CUE_TITLE);
 assert(music_get_cue()==MUSIC_CUE_TITLE);
 music_set_volume(-1);assert((test_sound_regs[10]&0x77)==0);
 music_set_volume(200);assert((test_sound_regs[10]&0x77)==0x77);
 puts("Danny Steel: real PSG timer, all scene cue switches, title restoration and volume clamp OK");
 return 0;
}
