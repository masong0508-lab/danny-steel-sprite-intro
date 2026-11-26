#pragma once
#include <stdint.h>
#if (defined(DANNY_OPENING_CACHE) && defined(__arm__) && !defined(HOST_TEST)) || defined(DANNY_ROTO_HOST_TEST)
typedef void (*RotoBase)(uint8_t *p);
typedef void (*RotoPaint)(uint8_t *p,unsigned tick);
void roto_cache_begin_frame(int preserve_picture);
int roto_cache_render(uint8_t *out,const uint32_t *clip,unsigned tick,RotoBase base,RotoPaint live);
#else
static inline void roto_cache_begin_frame(int preserve_picture){(void)preserve_picture;}
#endif
