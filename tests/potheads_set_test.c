#include "hd_canvas.h"
#include "hd_potheads_set.h"
#include "hd_potheads_fixed.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* An incorrect word count, stale generated set or
   unaligned word write must fail before this data reaches a cartridge. */
static uint8_t projected[HD_BYTES];
static uint8_t decoded[HD_BYTES+132];

static void verify_words(void){
 assert(sizeof pot_set_words==HD_BYTES);
 for(unsigned i=0;i<HD_BYTES/4u;i++){
  uint32_t four=pot_set_words[i];
  for(unsigned pixel=0;pixel<4u;pixel++){
   assert((four&255u)<64u);four>>=8;
  }
 }
}
static void crop(uint8_t *p){
 hd_rect(p,0,135,HD_W,25,HD_INK);
 hd_rect(p,0,0,240,2,HD_INK);
 hd_rect(p,0,158,240,2,HD_INK);
}
int main(void){
 verify_words();
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
 printf("Potheads set: all 38400 pixels match the projected camera, palette, guards, four alignments and exact word count OK (%zu ROM bytes)\n",sizeof pot_set_words);
 return 0;
}
