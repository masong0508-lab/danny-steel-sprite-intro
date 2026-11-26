#pragma once
#include <stdint.h>
/* One record covers the 240x135 picture. The caption is rendered live.
   Raw records start with UINT32_MAX; patches start with their run count.
   Run headers hold a word offset (low 16) and positive count (high 16). */
#define POT_CACHE_WORDS (240u*135u/4u)
#define POT_CACHE_RECORD_MAX (POT_CACHE_WORDS+1u)
#define POT_CACHE_RAW UINT32_MAX

typedef uint32_t PotCacheWord __attribute__((may_alias));
static inline void pot_cache_copy(volatile PotCacheWord *dst,const uint32_t *src,unsigned n){
 while(n>=4u){dst[0]=src[0];dst[1]=src[1];dst[2]=src[2];dst[3]=src[3];dst+=4;src+=4;n-=4u;}
 while(n--)*dst++=*src++;
}
static inline void pot_cache_apply(PotCacheWord *dst,const uint32_t *record){
 unsigned runs=*record++;
 if(runs==POT_CACHE_RAW){pot_cache_copy(dst,record,POT_CACHE_WORDS);return;}
 while(runs--){
  unsigned span=*record++,start=span&65535u,n=span>>16;
  pot_cache_copy(dst+start,record,n);record+=n;
 }
}
/* Build independent random-access records. Gaps of up to two unchanged
   words may be included to avoid another run header. Never exceed raw. */
static inline unsigned pot_cache_encode(const uint32_t *base,const uint32_t *frame,uint32_t *out,int force_raw){
 unsigned at=1u,runs=0u,i=0u;
 if(!force_raw){
  while(i<POT_CACHE_WORDS){
   if(base[i]==frame[i]){i++;continue;}
   unsigned start=i,end=++i;
   while(i<POT_CACHE_WORDS&&i-end<3u){
    if(base[i]!=frame[i])end=i+1u;
    i++;
   }
   unsigned n=end-start;
   if(at+1u+n>=POT_CACHE_RECORD_MAX)goto raw;
   out[at++]=start|(n<<16);
   for(unsigned j=start;j<end;j++)out[at++]=frame[j];
   runs++;
  }
  out[0]=runs;return at;
 }
raw:
 out[0]=POT_CACHE_RAW;
 for(unsigned j=0;j<POT_CACHE_WORDS;j++)out[j+1u]=frame[j];
 return POT_CACHE_RECORD_MAX;
}
