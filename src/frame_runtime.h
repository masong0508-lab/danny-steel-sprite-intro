#pragma once
#include <stdint.h>

/* The IRQ records video-time input while a synchronous painter is busy.
   Sixty-four samples cost 128 bytes, not another pixel page. */
#define FRAME_RUNTIME_QUEUE_CAPACITY 64u
extern volatile uint32_t frame_runtime_video_frames;
extern volatile uint32_t frame_runtime_read;
extern volatile uint32_t frame_runtime_write;
extern volatile uint32_t frame_runtime_dropped;
/* Ease brightness only when a completed new scene is presented. */
#define FRAME_RUNTIME_DIM_LEVEL 6u
#define FRAME_RUNTIME_KEEP 0u
#define FRAME_RUNTIME_SOFTEN 1u
#define FRAME_RUNTIME_CLEAR 2u
extern volatile unsigned frame_runtime_dim;

void frame_runtime_init(void);
void frame_runtime_start(unsigned displayed_page);
void frame_runtime_stop(void);
int frame_runtime_next(uint16_t *keys);
/* Claim the hidden page only after old input is consumed, no earlier
   publication is pending, and two real video frames have elapsed. */
int frame_runtime_begin_render(unsigned *page);
/* Call only after the complete pixel page has reached hidden VRAM. */
void frame_runtime_submit(unsigned page);
void frame_runtime_submit_transition(unsigned page,unsigned transition);

#if defined(__arm__)
#define FRAME_RUNTIME_FAST __attribute__((section(".iwram"),long_call,target("arm")))
#else
#define FRAME_RUNTIME_FAST
#endif
/* Shared, bounded IRQ body. Host tests feed the same chronological samples. */
FRAME_RUNTIME_FAST void frame_runtime_capture(uint16_t keys);
