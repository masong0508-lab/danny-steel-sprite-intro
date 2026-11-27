#pragma once
#include "story.h"
/* Two six-byte copies per slot. Independent cursor survives slot erasure.
   No base-save schema or existing companion offset changes. */
#define WORLD_EXT_OFFSET 0x280u
#define WORLD_EXT_LEN 6u
#define WORLD_EXT_COPIES 2u
#define WORLD_EXT_STRIDE (WORLD_EXT_LEN*WORLD_EXT_COPIES)
#define WORLD_CURSOR_OFFSET 0x2a0u
static inline uint8_t world_save_crc(const uint8_t *p){
 uint8_t v=0x9bu;
 for(unsigned i=0;i<WORLD_EXT_LEN-1u;i++){
  v^=p[i];
  for(unsigned k=0;k<8u;k++)v=(uint8_t)((v&128u)?((unsigned)v<<1)^7u:(unsigned)v<<1);
 }
 return v;
}
static inline void world_save_encode(unsigned tag,unsigned seed,uint8_t *p){
 p[0]='W';p[1]='S';p[2]=1u;p[3]=(uint8_t)tag;
 p[4]=(uint8_t)(seed%STORY_WORLD_LAYOUTS);p[5]=world_save_crc(p);
}
static inline int world_save_decode(unsigned tag,const uint8_t *p,unsigned *seed){
 if(p[0]!='W'||p[1]!='S'||p[2]!=1u||p[3]!=tag||p[4]>=STORY_WORLD_LAYOUTS||
    p[5]!=world_save_crc(p))return 0;
 *seed=p[4];return 1;
}
/* 37 and 192 are coprime: every new game visits a different layout until
   the full set is exhausted, even if emulator timing is identical. */
static inline unsigned world_seed_next(unsigned seed){return (seed+37u)%STORY_WORLD_LAYOUTS;}
