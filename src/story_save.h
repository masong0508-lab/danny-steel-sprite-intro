#pragma once
#include <stdint.h>
#include "story.h"

/* Original 32-byte SRAM slots still fit both saves. Version 5 uses 31
   bytes: 30 data bytes plus a one-byte checksum; byte 31 stays unused. */
#define STORY_SAVE_VER 5u
#define STORY_SAVE_BYTES 31u
#define STORY_SAVE_STRIDE 32u

/* Decode and migrate 1/2/3/4/5 without ever reading old checksums as data.
   Invalid signature, version or checksum leave the caller's buffer unchanged. */
int story_save_decode(const uint8_t *raw,uint8_t *normalized);
void story_save_encode(const Story *s,uint8_t *raw);
