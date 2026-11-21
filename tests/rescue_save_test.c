#include "rescue_save.h"
#include "story.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
int main(void){
 assert(RESCUE_EXT_OFFSET>STORY_W*STORY_H/4); /* outside basic slot range */
 assert(RESCUE_EXT_LEN==8u);
 uint8_t b[RESCUE_EXT_LEN],mask=0,phase=0;
 rescue_save_encode(0,STORY_BAD_LETTER|STORY_BAD_MUSIC,
                    STORY_RESCUE_WITNESS,b);
 assert(rescue_save_decode(0,b,&mask,&phase));
 assert(mask==(STORY_BAD_LETTER|STORY_BAD_MUSIC));
 assert(phase==STORY_RESCUE_WITNESS);
 assert(!rescue_save_decode(1,b,&mask,&phase));
 b[4]^=STORY_BAD_CLERK;
 assert(!rescue_save_decode(0,b,&mask,&phase));
 b[4]^=STORY_BAD_CLERK;
 b[2]=9;assert(!rescue_save_decode(0,b,&mask,&phase));
 b[2]=1;
 rescue_save_encode(1,15u,STORY_RESCUE_AFTERSHOCK,b);
 assert(rescue_save_decode(1,b,&mask,&phase));
 assert(mask==15u&&phase==STORY_RESCUE_AFTERSHOCK);
 memset(b,0xFF,sizeof(b));
 assert(!rescue_save_decode(1,b,&mask,&phase));
 puts("rescue SRAM: checksummed 8-byte slot-local extension, slot isolation and erased defaults OK");
 return 0;
}
