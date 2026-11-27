#include "hd_canvas.h"
#include "hd_landscape.h"

/* On GBA the navigable Old Quarter is now a genuine 2.5D projection
   of its unchanged 24x18 story grid. HUD and quest guidance remain
   readable over the detailed isometric night scene. */
static void heading(uint8_t *p,const Story *s){
 int gx=(int)s->x,gy=(int)s->y;
 const char *goal="";
 int has=story_goal(s,&gx,&gy,&goal);
 int direction=-1,steps=-1;
 int reachable=story_route(s,&direction,&steps);
 char route[]="N 000";
 const char *heading=route;
 if(!reachable)heading="NO PATH";
 else if(!steps)heading="HERE";
 else{
  unsigned count=(unsigned)steps;
  if(count>999u)count=999u;
  route[0]=direction==0?'E':direction==1?'S':
           direction==2?'W':'N';
  if(count>=100u){
   route[2]=(char)('0'+count/100u);
   route[3]=(char)('0'+(count/10u)%10u);
   route[4]=(char)('0'+count%10u);
  }else if(count>=10u){
   route[2]=(char)('0'+count/10u);
   route[3]=(char)('0'+count%10u);
   route[4]=0;
  }else{
   route[2]=(char)('0'+count);
   route[3]=0;
  }
 }
 hd_rect(p,0,0,HD_W,18,HD_INK);
 hd_rect(p,0,17,HD_W,1,HD_COPPER);
 hd_text(p,8,5,story_district_name(s->x,s->y),HD_PAPER,1,1);
 hd_rect(p,84,3,1,11,HD_COPPER);
 /* Distinguish necessary story beats from optional legacy work. */
 hd_text(p,91,5,has?story_goal_kind(s):"FREE",HD_LAMP,1,0);
 if(has)hd_text(p,126,5,goal,HD_PAPER,1,0);
 hd_rect(p,189,3,1,11,HD_COPPER);
 hd_text(p,197,5,heading,HD_STONE_LIT,1,0);
 /* Physical control cues, not a debug-style description of the
    rendering engine. Low contrast secondary keys protect the scene. */
 hd_rect(p,0,140,HD_W,20,HD_INK);
 hd_rect(p,0,140,HD_W,1,HD_COPPER);
 hd_rect(p,8,143,3,7,HD_GOLD);
 hd_text(p,16,143,story_action_hint(s),HD_PAPER,1,0);
 hd_rect(p,92,143,1,14,HD_STONE_DARK);
 hd_text(p,100,143,story_brush_mode(s)?"L CAMERA SELECT EXIT":story_land_tools_enabled()&&story_rural(s->x,s->y)?"L CAMERA SELECT LAND":"L CAMERA  START HELP",HD_STONE_LIT,1,0);
 if(story_rural(s->x,s->y))hd_land_status(p,s);
 else if(story_risk_available(s))hd_text(p,16,152,"B RISK OPTIONS",HD_STONE_LIT,1,0);
 hd_text(p,158,152,story_neighborhood_revisit(s)>=0?"R REVISIT":"R NOTES",HD_STONE_LIT,1,0);
}
void hd_world_draw(uint8_t *p,const Story *s,unsigned frame){
 hd_isometric_draw(p,s,frame);
 heading(p,s);
}
