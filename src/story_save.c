#include "story_save.h"
#include <string.h>

int story_save_decode(const uint8_t *raw,uint8_t *normalized){
    unsigned len=raw[4]==1?13u:raw[4]==2?16u:
        raw[4]==3?24u:raw[4]==4?28u:
        raw[4]==STORY_SAVE_VER?STORY_SAVE_BYTES:0u;
    if(!len || raw[0]!='D' || raw[1]!='S' || raw[2]!='T' || raw[3]!='2')
        return 0;
    unsigned check=0x6Du;
    for(unsigned i=0;i<len-1u;i++)check^=raw[i];
    if(raw[len-1u]!=(uint8_t)check)return 0;
    uint8_t b[STORY_SAVE_BYTES]={0};
    memcpy(b,raw,len);
    if(len==13u)b[12]=b[13]=b[14]=0;
    /* Versions 1/2 had immediate ending choices. Retain quest milestones,
       but restore the old game to a playable, numbered-day state. */
    if(len<24u){
        b[10]=b[11]=0;
        for(unsigned i=15;i<24u;i++)b[i]=0;
        b[16]=1u;
    }
    if(len<28u){
        /* Old byte 23 was the v3 checksum, not an energy value. */
        b[23]=3u;b[24]=1u;b[25]=1u;b[26]=0u;
    }
    if(len<STORY_SAVE_BYTES){
        /* Old byte 27 was the v4 checksum, not music progress. */
        b[27]=0;b[28]=0;b[29]=0;
    }
    memcpy(normalized,b,STORY_SAVE_BYTES);
    return 1;
}
void story_save_encode(const Story *s,uint8_t *b){
    uint8_t data[STORY_SAVE_BYTES]={
        'D','S','T','2',STORY_SAVE_VER,s->x,s->y,s->facing,s->view,s->flags,
        s->ended,s->ending,s->cult_stage,s->cult_pre_wins,s->cult_post_wins,
        s->life_stage,s->day_number,(uint8_t)s->day_frame,
        (uint8_t)(s->day_frame>>8),
        (uint8_t)s->finale_frame,(uint8_t)(s->finale_frame>>8),
        s->daily_mask,s->daily_memories,s->energy,s->spark,s->bond,s->work,
        s->song_steps,s->art_steps,s->project_style,0};
    unsigned check=0x6Du;
    for(unsigned i=0;i<STORY_SAVE_BYTES-1u;i++)check^=data[i];
    data[STORY_SAVE_BYTES-1u]=(uint8_t)check;
    memcpy(b,data,STORY_SAVE_BYTES);
}
