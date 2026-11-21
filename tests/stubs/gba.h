#pragma once
#include <stdint.h>
typedef uint8_t u8;typedef uint16_t u16;typedef uint32_t u32;typedef int32_t s32;
extern volatile uint16_t test_sound_regs[20];
extern volatile uint16_t test_wave_regs[8];
extern volatile uint16_t test_timer;
#define REG_SND1SWEEP test_sound_regs[0]
#define REG_SND1CNT test_sound_regs[1]
#define REG_SND1FREQ test_sound_regs[2]
#define REG_SND2CNT test_sound_regs[3]
#define REG_SND2FREQ test_sound_regs[4]
#define REG_SND3SEL test_sound_regs[5]
#define REG_SND3CNT test_sound_regs[6]
#define REG_SND3FREQ test_sound_regs[7]
#define REG_SND4CNT test_sound_regs[8]
#define REG_SND4FREQ test_sound_regs[9]
#define REG_SNDCTRL_L test_sound_regs[10]
#define REG_SNDCTRL_H test_sound_regs[11]
#define REG_SNDSTAT test_sound_regs[12]
#define REG_TM3CNT_H test_sound_regs[13]
#define REG_TM3CNT_L test_timer
#define WAVE_RAM test_wave_regs
