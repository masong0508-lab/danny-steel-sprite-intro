#include "hd_floor_revisit.h"
#include <assert.h>
#include <stdio.h>
static uint8_t src[HD_BYTES],dst[HD_BYTES+16];
int main(void){
 unsigned checks=0;
 for(unsigned i=0;i<HD_BYTES;i++)src[i]=(uint8_t)(i%64u);
 hd_floor_revisit_reset();
 for(unsigned n=0;n<12u;n++){
  HdFloorKey key={(int)n*64,-128,n&3u,1u};
  assert(hd_floor_revisit_find(&key)<0);
  assert(!hd_floor_revisit_admit(&key));
  assert(hd_floor_revisit_admit(&key));
  hd_floor_revisit_store(src,&key);
  int slot=hd_floor_revisit_find(&key);assert(slot>=0);
  for(unsigned alignment=0;alignment<4u;alignment++){
   memset(dst,0xA6,sizeof dst);uint8_t *p=dst+4u+alignment;
   hd_floor_revisit_restore(p,(unsigned)slot);
   assert(!memcmp(src+80*HD_W,p+80*HD_W,HD_FLOOR_BYTES));
   for(unsigned j=0;j<sizeof dst;j++)
    if(j<4u+alignment+80*HD_W||j>=4u+alignment+140*HD_W)
     assert(dst[j]==0xA6);
   checks++;
  }
 }
 for(unsigned n=0;n<12u;n++){
  HdFloorKey key={(int)n*64,-128,n&3u,1u};
  assert((hd_floor_revisit_find(&key)>=0)==(n>=8u));
 }
 HdFloorKey old={0,-128,0,1},different={0,-128,1,1};
 assert(hd_floor_revisit_admit(&old));
 assert(!hd_floor_revisit_admit(&different));
 hd_floor_revisit_reset();
 assert(hd_floor_revisit_find(&old)<0&&!hd_floor_revisit_admit(&old));
 printf("Revisit cache: %u guarded row copies, correct key/admission/eviction/reset; %zu bytes\n",
        checks,sizeof floor_revisit);
 return 0;
}
