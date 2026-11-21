#include "hd_canvas.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Native production painter; tests both the prop continuity and the
   conditional story cast without allocating any new GBA framebuffers. */
static struct {
 uint8_t before[32];
 uint8_t pixels[HD_BYTES];
 uint8_t after[32];
} sample;

static uint32_t film(Story *s,unsigned scene,unsigned frame,unsigned page){
 memset(&sample,0xA5,sizeof sample);
 s->cinema=(uint8_t)scene;s->cinema_frame=(uint16_t)frame;
 s->page=(uint8_t)page;
 hd_cinema_draw(sample.pixels,s,frame);
 uint32_t h=2166136261u;
 for(unsigned i=0;i<32u;i++)
  assert(sample.before[i]==0xA5&&sample.after[i]==0xA5);
 for(unsigned i=0;i<HD_BYTES;i++){
  assert(sample.pixels[i]<64u);
  h=(h^sample.pixels[i])*16777619u;
 }
 return h;
}
static unsigned color(int x,int y){
 assert(x>=0&&x<HD_W&&y>=0&&y<HD_H);
 return sample.pixels[y*HD_W+x];
}
int main(void){
 Story s;story_new(&s,0);s.dialogue=0;
 /* The envelope remains laid on the SAME counter; the hand enters
    the shot without replacing the whole asset or moving the letter. */
 uint32_t letter_a=film(&s,CINE_LETTER,140u,0u);
 assert(color(181,105)==HD_PAPER);
 uint32_t letter_b=film(&s,CINE_LETTER,140u,2u);
 assert(letter_a!=letter_b);
 assert(color(181,105)==HD_PAPER);
 /* The filed form can acquire an approval stamp on the later page. */
 unsigned stamp_x=163u,stamp_y=89u;
 (void)film(&s,CINE_BORE,140u,0u);
 unsigned before=color((int)stamp_x,(int)stamp_y);
 (void)film(&s,CINE_BORE,140u,2u);
 unsigned after=color((int)stamp_x,(int)stamp_y);
 assert(before!=after);
 /* The same cafe is distinct from the archival city street, and
    the actors continue moving until their table blocking is finished. */
 uint32_t cafe0=film(&s,CINE_MARA,35u,0u);
 uint32_t cafe1=film(&s,CINE_MARA,140u,2u);
 assert(cafe0!=cafe1 && cafe1!=letter_b);
 /* The bus moves, stops and only then opens its folding door. */
 uint32_t arriving=film(&s,CINE_BUS,80u,0u);
 uint32_t boarding=film(&s,CINE_BUS,160u,2u);
 assert(arriving!=boarding);
 /* Rooftop paint is progressive while the physical easel stays fixed. */
 uint32_t early=film(&s,CINE_ART,65u,0u);
 uint32_t finished=film(&s,CINE_ART,295u,2u);
 assert(early!=finished);
 /* Unresolved disappearance must never summon Mara into the clinic. */
 s.rescue_stage=STORY_RESCUE_NONE;
 uint32_t attended=film(&s,CINE_LAST_CLINIC,170u,2u);
 s.rescue_stage=STORY_RESCUE_MISSING;
 uint32_t missing=film(&s,CINE_LAST_CLINIC,170u,2u);
 assert(attended!=missing);
 s.rescue_stage=STORY_RESCUE_LOST;
 assert(missing==film(&s,CINE_LAST_CLINIC,170u,2u));
 s.rescue_stage=STORY_RESCUE_SAVED;
 assert(attended==film(&s,CINE_LAST_CLINIC,170u,2u));
 /* Close-up reaction beats use real pixel-painted characters, not
    a palette swap, a face overlay or a duplicated sprite in the scene. */
 uint32_t letter_reaction=film(&s,CINE_LETTER,174u,3u);
 assert(letter_reaction!=film(&s,CINE_LETTER,174u,2u));
 uint32_t mara_close=film(&s,CINE_MARA,177u,3u);
 assert(mara_close!=film(&s,CINE_MARA,177u,2u));
 uint32_t bus_close=film(&s,CINE_BUS,195u,3u);
 assert(bus_close!=film(&s,CINE_BUS,195u,2u));
 uint32_t art_close=film(&s,CINE_ART,227u,3u);
 assert(art_close!=film(&s,CINE_ART,227u,2u));
 uint32_t doctor_close=film(&s,CINE_LAST_CLINIC,195u,3u);
 assert(doctor_close!=film(&s,CINE_LAST_CLINIC,195u,2u));
 /* Every role's distinct portrait must stay bounded in the exact same
    guarded 240x160 buffer, with no new RAM backing. */
 uint32_t portraits[5];
 for(int role=1;role<=5;role++){
  memset(&sample,0xA5,sizeof sample);
  hd_rect(sample.pixels,0,0,HD_W,HD_H,HD_STONE_DARK);
  hd_actor_closeup(sample.pixels,120,143,role,171u,7);
  uint32_t h=2166136261u;
  for(unsigned k=0;k<HD_BYTES;k++){
   assert(sample.pixels[k]<64u);
   h=(h^sample.pixels[k])*16777619u;
  }
  for(unsigned g=0;g<32u;g++)
   assert(sample.before[g]==0xA5&&sample.after[g]==0xA5);
  portraits[role-1]=h;
  for(int prior=1;prior<role;prior++)
   assert(portraits[role-1]!=portraits[prior-1]);
 }
 /* Reunion opening and exit are physically distinct framings. */
 uint32_t door_early=film(&s,CINE_RESCUE_SAVED,80u,1u);
 uint32_t door_late=film(&s,CINE_RESCUE_SAVED,240u,2u);
 assert(door_early!=door_late);
 puts("cinematic staging: letter, ink stamp, Mara cafe, bus, easel and");
 puts("rescue-aware clinic, 240x160 palette, frame bounds and reunion OK");
 return 0;
}
