#pragma once
#include <stdint.h>
#if defined(DANNY_OPENING_CACHE) && defined(__arm__) && !defined(HOST_TEST)
typedef void (*RotoBase)(uint8_t *p);
typedef void (*RotoPaint)(uint8_t *p,unsigned tick);
/* One shared workspace for any authored film, not per scene. Each clip
   supplies its own immutable data, background and deterministic painter. */
int roto_cache_render(uint8_t *out,const uint32_t *clip,unsigned tick,RotoBase base,RotoPaint live);
#endif
