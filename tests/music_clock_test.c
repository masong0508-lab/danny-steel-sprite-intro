#include "music.h"
#include "data.h"
#include <assert.h>
#include <stdio.h>

volatile uint16_t test_sound_regs[20];
volatile uint16_t test_wave_regs[8];
volatile uint16_t test_timer;
const int MUSIC_COUNT=1,MUSIC_LOOP_IDX=0;
const u32 MUSIC_LOOP_UNITS=4000000u;
const MusEv music_events[]={{2000000u,800u,0u,10u}};

int main(void){
 music_init();music_start();music_begin_ending(2u);
 REG_IME=1u;
 uint16_t original[14];
 for(unsigned i=0;i<14u;i++)original[i]=test_sound_regs[i];
 /* 4389 timer units per GBA video frame. A stalled painter can cross
    the 16-bit timer's250ms wrap without losing elapsed music time. */
 for(unsigned frame=0;frame<4300u;frame++){
  test_timer=(uint16_t)(test_timer+4389u);
  music_clock_vblank();
 }
 /* Clock extension has not sequenced or touched any PSG register. */
 for(unsigned i=0;i<14u;i++)assert(test_sound_regs[i]==original[i]);
 assert(music_ending_phase()==1u&&REG_IME==1u);
 music_poll();assert(music_ending_phase()==1u&&REG_IME==1u);
 /* Main sampling immediately after IRQ sampling must not count the
    same elapsed interval twice. The score ends only after72.5s. */
 for(unsigned frame=4300u;frame<4330u;frame++){
  test_timer=(uint16_t)(test_timer+4389u);
  music_clock_vblank();music_poll();
 }
 assert(music_ending_phase()==1u);
 for(unsigned frame=4330u;frame<4350u;frame++){
  test_timer=(uint16_t)(test_timer+4389u);
  music_clock_vblank();music_poll();
 }
 assert(music_ending_phase()==3u&&REG_IME==1u);
 puts("Music clock: timer wraps survive blocked polling, IRQ never sequences PSG, main/IRQ sampling does not double-count, IME restored");
 return 0;
}
