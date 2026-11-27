#pragma once
#include "potheads_sign_geometry.h"
/* Native sign artwork. Each texel is attached to the measured face;
   whitespace leaves the underlying paper alone. No per-pixel division. */
static const char pot_sign_art[26][65]={
 "                                                                ",
 "  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  ",
 "                                                                ",
 "                                                                ",
 "     XXXXX XXXX  XXXXX XXXXX       X   X XXXXX XXXXX XXXX       ",
 "     X     X   X X     X           X   X X     X     X   X      ",
 "     X     X   X X     X           X   X X     X     X   X      ",
 "     XXXX  XXXX  XXXX  XXXX        X X X XXXX  XXXX  X   X      ",
 "     X     X X   X     X           X X X X     X     X   X      ",
 "     X     X  X  X     X           XX XX X     X     X   X      ",
 "     X     X   X XXXXX XXXXX       X   X XXXXX XXXXX XXXX       ",
 "                                                                ",
 "                                                                ",
 "                                                                ",
 "                                                                ",
 "            XXX    X          XX         X   X XXXX             ",
 "           X   X  XX         X  X        X   X X   X            ",
 "               X   X         X X         X   X X   X            ",
 "              X    X          X          X   X XXXX             ",
 "     g       X     X         X X X       X   X X                ",
 "    ggg     X      X         X  X        X   X X                ",
 "   g g g   XXXXX  XXX         XX X        XXX  X                ",
 "  g  g  g                                                       ",
 "     g                                                          ",
 "  ............................................................  ",
 "                                                                ",
};
static void pot_sign_triangle(uint8_t *p,PotPoint a,PotPoint b,PotPoint c,
                              int ua,int va,int ub,int vb,int uc,int vc,unsigned detail){
 int area=pot_sign_cross(a,b,c);
 if(area<=0)return;
 int left=a.x,right=a.x,top=a.y,bottom=a.y;
 if(b.x<left)left=b.x;
 if(c.x<left)left=c.x;
 if(b.x>right)right=b.x;
 if(c.x>right)right=c.x;
 if(b.y<top)top=b.y;
 if(c.y<top)top=c.y;
 if(b.y>bottom)bottom=b.y;
 if(c.y>bottom)bottom=c.y;
 if(left<0)left=0;
 if(right>=HD_W)right=HD_W-1;
 if(top<0)top=0;
 if(bottom>=135)bottom=134;
 if(left>right||top>bottom)return;
 int ux=((ub-ua)*(c.y-a.y)-(uc-ua)*(b.y-a.y))*4096/area;
 int uy=((uc-ua)*(b.x-a.x)-(ub-ua)*(c.x-a.x))*4096/area;
 int vx=((vb-va)*(c.y-a.y)-(vc-va)*(b.y-a.y))*4096/area;
 int vy=((vc-va)*(b.x-a.x)-(vb-va)*(c.x-a.x))*4096/area;
 for(int y=top;y<=bottom;y++){
  PotPoint start={left,y};
  int ea=pot_sign_cross(b,c,start),eb=pot_sign_cross(c,a,start),ec=pot_sign_cross(a,b,start);
  int u=ua*4096+ux*(left-a.x)+uy*(y-a.y);
  int v=va*4096+vx*(left-a.x)+vy*(y-a.y);
  for(int x=left;x<=right;x++){
   if(ea>=0&&eb>=0&&ec>=0){
    int ix=(u+2048)>>12,iy=(v+2048)>>12;
    if(ix>=0&&ix<64&&iy>=0&&iy<26){
     char ink=pot_sign_art[iy][ix];
     if(ink=='X')hd_pixel(p,x,y,detail==2u?HD_INK:HD_STONE_DARK);
     else if(ink=='g'&&detail==2u)hd_pixel(p,x,y,HD_LEAF_DARK);
     else if(ink=='+'||ink=='.')hd_pixel(p,x,y,ink=='+'?HD_PLASTER_LIT:HD_STONE_LIT);
    }
   }
   u+=ux;v+=vx;ea-=c.y-b.y;eb-=a.y-c.y;ec-=b.y-a.y;
  }
 }
}
static unsigned pot_sign_projected(const PotCamera *c,const PotPose *pose,PotPoint q[5],PotPoint hull[5]){
 for(unsigned i=0;i<5u;i++)q[i]=pot_actor_point(c,pose->sign[i]);
 return pot_sign_hull(q,(pose->flags&POT_SIGN_TIP_VISIBLE)?5u:4u,hull);
}
/* A measured crop intersection is not a physical cardboard edge.
   Source (0,0)..(640,360) quantizes to these same limits. */
static int pot_sign_crop_edge(const PotPose *pose,const PotPoint q[5],PotPoint a,PotPoint b){
 if(!(pose->flags&POT_SIGN_CLIPPED))return 0;
 unsigned count=(pose->flags&POT_SIGN_TIP_VISIBLE)?5u:4u;
 for(unsigned i=0;i<count;i++)if(q[i].x==a.x&&q[i].y==a.y)
  for(unsigned j=0;j<count;j++)if(q[j].x==b.x&&q[j].y==b.y){
   PotJoint x=pose->sign[i],y=pose->sign[j];
   if((x.x<=-100&&y.x<=-100)||(x.x>=113&&y.x>=113)||
      (x.y<=-117&&y.y<=-117)||(x.y>=3&&y.y>=3))return 1;
  }
 return 0;
}
static void pot_sign_draw(uint8_t *p,const PotCamera *c,const PotPose *pose){
 PotPoint q[5],h[5],face[4];
 unsigned n=pot_sign_projected(c,pose,q,h);
 if(!n)return;
 if(n==1u){hd_pixel(p,h[0].x,h[0].y,HD_PAPER);return;}
 if(n==2u){hd_line(p,h[0].x,h[0].y,h[1].x,h[1].y,HD_PAPER);return;}
 pot_shape(p,h,n,HD_PAPER);
 unsigned detail=(unsigned)pot_sign_face(q,pose->flags,face);
 if(detail){
  pot_sign_triangle(p,face[0],face[1],face[2],0,0,63,0,63,25,detail);
  pot_sign_triangle(p,face[0],face[2],face[3],0,0,63,25,0,25,detail);
 }
 /* Give the visible board a thin paper/card edge even when print is
    too foreshortened or cropped to reconstruct. Inset toward the
    measured envelope, never outside it or across a source crop edge. */
 PotPoint centre={0,0};
 for(unsigned i=0;i<n;i++){centre.x+=h[i].x;centre.y+=h[i].y;}
 centre.x/=(int)n;centre.y/=(int)n;
 for(unsigned i=0;i<n;i++){
  PotPoint a=h[i],b=h[(i+1u)%n];
  if(pot_sign_crop_edge(pose,q,a,b))continue;
  if(!detail&&pot_sign_dist(a,b)>=10){
   PotPoint aa={a.x+(centre.x>a.x)-(centre.x<a.x),a.y+(centre.y>a.y)-(centre.y<a.y)};
   PotPoint bb={b.x+(centre.x>b.x)-(centre.x<b.x),b.y+(centre.y>b.y)-(centre.y<b.y)};
   if(pot_sign_near(h,n,aa,0)&&pot_sign_near(h,n,bb,0))
    hd_line(p,aa.x,aa.y,bb.x,bb.y,b.y>a.y?HD_STONE_LIT:HD_PLASTER_LIT);
  }
  hd_line(p,a.x,a.y,b.x,b.y,HD_STONE_DARK);
 }
}
