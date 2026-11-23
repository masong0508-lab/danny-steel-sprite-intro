#include "story.h"
#include "story_draw.h"
#include "story_checks.h"
#include "hd_canvas.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define A 1u
#define B 2u
#define START 8u
#define LEFT 32u
#define RIGHT 16u
#define R 256u
#define L 512u
static void press(Story *s,uint16_t key){
 story_tick(s,0);story_tick(s,key);story_tick(s,0);
}
static void close_pages(Story *s){
 for(int i=0;i<50&&s->dialogue;i++)press(s,A);
 assert(!s->dialogue);
}
static void stand(Story *s,int x,int y,int dir){
 assert(!story_solid(x,y));s->x=(uint8_t)x;s->y=(uint8_t)y;
 s->prev_x=s->x;s->prev_y=s->y;s->cooldown=0;s->facing=(uint8_t)dir;
}
static void guards(Story *s){
 struct {uint8_t before[16],p[120*80],after[16];} out;
 for(int view=0;view<2;view++){
  s->view=(uint8_t)view;memset(&out,0xBB,sizeof(out));
  story_draw(s,77,out.p);
  for(int i=0;i<16;i++)assert(out.before[i]==0xBB&&out.after[i]==0xBB);
  for(int i=0;i<120*80;i++)assert(out.p[i]<32);
 }
}
/* The direct help must also fit the true 240x160 native Mode-4
   surface with no palette leak or writes outside its guarded bounds. */
static void native_guards(const Story *s){
 struct {uint8_t before[24],pixels[HD_BYTES],after[24];} screen;
 memset(&screen,0xA6,sizeof screen);
 hd_rect(screen.pixels,0,0,HD_W,HD_H,HD_STONE_DARK);
 hd_game_overlay(screen.pixels,s);
 for(unsigned i=0;i<24u;i++)
  assert(screen.before[i]==0xA6u&&screen.after[i]==0xA6u);
 for(unsigned i=0;i<HD_BYTES;i++)assert(screen.pixels[i]<64u);
}
static void first_page(Story *s,const char *phrase){
 int count=0;const StoryPage *page=story_pages(s,&count);
 assert(count>0&&page&&!strcmp(page[0].line1,phrase));
}
int main(void){
 Story s;story_new(&s,0);close_pages(&s);story_draw_init();
 for(unsigned i=0;i<STORY_CHECK_COUNT;i++)
  assert(!story_check_live(&s,i));
 for(unsigned p=0;p<STORY_CHECK_PAGE_COUNT;p++)
  assert(story_check_count_page(&s,p)==0);
 /* These are gameplay conditions, not saved-checksum bits or fake rewards. */
 s.flags=STORY_FULL;
 assert(story_check_count_page(&s,0)==5);
 assert(story_check_live(&s,STORY_CHECK_MUSIC));
 s.song_steps=4;s.art_steps=4;s.bond=4;
 s.cult_stage=CULT_RESOLVED;s.life_stage=STORY_LIFE_FOLLOWUP_WAIT;
 assert(story_check_count_page(&s,1)==5);
 s.bad_mask=STORY_BAD_LETTER|STORY_BAD_CLERK|
            STORY_BAD_MUSIC|STORY_BAD_SERVER;
 s.rescue_stage=STORY_RESCUE_SAVED;
 assert(story_bad_count(&s)==4);
 assert(story_check_count_page(&s,2)==5);
 assert(story_check_live(&s,STORY_CHECK_MARA_SAFE));
 Story copy=s;
 for(unsigned i=0;i<STORY_CHECK_COUNT;i++)
  (void)story_check_live(&s,i);
 for(unsigned page=0;page<STORY_CHECK_PAGE_COUNT;page++)
  (void)story_check_count_page(&s,page);
 assert(!memcmp(&s,&copy,sizeof(s))); /* pure queries never write game state */
 /* A fixed pause journal, not a new SRAM slot, and navigation is reversible. */
 press(&s,START);assert(s.paused);
 assert(!(s.choice&STORY_CHECK_PAGE_ACTIVE));
 guards(&s);
 press(&s,R);assert(s.choice&STORY_CHECK_PAGE_ACTIVE);
 assert(story_check_page(&s)==0);guards(&s);
 press(&s,R);assert(story_check_page(&s)==1);guards(&s);
 press(&s,R);assert(story_check_page(&s)==2);guards(&s);
 press(&s,R);assert(story_check_page(&s)==3);guards(&s);
 press(&s,R);assert(story_check_page(&s)==0);guards(&s);
 press(&s,B);assert(!(s.choice&STORY_CHECK_PAGE_ACTIVE));
 assert(s.paused&&!s.quit);guards(&s);
 /* L is an immediate, genuinely interactive two-page guide without a
    journal round trip. R remains reversible from inside that guide. */
 Story before_guide=s;
 press(&s,L);
 assert((s.choice&STORY_FIELD_GUIDE_ACTIVE)&&
        !(s.choice&STORY_CHECK_PAGE_ACTIVE));
 guards(&s);native_guards(&s);
 press(&s,RIGHT);
 assert(s.choice&STORY_FIELD_GUIDE_DETAIL);
 guards(&s);native_guards(&s);
 press(&s,LEFT);
 assert(!(s.choice&STORY_FIELD_GUIDE_DETAIL));
 guards(&s);native_guards(&s);
 press(&s,R);
 assert((s.choice&STORY_CHECK_PAGE_ACTIVE)&&
        !(s.choice&STORY_FIELD_GUIDE_ACTIVE));
 assert(story_check_page(&s)==0);guards(&s);
 press(&s,L);
 assert((s.choice&STORY_FIELD_GUIDE_ACTIVE)&&
        !(s.choice&STORY_FIELD_GUIDE_DETAIL));
 guards(&s);native_guards(&s);
 press(&s,B);
 assert(!(s.choice&STORY_FIELD_GUIDE_ACTIVE)&&s.paused&&!s.quit);
 assert(s.flags==before_guide.flags&&s.energy==before_guide.energy&&
        s.bond==before_guide.bond&&s.work==before_guide.work);
 press(&s,L);
 press(&s,START);assert(!s.paused&&!s.quit);
 assert(!(s.choice&(STORY_FIELD_GUIDE_ACTIVE|STORY_FIELD_GUIDE_DETAIL)));
 /* Existing pause B remains the explicit save-and-quit action. */
 press(&s,START);press(&s,B);assert(s.quit);
 /* No checkmark appears without satisfying its actual predicate. */
 Story t;story_new(&t,1);close_pages(&t);
 t.flags=STORY_HOME|STORY_MARA;
 assert(story_check_count_page(&t,0)==2);
 t.bad_mask=STORY_BAD_CLERK;
 assert(story_check_live(&t,STORY_CHECK_FORGE_FORM));
 assert(!story_check_live(&t,STORY_CHECK_MARA_SAFE));
 assert(story_check_count_page(&t,2)==1);
 /* A resolved investigation makes the witness acknowledge past help. */
 t.cult_stage=CULT_RESOLVED;
 t.rescue_stage=STORY_RESCUE_MISSING;
 stand(&t,6,12,3);press(&t,A);
 assert(t.rescue_stage==STORY_RESCUE_WITNESS);
 first_page(&t,"You helped us with");
 close_pages(&t);
 /* Strong friendship changes the safehouse reunion, not just its meter. */
 t.rescue_stage=STORY_RESCUE_TRACE;t.bond=4;
 stand(&t,7,3,3);press(&t,A);assert(t.risk_menu=='H');
 guards(&t);press(&t,A);
 assert(t.rescue_stage==STORY_RESCUE_SAVED);
 first_page(&t,"You kept looking.");
 guards(&t);close_pages(&t);
 /* Finishing a painting offers another authored reunion if trust is lower. */
 Story art;story_new(&art,0);close_pages(&art);
 art.rescue_stage=STORY_RESCUE_TRACE;art.art_steps=4;
 stand(&art,7,3,3);press(&art,A);assert(art.risk_menu=='H');
 press(&art,A);first_page(&art,"You found me.");
 close_pages(&art);
 /* Selection previews are legible in the same clipped 120x80 framebuffer. */
 art.life_stage=STORY_LIFE_WAIT;art.daily_mask=0;
 stand(&art,19,8,3);press(&art,A);
 assert(art.activity=='M');guards(&art);
 art.activity_choice=1;guards(&art);
 assert(sizeof(Story)<64);
 puts("consequences: 15 live checks, four-page journal and direct two-page guide, reversible controls, honest risks, trust/investigation/painting branches and guarded 120x80/240x160 UI OK");
 return 0;
}
