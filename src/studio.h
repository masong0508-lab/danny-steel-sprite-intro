#pragma once
#include <stdint.h>
#include "music_motifs.h"

/* A tiny, independent editor, shared by gameplay, GBA audio and host tests.
   This is a composed PSG arrangement, not a PCM/vocal recorder. */
#define STUDIO_STEPS 8u
#define STUDIO_SECTIONS 4u
#define STUDIO_TAB_COUNT 6u
#define STUDIO_EXT_OFFSET 0x140u
#define STUDIO_EXT_LEN 24u
enum { STUDIO_SONG, STUDIO_NOTES, STUDIO_ORDER, STUDIO_LYRICS,
       STUDIO_CHOIR, STUDIO_PERFORM };
typedef struct {
 uint8_t notes[STUDIO_STEPS];      /* 0 rest; 1..7 notes in C major */
 uint8_t order[STUDIO_SECTIONS];  /* 0..3 phrase variants */
 uint8_t tempo;                    /* 0..3, slow through brisk */
 uint8_t mood;                     /* 0..2, soft/warm/bright */
 uint8_t lyric;                    /* 0..3, displayed lyric cue */
 uint8_t choir;                    /* 0..3, PSG harmony density */
} StudioPatch;

void studio_reset(void);
const StudioPatch *studio_patch(void);
const char *studio_lyric(unsigned index);
const char *studio_note_label(unsigned pitch);
void studio_change(unsigned tab,unsigned cursor,int delta);
void studio_toggle(unsigned tab,unsigned cursor);
unsigned studio_cursor_count(unsigned tab);
void studio_motif_step(unsigned step,MotifBeat *out);
uint32_t studio_ticks(void);
unsigned studio_playhead(void);
unsigned studio_playpart(void);
void studio_save_encode(unsigned slot,uint8_t out[STUDIO_EXT_LEN]);
int studio_save_decode(unsigned slot,const uint8_t in[STUDIO_EXT_LEN]);
