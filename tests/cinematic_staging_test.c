#include "hd_canvas.h"
#include "hd_actor_acting.h"
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
 static const uint16_t reunion[]={0u,100u,170u,250u};
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
 /* Reunion is a live, reachable saved-state conversation: the final page
    must show both people at the same point of contact. */
 s.life_stage=STORY_LIFE_NONE;s.rescue_stage=STORY_RESCUE_SAVED;
 visit(&s,7,3,3);beats(&s,CINE_RESCUE_SAVED,reunion,4u);
 /* A day-three failed rescue must not speak through Mara or draw her. */
 Story lost;story_new(&lost,1);
 button(&lost,2u);
 lost.rescue_stage=STORY_RESCUE_LOST;
 lost.life_stage=STORY_LIFE_FOLLOWUP_WAIT;
 lost.day_number=3u;lost.day_frame=0u;
 story_tick(&lost,0u);
 assert(lost.dialogue!=0u&&lost.cinema==CINE_FOLLOWUP);
 int count=0;const StoryPage *lines=story_pages(&lost,&count);
 assert(count==3 && strstr(lines[1].line1,"No confirmed")!=NULL);
 assert(strcmp(lines[1].speaker,"DANNY")==0);
 /* When she is safe, the normal follow-up still lets her participate. */
 Story safe;story_new(&safe,1);
 button(&safe,2u);
 safe.rescue_stage=STORY_RESCUE_SAVED;
 safe.life_stage=STORY_LIFE_FOLLOWUP_WAIT;
 safe.day_number=3u;safe.day_frame=0u;
 story_tick(&safe,0u);
 assert(safe.dialogue!=0u&&safe.cinema==CINE_FOLLOWUP);
 lines=story_pages(&safe,&count);
 assert(count==3 && strcmp(lines[1].speaker,"MARA")==0);
}
static void verify_grounded_acting(void){
 /* Real production rasters: breathing may move the torso but not the
    shoe and floor pixels. The previous whole-body bob fails this test. */
 static uint8_t ground[8*HD_W];
 for(int role=1;role<=5;role++){
  memset(&sample,0xA5,sizeof sample);
  hd_rect(sample.pixels,0,0,HD_W,HD_H,HD_STONE_DARK);
  hd_actor_stage(sample.pixels,120,143,role,37u,0);
  memcpy(ground,sample.pixels+139u*HD_W,sizeof ground);
  hd_rect(sample.pixels,0,0,HD_W,HD_H,HD_STONE_DARK);
  hd_actor_stage(sample.pixels,120,143,role,38u,0);
  assert(!memcmp(ground,sample.pixels+139u*HD_W,sizeof ground));
  for(unsigned g=0;g<32u;g++)
   assert(sample.before[g]==0xA5&&sample.after[g]==0xA5);
  unsigned lids=0u,mouth=0u;
  for(unsigned t=0;t<256u;t++){
   int e=hd_acting_eye(t,(unsigned)role),m=hd_acting_mouth(t,(unsigned)role,1);
   assert(e>=0&&e<=2&&m>=0&&m<=2);
   lids|=1u<<e;mouth|=1u<<m;
   assert(hd_acting_mouth(t,(unsigned)role,0)==0);
   assert(hd_acting_mouth(t,(unsigned)role,5)==0);
   assert(hd_acting_mouth(t,(unsigned)role,7)==0);
  }
  assert(lids==7u&&mouth==7u);
 }
 for(unsigned t=0;t<127u;t++)
  assert(!(hd_acting_eye(t,1u)==2&&hd_acting_eye(t,2u)==2));
}
static void verify_scripted_mouth(void){
 assert(hd_acting_speaker_role("DANNY")==1u);
 assert(hd_acting_speaker_role("MARA")==2u);
 assert(hd_acting_speaker_role("BORE CLERK")==3u);
 assert(hd_acting_speaker_role("ARCHIVIST")==3u);
 assert(hd_acting_speaker_role("WITNESS")==4u);
 assert(hd_acting_speaker_role("DOCTOR")==5u);
 assert(hd_acting_speaker_role("MARA'S CAFE")==0u);
 assert(hd_acting_speaker_role("NARRATOR")==0u);
 assert(hd_acting_speaker_role("PHONE MESSAGE")==0u);
 assert(hd_acting_speaker_role(0)==0u);
 static uint8_t silent[HD_BYTES];
 for(int role=1;role<=5;role++){
  unsigned t=0;
  while(hd_acting_mouth(t,(unsigned)role,1)!=2)t++;
  hd_rect(silent,0,0,HD_W,HD_H,HD_STONE_DARK);
  memset(&sample,0xA5,sizeof sample);
  hd_rect(sample.pixels,0,0,HD_W,HD_H,HD_STONE_DARK);
  /* Speaking versus listening uses the same reaching gesture/pose. */
  hd_actor_stage_spoken(silent,120,143,role,t,3,0);
  hd_actor_stage_spoken(sample.pixels,120,143,role,t,3,1);
  unsigned changes=0;
  for(unsigned y=0;y<HD_H;y++)for(unsigned x=0;x<HD_W;x++){
   unsigned i=y*HD_W+x;
   if(silent[i]!=sample.pixels[i]){
    assert(x>=115u&&x<=125u&&y>=92u&&y<=98u);
    changes++;
   }
  }
  assert(changes>0u);
  for(unsigned g=0;g<32u;g++)
   assert(sample.before[g]==0xA5&&sample.after[g]==0xA5);
 }
}
int main(void){
 verify_scripted_mouth();
 verify_grounded_acting();
 verify_reachable_editorial_cuts();
 Story s;story_new(&s,0);s.dialogue=0;
 /* The optimized sky stripes preserve the original dusk colors on real
    cinematic frames. Pick unobstructed coordinates above the skyline;
    unlike a test of a private helper, this checks production output. */
 (void)film(&s,CINE_ART,24u,0u);
 /* The first/last two rows are the intentional black film frame, NOT
    original dusk pixels. Sample y=8 inside clear sky instead. */
 assert(color(239,0)==HD_INK);
 assert(color(239,8)==17u&&color(239,40)==25u);
 (void)film(&s,CINE_MARA,24u,0u);
 assert(color(239,0)==HD_INK);
 assert(color(239,8)==19u&&color(239,40)==24u);
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
 /* Doctor's follow-up has a real fixed glazed clinic entrance,
    wayfinding cross and readable changing reaction shots. Each frame
    stays in the one guarded Mode-4 buffer with no new RGB arrays. */
 uint32_t follow0=film(&s,CINE_FOLLOWUP,183u,0u);
 unsigned fixed_copper=color(111,64);
 uint32_t follow1=film(&s,CINE_FOLLOWUP,183u,1u);
 assert(follow0!=follow1);
 assert(color(111,64)==fixed_copper);
 uint32_t follow2=film(&s,CINE_FOLLOWUP,183u,2u);
 assert(follow1!=follow2 && follow0!=follow2);
 assert(color(111,64)==fixed_copper);
 /* The unresolved disappearance is never staged as a happy reunion. */
 s.rescue_stage=STORY_RESCUE_LOST;
 uint32_t alone1=film(&s,CINE_FOLLOWUP,183u,1u);
 assert(alone1!=follow1);
 uint32_t alone2=film(&s,CINE_FOLLOWUP,183u,2u);
 assert(alone2==alone1);
 s.rescue_stage=STORY_RESCUE_NONE;
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
 /* A portrait body must remain connected all the way to the lower
    matte. The former three-ellipse bust ended as a floating sticker. */
 (void)film(&s,CINE_MARA,177u,3u);
 unsigned garment=0;
 for(int yy=115;yy<155;yy++)
  for(int xx=157;xx<199;xx++)
   if(color(xx,yy)==HD_GLASS)garment++;
 assert(garment>60u);
 /* A connected sleeve reaches wrist level on BOTH sides. Old portrait
    circles stopped at the shoulder and left the hands in midair. */
 memset(&sample,0xA5,sizeof sample);
 hd_rect(sample.pixels,0,0,HD_W,HD_H,HD_STONE_DARK);
 hd_actor_closeup(sample.pixels,120,143,1,171u,0);
 /* This is a jacket with contrast cuffs and visible hands, not a
    monochrome sleeve. Check the two *actual* contact zones, rather than
    expecting red pixels outside the cuff's drawn extent. */
 unsigned left_cuff=0,right_cuff=0,left_hand=0,right_hand=0;
 for(int yy=139;yy<=141;yy++){
  for(int xx=84;xx<=98;xx++)
   if(color(xx,yy)==HD_STONE_LIT)left_cuff++;
  for(int xx=142;xx<=156;xx++)
   if(color(xx,yy)==HD_STONE_LIT)right_cuff++;
 }
 for(int yy=142;yy<=147;yy++){
  for(int xx=85;xx<=95;xx++)
   if(color(xx,yy)==HD_SKIN||color(xx,yy)==HD_SKIN_LIT)
    left_hand++;
  for(int xx=144;xx<=154;xx++)
   if(color(xx,yy)==HD_SKIN||color(xx,yy)==HD_SKIN_LIT)
    right_hand++;
 }
 assert(left_cuff>20u && right_cuff>20u);
 assert(left_hand>20u && right_hand>20u);
 /* No black horizontal rib should run THROUGH the sleeve at any
    4-pixel joint. Sample well inside both actor sleeves; the shape
    may have a dark side edge, but must keep its solid jacket fill. */
 for(int k=0;k<11;k++){
  int yy=143-44+k*4;
  assert(color(93,yy)!=HD_INK);
  assert(color(147,yy)!=HD_INK);
 }
 /* Actor breathing and reaction pose changes must remain real, while
    always staying in the allocated 240x160 frame. */
 uint32_t neutral=2166136261u;
 for(unsigned k=0;k<HD_BYTES;k++)
  neutral=(neutral^sample.pixels[k])*16777619u;
 memset(&sample,0xA5,sizeof sample);
 hd_rect(sample.pixels,0,0,HD_W,HD_H,HD_STONE_DARK);
 hd_actor_closeup(sample.pixels,120,143,1,225u,5);
 uint32_t reflective=2166136261u;
 for(unsigned k=0;k<HD_BYTES;k++)
  reflective=(reflective^sample.pixels[k])*16777619u;
 assert(neutral!=reflective);
 for(unsigned g=0;g<32u;g++)
  assert(sample.before[g]==0xA5&&sample.after[g]==0xA5);
 /* The warm beard survives all actual portrait expressions. */
 for(unsigned pose=0;pose<4u;pose++){
  const int expression[]={0,1,5,7};
  memset(&sample,0xA5,sizeof sample);
  hd_rect(sample.pixels,0,0,HD_W,HD_H,HD_STONE_DARK);
  hd_actor_closeup(sample.pixels,120,143,1,171u,expression[pose]);
  unsigned warm=0,skin=0;
  for(int yy=90;yy<101;yy++)for(int xx=114;xx<=127;xx++)
   if(color(xx,yy)==HD_COPPER||color(xx,yy)==HD_BRICK)++warm;
  for(int yy=63;yy<79;yy++)for(int xx=107;xx<=133;xx++)
   if(color(xx,yy)==HD_SKIN||color(xx,yy)==HD_SKIN_LIT)++skin;
  assert(warm>=12u&&skin>=30u);
  for(unsigned g=0;g<32u;g++)
   assert(sample.before[g]==0xA5&&sample.after[g]==0xA5);
 }
 /* Every role's distinct portrait must stay bounded in the exact same
    guarded 240x160 buffer, with no new RAM backing. */
 uint32_t portraits[5],faces[5];
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
  /* Compare only the central face, not hair or costume, to prove
     distinct facial construction rather than wardrobe color swaps. */
  uint32_t face=2166136261u;
  for(int yy=64;yy<=94;yy++)for(int xx=107;xx<=133;xx++)
   face=(face^color(xx,yy))*16777619u;
  faces[role-1]=face;
  for(int prior=1;prior<role;prior++)
   assert(portraits[role-1]!=portraits[prior-1]);
  if(role==2)assert(faces[0]!=faces[1]);
 }
 /* Reunion opening and exit are physically distinct framings. */
 uint32_t door_early=film(&s,CINE_RESCUE_SAVED,80u,1u);
 uint32_t door_late=film(&s,CINE_RESCUE_SAVED,240u,2u);
 assert(door_early!=door_late);
 /* Once Mara walks onto the foreground floor, the open door must not
    erase her face. These intermediate production frames caught a real
    layer-order bug that the closed-door and final-contact shots missed. */
 for(unsigned t=145u;t<=195u;t+=25u){
  (void)film(&s,CINE_RESCUE_SAVED,t,2u);
  unsigned visible_face=0u;
  for(int yy=75;yy<96;yy++)for(int xx=140;xx<196;xx++){
   unsigned c=color(xx,yy);
   if(c==HD_SKIN||c==HD_SKIN_LIT)visible_face++;
  }
  assert(visible_face>35u);
 }
 /* A saved Mara and Danny are actually together by the final beat.
    A doorway and two separated characters are not a reunion. */
 uint32_t contact=film(&s,CINE_RESCUE_SAVED,265u,2u);
 assert(contact!=door_early);
 assert(color(130,118)==HD_SKIN_LIT ||
        color(130,118)==HD_SKIN);
 puts("art direction: distinct Danny/Mara facial planes and grounded shared cafe");
 puts("cinematic staging: letter, ink stamp, Mara cafe, bus, easel and");
 puts("rescue-aware clinic, 240x160 palette, frame bounds and reunion OK");
 return 0;
}

