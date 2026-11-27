/* Bake exact native 30-Hz pictures from the authored 60-Hz timeline.
   ROT3 separates sequential deltas from independent cue recovery records. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lz4hc.h>
#include "hd_potheads_film.h"
static uint32_t base_words[HD_BYTES/4u],frame_words[HD_BYTES/4u],prev_words[HD_BYTES/4u];
static uint32_t guarded[ROTO_WORDS+2u],decoded[ROTO_PACKET_MAX/4u+2u],packet[ROTO_PACKET_MAX/4u];
static char compressed[LZ4_COMPRESSBOUND(ROTO_PACKET_MAX)];
/* Scripted page cues at 90,180,450,780,990,1188 ticks, plus
   optional quarter-film recovery points and the exact final sample. */
static const unsigned anchors[]={45,90,128,225,256,384,390,495,512,594,POT_CACHE_SAMPLES-1u};
#define SEEK_COUNT (sizeof anchors/sizeof anchors[0])
static uint32_t header[6u+POT_CACHE_SAMPLES+2u*SEEK_COUNT];
static unsigned max_packet,raw_packets;
static unsigned patch(const uint32_t *a,const uint32_t *b,uint32_t *out,unsigned gap){
 unsigned at=0,i=0;
 while(i<ROTO_WORDS){
  if(a[i]==b[i]){i++;continue;}
  unsigned start=i,end=++i;
  while(i<ROTO_WORDS&&i-end<gap){if(a[i]!=b[i])end=i+1u;i++;}
  unsigned n=end-start;
  if(at+1u+n>ROTO_PACKET_MAX/4u)return UINT32_MAX;
  out[at++]=start|(n<<16);memcpy(out+at,b+start,n*4u);at+=n;
 }
 return at*4u;
}
static int put(FILE *out,const void *p,unsigned n){return fwrite(p,1,n,out)==n;}
static int encode(FILE *out,const uint32_t *from,const uint32_t *to,unsigned *at){
 /* Choose the smallest real encoded packet, without lowering resolution,
    dropping pictures or changing the decoder. Host CPU only. */
 static const unsigned gaps[]={2u,3u,5u,8u,12u};
 unsigned best=UINT32_MAX,best_gap=0;
 for(unsigned j=0;j<sizeof gaps/sizeof gaps[0];j++){
  unsigned n=patch(from,to,packet,gaps[j]);
  if(n>ROTO_PACKET_MAX)continue;
  int z=n?LZ4_compress_HC((const char *)packet,compressed,(int)n,sizeof compressed,9):0;
  if(n&&z<=0)continue;
  unsigned stored=(z>0&&(unsigned)z<n)?(unsigned)z:n;
  if(stored<best){best=stored;best_gap=gaps[j];}
 }
 if(!best_gap)return 0;
 unsigned n=patch(from,to,packet,best_gap);
 if(n>max_packet)max_packet=n;
 int z=n?LZ4_compress_HC((const char *)packet,compressed,(int)n,sizeof compressed,9):0;
 if(n&&z<=0)return 0;
 if((unsigned)z>=n)z=0;
 decoded[0]=decoded[ROTO_PACKET_MAX/4u+1u]=0xAFAFAFAFu;
 guarded[0]=guarded[ROTO_WORDS+1u]=0xB7B7B7B7u;
 const uint8_t *payload=(const uint8_t *)packet;
 if(z){
  if(!roto_lz4_decode((const uint8_t *)compressed,(unsigned)z,(uint8_t *)(decoded+1u),n)||
     memcmp(decoded+1u,packet,n))return 0;
  payload=(const uint8_t *)(decoded+1u);
 }else raw_packets++;
 memcpy(guarded+1u,from,ROTO_PIXELS);
 if(!roto_apply_words((uint8_t *)(guarded+1u),payload,n)||
    memcmp(guarded+1u,to,ROTO_PIXELS)||
    decoded[0]!=0xAFAFAFAFu||decoded[ROTO_PACKET_MAX/4u+1u]!=0xAFAFAFAFu||
    guarded[0]!=0xB7B7B7B7u||guarded[ROTO_WORDS+1u]!=0xB7B7B7B7u)return 0;
 uint32_t lengths=n|((unsigned)z<<16);
 unsigned stored=z?(unsigned)z:n,padded=(stored+3u)&~3u;
 if(*at+4u+padded>ROTO_CLIP_LIMIT)return 0;
 static const uint8_t zeros[3]={0,0,0};
 if(!put(out,&lengths,4u)||!put(out,z?(const void *)compressed:(const void *)packet,stored)||
    !put(out,zeros,padded-stored))return 0;
 *at+=4u+padded;return 1;
}
#ifdef DANNY_ROTO_HOST_TEST
static unsigned base_calls,live_calls;
static void counted_base(uint8_t *p){base_calls++;pot_scene_fixed(p);}
static void counted_live(uint8_t *p,unsigned t){live_calls++;pot_film_live(p,t);}
static int compare_runtime(const uint32_t *clip,unsigned t){
 uint8_t *out=(uint8_t *)(void *)hd_shared_frame;
 roto_cache_begin_frame(1);
 if(!roto_cache_render(out,clip,t,counted_base,counted_live))return 0;
 unsigned sample=t>=POT_CACHE_LAST?POT_CACHE_SAMPLES-1u:t/2u;
 pot_film_live((uint8_t *)frame_words,sample==POT_CACHE_SAMPLES-1u?POT_CACHE_LAST:sample*2u);
 memset(out,HD_INK,2u*HD_W);memset(frame_words,HD_INK,2u*HD_W);
 if(memcmp(out,frame_words,ROTO_PIXELS))return 0;
 for(unsigned i=ROTO_PIXELS;i<HD_BYTES;i++)if(out[i]!=HD_INK)return 0;
 /* Emulate live captions writing only the reserved region. */
 memset(out+ROTO_PIXELS,HD_PAPER,HD_BYTES-ROTO_PIXELS);
 return 1;
}
static int verify_runtime(FILE *out,unsigned bytes){
 uint32_t *clip=malloc(bytes);if(!clip)return 0;
 int ok=0;unsigned count=0;
 if(fseek(out,0,SEEK_SET)||fread(clip,1,bytes,out)!=bytes)goto done;
 roto_cache_begin_frame(0);
 for(unsigned t=0;t<=POT_CACHE_LAST+1u;t+=2u){
  if(!compare_runtime(clip,t))goto done;
  count++;
 }
 if(base_calls!=1u||live_calls)goto done;
 unsigned seeks=0;
 for(unsigned i=0;i<SEEK_COUNT;i++)for(unsigned k=0;k<=ROTO_SEEK_WINDOW;k++){
  unsigned sample=anchors[i]+k;if(sample>=POT_CACHE_SAMPLES)continue;
  roto_cache_begin_frame(0);memset(hd_shared_frame,0xA5,HD_BYTES);
  unsigned before=live_calls;
  if(!compare_runtime(clip,sample*2u)||live_calls!=before)goto done;
  count++;seeks++;
 }
 uint32_t rng=41u;
 for(unsigned i=0;i<128u;i++){
  rng=rng*1664525u+1013904223u;
  if(i%7u==0u){roto_cache_begin_frame(0);memset(hd_shared_frame,0xA5,HD_BYTES);}
  unsigned t=rng%(POT_CACHE_LAST+200u);
  if(!compare_runtime(clip,t)||!compare_runtime(clip,t+2u)||!compare_runtime(clip,t+4u))goto done;
  count+=3u;
 }
 printf("ROT3 production runtime: %u exact presented pictures, %u cue recoveries without live painting; sequential one base, no live draws\n",count,seeks);
 ok=1;
done:
 roto_cache_begin_frame(0);free(clip);return ok;
}
#endif
int main(int argc,char **argv){
 const uint16_t endian=1u;
 if(argc!=2||*(const uint8_t *)(const void *)&endian!=1u){
  fputs("Usage: build_potheads_cache OUTPUT (little-endian host)\n",stderr);return 1;
 }
 FILE *out=fopen(argv[1],"w+b");if(!out){perror(argv[1]);return 1;}
 pot_scene_fixed((uint8_t *)base_words);
 hd_rect((uint8_t *)base_words,0,135,HD_W,25,HD_INK);
 memcpy(prev_words,base_words,HD_BYTES);
 header[0]=ROTO_CLIP_MAGIC;header[1]=POT_CACHE_SAMPLES;
 header[2]=POT_CACHE_LAST;header[3]=SEEK_COUNT;
 unsigned at=sizeof header;
 if(SEEK_COUNT>ROTO_SEEK_MAX||!put(out,header,sizeof header))goto fail;
 for(unsigned i=0;i<POT_CACHE_SAMPLES;i++){
  unsigned t=i==POT_CACHE_SAMPLES-1u?POT_CACHE_LAST:i*2u;
  pot_film_live((uint8_t *)frame_words,t);
  for(unsigned j=0;j<HD_BYTES;j++)if(((const uint8_t *)frame_words)[j]>=64u)goto fail;
  header[5u+i]=at;
  if(!encode(out,prev_words,frame_words,&at))goto fail;
  memcpy(prev_words,frame_words,HD_BYTES);
 }
 header[5u+POT_CACHE_SAMPLES]=at;
 pot_film_live((uint8_t *)frame_words,POT_CACHE_TICKS+600u);
 if(memcmp(prev_words,frame_words,HD_BYTES))goto fail;
 for(unsigned i=0;i<SEEK_COUNT;i++){
  unsigned sample=anchors[i];
  if(!sample||sample>=POT_CACHE_SAMPLES||(i&&sample<=anchors[i-1u]))goto fail;
  pot_film_live((uint8_t *)frame_words,sample==POT_CACHE_SAMPLES-1u?POT_CACHE_LAST:sample*2u);
  header[6u+POT_CACHE_SAMPLES+2u*i]=sample;
  header[7u+POT_CACHE_SAMPLES+2u*i]=at;
  if(!encode(out,base_words,frame_words,&at))goto fail;
 }
 header[4]=at;
 if(fseek(out,0,SEEK_SET)||!put(out,header,sizeof header)||fflush(out))goto fail;
#ifdef DANNY_ROTO_HOST_TEST
 if(!verify_runtime(out,at))goto fail;
#endif
 if(fclose(out)){remove(argv[1]);return 1;}
 printf("ROT3: %u exact pictures, %zu seek records, %u bytes (limit %u), max packet %u, %u raw packets; raster and guard checks passed\n",
        POT_CACHE_SAMPLES,SEEK_COUNT,at,ROTO_CLIP_LIMIT,max_packet,raw_packets);
 return 0;
fail:
 fputs("ROT3 generation, raster/seek verification or 1.2 MiB scene budget failed\n",stderr);
 fclose(out);remove(argv[1]);return 1;
}
