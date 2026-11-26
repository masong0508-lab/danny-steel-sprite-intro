#include "studio.h"

/* One small pattern per active slot, with no heap or sample buffers.
   The complete arrangement and settings occupy only sixteen data bytes. */
static const StudioPatch initial={
 {3,0,5,3,4,0,5,6},
 {0,1,0,2},1,1,0,0xABu
};
static StudioPatch patch={
 {3,0,5,3,4,0,5,6},
 {0,1,0,2},1,1,0,0xABu
};
static unsigned playhead;
static const char *const lines[4]={
 "THE SKY STAYS OPEN",
 "SOMEONE LEFT A LIGHT",
 "KEEP THE LINE OPEN",
 "SONGS FIND THEIR WAY"
};
void studio_reset(void){patch=initial;playhead=0;}
const StudioPatch *studio_patch(void){return &patch;}
const char *studio_lyric(unsigned idx){return lines[idx&3u];}
const char *studio_note_label(unsigned pitch){
 static const char *const labels[8]={"REST","C4","D4","E4","F4","G4","A4","B4"};
 return pitch<8u?labels[pitch]:"?";
}
unsigned studio_cursor_count(unsigned tab){
 return tab==STUDIO_NOTES?8u:tab==STUDIO_ORDER?4u:
        tab==STUDIO_MIX?STUDIO_TRACK_COUNT:tab==STUDIO_SONG?2u:1u;
}
static uint8_t wrap(unsigned value,unsigned count,int delta){
 int n=(int)value+delta;
 while(n<0)n+=(int)count;
 while(n>=(int)count)n-=(int)count;
 return (uint8_t)n;
}
unsigned studio_mix_level(const StudioPatch *p,unsigned track){
 return track<STUDIO_TRACK_COUNT?(p->mix>>(track*2u))&3u:0u;
}
const char *studio_mix_name(unsigned track){
 static const char *const names[STUDIO_TRACK_COUNT]={"LEAD","KEYS","BASS","DRUMS"};
 return track<STUDIO_TRACK_COUNT?names[track]:"";
}
static void mix_set(unsigned track,unsigned level){
 if(track>=STUDIO_TRACK_COUNT)return;
 unsigned shift=track*2u;
 patch.mix=(uint8_t)((patch.mix&~(3u<<shift))|((level&3u)<<shift));
}
void studio_change(unsigned tab,unsigned cursor,int delta){
 if(!delta)return;
 switch(tab){
 case STUDIO_SONG:
  if(cursor==0u)patch.tempo=wrap(patch.tempo,4u,delta);
  else patch.mood=wrap(patch.mood,3u,delta);
  break;
 case STUDIO_NOTES:
  if(cursor<8u){
   uint8_t *n=&patch.notes[cursor];
   if(!*n)*n=4u;
   else {
    int value=(int)*n+delta;
    if(value<1)value=1;
    if(value>7)value=7;
    *n=(uint8_t)value;
   }
  }
  break;
 case STUDIO_ORDER:
  if(cursor<4u)patch.order[cursor]=wrap(patch.order[cursor],4u,delta);
  break;
 case STUDIO_LYRICS:patch.lyric=wrap(patch.lyric,4u,delta);break;
 case STUDIO_MIX:{
  int level=(int)studio_mix_level(&patch,cursor)+delta;
  mix_set(cursor,(unsigned)(level<0?0:level>3?3:level));
  break;
 }
 default:break;
 }
}
void studio_toggle(unsigned tab,unsigned cursor){
 if(tab==STUDIO_NOTES && cursor<8u){
  patch.notes[cursor]=patch.notes[cursor]?0u:4u;
 }else if(tab==STUDIO_MIX)mix_set(cursor,studio_mix_level(&patch,cursor)?0u:2u);
 else studio_change(tab,cursor,1);
}
uint32_t studio_ticks(void){
 static const uint32_t beat[4]={88000u,65536u,56000u,48000u};
 return beat[patch.tempo];
}
unsigned studio_playhead(void){return playhead&7u;}
unsigned studio_playpart(void){return (playhead>>3)&3u;}
void studio_preview_step(const StudioPatch *p,unsigned step,MotifBeat *out){
 static const uint8_t pitch[8]={0u,60u,62u,64u,65u,67u,69u,71u};
 static const int8_t transpose[4]={0,2,-2,5};
 unsigned section=(step/8u)&3u;
 unsigned index=step&7u;
 unsigned mode=p->order[section];
 int note=p->notes[index];
 int n=note?((int)pitch[note]+transpose[mode]):0;
 if(n>96)n=96;
 if(n<36&&n>0)n=36;
 unsigned lead=studio_mix_level(p,STUDIO_LEAD);
 unsigned volume=p->mood==0u?7u:p->mood==1u?10u:12u;
 out->lead=n&&lead?(int8_t)n:-1;
 out->volume=(uint8_t)((volume*lead+2u)/3u);
 out->duty=(uint8_t)(p->mood==0u?2u:p->mood==1u?1u:0u);
 /* The order controls chord-root motion as well as melodic register. */
 unsigned root=48u+(unsigned)(mode==1u?2u:mode==2u?5u:mode==3u?7u:0u);
 out->bass=(uint8_t)(studio_mix_level(p,STUDIO_BASS)&&!(index&3u)?root:0u);
 /* A simple electric-keyboard fifth, not a simulated singer. */
 out->comp=(uint8_t)(studio_mix_level(p,STUDIO_KEYS)&&!(index&1u)?root+19u:0u);
 out->noise=(uint8_t)(!studio_mix_level(p,STUDIO_DRUMS)?0u:
             (index==2u||index==6u)?2u:(p->mood==2u&&(index&1u)?1u:0u));
}
void studio_motif_step(unsigned step,MotifBeat *out){
 studio_preview_step(&patch,step,out);
 playhead=step&31u;
}
void studio_save_encode(unsigned slot,uint8_t out[STUDIO_EXT_LEN]){
 out[0]='B';out[1]='S';out[2]=2u;out[3]=(uint8_t)slot;
 for(unsigned i=0;i<8u;i++)out[4u+i]=patch.notes[i];
 for(unsigned i=0;i<4u;i++)out[12u+i]=patch.order[i];
 out[16]=patch.tempo;out[17]=patch.mood;
 out[18]=patch.lyric;out[19]=patch.mix;
 out[20]=0x6Bu;out[21]=0x2Fu;out[22]=0xC3u;
 uint8_t sum=0xB6u;
 for(unsigned i=0;i<STUDIO_EXT_LEN-1u;i++)sum^=out[i];
 out[STUDIO_EXT_LEN-1u]=sum;
}
int studio_save_decode(unsigned slot,const uint8_t in[STUDIO_EXT_LEN]){
 if(in[0]!='B'||in[1]!='S'||(in[2]!=1u&&in[2]!=2u)||in[3]!=slot||
    in[20]!=0x6Bu||in[21]!=0x2Fu||in[22]!=0xC3u)
    return 0;
 uint8_t sum=0xB6u;
 for(unsigned i=0;i<STUDIO_EXT_LEN-1u;i++)sum^=in[i];
 if(sum!=in[STUDIO_EXT_LEN-1u])return 0;
 for(unsigned i=0;i<8u;i++)if(in[4u+i]>7u)return 0;
 for(unsigned i=0;i<4u;i++)if(in[12u+i]>3u)return 0;
 if(in[16]>3u||in[17]>2u||in[18]>3u||(in[2]==1u&&in[19]>3u))return 0;
 for(unsigned i=0;i<8u;i++)patch.notes[i]=in[4u+i];
 for(unsigned i=0;i<4u;i++)patch.order[i]=in[12u+i];
 patch.tempo=in[16];patch.mood=in[17];
 patch.lyric=in[18];
 /* Migrate v1's former accompaniment setting to keyboard on/off.
    Preserve melody, order, tempo, feel and lyrics; byte count unchanged. */
 patch.mix=in[2]==2u?in[19]:(uint8_t)(3u|(in[19]?8u:0u)|32u|(in[17]==2u?128u:0u));
 playhead=0;
 return 1;
}
