#include "story.h"
#include "terrain_save.h"
#include "brush_save.h"
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
static void walk_goal(Story *s,int x,int y,const char *name){
 int gx,gy,dir,steps;const char *label=0;
 assert(story_goal(s,&gx,&gy,&label)&&gx==x&&gy==y&&!strcmp(label,name));
 assert(story_route(s,&dir,&steps));
 static const unsigned key[]={16u,128u,32u,64u};
 unsigned count=0;
 while(steps){
  int before=steps;assert(dir>=0&&dir<4&&count++<STORY_W*STORY_H);
  s->cooldown=0;s->old_keys=0;
  press(s,key[dir]);assert(story_route(s,&dir,&steps)&&steps==before-1);
 }
 assert(s->x==x&&s->y==y);
}
static const unsigned konami[]={64u,64u,128u,128u,32u,16u,32u,16u,2u,1u,8u};
static void unlock_world(Story *s){
 if(!s->paused)press(s,8u);
 assert(s->paused&&!story_debug_menu(s));
 for(unsigned i=0;i<sizeof konami/sizeof konami[0];i++){
  press(s,konami[i]);
  assert(!!story_debug_menu(s)==(i+1u==sizeof konami/sizeof konami[0]));
 }
 assert(s->paused&&!s->quit);
}
static void debug_select(Story *s,unsigned row){
 unlock_world(s);
 for(unsigned i=0;i<row;i++)press(s,128u);
 press(s,1u);
}
static void verify_konami_lock(void){
 Story s;story_new_seeded(&s,0,37u);done(&s);
 unsigned seed=story_world_seed(),style=s.terrain_style,rev=story_brush_revision(s.slot);
 /* Former shortcut is no longer an unlock, even in the regular ROM. */
 press(&s,12u);assert(s.paused&&!story_debug_menu(&s));press(&s,8u);
 assert(!s.paused);
 /* A held direction counts once, never as the repeated UP press. */
 press(&s,8u);story_tick(&s,64u);
 for(unsigned n=0;n<60u;n++)story_tick(&s,64u);
 story_tick(&s,0);
 press(&s,128u);press(&s,128u);press(&s,32u);press(&s,16u);
 press(&s,32u);press(&s,16u);
 assert(!story_debug_menu(&s));
 /* Wrong sequence does not swallow the normal A field-guide action. */
 press(&s,1u);assert(s.paused&&!story_debug_menu(&s)&&(s.choice&0x40u));
 press(&s,2u);assert(s.paused&&!story_debug_menu(&s));
 /* Leaving the pause screen invalidates any partial code. */
 press(&s,64u);press(&s,64u);press(&s,8u);assert(!s.paused);
 press(&s,8u);for(unsigned i=2u;i<8u;i++)press(&s,konami[i]);
 assert(!story_debug_menu(&s));
 press(&s,8u);assert(!s.paused);
 unlock_world(&s);press(&s,2u);
 assert(!s.paused&&!story_debug_menu(&s)&&story_world_seed()==seed);
 assert(s.terrain_style==style&&story_brush_revision(s.slot)==rev);
 /* Loading a saved session does not retain the hidden UI or a prefix. */
 press(&s,8u);press(&s,64u);press(&s,64u);
 story_resume_seeded(&s,0,s.x,s.y,s.facing,s.view,s.flags,0,0,seed);done(&s);
 assert(!story_debug_menu(&s)&&!story_brush_mode(&s)&&!story_debug_survey(&s));
 press(&s,8u);for(unsigned i=2u;i<8u;i++)press(&s,konami[i]);
 assert(!story_debug_menu(&s));press(&s,8u);assert(!s.paused);
 unlock_world(&s);press(&s,2u);assert(!s.paused);
 puts("Konami lock: exact released-key sequence only, old shortcut blocked, wrong/held keys and pause/load reset, relocks on exit");
}
static void verify_debug_menu(void){
 Story s;story_new_seeded(&s,0,37u);done(&s);s.flags=STORY_HOME;s.art_steps=2u;
 stand(&s,50,7);
 unsigned seed=story_world_seed(),style=s.terrain_style;
 press(&s,4u);assert(!story_brush_mode(&s)&&!story_debug_menu(&s));
 s.view=0;
 debug_select(&s,0u);
 assert(story_debug_menu(&s)&&(s.choice&STORY_DEBUG_CONFIRM));
 unsigned proposed=s.studio_cursor;
 assert(story_world_seed()==seed&&proposed!=seed);
 press(&s,2u);assert(story_world_seed()==seed&&story_debug_menu(&s));
 press(&s,16u);assert(s.studio_cursor!=proposed);
 proposed=s.studio_cursor;press(&s,1u);press(&s,1u);
 assert(story_world_seed()==proposed&&story_debug_menu(&s));
 assert(s.x==50&&s.y==7&&s.flags==STORY_HOME&&s.art_steps==2u&&s.terrain_style==style);
 press(&s,2u);assert(!s.paused&&!story_debug_menu(&s));
 int x=20,y=11;story_world_place(&x,&y);stand(&s,x,y);s.facing=(uint8_t)story_world_facing(0u);
 debug_select(&s,0u);proposed=s.studio_cursor;press(&s,1u);press(&s,2u);
 int sx=20,sy=11;story_world_place(&sx,&sy);
 assert(story_world_seed()==proposed&&s.x==sx&&s.y==sy&&!strcmp(story_action_hint(&s),"A SHIFT"));
 stand(&s,44,8);s.view=0;
 press(&s,1u);done(&s);assert(s.terrain_style==style);
 debug_select(&s,2u);assert(story_debug_survey(&s));press(&s,1u);
 assert(story_land_level(&s,0)==2u);
 press(&s,4u);assert(!story_debug_survey(&s));
 debug_select(&s,1u);assert(story_brush_mode(&s));press(&s,1u);
 assert(story_brush_grade(&s)==2u);press(&s,4u);
 story_hike_restore(0,2u);
 debug_select(&s,3u);assert(story_debug_menu(&s));press(&s,1u);press(&s,2u);
 assert(s.terrain_style==STORY_LAND_DEFAULT&&story_brush_grade(&s)==1u&&story_hike_stage(0)==2u);
 assert(s.flags==STORY_HOME&&s.art_steps==2u);
 puts("Debug menu: preview/cancel/confirm, relocated spawn and facing, protected normal inputs, survey/brush/reset and preserved story");
}
static void verify_brush(void){
 Story s,other,resumed;story_new(&s,0);done(&s);
 stand(&s,50,7);s.facing=0;
 assert(!story_brush_editable(&s)&&story_brush_grade(&s)==1u);
 int initial=elevation(&s,50,7),shore=story_terrain_vertex(&s,47,16);
 unsigned revision=story_brush_revision(0);
 debug_select(&s,1u);assert(story_brush_mode(&s)&&story_brush_editable(&s)&&!s.dialogue&&!s.view);
 paint(&s,0);memcpy(prior,frame.p,HD_BYTES);
 press(&s,1u);press(&s,1u);press(&s,1u);
 assert(story_brush_grade(&s)==3u&&elevation(&s,50,7)>initial);
 assert(story_brush_revision(0)==revision+2u);
 paint(&s,0);assert(memcmp(prior,frame.p,HD_BYTES)!=0);
 paint(&s,1);memcpy(prior,frame.p,HD_BYTES);
 press(&s,2u);press(&s,2u);
 assert(story_brush_grade(&s)==1u&&elevation(&s,50,7)==initial);
 paint(&s,1);assert(memcmp(prior,frame.p,HD_BYTES)!=0);
 press(&s,3u);assert(story_brush_grade(&s)==1u);
 press(&s,1u);press(&s,1u);
 assert(story_terrain_vertex(&s,47,16)==shore);
 int raised=elevation(&s,50,7);
 press(&s,4u);assert(!story_brush_mode(&s));
 uint8_t data[BRUSH_EXT_LEN];brush_save_encode(&s,data);
 story_new(&other,1);done(&other);stand(&other,50,7);
 assert(story_brush_grade(&other)==1u&&elevation(&other,50,7)==initial);
 assert(story_brush_grade(&s)==3u&&elevation(&s,50,7)==raised);
 story_new(&resumed,0);done(&resumed);stand(&resumed,50,7);
 assert(story_brush_grade(&resumed)==1u&&brush_save_decode(&resumed,data));
 assert(story_brush_grade(&resumed)==3u&&elevation(&resumed,50,7)==raised);
 for(unsigned i=0;i<BRUSH_EXT_LEN;i++){
  data[i]^=1u;assert(!brush_save_decode(&resumed,data));data[i]^=1u;
 }
 assert(!brush_save_decode(&other,data));
 /* Even a valid CRC cannot authorize alteration of locked boundary nodes. */
 data[4]^=1u;data[BRUSH_EXT_LEN-1u]=brush_save_crc(data);
 assert(!brush_save_decode(&resumed,data)&&story_brush_grade(&resumed)==3u);
 stand(&resumed,40,8);debug_select(&resumed,1u);
 assert(story_brush_mode(&resumed)&&!story_brush_editable(&resumed));
 revision=story_brush_revision(0);press(&resumed,1u);press(&resumed,2u);
 assert(story_brush_revision(0)==revision);
 stand(&resumed,50,7);resumed.life_stage=STORY_LIFE_CLINIC;
 press(&resumed,1u);done(&resumed);
 assert(!story_brush_mode(&resumed)&&story_brush_revision(0)==revision);
 puts("Landscape brush: live pad controls, both cameras, shore protection, locked-node validation, two independent saved worlds");
}
static void verify_hike(void){
 Story s;story_new(&s,0);done(&s);
 stand(&s,42,7);
 press(&s,1u);done(&s);assert(story_hike_stage(0)==1u);
 walk_goal(&s,49,6,"MEADOW PATH");
 press(&s,1u);done(&s);assert(story_hike_stage(0)==2u);
 uint8_t saved[BRUSH_EXT_LEN];brush_save_encode(&s,saved);
 story_resume(&s,0,49,6,0,0,0,0,0);
 s.dialogue=0;assert(brush_save_decode(&s,saved)&&story_hike_stage(0)==2u);
 s.life_stage=STORY_LIFE_CLINIC;
 int gx,gy;const char *label=0;
 assert(story_goal(&s,&gx,&gy,&label)&&!strcmp(label,"CLINIC"));
 s.life_stage=STORY_LIFE_NONE;
 walk_goal(&s,53,13,"RIDGE TRAIL");
 press(&s,1u);done(&s);assert(story_hike_stage(0)==3u);
 walk_goal(&s,56,11,"SUMMIT");
 s.spark=1u;press(&s,1u);done(&s);
 assert(story_hike_stage(0)==4u&&s.spark==3u&&!s.art_steps);
 assert(story_goal(&s,&gx,&gy,&label)&&!strcmp(label,"LETTER"));
 stand(&s,42,7);press(&s,1u);done(&s);
 assert(story_hike_stage(0)==4u&&s.spark==3u);
 puts("Hike: physical trailhead, saved route progress, two intermediate checkpoints, summit reward and restored campaign routing");
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
 debug_select(&s,2u);assert(story_debug_survey(&s));
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
 verify_brush();
 verify_hike();
 verify_debug_menu();
 verify_konami_lock();
 return 0;
}
