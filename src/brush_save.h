#pragma once
#include "story.h"
/* Two independent slot records, disjoint from previous 0x1c0 terrain data. */
#define BRUSH_EXT_OFFSET 0x200u
#define BRUSH_EXT_LEN (STORY_BRUSH_BYTES+6u)
static inline uint8_t brush_save_crc(const uint8_t *p){
 uint8_t v=0x3du;
 for(unsigned i=0;i<BRUSH_EXT_LEN-1u;i++){
  v^=p[i];
  for(unsigned k=0;k<8u;k++)v=(uint8_t)((v&128u)?((unsigned)v<<1)^7u:(unsigned)v<<1);
 }
 return v;
}
static inline void brush_save_encode(const Story *s,uint8_t *p){
 p[0]='B';p[1]='R';p[2]=1u;p[3]=s->slot;
 story_brush_export(s->slot,p+4);
 p[BRUSH_EXT_LEN-2u]=(uint8_t)story_hike_stage(s->slot);
 p[BRUSH_EXT_LEN-1u]=brush_save_crc(p);
}
static inline int brush_save_decode(const Story *s,const uint8_t *p){
 if(p[0]!='B'||p[1]!='R'||p[2]!=1u||p[3]!=s->slot||
    p[BRUSH_EXT_LEN-2u]>4u||p[BRUSH_EXT_LEN-1u]!=brush_save_crc(p))return 0;
 if(!story_brush_import(s->slot,p+4))return 0;
 story_hike_restore(s->slot,p[BRUSH_EXT_LEN-2u]);return 1;
}
