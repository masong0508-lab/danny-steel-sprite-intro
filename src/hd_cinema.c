#include "hd_canvas.h"
#include "hd_actor_acting.h"
#include "hd_neighborhood_panels.h"
#include <string.h>

/* Carefully staged Old Quarter locations use the SAME narrative scene and
   page state as the original 120x80 director. They replace the most-seen
   cinema sets at real 240x160 while leaving specialized endings untouched. */
int hd_cinema_supports(unsigned scene){
 switch(scene){
  case CINE_OPEN:case CINE_LETTER:case CINE_BORE:
  case CINE_MARA:case CINE_BUS:case CINE_STAY:
  case CINE_MUSIC:case CINE_ART:case CINE_NOTICE:
  case CINE_CLINIC:case CINE_FOLLOWUP:case CINE_LAST_CLINIC:
   return 1;
  default:return hd_special_supports(scene);
 }
}
static unsigned hash(int x,int y){
 uint32_t n=(uint32_t)(x+137)*1664525u+(uint32_t)(y+101)*1013904223u;
 n^=n>>13;n*=2246822519u;return n^(n>>14);
}
static int ease(int from,int to,unsigned t,unsigned begin,unsigned span){
 if(t<=begin)return from;
 if(t>=begin+span)return to;
 unsigned f=(t-begin)*256u/span;
 unsigned smooth=f*f*(768u-2u*f)>>16;
 return from+(to-from)*(int)smooth/256;
}
static void dusk(uint8_t *p,unsigned frame,int night){
 /* Six contiguous bands are byte-for-byte identical to the old 160
    scanline fills, with 154 fewer rectangle dispatches per frame. These
    scenes run on the GBA CPU; reducing setup work matters more than
    adding another decorative layer or framebuffer. */
 hd_rect(p,0,  0,HD_W,37,night?19:17);
 hd_rect(p,0, 37,HD_W,34,night?24:25);
 hd_rect(p,0, 71,HD_W,20,night?HD_GLASS:HD_GLASS_LIT);
 hd_rect(p,0, 91,HD_W,26,HD_STONE_DARK);
 hd_rect(p,0,117,HD_W,18,HD_ROAD_COOL);
 hd_rect(p,0,135,HD_W,25,HD_ROAD);
 /* Warm layered cloud banks turn the dead-flat night sky into a location.
    Bands use the existing palette and drift only on coarse frame ticks. */
 /* Layered cloud silhouettes, moon/sun halo, and two roof depths.
    Individual windows belong to fixed walls, never random strobing. */
 hd_ellipse(p,186,34,12,12,night?HD_ROAD_COOL:HD_BRICK_LIT);
 hd_ellipse(p,186,34,9,9,night?HD_ROAD_LIGHT:HD_LAMP);
 hd_ellipse(p,184,32,6,6,night?HD_ROAD_LIGHT:HD_PAPER);
 for(int i=0;i<4;i++){
  int xx=24+i*57+(int)((frame/(i&1?109u:171u))%6u);
  int yy=18+(i*11)%28;
  unsigned low=night?HD_STONE_DARK:HD_BRICK_LIT;
  unsigned mid=night?HD_ROAD_COOL:HD_COPPER;
  unsigned high=night?HD_ROAD_LIGHT:HD_GOLD;
  hd_rect(p,xx-15,yy,33,2,low);
  hd_rect(p,xx-11,yy-3,26,3,mid);
  hd_rect(p,xx-5,yy-6,15,3,mid);
  hd_rect(p,xx+1,yy-8,6,2,high);
  hd_line(p,xx-8,yy-3,xx+11,yy-3,high);
 }
 for(int i=0;i<8;i++){
  int x=i*33-12,h=24+(int)(hash(i,23)%24u);
  hd_rect(p,x,106-h,32,h,HD_ROAD_COOL);
  hd_rect(p,x+4,103-h,17,3,HD_ROAD_COOL);
  hd_line(p,x+4,103-h,x+20,103-h,HD_ROAD_LIGHT);
 }
 for(int i=0;i<12;i++){
  int x=i*22-10,h=18+(int)(hash(i,11)%29u);
  hd_rect(p,x,112-h,21,h,i%3==0?HD_BRICK_DARK:HD_STONE_DARK);
  hd_rect(p,x,111-h,21,2,HD_SHADOW);
  hd_rect(p,x+2,112-h,2,h,HD_ROAD_COOL);
  if(i%3==1)hd_rect(p,x+12,108-h,4,4,HD_SHADOW);
  for(int j=5;j<19;j+=7)for(int yy=112-h+7;yy<106;yy+=11){
   unsigned lit=hash(i+j,yy)%7u;
   hd_rect(p,x+j,yy,3,4,lit<2u?HD_COPPER:HD_ROAD_COOL);
   if(lit==0u)hd_pixel(p,x+j+1,yy+1,HD_GOLD);
  }
 }
 /* Atmospheric power lines and a few real rooftops, not decorative HUD. */
 hd_line(p,0,84,239,84,HD_BRICK_DARK);
 hd_line(p,7,79,186,89,HD_STONE_DARK);
 for(int i=0;i<3;i++){
  int x=13+i*99;
  hd_rect(p,x,60,2,55,HD_SHADOW);
  hd_rect(p,x-4,61,9,2,HD_COPPER);
 }
}
static void sidewalk(uint8_t *p,unsigned frame){
 hd_rect(p,0,111,240,49,HD_ROAD);
 hd_rect(p,0,111,240,4,HD_STONE_LIT);
 hd_rect(p,0,115,240,2,HD_STONE_DARK);
 /* Quiet wet paving with partial seams: density follows stone edges,
    leaving space around the actor instead of a bright debug-like grid. */
 for(int i=0;i<18;i++){
  int x=(i*43+7)%238,y=122+(i*13)%34;
  hd_line(p,x,y,x+8+(i&3)*2,y,HD_ROAD_LIGHT);
  if(i%3==0)hd_line(p,x-1,y+1,x-3,y+5,HD_WET);
 }
 /* The bus stop's actual lamp, stable in every exterior scene. */
 hd_rect(p,213,42,3,74,HD_INK);
 hd_rect(p,209,42,12,3,HD_COPPER);
 hd_rect(p,210,45,10,4,HD_LAMP);
 hd_rect(p,211,116,9,2,HD_SHADOW);
 /* Broken wet reflections are anchored beneath the lamp. */
 hd_ellipse(p,214,130,18,6,HD_WET);
 hd_line(p,207,124,221,124,HD_ROAD_LIGHT);
 hd_line(p,210,128,218,128,HD_COPPER);
 hd_line(p,204,131,217,131,HD_REFLECT);
 hd_line(p,212,134,224,134,HD_ROAD_LIGHT);
 hd_line(p,208,139,215,139,HD_COPPER);
 if((frame/22u)&1u)hd_line(p,212,128,216,128,HD_GOLD);
 for(unsigned i=0;i<9u;i++){
  unsigned seed=hash((int)i,37);
  int x=(int)((seed+frame*2u)%236u),y=94+(int)((seed/13u+frame*3u)%49u);
  hd_line(p,x,y,x-1,y+3,HD_ROAD_COOL);
 }
}
static void street(uint8_t *p,unsigned frame,int night){
 dusk(p,frame,night);
 hd_rect(p,16,55,53,60,HD_BRICK_DARK);
 for(int y=60;y<110;y+=9){
  hd_line(p,17,y,68,y,HD_BRICK);
  for(int x=23+((y/9)&1)*10;x<67;x+=17)
   hd_line(p,x,y,x,y+7,HD_BRICK);
 }
 hd_rect(p,23,70,17,26,HD_SHADOW);
 hd_rect(p,25,72,13,22,HD_GLASS);
 hd_rect(p,47,70,15,26,HD_SHADOW);
 hd_rect(p,49,72,11,22,HD_GLASS_LIT);
 hd_line(p,31,73,31,94,HD_COPPER);
 hd_line(p,54,73,54,94,HD_COPPER);
 hd_rect(p,17,98,51,4,HD_STONE_LIT);
 hd_rect(p,0,104,240,8,HD_BRICK_DARK);
 sidewalk(p,frame);
}
static void bus(uint8_t *p,int x,int y,unsigned t){
 /* Larger authored bus with window muntins, reflectors and shadows. */
 hd_ellipse(p,x+56,y+40,62,5,HD_SHADOW);
 hd_rect(p,x,y+3,112,36,HD_RED_SHADE);
 hd_rect(p,x+3,y,106,5,HD_PLASTER);
 hd_line(p,x+6,y+1,x+103,y+1,HD_PAPER);
 hd_rect(p,x+3,y+5,106,26,HD_RED);
 hd_rect(p,x+6,y+7,97,17,HD_GLASS);
 hd_rect(p,x+6,y+7,97,3,HD_ROAD_COOL);
 hd_line(p,x+7,y+22,x+102,y+22,HD_BRICK_DARK);
 for(int k=0;k<4;k++){
  int xx=x+7+k*25;
  hd_rect(p,xx,y+7,1,18,HD_COPPER);
  hd_rect(p,xx+3,y+9,15,2,HD_GLASS_LIT);
 }
 hd_rect(p,x+7,y+26,96,2,HD_PLASTER_LIT);
 hd_rect(p,x+63,y+25,2,13,HD_BRICK_LIT);
 hd_rect(p,x+1,y+31,108,5,HD_BRICK_DARK);
 hd_rect(p,x+6,y+33,102,1,HD_COPPER);
 hd_ellipse(p,x+24,y+40,8,8,HD_INK);
 hd_ellipse(p,x+85,y+40,8,8,HD_INK);
 hd_ellipse(p,x+24,y+40,4,4,HD_STONE);
 hd_ellipse(p,x+85,y+40,4,4,HD_STONE);
 hd_rect(p,x+104,y+23,6,5,(t/32u)&1u?HD_LAMP:HD_PLASTER_LIT);
 hd_rect(p,x+9,y+29,18,2,HD_LAMP);
 hd_rect(p,x+91,y+7,12,8,HD_INK);
 hd_text(p,x+92,y+7,"01",HD_GOLD,1,0);
 hd_pixel(p,x+104,y+30,HD_PAPER);
}
/* All narrative characters share one connected skeletal renderer. */
static int scene_speaking(const Story *s,int role,int emotion){
 /* The authored gesture and script speaker are independent: a person can
    reach, walk or look worried while speaking. Silent listeners keep the
    same pose but never inherit the speaker's mouth animation. */
 if(!s->dialogue)return emotion==1;
 int count=0;const StoryPage *pages=story_pages(s,&count);
 if(!pages||s->page>=(unsigned)count)return 0;
 return hd_acting_speaker_role(pages[s->page].speaker)==(unsigned)role;
}
static void actor(uint8_t *p,const Story *s,int x,int feet,int who,unsigned t,int emotion){
 hd_actor_stage_spoken(p,x,feet,who,t,emotion,scene_speaking(s,who,emotion));
}
static void close_actor(uint8_t *p,const Story *s,int x,int feet,int who,unsigned t,int emotion){
 hd_actor_closeup_spoken(p,x,feet,who,t,emotion,scene_speaking(s,who,emotion));
}

/* The opening uses one continuous projected set and compact source-video
   joint measurements. Scene time never depends on dialogue-page rendering. */
#include "hd_potheads_set.h"
#include "hd_potheads_actor.h"
#include "hd_potheads_fixed.h"
static void potheads_open(uint8_t *p,unsigned t){
 PotCamera camera=pot_camera(t);
 if(t>=180u)pot_scene_fixed(p);
 else pot_scene(p,&camera,t);
 pot_dancer(p,&camera,t);
 hd_rect(p,0,135,HD_W,25,HD_INK); /* preserve source-camera crop */
}

static void cafe(uint8_t *p,unsigned t){
 street(p,t,1);
 hd_cafe_facade(p,4,38,t);
}
static void table(uint8_t *p,int x,int y,unsigned t){
 /* Two bistro chairs, bevelled wood, a pedestal and two ceramic cups.
    Rim positions retain Danny and Mara's scripted contact anchors. */
 hd_ellipse(p,x,y+21,36,6,HD_SHADOW);
 for(int side=-1;side<=1;side+=2){
  int q=x+side*43;
  hd_ellipse(p,q,y+25,10,3,HD_SHADOW);
  hd_rect(p,q-6,y-9,12,2,HD_BARK);
  hd_rect(p,q-7,y-8,2,18,HD_INK);
  hd_rect(p,q+5,y-8,2,18,HD_INK);
  hd_rect(p,q-6,y-4,12,2,HD_COPPER);
  hd_rect(p,q-8,y+8,16,3,HD_BARK);
  hd_line(p,q-6,y+11,q-8,y+23,HD_BARK);
  hd_line(p,q+6,y+11,q+8,y+23,HD_BARK);
 }
 hd_rect(p,x-29,y-4,58,5,HD_BARK);
 hd_rect(p,x-32,y-2,64,5,HD_COPPER);
 hd_rect(p,x-30,y+3,60,3,HD_BARK);
 hd_line(p,x-27,y-3,x+26,y-3,HD_GOLD);
 hd_line(p,x-26,y+1,x-8,y+1,HD_BRICK_LIT);
 hd_rect(p,x-3,y+6,6,18,HD_BARK);
 hd_line(p,x-1,y+23,x-19,y+26,HD_INK);
 hd_line(p,x+1,y+23,x+19,y+26,HD_INK);
 for(int i=0;i<2;i++){
  int xx=x+(i?20:-7);
  hd_ellipse(p,xx,y-3,7,2,HD_STONE_LIT);
  hd_rect(p,xx-4,y-10,9,6,HD_PAPER);
  hd_rect(p,xx-3,y-5,7,2,HD_PLASTER);
  hd_ellipse(p,xx,y-11,5,2,HD_COPPER);
  hd_line(p,xx-3,y-11,xx+3,y-11,HD_BARK);
  hd_rect(p,xx+5,y-9,3,3,HD_PLASTER_LIT);
  hd_pixel(p,xx+5,y-8,HD_BARK);
  if((t/80u+(unsigned)i)%3u==0u)
   hd_line(p,xx,y-14,xx+1,y-17,HD_ROAD_LIGHT);
 }
 hd_rect(p,x+3,y-3,10,5,HD_PAPER);
 hd_line(p,x+5,y-2,x+10,y-2,HD_STONE);
 hd_pixel(p,x+11,y,HD_RED_SHADE);
}
static void interior(uint8_t *p,unsigned t,int variant){
 /* Warm archive, deep blue studio and cooler clinic share a physically
    recessed window and converging floor joints, not a bright flat grid. */
 unsigned wall=variant==1?HD_STONE_DARK:variant==2?HD_ROAD_COOL:HD_BRICK_DARK;
 unsigned panel=variant==2?HD_STONE:variant==1?HD_ROAD:HD_BARK;
 unsigned trim=variant==2?HD_STONE_LIT:HD_COPPER;
 hd_rect(p,0,0,HD_W,112,wall);
 hd_rect(p,0,0,240,7,HD_SHADOW);
 hd_rect(p,0,7,240,2,panel);
 hd_rect(p,0,9,240,1,trim);
 for(int x=0;x<HD_W;x+=48){
  hd_rect(p,x,11,2,78,variant==2?HD_WET:HD_SHADOW);
  hd_rect(p,x+2,11,1,78,panel);
 }
 hd_rect(p,0,88,240,21,panel);
 hd_rect(p,0,88,240,2,trim);
 for(int x=3;x<240;x+=48){
  hd_rect(p,x,94,43,11,wall);
  hd_line(p,x+2,94,x+40,94,HD_SHADOW);
  hd_line(p,x+2,104,x+40,104,panel);
 }
 hd_rect(p,0,108,240,3,HD_SHADOW);
 hd_rect(p,0,111,HD_W,49,variant==2?HD_STONE_DARK:HD_WET);
 hd_line(p,0,116,239,116,HD_ROAD_COOL);
 hd_line(p,0,125,239,125,HD_ROAD_COOL);
 hd_line(p,0,139,239,139,HD_ROAD_COOL);
 hd_line(p,0,157,239,157,HD_ROAD_COOL);
 for(int i=-2;i<=2;i++)
  hd_line(p,120+i*17,111,120+i*54,159,HD_ROAD_COOL);
 hd_rect(p,13,15,95,77,HD_SHADOW);
 hd_rect(p,15,17,90,73,HD_BARK);
 hd_rect(p,18,20,84,67,HD_GLASS);
 hd_rect(p,18,20,84,19,HD_ROAD_COOL);
 for(int i=0;i<7;i++){
  int xx=19+i*12,h=20+(int)(hash(i,39)%19u);
  hd_rect(p,xx,86-h,12,h,HD_STONE_DARK);
  hd_rect(p,xx+2,86-h+6,3,3,HD_COPPER);
  if((i&1)==0)hd_pixel(p,xx+3,86-h+7,HD_LAMP);
 }
 for(int i=0;i<5;i++){
  int xx=22+i*16,yy=27+(i*11+(int)(t/12u)%13)%22;
  hd_line(p,xx,yy,xx-1,yy+5,HD_ROAD_LIGHT);
 }
 hd_rect(p,59,20,3,69,HD_BARK);
 hd_line(p,60,21,60,86,HD_COPPER);
 hd_rect(p,18,64,85,3,HD_BARK);
 hd_line(p,19,64,101,64,trim);
 hd_rect(p,14,87,93,4,HD_SHADOW);
 hd_rect(p,13,87,95,2,trim);
 hd_line(p,26,119,93,119,HD_ROAD_COOL);
 hd_line(p,36,124,76,124,HD_ROAD_LIGHT);
 hd_rect(p,140,25,85,6,HD_SHADOW);
 hd_rect(p,141,25,83,2,HD_GOLD);
}
static void archive(uint8_t *p,const Story *s,unsigned t,unsigned page){
 interior(p,t,0);
 hd_rect(p,144,37,77,63,HD_INK);
 for(int i=0;i<7;i++){
  hd_rect(p,149+i*10,43,7,32,HD_BRICK);
  hd_rect(p,149+i*10,46,7,2,HD_GOLD);
  hd_rect(p,149+i*10,78,7,3,HD_STONE_LIT);
 }
 hd_rect(p,118,93,119,9,HD_BARK);
 hd_rect(p,127,99,7,33,HD_BARK);
 hd_rect(p,213,99,7,33,HD_BARK);
 hd_rect(p,134,86,32,11,HD_PAPER);
 hd_rect(p,138,89,23,1,HD_STONE_DARK);
 hd_rect(p,141,91,21,1,HD_STONE_DARK);
 hd_rect(p,182,87,16,12,HD_COPPER);
 hd_rect(p,186,83,8,5,HD_PAPER);
 /* The form, approval stamp and connected clerk hand share a desk. */
 hd_rect(p,150,81,25,13,HD_PAPER);
 hd_rect(p,152,83,18,1,HD_STONE);
 hd_rect(p,152,86,16,1,HD_STONE);
 hd_rect(p,152,89,11,1,HD_STONE);
 if(page>=2u){
  hd_ellipse(p,163,89,5,3,HD_RED_SHADE);
  hd_pixel(p,163,88,HD_GOLD);
 }
 hd_rect(p,178,84,10,7,HD_INK);
 hd_rect(p,180,82,6,5,HD_BARK);
 actor(p,s,ease(64,76,t,8,60),137,1,t,page==0?2:page>=2?3:1);
 actor(p,s,193,135,3,t,page&1u?1:3);
 if(page>=1u){
  hd_line(p,176,107,165,97,HD_STONE_LIT);
  hd_line(p,175,106,165,96,HD_PLASTER_LIT);
  hd_ellipse(p,164,96,3,2,HD_SKIN);
 }
 hd_line(p,126,32,237,32,HD_STONE_DARK);
 hd_text(p,143,14,"BORE / THE ARCHIVE",HD_PAPER,1,0);
}
static void record_room(uint8_t *p,const Story *s,unsigned t){
 unsigned phase=s->song_steps;
 if(phase>STORY_PROJECT_DONE)phase=STORY_PROJECT_DONE;
 unsigned take_ink=(s->project_style&STORY_STYLE_SONG_SOFT)?
                  HD_COPPER:HD_GLASS_LIT;
 interior(p,t,1);
 hd_rect(p,127,29,102,71,HD_INK);
 hd_rect(p,130,32,95,64,HD_STONE_DARK);
 for(int i=0;i<6;i++){
  int x=134+i*15;int level=(int)((t/7u+(unsigned)i*3u)%20u);
  if(level>10)level=20-level;
  hd_rect(p,x,66-level,8,level+5,level>5?HD_LAMP:HD_STONE_LIT);
  hd_rect(p,x,81,8,1,HD_STONE);
 }
 hd_rect(p,19,102,208,10,HD_BARK);
 hd_rect(p,26,112,8,35,HD_BARK);
 hd_rect(p,212,112,8,35,HD_BARK);
 hd_ellipse(p,70,100,31,10,HD_INK);
 hd_ellipse(p,70,100,26,8,HD_BRICK_DARK);
 hd_ellipse(p,70,100,20,6,HD_STONE_DARK);
 hd_ellipse(p,70,100,7,4,HD_GOLD);
 int px=(int)(t/7u&3u);
 hd_line(p,70,100,76+px,97,HD_PAPER);
 hd_rect(p,97,80,3,16,HD_COPPER);
 hd_line(p,99,82,87,92,HD_COPPER);
 hd_rect(p,150,87,46,12,HD_BARK);
 hd_rect(p,153,85,40,9,HD_PAPER); /* a real illustrated sleeve */
 hd_rect(p,155,87,13,5,HD_GOLD);
 hd_line(p,171,88,190,88,HD_STONE_DARK);
 hd_line(p,171,90,187,90,HD_STONE);
 if(phase>=STORY_PROJECT_DONE){
  hd_rect(p,153,85,40,1,HD_GOLD);
  hd_rect(p,155,87,13,5,take_ink);
  hd_line(p,169,93,191,93,HD_GOLD);
 }
 hd_rect(p,122,31,3,68,HD_COPPER);
 hd_rect(p,130,28,94,2,HD_STONE_LIT);
 /* The console keeps an actual four-channel session ledger. Each notch
    follows the saved take count; the soft arrangement uses a warm ink. */
 hd_rect(p,134,35,86,13,HD_SHADOW);
 hd_text(p,138,38,"TAKES",HD_PAPER,1,0);
 for(unsigned j=0;j<4u;j++){
  int x=167+(int)j*12;
  hd_rect(p,x,38,10,7,HD_STONE_DARK);
  hd_rect(p,x+1,39,8,5,j<phase?take_ink:HD_INK);
  if(j==phase)hd_pixel(p,x+4,36,HD_LAMP);
 }
 for(int j=0;j<5;j++)
  hd_rect(p,136+j*17,76,11,2,HD_RED_SHADE);
 actor(p,s,38,136,1,t,t<80?2:3);
}
static void rooftop(uint8_t *p,const Story *s,unsigned t,unsigned page){
 dusk(p,t,0);
 hd_rect(p,0,103,HD_W,57,HD_ROAD);
 hd_rect(p,0,102,HD_W,4,HD_STONE);
 for(int i=0;i<6;i++){
  int x=i*41;
  hd_line(p,x,109,x+14,156,HD_ROAD_COOL);
  hd_rect(p,x,145,36,1,HD_ROAD_LIGHT);
 }
 /* Lit parapet string and foreground rooftop greenery frame the artist. */
 hd_rect(p,0,75,HD_W,4,HD_COPPER);
 for(int i=0;i<6;i++){
  int bx=12+i*41;
  hd_line(p,bx,73,bx+22,73,HD_STONE_DARK);
  hd_rect(p,bx+4,74,3,3,(i&1)?HD_LAMP:HD_GOLD);
 }
 for(int i=0;i<3;i++){
  int px=8+i*109;
  hd_rect(p,px,121,17,14,HD_BRICK);
  hd_rect(p,px+2,121,13,3,HD_BRICK_LIT);
  hd_line(p,px+8,119,px+8,105,HD_LEAF_DARK);
  hd_line(p,px+8,109,px+1,101,HD_LEAF);
  hd_line(p,px+8,109,px+17,98,HD_LEAF_LIT);
 }
 for(int x=12;x<240;x+=27)hd_rect(p,x,77,2,37,HD_COPPER);
 hd_line(p,0,105,239,105,HD_STONE_DARK);
 /* A real stage-dependent painting on the same easel, never a generic
    full-color thumbnail at stage zero. Progress and alternate pigment
    are read directly from the existing saved project fields. */
 unsigned phase=s->art_steps;
 if(phase>STORY_PROJECT_DONE)phase=STORY_PROJECT_DONE;
 unsigned warm=s->project_style&STORY_STYLE_ART_WARM;
 hd_rect(p,151,52,67,51,HD_BARK);
 hd_rect(p,155,56,59,43,HD_PAPER);
 if(phase==0u){
  hd_rect(p,158,59,53,32,HD_PLASTER);
  hd_line(p,158,89,210,89,HD_STONE);
  hd_line(p,160,85,177,78,HD_STONE_LIT);
  hd_line(p,177,78,190,83,HD_STONE_LIT);
  hd_line(p,190,83,207,76,HD_STONE_LIT);
  hd_line(p,160,95,208,95,HD_MORTAR);
 }else{
  /* First layer: atmosphere. Second: masonry silhouettes. Third:
     individual windows. Fourth: illuminated streets and final frame. */
  hd_rect(p,158,59,53,32,warm?HD_COPPER:HD_GLASS);
  hd_rect(p,158,59,53,5,warm?HD_GOLD:HD_GLASS_LIT);
  hd_rect(p,158,82,53,9,warm?HD_BRICK_LIT:HD_REFLECT);
  hd_ellipse(p,195,67,7,7,HD_LAMP);
  if(phase>=2u){
   for(int i=0;i<9;i++){
    int x=158+i*6,h=(i*13)%17+7;
    hd_rect(p,x,91-h,6,h,HD_STONE_DARK);
    hd_rect(p,x,91-h,5,1,warm?HD_COPPER:HD_STONE_LIT);
    if(phase>=3u){
     hd_rect(p,x+2,87-h,2,2,HD_LAMP);
     if(i&1)hd_pixel(p,x+2,89-h,HD_GOLD);
    }
   }
  }
  hd_rect(p,157,90,55,7,phase>=3u?HD_LEAF:HD_STONE);
  if(phase>=STORY_PROJECT_DONE){
   hd_line(p,159,94,209,94,HD_GOLD);
   hd_line(p,162,93,174,92,HD_PAPER);
   hd_line(p,190,96,205,95,HD_COPPER);
   hd_rect(p,152,53,65,1,HD_GOLD);
   hd_rect(p,152,101,65,1,HD_COPPER);
  }
 }
 hd_rect(p,169,105,7,35,HD_BARK);
 hd_line(p,171,113,156,147,HD_BARK);
 hd_line(p,175,113,189,147,HD_BARK);
 hd_rect(p,208,135,12,9,HD_BRICK_DARK);
 hd_rect(p,211,130,6,6,HD_RED_SHADE);
 /* Advance to the same canvas edge before touching it. The brush
    extends from the raised hand and pigments appear only on contact. */
 int painter=ease(78,128,t,7,78);
 int reaction=page>=1u&&t>=192u;
 if(reaction)close_actor(p,s,106,143,1,t,5);
 else actor(p,s,painter,142,1,t,t<92u?2:3);
 if(t>=92u){
  /* Foreground reaction shots do not ERASE the work that was painted
     before the camera cut. Only the physically absent brush is hidden. */
  if(!reaction){
   int hand=painter+23;
   hd_line(p,hand,93,hand+8,87,HD_BARK);
   hd_pixel(p,hand+8,87,HD_RED_SHADE);
  }
  int strokes=(int)((t-92u)/32u);
  if(strokes>6)strokes=6;
  for(int j=0;j<strokes;j++){
   int px=160+j*7;
   hd_line(p,px,78-(j&1)*3,px+5,76+(j&1)*3,
           j&1?HD_COPPER:HD_RED_SHADE);
  }
 }
}
static void clinic(uint8_t *p,const Story *s,unsigned t,int last){
 interior(p,t,2);
 hd_rect(p,146,31,77,68,HD_PLASTER);
 hd_rect(p,150,35,69,60,HD_GLASS_LIT);
 hd_line(p,151,64,217,64,HD_STONE);
 hd_line(p,177,36,177,94,HD_STONE);
 hd_rect(p,150,105,77,11,HD_PAPER);
 hd_rect(p,163,102,16,7,HD_STONE);
 hd_rect(p,181,103,29,4,HD_BRICK_DARK);
 hd_rect(p,184,96,21,10,HD_PAPER);
 for(int i=0;i<4;i++)hd_line(p,185,98+i*2,202,98+i*2,HD_ROAD_COOL);
 /* The clinic gains specific working equipment and a warmer lighting key. */
 hd_rect(p,12,11,112,3,HD_STONE_LIT);
 hd_rect(p,17,14,102,4,HD_PAPER);
 hd_rect(p,17,18,102,2,HD_GOLD);
 for(int j=0;j<4;j++){
  int yy=35+j*13;
  hd_rect(p,114,yy,26,3,HD_BARK);
  hd_rect(p,117,yy-7,7,7,HD_GLASS);
  hd_rect(p,128,yy-6,7,6,j&1?HD_COPPER:HD_LEAF);
 }
 hd_rect(p,21,93,13,14,HD_BARK);
 hd_line(p,27,92,27,72,HD_LEAF_DARK);
 hd_line(p,27,82,15,75,HD_LEAF_LIT);
 hd_line(p,27,82,38,73,HD_LEAF);
 /* Consultation chart, desk and patient-clinician line of action. */
 hd_rect(p,83,107,123,5,HD_BARK);
 hd_rect(p,87,111,6,27,HD_BARK);
 hd_rect(p,197,111,6,27,HD_BARK);
 hd_rect(p,115,98,39,11,HD_PAPER);
 hd_line(p,120,101,146,101,HD_STONE);
 hd_line(p,120,104,140,104,HD_STONE);
 hd_rect(p,126,93,15,5,HD_COPPER);
 hd_rect(p,142,96,14,8,HD_GLASS_LIT);
 hd_pixel(p,146,98,HD_RED_SHADE);
 hd_rect(p,154,36,60,51,HD_PAPER);
 hd_rect(p,156,38,56,47,HD_GLASS_LIT);
 for(int i=0;i<5;i++)
  hd_line(p,160,47+i*7,207,47+i*7,HD_STONE_LIT);
 hd_line(p,162,75,169,69,HD_RED_SHADE);
 hd_line(p,169,69,178,76,HD_RED_SHADE);
 hd_line(p,178,76,189,61,HD_RED_SHADE);
 /* No unverified living-person appearance while Mara is missing. */
 int missing=hd_mara_absent(s);
 /* Shot/reverse-shot follows the person who is ACTUALLY speaking:
    first visit: doctor / Danny / doctor.
    last visit: doctor / Mara / Danny, but never invent a rescued Mara. */
 int doctor_close=(s->page==1u || (!last&&s->page==3u))&&t>=120u;
 int danny_close=(last?(s->page==3u||(missing&&s->page==2u)):s->page==2u)&&t>=150u;
 int mara_close=last&&!missing&&s->page==2u&&t>=150u;
 if(danny_close)close_actor(p,s,78,145,1,t,last?5:7);
 else actor(p,s,68,135,1,t,last?7:1);
 if(last&&!missing){
  if(mara_close)close_actor(p,s,125,145,2,t,5);
  else actor(p,s,115,136,2,t,5);
 }
 if(doctor_close){
  close_actor(p,s,184,142,5,t,1);
 }else{
  actor(p,s,198,133,5,t,1);
  /* In a wide shot, the doctor makes contact with the chart. */
  hd_line(p,181,106,157,102,HD_PLASTER_LIT);
  hd_ellipse(p,157,101,3,2,HD_SKIN);
 }
}
void hd_cinema_draw(uint8_t *p,const Story *s,unsigned t){
 int panel=story_neighborhood_panel(s);
 if(panel>=0&&(unsigned)panel<STORY_NEIGHBORHOOD_COUNT*3u){
  memcpy(p,hd_neighborhood_art+(unsigned)panel*HD_NEIGHBORHOOD_PANEL_BYTES,
         HD_NEIGHBORHOOD_PANEL_BYTES);
  hd_rect(p,0,104,HD_W,HD_H-104,HD_INK);
  return;
 }
 unsigned sc=s->cinema,page=s->page;
 switch(sc){
  case CINE_OPEN:
   potheads_open(p,t);
   break;
  case CINE_LETTER:{
   street(p,t,1);
   hd_rect(p,9,56,91,59,HD_BARK);
   hd_rect(p,12,59,85,53,HD_BRICK);
   for(int i=0;i<5;i++)
    hd_line(p,14,67+i*9,96,67+i*9,HD_BRICK_DARK);
   hd_rect(p,18,75,43,37,HD_INK);
   hd_rect(p,21,78,37,31,HD_GLASS_LIT);
   hd_line(p,38,78,38,109,HD_BARK);
   hd_rect(p,65,75,23,36,HD_STONE_DARK);
   hd_rect(p,67,76,20,4,HD_COPPER);
   hd_rect(p,69,81,16,16,HD_SHADOW);
   int dx=ease(174,141,t,10,70);
   /* Same counter and letter for all dialogue pages. */
   hd_rect(p,153,111,70,7,HD_BARK);
   hd_rect(p,155,117,5,29,HD_BARK);
   hd_rect(p,217,117,5,29,HD_BARK);
   hd_rect(p,165,91,24,17,HD_PAPER);
   hd_rect(p,166,92,22,2,HD_STONE_LIT);
   hd_line(p,166,94,177,103,HD_COPPER);
   hd_line(p,188,94,177,103,HD_COPPER);
   hd_rect(p,176,97,3,2,HD_RED_SHADE);
   int reaction=page>=3u&&t>=150u;
   if(reaction)close_actor(p,s,132,143,1,t,7);
   else actor(p,s,dx,145,1,t,t<85u?2:page==0?1:3);
   if(!reaction&&t>=85u&&page>=1u){
    hd_line(p,dx+23,96,166,97,HD_SKIN);
    hd_pixel(p,166,97,HD_SKIN_LIT);
   }
   break;
  }
  case CINE_BORE:archive(p,s,t,page);break;
  case CINE_MARA:
  case CINE_STAY:{
   cafe(p,t);
   /* Background commuter: connected legs and depth BEFORE the actors. */
   int passer=235-(int)((t/5u)%80u);
   int stride=((t/8u)&1u)?2:-2;
   hd_ellipse(p,passer,113,7,2,HD_SHADOW);
   hd_line(p,passer-1,104,passer-3+stride,112,HD_INK);
   hd_line(p,passer+1,104,passer+3-stride,112,HD_INK);
   hd_rect(p,passer-3+stride,112,4,1,HD_SHADOW);
   hd_rect(p,passer+2-stride,112,4,1,HD_SHADOW);
   hd_rect(p,passer-3,95,7,10,HD_STONE_DARK);
   hd_line(p,passer-2,96,passer-5-stride,103,HD_ROAD_COOL);
   hd_rect(p,passer-2,89,5,6,HD_SKIN);
   hd_rect(p,passer-2,89,5,2,HD_HAIR);
   table(p,121,110,t);
   int danny=ease(78,91,t,11,60);
   int mara=ease(178,164,t,18,63);
   actor(p,s,danny,141,1,t,t<70?2:page>=1?5:1);
   int reaction=page>=3u&&t>=145u;
   if(reaction)close_actor(p,s,178,145,2,t,page>=4u?1:5);
   else actor(p,s,mara,141,2,t,t<82?2:page>=2?3:1);
   /* Body gestures have real table targets instead of floating cups. */
   if(t>=86u&&page>=1u){
    hd_line(p,danny+18,108,114,99,HD_SKIN);
    hd_pixel(p,114,99,HD_SKIN_LIT);
   }
   if(!reaction&&t>=104u&&page>=2u){
    hd_line(p,mara-18,108,141,99,HD_SKIN);
    hd_pixel(p,141,99,HD_SKIN_LIT);
   }
   break;
  }
  case CINE_BUS:{
   street(p,t,1);
   /* Arrival, settle and opening door are sequential physical events. */
   int bx=ease(260,74,t,0,116);
   bus(p,bx,67,t);
   hd_rect(p,201,68,30,19,HD_INK); /* bus route timetable */
   hd_rect(p,203,70,26,14,HD_PAPER);
   hd_line(p,207,74,224,74,HD_STONE);
   hd_line(p,207,78,220,78,HD_STONE);
   hd_rect(p,200,88,3,25,HD_BARK);
   if(t>=116u){
    int gap=ease(0,12,t,116,36);
    hd_rect(p,bx+66,92,20,12,HD_INK);
    hd_rect(p,bx+68,94,16,9,HD_STONE_DARK);
    hd_rect(p,bx+68,94,8-gap/3,9,HD_RED);
    hd_rect(p,bx+84-gap/3,94,2,9,HD_COPPER);
    hd_rect(p,bx+85,103,4,9,HD_STONE_LIT);
    hd_line(p,bx+68,113,bx+92,113,HD_LAMP);
   }
   int danny=ease(192,165,t,20,78);
   if(page>=2u&&t>=170u)close_actor(p,s,162,143,1,t,7);
   else actor(p,s,danny,142,1,t,t<100u?2:page>=2?7:0);
   break;
  }
  case CINE_MUSIC:record_room(p,s,t);break;
  case CINE_ART:rooftop(p,s,t,page);break;
  case CINE_NOTICE:{
   street(p,t,1);
   int x=ease(68,91,t,13,71);
   actor(p,s,x,142,1,t,t<80?2:7);
   /* Phone aligns with Danny's raised hand at (x+19,85). */
   hd_rect(p,x+18,75,11,20,HD_INK);
   hd_rect(p,x+20,77,7,16,HD_GLASS_LIT);
   hd_pixel(p,x+23,80,HD_LAMP);
   hd_line(p,x+19,85,x+20,87,HD_SKIN);
   break;
  }
  case CINE_CLINIC:clinic(p,s,t,0);break;
  case CINE_LAST_CLINIC:clinic(p,s,t,1);break;
  case CINE_FOLLOWUP:{
   /* A recognizable clinic on the SAME rainy Old Quarter street: an
      awning with physical depth, lit vestibule, backlit clinic name,
      accessible entrance, and grounded warm/cool wet-stone reflections.
      Three editorial framings make Danny's appointment meaningful. */
   street(p,t,1);
   hd_rect(p,10,47,119,66,HD_INK);
   hd_rect(p,13,49,113,61,HD_PLASTER);
   hd_rect(p,15,53,109,7,HD_STONE_DARK);
   hd_rect(p,17,55,105,3,HD_GLASS_LIT);
   hd_text(p,33,52,"CLINIC",HD_PAPER,1,0);
   hd_rect(p,19,63,16,36,HD_BARK);
   hd_rect(p,21,65,12,32,HD_GLASS_LIT);
   hd_line(p,22,77,31,77,HD_PAPER);
   hd_rect(p,41,61,72,45,HD_SHADOW);
   hd_rect(p,44,64,66,40,HD_GLASS);
   hd_rect(p,47,66,60,35,HD_STONE_DARK);
   hd_rect(p,50,69,54,31,HD_GLASS_LIT);
   hd_rect(p,75,67,2,35,HD_STONE);
   hd_rect(p,52,79,21,19,HD_GLASS);
   hd_rect(p,80,79,22,19,HD_GLASS);
   hd_rect(p,70,85,4,2,HD_COPPER);
   hd_rect(p,79,85,4,2,HD_COPPER);
   hd_line(p,44,106,110,106,HD_PAPER);
   hd_rect(p,40,108,75,4,HD_STONE_LIT);
   /* Ceiling lights and a plant are visible THROUGH the glazed door.
      The silhouettes are fixed relative to the building, not the actor. */
   hd_rect(p,55,71,38,2,HD_GOLD);
   hd_rect(p,61,75,21,1,HD_PAPER);
   hd_rect(p,86,85,5,14,HD_BARK);
   hd_line(p,88,86,88,76,HD_LEAF_DARK);
   hd_line(p,88,80,82,77,HD_LEAF_LIT);
   hd_line(p,88,80,94,73,HD_LEAF);
   /* External clinic sign projects away from the masonry with brackets. */
   hd_rect(p,117,61,17,23,HD_INK);
   hd_rect(p,119,63,13,19,HD_PAPER);
   hd_rect(p,124,65,3,16,HD_RED);
   hd_rect(p,121,71,9,4,HD_RED);
   hd_line(p,111,64,118,64,HD_COPPER);
   hd_line(p,111,83,118,83,HD_COPPER);
   /* Exterior arrival hall: benches, drain and a sheltered pavement
      reflection converge toward the door rather than floating in space. */
   hd_rect(p,7,107,29,3,HD_BARK);
   hd_rect(p,11,110,3,17,HD_BARK);
   hd_rect(p,30,110,3,17,HD_BARK);
   hd_rect(p,14,103,16,3,HD_COPPER);
   hd_line(p,41,117,105,117,HD_STONE_LIT);
   hd_line(p,52,123,97,123,HD_ROAD_LIGHT);
   hd_line(p,61,132,87,132,HD_REFLECT);
   for(int j=0;j<4;j++){
    int px=46+j*18;
    hd_line(p,px,116,px-4,131,HD_STONE_DARK);
   }
   int danny=ease(139,156,t,12,74);
   int mara=ease(193,212,t,23,78);
   /* Shot/reverse-shot is tied to spoken meaning. Page 0 establishes
      the appointment, then Mara's offer, then Danny asking for company.
      Background actors remain grounded when the foreground reacts. */
   int absent=hd_mara_absent(s);
   int mara_close=!absent&&page==1u&&t>=91u;
   int danny_close=page>=2u&&t>=172u;
   if(absent){
    /* The single actor cannot duplicate himself or silently confirm
       Mara's safety while the unanswered-phone plot is unresolved. */
    if(page>=1u&&t>=91u)
     close_actor(p,s,176,145,1,t,7);
    else actor(p,s,danny,142,1,t,t<89u?2:7);
   }else{
    if(danny_close)close_actor(p,s,157,145,1,t,7);
    else actor(p,s,danny,142,1,t,t<89u?2:page>=2u?7:0);
    if(mara_close)close_actor(p,s,176,145,2,t,5);
    else actor(p,s,mara,142,2,t,t<104u?2:5);
   }
   break;
  }
  default:
   if(!hd_special_cinema_draw(p,s,t))street(p,t,1);
   break;
 }
 /* Filmic edge line only; the native subtitle layer owns the readable
    bottom panel. Original scene script governs which line is spoken. */
 hd_rect(p,0,0,240,2,HD_INK);
 hd_rect(p,0,158,240,2,HD_INK);
}

