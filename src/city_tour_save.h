#pragma once
#include "story.h"
/* Independent companion save; never alter the base slot or layout seed. */
#define CITY_TOUR_OFFSET 0x300u
#define CITY_TOUR_LEN 6u
static inline uint8_t city_tour_crc(const uint8_t *p){
 uint8_t v=0x73u;
 for(unsigned i=0;i<CITY_TOUR_LEN-1u;i++){
  v^=p[i];for(unsigned k=0;k<8u;k++)v=(uint8_t)((v&128u)?((unsigned)v<<1)^7u:(unsigned)v<<1);
 }
 return v;
}
static inline void city_tour_encode(const Story *s,uint8_t *p){
 p[0]='C';p[1]='T';p[2]=1u;p[3]=s->slot;p[4]=(uint8_t)story_city_tour_stage(s->slot);p[5]=city_tour_crc(p);
}
static inline int city_tour_decode(const Story *s,const uint8_t *p){
 if(p[0]!='C'||p[1]!='T'||p[2]!=1u||p[3]!=s->slot||p[4]>STORY_CITY_TOUR_DONE||p[5]!=city_tour_crc(p))return 0;
 story_city_tour_restore(s->slot,p[4]);return 1;
}
