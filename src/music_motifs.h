#pragma once
#include <stdint.h>
/* Cues 1..6 are four-voice GBA arrangements of BORE composition motifs.
   Cue 0 keeps Danny Steel's existing D-major title MIDI intact. */
enum {
 MUSIC_CUE_TITLE=0, MUSIC_CUE_STREETS, MUSIC_CUE_FARCE,
 MUSIC_CUE_ORCHARD, MUSIC_CUE_CLINIC, MUSIC_CUE_ROOFTOP,
 MUSIC_CUE_LEGACY, MUSIC_CUE_HERE_TODAY, MUSIC_CUE_COUNT
};
typedef struct {
 int8_t lead;         /* -1=release, 0=sustain, 1..127=new MIDI note */
 uint8_t bass, comp;  /* 0=rest/hold; otherwise a MIDI note-on */
 uint8_t noise;       /* 0=quiet, 1=hat, 2=side-stick */
 uint8_t volume;      /* lead envelope 1..15 */
 uint8_t duty;        /* pulse duty hardware 0..3 */
} MotifBeat;
void music_motif_step(unsigned cue, unsigned step, MotifBeat *out);
uint32_t music_motif_ticks(unsigned cue);  /* F/64 units, exact integer timing */
const char *music_motif_title(unsigned cue);

/* Timed note-for-note HERE TODAY original 72-second BORE chiptune phrases.
   All time values are 1/60 s ticks; not a short repeating motif. */
typedef struct {
 uint16_t at;
 uint8_t length, pitch, phrase;
} HereNote;
extern const HereNote music_here_notes[];
extern const unsigned MUSIC_HERE_COUNT;
extern const uint8_t music_here_roots[20];
#define MUSIC_HERE_END_FRAMES 4350u /* 72.5 sec, includes the ringing final chord */
