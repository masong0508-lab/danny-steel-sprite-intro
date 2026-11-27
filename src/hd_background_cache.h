#pragma once
#include "hd_canvas.h"
#include "hd_render_copy.h"
/* One immutable-environment snapshot, captured before any actor, weather,
   HUD or menu. Only the current exact wall-cache camera can reuse it.
   No texture/video memory allocation or second VRAM page is required. */
typedef struct {
 uint32_t pixels[HD_BYTES/4];
 unsigned valid,sky_key,checked,wet;
} HdBackgroundCache;
static HdBackgroundCache background __attribute__((section(".ewram"),aligned(4)));
static inline void hd_background_copy(void *dst,const void *src){
 hd_render_copy(dst,src,HD_BYTES);
}
static inline unsigned hd_background_sky_key(unsigned tick){
 return ((tick/180u)%5u)*40u+(tick/42u)%40u;
}
