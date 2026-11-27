#pragma once
#include <stdint.h>
#include <string.h>
/* Shared synchronous main-thread copy. DMA3 is not used by the IRQ
   or PSG music. Never publish partial video pages from this helper. */
static inline void hd_render_copy(void *dst,const void *src,unsigned bytes){
#if defined(__arm__) && !defined(HOST_TEST)
 if(bytes&&bytes<=0x3fffcu&&(((uintptr_t)dst|(uintptr_t)src|bytes)&3u)==0u){
  volatile uint32_t *dma=(volatile uint32_t *)0x040000D4u;
  dma[2]=0;
  dma[0]=(uint32_t)(uintptr_t)src;
  dma[1]=(uint32_t)(uintptr_t)dst;
  dma[2]=0x84000000u|(bytes>>2);
  while(dma[2]&0x80000000u){}
  return;
 }
#endif
 memcpy(dst,src,bytes);
}
