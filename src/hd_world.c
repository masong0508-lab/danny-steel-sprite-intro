#include "hd_canvas.h"

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
 hd_text(p,8,5,"OLD QUARTER",HD_PAPER,1,1);
 hd_text(p,107,5,has?"NEXT":"FREE TIME",HD_LAMP,1,1);
 if(has)hd_text(p,140,5,goal,HD_PAPER,1,0);
 hd_text(p,199,5,heading,HD_STONE_LIT,1,0);
 /* Physical control cues, not a debug-style description of the
    rendering engine. Low contrast secondary keys protect the scene. */
 hd_rect(p,0,140,HD_W,20,HD_INK);
 hd_rect(p,0,140,HD_W,1,HD_COPPER);
 hd_rect(p,8,145,3,10,HD_GOLD);
 hd_text(p,17,147,story_action_hint(s),HD_PAPER,1,1);
 hd_text(p,88,147,"L CAMERA",HD_STONE_LIT,1,1);
 hd_text(p,167,147,"R JOURNAL",HD_STONE_LIT,1,0);
}
void hd_world_draw(uint8_t *p,const Story *s,unsigned frame){
 hd_isometric_draw(p,s,frame);
 heading(p,s);
}
