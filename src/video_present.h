#pragma once
#include <stdint.h>

/* Copy the completed shared Mode-4 canvas to the selected hidden VRAM
   page. The immediate transfer has finished before this function returns;
   only then may the presentation scheduler publish that page as ready. */
void video_copy_frame(const uint32_t *src,unsigned page);
