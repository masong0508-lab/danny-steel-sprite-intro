/* Generate the original renderer's own pixels, not source video.
   All 60-Hz authored ticks are retained; the GBA can present whichever
   exact frame the live input/story clock asks for. */
#include <stdio.h>
#include <string.h>
#include "hd_potheads_film.h"
static uint32_t base[HD_BYTES/4u],frame[HD_BYTES/4u];
static uint32_t checked[HD_BYTES/4u+2u],record[POT_CACHE_RECORD_MAX+2u];
static uint32_t offsets[POT_CACHE_TICKS];
#define CACHE_LIMIT (24u*1024u*1024u)
static int put(FILE *out,const uint32_t *words,unsigned n){
 return fwrite(words,sizeof(*words),n,out)==n;
}
int main(int argc,char **argv){
 const uint16_t endian=1u;
 if(argc!=2||*(const uint8_t *)(const void *)&endian!=1u){
  fputs("Usage: build_potheads_cache OUTPUT (little-endian host)\n",stderr);return 1;
 }
 FILE *out=fopen(argv[1],"wb");if(!out){perror(argv[1]);return 1;}
 pot_scene_fixed((uint8_t *)base);
 hd_rect((uint8_t *)base,0,135,HD_W,25,HD_INK);
 if(!put(out,offsets,POT_CACHE_TICKS))goto fail;
 unsigned at=POT_CACHE_TICKS,raw=0,patch=0;
 for(unsigned t=0;t<POT_CACHE_TICKS;t++){
  pot_film_live((uint8_t *)frame,t);
  for(unsigned j=0;j<HD_BYTES;j++)if(((uint8_t *)frame)[j]>=64u)goto fail;
  record[POT_CACHE_RECORD_MAX]=record[POT_CACHE_RECORD_MAX+1u]=0xB7B7B7B7u;
  unsigned n=pot_cache_encode(base,frame,record,t<POT_CACHE_DESCENT);
  if(!n||n>POT_CACHE_RECORD_MAX||at+n>CACHE_LIMIT/4u)goto fail;
  if(record[0]==POT_CACHE_RAW){if(n!=POT_CACHE_RECORD_MAX)goto fail;raw++;}
  else{
   unsigned k=1u;patch++;
   for(unsigned i=0;i<record[0];i++){
    if(k>=n)goto fail;
    unsigned span=record[k++],start=span&65535u,count=span>>16;
    if(!count||start+count>POT_CACHE_WORDS||count>n-k)goto fail;
    k+=count;
   }
   if(k!=n)goto fail;
  }
  checked[0]=checked[HD_BYTES/4u+1u]=0xA5A5A5A5u;
  memcpy(checked+1u,base,HD_BYTES);
  pot_cache_apply(checked+1u,record);
  if(memcmp(checked+1u,frame,HD_BYTES)||
     checked[0]!=0xA5A5A5A5u||checked[HD_BYTES/4u+1u]!=0xA5A5A5A5u||
     record[POT_CACHE_RECORD_MAX]!=0xB7B7B7B7u||
     record[POT_CACHE_RECORD_MAX+1u]!=0xB7B7B7B7u)goto fail;
  offsets[t]=at;at+=n;
  if(!put(out,record,n))goto fail;
 }
 /* Confirm the final held pose is stable, including large clock values. */
 pot_film_live((uint8_t *)(checked+1u),POT_CACHE_TICKS+600u);
 if(memcmp(checked+1u,frame,HD_BYTES))goto fail;
 if(fseek(out,0,SEEK_SET)||!put(out,offsets,POT_CACHE_TICKS))goto fail;
 if(fclose(out)){remove(argv[1]);return 1;}
 printf("Potheads opening cache: %u exact frames (%u raw, %u patches), %u bytes; full raster/guard equivalence OK\n",
        POT_CACHE_TICKS,raw,patch,at*4u);
 return 0;
fail:
 fputs("Opening cache generation, raster verification or 24 MiB budget failed\n",stderr);
 fclose(out);remove(argv[1]);return 1;
}
