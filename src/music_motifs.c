#include "music_motifs.h"
/* Authored sources (in masong0508-lab/bore-gba):
  make_whistler_rework.py  : TUNE from original Whistler Man, G C Eb C / F G Eb
  make_nursery_rework.py  : nursery music-box melody C C C D C / B E E
  make_closer_rework.py   : original Closer to the End tresillo Bb Ab Gb, B turn
  make_heretoday.py       : HERE TODAY lyric-melody PHRASES (note numbers 59...)
  BORE2 assets/songs.inc  : canonical titles and Danny Steele credits.
  Edits here are arrangement/orchestration, not claims of CD-quality reproduction.
  No heap, large audio samples, PCM buffers, or per-frame note synthesis.
*/
static const int8_t whistler[32] = {
 67, 0, 72, 0, 75, 0, 72, 0, 72, 0, 72, 0, 77, 0, 77, 0, 79, 0, 0, 0, 75, 0, 0, 0, -1, 0, 0, 0, 67, 0, 72, 0
};
static const int8_t nursery[32] = {
 72, 72, 72, 74, 72, -1, 71, 76, 76, -1, 70, 70, 70, 72, 70, -1, 69, 0, 74, 0, 72, 72, 72, 74, 72, -1, 71, 76, 76, -1, 69, 74
};
static const int8_t closer[32] = {
 70, 0, 0, 68, 0, 0, 66, 0, 70, 0, 0, 68, 0, 0, 66, 0, 71, 70, 0, 68, 0, 0, 66, 0, 70, 0, 0, 68, 0, 0, 71, -1
};
static const int8_t here[32] = {
 59, 0, 0, 0, 56, 0, 0, -1, 56, 59, 0, 60, 0, -1, 0, 0, 62, 0, 62, 0, 58, 59, 57, 55, 0, 55, 0, 0, -1, 0, 55, 0
};

static const uint8_t root_whistler[8]={43,43,48,48,41,41,38,38}; // Gm9 C9 Fadd9 D9
static const uint8_t root_nursery[8]={41,40,46,45,41,40,45,43}; // F E Bb A F E Am G
static const uint8_t root_closer[8]={36,38,41,43,36,38,39,43}; // C minor suite
static const uint8_t root_here[8]={43,43,46,46,39,39,38,38}; // G minor / Bb, sparse
static const char *const titles[MUSIC_CUE_COUNT]={
 "Original Danny Steel title bossa",
 "WHISTLER MAN - old-quarter shuffle",
 "NURSERY TIME - absurd bureaucracy",
 "CLOSER TO THE END - online uproar",
 "HERE TODAY - clinic recollection",
 "HERE TODAY - last rooftop sketch",
 "CLOSER TO THE END / WHISTLER MAN - legacy reprise",
 "HERE TODAY - full original 20-phrase ending performance"
};
const char *music_motif_title(unsigned cue){return cue<MUSIC_CUE_COUNT?titles[cue]:titles[0];}
uint32_t music_motif_ticks(unsigned cue){
 static const uint32_t ticks[MUSIC_CUE_COUNT]={93622,75618,56987,57825,100824,117377,65536,117377};
 return ticks[cue<MUSIC_CUE_COUNT?cue:0];
}
void music_motif_step(unsigned cue,unsigned step,MotifBeat *out){
 static const uint8_t comp_tones[4]={12,19,15,22};
 const int8_t *mel=whistler;
 const uint8_t *roots=root_whistler;
 unsigned x=step&31u;
 unsigned group=(x>>2)&7u;
 uint8_t level=9,duty=1,drums=0;
 switch(cue){
 case MUSIC_CUE_FARCE:
     mel=nursery;roots=root_nursery;level=11;duty=0;drums=2;break;
 case MUSIC_CUE_ORCHARD:
     mel=closer;roots=root_closer;level=12;duty=2;drums=3;break;
 case MUSIC_CUE_CLINIC:
     mel=here;roots=root_here;level=7;duty=2;drums=0;break;
 case MUSIC_CUE_ROOFTOP:
     mel=here;roots=root_here;level=6;duty=2;drums=0;break;
 case MUSIC_CUE_LEGACY:
     mel=whistler;roots=root_closer;level=13;duty=2;drums=2;break;
 default: drums=1;break;
 }
 out->lead=mel[x];
 if(cue==MUSIC_CUE_CLINIC||cue==MUSIC_CUE_ROOFTOP){
   /* HERE TODAY's original B3-to-D4 melody rises by one octave as in BORE's
      original chiptune. Let each phrase breathe, with little/no percussion. */
   if(out->lead>0)out->lead=(int8_t)(out->lead+12);
   out->bass=(x%8u==0u)?roots[group]:0;
   out->comp=(x%8u==4u && cue==MUSIC_CUE_CLINIC)?(uint8_t)(roots[group]+19):0;
 } else {
   out->bass=(x%4u==0u)?roots[group]:(x%4u==2u?(uint8_t)(roots[group]+7):0);
   out->comp=(x&1u)?(uint8_t)(roots[group]+comp_tones[(x>>1)&3u]):0;
 }
 out->noise=0;
 if(drums && (x&1u))out->noise=1;
 if(drums>1 && (x%8u)==4u)out->noise=2;
 out->volume=(uint8_t)((x%8u==0u)?level:(level>2?level-2:level));
 out->duty=duty;
}

/* BORE authorship and musical source: bore-gba/tools/make_heretoday.py,
   PHRASES (20 sections, 72 s). Absolute event times preserve expressive
   pauses and the long melody, instead of looping its first two bars. */
const HereNote music_here_notes[] = {
 {9,64,71,0},
 {73,8,68,0},
 {133,13,68,1},
 {146,15,71,1},
 {181,14,72,1},
 {295,11,74,2},
 {320,8,74,2},
 {350,10,70,2},
 {360,8,71,2},
 {377,10,69,2},
 {386,15,67,2},
 {413,20,67,2},
 {503,13,67,3},
 {529,13,69,3},
 {555,34,67,3},
 {589,13,66,3},
 {661,10,66,4},
 {671,53,67,4},
 {724,64,65,4},
 {788,8,63,4},
 {796,46,62,4},
 {905,16,62,5},
 {932,18,63,5},
 {967,30,59,5},
 {1129,8,58,6},
 {1150,9,60,6},
 {1180,38,60,6},
 {1234,10,64,6},
 {1327,20,62,7},
 {1371,30,65,7},
 {1424,9,62,7},
 {1565,23,62,8},
 {1589,11,63,8},
 {1616,18,65,8},
 {1645,8,63,8},
 {1698,18,67,9},
 {1727,12,63,9},
 {1754,17,63,9},
 {1793,22,67,9},
 {1838,8,71,9},
 {1869,19,71,9},
 {1898,11,75,9},
 {1909,8,76,9},
 {1970,22,79,10},
 {1993,12,78,10},
 {2017,13,79,10},
 {2134,69,79,11},
 {2216,9,76,11},
 {2238,22,76,11},
 {2260,21,75,11},
 {2413,16,79,12},
 {2429,11,81,12},
 {2455,10,81,12},
 {2465,42,83,12},
 {2507,19,84,12},
 {2541,14,83,12},
 {2628,15,81,13},
 {2653,12,83,13},
 {2680,25,84,13},
 {2717,10,86,13},
 {2858,8,78,14},
 {2867,16,79,14},
 {2883,7,81,14},
 {2933,20,78,15},
 {2962,8,76,15},
 {3023,10,77,16},
 {3033,71,79,16},
 {3129,72,77,16},
 {3228,37,79,16},
 {3265,8,77,16},
 {3282,23,77,16},
 {3336,32,75,17},
 {3368,14,74,17},
 {3391,35,74,17},
 {3426,8,72,17},
 {3460,17,71,17},
 {3609,11,67,18},
 {3637,73,67,18},
 {3733,79,65,18},
 {3839,65,63,18},
 {3946,50,62,19},
 {4012,65,60,19},
 {4090,30,59,19},
 {4148,14,60,19},
 {4182,8,59,19},
 {4210,29,59,19}
};
const unsigned MUSIC_HERE_COUNT = sizeof(music_here_notes)/sizeof(music_here_notes[0]);
/* G-minor/B-flat harmonic underpinning, deliberately minimal at the end. */
const uint8_t music_here_roots[20]={
 43,43,38,38,43,43,46,46,43,43,39,39,43,43,41,41,43,38,46,43
};
