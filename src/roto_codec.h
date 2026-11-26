#pragma once
#include <stdint.h>
#define ROTO_PIXELS 32400u
#define ROTO_WORDS (ROTO_PIXELS/4u)
#define ROTO_PACKET_MAX 32768u
#define ROTO_SEEK_MAX 64u
#define ROTO_SEEK_WINDOW 2u
#define ROTO_CLIP_MAGIC 0x33544f52u
#define ROTO_CLIP_LIMIT 1258291u
#if defined(__arm__)
#define ROTO_PATCH_CODE __attribute__((section(".iwram"),target("arm"),long_call,noinline))
#else
#define ROTO_PATCH_CODE
#endif
int roto_lz4_decode(const uint8_t *src,unsigned src_size,uint8_t *dst,unsigned dst_size);
ROTO_PATCH_CODE int roto_apply_words(uint8_t *dst,const uint8_t *src,unsigned size);
