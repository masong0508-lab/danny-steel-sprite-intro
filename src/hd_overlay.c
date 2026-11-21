#include "hd_canvas.h"

/* Native-pixel film subtitles, menus and prompts shared by the painted map
   and legacy cinematic/raycast frames. Narrative text is never removed. */
static void framebox(uint8_t *p,int x,int y,int w,int h){
 hd_rect(p,x+2,y+2,w,h,HD_SHADOW);
 hd_rect(p,x,y,w,h,HD_INK);
 hd_rect(p,x,y,w,1,HD_COPPER);
 hd_rect(p,x,y,2,h,HD_COPPER);
 hd_rect(p,x+4,y+h-2,w-8,1,HD_STONE_DARK);
}
static void selectable(uint8_t *p,int x,int y,int w,const char *label,int selected){
 hd_rect(p,x,y,w,21,selected?HD_ROAD_COOL:HD_SHADOW);
 hd_rect(p,x,y,selected?3:1,21,selected?HD_LAMP:HD_STONE_DARK);
 hd_text(p,x+9,y+7,label,selected?HD_PAPER:HD_STONE_LIT,1,1);
}
void hd_game_overlay(uint8_t *p,const Story *s){
 if(s->activity==STORY_ACT_STUDIO||s->battle||
    s->life_stage==STORY_LIFE_FINALE||s->ended)return;
 if(s->dialogue){
  int count=0;const StoryPage *pages=story_pages(s,&count);
  if(pages && s->page<(unsigned)count){
   const StoryPage *d=&pages[s->page];
   framebox(p,8,103,224,54);
   hd_text(p,18,111,d->speaker,s->cinema==CINE_MARA?HD_SKIN_LIT:HD_LAMP,1,1);
   hd_line(p,18,120,222,120,HD_STONE_DARK);
   hd_text(p,18,124,d->line1,HD_PAPER,1,1);
   hd_text(p,18,136,d->line2,HD_PAPER,1,1);
   hd_text(p,174,147,"A NEXT",HD_STONE_LIT,1,0);
   for(int i=0;i<count&&i<9;i++)
    hd_rect(p,205+i*3,112,2,3,i<=s->page?HD_LAMP:HD_STONE_DARK);
  }
  return;
 }
 if(s->paused && !(s->choice&0x80u)){
  framebox(p,16,67,208,87);
  hd_text(p,29,79,"A MOMENT TO BREATHE",HD_PAPER,1,1);
  hd_line(p,29,92,210,92,HD_STONE_DARK);
  hd_text(p,30,101,"START  RETURN TO STREET",HD_PAPER,1,0);
  hd_text(p,30,116,"R  LIFE AND CONSEQUENCES",HD_STONE_LIT,1,0);
  hd_text(p,30,131,"B  SAVE AND LEAVE",HD_LAMP,1,0);
  return;
 }
 if(s->risk_menu){
  framebox(p,13,76,214,80);
  hd_text(p,23,85,"A CHOICE WITH CONSEQUENCES",HD_LAMP,1,0);
  hd_text(p,23,98,s->risk_menu=='L'?"THE LETTER":s->risk_menu=='C'?
   "THE ARCHIVE":s->risk_menu=='M'?"THE MUSIC":s->risk_menu=='H'?
   "MARA AT THE DOOR":"THE TERMINAL",HD_PAPER,1,1);
  hd_text(p,23,109,s->risk_choice?"THIS CAN CAUSE HARM":"YOU CAN WALK AWAY",HD_STONE_LIT,1,0);
  selectable(p,23,121,91,"BACK",!s->risk_choice);
  selectable(p,122,121,92,s->risk_menu=='H'?"LEAVE":"DO IT",s->risk_choice);
  hd_text(p,28,146,"LEFT/RIGHT CHOOSE    B CANCEL",HD_PAPER,1,0);
  return;
 }
 if(s->activity){
  const char *where="OLD QUARTER",*left="CONTINUE",*right="REST",*benefit="";
  switch(s->activity){
  case 'F':where="MARA / AFTERNOON";left="WALK";right="TALK";benefit="TIME TOGETHER";break;
  case 'L':where="LETTER / MEMORY";left="WRITE";right="KEEP";benefit="A MEMORY OF HOME";break;
  case 'C':where="BORE / ARCHIVE";left="FILE";right="JOKE";benefit="ANOTHER SMALL FORM";break;
  case 'M':where="BORE / MUSIC";left="ARRANGE";right="LISTEN";benefit="ONE MORE SONG";break;
  case 'E':where="MUSIC / MASTER";left="LISTEN";right="STUDIO";benefit="YOUR FINISHED MUSIC";break;
  case 'm':where="MUSIC / TAKE";left="BUILD";right="SOFTEN";benefit="THE WORK CONTINUES";break;
  case 'r':where="ROOFTOP / ART";left="LAYER";right="LIGHT";benefit="MAKE THE CITY YOURS";break;
  default:where="ROOFTOP / THE SKY";left="PAINT";right="REST";benefit="A QUIET AFTERNOON";break;
  }
  framebox(p,12,75,216,81);
  hd_text(p,22,85,where,HD_LAMP,1,1);
  hd_text(p,22,99,benefit,HD_PAPER,1,0);
  selectable(p,22,115,92,left,!s->activity_choice);
  selectable(p,123,115,93,right,s->activity_choice);
  hd_text(p,25,145,"LEFT/RIGHT CHOOSE  A OK  B BACK",HD_STONE_LIT,1,0);
  return;
 }
 if(s->choosing){
  framebox(p,13,79,214,77);
  hd_text(p,23,90,"THE LAST BUS",HD_LAMP,1,1);
  hd_text(p,23,103,"LEAVE OR STAY A LITTLE LONGER",HD_PAPER,1,0);
  selectable(p,23,118,92,"GO",!s->choice);
  selectable(p,123,118,91,"STAY",s->choice);
  hd_text(p,23,145,"LEFT/RIGHT   A CHOOSE   B BACK",HD_STONE_LIT,1,0);
  return;
 }
 /* Refresh only the UI portions of low-res first-person views. No rows
    are overwritten in the detailed overhead scene except the common HUD. */
 if(s->view){
  int gx=0,gy=0;const char *goal="";
  int has=story_goal(s,&gx,&gy,&goal);
  hd_rect(p,0,0,240,18,HD_INK);
  hd_rect(p,0,17,240,1,HD_COPPER);
  hd_text(p,8,5,"OLD QUARTER",HD_PAPER,1,1);
  hd_text(p,110,5,has?goal:"LOOK AROUND",HD_LAMP,1,1);
  hd_rect(p,0,140,240,20,HD_INK);
  hd_rect(p,0,140,240,1,HD_COPPER);
  hd_text(p,8,145,"AT STREET LEVEL",HD_LAMP,1,1);
  hd_text(p,8,153,"A TALK   B CHOICE   L MAP   R JOURNAL",HD_PAPER,1,0);
 }
}
