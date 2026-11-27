#include "hd_canvas.h"
#include "story_checks.h"
#include "hd_city_canvas.h"
#include "hd_guide_map.h"
#include "hd_landscape.h"

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
  hd_text(p,27,31,"FIELD GUIDE / CHOOSE A GOAL",HD_LAMP,1,0);
  hd_line(p,21,44,217,44,HD_STONE_DARK);
  unsigned selected=story_track_selected(s);
  for(unsigned i=0;i<STORY_TRACK_COUNT;i++){
   int y=49+(int)i*13,available=story_track_available(s,i);
   if(i==selected)hd_rect(p,20,y-2,200,15,HD_ROAD_COOL);
   hd_text(p,25,y,story_track_name(i),available?HD_PAPER:HD_STONE_DARK,1,0);
   if(story_track_id(i)&&s->neighborhood_target==story_track_id(i))hd_text(p,177,y,"TRACK",HD_LAMP,1,0);
   else if(i==STORY_TRACK_SHOW_INDEX&&story_show_stage(s)==STORY_SHOW_DONE)
    hd_text(p,177,y,"DONE",HD_LAMP,1,0);
   else if(!available)hd_text(p,177,y,"LATER",HD_STONE_LIT,1,0);
   if(i==STORY_TRACK_SHOW_INDEX){char progress[]="0/7";progress[0]=(char)('0'+story_show_stage(s));hd_text(p,145,y,progress,HD_GOLD,1,0);}
  }
  hd_text(p,22,119,selected==0u?"EAST PLAZA: EDIT AND SAVE TAKES":
       selected==1u?"PAINT THE CITY / FOUR LAYERS":
       selected==2u?"START AT THE EAST TERMINAL":
       selected==STORY_TRACK_SHOW_INDEX?"MUSIC + ART / 7 SHOW STEPS":
       "LETTER, BORE, MARA, THEN BUS",HD_PAPER,1,0);
  hd_text(p,22,131,"UP/DOWN CHOOSE  A TRACK",HD_GOLD,1,0);
 }else{
  hd_text(p,27,31,"FIELD GUIDE / OLD QUARTER",HD_LAMP,1,0);
  hd_line(p,21,44,217,44,HD_STONE_DARK);
  int gx=0,gy=0;const char *goal=0;
  int has=story_goal(s,&gx,&gy,&goal);
  hd_guide_map(p,s,144,50);
  hd_text(p,22,51,has?story_goal_kind(s):"FREE TIME",HD_STONE_LIT,1,0);
  if(has){
   hd_text(p,22,62,goal,HD_PAPER,1,0);
   int dir=-1,steps=-1;
   if(story_route(s,&dir,&steps)){
    static const char *const compass[]={"EAST","SOUTH","WEST","NORTH"};
    static const char *const relative[]={"AHEAD","RIGHT","BACK","LEFT"};
    const char *heading=dir<0?"HERE":s->view?relative[(dir-(int)s->facing+4)&3]:compass[dir];
    char count[]="000 STEPS";unsigned n=steps<0?0u:steps>999?999u:(unsigned)steps;
    count[0]=(char)('0'+n/100u);count[1]=(char)('0'+n/10u%10u);count[2]=(char)('0'+n%10u);
    hd_text(p,22,75,heading,HD_GOLD,1,0);
    hd_text(p,22,86,steps?count:"A TO INTERACT",HD_PAPER,1,0);
   }else hd_text(p,22,75,"NO SAFE ROUTE",HD_RED,1,0);
  }
  hd_text(p,22,98,story_action_hint(s),HD_LAMP,1,0);
  hd_text(p,22,111,has?story_goal_reason(s):"EXPLORE AND CREATE",HD_PAPER,1,0);
  hd_text(p,22,125,"WHITE YOU / GOLD GOAL",HD_STONE_LIT,1,0);
  hd_text(p,22,135,"GREEN NEXT STEP / R CHECKS",HD_STONE_LIT,1,0);
 }
 hd_rect(p,19,143,202,1,HD_STONE_DARK);
 hd_text(p,22,147,"LEFT/RIGHT PAGE  B BACK  R CHECKS",HD_STONE_LIT,1,0);
}
static void selectable(uint8_t *p,int x,int y,int w,const char *label,int selected){
 hd_rect(p,x,y,w,21,selected?HD_ROAD_COOL:HD_SHADOW);
 hd_rect(p,x,y,selected?3:1,21,selected?HD_LAMP:HD_STONE_DARK);
 hd_text(p,x+9,y+7,label,selected?HD_PAPER:HD_STONE_LIT,1,1);
}
static void neighborhood_journal(uint8_t *p,const Story *s){
 framebox(p,8,8,224,147);
 hd_text(p,18,16,"OLD QUARTER / SIDE STORIES",HD_LAMP,1,0);
 char total[]="00/14";unsigned count=story_neighborhood_count(s);
 total[0]=(char)('0'+count/10u);total[1]=(char)('0'+count%10u);
 hd_text(p,192,16,total,HD_PAPER,1,0);
 unsigned cursor=s->neighborhood_cursor%14u,first=(cursor/7u)*7u;
 for(unsigned i=0;i<7u;i++){
  unsigned id=first+i,seen=(s->neighborhood_seen>>id)&1u;
  int y=29+(int)i*14;
  if(id==cursor)hd_rect(p,14,y-2,212,13,HD_ROAD_COOL);
  hd_rect(p,17,y,4,7,seen?HD_GOLD:HD_STONE_DARK);
  hd_text(p,25,y,story_neighborhood_name(id),id==cursor?HD_PAPER:HD_STONE_LIT,1,0);
  hd_text(p,104,y,seen?story_neighborhood_option(id,(s->neighborhood_right>>id)&1u):
      "UNREAD",HD_STONE_LIT,1,0);
  if(s->neighborhood_target==id+1u)hd_text(p,184,y,"TRACK",HD_LAMP,1,0);
 }
 hd_text(p,18,130,story_neighborhood_enabled(s)?"A TRACK / UNTRACK AND WALK":
     "FINISH CORE / URGENT TASK FIRST",HD_PAPER,1,0);
 hd_text(p,18,142,"D-PAD SELECT   B BACK",HD_STONE_LIT,1,0);
}
void hd_game_overlay(uint8_t *p,const Story *s){
 if(s->activity==STORY_ACT_STUDIO||s->battle||
    s->life_stage==STORY_LIFE_FINALE||s->ended)return;
 if(s->dialogue){
  int count=0;const StoryPage *pages=story_pages(s,&count);
  if(pages && s->page<(unsigned)count){
   const StoryPage *d=&pages[s->page];
   const char *accept=story_dialogue_accept(s),*back=story_dialogue_back(s);
   unsigned number=(unsigned)s->page+1u,total=(unsigned)count;
   char progress[]="00/00";
   progress[0]=(char)('0'+number/10u);progress[1]=(char)('0'+number%10u);
   progress[3]=(char)('0'+total/10u);progress[4]=(char)('0'+total%10u);
   if(s->cinema==CINE_OPEN){
    /* Full 240x135 source framing stays above this film caption. */
    hd_rect(p,0,135,240,25,HD_INK);
    hd_rect(p,0,135,240,1,HD_GOLD);
    hd_text(p,6,139,d->speaker,HD_LAMP,1,0);
    hd_rect(p,37,139,1,16,HD_STONE_DARK);
    hd_text(p,44,139,d->line1,HD_PAPER,1,0);
    hd_text(p,44,149,d->line2,HD_PAPER,1,0);
    hd_text(p,234-hd_text_width(accept,1,0),139,accept,HD_LAMP,1,0);
    hd_text(p,234-hd_text_width(back,1,0),149,back,HD_STONE_LIT,1,0);
    hd_text(p,6,149,progress,HD_STONE_LIT,1,0);
    return;
   }
   /* Real cinematic frames deserve the space previously occupied by the
      54-row dialogue card. Keep authored neighborhood panels (whose art
      intentionally ends at row 104) and ordinary world dialogue unchanged. */
   if(s->cinema!=CINE_NONE&&story_neighborhood_panel(s)<0){
    hd_rect(p,0,122,HD_W,HD_H-122,HD_INK);
    hd_rect(p,0,122,HD_W,1,HD_COPPER);
    hd_text(p,8,125,d->speaker,HD_LAMP,1,0);
    hd_text(p,98,125,progress,HD_STONE_LIT,1,0);
    hd_text(p,136,125,accept,HD_LAMP,1,0);
    hd_text(p,234-hd_text_width(back,1,0),125,back,HD_STONE_LIT,1,0);
    const char *lines[2]={d->line1,d->line2};
    for(unsigned i=0;i<2u;i++){
     int track=hd_text_width(lines[i],1,1)>224?0:1;
     int width=hd_text_width(lines[i],1,track);
     int x=(HD_W-width)/2;
     hd_text(p,x<4?4:x,136+(int)i*10,lines[i],HD_PAPER,1,track);
    }
    return;
   }
   framebox(p,8,103,224,54);
   hd_rect(p,13,108,4,10,HD_GOLD);
   hd_rect(p,19,106,91,13,HD_BRICK_DARK);
   hd_text(p,23,110,d->speaker,s->cinema==CINE_MARA?HD_SKIN_LIT:HD_LAMP,1,0);
   hd_line(p,18,120,222,120,HD_STONE_DARK);
   hd_text(p,18,124,d->line1,HD_PAPER,1,1);
   hd_text(p,18,136,d->line2,HD_PAPER,1,1);
   hd_text(p,222-hd_text_width(accept,1,0),147,accept,HD_LAMP,1,0);
   hd_text(p,18,147,back,HD_STONE_LIT,1,0);
   hd_text(p,197,110,progress,HD_STONE_LIT,1,0);
  }
  return;
 }
 if(s->paused && (s->choice&STORY_NEIGHBORHOOD_JOURNAL)){
  neighborhood_journal(p,s);return;
 }
 if(s->paused && (s->choice&STORY_FIELD_GUIDE_ACTIVE)){
  field_guide(p,s);return;
 }
 if(s->paused && !(s->choice&STORY_CHECK_PAGE_ACTIVE)){
  framebox(p,16,67,208,87);
  hd_text(p,29,77,"A MOMENT TO BREATHE",HD_PAPER,1,0);
  hd_line(p,29,91,210,91,HD_STONE_DARK);
  hd_text(p,30,96,"START  RETURN TO STREET",HD_PAPER,1,0);
  hd_text(p,30,107,"A  GOALS / L FIELD GUIDE",HD_LAMP,1,0);
  hd_text(p,30,119,"R  JOURNAL / CONSEQUENCES",HD_STONE_LIT,1,0);
  hd_text(p,30,128,"SELECT  SIDE STORIES",HD_LAMP,1,0);
  hd_text(p,30,139,"B  SAVE AND LEAVE",HD_PAPER,1,0);
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
       "ANOTHER HARM CAN END THE STORY";
  else if(s->rescue_stage>=STORY_RESCUE_MISSING &&
          s->rescue_stage<=STORY_RESCUE_TRACE)risk_hint=
       "ANOTHER HARM CAN BLOCK RESCUE";
  else if(story_bad_count(s)>=2)risk_hint=
       "THIS CHOICE MAY ENDANGER MARA";
  else risk_hint="THIS CAN DAMAGE SOMEONE'S TRUST";
  hd_text(p,23,109,risk_hint,HD_STONE_LIT,1,0);
  selectable(p,23,121,91,s->risk_menu=='H'?"OPEN":"BACK",!s->risk_choice);
  selectable(p,122,121,92,story_risk_action(s),s->risk_choice);
  hd_text(p,23,146,"D-PAD CHOOSE   A OK   B CANCEL",HD_PAPER,1,0);
  return;
 }
 if(s->activity==STORY_ACT_PAINT){
  framebox(p,8,26,224,131);
  char count[]="LAYER 1/4";count[6]=(char)('1'+s->art_steps);
  hd_text(p,18,34,"ROOFTOP WORKSHOP",HD_LAMP,1,0);
  hd_text(p,169,34,count,HD_PAPER,1,0);
  hd_text(p,18,45,story_paint_layer(s),HD_PAPER,1,1);
  hd_text(p,168,45,story_paint_warm(s)?"WARM":"COOL",HD_GOLD,1,0);
  unsigned targets=story_paint_targets(s),brush=story_paint_brush(s);
  unsigned done=0;
  hd_rect(p,63,53,114,66,HD_BARK);
  hd_city_painting(p,68,54,104,64,s->art_steps,story_paint_warm(s),s->studio_tab);
  for(unsigned i=0;i<8u;i++){
   int x=68+(int)(i&3u)*26,y=54+(int)(i>>2)*32;
   unsigned bit=1u<<i,marked=targets&bit,filled=s->studio_tab&bit;
   if(marked&&filled)done++;
   /* Corner indicators keep the composition visible, including filled cells. */
   if(brush==i){
    hd_rect(p,x,y,7,1,HD_LAMP);hd_rect(p,x,y,1,7,HD_LAMP);
    hd_rect(p,x+19,y+31,7,1,HD_LAMP);hd_rect(p,x+25,y+25,1,7,HD_LAMP);
   }
   if(marked&&!filled){hd_rect(p,x+10,y+14,6,5,HD_INK);hd_rect(p,x+11,y+15,4,3,HD_GOLD);}
  }
  hd_text(p,18,59,"STROKES",HD_STONE_LIT,1,0);
  char marks[]="0/4";marks[0]=(char)('0'+done);
  hd_text(p,20,72,marks,HD_PAPER,1,1);
  for(unsigned j=0;j<4u;j++)hd_rect(p,18+(int)j*10,86,8,5,j<done?HD_GOLD:HD_STONE_DARK);
  hd_text(p,183,60,"TONE",HD_STONE_LIT,1,0);
  hd_rect(p,184,73,29,14,story_paint_warm(s)?HD_COPPER:HD_GLASS_LIT);
  hd_text(p,182,94,story_paint_warm(s)?"WARM":"COOL",HD_PAPER,1,0);
  hd_text(p,18,119,story_paint_ready(s)?"LAYER READY - START TO SAVE":
       s->studio_cursor==2u?"FILL ALL FOUR GOLD MARKS":
       s->studio_cursor==1u?"MOVE TO A GOLD MARK":"PAINT THE FOUR GOLD MARKS",HD_PAPER,1,0);
  hd_text(p,18,133,"D-PAD BRUSH  A PAINT  SELECT TONE",HD_STONE_LIT,1,0);
  hd_text(p,18,145,"START SAVE LAYER  B BACK",HD_PAPER,1,0);
  return;
 }
 if(s->activity==STORY_ACT_REHEARSAL){
  framebox(p,8,66,224,91);
  char part[]="PART 1/4";part[5]=(char)('1'+(s->studio_tab&3u));
  hd_text(p,18,74,"SET LIST REHEARSAL",HD_LAMP,1,0);
  hd_text(p,170,74,part,HD_PAPER,1,0);
  hd_text(p,18,86,s->studio_cursor?"TRY THE SHOWN PHRASE":"MATCH THE NEXT CUE",s->studio_cursor?HD_LAMP:HD_PAPER,1,0);
  hd_text(p,18,98,"YOUR ORDER:",HD_STONE_LIT,1,0);
  hd_text(p,100,98,story_show_phrase(story_show_expected(s)),HD_GOLD,1,0);
  for(unsigned i=0;i<4u;i++)selectable(p,16+(int)i*53,111,50,story_show_phrase(i),s->activity_choice==i);
  hd_text(p,18,145,"D-PAD CHOOSE  A CUE  B BACK",HD_PAPER,1,0);
  return;
 }
 if(s->activity==STORY_ACT_SHOW){
  framebox(p,12,104,216,53);
  hd_text(p,21,111,story_show_title(s),HD_LAMP,1,0);
  selectable(p,21,121,95,story_show_option(s,0),!s->activity_choice);
  selectable(p,124,121,95,story_show_option(s,1),s->activity_choice);
  hd_text(p,21,145,"D-PAD CHOOSE  A OK  B BACK",HD_PAPER,1,0);
  return;
 }
 if(s->activity==STORY_ACT_NEIGHBORHOOD){
  int id=story_neighborhood_at(s);
  framebox(p,8,103,224,54);
  hd_text(p,18,110,id>=0?story_neighborhood_name((unsigned)id):"SIDE STORY",HD_LAMP,1,0);
  if(id>=0){
   selectable(p,18,121,98,story_neighborhood_option((unsigned)id,0),!s->activity_choice);
   selectable(p,124,121,98,story_neighborhood_option((unsigned)id,1),s->activity_choice);
  }
  hd_text(p,18,146,"D-PAD CHOOSE   A OK   B BACK",HD_PAPER,1,0);
  return;
 }
 if(s->activity){
  const char *where="OLD QUARTER",*left="CONTINUE",*right="REST",*benefit="";
  const char *effect="";
  switch(s->activity){
  case 'f':where="MARA / FIRST EVENING";left="WALK";right="TALK";benefit="BEFORE THE LAST BUS";break;
  case 'F':where="MARA / AFTERNOON";left="WALK";right="TALK";benefit="TIME TOGETHER";break;
  case 'L':where="LETTER / MEMORY";left="WRITE";right="KEEP";benefit="A MEMORY OF HOME";break;
  case 'C':where="BORE / ARCHIVE";left="FILE";right="JOKE";benefit="ANOTHER SMALL FORM";break;
  case 'M':where="MUSIC DESK";left="MIX";right="LISTEN";benefit="MAKE A SONG OR EDIT IT";break;
  case 'E':where="MUSIC DESK";left="LISTEN";right="EDIT";benefit="ADVANCED STUDIO UNLOCKED";break;
  case 'J':where="POTHEADS / SHIFT";left="WORK";right="LEAVE";benefit="PAID HOURS, NOT HIS DREAM";break;
  case STORY_ACT_ROOF_REST:where="ROOFTOP / FINISHED ART";left="REST";right="LEAVE";benefit="WATCH THE SKY FOR AWHILE";break;
  case 'R':where="ROOFTOP / THE SKY";left=s->art_steps>=STORY_PROJECT_DONE?"VIEW":"PAINT";right="REST";benefit="A QUIET AFTERNOON";break;
  case 'm':where="MUSIC DESK";left="MIX";right="SOFT";benefit="MAKE A SONG OR EDIT IT";break;
  case 'r':where="ROOFTOP / ART";left="LAYER";right="LIGHT";benefit="MAKE THE CITY YOURS";break;
  default:where="ROOFTOP / THE SKY";left="PAINT";right="REST";benefit="A QUIET AFTERNOON";break;
  }
  switch(s->activity){
   case 'f':case 'F':effect=s->activity_choice?"BOND +1  ENERGY +1":"BOND +2  ENERGY -1";break;
   case 'L':effect=s->activity_choice?"SPARK +1 ENERGY +1":"SPARK +2 ENERGY -1";break;
   case 'C':effect=s->activity_choice?"BOND +1 ENERGY +1":"SPARK +1 ENERGY -1";break;
   case 'M':effect=s->activity_choice?"SPARK +1 ENERGY +1":"MUSIC STAGE +1";break;
   case 'E':effect=s->activity_choice?"ADVANCED SIX-TAB STUDIO":
       story_daily_pending(s,STORY_DAY_MUSIC)?"DAILY MEMORY / ENERGY +1":"LISTEN TO THE MASTER";break;
   case STORY_ACT_ROOF_REST:effect=s->activity_choice?"BACK TO THE STREET":
       s->energy<STORY_STAT_MAX?"ENERGY +1 / NO WORK COST":"ENERGY FULL / ENJOY THE VIEW";break;
   case 'R':effect=s->activity_choice?"ENERGY +2 SPARK +1":
       s->art_steps>=STORY_PROJECT_DONE?"VIEW YOUR FINISHED CANVAS":"PAINT STAGE +1";break;
   case 'J':effect=s->activity_choice?"NO ENERGY SPENT":"WORK +2 ENERGY -1";break;
   case 'm':case 'r':effect=s->activity_choice?
       "STAGE +1  ENERGY +1":"STAGE +1  MORE WORK";break;
   default:effect=s->activity_choice?"ENERGY +2 SPARK +1":"PAINT STAGE +1";break;
  }
  framebox(p,12,72,216,84);
  hd_text(p,22,79,where,HD_LAMP,1,1);
  /* A four-notch progress strip mirrors the actual persisted work.
     The music and painting menus now show the same visual language. */
  if(s->activity=='m'||s->activity=='M'||s->activity=='E'||
     s->activity=='r'||s->activity=='R'){
   unsigned art=s->activity=='r'||s->activity=='R';
   unsigned phase=art?s->art_steps:s->song_steps;
   if(phase>STORY_PROJECT_DONE)phase=STORY_PROJECT_DONE;
   char count[]="0/4";count[0]=(char)('0'+phase);
   hd_text(p,157,79,count,HD_PAPER,1,0);
   for(unsigned i=0;i<4u;i++){
    int x=179+(int)i*9;
    hd_rect(p,x,79,7,7,HD_STONE_DARK);
    hd_rect(p,x+1,80,5,5,i<phase?HD_GOLD:HD_INK);
    if(i==phase)hd_rect(p,x,87,7,1,HD_LAMP);
   }
  }
  hd_text(p,22,91,benefit,HD_PAPER,1,0);
  int music=s->activity=='m'||s->activity=='M',painting=story_paint_workshop_available(s);
  if(music&&s->activity_choice==2u)effect="BASIC: EDIT, PLAY, SAVE TAKE";
  if(painting&&s->activity_choice==2u)effect="BRUSH, TONE AND SAVE A LAYER";
  hd_text(p,22,103,effect,HD_STONE_LIT,1,0);
  if(music||painting){
   selectable(p,20,115,66,left,s->activity_choice==0u);
   selectable(p,87,115,66,right,s->activity_choice==1u);
   selectable(p,154,115,66,painting?"BRUSH":"STUDIO",s->activity_choice==2u);
  }else{
   selectable(p,22,115,92,left,!s->activity_choice);
   selectable(p,123,115,93,right,s->activity_choice);
  }
  hd_text(p,25,145,"LEFT/RIGHT CHOOSE  A OK  B BACK",HD_STONE_LIT,1,0);
  return;
 }
 if(s->choosing){
  framebox(p,13,79,214,77);
  hd_text(p,23,90,"THE LAST BUS",HD_LAMP,1,1);
  hd_text(p,23,103,(s->flags&STORY_FULL)==STORY_FULL?
     "READY TO LEAVE WITH YOUR WORK":
     s->life_stage?"YOU ARE STILL IN THE OLD QUARTER":
     "LEAVING WILL START A NEW DAY",HD_PAPER,1,0);
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
  hd_text(p,8,5,story_district_name(s->x,s->y),HD_PAPER,1,1);
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
  hd_text(p,100,143,"L CAMERA  START HELP",HD_STONE_LIT,1,0);
  if(story_rural(s->x,s->y))hd_land_status(p,s);
  else if(story_risk_available(s))hd_text(p,16,152,"B RISK OPTIONS",HD_STONE_LIT,1,0);
  hd_text(p,158,152,story_neighborhood_revisit(s)>=0?"R REVISIT":"R NOTES",HD_STONE_LIT,1,0);
 }
}

