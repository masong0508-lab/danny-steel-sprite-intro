#include "story.h"
#include "terrain_save.h"
#include "hd_canvas.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static struct {uint8_t before[32],p[HD_BYTES],after[32];} frame;
static uint8_t prior[HD_BYTES];
static void press(Story *s,unsigned keys){story_tick(s,0);story_tick(s,(uint16_t)keys);story_tick(s,0);}
static void done(Story *s){for(unsigned i=0;i<20u&&s->dialogue;i++)press(s,1u);assert(!s->dialogue);}
static void stand(Story *s,int x,int y){
 assert(!story_solid(x,y));s->x=(uint8_t)x;s->y=(uint8_t)y;
 s->prev_x=s->x;s->prev_y=s->y;s->anim=s->cooldown=0;s->old_keys=0;
}
static int elevation(const Story *s,int x,int y){return story_terrain_height(s,x*256+128,y*256+128);}
static void paint(Story *s,int view){
 memset(&frame,0xA5,sizeof frame);
 if(view)hd_first_person_draw(frame.p,s,71u);
 else hd_isometric_draw(frame.p,s,71u);
 for(unsigned i=0;i<32u;i++)assert(frame.before[i]==0xA5&&frame.after[i]==0xA5);
 for(unsigned i=0;i<HD_BYTES;i++)assert(frame.p[i]<64u);
}
int main(void){
 Story s;story_new(&s,0);done(&s);
 assert(sizeof(Story)<64u&&s.terrain_style==STORY_LAND_DEFAULT);
 assert(elevation(&s,56,11)>=70&&elevation(&s,43,22)==0);
 assert(elevation(&s,19,7)==0&&story_tile(19,7)=='M');
 assert(!story_solid(39,8)&&!story_solid(40,8));
 /* All editable grades retain safe walkable slopes. Test the actual
    shared height sampler, not the generator's assumed elevation. */
 static const int dx[]={1,0,-1,0},dy[]={0,1,0,-1};
 static const unsigned keys[]={16u,128u,32u,64u};
 unsigned grades=0;int steep_x=-1,steep_y=-1,steep_d=-1;
 for(unsigned style=0;style<16u;style++){
  s.terrain_style=style;
  for(int y=1;y<STORY_H-1;y++)for(int x=39;x<STORY_W-1;x++){
   if(story_solid(x,y))continue;
   int h=elevation(&s,x,y);assert(h>=0&&h<=88);
   for(unsigned d=0;d<4u;d++){
    int nx=x+dx[d],ny=y+dy[d];if(story_solid(nx,ny))continue;
    int delta=elevation(&s,nx,ny)-h;
    assert(delta>=-20&&delta<=20);grades++;
    if(style==STORY_LAND_DEFAULT&&delta>=6&&x>=40){
     steep_x=x;steep_y=y;steep_d=(int)d;
    }
   }
  }
 }
 s.terrain_style=STORY_LAND_DEFAULT;assert(steep_d>=0);
 stand(&s,steep_x,steep_y);
 story_tick(&s,(uint16_t)keys[steep_d]);
 assert(s.x==steep_x+dx[steep_d]&&s.y==steep_y+dy[steep_d]&&s.cooldown>=9u);
 /* Real pad input modifies only its patch. B lowers, A+B cancels,
    cap prevents wrap, and a clinic appointment overrides optional work. */
 stand(&s,44,8);s.facing=0;
 int h=elevation(&s,44,8),other=elevation(&s,54,24);
 press(&s,1u);assert(story_land_level(&s,0)==2u&&elevation(&s,44,8)>h);
 assert(story_land_level(&s,1)==1u&&elevation(&s,54,24)==other);
 press(&s,1u);press(&s,1u);assert(story_land_level(&s,0)==3u);
 press(&s,3u);assert(story_land_level(&s,0)==3u);
 press(&s,2u);press(&s,2u);press(&s,2u);press(&s,2u);
 assert(story_land_level(&s,0)==0u&&elevation(&s,44,8)<h);
 stand(&s,54,24);press(&s,1u);assert(story_land_level(&s,1)==2u);
 uint8_t raw[TERRAIN_EXT_LEN];terrain_save_encode(&s,raw);
 Story resumed;story_new(&resumed,0);assert(terrain_save_decode(&resumed,raw));
 assert(resumed.terrain_style==s.terrain_style);
 for(unsigned i=0;i<TERRAIN_EXT_LEN;i++){
  raw[i]^=1u;assert(!terrain_save_decode(&resumed,raw));raw[i]^=1u;
 }
 resumed.slot=1;assert(!terrain_save_decode(&resumed,raw));
 s.life_stage=STORY_LIFE_CLINIC;
 unsigned old=s.terrain_style;press(&s,1u);done(&s);
 assert(s.terrain_style==old);
 s.life_stage=STORY_LIFE_NONE;
 /* A real hike advances the same art project; repeated attempts cap it.
    The lake rest doesn't create money or painting progress. */
 stand(&s,56,11);
 for(unsigned phase=1;phase<=4u;phase++){
  press(&s,1u);done(&s);assert(s.art_steps==phase&&(s.flags&STORY_ART));
 }
 unsigned work=s.work;press(&s,1u);done(&s);assert(s.art_steps==4u&&s.work==work);
 stand(&s,43,22);s.energy=1;press(&s,1u);done(&s);
 assert(s.energy==2u&&s.art_steps==4u&&s.work==work);
 /* Physical elevation changes both in-game views, not just labels. */
 stand(&s,44,8);s.facing=0;
 for(int view=0;view<2;view++){
  s.terrain_style=4u;paint(&s,view);memcpy(prior,frame.p,HD_BYTES);
  s.terrain_style=7u;paint(&s,view);assert(memcmp(prior,frame.p,HD_BYTES)!=0);
  memcpy(prior,frame.p,HD_BYTES);paint(&s,view);assert(!memcmp(prior,frame.p,HD_BYTES));
  stand(&s,56,11);for(unsigned d=0;d<4u;d++){s.facing=(uint8_t)d;paint(&s,view);}
  stand(&s,44,8);s.facing=0;
 }
 printf("Landscape: %u bounded slope samples, live uphill step, two editable plots, CRC save, art/rest and both native views\n",grades);
 return 0;
}
