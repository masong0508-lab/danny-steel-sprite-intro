#include "hd_canvas.h"
#include "cinema.h"

/* A native-resolution score-length film. All scene/shot boundaries come
   from cinema_ending_scene/shots in the EXISTING music director. The film
   shares the same here-today score, rescue condition, and saved state.
   New art never selects or advances an ending. */
static unsigned ending_type(const Story *s){
 if(s->ended&&s->ending==STORY_END_GOOD)return STORY_END_GOOD;
 if(s->ended&&s->ending==STORY_END_BAD)return STORY_END_BAD;
 return STORY_END_NEUTRAL;
}
static unsigned page_for_shot(unsigned ending,unsigned shot){
 static const uint8_t good[]={1,1,2,2,3,4,3,5,3,6,7,3};
 static const uint8_t neutral[]={0,0,1,2,1,0,6,7,8,9,10};
 static const uint8_t bad[]={0,1,2,1,1,0,1,1,0,1,0};
 if(ending==STORY_END_GOOD)return good[shot<12u?shot:11u];
 if(ending==STORY_END_BAD)return bad[shot<11u?shot:10u];
 return neutral[shot<11u?shot:10u];
}
static void film_text(const Story *s,unsigned ending,unsigned f,
                      const char **first,const char **second){
 const char *a=0,*b=0;
 unsigned harm=(unsigned)story_bad_count(s);
 unsigned music=s->song_steps>=STORY_PROJECT_DONE;
 if(ending==STORY_END_GOOD){
  if(f<530u){
   a=harm?"HE STILL OWED AN APOLOGY":"DANNY MADE ONE LAST PROMISE";
   b=harm?"HE DID NOT GET A CLEAN SLATE":"TO FINISH WHAT HE BEGAN";
  }else if(f<1080u){
   a="HE LEFT SOME SONGS BEHIND";
   b=harm?"NOT EVERY RECORD WAS HONEST":"AND THE BORE DEMOS TOO";
  }else if(f<1560u){
   a="MARA REMEMBERED HIS LAUGH";b="EVEN ON QUIET MORNINGS";
  }else if(f<2050u){
   a="HIS NAME WAS FILED AWAY";
   b=harm?"HIS ACTIONS WERE FILED TOO":"BUT NOT HIS MUSIC";
  }else if(f<2600u){
   a="AFTER DANNY WAS GONE";b="SOMEONE FOUND THE RECORDINGS";
  }else if(f<3200u){
   a="THE SONGS FOUND LISTENERS";b="FAR BEYOND THE OLD QUARTER";
  }else if(f<3730u){
   a="MILLIONS OF RECORDS SOLD";b="THE BORE GAMES FOUND FANS";
  }else if(f<4350u){
   a="HE NEVER SAW THEIR SUCCESS";
   b=harm?"MARA TOLD THE WHOLE STORY":"BUT PEOPLE HEARD HIS VOICE";
  }else if(f<4890u){
   a="ONE MORE SONG FOR DANNY";b="CLOSER TO THE END";
  }else if(f<5140u){
   a="THE CITY KEPT PLAYING";b="THE MUSIC KEPT TRAVELING";
  }else{
   a="THE LAST DAYS OF DANNY STEEL";b="GOOD END / A LEGACY";
  }
 }else if(ending==STORY_END_BAD){
  if(s->rescue_stage==STORY_RESCUE_AFTERSHOCK){
   if(f<780u){a="MARA GOT HOME SAFELY";b="THE SECOND CHANCE WAS REAL";}
   else if(f<1650u){a="ONE MORE BAD CHOICE";b="AND TRUST FELL APART";}
   else if(f<2490u){a="THE CITY REMEMBERED";b="THE PROMISES HE BROKE";}
   else if(f<3670u){a="DANNY LOST HIS WAY";b="BEFORE HIS LAST DAYS ENDED";}
   else{a="THE LAST DAYS OF DANNY STEEL";b="BAD END / MISUSED CHANCE";}
  }else{
   if(f<780u){a="HE KEPT THE PHONE NEAR";b="SHE WAS SUPPOSED TO CALL";}
   else if(f<1650u){a="A DAY PASSED WITH NO WORD";b="STILL NO VERIFIED RECOVERY";}
   else if(f<2490u){a="THE DOOR STOOD CLOSED";b="HER FATE WAS UNKNOWN";}
   else if(f<3670u){a="DANNY HAD NO ANSWERS";b="ONLY THE DAYS HE LOST";}
   else{a="THE LAST DAYS OF DANNY STEEL";b="BAD END / UNANSWERED";}
  }
 }else{
  if(f<370u){a="ONE LAST LINE OF PAINT";b="HIS HAND REACHED THE CANVAS";}
  else if(f<810u){a="HIS LAST WORK WAS QUIET";b="THE BRUSH STAYED THERE";}
  else if(f<1130u){a="BEFORE / A LETTER FROM HOME";b="SOMEBODY HAD WAITED";}
  else if(f<1590u){a="BEFORE / MARA AND DANNY";b="LAUGHED AT THE BAD JOKES";}
  else if(f<2010u){a="BEFORE / THE BORE OFFICE";b="CERTIFIED THAT HE EXISTED";}
  else if(f<2490u){
   a="BEFORE / THE MUSIC STUDIO";
   b=music?"A FINISHED MASTER TAPE":"AN UNFINISHED SONG";
  }else if(f<2870u){a="MARA RETURNED TO THE ROOF";b="THE PAINTING REMAINED";}
  else if(f<3250u){a="SHE FOUND A FOLDED NOTE";b="BESIDE THE PAINT SUPPLIES";}
  else if(f<3650u){a="SOME WORDS WERE LEFT UNSAID";b="BUT THE LETTER REMAINED";}
  else if(f<4050u){a="SHE SAW HIS COLORFUL CITY";b="JUST AS HE HAD PAINTED IT";}
  else{a="THE LAST DAYS OF DANNY STEEL";b="NEUTRAL END / THE CANVAS";}
 }
 *first=a;*second=b;
}
static void caption(uint8_t *p,const Story *s,unsigned ending,unsigned f){
 int page_count=0;
 const StoryPage *pages=s->dialogue?story_pages(s,&page_count):0;
 if(pages && s->page<(unsigned)page_count){
  const StoryPage *d=&pages[s->page];
  hd_rect(p,0,121,240,39,HD_SHADOW);
  hd_rect(p,0,121,240,2,HD_GOLD);
  hd_text(p,10,125,d->speaker,HD_GOLD,1,1);
  hd_text(p,10,136,d->line1,HD_PAPER,1,1);
  hd_text(p,10,146,d->line2,HD_PAPER,1,1);
  hd_text(p,185,125,"A NEXT",HD_STONE_LIT,1,0);
  return;
 }
 unsigned end_at=ending==STORY_END_GOOD?
                 STORY_GOOD_FINAL_FRAMES:STORY_END_SCORE_FRAMES;
 if(f>=end_at){
  hd_rect(p,0,136,240,24,HD_INK);
  hd_rect(p,0,136,240,2,HD_COPPER);
  hd_text(p,10,143,"START / RETURN TO MENU",HD_PAPER,1,1);
  return;
 }
 const char *a,*b;
 film_text(s,ending,f,&a,&b);
 hd_rect(p,0,123,240,37,HD_INK);
 hd_rect(p,0,123,240,2,HD_COPPER);
 hd_rect(p,5,129,2,23,HD_GOLD);
 hd_text(p,14,131,a,HD_PAPER,1,1);
 hd_text(p,14,144,b,HD_LAMP,1,1);
}
void hd_ending_draw(uint8_t *p,const Story *s,unsigned frame){
 unsigned ending=ending_type(s);
 unsigned safe=s->rescue_stage==STORY_RESCUE_SAVED ||
               s->rescue_stage==STORY_RESCUE_AFTERSHOCK;
 unsigned shot=cinema_ending_shot(ending,frame);
 unsigned scene=cinema_ending_scene(ending,frame,safe);
 Story film=*s;
 film.cinema=(uint8_t)scene;
 film.page=(uint8_t)page_for_shot(ending,shot);
 /* Include the original frame clock: vinyl, water, streetlight and actor
    gestures never reset when the musical storyboard cuts to a new shot. */
 hd_cinema_draw(p,&film,frame);
 /* Across all endings, preserve distinct scene locations and the same
    non-graphic narrative. Simple film-gate fades serve musical cuts. */
 unsigned start=0;
 if(shot>0u){
  unsigned lo=0u,hi=frame;
  for(int i=0;i<16;i++){
   unsigned m=lo+(hi-lo)/2u;
   if(cinema_ending_shot(ending,m)<shot)lo=m+1u;
   else hi=m;
  }
  start=hi;
 }
 unsigned local=frame>start?frame-start:0u;
 if(shot>0u && local<12u){
  int border=(int)((12u-local)*14u/12u);
  hd_rect(p,0,0,240,border,HD_INK);
  hd_rect(p,0,160-border,240,border,HD_INK);
 }
 /* The dialogue or timecoded ending captions are painted LAST, so the
    film never accidentally obscures the story text with moving scenery. */
 caption(p,s,ending,frame);
}
