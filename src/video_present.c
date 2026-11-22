#include "video_present.h"
#include "hd_canvas.h"
#include "gba.h"

void video_copy_frame(const uint32_t *src,unsigned page){
 volatile uint32_t *dst=(volatile uint32_t *)(void *)(VRAM8+(page?0xA000u:0u));
#if defined(__arm__) && !defined(HOST_TEST)
 /* Intro and gameplay own DMA3 at disjoint times. PSG music uses no DMA.
    Both addresses and the exact 38,400-byte canvas are word-aligned. */
 volatile uint32_t *dma=(volatile uint32_t *)0x040000D4u;
 dma[2]=0;
 dma[0]=(uint32_t)(uintptr_t)src;
 dma[1]=(uint32_t)(uintptr_t)dst;
 dma[2]=0x84000000u|HD_BYTES/4u; /* immediate, 32-bit, increment both */
 while(dma[2]&0x80000000u){}
#else
 /* Same bounded transfer contract in host verification. */
 for(unsigned i=0;i<HD_BYTES/4u;i++)dst[i]=src[i];
#endif
}
