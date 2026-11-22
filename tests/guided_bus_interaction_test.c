#include "story.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void press(Story *s,unsigned key){
 story_tick(s,0u);story_tick(s,(uint16_t)key);story_tick(s,0u);
}
int main(void){
 Story s;story_new(&s,0);s.dialogue=0;s.cinema=CINE_NONE;
 s.flags=STORY_FULL;s.x=20;s.y=11;s.facing=1;
 static const unsigned key[4]={16u,128u,32u,64u};
 for(int i=0;i<STORY_W*STORY_H;i++){
  int dir=-1,steps=-1;
  assert(story_route(&s,&dir,&steps));
  if(steps==0)break;
  assert(dir>=0&&dir<4);
  int before_x=s.x,before_y=s.y;
  s.old_keys=0;s.cooldown=0;
  story_tick(&s,(uint16_t)key[dir]);story_tick(&s,0u);
  assert(before_x!=s.x||before_y!=s.y);
  assert(!story_solid(s.x,s.y));
 }
 {int dir=-1,steps=-1;
  assert(story_route(&s,&dir,&steps)&&steps==0);}
 /* Deliberately make BOTH actors adjacent with Danny facing NEITHER.
    Prior to the fix EAST/SOUTH fallback selected the music station. */
 assert(!story_solid(19,6));
 s.x=19;s.y=6;s.facing=0;s.old_keys=0;s.cooldown=0;
 assert(strcmp(story_action_hint(&s),"A BUS")==0);
 press(&s,1u);
 assert(s.dialogue);
 while(s.dialogue)press(&s,1u);
 assert(s.choosing);
 s.choosing=0;s.dialogue=0;s.cinema=CINE_NONE;
 s.x=12;s.y=12;s.facing=0;s.old_keys=0;
 assert(!story_risk_available(&s));
 assert(strcmp(story_action_hint(&s),"A LOOK")==0);
 press(&s,1u);
 int count=0;const StoryPage *pages=story_pages(&s,&count);
 assert(s.dialogue&&count==2&&strcmp(pages[0].speaker,"DANNY")==0);
 press(&s,2u);
 s.dialogue=0;s.cinema=CINE_NONE;
 s.x=8;s.y=12;s.facing=1;
 s.rescue_stage=STORY_RESCUE_LOST;
 assert(!strcmp(story_action_hint(&s),"A PHONE"));
 s.rescue_stage=STORY_RESCUE_MISSING;
 assert(!strcmp(story_action_hint(&s),"A EMPTY CAFE"));
 puts("guided bus route, actor-priority, ambient street writing and missing-Mara prompts passed");
 return 0;
}
