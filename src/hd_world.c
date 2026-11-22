#include "hd_canvas.h"

/* On GBA the navigable Old Quarter is now a genuine 2.5D projection
   of its unchanged 24x18 story grid. HUD and quest guidance remain
   readable over the detailed isometric night scene. */
static void heading(uint8_t *p,const Story *s){
 int gx=(int)s->x,gy=(int)s->y;
 const char *goal="";
 int has=story_goal(s,&gx,&gy,&goal);
 unsigned toward=4,steps=0;
 int route=has&&story_route(s,&toward,&steps);
 char compact[]="W00";
 const char points[]="ESWNH";
 compact[0]=route?points[toward]:'?';
 if(steps>99u)steps=99u;
 compact[1]=(char)('0'+steps/10u);
 compact[2]=(char)('0'+steps%10u);
 hd_rect(p,0,0,HD_W,18,HD_INK);
 hd_rect(p,0,17,HD_W,1,HD_COPPER);
 hd_text(p,8,5,"OLD QUARTER",HD_PAPER,1,1);
 hd_text(p,107,5,has?"NEXT":"FREE TIME",HD_LAMP,1,1);
 if(has)hd_text(p,140,5,goal,HD_PAPER,1,0);
 /* A real first step, not a bearing through solid buildings.
    Distance is measured in WALKABLE steps, not map pixels. */
 hd_text(p,202,5,route?compact:"---",HD_GOLD,1,0);
 /* Physical control cues, not a debug-style description of the
    rendering engine. Low contrast secondary keys protect the scene. */
 hd_rect(p,0,140,HD_W,20,HD_INK);
 hd_rect(p,0,140,HD_W,1,HD_COPPER);
 hd_rect(p,8,145,3,10,HD_GOLD);
 hd_text(p,17,144,story_action_hint(s),HD_PAPER,1,1);
 hd_text(p,93,144,story_risk_available(s)?"B RISK":"L CAMERA",
         HD_STONE_LIT,1,1);
 hd_text(p,167,144,"R JOURNAL",HD_STONE_LIT,1,0);
 hd_text(p,17,153,"START PAUSE / L GUIDE",HD_STONE_LIT,1,0);
}
void hd_world_draw(uint8_t *p,const Story *s,unsigned frame){
 hd_isometric_draw(p,s,frame);
 heading(p,s);
}
