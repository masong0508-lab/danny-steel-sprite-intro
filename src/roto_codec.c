#include "roto_codec.h"
#if defined(__arm__)
#define ROTO_CODE __attribute__((target("arm"),noinline,long_call))
#else
#define ROTO_CODE
#endif
typedef uint32_t RotoWord __attribute__((may_alias));
/* Forward copy is required for overlapping LZ matches. Word copying is
   safe only with equal alignment and a distance of at least four. */
static inline void forward(uint8_t *d,const uint8_t *s,unsigned n){
 if(d==s)return;
 if(((uintptr_t)d^(uintptr_t)s)&3u){while(n--)*d++=*s++;return;}
 while(n&&((uintptr_t)d&3u)){*d++=*s++;n--;}
 while(n>=4u){*(RotoWord *)(void *)d=*(const RotoWord *)(const void *)s;d+=4;s+=4;n-=4u;}
 while(n--)*d++=*s++;
}
static inline void repeat(uint8_t *d,unsigned n,uint8_t v){
 while(n&&((uintptr_t)d&3u)){*d++=v;n--;}
 uint32_t four=(uint32_t)v*0x01010101u;
 while(n>=4u){*(RotoWord *)(void *)d=four;d+=4;n-=4u;}
 while(n--)*d++=v;
}
static int length_extra(const uint8_t **cursor,const uint8_t *end,unsigned *length){
 unsigned v;do{if(*cursor==end)return 0;v=*(*cursor)++;*length+=v;}while(v==255u);
 return 1;
}
ROTO_CODE int roto_lz4_decode(const uint8_t *src,unsigned src_size,uint8_t *dst,unsigned dst_size){
 const uint8_t *end=src+src_size;unsigned at=0;
 if(!dst_size)return src_size==0u;
 while(src<end){
  unsigned token=*src++,n=token>>4;
  if(n==15u&&!length_extra(&src,end,&n))return 0;
  if(n>(unsigned)(end-src)||n>dst_size-at)return 0;
  forward(dst+at,src,n);src+=n;at+=n;
  if(src==end)return at==dst_size;
  if(end-src<2)return 0;
  unsigned distance=(unsigned)src[0]|((unsigned)src[1]<<8);src+=2;
  n=token&15u;if(n==15u&&!length_extra(&src,end,&n))return 0;n+=4u;
  if(!distance||distance>at||n>dst_size-at)return 0;
  if(distance==1u)repeat(dst+at,n,dst[at-1u]);
  else if(distance<4u){for(unsigned i=0;i<n;i++)dst[at+i]=dst[at+i-distance];}
  else forward(dst+at,dst+at-distance,n);
  at+=n;
 }
 return 0;
}
static int varint(const uint8_t **s,const uint8_t *end,unsigned *out){
 unsigned n=0;for(unsigned shift=0;shift<=14u;shift+=7u){
  if(*s==end)return 0;
  unsigned v=*(*s)++;n|=(v&127u)<<shift;
  if(!(v&128u)){*out=n;return 1;}
 }
 return 0;
}
ROTO_CODE int roto_apply_patch(uint8_t *dst,const uint8_t *src,unsigned size){
 const uint8_t *end=src+size;unsigned at=0;
 while(src<end){
  unsigned skip,n;
  if(!varint(&src,end,&skip)||!varint(&src,end,&n)||!n||skip>ROTO_PIXELS-at)return 0;
  at+=skip;if(n>ROTO_PIXELS-at||n>(unsigned)(end-src))return 0;
  forward(dst+at,src,n);src+=n;at+=n;
 }
 return 1;
}
