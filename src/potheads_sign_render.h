#pragma once
#include "potheads_sign_geometry.h"
/* Native sign artwork. Each texel is attached to the measured face;
   whitespace leaves the underlying paper alone. Playback uses baked frames. */
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
/* All samples come from the same projective face, without a diagonal
   seam. The released ROM plays lossless precomputed frames; this
   source painter adds no playback scratch. */
static unsigned pot_sign_print(uint8_t *p,const PotPoint q[4],unsigned detail){
 PotSignMap map;
 if(!pot_sign_map_init(q,&map))return 0;
 int left=q[0].x,right=left,top=q[0].y,bottom=top,step[4];
 for(unsigned i=0;i<4u;i++){
  if(q[i].x<left)left=q[i].x;
  if(q[i].x>right)right=q[i].x;
  if(q[i].y<top)top=q[i].y;
  if(q[i].y>bottom)bottom=q[i].y;
  step[i]=q[i].y-q[(i+1u)&3u].y;
 }
 if(left<0)left=0;
 if(right>=HD_W)right=HD_W-1;
 if(top<0)top=0;
 if(bottom>=135)bottom=134;
 if(left>right||top>bottom)return detail;
 for(int y=top;y<=bottom;y++){
  int e[4];PotPoint start={left,y};
  for(unsigned i=0;i<4u;i++)e[i]=pot_sign_cross(q[i],q[(i+1u)&3u],start);
  uint8_t *out=p+y*HD_W+left;
  for(int x=left;x<=right;x++,out++){
   unsigned u,v;
   if(pot_sign_map_uv(&map,e,&u,&v)){
    char ink=pot_sign_art[v][u];
    if(ink=='X')*out=detail==2u?HD_INK:HD_STONE_DARK;
    else if(ink=='g'&&detail==2u)*out=HD_LEAF_DARK;
    else if(ink=='+'||ink=='.')*out=ink=='+'?HD_PLASTER_LIT:HD_STONE_LIT;
   }
   for(unsigned i=0;i<4u;i++)e[i]+=step[i];
  }
 }
 return detail;
}
static unsigned pot_sign_projected(const PotCamera *c,const PotPose *pose,PotPoint q[5],PotPoint hull[5]){
 for(unsigned i=0;i<5u;i++)q[i]=pot_actor_point(c,pose->sign[i]);
 /* Private mattes remain aligned to the original measured envelope. */
#if !POTHEADS_CLOTH_ENABLED && !POTHEADS_LEG_ENABLED && !POTHEADS_UPPER_ENABLED
 (void)pot_sign_stabilize(q,pose->flags);
#endif
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
 if(detail)detail=pot_sign_print(p,face,detail);
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
