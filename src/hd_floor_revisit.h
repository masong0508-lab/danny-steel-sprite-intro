#pragma once
#include "hd_canvas.h"
#include "hd_render_copy.h"
/* The original floor/wall split covers every pixel in rows 80..139.
   Nothing animated except possible water is present before billboards
   and rain. Store these completed rows, not an extra full-frame page. */
enum { HD_FLOOR_TOP=80,HD_FLOOR_BYTES=60*HD_W,HD_FLOOR_SLOTS=4,HD_FLOOR_HISTORY=24 };
typedef struct {int x,y;unsigned facing,valid;} HdFloorKey;
typedef struct {uint32_t pixels[HD_FLOOR_BYTES/4];HdFloorKey key;} HdFloorSnapshot;
typedef struct {
 HdFloorSnapshot entries[HD_FLOOR_SLOTS];
 HdFloorKey history[HD_FLOOR_HISTORY];
 unsigned next,history_next;
} HdFloorRevisit;
static HdFloorRevisit floor_revisit __attribute__((section(".ewram"),aligned(4)));
static inline int hd_floor_key_equal(const HdFloorKey *a,const HdFloorKey *b){
 return a->valid&&b->valid&&a->x==b->x&&a->y==b->y&&a->facing==b->facing;
}
static inline void hd_floor_revisit_reset(void){
 for(unsigned i=0;i<HD_FLOOR_SLOTS;i++)floor_revisit.entries[i].key.valid=0;
 for(unsigned i=0;i<HD_FLOOR_HISTORY;i++)floor_revisit.history[i].valid=0;
 floor_revisit.next=floor_revisit.history_next=0;
}
static inline int hd_floor_revisit_find(const HdFloorKey *key){
 for(unsigned i=0;i<HD_FLOOR_SLOTS;i++)
  if(hd_floor_key_equal(key,&floor_revisit.entries[i].key))return (int)i;
 return -1;
}
/* Do not pay for copying unique views while exploring. Only admit a
   changed camera that has appeared before in the recent-pose history. */
static inline int hd_floor_revisit_admit(const HdFloorKey *key){
 for(unsigned i=0;i<HD_FLOOR_HISTORY;i++)
  if(hd_floor_key_equal(key,&floor_revisit.history[i]))return 1;
 floor_revisit.history[floor_revisit.history_next]=*key;
 floor_revisit.history_next=(floor_revisit.history_next+1u)%HD_FLOOR_HISTORY;
 return 0;
}
static inline void hd_floor_revisit_restore(uint8_t *p,unsigned slot){
 hd_render_copy(p+HD_FLOOR_TOP*HD_W,floor_revisit.entries[slot].pixels,HD_FLOOR_BYTES);
}
static inline void hd_floor_revisit_store(const uint8_t *p,const HdFloorKey *key){
 unsigned slot=floor_revisit.next;
 hd_render_copy(floor_revisit.entries[slot].pixels,p+HD_FLOOR_TOP*HD_W,HD_FLOOR_BYTES);
 floor_revisit.entries[slot].key=*key;
 floor_revisit.next=(slot+1u)%HD_FLOOR_SLOTS;
}
