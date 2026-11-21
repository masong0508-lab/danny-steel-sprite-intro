#include "story.h"
#include "story_draw.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define A 1
#define START 8
static void press(Story *s, unsigned k){story_tick(s,0);story_tick(s,(uint16_t)k);story_tick(s,0);}
static void done(Story *s){for(int n=0;n<32&&s->dialogue;n++)press(s,A);assert(!s->dialogue);}
static void stand(Story *s,int x,int y,int dir){
    assert(!story_solid(x,y));s->x=(uint8_t)x;s->y=(uint8_t)y;s->facing=(uint8_t)dir;
    s->prev_x=s->x;s->prev_y=s->y;s->cooldown=0;
}
static void bus(Story *s){stand(s,19,4,1);press(s,A);assert(s->dialogue);done(s);assert(s->choosing);press(s,A);}
static void clinic(Story *s){stand(s,15,5,0);press(s,A);assert(s->scene==STORY_SCENE_CLINIC);done(s);}
static void render_guard(Story *s){
    struct {uint8_t a[12], pixels[120*80], z[12];} out;
    memset(&out,0xED,sizeof(out));story_draw_init();
    for(int frame=0;frame<30;frame++){
        story_draw(s,frame,out.pixels);
        for(int i=0;i<12;i++)assert(out.a[i]==0xED&&out.z[i]==0xED);
        for(int i=0;i<120*80;i++)assert(out.pixels[i]<32);
    }
}
static void test_neutral(void){
    Story s;story_new(&s,0);done(&s);s.flags=STORY_ALL;
    bus(&s);assert(!s.ended&&s.life_stage==STORY_LIFE_WAIT);done(&s);
    assert(s.day_number==1&&s.day_frame<100);
    render_guard(&s);
    /* Near-real-time clock is 60Hz; tests skip to the *last* frame. */
    s.day_frame=STORY_DAY_FRAMES-1;story_tick(&s,0);
    assert(s.day_number==2&&s.life_stage==STORY_LIFE_CLINIC);
    assert(s.scene==STORY_SCENE_NOTICE);done(&s);
    clinic(&s);assert(s.life_stage==STORY_LIFE_FOLLOWUP_WAIT);
    render_guard(&s);
    s.day_frame=STORY_DAY_FRAMES-1;story_tick(&s,0);
    assert(s.day_number==3&&s.life_stage==STORY_LIFE_FINAL_CLINIC);
    done(&s);clinic(&s);assert(s.life_stage==STORY_LIFE_LAST_WORK);
    /* Waiting is permitted after the final visit. Nothing auto-kills Danny. */
    for(int i=0;i<600;i++)story_tick(&s,0);
    assert(!s.ended);
    stand(&s,14,7,3);press(&s,A);
    assert(s.life_stage==STORY_LIFE_FINALE&&!s.ended);render_guard(&s);
    done(&s);while(s.finale_frame<STORY_END_SCORE_FRAMES-1)story_tick(&s,0);
    assert(!s.ended);story_tick(&s,0);
    assert(s.ended&&s.ending==STORY_END_NEUTRAL&&s.dialogue);
    render_guard(&s);done(&s);press(&s,START);assert(s.quit);
    puts("neutral: numbered days, two doctor visits and 72.5-second film ending OK");
}
static void test_good(void){
    Story s;story_new(&s,1);done(&s);s.flags=STORY_FULL;
    bus(&s);assert(s.ended&&s.ending==STORY_END_GOOD);
    assert(s.life_stage==STORY_LIFE_NONE && !s.dialogue);render_guard(&s);
    while(s.finale_frame<STORY_END_SCORE_FRAMES)story_tick(&s,0);
    assert(!s.dialogue && s.finale_frame==STORY_END_SCORE_FRAMES);
    while(s.finale_frame<STORY_GOOD_FINAL_FRAMES-1)story_tick(&s,0);
    assert(!s.dialogue);story_tick(&s,0);
    assert(s.dialogue && s.finale_frame==STORY_GOOD_FINAL_FRAMES);
    done(&s);press(&s,START);assert(s.quit);
    story_resume(&s,1,19,4,1,0,STORY_FULL,1,STORY_END_GOOD);
    assert(!s.dialogue && s.ended && s.finale_frame==0);
    puts("good: full score montage + 16-second legacy coda before dialogue ending OK");
}
static void test_art_music(void){
    Story s;story_new(&s,0);done(&s);
    stand(&s,19,8,3);press(&s,A);assert(s.flags&STORY_MUSIC);done(&s);
    stand(&s,14,7,3);press(&s,A);assert(s.flags&STORY_ART);done(&s);
    puts("art and music: tangible, accessible main-story completion flags OK");
}
int main(void){test_art_music();test_good();test_neutral();return 0;}
