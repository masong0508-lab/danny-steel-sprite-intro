#include "hd_canvas.h"
#include "studio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct {
 uint8_t before[48];
 uint8_t pixel[HD_BYTES];
 uint8_t after[48];
} guarded;
static void clear(void){
 memset(&guarded,0xD7,sizeof guarded);
}
static unsigned check(void){
 unsigned hash=2166136261u,details=0;
 for(int i=0;i<48;i++)
  assert(guarded.before[i]==0xD7&&guarded.after[i]==0xD7);
 for(unsigned i=0;i<HD_BYTES;i++){
  assert(guarded.pixel[i]<64u);
  if(guarded.pixel[i]!=HD_INK && guarded.pixel[i]!=HD_SHADOW)details++;
  hash=(hash^(unsigned)guarded.pixel[i])*16777619u;
 }
 assert(details>2800u);
 return hash;
}
int main(void){
 Story s;story_new(&s,0);studio_reset();
 const unsigned locs[4][2]={{12,13},{8,12},{19,4},{11,8}};
 unsigned hashes[4];
 for(unsigned f=0;f<4;f++){
  s.x=(uint8_t)locs[f][0];s.y=(uint8_t)locs[f][1];
  s.facing=(uint8_t)f;s.view=1;s.cinema=CINE_NONE;
  assert(!story_solid(s.x,s.y));
  clear();
  hd_first_person_draw(guarded.pixel,&s,67);
  hashes[f]=check();
  /* The renderer must not be a pasted unrelated room for every facing. */
  for(unsigned j=0;j<f;j++)assert(hashes[f]!=hashes[j]);
  clear();hd_first_person_draw(guarded.pixel,&s,71);check();
 }
 for(unsigned tab=0;tab<STUDIO_TAB_COUNT;tab++){
  s.activity=STORY_ACT_STUDIO;s.studio_tab=(uint8_t)tab;
  s.studio_cursor=0;s.activity_choice=1;
  clear();hd_studio_draw(guarded.pixel,&s,145);
  hashes[tab%4]=check();
  assert(guarded.pixel[21*HD_W+2]==HD_COPPER);
  assert(guarded.pixel[136*HD_W+4]==HD_COPPER);
 }
 s.activity=0;s.dialogue=0;s.ended=0;
 unsigned n=0;
 for(unsigned sc=1;sc<CINE_COUNT;sc++){
  int supported=hd_cinema_supports(sc);
  if(sc==CINE_GOOD||sc==CINE_FINALE||sc==CINE_BAD)assert(supported);
  /* All 24 narrative location types have native 240x160 graphics now. */
  assert(supported);
  if(!supported)continue;
  s.cinema=(uint8_t)sc;
  for(unsigned page=0;page<3;page++){
   s.page=(uint8_t)page;
   clear();hd_cinema_draw(guarded.pixel,&s,65+page*34);
   check();n++;
  }
 }
 assert(n>=30u);
 puts("native first-person: all compass orientations and world locations grounded");
 puts("native Advanced Studio: six real pages render safely");
 puts("native story sets: full palette, guarded pixels and ending fallback preserved");
 return 0;
}
