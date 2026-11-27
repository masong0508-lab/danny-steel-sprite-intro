#include "story_save.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
static void make_old(unsigned ver,unsigned len,uint8_t *raw){
 memset(raw,0xE9,STORY_SAVE_STRIDE);
 raw[0]='D';raw[1]='S';raw[2]='T';raw[3]='2';raw[4]=(uint8_t)ver;
 raw[5]=12;raw[6]=13;raw[7]=3;raw[8]=1;
 raw[9]=STORY_HOME|STORY_STAMP;raw[10]=1;raw[11]=STORY_END_GOOD;
 if(len>=16){raw[12]=2;raw[13]=1;raw[14]=4;}
 if(len>=24){
    raw[15]=STORY_LIFE_WAIT;raw[16]=2;
    raw[17]=0x34;raw[18]=0x12;raw[19]=0x28;raw[20]=0;
    raw[21]=STORY_DAY_MARA|STORY_DAY_ART;raw[22]=7;
 }
 if(len>=28){raw[23]=4;raw[24]=5;raw[25]=2;raw[26]=33;}
 unsigned check=0x6Du;
 for(unsigned i=0;i<len-1;i++)check^=raw[i];
 raw[len-1]=(uint8_t)check;
}
int main(void){
 assert(STORY_SAVE_BYTES<STORY_SAVE_STRIDE);
 Story s;memset(&s,0,sizeof(s));
 s.x=12;s.y=13;s.flags=STORY_FULL;s.day_number=3;
 s.life_stage=STORY_LIFE_FOLLOWUP_WAIT;
 s.day_frame=1115;s.daily_mask=STORY_DAY_MARA|STORY_DAY_MUSIC;
 s.energy=2;s.spark=5;s.bond=4;s.work=27;
 s.song_steps=4;s.art_steps=3;s.project_style=STORY_STYLE_ART_WARM;
 uint8_t b[STORY_SAVE_STRIDE],decoded[STORY_SAVE_BYTES];
 memset(b,0x8B,sizeof(b));story_save_encode(&s,b);
 assert(b[30]!=0x8B&&b[31]==0x8B);
 assert(story_save_decode(b,decoded));
 assert(decoded[4]==STORY_SAVE_VER&&decoded[27]==4&&decoded[28]==3);
 assert(decoded[29]==STORY_STYLE_ART_WARM);
 assert(decoded[21]==s.daily_mask&&decoded[26]==s.work);
 assert(decoded[16]==3&&decoded[17]==(1115u&255u));
 b[20]^=4u;uint8_t guard[STORY_SAVE_BYTES];
 memset(guard,0xC9,sizeof(guard));
 assert(!story_save_decode(b,guard));
 for(unsigned i=0;i<STORY_SAVE_BYTES;i++)assert(guard[i]==0xC9);
 b[20]^=4u;
 s.x=35;s.y=22;story_save_encode(&s,b);
 assert(story_save_decode(b,decoded)&&decoded[5]==35u&&decoded[6]==22u);
 const unsigned lengths[4]={13,16,24,28};
 for(unsigned v=1;v<=4;v++){
    make_old(v,lengths[v-1],b);
    memset(decoded,0xD4,sizeof(decoded));
    assert(story_save_decode(b,decoded));
    assert(decoded[4]==v && decoded[9]==(STORY_HOME|STORY_STAMP));
    assert(decoded[27]==0&&decoded[28]==0&&decoded[29]==0);
    if(v<3){assert(decoded[10]==0&&decoded[11]==0&&decoded[16]==1);}
    else{assert(decoded[21]==(STORY_DAY_MARA|STORY_DAY_ART)&&decoded[22]==7);}
    if(v<=3){assert(decoded[23]==3&&decoded[24]==1&&decoded[25]==1&&decoded[26]==0);}
    else{assert(decoded[23]==4&&decoded[24]==5&&decoded[25]==2&&decoded[26]==33);}
    b[4]=99;assert(!story_save_decode(b,decoded));
 }
 puts("SRAM: v1-v5 checksum, migrated daily data, four stats, two projects, bounded slots and invalid input OK");
 return 0;
}
