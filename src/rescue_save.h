#pragma once
#include <stdint.h>
/* Rescue companion data lives at SRAM offset 0x100 + slot*8.
   The existing 31-byte v5 save and 32-byte slot stride remain unchanged. */
#define RESCUE_EXT_OFFSET 0x100u
#define RESCUE_EXT_LEN 8u
void rescue_save_encode(unsigned slot,unsigned mask,unsigned stage,uint8_t out[8]);
int rescue_save_decode(unsigned slot,const uint8_t in[8],uint8_t *mask,uint8_t *stage);
