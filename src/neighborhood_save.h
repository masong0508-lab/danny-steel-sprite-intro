#pragma once
#include "story.h"
/* 0x180..0x193 is disjoint from base, rescue and studio save slots. */
#define NEIGHBORHOOD_EXT_OFFSET 0x180u
#define NEIGHBORHOOD_EXT_LEN 10u
static inline uint8_t neighborhood_crc(const uint8_t *p){
 uint8_t v=0xA7u;
 for(unsigned i=0;i<NEIGHBORHOOD_EXT_LEN-1u;i++){
  v^=p[i];
  for(unsigned k=0;k<8;k++)
   v=(uint8_t)((v&0x80u)?((unsigned)v<<1)^0x07u:(unsigned)v<<1);
 }
 return v;
}
static inline void neighborhood_save_encode(const Story *s,uint8_t *p){
 unsigned seen=s->neighborhood_seen&STORY_NEIGHBORHOOD_MASK,right=s->neighborhood_right&seen;
 p[0]='N';p[1]='Q';p[2]=1u;p[3]=s->slot;
 p[4]=(uint8_t)seen;p[5]=(uint8_t)(seen>>8);p[6]=(uint8_t)right;p[7]=(uint8_t)(right>>8);
 p[8]=0xB6u;p[9]=neighborhood_crc(p);
}
static inline int neighborhood_save_decode(Story *s,const uint8_t *p){
 unsigned seen=(unsigned)p[4]|((unsigned)p[5]<<8),right=(unsigned)p[6]|((unsigned)p[7]<<8);
 if(p[0]!='N'||p[1]!='Q'||p[2]!=1u||p[3]!=s->slot||p[8]!=0xB6u||
    p[9]!=neighborhood_crc(p)||(seen&~STORY_NEIGHBORHOOD_MASK)||(right&~seen))return 0;
 s->neighborhood_seen=(uint16_t)seen;s->neighborhood_right=(uint16_t)right;return 1;
}
