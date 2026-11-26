#pragma once
#include "music_motifs.h"
/* Original D-major title MIDI + BORE-aware GBA PSG soundtrack. */
void music_init(void);
void music_start(void);
void music_stop(void);
void music_poll(void);
#if defined(__arm__)
#define MUSIC_CLOCK_CODE __attribute__((section(".iwram"),long_call,target("arm")))
#else
#define MUSIC_CLOCK_CODE
#endif
/* IRQ samples only the free-running clock. PSG sequencing remains on main. */
MUSIC_CLOCK_CODE void music_clock_vblank(void);
void music_set_volume(int pct);
void music_set_cue(unsigned cue);
unsigned music_get_cue(void);
/* Studio and genuinely silent transport; original soundtrack IDs unchanged. */
#define MUSIC_CUE_STUDIO MUSIC_CUE_COUNT
#define MUSIC_CUE_STUDIO_IDLE (MUSIC_CUE_COUNT+1u)

/* Original 72.5-second HERE TODAY score, played one time through the ending.
   Good ending adds a distinct BORE legacy coda; other endings end quietly.
   Calls are idempotent so changing cutscene shots cannot restart the song. */
void music_begin_ending(unsigned ending); /* 1 good, 2 neutral, 3 bad */
unsigned music_ending_phase(void);       /* 0 no ending, 1 HERE TODAY, 2 good coda, 3 completed */

