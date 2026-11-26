/* Bake native authored artwork, not reference video. A 30-Hz picture
   stream follows the original 60-Hz game clock, including its final pose.
   Future scene generators can reuse this same sparse/LZ4 codec. */
#include <stdio.h>
#include <string.h>
#include <lz4hc.h>
#include "hd_potheads_film.h"
static uint32_t base_words[HD_BYTES/4u],frame_words[HD_BYTES/4u],prev_words[HD_BYTES/4u];
static uint8_t guarded[ROTO_PIXELS+2u],decoded[ROTO_PACKET_MAX+2u],packet[ROTO_PACKET_MAX];
static char compressed[LZ4_COMPRESSBOUND(ROTO_PACKET_MAX)];
static uint32_t header[6u+POT_CACHE_SAMPLES];
static unsigned var(uint8_t *out,unsigned n){
 unsigned k=0;while(n>=128u){out[k++]=(uint8_t)(n|128u);n>>=7;}out[k++]=(uint8_t)n;return k;
}
static unsigned patch(const uint8_t *a,const uint8_t *b,uint8_t *out){
 unsigned at=0,i=0,last=0;while(i<ROTO_PIXELS){
  if(a[i]==b[i]){i++;continue;}
  unsigned start=i,end=++i;
  while(i<ROTO_PIXELS&&i-end<2u){if(a[i]!=b[i])end=i+1u;i++;}
  unsigned n=end-start;if(at+6u+n>ROTO_PACKET_MAX)return UINT32_MAX;
  at+=var(out+at,start-last);at+=var(out+at,n);
  memcpy(out+at,b+start,n);at+=n;last=end;
 }
 return at;
}
static int put(FILE *out,const void *p,unsigned n){return fwrite(p,1,n,out)==n;}
int main(int argc,char **argv){
 const uint16_t endian=1u;
 if(argc!=2||*(const uint8_t *)(const void *)&endian!=1u){
  fputs("Usage: build_potheads_cache OUTPUT (little-endian host)\n",stderr);return 1;
 }
 FILE *out=fopen(argv[1],"wb");if(!out){perror(argv[1]);return 1;}
 uint8_t *base=(uint8_t *)base_words,*frame=(uint8_t *)frame_words,*prev=(uint8_t *)prev_words;
 pot_scene_fixed(base);hd_rect(base,0,135,HD_W,25,HD_INK);
 header[0]=ROTO_CLIP_MAGIC;header[1]=POT_CACHE_SAMPLES;
 header[2]=POT_CACHE_LAST;header[3]=ROTO_KEY_INTERVAL;
 unsigned at=sizeof header,max_packet=0,raw_packets=0;
 if(!put(out,header,sizeof header))goto fail;
 for(unsigned i=0;i<POT_CACHE_SAMPLES;i++){
  unsigned t=i==POT_CACHE_SAMPLES-1u?POT_CACHE_LAST:i*2u;
  pot_film_live(frame,t);
  for(unsigned j=0;j<HD_BYTES;j++)if(frame[j]>=64u)goto fail;
  if(i%ROTO_KEY_INTERVAL==0u)memcpy(prev,base,HD_BYTES);
  unsigned n=patch(prev,frame,packet);
  if(n>ROTO_PACKET_MAX)goto fail;
  if(n>max_packet)max_packet=n;
  int z=n?LZ4_compress_HC((const char *)packet,compressed,(int)n,sizeof compressed,5):0;
  if(n&&z<=0)goto fail;
  if((unsigned)z>=n)z=0;
  decoded[0]=decoded[ROTO_PACKET_MAX+1u]=0xAF;
  guarded[0]=guarded[ROTO_PIXELS+1u]=0xB7;
  const uint8_t *payload=packet;
  if(z){
   if(!roto_lz4_decode((const uint8_t *)compressed,(unsigned)z,decoded+1u,n)||
      memcmp(decoded+1u,packet,n))goto fail;
   payload=decoded+1u;
  }else raw_packets++;
  memcpy(guarded+1u,prev,ROTO_PIXELS);
  if(!roto_apply_patch(guarded+1u,payload,n)||
     memcmp(guarded+1u,frame,ROTO_PIXELS)||
     decoded[0]!=0xAF||decoded[ROTO_PACKET_MAX+1u]!=0xAF||
     guarded[0]!=0xB7||guarded[ROTO_PIXELS+1u]!=0xB7)goto fail;
  uint32_t lengths=n|((unsigned)z<<16);
  unsigned stored=z?(unsigned)z:n,padded=(stored+3u)&~3u;
  if(at+4u+padded>ROTO_CLIP_LIMIT)goto fail;
  header[5u+i]=at;at+=4u+padded;
  static const uint8_t zeros[3]={0,0,0};
  if(!put(out,&lengths,4u)||!put(out,z?(const void *)compressed:(const void *)packet,stored)||
     !put(out,zeros,padded-stored))goto fail;
  memcpy(prev,frame,HD_BYTES);
 }
 pot_film_live(frame,POT_CACHE_TICKS+600u);
 if(memcmp(prev,frame,HD_BYTES))goto fail;
 header[4]=at;header[5u+POT_CACHE_SAMPLES]=at;
 if(fseek(out,0,SEEK_SET)||!put(out,header,sizeof header))goto fail;
 if(fclose(out)){remove(argv[1]);return 1;}
 printf("ROT1: %u pixel-exact pictures, %u original 60-Hz ticks, %u bytes (limit %u), max packet %u, %u raw packets; all raster and guard checks passed\n",
        POT_CACHE_SAMPLES,POT_CACHE_TICKS,at,ROTO_CLIP_LIMIT,max_packet,raw_packets);
 return 0;
fail:
 fputs("ROT1 generation, raster verification or 1.2 MiB scene budget failed\n",stderr);
 fclose(out);remove(argv[1]);return 1;
}
