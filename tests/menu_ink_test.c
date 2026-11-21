#include "menu_ink.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {
 unsigned char before[32];
 unsigned char pixels[MENU_INK_W*MENU_INK_H];
 unsigned char after[32];
} frame;
static void check(unsigned level,unsigned mode,unsigned slot,
                  unsigned valid,unsigned held,unsigned feedback){
 memset(&frame,0xCC,sizeof frame);
 /* Pixel 255 in the footage must not accidentally turn into paper pixels. */
 frame.pixels[25*MENU_INK_W+20]=255u;
 menu_ink_draw(frame.pixels,level,mode,slot,valid,held,feedback);
 for(unsigned i=0;i<32;i++)
  assert(frame.before[i]==0xCC&&frame.after[i]==0xCC);
 assert(frame.pixels[25*MENU_INK_W+20]==253u);
 assert(frame.pixels[0]==254u);
 assert(frame.pixels[3*MENU_INK_W+4]==254u || frame.pixels[3*MENU_INK_W+4]==255u);
 assert(frame.pixels[73*MENU_INK_W+4]<=255u);
}
int main(void){
 for(unsigned level=0;level<2;level++)
  for(unsigned mode=0;mode<3;mode++)
   for(unsigned slot=0;slot<3;slot++)
    for(unsigned valid=0;valid<2;valid++)
     for(unsigned feedback=0;feedback<3;feedback++)
      check(level,mode,slot,valid,0,feedback);
 check(1,2,1,1,MENU_ERASE_HOLD/2,MENU_NOTE_NONE);
 assert(frame.pixels[69*MENU_INK_W+6]==255u);
 assert(frame.pixels[69*MENU_INK_W+100]!=255u);
 check(1,2,0,1,MENU_ERASE_HOLD,MENU_NOTE_NONE);
 assert(frame.pixels[69*MENU_INK_W+110]==255u);
 puts("menu film overlay: guarded pixels, reserved palette, all menu states and erase progress OK");
 return 0;
}
