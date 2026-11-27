#pragma once
#include "hd_canvas.h"
#include <string.h>
/* One immutable-environment snapshot, captured before any actor, weather,
   HUD or menu. Only the current exact wall-cache camera can reuse it.
   No texture/video memory allocation or second VRAM page is required. */
typedef struct {
 uint32_t pixels[HD_BYTES/4];
 unsigned valid,sky_key,checked,wet;
} HdBackgroundCache;
static HdBackgroundCache background __attribute__((section(".ewram"),aligned(4)));
static void hd_background_copy(void *dst,const void *src){
#if defined(__arm__) && !defined(HOST_TEST)
 if((((uintptr_t)dst|(uintptr_t)src)&3u)==0u){
  /* DMA3 is synchronous and exclusively used by main-thread rendering
     and later hidden-page publication. Audio and IRQ use no DMA. */
  volatile uint32_t *dma=(volatile uint32_t *)0x040000D4u;
  dma[2]=0;
  dma[0]=(uint32_t)(uintptr_t)src;
  dma[1]=(uint32_t)(uintptr_t)dst;
  dma[2]=0x84000000u|HD_BYTES/4u;
  while(dma[2]&0x80000000u){}
  return;
 }
#endif
 memcpy(dst,src,HD_BYTES);
}
static inline unsigned hd_background_sky_key(unsigned tick){
 return ((tick/180u)%5u)*40u+(tick/42u)%40u;
}
