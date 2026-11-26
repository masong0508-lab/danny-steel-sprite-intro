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
 if(((uintptr_t)d^(uintptr_t)s)&3u){
  while(n>=4u){d[0]=s[0];d[1]=s[1];d[2]=s[2];d[3]=s[3];d+=4;s+=4;n-=4u;}
  while(n--)*d++=*s++;
  return;
 }
 while(n&&((uintptr_t)d&3u)){*d++=*s++;n--;}
 while(n>=16u){
  RotoWord *dw=(RotoWord *)(void *)d;const RotoWord *sw=(const RotoWord *)(const void *)s;
  dw[0]=sw[0];dw[1]=sw[1];dw[2]=sw[2];dw[3]=sw[3];d+=16;s+=16;n-=16u;
 }
 while(n>=4u){*(RotoWord *)(void *)d=*(const RotoWord *)(const void *)s;d+=4;s+=4;n-=4u;}
 while(n--)*d++=*s++;
}
static inline void repeat(uint8_t *d,unsigned n,uint8_t v){
 while(n&&((uintptr_t)d&3u)){*d++=v;n--;}
 uint32_t four=(uint32_t)v*0x01010101u;
 while(n>=16u){RotoWord *dw=(RotoWord *)(void *)d;dw[0]=four;dw[1]=four;dw[2]=four;dw[3]=four;d+=16;n-=16u;}
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
/* Absolute word spans allow a single aligned copy per run. */
ROTO_PATCH_CODE int roto_apply_words(uint8_t *dst,const uint8_t *src,unsigned size){
 if(((uintptr_t)dst|(uintptr_t)src|size)&3u)return 0;
 RotoWord *out=(RotoWord *)(void *)dst;
 const RotoWord *in=(const RotoWord *)(const void *)src;
 const RotoWord *end=in+size/4u;
 while(in<end){
  unsigned span=*in++,start=span&65535u,n=span>>16;
  if(!n||start>=ROTO_WORDS||n>ROTO_WORDS-start||n>(unsigned)(end-in))return 0;
  RotoWord *d=out+start;
  while(n>=4u){d[0]=in[0];d[1]=in[1];d[2]=in[2];d[3]=in[3];d+=4;in+=4;n-=4u;}
  while(n--)*d++=*in++;
 }
 return 1;
}
