#pragma once
#include "story.h"
#define INTERIOR_EXT_OFFSET 0x1a0u
#define INTERIOR_EXT_LEN 8u
static inline uint8_t interior_save_crc(const uint8_t *p){
 uint8_t v=0x96u;
 for(unsigned i=0;i<INTERIOR_EXT_LEN-1u;i++){
  v^=p[i];
  for(unsigned k=0;k<8u;k++)
   v=(uint8_t)((v&128u)?((unsigned)v<<1)^7u:(unsigned)v<<1);
 }
 return v;
}
static inline void interior_save_encode(const Story *s,uint8_t *p){
 unsigned done=s->interior_done&STORY_NEIGHBORHOOD_MASK;
 p[0]='I';p[1]='N';p[2]=1u;p[3]=s->slot;
 p[4]=(uint8_t)done;p[5]=(uint8_t)(done>>8);p[6]=0x67u;
 p[7]=interior_save_crc(p);
}
static inline int interior_save_decode(Story *s,const uint8_t *p){
 unsigned done=(unsigned)p[4]|((unsigned)p[5]<<8);
 if(p[0]!='I'||p[1]!='N'||p[2]!=1u||p[3]!=s->slot||
    p[6]!=0x67u||p[7]!=interior_save_crc(p)||(done&~STORY_NEIGHBORHOOD_MASK))
    return 0;
 s->interior_done=(uint16_t)done;return 1;
}
