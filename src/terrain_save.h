#pragma once
#include "story.h"
/* Disjoint from base, rescue, studio, neighborhood and interior records. */
#define TERRAIN_EXT_OFFSET 0x1c0u
#define TERRAIN_EXT_LEN 6u
static inline uint8_t terrain_save_crc(const uint8_t *p){
 uint8_t v=0xc7u;
 for(unsigned i=0;i<TERRAIN_EXT_LEN-1u;i++){
  v^=p[i];
  for(unsigned k=0;k<8u;k++)v=(uint8_t)((v&128u)?((unsigned)v<<1)^7u:(unsigned)v<<1);
 }
 return v;
}
static inline void terrain_save_encode(const Story *s,uint8_t *p){
 p[0]='L';p[1]='D';p[2]=1u;p[3]=s->slot;p[4]=s->terrain_style;p[5]=terrain_save_crc(p);
}
static inline int terrain_save_decode(Story *s,const uint8_t *p){
 if(p[0]!='L'||p[1]!='D'||p[2]!=1u||p[3]!=s->slot||p[4]>15u||p[5]!=terrain_save_crc(p))return 0;
 s->terrain_style=p[4];return 1;
}
