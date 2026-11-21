#include "story.h"
#include <assert.h>
#include <stdio.h>
#define A 1
#define B 2
#define RIGHT 16
#define LEFT 32
#define UP 64
#define DOWN 128
static void press(Story *s, unsigned key) { story_tick(s, 0); story_tick(s, key); story_tick(s, 0); }
static void close_dialogue(Story *s) {
    int count; story_pages(s,&count);
    while(s->dialogue) press(s,A);
}
static void walk_to(Story *s, int x, int y) {
    /* Reaching a quest actor is guaranteed by a flood fill on the compiled map. */
    assert(!story_solid(x,y));
    s->x = x; s->y=y; s->cooldown=0;
}
int main(void) {
    int reach[STORY_H][STORY_W]={0}, qx[STORY_W*STORY_H],qy[STORY_W*STORY_H], a=0,b=0;
    qx[b]=12; qy[b++]=13; reach[13][12]=1;
    while (a<b) {
        int x=qx[a],y=qy[a++];
        int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};
        for(int i=0;i<4;i++) {int xx=x+dx[i],yy=y+dy[i];
            if(xx>=0&&xx<STORY_W&&yy>=0&&yy<STORY_H&&!reach[yy][xx]&&!story_solid(xx,yy)){
                reach[yy][xx]=1;qx[b]=xx;qy[b++]=yy;}
        }
    }
    int found[4]={0};
    for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++){
        char c=story_tile(x,y);int idx=c=='L'?0:c=='C'?1:c=='F'?2:c=='B'?3:-1;
        if(idx>=0){int near=0;near|=x>0&&reach[y][x-1];near|=x+1<STORY_W&&reach[y][x+1];
            near|=y>0&&reach[y-1][x];near|=y+1<STORY_H&&reach[y+1][x];
            found[idx]+=near;}
    }
    for(int i=0;i<4;i++)assert(found[i]==1);
    Story s; story_new(&s,0); assert(s.dialogue);close_dialogue(&s);
    walk_to(&s,19,4); press(&s,A); assert(s.flags==0);close_dialogue(&s);
    walk_to(&s,5,5); press(&s,A); assert(s.flags==STORY_HOME);close_dialogue(&s);
    walk_to(&s,11,8); press(&s,A); assert((s.flags&STORY_STAMP));close_dialogue(&s);
    walk_to(&s,8,12); press(&s,A); assert((s.flags&STORY_MARA));close_dialogue(&s);
    walk_to(&s,19,4); press(&s,A);close_dialogue(&s);assert(s.choosing);
    press(&s,RIGHT);press(&s,A); assert(s.ended&&s.ending==1);close_dialogue(&s);
    story_resume(&s,0,11,12,2,1,7,1,1);assert(s.flags==7&&s.view&&s.ended);
    puts("Danny Steel: all four story anchors reachable, dialogue and both modes functional.");
    return 0;
}
