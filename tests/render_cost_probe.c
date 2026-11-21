/* Informational host-side renderer profile: never confuse these CPU times
   with actual GBA FPS. The build still requires device/emulator timing QA. */
#include "hd_canvas.h"
#include "studio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static uint8_t pixels[HD_BYTES];
static volatile unsigned checksum;
static void render(unsigned kind,Story *s,unsigned frame){
 switch(kind){
  case 0:hd_world_draw(pixels,s,frame);break;
  case 1:hd_first_person_draw(pixels,s,frame);break;
  case 2:s->cinema=CINE_MARA;hd_cinema_draw(pixels,s,frame);break;
  case 3:s->cinema=CINE_RESCUE_SAVED;hd_cinema_draw(pixels,s,frame);break;
  case 4:s->activity=STORY_ACT_STUDIO;s->studio_tab=STUDIO_NOTES;
         hd_studio_draw(pixels,s,frame);break;
  case 5:s->ended=1;s->ending=STORY_END_BAD;
         s->rescue_stage=STORY_RESCUE_LOST;
         s->finale_frame=(uint16_t)(1200u+frame%200u);
         hd_ending_draw(pixels,s,s->finale_frame);break;
  default:assert(0);
 }
}
int main(void){
 static const char *const names[]={
  "overhead town","240-column 3D","Mara scene","rescue scene",
  "Studio notes","score-length ending"
 };
 const unsigned frames=18u;
 studio_reset();
 for(unsigned scene=0;scene<6;scene++){
  Story s;story_new(&s,0);
  s.x=12;s.y=13;s.facing=3;s.prev_x=s.x;s.prev_y=s.y;
  s.cinema=CINE_NONE;s.dialogue=0;s.activity=0;
  s.scene=STORY_SCENE_NONE;s.ended=0;s.life_stage=STORY_LIFE_NONE;
  render(scene,&s,0); /* remove first-use setup from measured warm path */
  clock_t start=clock();
  for(unsigned f=0;f<frames;f++){
   render(scene,&s,47u+f*11u);
   checksum+=(unsigned)pixels[(f*733u)%HD_BYTES];
   checksum^=(unsigned)pixels[((f+3u)*997u)%HD_BYTES];
  }
  clock_t end=clock();
  assert(end>=start);
  double ms=1000.0*(double)(end-start)/
              (double)CLOCKS_PER_SEC/(double)frames;
  printf("%-20s %.3f host CPU ms/frame (not GBA FPS)\n",names[scene],ms);
 }
 printf("render profile checksum: %u\n",checksum);
 return 0;
}
