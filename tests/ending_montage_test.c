#include "story.h"
#include "story_draw.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static void sample(Story *s,unsigned frame,uint8_t *out){
 struct Buffer { uint8_t prefix[16];uint8_t pixels[120*80];uint8_t suffix[16]; } a;
 memset(&a,0xa7,sizeof(a));s->finale_frame=(uint16_t)frame;
 story_draw(s,(int)frame,a.pixels);
 for(int i=0;i<16;i++)assert(a.prefix[i]==0xa7&&a.suffix[i]==0xa7);
 for(int i=0;i<120*80;i++)assert(a.pixels[i]<32);
 memcpy(out,a.pixels,120*80);
}
int main(void){
 Story s;uint8_t a[120*80],b[120*80],c[120*80];
 story_draw_init();story_new(&s,0);s.dialogue=0;
 s.life_stage=STORY_LIFE_FINALE;
 unsigned boundaries[]={0,180,370,810,1130,1590,2010,2490,2870,3250,3650,4050,4349,4350};
 for(unsigned i=0;i<sizeof(boundaries)/sizeof(boundaries[0]);i++)sample(&s,boundaries[i],a);
 assert(cinema_ending_shot(2,0)==0);
 assert(cinema_ending_shot(2,4349)==10);
 sample(&s,120,a);sample(&s,120,b);assert(!memcmp(a,b,sizeof(a)));
 sample(&s,1020,b);assert(memcmp(a,b,sizeof(a)));
 sample(&s,4220,c);assert(memcmp(b,c,sizeof(b)));
 s.life_stage=STORY_LIFE_NONE;s.ended=1;s.ending=STORY_END_GOOD;
 unsigned good[]={0,530,1080,1560,2050,2600,3200,3730,4349,4350,4610,4890,5140,5309,5310};
 for(unsigned i=0;i<sizeof(good)/sizeof(good[0]);i++)sample(&s,good[i],a);
 assert(cinema_ending_shot(1,4349)==7);
 assert(cinema_ending_shot(1,4350)==8);
 assert(cinema_ending_shot(1,5309)==11);
 sample(&s,4350,a);sample(&s,4910,b);assert(memcmp(a,b,sizeof(a)));
 puts("cinematics: neutral 11 shots, good 12 shots, coda only at 4350+ frames, guard/color OK");
 return 0;
}
