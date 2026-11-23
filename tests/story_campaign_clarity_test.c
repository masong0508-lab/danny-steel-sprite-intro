#include "story.h"
#include "hd_canvas.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Contract: story direction, actual bus consequences and picture cues must
   agree, even if all optional progress is left unfinished. */
static void press(Story *s,unsigned key){
 story_tick(s,0u);
 story_tick(s,(uint16_t)key);
 story_tick(s,0u);
}
static void finish_talk(Story *s){
 for(int i=0;i<40&&s->dialogue;i++)press(s,1u);
 assert(!s->dialogue);
}
static const StoryPage *pages(const Story *s,int min){
 int count=0;
 const StoryPage *p=story_pages(s,&count);
 assert(p&&count>=min);
 return p;
}
static void check_quest(Story *s,const char *kind,
                        const char *label,const char *reason){
 int x=-1,y=-1;
 const char *name=0;
 Story old=*s;
 assert(story_goal(s,&x,&y,&name));
 assert(!strcmp(story_goal_kind(s),kind));
 assert(!strcmp(name,label));
 assert(!strcmp(story_goal_reason(s),reason));
 int dir=-1,steps=-1;
 assert(story_route(s,&dir,&steps));
 assert(steps>=0&&steps<STORY_H*STORY_W);
 if(steps)assert(dir>=0&&dir<4);
 assert(!memcmp(&old,s,sizeof old)); /* chapter queries never touch SRAM */
}
static void board(Story *s){
 s->dialogue=0;s->cinema=CINE_NONE;s->old_keys=0;
 s->x=19;s->y=6;s->facing=3;s->cooldown=0;
 assert(!strcmp(story_action_hint(s),"A BUS"));
 press(s,1u);
 assert(s->dialogue);
 assert(!strcmp(pages(s,3)[0].line1,"Your stamp checks out."));
 finish_talk(s);
 assert(s->choosing);
}
static struct {
 unsigned char before[32],pixels[HD_BYTES],after[32];
} screen_a,screen_b;
static void paint(uint8_t *p,const Story *s,int overlay){
 memset(&screen_a,0xA8,sizeof screen_a);
 if(overlay)hd_game_overlay(screen_a.pixels,s);
 else hd_isometric_draw(screen_a.pixels,s,93u);
 for(int i=0;i<32;i++)assert(screen_a.before[i]==0xA8&&screen_a.after[i]==0xA8);
 for(unsigned i=0;i<HD_BYTES;i++)assert(screen_a.pixels[i]<64u);
 memcpy(p,screen_a.pixels,HD_BYTES);
}
int main(void){
 Story s;
 story_new(&s,0);
 const StoryPage *intro=pages(&s,7);
 assert(!strcmp(intro[4].line2,"Clinic called again."));
 assert(!strcmp(intro[6].line2,"Then clerk. Then Mara."));
 finish_talk(&s);
 check_quest(&s,"CORE","LETTER","LETTER / WEST OLD QUARTER");
 s.flags=STORY_HOME;
 check_quest(&s,"CORE","STAMP","PROVE ADDRESS AT BORE");
 s.flags=STORY_HOME|STORY_STAMP;
 check_quest(&s,"CORE","MARA","TELL MARA ABOUT THE CALL");
 s.flags=STORY_ALL;
 check_quest(&s,"EXTRA","MUSIC","OPTIONAL TAPE / BUS OPEN");
 s.flags|=STORY_MUSIC;
 check_quest(&s,"EXTRA","CANVAS","OPTIONAL ROOF / BUS OPEN");
 s.flags=STORY_FULL;
 check_quest(&s,"BUS","LAST BUS","BOARD THE BUS OR STAY");
 s.life_stage=STORY_LIFE_CLINIC;
 assert(!strcmp(story_goal_kind(&s),"CARE"));
 s.life_stage=STORY_LIFE_LAST_WORK;
 assert(!strcmp(story_goal_kind(&s),"LAST"));
 s.life_stage=STORY_LIFE_WAIT;
 assert(!strcmp(story_goal_kind(&s),"DAY"));
 s.rescue_stage=STORY_RESCUE_MISSING;
 assert(!strcmp(story_goal_kind(&s),"SAVE"));
 s.rescue_stage=STORY_RESCUE_LOST;
 assert(!strcmp(story_goal_kind(&s),"HOPE"));
 s.rescue_stage=STORY_RESCUE_NONE;
 s.life_stage=STORY_LIFE_NONE;

 /* A player may choose the bus after the three core tasks.
    The phone interruption is EXPLAINED rather than teleporting them
    into the neutral route with no causal link. */
 Story neutral=s;
 neutral.flags=STORY_ALL;
 board(&neutral);
 press(&neutral,1u); /* TRY TO GO */
 assert(neutral.life_stage==STORY_LIFE_WAIT && !neutral.ended);
 assert(!strcmp(pages(&neutral,5)[0].line1,"I reached the bus."));
 assert(!strcmp(pages(&neutral,5)[2].line1,"I let the bus go."));
 /* Choosing STAY is still a reversible, open-world action. */
 Story stay=s;
 stay.flags=STORY_ALL;
 board(&stay);
 press(&stay,16u); /* choose STAY */
 press(&stay,1u);
 assert(stay.life_stage==STORY_LIFE_NONE&&!stay.ended&&!stay.choosing);
 Story good=s;
 board(&good);
 press(&good,1u); /* BOARD */
 assert(good.ended&&good.ending==STORY_END_GOOD);
 assert(good.life_stage==STORY_LIFE_NONE);

 /* Native confirmation cards must visibly distinguish TRY from BOARD. */
 Story try_board=s,ready_to_board=s;
 try_board.flags=STORY_ALL;try_board.dialogue=0;try_board.choosing=1;
 ready_to_board.flags=STORY_FULL;ready_to_board.dialogue=0;
 ready_to_board.choosing=1;
 paint(screen_b.pixels,&try_board,1);
 unsigned char choice_image[HD_BYTES];
 memcpy(choice_image,screen_b.pixels,HD_BYTES);
 paint(screen_b.pixels,&ready_to_board,1);
 assert(memcmp(choice_image+93*HD_W,screen_b.pixels+93*HD_W,29*HD_W));
 /* Breadcrumb remains on the next walkable square, with guard bytes
    and no painted objective when a completed story is on screen. */
 Story map=s;
 map.x=20;map.y=11;map.prev_x=map.x;map.prev_y=map.y;
 map.anim=0;map.flags=0;map.ended=0;
 paint(screen_b.pixels,&map,0);
 memcpy(choice_image,screen_b.pixels,HD_BYTES);
 map.ended=1;
 paint(screen_b.pixels,&map,0);
 assert(memcmp(choice_image,screen_b.pixels,HD_BYTES));
 puts("campaign: core/extra clarity, bus-clinic continuity, live chapter tags, two endings and guarded native breadcrumb + choice art OK");
 return 0;
}
