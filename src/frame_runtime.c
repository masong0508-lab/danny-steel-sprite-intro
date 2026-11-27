#include "frame_runtime.h"
#include "gba.h"
#include "music.h"

volatile uint32_t frame_runtime_video_frames;
volatile uint32_t frame_runtime_read;
volatile uint32_t frame_runtime_write;
volatile uint32_t frame_runtime_dropped;
static volatile uint16_t samples[FRAME_RUNTIME_QUEUE_CAPACITY];
static volatile unsigned active,displayed,pending_page=2u;
static uint32_t render_started;
volatile unsigned frame_runtime_dim;
static volatile unsigned pending_transition;

static unsigned lock_irq(void){
#if defined(__arm__)
 unsigned previous=REG_IME;
 REG_IME=0;
 __asm__ volatile("" ::: "memory");
 return previous;
#else
 return 0u;
#endif
}
static void unlock_irq(unsigned previous){
#if defined(__arm__)
 __asm__ volatile("" ::: "memory");
 REG_IME=(u16)previous;
#else
 (void)previous;
#endif
}

void frame_runtime_capture(uint16_t keys){
 ++frame_runtime_video_frames;
 if(!active)return;
 /* With no main-thread consumption for over one second, discard the
    oldest sample, retain the newest chronology and report the overrun.
    The ROM regression probe requires this diagnostic to remain zero. */
 uint32_t write=frame_runtime_write;
 if(write-frame_runtime_read>=FRAME_RUNTIME_QUEUE_CAPACITY){
  ++frame_runtime_read;
  ++frame_runtime_dropped;
 }
 samples[write&(FRAME_RUNTIME_QUEUE_CAPACITY-1u)]=(uint16_t)(keys&0x03ffu);
 frame_runtime_write=write+1u;
 /* Publication is the only display work in IRQ. The painter and DMA
    must both finish before submit makes a page eligible for this flip. */
#if defined(__arm__)
 /* Neither a page flip nor a brightness change belongs in scanout. */
 if(REG_VCOUNT<160u)return;
#endif
 unsigned old_dim=frame_runtime_dim;
 if(frame_runtime_dim) --frame_runtime_dim;
 if(pending_page<2u){
  displayed=pending_page;
#if defined(__arm__)
  REG_DISPCNT=(u16)(4u|DCNT_BG2|(displayed?0x10u:0u));
#endif
  if(pending_transition==FRAME_RUNTIME_SOFTEN)frame_runtime_dim=FRAME_RUNTIME_DIM_LEVEL;
  else if(pending_transition==FRAME_RUNTIME_CLEAR)frame_runtime_dim=0u;
  pending_page=2u;pending_transition=FRAME_RUNTIME_KEEP;
 }
#if defined(__arm__)
 if(old_dim!=frame_runtime_dim){
  REG_BLDY=(u16)frame_runtime_dim;
  REG_BLDCNT=frame_runtime_dim?0x00C4u:0u; /* BG2 brightness decrease */
 }
#else
 (void)old_dim;
#endif
}

#if defined(__arm__)
/* BIOS already saves r0-r3/r12/lr and restores the interrupted context.
   Its vector must point to a normal ARM function, NOT an interrupt("IRQ")
   return sequence. This handler does not nest or run game/audio logic. */
static void frame_runtime_irq(void)
 __attribute__((section(".iwram"),target("arm")));
static void frame_runtime_irq(void){
 unsigned raised=(unsigned)(REG_IE&REG_IF)&1u;
 if(!raised)return;
 REG_IF=(u16)raised;
 *(volatile u16*)0x03007ff8|=(u16)raised; /* BIOS IRQ flags; do not touch reset flag. */
 music_clock_vblank();
 frame_runtime_capture((uint16_t)(~REG_KEYINPUT&0x03ffu));
}
#endif

void frame_runtime_init(void){
 (void)lock_irq();
 active=0u;displayed=0u;pending_page=2u;render_started=0u;
 frame_runtime_dim=0u;pending_transition=FRAME_RUNTIME_KEEP;
 frame_runtime_video_frames=0u;
 frame_runtime_read=frame_runtime_write=frame_runtime_dropped=0u;
#if defined(__arm__)
 typedef void (*RuntimeHandler)(void);
 *(RuntimeHandler volatile*)0x03007ffc=frame_runtime_irq;
 REG_IE=1u;REG_IF=1u;
 REG_DISPSTAT|=8u;
 REG_IME=1u;
#endif
}
void frame_runtime_start(unsigned displayed_page){
 unsigned previous=lock_irq();
 displayed=displayed_page&1u;pending_page=2u;
 frame_runtime_dim=0u;pending_transition=FRAME_RUNTIME_KEEP;
#if defined(__arm__)
 REG_BLDY=0u;REG_BLDCNT=0u;
#endif
 frame_runtime_read=frame_runtime_write=frame_runtime_dropped=0u;
 render_started=frame_runtime_video_frames;
 active=1u;
 unlock_irq(previous);
}
void frame_runtime_stop(void){
 unsigned previous=lock_irq();
 active=0u;pending_page=2u;
 frame_runtime_dim=0u;pending_transition=FRAME_RUNTIME_KEEP;
#if defined(__arm__)
 REG_BLDY=0u;REG_BLDCNT=0u;
#endif
 frame_runtime_read=frame_runtime_write;
 unlock_irq(previous);
}
int frame_runtime_next(uint16_t *keys){
 unsigned previous=lock_irq();
 uint32_t read=frame_runtime_read;
 int available=active&&read!=frame_runtime_write;
 if(available){
  *keys=samples[read&(FRAME_RUNTIME_QUEUE_CAPACITY-1u)];
  frame_runtime_read=read+1u;
 }
 unlock_irq(previous);
 return available;
}
int frame_runtime_begin_render(unsigned *page){
 unsigned previous=lock_irq();
 uint32_t now=frame_runtime_video_frames;
 int ready=active&&pending_page==2u&&
           frame_runtime_read==frame_runtime_write&&now-render_started>=2u;
 if(ready){*page=displayed^1u;render_started=now;}
 unlock_irq(previous);
 return ready;
}
void frame_runtime_submit_transition(unsigned page,unsigned transition){
 unsigned previous=lock_irq();
 if(active&&pending_page==2u&&(page&1u)!=displayed){
  pending_transition=transition<=FRAME_RUNTIME_CLEAR?transition:FRAME_RUNTIME_KEEP;
  pending_page=page&1u;
 }
 unlock_irq(previous);
}
void frame_runtime_submit(unsigned page){
 frame_runtime_submit_transition(page,FRAME_RUNTIME_KEEP);
}
