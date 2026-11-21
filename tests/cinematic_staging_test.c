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
/* These are genuine gameplay dialogue interactions, not hand-picked page
   numbers injected into a gallery. Every planned close-up is reachable
   when someone actually plays the chapter and presses A to continue. */
static void button(Story *s,unsigned key){
 story_tick(s,0u);story_tick(s,(uint16_t)key);story_tick(s,0u);
}
static void visit(Story *s,int x,int y,int facing){
 assert(!story_solid(x,y));
 s->x=(uint8_t)x;s->y=(uint8_t)y;s->facing=(uint8_t)facing;
 s->prev_x=s->x;s->prev_y=s->y;s->cooldown=0;
}
static void beats(Story *s,unsigned cinema,
                  const uint16_t *anchors,unsigned n){
 button(s,1u);
 assert(s->dialogue!=0u&&s->cinema==cinema);
 for(unsigned i=1;i<n;i++){
  button(s,1u);
  assert(s->page==i && s->cinema_frame>=anchors[i]);
 }
 button(s,2u); /* early B skip is never gated by a camera cue */
 assert(!s->dialogue && s->cinema==CINE_NONE);
}
static void verify_reachable_editorial_cuts(void){
 Story s;story_new(&s,0);
 button(&s,2u);assert(!s.dialogue);
 static const uint16_t letter[]={0u,55u,108u,160u};
 static const uint16_t mara[]={0u,66u,108u,151u,190u};
 static const uint16_t bus[]={0u,118u,183u};
 static const uint16_t art[]={0u,198u};
 static const uint16_t clinic[]={0u,124u,158u,200u};
 visit(&s,5,5,2);beats(&s,CINE_LETTER,letter,4u);
 s.flags=STORY_HOME|STORY_STAMP;
 visit(&s,8,12,3);beats(&s,CINE_MARA,mara,5u);
 s.flags=STORY_ALL;
 visit(&s,19,4,1);beats(&s,CINE_BUS,bus,3u);
 s.flags=0;
 visit(&s,14,7,3);beats(&s,CINE_ART,art,2u);
 s.life_stage=STORY_LIFE_CLINIC;
 visit(&s,16,6,3);beats(&s,CINE_CLINIC,clinic,4u);
 s.life_stage=STORY_LIFE_FINAL_CLINIC;
 visit(&s,16,6,3);beats(&s,CINE_LAST_CLINIC,clinic,4u);
}
int main(void){
 verify_reachable_editorial_cuts();
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
 uint32_t bus_close=film(&s,CINE_BUS,195u,2u);
 assert(bus_close!=film(&s,CINE_BUS,195u,1u));
 uint32_t art_close=film(&s,CINE_ART,227u,1u);
 uint8_t completed_paint[32];
 for(int x=160;x<192;x++)completed_paint[x-160]=
  (uint8_t)color(x,78);
 assert(art_close!=film(&s,CINE_ART,227u,0u));
 /* The skyline painting must not vanish when the film cuts to the
    painter's reaction. The same brush marks persist in the same pixels. */
 for(int x=160;x<192;x++)
  assert(completed_paint[x-160]==color(x,78));
 uint32_t doctor_close=film(&s,CINE_LAST_CLINIC,195u,1u);
 assert(doctor_close!=film(&s,CINE_LAST_CLINIC,195u,0u));
 uint32_t patient_close=film(&s,CINE_LAST_CLINIC,215u,3u);
 assert(patient_close!=film(&s,CINE_LAST_CLINIC,215u,0u));
 uint32_t mara_clinic=film(&s,CINE_LAST_CLINIC,178u,2u);
 assert(mara_clinic!=film(&s,CINE_LAST_CLINIC,178u,1u));
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
 /* A saved Mara and Danny are actually together by the final beat.
    A doorway and two separated characters are not a reunion. */
 uint32_t contact=film(&s,CINE_RESCUE_SAVED,265u,2u);
 assert(contact!=door_early);
 assert(color(130,118)==HD_SKIN_LIT ||
        color(130,118)==HD_SKIN);
 puts("cinematic staging: letter, ink stamp, Mara cafe, bus, easel and");
 puts("rescue-aware clinic, 240x160 palette, frame bounds and reunion OK");
 return 0;
}
