/* Bake exact native 30-Hz pictures from the authored 60-Hz timeline.
   ROT2 uses aligned absolute word spans to reduce ARM7 decoding cost. */
#include <stdio.h>
#include <string.h>
#include <lz4hc.h>
#include "hd_potheads_film.h"
static uint32_t base_words[HD_BYTES/4u],frame_words[HD_BYTES/4u],prev_words[HD_BYTES/4u];
static uint32_t guarded[ROTO_WORDS+2u],decoded[ROTO_PACKET_MAX/4u+2u],packet[ROTO_PACKET_MAX/4u];
static uint32_t presented[ROTO_WORDS],expected[ROTO_WORDS];
static char compressed[LZ4_COMPRESSBOUND(ROTO_PACKET_MAX)];
static uint32_t header[6u+POT_CACHE_SAMPLES];
static unsigned patch(const uint32_t *a,const uint32_t *b,uint32_t *out){
 unsigned at=0,i=0;
 while(i<ROTO_WORDS){
  if(a[i]==b[i]){i++;continue;}
  unsigned start=i,end=++i;
  while(i<ROTO_WORDS&&i-end<5u){if(a[i]!=b[i])end=i+1u;i++;}
  unsigned n=end-start;
  if(at+1u+n>ROTO_PACKET_MAX/4u)return UINT32_MAX;
  out[at++]=start|(n<<16);memcpy(out+at,b+start,n*4u);at+=n;
 }
 return at*4u;
}
static int put(FILE *out,const void *p,unsigned n){return fwrite(p,1,n,out)==n;}
int main(int argc,char **argv){
 const uint16_t endian=1u;
 if(argc!=2||*(const uint8_t *)(const void *)&endian!=1u){
  fputs("Usage: build_potheads_cache OUTPUT (little-endian host)\n",stderr);return 1;
 }
 FILE *out=fopen(argv[1],"wb");if(!out){perror(argv[1]);return 1;}
 pot_scene_fixed((uint8_t *)base_words);
 hd_rect((uint8_t *)base_words,0,135,HD_W,25,HD_INK);
 header[0]=ROTO_CLIP_MAGIC;header[1]=POT_CACHE_SAMPLES;
 header[2]=POT_CACHE_LAST;header[3]=ROTO_KEY_INTERVAL;
 unsigned at=sizeof header,max_packet=0,raw_packets=0;
 if(!put(out,header,sizeof header))goto fail;
 for(unsigned i=0;i<POT_CACHE_SAMPLES;i++){
  unsigned t=i==POT_CACHE_SAMPLES-1u?POT_CACHE_LAST:i*2u;
  pot_film_live((uint8_t *)frame_words,t);
  for(unsigned j=0;j<HD_BYTES;j++)if(((const uint8_t *)frame_words)[j]>=64u)goto fail;
  if(i%ROTO_KEY_INTERVAL==0u){
   memcpy(prev_words,base_words,HD_BYTES);
   memcpy(presented,base_words,ROTO_PIXELS);
  }
  unsigned n=patch(prev_words,frame_words,packet);
  if(n>ROTO_PACKET_MAX)goto fail;
  if(n>max_packet)max_packet=n;
  int z=n?LZ4_compress_HC((const char *)packet,compressed,(int)n,sizeof compressed,5):0;
  if(n&&z<=0)goto fail;
  if((unsigned)z>=n)z=0;
  decoded[0]=decoded[ROTO_PACKET_MAX/4u+1u]=0xAFAFAFAFu;
  guarded[0]=guarded[ROTO_WORDS+1u]=0xB7B7B7B7u;
  const uint8_t *payload=(const uint8_t *)packet;
  if(z){
   if(!roto_lz4_decode((const uint8_t *)compressed,(unsigned)z,(uint8_t *)(decoded+1u),n)||
      memcmp(decoded+1u,packet,n))goto fail;
   payload=(const uint8_t *)(decoded+1u);
  }else raw_packets++;
  memcpy(guarded+1u,prev_words,ROTO_PIXELS);
  if(!roto_apply_words((uint8_t *)(guarded+1u),payload,n)||
     memcmp(guarded+1u,frame_words,ROTO_PIXELS)||
     decoded[0]!=0xAFAFAFAFu||decoded[ROTO_PACKET_MAX/4u+1u]!=0xAFAFAFAFu||
     guarded[0]!=0xB7B7B7B7u||guarded[ROTO_WORDS+1u]!=0xB7B7B7B7u)goto fail;
  /* Model the persistent runtime canvas: the film's top matte is
     overwritten after each render, without poisoning later pictures. */
  if(!roto_apply_words((uint8_t *)presented,payload,n))goto fail;
  memcpy(expected,frame_words,ROTO_PIXELS);
  memset(presented,HD_INK,2u*HD_W);
  memset(expected,HD_INK,2u*HD_W);
  if(memcmp(presented,expected,ROTO_PIXELS))goto fail;
  uint32_t lengths=n|((unsigned)z<<16);
  unsigned stored=z?(unsigned)z:n,padded=(stored+3u)&~3u;
  if(at+4u+padded>ROTO_CLIP_LIMIT)goto fail;
  header[5u+i]=at;at+=4u+padded;
  static const uint8_t zeros[3]={0,0,0};
  if(!put(out,&lengths,4u)||!put(out,z?(const void *)compressed:(const void *)packet,stored)||
     !put(out,zeros,padded-stored))goto fail;
  memcpy(prev_words,frame_words,HD_BYTES);
 }
 pot_film_live((uint8_t *)frame_words,POT_CACHE_TICKS+600u);
 if(memcmp(prev_words,frame_words,HD_BYTES))goto fail;
 header[4]=at;header[5u+POT_CACHE_SAMPLES]=at;
 if(fseek(out,0,SEEK_SET)||!put(out,header,sizeof header))goto fail;
 if(fclose(out)){remove(argv[1]);return 1;}
 printf("ROT2: %u exact pictures, %u original 60-Hz ticks, %u bytes (limit %u), max packet %u, %u raw packets; raster, matte and guard checks passed\n",
        POT_CACHE_SAMPLES,POT_CACHE_TICKS,at,ROTO_CLIP_LIMIT,max_packet,raw_packets);
 return 0;
fail:
 fputs("ROT2 generation, raster verification or 1.2 MiB scene budget failed\n",stderr);
 fclose(out);remove(argv[1]);return 1;
}
