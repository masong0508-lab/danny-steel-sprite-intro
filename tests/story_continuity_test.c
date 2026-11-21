#include "story.h"
#include "story_draw.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
enum { A=1,B=2,RIGHT=16 };
static void press(Story *s,unsigned key){
 story_tick(s,0);story_tick(s,(uint16_t)key);story_tick(s,0);
}
static void done(Story *s){
 for(int i=0;i<45&&s->dialogue;i++)press(s,A);
 assert(!s->dialogue);
}
static void stand(Story *s,unsigned x,unsigned y){
 assert(!story_solid((int)x,(int)y));
 s->x=(uint8_t)x;s->y=(uint8_t)y;s->prev_x=s->x;s->prev_y=s->y;
 s->facing=3;s->cooldown=0;
}
static void first(const Story *s,const char *line){
 int count=0;const StoryPage *p=story_pages(s,&count);
 assert(count>0&&p&&s->dialogue);
 assert(!strcmp(p[0].line1,line));
}
static void new_game(Story *s){
 story_new(s,0);done(s);
}
static void music_risk(Story *s){
 stand(s,19,8);press(s,B);
 assert(s->risk_menu=='M');
 press(s,RIGHT);assert(s->risk_choice);
 press(s,A);assert(s->dialogue&&s->bad_mask==STORY_BAD_MUSIC);
}
static void check_frame(Story *s,uint8_t *copy){
 struct {
  uint8_t left[24],pixels[120*80],right[24];
 } buf;
 memset(&buf,0xE1,sizeof(buf));
 story_draw(s,0,buf.pixels);
 for(int i=0;i<24;i++)assert(buf.left[i]==0xE1&&buf.right[i]==0xE1);
 for(int i=0;i<120*80;i++)assert(buf.pixels[i]<32u);
 memcpy(copy,buf.pixels,sizeof(buf.pixels));
}
static unsigned caption_diff(const uint8_t *a,const uint8_t *b){
 unsigned n=0;
 for(int y=62;y<80;y++)for(int x=0;x<120;x++)
  n+=(a[y*120+x]!=b[y*120+x]);
 return n;
}
int main(void){
 story_draw_init();
 Story s;new_game(&s);
 s.flags=STORY_HOME|STORY_STAMP;
 stand(&s,8,12);press(&s,A);
 first(&s,"You only joke like");
 assert(!(s.flags&STORY_MARA));
 /* Skipping a conversation cannot award an unheard promise. */
 press(&s,B);assert(!s.dialogue&&!(s.flags&STORY_MARA));
 stand(&s,8,12);press(&s,A);
 done(&s);assert(s.flags&STORY_MARA);
 /* Existing relationship continues even after an optional harmful act:
    the dialogue acknowledges it without blocking ordinary time. */
 s.bad_mask=STORY_BAD_LETTER;stand(&s,8,12);press(&s,A);
 first(&s,"You've been talking");done(&s);
 s.bad_mask=STORY_BAD_LETTER|STORY_BAD_CLERK;
 stand(&s,8,12);press(&s,A);
 first(&s,"People are getting");done(&s);
 /* Studio work is a four-session playable project. Erasing an old demo
    is independent from the later surviving finished master. */
 Story old;new_game(&old);music_risk(&old);
 first(&old,"EARLY DEMO ERASED.");done(&old);
 Story master;new_game(&master);
 master.song_steps=STORY_PROJECT_DONE;music_risk(&master);
 first(&master,"EARLY TAKE ERASED.");done(&master);
 /* The fourth offense can occur at the remote terminal without creating
    the false impression Danny already approached a safe-house door. */
 Story remote;new_game(&remote);
 remote.bad_mask=STORY_BAD_LETTER|STORY_BAD_CLERK|STORY_BAD_MUSIC;
 remote.rescue_stage=STORY_RESCUE_MISSING;
 stand(&remote,21,8);press(&remote,B);
 assert(remote.risk_menu=='S');
 press(&remote,RIGHT);press(&remote,A);
 first(&remote,"A lead went quiet.");
 assert(remote.rescue_stage==STORY_RESCUE_LOST&&!remote.ended);
 assert(remote.day_number==0u&&remote.day_frame<=2u);
 /* A real failure *at* the safe house uses its own truthful geography. */
 Story house;new_game(&house);
 house.rescue_stage=STORY_RESCUE_TRACE;
 stand(&house,7,3);press(&house,A);
 assert(house.risk_menu=='H');
 press(&house,RIGHT);press(&house,A);
 first(&house,"The light went out.");
 assert(house.rescue_stage==STORY_RESCUE_LOST&&!house.ended);
 /* Past harm changes the good ending's dialogue instead of being erased
    by an otherwise successful music and bus route. */
 Story clean;new_game(&clean);
 clean.ended=1;clean.ending=STORY_END_GOOD;clean.dialogue=0;
 clean.finale_frame=STORY_GOOD_FINAL_FRAMES-1u;
 story_tick(&clean,0);
 first(&clean,"The bus was only");
 Story scar;new_game(&scar);
 scar.ended=1;scar.ending=STORY_END_GOOD;scar.dialogue=0;
 scar.finale_frame=STORY_GOOD_FINAL_FRAMES-1u;
 scar.bad_mask=STORY_BAD_LETTER;
 story_tick(&scar,0);
 first(&scar,"Danny lived beyond");
 uint8_t a[120*80],b[120*80];
 clean.dialogue=scar.dialogue=0;
 clean.finale_frame=scar.finale_frame=700u;
 check_frame(&clean,a);check_frame(&scar,b);
 assert(caption_diff(a,b)>12u);
 /* Neutral film does not say "unfinished song" when the master was
    actually finished by the player in the music studio. */
 Story neutral;new_game(&neutral);
 neutral.ended=1;neutral.ending=STORY_END_NEUTRAL;neutral.dialogue=0;
 neutral.finale_frame=2150u;neutral.song_steps=0;
 check_frame(&neutral,a);
 neutral.song_steps=STORY_PROJECT_DONE;check_frame(&neutral,b);
 assert(caption_diff(a,b)>12u);
 puts("story audit: heard promise, responsive Mara trust, distinct erased demo/master, remote vs house failure, honest good/neutral cinematics OK");
 return 0;
}
