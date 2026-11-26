#pragma once
#include "hd_potheads_set.h"
#include "hd_potheads_actor.h"
#include "hd_potheads_fixed.h"
#include "potheads_cache_format.h"
#define POT_CACHE_DESCENT 180u
#define POT_CACHE_TICKS (POT_CACHE_DESCENT+POT_TRACE_LAST_TICK+1u)
/* The original renderer is the only source of truth, and remains the
   fallback for uncached builds or non-EWRAM destinations. */
static void pot_film_live(uint8_t *p,unsigned t){
 PotCamera camera=pot_camera(t);
 if(t>=POT_CACHE_DESCENT)pot_scene_fixed(p);
 else pot_scene(p,&camera,t);
 pot_dancer(p,&camera,t);
 hd_rect(p,0,135,HD_W,25,HD_INK);
}
#if defined(DANNY_OPENING_CACHE) && defined(__arm__) && !defined(HOST_TEST)
extern const uint32_t pot_film_cache_words[];
/* Random access at every original logical tick, including dialogue cue
   jumps. Copy only the completed picture; captions are still drawn live.
   No additional framebuffer, and DMA3 remains on the render thread. */
static HD_CANVAS_CODE __attribute__((noinline)) int pot_film_cached(uint8_t *p,unsigned t){
 uintptr_t dest=(uintptr_t)p;
 if((dest&3u)||dest<0x02000000u||dest>0x02040000u-HD_BYTES)return 0;
 if(t>=POT_CACHE_TICKS)t=POT_CACHE_TICKS-1u;
 const uint32_t *record=pot_film_cache_words+pot_film_cache_words[t];
 if(record[0]==POT_CACHE_RAW){
  volatile uint32_t *dma=(volatile uint32_t *)0x040000D4u;
  while(dma[2]&0x80000000u){}
  dma[2]=0;
  dma[0]=(uint32_t)(uintptr_t)(record+1);
  dma[1]=(uint32_t)dest;
  __asm__ volatile("" ::: "memory");
  dma[2]=0x84000000u|POT_CACHE_WORDS;
  while(dma[2]&0x80000000u){}
  __asm__ volatile("" ::: "memory");
 }else{
  pot_scene_fixed(p);
  pot_cache_apply((PotCacheWord *)(void *)p,record);
 }
 hd_rect(p,0,135,HD_W,25,HD_INK);
 return 1;
}
#endif
