#include "roto_cache.h"
#include "roto_codec.h"
#include "hd_canvas.h"
#if defined(DANNY_OPENING_CACHE) && defined(__arm__) && !defined(HOST_TEST)
/* 38,400 + 32,768 bytes, reusable across every scene. No VRAM feedback or
   dependence on the preceding subtitle, palette overlay or displayed page. */
static uint32_t picture[HD_BYTES/4u] __attribute__((section(".ewram"),aligned(4)));
static uint8_t packet[ROTO_PACKET_MAX] __attribute__((section(".ewram"),aligned(4)));
static const uint32_t *current_clip;
static unsigned current_sample;
static int valid;
static int decode(const uint32_t *clip,unsigned sample,RotoBase base){
 unsigned a=clip[5u+sample],b=clip[6u+sample];
 if(a<4u*(6u+clip[1])||b<a+4u||b>clip[4]||(a&3u)||(b&3u))return 0;
 const uint32_t *rec=(const uint32_t *)((const uint8_t *)clip+a);
 unsigned n=rec[0]&65535u,z=rec[0]>>16;
 unsigned stored=z?z:n;
 if(n>ROTO_PACKET_MAX||stored>b-a-4u||(!n&&z))return 0;
 if(sample%clip[3]==0u)base((uint8_t *)picture);
 if(!n)return 1;
 const uint8_t *payload=(const uint8_t *)(rec+1);
 if(z){
  if(!roto_lz4_decode(payload,z,packet,n))return 0;
  payload=packet;
 }
 return roto_apply_patch((uint8_t *)picture,payload,n);
}
int roto_cache_render(uint8_t *out,const uint32_t *clip,unsigned tick,RotoBase base,RotoPaint live){
 uintptr_t dest=(uintptr_t)out;
 if((dest&3u)||dest<0x02000000u||dest>0x02040000u-HD_BYTES||!base||!live)return 0;
 /* ROT1, count, last authored 60-Hz tick, key interval, total bytes. */
 if(clip[0]!=ROTO_CLIP_MAGIC||clip[2]>65535u||clip[3]!=ROTO_KEY_INTERVAL||
    clip[1]!=clip[2]/2u+1u+(clip[2]&1u)||clip[4]>ROTO_CLIP_LIMIT||
    clip[4]<4u*(6u+clip[1]))return 0;
 unsigned sample=tick>=clip[2]?clip[1]-1u:tick/2u;
 if(!valid||current_clip!=clip||sample<current_sample||sample-current_sample>2u){
  /* A dialogue cue can jump hundreds of ticks. Do not decode a long
     delta chain in one VBlank: seed once from the original live painter.
     Keyframes can be independently decoded over the immutable set. */
  valid=0;
  if(sample%clip[3]==0u){if(!decode(clip,sample,base))return 0;}
  else live((uint8_t *)picture,sample==clip[1]-1u?clip[2]:sample*2u);
 }else{
  for(unsigned k=current_sample+1u;k<=sample;k++){
   if(!decode(clip,k,base)){valid=0;return 0;}
  }
 }
 current_clip=clip;current_sample=sample;valid=1;
 volatile uint32_t *dma=(volatile uint32_t *)0x040000D4u;
 while(dma[2]&0x80000000u){}
 dma[2]=0;
 dma[0]=(uint32_t)(uintptr_t)picture;
 dma[1]=(uint32_t)dest;
 __asm__ volatile("" ::: "memory");
 dma[2]=0x84000000u|(ROTO_PIXELS/4u);
 while(dma[2]&0x80000000u){}
 __asm__ volatile("" ::: "memory");
 hd_rect(out,0,135,HD_W,25,HD_INK);
 return 1;
}
#endif
