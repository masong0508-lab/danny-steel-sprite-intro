#include "roto_cache.h"
#include "roto_codec.h"
#include "hd_canvas.h"
#if (defined(DANNY_OPENING_CACHE) && defined(__arm__) && !defined(HOST_TEST)) || defined(DANNY_ROTO_HOST_TEST)
/* A single shared packet; sequential frames never reset the background. */
static uint8_t packet[ROTO_PACKET_MAX] __attribute__((section(".ewram"),aligned(4)));
static const uint32_t *current_clip;
static unsigned current_sample;
static int valid,lease;
void roto_cache_begin_frame(int preserve_picture){lease=preserve_picture;if(!lease)valid=0;}
static int decode_record(uint8_t *out,const uint32_t *clip,unsigned a,unsigned b){
 if((a|b)&3u||b<a||b-a<4u||b>clip[4])return 0;
 const uint32_t *rec=(const uint32_t *)((const uint8_t *)clip+a);
 unsigned n=rec[0]&65535u,z=rec[0]>>16,stored=z?z:n;
 if((n&3u)||n>ROTO_PACKET_MAX||stored>b-a-4u||(!n&&z))return 0;
 if(!n)return 1;
 const uint8_t *payload=(const uint8_t *)(rec+1);
 if(z){if(!roto_lz4_decode(payload,z,packet,n))return 0;payload=packet;}
 return roto_apply_words(out,payload,n);
}
static int delta(uint8_t *out,const uint32_t *clip,unsigned i){
 unsigned a=clip[5u+i],b=clip[6u+i];
 unsigned directory=4u*(6u+clip[1]+2u*clip[3]);
 if(a<directory||b>clip[5u+clip[1]])return 0;
 return decode_record(out,clip,a,b);
}
/* Seek table pairs (sample, absolute record offset), in increasing
   sample order. Recover only from a nearby independent picture, then
   apply at most ROTO_SEEK_WINDOW ordinary deltas. */
static int recover(uint8_t *out,const uint32_t *clip,unsigned sample,RotoBase base){
 if(sample<=ROTO_SEEK_WINDOW){
  base(out);for(unsigned k=0;k<=sample;k++)if(!delta(out,clip,k))return -1;
  return 1;
 }
 unsigned count=clip[3],begin=clip[5u+clip[1]];
 const uint32_t *table=clip+6u+clip[1];
 unsigned lo=0,hi=count;
 while(lo<hi){unsigned mid=lo+(hi-lo)/2u;if(table[2u*mid]<=sample)lo=mid+1u;else hi=mid;}
 if(!lo)return 0;
 unsigned i=lo-1u,k=table[2u*i];
 if(k>sample||sample-k>ROTO_SEEK_WINDOW)return 0;
 unsigned a=table[2u*i+1u],b=i+1u<count?table[2u*i+3u]:clip[4];
 if(a<begin||k>=clip[1])return -1;
 base(out);if(!decode_record(out,clip,a,b))return -1;
 for(k++;k<=sample;k++)if(!delta(out,clip,k))return -1;
 return 1;
}
int roto_cache_render(uint8_t *out,const uint32_t *clip,unsigned tick,RotoBase base,RotoPaint live){
 if(!lease||out!=(uint8_t *)(void *)hd_shared_frame||!clip||!base||!live){valid=0;return 0;}
 if(clip[0]!=ROTO_CLIP_MAGIC||clip[2]>65535u||clip[3]>ROTO_SEEK_MAX||
    clip[1]!=clip[2]/2u+1u+(clip[2]&1u)||clip[4]>ROTO_CLIP_LIMIT||
    clip[4]<4u*(6u+clip[1]+2u*clip[3])||clip[5u+clip[1]]>clip[4]){valid=0;return 0;}
 unsigned sample=tick>=clip[2]?clip[1]-1u:tick/2u;
 if(!valid||current_clip!=clip||sample<current_sample||sample-current_sample>2u){
  valid=0;int found=recover(out,clip,sample,base);
  if(found<0)return 0;
  if(!found)live(out,sample==clip[1]-1u?clip[2]:sample*2u);
 }else{
  for(unsigned k=current_sample+1u;k<=sample;k++)if(!delta(out,clip,k)){valid=0;return 0;}
 }
 current_clip=clip;current_sample=sample;valid=1;
 hd_rect(out,0,135,HD_W,25,HD_INK);
 return 1;
}
#endif
