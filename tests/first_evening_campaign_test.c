#include "story.h"
#include "story_save.h"
#include "story_draw.h"
#include "hd_canvas.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
enum {A=1,B=2,RIGHT=16};
static void press(Story *s,unsigned k){
 story_tick(s,0);story_tick(s,(uint16_t)k);story_tick(s,0);
}
static void done(Story *s){
 for(unsigned i=0;i<45u&&s->dialogue;i++)press(s,A);
 assert(!s->dialogue);
}
static void stand(Story *s,int x,int y,int f){
 assert(!story_solid(x,y));s->x=(uint8_t)x;s->y=(uint8_t)y;
 s->prev_x=s->x;s->prev_y=s->y;s->facing=(uint8_t)f;s->cooldown=0;
}
static void first(const Story *s,const char *line){
 int n=0;const StoryPage *p=story_pages(s,&n);
 assert(p&&n>0&&!strcmp(p[0].line1,line));
}
static void core(Story *s){
 story_new(s,0);done(s);
 stand(s,5,5,2);press(s,A);done(s);assert(s->flags&STORY_HOME);
 stand(s,11,8,3);press(s,A);done(s);assert(s->flags&STORY_STAMP);
 stand(s,8,12,3);press(s,A);press(s,B);assert(!(s->flags&STORY_MARA));
 press(s,A);done(s);assert((s->flags&STORY_ALL)==STORY_ALL);
}
static void open_bus(Story *s){
 stand(s,19,6,3);press(s,A);first(s,"Your stamp checks out.");
 done(s);assert(s->choosing);
}
static struct {uint8_t left[32],p[HD_BYTES],right[32];} frame;
static unsigned render(const Story *s,unsigned scene,unsigned phase,unsigned style,int overlay){
 Story copy=*s;copy.cinema=(uint8_t)scene;copy.page=0;
 copy.project_style=(uint8_t)style;
 if(scene==CINE_ART)copy.art_steps=(uint8_t)phase;
 else copy.song_steps=(uint8_t)phase;
 memset(&frame,0xC3,sizeof frame);
 hd_cinema_draw(frame.p,&copy,80u);
 if(overlay)hd_game_overlay(frame.p,&copy);
 for(unsigned i=0;i<32;i++)assert(frame.left[i]==0xC3&&frame.right[i]==0xC3);
 for(unsigned i=0;i<HD_BYTES;i++)assert(frame.p[i]<64u);
 /* Compare authored pixels at the same camera time, rather than relying
    on changing actor animation or a time-dependent full-frame hash. */
 unsigned hash=2166136261u;
 int x0=scene==CINE_ART?151:134;
 int x1=scene==CINE_ART?218:226;
 int y0=scene==CINE_ART?52:35;
 int y1=scene==CINE_ART?102:49;
 if(overlay){x0=150;x1=220;y0=78;y1=88;}
 for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++)
  hash=(hash^frame.p[y*HD_W+x])*16777619u;
 return hash;
}
int main(void){
 Story s;core(&s);
 unsigned bond=s.bond,energy=s.energy;
 press(&s,A);assert(s.activity=='f'&&s.cinema==CINE_MARA);
 press(&s,RIGHT);press(&s,A|B);
 assert(!s.activity&&!s.activity_choice&&s.bond==bond&&s.energy==energy);
 assert(!(s.daily_mask&STORY_PREBUS_MARA)&&!s.daily_memories);
 press(&s,A);press(&s,A);
 assert(s.daily_mask&STORY_PREBUS_MARA);
 assert(s.bond==bond+2u&&s.energy==energy-1u);
 assert(s.life_stage==STORY_LIFE_NONE&&!s.daily_memories);
 first(&s,"Let's take the long");done(&s);
 press(&s,A);assert(!s.activity&&s.dialogue);done(&s);
 assert(s.bond==bond+2u); /* repeat talk cannot farm friendship */
 /* The alternative is genuinely different, and no day is consumed. */
 Story alt;core(&alt);press(&alt,A);press(&alt,RIGHT);press(&alt,A);
 first(&alt,"You can be honest.");assert(alt.bond==bond+1u);
 assert(alt.energy==energy+1u&&alt.day_frame==0);
 done(&alt);
 /* A+B on a bus prompt must not accidentally commit a finale. */
 open_bus(&s);press(&s,A|B);
 assert(!s.choosing&&!s.life_stage&&!s.ended);
 open_bus(&s);press(&s,RIGHT);press(&s,A);
 assert(!s.ended&&!s.life_stage&&s.cinema==CINE_BUS);
 first(&s,"No rush to board.");done(&s);
 /* Initial archive copy remains a legacy flag, not a forced four-stage
    grind. Every additional session is accessible before day one. */
 stand(&s,19,8,3);press(&s,A);assert(s.flags&STORY_MUSIC);
 done(&s);assert(!s.song_steps);
 press(&s,A);assert(s.activity=='m'&&s.cinema==CINE_MUSIC);
 press(&s,RIGHT);press(&s,B);assert(!s.song_steps&&!s.activity);
 for(unsigned stage=1;stage<=4;stage++){
  press(&s,A);assert(s.activity=='m'&&s.cinema==CINE_MUSIC);
  if(stage==1)press(&s,RIGHT);
  press(&s,A);assert(s.song_steps==stage&&!s.activity);
  if(stage==1)first(&s,"Here is the melody.");
  assert(s.life_stage==STORY_LIFE_NONE);
  done(&s);
 }
 assert(s.project_style&STORY_STYLE_SONG_SOFT);
 press(&s,A);assert(s.activity=='E');
 press(&s,RIGHT);press(&s,A);assert(s.activity==STORY_ACT_STUDIO);
 press(&s,B);assert(!s.activity);
 stand(&s,11,8,3);press(&s,A);first(&s,"This recording");done(&s);
 stand(&s,8,12,3);press(&s,A);first(&s,"You finished our song.");done(&s);
 stand(&s,14,7,3);press(&s,A);assert(s.flags&STORY_ART);done(&s);
 for(unsigned stage=1;stage<=4;stage++){
  press(&s,A);assert(s.activity=='r'&&s.cinema==CINE_ART);
  if(stage==1)press(&s,RIGHT);
  press(&s,A);assert(s.art_steps==stage&&!s.activity);
  if(stage==1)first(&s,"Start with the sky.");
  done(&s);
 }
 assert(s.project_style&STORY_STYLE_ART_WARM);
 stand(&s,8,12,3);press(&s,A);first(&s,"The tape and painting.");done(&s);
 assert(s.flags==STORY_FULL&&!s.daily_memories&&s.day_frame==0);
 /* SRAM codec and the exact loader's restore function preserve both
    previously lost shift state and the new single-event marker. */
 uint8_t raw[STORY_SAVE_BYTES],decoded[STORY_SAVE_BYTES];
 s.daily_mask|=STORY_DAY_SHIFT;
 story_save_encode(&s,raw);assert(story_save_decode(raw,decoded));
 assert(decoded[27]==4u&&decoded[28]==4u&&decoded[29]==s.project_style);
 assert(story_restore_daily_mask(decoded[21],STORY_LIFE_NONE)==s.daily_mask);
 assert(story_restore_daily_mask(decoded[21],STORY_LIFE_WAIT)==STORY_DAY_SHIFT);
 /* New-day route still starts without the first-evening memory marker. */
 Story neutral;core(&neutral);
 press(&neutral,A);press(&neutral,A);done(&neutral);
 open_bus(&neutral);press(&neutral,A);
 assert(neutral.life_stage==STORY_LIFE_WAIT&&!neutral.ended);
 assert(!(neutral.daily_mask&STORY_PREBUS_MARA));done(&neutral);
 stand(&neutral,14,7,3);press(&neutral,A);assert(neutral.activity=='R');
 press(&neutral,RIGHT);press(&neutral,A);done(&neutral);
 assert(!neutral.art_steps&&!(neutral.flags&STORY_ART)); /* REST is not paint */
 press(&neutral,A);assert(neutral.activity=='r');press(&neutral,A);done(&neutral);
 assert(neutral.art_steps==1&&(neutral.flags&STORY_ART));
 /* The clinic cannot name an absent Mara as though she were present. */
 neutral.rescue_stage=STORY_RESCUE_LOST;
 neutral.life_stage=STORY_LIFE_FINAL_CLINIC;
 stand(&neutral,15,5,0);press(&neutral,A);
 int n=0;const StoryPage *pg=story_pages(&neutral,&n);
 assert(n==4&&!strcmp(pg[2].speaker,"DANNY"));
 assert(!strcmp(pg[2].line1,"I wish Mara were here."));
 /* Native pixels must distinguish all five saved stages, and styles. */
 Story art=s;art.activity=0;art.dialogue=0;
 unsigned pictures[5],takes[5],meters[5];
 for(unsigned i=0;i<=4;i++){
  pictures[i]=render(&art,CINE_ART,i,0,0);
  takes[i]=render(&art,CINE_MUSIC,i,0,0);
  art.activity='r';meters[i]=render(&art,CINE_ART,i,0,1);
  art.activity=0;
  for(unsigned j=0;j<i;j++){
   assert(pictures[i]!=pictures[j]&&takes[i]!=takes[j]);
   assert(meters[i]!=meters[j]);
  }
 }
 assert(render(&art,CINE_ART,4,STORY_STYLE_ART_WARM,0)!=pictures[4]);
 assert(render(&art,CINE_MUSIC,4,STORY_STYLE_SONG_SOFT,0)!=takes[4]);
 /* Returning to the bus with both legacy flags still boards. */
 open_bus(&s);press(&s,A);assert(s.ended&&s.ending==STORY_END_GOOD);
 puts("first evening: both Mara choices, bounded friendship, early 2x4 work, real studio, safe cancel, shift SRAM, reactive scenes and native stage art passed");
 return 0;
}
