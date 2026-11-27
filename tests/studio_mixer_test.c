#include "studio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void seal(uint8_t *b){unsigned sum=0xB6u;for(unsigned i=0;i<STUDIO_EXT_LEN-1u;i++)sum^=b[i];b[STUDIO_EXT_LEN-1u]=(uint8_t)sum;}
int main(void){
 studio_reset();
 for(unsigned mask=0;mask<256u;mask++){
  for(unsigned t=0;t<4u;t++)studio_change(STUDIO_MIX,t,(int)((mask>>(2u*t))&3u)-(int)studio_mix_level(studio_patch(),t));
  assert(studio_patch()->mix==mask);
  unsigned head=studio_playhead();
  for(unsigned step=0;step<32u;step++){
   MotifBeat n;studio_preview_step(studio_patch(),step,&n);
   if(!(mask&3u))assert(n.lead<0&&!n.volume);
   if(!(mask&12u))assert(!n.comp);
   if(!(mask&48u))assert(!n.bass);
   if(!(mask&192u))assert(!n.noise);
   assert(studio_playhead()==head);
  }
  uint8_t b[STUDIO_EXT_LEN];studio_save_encode(0,b);assert(b[2]==2u&&b[19]==mask);
  StudioPatch old=*studio_patch();studio_reset();assert(studio_save_decode(0,b));
  assert(!memcmp(&old,studio_patch(),sizeof old));
 }
 /* All v1 accompaniment settings migrate without losing the song. */
 uint8_t b[STUDIO_EXT_LEN];studio_reset();studio_save_encode(1,b);
 b[2]=1u;b[4]=7u;b[12]=3u;
 for(unsigned mood=0;mood<3u;mood++)for(unsigned old=0;old<4u;old++){
  b[17]=(uint8_t)mood;b[19]=(uint8_t)old;seal(b);assert(studio_save_decode(1,b));
  assert(studio_patch()->notes[0]==7u&&studio_patch()->order[0]==3u);
  assert(studio_mix_level(studio_patch(),STUDIO_KEYS)==(old?2u:0u));
  assert(studio_mix_level(studio_patch(),STUDIO_DRUMS)==(mood==2u?2u:0u));
 }
 puts("mixer: 256 saved level combinations, 8192 pure sequencer checks, 12 legacy save migrations");return 0;
}
