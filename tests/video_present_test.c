#include "video_present.h"
#include "hd_canvas.h"
#include "gba.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

u8 host_vram[0x18000] __attribute__((aligned(4)));
u16 host_pal[512];
static struct {uint32_t before[16],frame[HD_BYTES/4],after[16];} source;

int main(void){
 memset(&source,0x91,sizeof source);
 for(unsigned i=0;i<HD_BYTES/4u;i++)source.frame[i]=(i*0x10204081u)^0xA53C7E19u;
 for(unsigned page=0;page<2u;page++){
  memset(host_vram,0xD3,sizeof host_vram);
  video_copy_frame(source.frame,page);
  unsigned start=page?0xA000u:0u;
  for(unsigned i=0;i<sizeof host_vram;i++){
   uint8_t expected=i>=start&&i<start+HD_BYTES?
     ((const uint8_t *)source.frame)[i-start]:0xD3u;
   assert(host_vram[i]==expected);
  }
  for(unsigned i=0;i<HD_BYTES/4u;i++)assert(source.frame[i]==((i*0x10204081u)^0xA53C7E19u));
  for(unsigned i=0;i<16u;i++)assert(source.before[i]==0x91919191u&&source.after[i]==0x91919191u);
 }
 puts("video transfer: both exact 38,400-byte pages, VRAM gaps and source guards preserved");
 return 0;
}
