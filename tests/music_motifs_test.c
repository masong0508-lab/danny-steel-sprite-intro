#include "music_motifs.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
static void test_loop(unsigned cue){
    MotifBeat a,b;
    int heard=0,bass=0,comp=0,drums=0;
    assert(music_motif_ticks(cue)>30000);
    for(unsigned k=0;k<32;k++){
        music_motif_step(cue,k,&a);
        music_motif_step(cue,k+32,&b);
        assert(!memcmp(&a,&b,sizeof(a)));
        if(a.lead>0){heard++;assert(a.lead>=48&&a.lead<=96);}
        if(a.bass){bass++;assert(a.bass>=36&&a.bass<=96);}
        if(a.comp){comp++;assert(a.comp>=36&&a.comp<=96);}
        if(a.noise){drums++;assert(a.noise<3);}
        assert(a.volume<=15&&a.duty<4);
    }
    assert(heard>=6 && bass>=4);
    if(cue==MUSIC_CUE_CLINIC || cue==MUSIC_CUE_ROOFTOP)assert(drums==0);
    else assert(comp>0 && drums>0);
}
int main(void){
    for(unsigned c=1;c<MUSIC_CUE_HERE_TODAY;c++)test_loop(c);
    MotifBeat x;
    music_motif_step(MUSIC_CUE_STREETS,0,&x);assert(x.lead==67);
    music_motif_step(MUSIC_CUE_STREETS,2,&x);assert(x.lead==72);
    music_motif_step(MUSIC_CUE_FARCE,0,&x);assert(x.lead==72);
    music_motif_step(MUSIC_CUE_FARCE,3,&x);assert(x.lead==74);
    music_motif_step(MUSIC_CUE_ORCHARD,0,&x);assert(x.lead==70);
    music_motif_step(MUSIC_CUE_ORCHARD,3,&x);assert(x.lead==68);
    music_motif_step(MUSIC_CUE_CLINIC,0,&x);assert(x.lead==71);
    assert(strstr(music_motif_title(MUSIC_CUE_LEGACY),"CLOSER TO THE END"));
    assert(MUSIC_HERE_COUNT>=80);
    assert(music_here_notes[0].at==9 && music_here_notes[0].pitch==71);
    assert(music_here_notes[MUSIC_HERE_COUNT-1].at>4000);
    for(unsigned i=0;i<MUSIC_HERE_COUNT;i++){
      assert(music_here_notes[i].length && music_here_notes[i].phrase<20);
      assert(music_here_notes[i].pitch>=48 && music_here_notes[i].pitch<=96);
      if(i)assert(music_here_notes[i].at>=music_here_notes[i-1].at);
    }
    puts("Danny Steel: 6 authentic BORE motif arrangements, exact phrase loops, GBA MIDI range and silent clinic drums OK");
    return 0;
}
