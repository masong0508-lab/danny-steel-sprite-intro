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

/* Randomized, non-repeating New Game bag. The previous one-byte cursor is
   still written for migration and recovery. No user-facing seed selector. */
#define WORLD_POOL_OFFSET 0x2c0u
#define WORLD_POOL_BYTES ((STORY_WORLD_LAYOUTS+7u)/8u)
#define WORLD_POOL_LEN (WORLD_POOL_BYTES+9u)
typedef struct {uint8_t used[WORLD_POOL_BYTES];uint8_t last;uint32_t rng;} WorldPool;
static inline uint32_t world_mix(uint32_t v){
 v^=v>>16;v*=0x7feb352du;v^=v>>15;v*=0x846ca68bu;return v^(v>>16);
}
static inline void world_pool_init(WorldPool *p,uint32_t entropy,unsigned last){
 for(unsigned i=0;i<WORLD_POOL_BYTES;i++)p->used[i]=0u;
 p->last=(uint8_t)(last<STORY_WORLD_LAYOUTS?last:255u);
 p->rng=world_mix(entropy^0x9e3779b9u);
 if(last<STORY_WORLD_LAYOUTS)p->used[last>>3]|=(uint8_t)(1u<<(last&7u));
}
static inline int world_pool_used(const WorldPool *p,unsigned seed){
 return !!(p->used[seed>>3]&(1u<<(seed&7u)));
}
static inline unsigned world_pool_pick(WorldPool *p,uint32_t entropy){
 p->rng=world_mix(p->rng+entropy+0x9e3779b9u);
 unsigned free_count=0;
 for(unsigned i=0;i<STORY_WORLD_LAYOUTS;i++)if(!world_pool_used(p,i)&&i!=p->last)free_count++;
 if(!free_count){
  for(unsigned i=0;i<WORLD_POOL_BYTES;i++)p->used[i]=0u;
  free_count=STORY_WORLD_LAYOUTS-(p->last<STORY_WORLD_LAYOUTS);
 }
 unsigned rank=p->rng%free_count;
 for(unsigned i=0;i<STORY_WORLD_LAYOUTS;i++)if(!world_pool_used(p,i)&&i!=p->last){
  if(!rank){
   p->used[i>>3]|=(uint8_t)(1u<<(i&7u));p->last=(uint8_t)i;return i;
  }
  --rank;
 }
 return 0; /* unreachable for a valid fixed-size bag */
}
static inline uint8_t world_pool_crc(const uint8_t *p){
 uint8_t v=0x63u;
 for(unsigned i=0;i<WORLD_POOL_LEN-1u;i++){
  v^=p[i];for(unsigned k=0;k<8u;k++)v=(uint8_t)((v&128u)?((unsigned)v<<1)^7u:(unsigned)v<<1);
 }
 return v;
}
static inline void world_pool_encode(const WorldPool *p,uint8_t *raw){
 raw[0]='W';raw[1]='P';raw[2]=1u;raw[3]=p->last;
 for(unsigned i=0;i<4u;i++)raw[4u+i]=(uint8_t)(p->rng>>(i*8u));
 for(unsigned i=0;i<WORLD_POOL_BYTES;i++)raw[8u+i]=p->used[i];
 raw[WORLD_POOL_LEN-1u]=world_pool_crc(raw);
}
static inline int world_pool_decode(WorldPool *p,const uint8_t *raw){
 if(raw[0]!='W'||raw[1]!='P'||raw[2]!=1u||raw[3]>=STORY_WORLD_LAYOUTS||
    raw[WORLD_POOL_LEN-1u]!=world_pool_crc(raw)||
    !(raw[8u+(raw[3]>>3)]&(1u<<(raw[3]&7u))))return 0;
 p->last=raw[3];p->rng=0u;
 for(unsigned i=0;i<4u;i++)p->rng|=(uint32_t)raw[4u+i]<<(i*8u);
 for(unsigned i=0;i<WORLD_POOL_BYTES;i++)p->used[i]=raw[8u+i];
 return 1;
}
