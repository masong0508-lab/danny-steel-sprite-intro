#include "rescue_save.h"
#include "story.h"
void rescue_save_encode(unsigned slot,unsigned mask,unsigned stage,uint8_t out[8]){
    out[0]='M';out[1]='R';out[2]=1u;out[3]=(uint8_t)slot;
    out[4]=(uint8_t)(mask&15u);out[5]=(uint8_t)stage;
    out[6]=0x5Bu;
    uint8_t sum=0xA7u;
    for(int i=0;i<7;i++)sum^=out[i];
    out[7]=sum;
}
int rescue_save_decode(unsigned slot,const uint8_t in[8],uint8_t *mask,uint8_t *stage){
    if(in[0]!='M'||in[1]!='R'||in[2]!=1u||in[3]!=slot||
       (in[4]&0xF0u)||in[5]>STORY_RESCUE_AFTERSHOCK||in[6]!=0x5Bu)return 0;
    uint8_t sum=0xA7u;
    for(int i=0;i<7;i++)sum^=in[i];
    if(sum!=in[7])return 0;
    *mask=in[4];*stage=in[5];
    return 1;
}
