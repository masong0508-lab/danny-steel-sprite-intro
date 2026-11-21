#pragma once
#include "gba.h"
typedef struct {u32 t;u16 val;u8 ch;u8 arg;} MusEv;
extern const int MUSIC_COUNT,MUSIC_LOOP_IDX;
extern const u32 MUSIC_LOOP_UNITS;
extern const MusEv music_events[];
