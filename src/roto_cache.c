#include "roto_cache.h"
#include "roto_codec.h"
#include "hd_canvas.h"
#if defined(DANNY_OPENING_CACHE) && defined(__arm__) && !defined(HOST_TEST)
/* One shared 32 KiB packet workspace; the game canvas is the picture.
   Every non-film draw or new game revokes the lease before painting. */
static uint8_t packet[ROTO_PACKET_MAX] __attribute__((section(".ewram"),aligned(4)));
static const uint32_t *current_clip;
static unsigned current_sample;
static int valid,lease;
void roto_cache_begin_frame(int preserve_picture){
 lease=preserve_picture;
 if(!lease)valid=0;
}
static int decode(uint8_t *out,const uint32_t *clip,unsigned sample,RotoBase base){
 unsigned a=clip[5u+sample],b=clip[6u+sample];
 if(a<4u*(6u+clip[1])||b<a+4u||b>clip[4]||(a&3u)||(b&3u))return 0;
 const uint32_t *rec=(const uint32_t *)((const uint8_t *)clip+a);
 unsigned n=rec[0]&65535u,z=rec[0]>>16;
 unsigned stored=z?z:n;
 if((n&3u)||n>ROTO_PACKET_MAX||stored>b-a-4u||(!n&&z))return 0;
 if(sample%clip[3]==0u)base(out);
 if(!n)return 1;
 const uint8_t *payload=(const uint8_t *)(rec+1);
 if(z){
  if(!roto_lz4_decode(payload,z,packet,n))return 0;
  payload=packet;
 }
 return roto_apply_words(out,payload,n);
}
int roto_cache_render(uint8_t *out,const uint32_t *clip,unsigned tick,RotoBase base,RotoPaint live){
 /* Only the leased game canvas may retain a temporal frame. Host
    galleries, menu transitions, or other consumers use the live painter. */
 if(!lease||out!=(uint8_t *)(void *)hd_shared_frame||!base||!live){valid=0;return 0;}
 if(clip[0]!=ROTO_CLIP_MAGIC||clip[2]>65535u||clip[3]!=ROTO_KEY_INTERVAL||
    clip[1]!=clip[2]/2u+1u+(clip[2]&1u)||clip[4]>ROTO_CLIP_LIMIT||
    clip[4]<4u*(6u+clip[1])){valid=0;return 0;}
 unsigned sample=tick>=clip[2]?clip[1]-1u:tick/2u;
 if(!valid||current_clip!=clip||sample<current_sample||sample-current_sample>2u){
  valid=0;
  if(sample%clip[3]==0u){if(!decode(out,clip,sample,base))return 0;}
  else live(out,sample==clip[1]-1u?clip[2]:sample*2u);
 }else{
  for(unsigned k=current_sample+1u;k<=sample;k++){
   if(!decode(out,clip,k,base)){valid=0;return 0;}
  }
 }
 current_clip=clip;current_sample=sample;valid=1;
 /* The film matte is painted by hd_cinema_draw. Its top two rows
    are black each frame, and all patches write absolute pixel values.
    The subtitle strip is outside the 240x135 delta region. */
 hd_rect(out,0,135,HD_W,25,HD_INK);
 return 1;
}
#endif
