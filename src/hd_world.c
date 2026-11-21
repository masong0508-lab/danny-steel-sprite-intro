#include "hd_canvas.h"

/* On GBA the navigable Old Quarter is now a genuine 2.5D projection
   of its unchanged 24x18 story grid. HUD and quest guidance remain
   readable over the detailed isometric night scene. */
static void heading(uint8_t *p,const Story *s){
 int gx=(int)s->x,gy=(int)s->y;
 const char *goal="";
 int has=story_goal(s,&gx,&gy,&goal);
 int dx=gx-(int)s->x,dy=gy-(int)s->y;
 const char *direction=dx==0&&dy==0?"HERE":
   (dx<0?-dx:dx)>(dy<0?-dy:dy)?(dx>0?"EAST":"WEST"):
   (dy>0?"SOUTH":"NORTH");
 hd_rect(p,0,0,HD_W,18,HD_INK);
 hd_rect(p,0,17,HD_W,1,HD_COPPER);
 hd_text(p,8,5,"OLD QUARTER",HD_PAPER,1,1);
 hd_text(p,107,5,has?"NEXT":"FREE TIME",HD_LAMP,1,1);
 if(has)hd_text(p,140,5,goal,HD_PAPER,1,0);
 hd_text(p,200,5,direction,HD_STONE_LIT,1,0);
 hd_rect(p,0,140,HD_W,20,HD_INK);
 hd_rect(p,0,140,HD_W,1,HD_COPPER);
 hd_text(p,8,145,"ISOMETRIC / OLD QUARTER",HD_LAMP,1,1);
 hd_text(p,8,153,"D-PAD GRID  A TALK  L VIEW  R JOURNAL",HD_PAPER,1,0);
}
void hd_world_draw(uint8_t *p,const Story *s,unsigned frame){
 hd_isometric_draw(p,s,frame);
 heading(p,s);
}
