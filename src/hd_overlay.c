#include "hd_canvas.h"
#include "story_checks.h"

/* Native-pixel film subtitles, menus and prompts shared by the painted map
   and legacy cinematic/raycast frames. Narrative text is never removed. */
static void framebox(uint8_t *p,int x,int y,int w,int h){
 hd_rect(p,x+2,y+2,w,h,HD_SHADOW);
 hd_rect(p,x,y,w,h,HD_INK);
 hd_rect(p,x,y,w,1,HD_COPPER);
 hd_rect(p,x,y,2,h,HD_COPPER);
 hd_rect(p,x+4,y+h-2,w-8,1,HD_STONE_DARK);
 hd_line(p,x+3,y+3,x+10,y+3,HD_GOLD);
 hd_line(p,x+3,y+3,x+3,y+10,HD_GOLD);
 hd_line(p,x+w-11,y+3,x+w-4,y+3,HD_GOLD);
 hd_line(p,x+w-4,y+3,x+w-4,y+10,HD_GOLD);
 hd_rect(p,x+5,y+4,w-10,1,HD_BRICK_DARK);
}
/* This is deliberately drawn only while paused, over the existing
   native scene. It is pure story/route information, not a new quest state
   or a hidden framebuffer. All labels fit the actual 240x160 canvas. */
static void field_guide(uint8_t *p,const Story *s){
 framebox(p,12,20,216,135);
 hd_rect(p,19,30,3,11,HD_GOLD);
 if(s->choice&STORY_FIELD_GUIDE_DETAIL){
  hd_text(p,27,31,"FIELD GUIDE / HOW TO PLAY",HD_LAMP,1,0);
  hd_line(p,21,44,217,44,HD_STONE_DARK);
  hd_text(p,22,52,"CORE: LETTER > BORE > MARA",HD_PAPER,1,0);
  hd_text(p,22,63,"AFTER CORE THE BUS IS OPEN",HD_PAPER,1,0);
  hd_text(p,22,74,"EXTRAS: TAPE + ROOF CHANGE END",HD_STONE_LIT,1,0);
  hd_line(p,21,85,217,85,HD_STONE_DARK);
  hd_text(p,22,91,"TRY TO LEAVE: CLINIC STOPS TRIP",HD_PAPER,1,0);
  hd_text(p,22,102,"A INTERACT / L CAMERA / R NOTES",HD_PAPER,1,0);
  hd_text(p,22,113,"B RISK IS OPTIONAL + CONFIRMED",HD_STONE_LIT,1,0);
  hd_text(p,22,124,"REST = ENERGY / BOND = TRUST",HD_PAPER,1,0);
  hd_text(p,22,135,"SPARK = ART / NO DEADLINE",HD_STONE_LIT,1,0);
 }else{
  hd_text(p,27,31,"FIELD GUIDE / OLD QUARTER",HD_LAMP,1,0);
  hd_line(p,21,44,217,44,HD_STONE_DARK);
  int gx=0,gy=0;const char *goal=0;
  int has=story_goal(s,&gx,&gy,&goal);
  hd_text(p,22,51,has?story_goal_kind(s):"NO URGENT ERRAND",
          HD_STONE_LIT,1,0);
  if(has){
   hd_text(p,22,63,goal,HD_PAPER,1,0);
   int dir=-1,steps=-1;
   if(story_route(s,&dir,&steps)){
    static const char *const compass[]={"AHEAD","RIGHT","BACK","LEFT"};
    const char *heading=dir<0?"HERE":compass[(dir-(int)s->facing+4)&3];
    char count[]="00 STEPS";
    unsigned number=steps<0?0u:steps>99?99u:(unsigned)steps;
    count[0]=(char)('0'+number/10u);
    count[1]=(char)('0'+number%10u);
    hd_text(p,139,63,heading,HD_GOLD,1,0);
    hd_text(p,178,63,steps==0?"ACT NOW":count,HD_LAMP,1,0);
   }else hd_text(p,121,63,"NO SAFE ROUTE",HD_RED,1,0);
   hd_text(p,22,75,story_goal_reason(s),HD_PAPER,1,0);
  }else hd_text(p,22,75,"EXPLORE AND MAKE SOMETHING",HD_PAPER,1,0);
  hd_line(p,21,86,217,86,HD_STONE_DARK);
  hd_text(p,22,93,story_action_hint(s),HD_LAMP,1,0);
  hd_text(p,108,93,story_risk_available(s)?"B RISK / OPTIONAL":
          "B NO RISK HERE",HD_STONE_LIT,1,0);
  hd_text(p,22,105,"A TALK / WORK / INTERACT",HD_PAPER,1,0);
  hd_text(p,22,117,"L CAMERA / R NOTES WHILE PLAYING",HD_PAPER,1,0);
  char stats[]="ENERGY 0  SPARK 0  BOND 0";
  stats[7]=(char)('0'+(s->energy>5u?5u:s->energy));
  stats[16]=(char)('0'+(s->spark>5u?5u:s->spark));
  stats[24]=(char)('0'+(s->bond>5u?5u:s->bond));
  hd_text(p,22,132,stats,HD_GOLD,1,0);
 }
 hd_rect(p,19,143,202,1,HD_STONE_DARK);
 hd_text(p,22,147,"LEFT/RIGHT PAGE  B BACK  R CHECKS",HD_STONE_LIT,1,0);
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
   if(s->cinema==CINE_OPEN){
    /* Full 240x135 source framing stays above this film caption. */
    hd_rect(p,0,135,240,25,HD_INK);
    hd_rect(p,0,135,240,1,HD_GOLD);
    hd_text(p,6,139,d->speaker,HD_LAMP,1,0);
    hd_rect(p,37,139,1,16,HD_STONE_DARK);
    hd_text(p,44,139,d->line1,HD_PAPER,1,0);
    hd_text(p,44,149,d->line2,HD_PAPER,1,0);
    hd_text(p,201,139,"A NEXT",HD_STONE_LIT,1,0);
    hd_text(p,201,149,"B SKIP",HD_STONE_LIT,1,0);
    for(int i=0;i<count&&i<9;i++)
     hd_rect(p,7+i*4,150,2,3,i<=s->page?HD_LAMP:HD_STONE_DARK);
    return;
   }
   framebox(p,8,103,224,54);
   hd_rect(p,13,108,4,10,HD_GOLD);
   hd_rect(p,19,106,91,13,HD_BRICK_DARK);
   hd_text(p,23,110,d->speaker,s->cinema==CINE_MARA?HD_SKIN_LIT:HD_LAMP,1,0);
   hd_line(p,18,120,222,120,HD_STONE_DARK);
   hd_text(p,18,124,d->line1,HD_PAPER,1,1);
   hd_text(p,18,136,d->line2,HD_PAPER,1,1);
   hd_text(p,174,147,"A NEXT",HD_STONE_LIT,1,0);
   for(int i=0;i<count&&i<9;i++)
    hd_rect(p,205+i*3,112,2,3,i<=s->page?HD_LAMP:HD_STONE_DARK);
  }
  return;
 }
 if(s->paused && (s->choice&STORY_FIELD_GUIDE_ACTIVE)){
  field_guide(p,s);return;
 }
 if(s->paused && !(s->choice&STORY_CHECK_PAGE_ACTIVE)){
  framebox(p,16,67,208,87);
  hd_text(p,29,77,"A MOMENT TO BREATHE",HD_PAPER,1,0);
  hd_line(p,29,91,210,91,HD_STONE_DARK);
  hd_text(p,30,96,"START  RETURN TO STREET",HD_PAPER,1,0);
  hd_text(p,30,107,"L  FIELD GUIDE / WHERE NEXT",HD_LAMP,1,0);
  hd_text(p,30,119,"R  JOURNAL / CONSEQUENCES",HD_STONE_LIT,1,0);
  hd_text(p,30,137,"B  SAVE AND LEAVE",HD_PAPER,1,0);
  return;
 }
 if(s->risk_menu){
  framebox(p,13,76,214,80);
  hd_text(p,23,85,"A CHOICE WITH CONSEQUENCES",HD_LAMP,1,0);
  hd_text(p,23,98,s->risk_menu=='L'?"THE LETTER":s->risk_menu=='C'?
   "THE ARCHIVE":s->risk_menu=='M'?"THE MUSIC":s->risk_menu=='H'?
   "MARA AT THE DOOR":"THE TERMINAL",HD_PAPER,1,1);
  const char *risk_hint;
  if(s->risk_menu=='H')risk_hint=s->risk_choice?
       "LEAVE THE DOOR CLOSED":"TRY TO OPEN THE DOOR";
  else if(s->rescue_stage==STORY_RESCUE_SAVED)risk_hint=
       "A FOURTH CHECK CAN END THE STORY";
  else if(s->rescue_stage>=STORY_RESCUE_MISSING &&
          s->rescue_stage<=STORY_RESCUE_TRACE)risk_hint=
       "ANOTHER CHECK CAN BLOCK RESCUE";
  else if(story_bad_count(s)>=2)risk_hint=
       "THE NEXT CHECK MAY ENDANGER MARA";
  else risk_hint="YOUR CHOICES HAVE CONSEQUENCES";
  hd_text(p,23,109,risk_hint,HD_STONE_LIT,1,0);
  selectable(p,23,121,91,s->risk_menu=='H'?"OPEN":"BACK",!s->risk_choice);
  selectable(p,122,121,92,s->risk_menu=='H'?"LEAVE":"DO IT",s->risk_choice);
  hd_text(p,28,146,"LEFT/RIGHT CHOOSE    B CANCEL",HD_PAPER,1,0);
  return;
 }
 if(s->activity){
  const char *where="OLD QUARTER",*left="CONTINUE",*right="REST",*benefit="";
  const char *effect="";
  switch(s->activity){
  case 'F':where="MARA / AFTERNOON";left="WALK";right="TALK";benefit="TIME TOGETHER";break;
  case 'L':where="LETTER / MEMORY";left="WRITE";right="KEEP";benefit="A MEMORY OF HOME";break;
  case 'C':where="BORE / ARCHIVE";left="FILE";right="JOKE";benefit="ANOTHER SMALL FORM";break;
  case 'M':where="BORE / MUSIC";left="ARRANGE";right="LISTEN";benefit="ONE MORE SONG";break;
  case 'E':where="MUSIC / MASTER";left="LISTEN";right="STUDIO";benefit="YOUR FINISHED MUSIC";break;
  case 'J':where="POTHEADS / SHIFT";left="WORK";right="LEAVE";benefit="PAID HOURS, NOT HIS DREAM";break;
  case 'm':where="MUSIC / TAKE";left="BUILD";right="SOFTEN";benefit="THE WORK CONTINUES";break;
  case 'r':where="ROOFTOP / ART";left="LAYER";right="LIGHT";benefit="MAKE THE CITY YOURS";break;
  default:where="ROOFTOP / THE SKY";left="PAINT";right="REST";benefit="A QUIET AFTERNOON";break;
  }
  switch(s->activity){
   case 'F':effect=s->activity_choice?"BOND +1  ENERGY +1":"BOND +2  ENERGY -1";break;
   case 'L':effect=s->activity_choice?"SPARK +1 ENERGY +1":"SPARK +2 ENERGY -1";break;
   case 'C':effect=s->activity_choice?"BOND +1 ENERGY +1":"SPARK +1 ENERGY -1";break;
   case 'M':effect=s->activity_choice?"SPARK +1 ENERGY +1":"MUSIC STAGE +1";break;
   case 'E':effect=s->activity_choice?"ADVANCED SIX-TAB STUDIO":"LISTEN TO THE MASTER";break;
   case 'J':effect=s->activity_choice?"NO ENERGY SPENT":"WORK +2 ENERGY -1";break;
   case 'm':case 'r':effect=s->activity_choice?
       "STAGE +1  ENERGY +1":"STAGE +1  MORE WORK";break;
   default:effect=s->activity_choice?"ENERGY +2 SPARK +1":"PAINT STAGE +1";break;
  }
  framebox(p,12,72,216,84);
  hd_text(p,22,79,where,HD_LAMP,1,1);
  hd_text(p,22,91,benefit,HD_PAPER,1,0);
  hd_text(p,22,103,effect,HD_STONE_LIT,1,0);
  selectable(p,22,115,92,left,!s->activity_choice);
  selectable(p,123,115,93,right,s->activity_choice);
  hd_text(p,25,145,"LEFT/RIGHT CHOOSE  A OK  B BACK",HD_STONE_LIT,1,0);
  return;
 }
 if(s->choosing){
  framebox(p,13,79,214,77);
  hd_text(p,23,90,"THE LAST BUS",HD_LAMP,1,1);
  hd_text(p,23,103,(s->flags&STORY_FULL)==STORY_FULL?
     "FINISHED WORK: BOARD THE BUS":
     s->life_stage?"YOU ARE ALREADY STAYING":
     "CLINIC CALL WILL STOP THE TRIP",HD_PAPER,1,0);
  selectable(p,23,118,92,(s->flags&STORY_FULL)==STORY_FULL?
     "BOARD":"TRY TO GO",!s->choice);
  selectable(p,123,118,91,"STAY",s->choice);
  hd_text(p,23,145,"LEFT/RIGHT   A CHOOSE   B BACK",HD_STONE_LIT,1,0);
  return;
 }
 /* Refresh only the UI portions of low-res first-person views. No rows
    are overwritten in the detailed overhead scene except the common HUD. */
 if(s->view){
  int gx=0,gy=0;const char *goal="";
  int has=story_goal(s,&gx,&gy,&goal);
  int dir=-1,steps=-1;
  int route=story_route(s,&dir,&steps);
  const char *relative=!route?"LOST":steps==0?"HERE":
      (dir-(int)s->facing+4)%4==0?"AHEAD":
      (dir-(int)s->facing+4)%4==1?"RIGHT":
      (dir-(int)s->facing+4)%4==2?"BACK":"LEFT";
  hd_rect(p,0,0,240,18,HD_INK);
  hd_rect(p,0,17,240,1,HD_COPPER);
  hd_text(p,8,5,"OLD QUARTER",HD_PAPER,1,1);
  hd_text(p,85,5,has?story_goal_kind(s):"",HD_STONE_LIT,1,0);
   hd_text(p,121,5,has?goal:"LOOK AROUND",HD_LAMP,1,0);
  hd_text(p,186,5,has?relative:"",HD_STONE_LIT,1,0);
  hd_rect(p,0,140,240,20,HD_INK);
  hd_rect(p,0,140,240,1,HD_COPPER);
  /* A two-row native HUD, each label inside its own unclipped lane.
     Risk is contextual, not a permanent alarm while exploring. */
  hd_rect(p,8,143,3,7,HD_GOLD);
  hd_text(p,16,143,story_action_hint(s),HD_PAPER,1,0);
  hd_rect(p,92,143,1,14,HD_STONE_DARK);
  hd_text(p,100,143,"L MAP  R NOTES",HD_STONE_LIT,1,0);
  int risk=story_risk_available(s);
  hd_text(p,16,152,risk?
          "B RISK  CONFIRM FIRST":"B NO RISK AT THIS LOCATION",
          risk?HD_LAMP:HD_STONE_LIT,1,0);
 }
}

