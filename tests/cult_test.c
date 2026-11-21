#include "story.h"
#include "story_draw.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define A 1
#define B 2
#define RIGHT 16
#define LEFT 32
static void press(Story *s, unsigned key) {
    story_tick(s,0); story_tick(s,(uint16_t)key); story_tick(s,0);
}
static void close_dialogue(Story *s) {
    for(int i=0;i<20 && s->dialogue;i++)press(s,A);
    assert(!s->dialogue);
}
static void stand(Story *s,int x,int y,int facing){
    assert(!story_solid(x,y)); s->x=(uint8_t)x;s->y=(uint8_t)y;
    s->prev_x=s->x;s->prev_y=s->y;s->facing=(uint8_t)facing;
}
static void all_good(Story *s){
    for(int i=0;i<200 && s->battle;i++){
        if(s->battle==BATTLE_MENU)press(s,A);
        else {
            int next=s->battle_timer / 15;
            /* Choose a lane that will not be hit by the next incoming bubble. */
            int target=(story_attack_lane(s,next)+1)%3;
            if(s->battle_lane < target) press(s,RIGHT);
            else if(s->battle_lane > target) press(s,LEFT);
            else story_tick(s,0);
        }
    }
    assert(!s->battle);
    assert(s->dialogue);
    close_dialogue(s);
}
static void test_persistable_quest(void){
    Story s;story_new(&s,0);close_dialogue(&s);
    /* Entire server plot optional: no mainline flags are required or awarded. */
    assert(s.flags==0 && s.cult_stage==0);
    stand(&s,2,7,0);press(&s,A);
    assert(s.battle==BATTLE_MENU && s.cult_enemy==0);
    assert(s.view==0);all_good(&s);
    assert(s.cult_pre_wins==1 && s.cult_stage==0);
    /* Switch to 3D exploration. Combat still uses the same encounter locations. */
    press(&s,512);assert(s.view==1);
    stand(&s,16,7,0);press(&s,A);assert(s.battle==BATTLE_MENU && s.cult_enemy==1);
    press(&s,B);close_dialogue(&s);
    /* Investigate, contact the witness, arrange offline aid, preserve evidence. */
    stand(&s,20,7,0);press(&s,A);assert(s.cult_stage==1);close_dialogue(&s);
    stand(&s,5,11,0);press(&s,A);assert(s.cult_stage==2);close_dialogue(&s);
    stand(&s,8,2,2);press(&s,A);assert(s.cult_stage==3);close_dialogue(&s);
    stand(&s,20,7,0);press(&s,A);assert(s.cult_stage==CULT_RESOLVED);close_dialogue(&s);
    assert(s.flags==0 && s.cult_pre_wins==1);
    stand(&s,17,12,0);press(&s,A);
    assert(s.battle==BATTLE_MENU && s.cult_enemy==2);
    assert(strcmp(story_enemy_name(&s),"REPLY ARCHIVE")==0);
    all_good(&s);
    assert(s.cult_post_wins==(1u<<2));
    assert(s.cult_pre_wins==1);
    assert(s.cult_stage==CULT_RESOLVED);
    /* Post-plot enemies still roam and have distinct dialogue on rematches. */
    stand(&s,17,12,0);press(&s,A);assert(s.battle==BATTLE_MENU);
    press(&s,RIGHT);press(&s,RIGHT);press(&s,A);
    assert(!s.battle);close_dialogue(&s);
    /* Only mainline flags determine the bus lock, not completion of the cult plot. */
    stand(&s,20,5,2);press(&s,A);assert(s.dialogue);close_dialogue(&s);
    assert(!s.choosing);
    puts("cult: pre/in/post fights, 3D sharing, opt-in rescue, persistent milestones OK");
}
static void test_renderer(void){
    Story s;story_new(&s,0);s.dialogue=0;
    story_draw_init();
    struct { unsigned char guard[16]; unsigned char pixels[120*80]; unsigned char tail[16]; } out;
    memset(&out,0xAA,sizeof(out));
    s.battle=BATTLE_DODGE;s.battle_focus=3;s.cult_enemy=1;
    for(int phase=0;phase<=60;phase++){
        s.battle_timer=(uint8_t)phase;
        story_draw(&s,phase,out.pixels);
        for(int i=0;i<16;i++) assert(out.guard[i]==0xAA && out.tail[i]==0xAA);
        for(int i=0;i<120*80;i++)assert(out.pixels[i]<32);
    }
    puts("cult: 3D battle renderer stays in 120x80 / 32-colour buffer OK");
}
int main(void){test_persistable_quest();test_renderer();return 0;}
