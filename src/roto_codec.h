#pragma once
#include <stdint.h>
#define ROTO_PIXELS 32400u
#define ROTO_PACKET_MAX 32768u
#define ROTO_KEY_INTERVAL 32u
#define ROTO_CLIP_MAGIC 0x31544f52u
#define ROTO_CLIP_LIMIT 1258291u
int roto_lz4_decode(const uint8_t *src,unsigned src_size,uint8_t *dst,unsigned dst_size);
int roto_apply_patch(uint8_t *dst,const uint8_t *src,unsigned size);
