#include "story.h"
#include "story_draw.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static struct { uint8_t guard[16], pixels[120*80], tail[16]; } a,b;
static void check(Story *s,int x,int y,const char *label){
 int gx=-1,gy=-1;const char *got=0;
 assert(story_goal(s,&gx,&gy,&got));
 assert(gx==x && gy==y && strcmp(got,label)==0);
}
static void render(Story *s,void *where){
 struct { uint8_t guard[16], pixels[120*80], tail[16]; } *out=where;
 memset(out,0xB6,sizeof(*out));
 story_draw(s,45,out->pixels);
 for(int i=0;i<16;i++)assert(out->guard[i]==0xB6 && out->tail[i]==0xB6);
 for(int i=0;i<120*80;i++)assert(out->pixels[i]<32);
}
int main(void){
 Story s;story_new(&s,0);s.dialogue=0;
 check(&s,5,5,"LETTER");
 s.flags=STORY_HOME;check(&s,11,7,"STAMP");
 s.flags=STORY_HOME|STORY_STAMP;check(&s,8,11,"MARA");
 s.flags=STORY_ALL;check(&s,19,7,"MUSIC");
 s.flags=STORY_ALL|STORY_MUSIC;check(&s,14,6,"CANVAS");
 s.flags=STORY_FULL;check(&s,19,5,"LAST BUS");
 s.life_stage=STORY_LIFE_WAIT;check(&s,8,11,"MARA");
 s.life_stage=STORY_LIFE_CLINIC;check(&s,16,5,"CLINIC");
 s.life_stage=STORY_LIFE_FOLLOWUP_WAIT;check(&s,8,11,"MARA");
 s.life_stage=STORY_LIFE_FINAL_CLINIC;check(&s,16,5,"CLINIC");
 s.life_stage=STORY_LIFE_LAST_WORK;check(&s,14,6,"CANVAS");
 s.life_stage=STORY_LIFE_FINALE;
 {int x,y;const char *label;assert(!story_goal(&s,&x,&y,&label));}
 s.life_stage=STORY_LIFE_NONE;s.ended=1;
 {int x,y;const char *label;assert(!story_goal(&s,&x,&y,&label));}
 s.ended=0;s.flags=0;s.x=12;s.y=13;s.cinema=CINE_NONE;
 story_draw_init();
 s.view=0;render(&s,&a);
 s.view=1;render(&s,&b);
 assert(!memcmp(a.pixels,b.pixels,120*8));
 assert(memcmp(a.pixels+8*120,b.pixels+8*120,120*56)!=0);
 puts("guidance: quest stages, clinic priority, shared 2D/3D HUD and guarded render OK");
 return 0;
}
