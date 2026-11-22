#include "hd_canvas.h"
#include "hd_potheads_set.h"
#include "hd_potheads_fixed.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* A broken run count, truncated literal block, stale generated set or
   unaligned word write must fail before this data reaches a cartridge. */
static uint8_t projected[HD_BYTES];
static uint8_t decoded[HD_BYTES+132];

static void verify_stream(void){
 unsigned at=0,words=0,limit=(unsigned)(sizeof pot_set_rle/sizeof pot_set_rle[0]);
 while(at<limit){
  uint32_t token=pot_set_rle[at++];
  unsigned count=token&0x7fffffffu;
  assert(count>0u&&count<=HD_BYTES/4u-words);
  unsigned stored=token&0x80000000u?1u:count;
  assert(stored<=limit-at);
  for(unsigned i=0;i<stored;i++){
   uint32_t four=pot_set_rle[at++];
   for(unsigned pixel=0;pixel<4u;pixel++){
    assert((four&255u)<64u);four>>=8;
   }
  }
  words+=count;
 }
 assert(at==limit&&words==HD_BYTES/4u);
}
static void crop(uint8_t *p){
 hd_rect(p,0,135,HD_W,25,HD_INK);
 hd_rect(p,0,0,240,2,HD_INK);
 hd_rect(p,0,158,240,2,HD_INK);
}
int main(void){
 verify_stream();
 static const unsigned settled[]={180u,181u,450u,790u,1274u,65535u};
 for(unsigned t=0;t<sizeof settled/sizeof settled[0];t++){
  PotCamera camera=pot_camera(settled[t]);
  memset(projected,0xA6,sizeof projected);
  pot_scene(projected,&camera,settled[t]);
  for(unsigned alignment=0;alignment<4u;alignment++){
   memset(decoded,0xB7,sizeof decoded);
   uint8_t *pixels=decoded+64u+alignment;
   pot_scene_fixed(pixels);
   assert(memcmp(projected,pixels,HD_BYTES)==0);
   for(unsigned i=0;i<64u+alignment;i++)assert(decoded[i]==0xB7);
   for(unsigned i=64u+alignment+HD_BYTES;i<sizeof decoded;i++)assert(decoded[i]==0xB7);
   for(unsigned i=0;i<HD_BYTES;i++)assert(pixels[i]<64u);
  }
  uint8_t *pixels=decoded+64u;
  pot_scene_fixed(pixels);crop(pixels);crop(projected);
  assert(memcmp(projected,pixels,HD_BYTES)==0);
 }
 printf("Potheads set: all 38400 pixels match the projected camera, palette, guards, four alignments and exact stream end OK (%zu ROM bytes)\n",sizeof pot_set_rle);
 return 0;
}
