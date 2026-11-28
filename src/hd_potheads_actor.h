#pragma once
#include "potheads_motion.h"
#include "potheads_continuity.h"
#include "potheads_drape.h"
#include "potheads_cloth_render.h"
#include "potheads_leg_render.h"
#include "potheads_upper_render.h"

/* Source-timed articulated cutout. The anatomy and motion use measured
   landmarks; material planes and pixel lighting are authored native art.
   The final projection maps the complete source camera to 240x135. */
static unsigned pot_sqrt(unsigned value){
 unsigned root=0,bit=value<256u?1u<<6:value<65536u?1u<<14:
                       value<16777216u?1u<<22:1u<<30;
 while(bit>value)bit>>=2;
 while(bit){
  if(value>=root+bit){value-=root+bit;root=(root>>1)+bit;}
  else root>>=1;
  bit>>=2;
 }
 return root;
}
static int pot_actor_size(const PotCamera *c,int value){
 int out=value*c->zoom*9/(256*8);return out<1?1:out;
}
static PotPoint pot_actor_point(const PotCamera *c,PotJoint point){
 return pot_actor_project(c,point.x,point.y);
}
static PotJoint pot_mid(PotJoint a,PotJoint b){
 return (PotJoint){(a.x+b.x)/2,(a.y+b.y)/2};
}
static PotJoint pot_direction(PotJoint a,PotJoint b){
 int x=b.x-a.x,y=b.y-a.y;
 unsigned length=pot_sqrt((unsigned)(x*x+y*y));
 if(!length)return (PotJoint){0,256};
 return (PotJoint){x*256/(int)length,y*256/(int)length};
}
static PotJoint pot_local(PotJoint p,PotJoint across,PotJoint down,int x,int y){
 p.x+=pot_set_round(across.x*x+down.x*y,256);
 p.y+=pot_set_round(across.y*x+down.y*y,256);
 return p;
}
static int pot_has(const PotPose *pose,unsigned joint){
 return (pose->located&(1u<<joint))!=0u;
}
#include "potheads_shape.h"
#include "potheads_sign_geometry.h"
static void pot_shape_edge(uint8_t *p,const PotPoint *v,unsigned n,unsigned ink){
 for(unsigned i=0;i<n;i++){
  unsigned j=i+1u==n?0u:i+1u;
  hd_line(p,v[i].x,v[i].y,v[j].x,v[j].y,ink);
 }
}
static void pot_cloth(uint8_t *p,const PotCamera *c,const PotJoint *j,
                      unsigned count,unsigned ink,int outline){
 PotPoint v[8];
 if(count>8u)return;
 for(unsigned i=0;i<count;i++)v[i]=pot_actor_point(c,j[i]);
 pot_shape(p,v,count,ink);
 if(outline)pot_shape_edge(p,v,count,HD_INK);
}
static void pot_stroke(uint8_t *p,PotPoint a,PotPoint b,int width,
                       unsigned body,unsigned light,unsigned caps){
 int dx=b.x-a.x,dy=b.y-a.y;
 unsigned length=pot_sqrt((unsigned)(dx*dx+dy*dy));
 int half=width/2,xx=half,yy=0;
 if(length){xx=dy*half/(int)length;yy=-dx*half/(int)length;}
 PotPoint q[4]={{a.x+xx,a.y+yy},{b.x+xx,b.y+yy},
                {b.x-xx,b.y-yy},{a.x-xx,a.y-yy}};
 pot_shape(p,q,4u,body);
 /* Only the outside seams define the sleeve/trouser silhouette. Circular
    black end outlines made each measured joint look like a separate cuff. */
 hd_line(p,q[0].x,q[0].y,q[1].x,q[1].y,HD_INK);
 hd_line(p,q[3].x,q[3].y,q[2].x,q[2].y,HD_INK);
 if(half>1){
  if(caps&1u)hd_ellipse(p,a.x,a.y,half,half,body);
  if(caps&2u)hd_ellipse(p,b.x,b.y,half,half,body);
 }
 hd_line(p,a.x+xx/2,a.y+yy/2,b.x+xx/2,b.y+yy/2,light);
}
/* Taper both ends even in tightly folded limbs, instead of using the
   knee/elbow width for the entire lower segment. */
static void pot_taper_panel(uint8_t *p,PotPoint a,PotPoint b,int wa,int wb,
                             unsigned body,unsigned lit,unsigned caps){
 int dx=b.x-a.x,dy=b.y-a.y;
 unsigned l=pot_sqrt((unsigned)(dx*dx+dy*dy));
 if(!l){pot_stroke(p,a,b,wa>wb?wa:wb,body,lit,caps);return;}
 PotPoint na={dy*wa/(2*(int)l),-dx*wa/(2*(int)l)};
 PotPoint nb={dy*wb/(2*(int)l),-dx*wb/(2*(int)l)};
 PotPoint q[4]={{a.x+na.x,a.y+na.y},{b.x+nb.x,b.y+nb.y},
                {b.x-nb.x,b.y-nb.y},{a.x-na.x,a.y-na.y}};
 pot_shape(p,q,4u,body);
 hd_line(p,q[0].x,q[0].y,q[1].x,q[1].y,HD_INK);
 hd_line(p,q[3].x,q[3].y,q[2].x,q[2].y,HD_INK);
 hd_line(p,a.x+na.x/2,a.y+na.y/2,b.x+nb.x/2,b.y+nb.y/2,lit);
 if((caps&1u)&&wa>=4)hd_ellipse(p,a.x,a.y,wa/2,wa/2,body);
 if((caps&2u)&&wb>=4)hd_ellipse(p,b.x,b.y,wb/2,wb/2,body);
}
/* Taper a connected two-bone fabric ribbon. The bend uses one shared
   cross-section; no circular black cuff at the elbow or knee. For a
   nearly reversing limb, preserve both measured segments and use a
   rounded material joint rather than a long self-crossing miter. */
static void pot_jointed(uint8_t *p,PotPoint a,PotPoint b,PotPoint c,
                         int wa,int wb,int wc,unsigned body,unsigned lit,
                         unsigned shade,unsigned caps){
 int dx=b.x-a.x,dy=b.y-a.y,ex=c.x-b.x,ey=c.y-b.y;
 unsigned l=pot_sqrt((unsigned)(dx*dx+dy*dy)),m=pot_sqrt((unsigned)(ex*ex+ey*ey));
 /* A projected segment shorter than its own half-width can fold
    through the adjacent ribbon. Use the bounded tapered fallback there
    as well as at a true reversal. */
 if(!l||!m||l<(unsigned)(wa/2)||m<(unsigned)(wc/2)||dx*ex+dy*ey<-(int)(l*m/2u)){
  pot_taper_panel(p,a,b,wa,wb,body,lit,caps&1u);
  pot_taper_panel(p,b,c,wb,wc,body,shade,caps&2u);
  if(wb>=3)hd_ellipse(p,b.x,b.y,wb/2,wb/2,body);
  return;
 }
 int nx=dy*256/(int)l,ny=-dx*256/(int)l;
 int mx=ey*256/(int)m,my=-ex*256/(int)m;
 int jx=(nx+mx)/2,jy=(ny+my)/2;
 unsigned jl=pot_sqrt((unsigned)(jx*jx+jy*jy));
 if(jl){jx=jx*256/(int)jl;jy=jy*256/(int)jl;}
 else {jx=nx;jy=ny;}
 PotPoint n[3]={{nx*wa/512,ny*wa/512},{jx*wb/512,jy*wb/512},{mx*wc/512,my*wc/512}};
 /* A bounded fabric-colored bevel bridges raster rounding at the
    shared elbow/knee. It is under both panels, never an outline cuff. */
 if(wb>=3)hd_ellipse(p,b.x,b.y,wb/2,wb/2,body);
 PotPoint centers[3]={a,b,c},lo[3],hi[3];
 for(unsigned i=0;i<3u;i++){
  lo[i]=(PotPoint){centers[i].x-n[i].x,centers[i].y-n[i].y};
  hi[i]=(PotPoint){centers[i].x+n[i].x,centers[i].y+n[i].y};
 }
 /* Paint the connected silhouette first, then a single cloth plane.
    The old per-segment outline could overlap a second panel at the
    bend, leaving a black mechanical hinge. Neither panel has an
    artificial cross-limb seam at the tracked joint. */
 for(unsigned i=0;i<2u;i++){
  PotPoint quad[4]={lo[i],hi[i],hi[i+1u],lo[i+1u]};
  pot_shape(p,quad,4u,body);
 }
 for(unsigned i=0;i<2u;i++){
  PotPoint panel[4]={centers[i],hi[i],hi[i+1u],centers[i+1u]};
  pot_shape(p,panel,4u,shade);
 }
 /* Outer shadow edge and one fabric highlight, not twin black rails.
    At a compressed wrist or ankle the highlight ends naturally. */
 for(unsigned i=0;i<2u;i++){
  hd_line(p,hi[i].x,hi[i].y,hi[i+1u].x,hi[i+1u].y,HD_INK);
  if(n[i].x||n[i].y){
   PotPoint x={centers[i].x-n[i].x/2,centers[i].y-n[i].y/2};
   PotPoint y={centers[i+1u].x-n[i+1u].x/2,centers[i+1u].y-n[i+1u].y/2};
   hd_line(p,x.x,x.y,y.x,y.y,lit);
  }
 }
 if((caps&1u)&&wa>=4)hd_ellipse(p,a.x,a.y,wa/2,wa/2,body);
 if((caps&2u)&&wc>=4)hd_ellipse(p,c.x,c.y,wc/2,wc/2,body);
}
static unsigned pot_caps(const PotPose *pose,unsigned a,unsigned b){
 return ((pose->boundary&(1u<<a))?0u:1u)|((pose->boundary&(1u<<b))?0u:2u);
}
/* Paint contact anatomy as a short, tapered shape between the
   measured wrist and palm. A palm plant lies on its measured contact
   even when the wrist is inferred; no extra limb is created. */
static void pot_support_palm(uint8_t *p,PotPoint wrist,PotPoint palm,int radius,int connected){
 int dx=palm.x-wrist.x,dy=palm.y-wrist.y;
 unsigned len=pot_sqrt((unsigned)(dx*dx+dy*dy));
 int half=radius>1?radius-1:1;
 if(connected&&len&&len<=(unsigned)(radius*4)){
  int nx=dy*half/(int)len,ny=-dx*half/(int)len;
  PotPoint q[4]={{wrist.x+nx,wrist.y+ny},{palm.x+nx,palm.y+ny},
                 {palm.x-nx,palm.y-ny},{wrist.x-nx,wrist.y-ny}};
  pot_shape(p,q,4u,HD_SKIN);
  hd_line(p,wrist.x,wrist.y,palm.x,palm.y,HD_SKIN);
 }
 /* Flatten the visible palm against the observed support instead
    of making a large spherical cap. The centre never moves. */
 hd_ellipse(p,palm.x,palm.y,radius+1,half,HD_SKIN);
 hd_line(p,palm.x-half,palm.y,palm.x+half,palm.y,HD_SKIN_LIT);
 hd_pixel(p,palm.x,palm.y+half,HD_BARK);
}
static void pot_arm(uint8_t *p,const PotCamera *c,const PotPose *pose,unsigned side){
 unsigned shoulder=side?POT_RSHOULDER:POT_LSHOULDER;
 unsigned elbow=side?POT_RELBOW:POT_LELBOW;
 unsigned wrist=side?POT_RWRIST:POT_LWRIST;
 int width=pot_actor_size(c,pose->head_size.x+1);
 if(pot_has(pose,shoulder)&&pot_has(pose,elbow)&&pot_has(pose,wrist)&&
    !(pose->boundary&(1u<<elbow))){
  pot_jointed(p,pot_actor_point(c,pose->joint[shoulder]),pot_actor_point(c,pose->joint[elbow]),
              pot_actor_point(c,pose->joint[wrist]),width+1,width,width>3?width-2:2,
              HD_ROAD,HD_ROAD_COOL,HD_STONE_DARK,pot_caps(pose,shoulder,wrist));
 }else{
  if(pot_has(pose,shoulder)&&pot_has(pose,elbow))
   pot_stroke(p,pot_actor_point(c,pose->joint[shoulder]),
    pot_actor_point(c,pose->joint[elbow]),width,HD_ROAD,HD_ROAD_COOL,pot_caps(pose,shoulder,elbow));
  if(pot_has(pose,elbow)&&pot_has(pose,wrist))
   pot_stroke(p,pot_actor_point(c,pose->joint[elbow]),
    pot_actor_point(c,pose->joint[wrist]),width-1,HD_ROAD,HD_STONE_DARK,pot_caps(pose,elbow,wrist)&(pot_has(pose,shoulder)?2u:3u));
 }
 unsigned contact=side+2u;
 int r=pot_actor_size(c,3);
 if(pose->contact_mask&(1u<<contact)){
  PotPoint palm=pot_actor_point(c,pose->contact[contact]);
  int linked=pot_has(pose,wrist)&&!(pose->boundary&(1u<<wrist));
  PotPoint hand=linked?pot_actor_point(c,pose->joint[wrist]):palm;
  pot_support_palm(p,hand,palm,r,linked);
 }else if(pose->visible&(1u<<wrist)){
  PotPoint hand=pot_actor_point(c,pose->joint[wrist]);
  hd_ellipse(p,hand.x,hand.y,r,r>1?r-1:1,HD_SKIN);
  hd_line(p,hand.x-1,hand.y-1,hand.x+1,hand.y-1,HD_SKIN_LIT);
 }
}
/* Grounded shoe: the actual measured sole contact is the bottom
   anchor, while the ankle and toe retain their tracked positions.
   Use a compact wedge rather than an inflated circular tube. */
static void pot_ground_shoe(uint8_t *p,PotPoint ankle,PotPoint toe,PotPoint sole,int r){
 int dx=toe.x-sole.x,dy=toe.y-sole.y;
 if(dx*dx+dy*dy>(r*3)*(r*3)){
  /* Badly separated source landmarks must not stretch a new shoe. */
  pot_stroke(p,ankle,toe,r*2,HD_STONE_DARK,HD_STONE,3u);
  hd_line(p,sole.x-r,sole.y,sole.x+r,sole.y,HD_STONE_LIT);
  return;
 }
 PotPoint q[5]={{ankle.x-r,ankle.y},{ankle.x+r,ankle.y},
                 toe,{sole.x+r,sole.y},{sole.x-r,sole.y}};
 PotPoint h[5];unsigned n=pot_sign_hull(q,5u,h);
 if(n>=3u){pot_shape(p,h,n,HD_STONE_DARK);pot_shape_edge(p,h,n,HD_INK);}
 else pot_stroke(p,ankle,toe,r*2,HD_STONE_DARK,HD_STONE,3u);
 hd_line(p,ankle.x,ankle.y,toe.x,toe.y,HD_STONE);
 hd_line(p,sole.x-r,sole.y,sole.x+r,sole.y,HD_STONE_LIT);
}
static void pot_leg(uint8_t *p,const PotCamera *c,const PotPose *pose,unsigned side){
 unsigned hip=side?POT_RHIP:POT_LHIP,knee=side?POT_RKNEE:POT_LKNEE;
 unsigned ankle=side?POT_RANKLE:POT_LANKLE,toe=side?POT_RTOE:POT_LTOE;
 /* Use a stable garment width for the whole floor sequence. The
    near-leg flag changes only draw order, not the physical trouser
    diameter. This removes a two-unit width pop at support changes. */
 int thick=(pose->head_size.x*3+1)/2+3;
 int width=pot_actor_size(c,thick);
 if(pot_has(pose,hip)&&pot_has(pose,knee)&&pot_has(pose,ankle)&&
    !(pose->boundary&(1u<<knee))){
  pot_jointed(p,pot_actor_point(c,pose->joint[hip]),pot_actor_point(c,pose->joint[knee]),
              pot_actor_point(c,pose->joint[ankle]),width+1,width>3?width-1:2,width>5?width-3:2,
              HD_ROAD,HD_ROAD_COOL,HD_INK,pot_caps(pose,hip,ankle));
 }else{
  if(pot_has(pose,hip)&&pot_has(pose,knee))
   pot_stroke(p,pot_actor_point(c,pose->joint[hip]),pot_actor_point(c,pose->joint[knee]),
    width,HD_ROAD,HD_ROAD_COOL,pot_caps(pose,hip,knee));
  if(pot_has(pose,knee)&&pot_has(pose,ankle))
   pot_stroke(p,pot_actor_point(c,pose->joint[knee]),pot_actor_point(c,pose->joint[ankle]),
    width>3?width-2:2,HD_ROAD,HD_STONE_DARK,pot_caps(pose,knee,ankle)&(pot_has(pose,hip)?2u:3u));
 }
 if(pot_has(pose,ankle)&&pot_has(pose,toe)&&
    !(pose->boundary&((1u<<ankle)|(1u<<toe)))){
  PotPoint foot=pot_actor_point(c,pose->joint[ankle]);
  PotPoint tip=pot_actor_point(c,pose->joint[toe]);
  if(pose->contact_mask&(1u<<side)){
   PotPoint sole=pot_actor_point(c,pose->contact[side]);
   pot_ground_shoe(p,foot,tip,sole,pot_actor_size(c,3));
   return;
  }
  pot_stroke(p,foot,tip,pot_actor_size(c,7),HD_STONE_DARK,HD_STONE,3u);
  int sx=0,sy=1;
  if(!(pose->contact_mask&(1u<<side))&&pot_has(pose,knee)){
   PotPoint k=pot_actor_point(c,pose->joint[knee]);
   int dx=tip.x-foot.x,dy=tip.y-foot.y;
   sx=dy<0?1:dy>0?-1:0;sy=dx<0?-1:dx>0?1:0;
   if(sx*(foot.x-k.x)+sy*(foot.y-k.y)<0){sx=-sx;sy=-sy;}
   if(!sx&&!sy)sy=1;
  }
  hd_line(p,foot.x+sx,foot.y+sy,tip.x+sx,tip.y+sy,HD_STONE_LIT);
 }
}
#include "potheads_sign_render.h"
static void pot_bag_draw(uint8_t *p,const PotCamera *c,const PotPose *pose){
 if(pose->bag_state==POT_BAG_HIDDEN)return;
 PotPoint bag=pot_actor_point(c,pose->bag);
 int rx=pot_actor_size(c,pose->bag_size.x),ry=pot_actor_size(c,pose->bag_size.y);
 PotPoint q[6]={{bag.x-rx+2,bag.y-ry},{bag.x+rx-2,bag.y-ry},
  {bag.x+rx,bag.y-ry+3},{bag.x+rx-1,bag.y+ry},
  {bag.x-rx+1,bag.y+ry},{bag.x-rx,bag.y-ry+3}};
 if(pose->bag_state==POT_BAG_ATTACHED&&pot_has(pose,POT_RSHOULDER)){
  PotPoint shoulder=pot_actor_point(c,pose->joint[POT_RSHOULDER]);
  hd_line(p,shoulder.x,shoulder.y,bag.x-2,bag.y-ry,HD_INK);
  hd_line(p,shoulder.x+1,shoulder.y,bag.x-1,bag.y-ry,HD_INK);
 }else{
  int reach=pose->bag_state==POT_BAG_RESTING?rx+5:rx;
  hd_line(p,bag.x-rx,bag.y,bag.x-reach,bag.y-ry-3,HD_INK);
  hd_line(p,bag.x-reach,bag.y-ry-3,bag.x+2,bag.y-ry-2,HD_INK);
 }
 if(pose->bag_state==POT_BAG_RESTING)
  hd_ellipse(p,bag.x,bag.y+ry,rx+2,1,HD_SHADOW);
 pot_shape(p,q,6u,HD_INK);
 hd_line(p,bag.x-rx+3,bag.y-ry+3,bag.x+rx-3,bag.y-ry+3,HD_ROAD_COOL);
 hd_rect(p,bag.x+rx/3,bag.y-1,2,1,HD_STONE_DARK);
}
static void pot_head_oval(uint8_t *p,const PotCamera *c,PotJoint centre,
                           PotJoint across,PotJoint down,int rx,int ry,unsigned ink){
 static const int16_t circle[8][2]={
  {256,0},{181,181},{0,256},{-181,181},
  {-256,0},{-181,-181},{0,-256},{181,-181}};
 PotJoint outline[8];
 for(unsigned i=0;i<8u;i++)
  outline[i]=pot_local(centre,across,down,circle[i][0]*rx/256,circle[i][1]*ry/256);
 pot_cloth(p,c,outline,8u,ink,0);
}
static void pot_head(uint8_t *p,const PotCamera *c,const PotPose *pose){
 if(!pot_has(pose,POT_HEAD)||!pot_has(pose,POT_NECK)||
    (pose->boundary&(1u<<POT_HEAD)))return;
 PotJoint centre=pose->joint[POT_HEAD];
 PotJoint down=pot_direction(centre,pose->joint[POT_NECK]);
 PotJoint across={down.y,-down.x};
 int rx=pose->head_size.x,ry=pose->head_size.y;
 unsigned facing=(pose->flags>>POT_FACING_SHIFT)&3u;
 PotPoint head=pot_actor_point(c,centre),neck=pot_actor_point(c,pose->joint[POT_NECK]);
 pot_stroke(p,head,neck,pot_actor_size(c,5),HD_SKIN,HD_SKIN_LIT,3u);
 pot_head_oval(p,c,centre,across,down,rx+1,ry+1,HD_INK);
 pot_head_oval(p,c,centre,across,down,rx,ry,HD_HAIR);
 if(facing!=3u){
  PotJoint face=pot_local(centre,across,down,0,1);
  pot_head_oval(p,c,face,across,down,rx-1,ry-1,HD_SKIN);
  if(rx>=3&&ry>=3)
   pot_head_oval(p,c,pot_local(face,across,down,-1,-1),across,down,rx-2,ry-2,HD_SKIN_LIT);
  if(rx>=3&&ry>=3){
   int look=facing==1u?-1:facing==2u?1:0;
   int jaw=rx-1,chin=ry-1;
   /* Rotate the beard with the measured head, including inversions.
      A rear-facing pose never receives any facial hair. */
   PotJoint beard[6]={
    pot_local(centre,across,down,-jaw,1),
    pot_local(centre,across,down,-jaw,chin-1),
    pot_local(centre,across,down,-jaw/2,chin+1),
    pot_local(centre,across,down,jaw/2+look,chin+1),
    pot_local(centre,across,down,jaw,chin-1),
    pot_local(centre,across,down,jaw,1)};
   pot_cloth(p,c,beard,6u,HD_HAIR,0);
   PotJoint warm[4]={
    pot_local(centre,across,down,-jaw/2,chin-2),
    pot_local(centre,across,down,jaw/2+look,chin-2),
    pot_local(centre,across,down,jaw/2,chin),
    pot_local(centre,across,down,-jaw/2,chin)};
   pot_cloth(p,c,warm,4u,HD_COPPER,0);
   PotPoint upper=pot_actor_point(c,pot_local(centre,across,down,look,1));
   hd_pixel(p,upper.x,upper.y,HD_SKIN_LIT);
  }
 }
 PotJoint hair[6]={pot_local(centre,across,down,-rx,0),
  pot_local(centre,across,down,-rx,-ry/2),pot_local(centre,across,down,-rx/3,-ry-1),
  pot_local(centre,across,down,rx-1,-ry+1),pot_local(centre,across,down,rx,-2),
  pot_local(centre,across,down,0,-ry/2+1)};
 pot_cloth(p,c,hair,6u,HD_HAIR,0);
 if(rx>=4){
  PotPoint a=pot_actor_point(c,pot_local(centre,across,down,-rx/2,-ry+1));
  PotPoint b=pot_actor_point(c,pot_local(centre,across,down,1,-ry));
  hd_line(p,a.x,a.y,b.x,b.y,HD_BARK);
 }
 int look=facing==1u?-1:facing==2u?1:0;
 for(int side=-1;side<=1;side+=2){
  /* A profile exposes the earpiece behind the face, not both cups.
     Its material highlight follows the head's measured orientation. */
  if(look&&side==look)continue;
  PotPoint cup=pot_actor_point(c,pot_local(centre,across,down,side*(rx+1),0));
  PotPoint glint=pot_actor_point(c,pot_local(centre,across,down,side*(rx+1),-1));
  hd_ellipse(p,cup.x,cup.y,pot_actor_size(c,2),pot_actor_size(c,3),HD_INK);
  hd_pixel(p,glint.x,glint.y,HD_ROAD_COOL);
 }
 if(facing!=3u&&c->rise>140&&rx>=3&&ry>=3){
  int gap=rx>=6?3:2;
  for(int side=-1;side<=1;side+=2)if(!look||look==side){
   int eye_x=side*gap;
   PotPoint eye=pot_actor_point(c,pot_local(centre,across,down,eye_x,0));
   PotPoint brow=pot_actor_point(c,pot_local(centre,across,down,eye_x,-1));
   hd_pixel(p,brow.x,brow.y,HD_BARK);
   hd_pixel(p,eye.x,eye.y,HD_INK);
  }
  /* Brow, nose and mouth stay in head-local coordinates on the upside-down
     poses; the reference determines orientation, not a synthetic blink. */
  PotPoint nose=pot_actor_point(c,pot_local(centre,across,down,look*(gap+1),2));
  hd_pixel(p,nose.x,nose.y,HD_SKIN_LIT);
  int m0=look?look:-(rx/3),m1=look?look*(rx/2):rx/3;
  PotPoint a=pot_actor_point(c,pot_local(centre,across,down,m0,ry-2));
  PotPoint b=pot_actor_point(c,pot_local(centre,across,down,m1,ry-2));
  hd_line(p,a.x,a.y,b.x,b.y,HD_BARK);
 }
}
static void pot_source_cloth_draw(uint8_t *p,const PotCamera *c,int frame){
#if POTHEADS_CLOTH_ENABLED
 if(frame<0)return;
 unsigned a=pot_cloth_off[frame],b=pot_cloth_off[frame+1];
 for(unsigned k=a;k<b;k+=3u){
  PotClothRect r;
  if(pot_cloth_rect(&r,pot_cloth_spans[k+1],pot_cloth_spans[k],pot_cloth_spans[k+2],
                    c->zoom,c->ground,c->rise,c->skew,c->ground_y))
   hd_rect(p,r.x,r.y,r.w,r.h,r.lit?HD_LAMP:HD_GOLD);
 }
#else
 (void)p;(void)c;(void)frame;
#endif
}
static void pot_source_leg_draw(uint8_t *p,const PotCamera *c,int frame){
#if POTHEADS_LEG_ENABLED
 if(frame<0)return;
 static const uint8_t palette[5]={HD_INK,HD_ROAD,HD_ROAD_COOL,HD_STONE_DARK,HD_STONE};
 unsigned a=pot_leg_off[frame],b=pot_leg_off[frame+1];
 for(unsigned k=a;k<b;k+=3u){
  PotClothRect r;unsigned ink;
  if(pot_leg_rect(&r,&ink,pot_leg_spans[k+1],pot_leg_spans[k],pot_leg_spans[k+2],
                  c->zoom,c->ground,c->rise,c->skew,c->ground_y))
   hd_rect(p,r.x,r.y,r.w,r.h,palette[ink-1u]);
 }
#else
 (void)p;(void)c;(void)frame;
#endif
}
static void pot_source_upper_draw(uint8_t *p,const PotCamera *c,int frame){
#if POTHEADS_UPPER_ENABLED
 if(frame<0)return;
 static const uint8_t palette[5]={HD_INK,HD_ROAD,HD_ROAD_COOL,HD_SKIN,HD_SKIN_LIT};
 unsigned a=pot_upper_off[frame],b=pot_upper_off[frame+1];
 int settled=c->zoom==256&&c->ground==48&&c->rise==256&&c->skew==0&&c->ground_y==128;
 for(unsigned k=a;k<b;k+=4u){
  int y=pot_upper_rects[k],x=pot_upper_rects[k+1],h=pot_upper_rects[k+3];
  unsigned packed=pot_upper_rects[k+2];
  PotClothRect r;
  if(!pot_upper_settled_rect(&r,y,x,packed,h))continue;
  unsigned ink=palette[(packed>>5)-1u];
  if(settled)hd_rect(p,r.x,r.y,r.w,r.h,ink);
  else for(int j=0;j<h;j++){
   PotClothRect row;
   if(pot_cloth_rect(&row,x,y+j,r.w,c->zoom,c->ground,c->rise,c->skew,c->ground_y))
    hd_rect(p,row.x,row.y,row.w,row.h,ink);
  }
 }
#else
 (void)p;(void)c;(void)frame;
#endif
}
/* Shared anatomical torso axis: render-only cloth cross-sections
   prevent hip/shoulder ordering from forming a crossed bow tie.
   Do not move joints, contact points, cropped or private matte data. */
static int pot_torso_width(PotJoint a,PotJoint b,PotJoint across,int low,int high){
 int v=(a.x-b.x)*across.x+(a.y-b.y)*across.y;
 if(v<0)v=-v;
 v=(v+256)/512;
 return v<low?low:v>high?high:v;
}
static int pot_torso_sections(const PotPose *pose,PotJoint *ls,PotJoint *rs,PotJoint *lh,PotJoint *rh){
 const unsigned mask=(1u<<POT_LSHOULDER)|(1u<<POT_RSHOULDER)|(1u<<POT_LHIP)|(1u<<POT_RHIP);
 if((pose->located&mask)!=mask||(pose->boundary&mask))return 0;
 PotJoint s=pot_mid(*ls,*rs),h=pot_mid(*lh,*rh);
 int dx=h.x-s.x,dy=h.y-s.y;
 if(dx*dx+dy*dy<9)return 0;
 PotJoint down=pot_direction(s,h),across={down.y,-down.x};
 if((ls->x-rs->x)*across.x+(ls->y-rs->y)*across.y<0){across.x=-across.x;across.y=-across.y;}
 int rx=pose->head_size.x,min=rx/2;
 if(min<2)min=2;
 int sw=pot_torso_width(*ls,*rs,across,min,rx*2+2);
 int hw=pot_torso_width(*lh,*rh,across,min,(rx*3+1)/2+2);
 *ls=pot_local(s,across,down,sw,0);*rs=pot_local(s,across,down,-sw,0);
 *lh=pot_local(h,across,down,hw,0);*rh=pot_local(h,across,down,-hw,0);
 return 1;
}
/* Bevel the two corners beside the hem instead of introducing a
   long needle-point on the costume. All resulting vertices remain
   on the original annotated/derived contour edges. */
static PotJoint pot_costume_mix(PotJoint a,PotJoint b,int eighths){
 return (PotJoint){a.x+pot_set_round((b.x-a.x)*eighths,8),
                   a.y+pot_set_round((b.y-a.y)*eighths,8)};
}
static void pot_costume_bevel(const PotJoint suit[7],PotJoint rounded[8]){
 rounded[0]=pot_costume_mix(suit[0],suit[6],1);
 rounded[1]=pot_costume_mix(suit[1],suit[2],1);
 rounded[2]=suit[2];rounded[3]=suit[3];
 rounded[4]=pot_costume_mix(suit[4],suit[3],2);
 rounded[5]=pot_costume_mix(suit[4],suit[5],2);
 rounded[6]=suit[5];rounded[7]=suit[6];
}
static void pot_tunic(uint8_t *p,const PotCamera *c,const PotPose *pose,int source_cloth){
 unsigned body=(1u<<POT_LSHOULDER)|(1u<<POT_RSHOULDER)|(1u<<POT_LHIP)|(1u<<POT_RHIP);
 if((pose->located&body)!=body)return;
 PotJoint ls=pose->joint[POT_LSHOULDER],rs=pose->joint[POT_RSHOULDER];
 PotJoint lh=pose->joint[POT_LHIP],rh=pose->joint[POT_RHIP];
 if(source_cloth<0)(void)pot_torso_sections(pose,&ls,&rs,&lh,&rh);
 PotJoint shoulder=pot_mid(ls,rs),hip=pot_mid(lh,rh);
 PotJoint down=pot_direction(shoulder,hip),across={down.y,-down.x};
 if((ls.x-rs.x)*across.x+(ls.y-rs.y)*across.y<0){across.x=-across.x;across.y=-across.y;}
 PotJoint chest[4]={ls,rs,rh,lh};
 pot_cloth(p,c,chest,4u,HD_ROAD,1);
 if(source_cloth>=0)return; /* Source contour is drawn after the articulated limbs. */
 /* Loose cloth falls screen-down during inversion, exposing the dark
    shirt/waist; it does not remain a rigid upright banana silhouette. */
 int drape=pot_drape_weight(down.y);
 PotJoint ml=pot_mid(ls,lh),mr=pot_mid(rs,rh);
 PotJoint low_left={pot_drape_mix(lh.x,ml.x,drape),pot_drape_mix(lh.y,ml.y,drape)};
 PotJoint low_right={pot_drape_mix(rh.x,mr.x,drape),pot_drape_mix(rh.y,mr.y,drape)};
 PotJoint hem=pot_mid(low_left,low_right);
 int dx=hip.x-shoulder.x,dy=hip.y-shoulder.y;
 int tail=(int)pot_sqrt((unsigned)(dx*dx+dy*dy))/3;
 if(tail<3)tail=3;
 if(tail>9)tail=9;
 int fold_width=pose->head_size.x/2;
 if(fold_width<2)fold_width=2;
 if(fold_width>4)fold_width=4;
 hem=pot_local(hem,across,down,1,pot_drape_mix(tail,-3,drape));
 hem.y+=pot_drape_mix(3,10,drape);
 PotJoint suit[7]={pot_local(ls,across,down,fold_width,-2),
  pot_local(rs,across,down,-fold_width,-2),pot_local(rs,across,down,-fold_width-1,5),
  pot_local(low_right,across,down,-fold_width,3),hem,
  pot_local(low_left,across,down,fold_width,3),pot_local(ls,across,down,fold_width+1,5)};
 PotJoint rounded[8];pot_costume_bevel(suit,rounded);
 pot_cloth(p,c,rounded,8u,HD_GOLD,0);
 /* Inset each cross-section of the light plane along the matching
    costume edge. It shares the rounded hem instead of projecting
    a second, sharper triangle out of the main silhouette. */
 PotJoint light[8]={
  pot_costume_mix(rounded[0],rounded[1],2),pot_costume_mix(rounded[1],rounded[0],1),
  pot_costume_mix(rounded[2],rounded[7],1),pot_costume_mix(rounded[3],rounded[6],1),
  pot_costume_mix(rounded[4],rounded[5],1),pot_costume_mix(rounded[5],rounded[4],2),
  pot_costume_mix(rounded[6],rounded[3],2),pot_costume_mix(rounded[7],rounded[2],2)};
 pot_cloth(p,c,light,8u,HD_LAMP,0);
 hem=pot_mid(rounded[4],rounded[5]);
 /* A central folded seam follows the torso and its gravity-driven hem,
    rather than a static yellow block. Keep source matte builds intact. */
 PotPoint seam0=pot_actor_point(c,pot_local(shoulder,across,down,-2,5));
 PotPoint seam1=pot_actor_point(c,pot_local(hem,across,down,-1,-3));
 hd_line(p,seam0.x,seam0.y,seam1.x,seam1.y,HD_GOLD);
 if(pot_has(pose,POT_NECK)){
  int collar=pose->head_size.x/2+1;
  if(collar<2)collar=2;
  if(collar>5)collar=5;
  /* Compact folded collar; the old 7-unit spikes made the costume
     look like rigid shoulders/antennae during floor work. */
  PotJoint hood[4]={pose->joint[POT_NECK],pot_local(rs,across,down,-collar,-3),
                    pot_local(rs,across,down,-collar-1,1),pot_local(rs,across,down,-1,5)};
  pot_cloth(p,c,hood,4u,HD_GOLD,0);
  hood[1]=pot_local(ls,across,down,collar,-3);
  hood[2]=pot_local(ls,across,down,collar+1,1);
  hood[3]=pot_local(ls,across,down,1,5);
  pot_cloth(p,c,hood,4u,HD_LAMP,0);
  PotPoint tip=pot_actor_point(c,hood[1]);
  hd_pixel(p,tip.x,tip.y,HD_BARK);
 }
}
/* A bounded pelvic yoke follows the measured hip centres rather than
   painting a fat transverse capsule over both thighs. The two lower
   corners follow the actual knee directions where those joints exist.
   A cropped hip never receives a synthetic rounded end. */
static unsigned pot_pelvis_outline(const PotPose *pose,PotJoint q[6]){
 const unsigned hips=(1u<<POT_LHIP)|(1u<<POT_RHIP);
 if((pose->located&hips)!=hips||(pose->boundary&hips))return 0;
 PotJoint l=pose->joint[POT_LHIP],r=pose->joint[POT_RHIP];
 PotJoint centre=pot_mid(l,r);
 PotJoint down={0,256};
 if(pot_has(pose,POT_LSHOULDER)&&pot_has(pose,POT_RSHOULDER))
  down=pot_direction(pot_mid(pose->joint[POT_LSHOULDER],pose->joint[POT_RSHOULDER]),centre);
 else if(pot_has(pose,POT_NECK))down=pot_direction(pose->joint[POT_NECK],centre);
 else if(pot_has(pose,POT_LKNEE)&&pot_has(pose,POT_RKNEE))
  down=pot_direction(centre,pot_mid(pose->joint[POT_LKNEE],pose->joint[POT_RKNEE]));
 PotJoint across={down.y,-down.x};
 if((l.x-r.x)*across.x+(l.y-r.y)*across.y<0){across.x=-across.x;across.y=-across.y;}
 int rx=pose->head_size.x;
 int edge=rx/3+1;if(edge<2)edge=2;if(edge>4)edge=4;
 int depth=rx/2+1;if(depth<2)depth=2;if(depth>5)depth=5;
 PotJoint ld=down,rd=down;
 if(pot_has(pose,POT_LKNEE)&&!(pose->boundary&(1u<<POT_LKNEE)))
  ld=pot_direction(l,pose->joint[POT_LKNEE]);
 if(pot_has(pose,POT_RKNEE)&&!(pose->boundary&(1u<<POT_RKNEE)))
  rd=pot_direction(r,pose->joint[POT_RKNEE]);
 q[0]=pot_local(l,across,down,edge,-depth);
 q[1]=pot_local(r,across,down,-edge,-depth);
 q[2]=pot_local(r,across,down,-edge,0);
 q[3]=pot_local(r,across,rd,-edge/2,depth);
 q[4]=pot_local(l,across,ld,edge/2,depth);
 q[5]=pot_local(l,across,down,edge,0);
 return 6u;
}
static void pot_pelvis(uint8_t *p,const PotCamera *c,const PotPose *pose){
 PotJoint q[6];
 if(!pot_pelvis_outline(pose,q))return;
 /* Convex envelope of these six bounded garment points keeps inverted
    or crossed hip estimates from producing a bow-tie patch. */
 PotPoint v[6];
 for(unsigned i=0;i<6u;i++)v[i]=pot_actor_point(c,q[i]);
 /* The outline is small: sort its points and form the six-point hull.
    The general sign hull is intentionally limited to five vertices. */
 PotPoint hull[12];unsigned n=0;
 for(unsigned i=1;i<6u;i++){
  PotPoint x=v[i];unsigned j=i;
  while(j&&(v[j-1u].x>x.x||(v[j-1u].x==x.x&&v[j-1u].y>x.y))){v[j]=v[j-1u];j--;}
  v[j]=x;
 }
 for(unsigned i=0;i<6u;i++){
  while(n>=2u&&pot_sign_cross(hull[n-2u],hull[n-1u],v[i])<=0)n--;
  hull[n++]=v[i];
 }
 unsigned lower=n;
 for(int i=4;i>=0;i--){
  while(n>lower&&pot_sign_cross(hull[n-2u],hull[n-1u],v[i])<=0)n--;
  hull[n++]=v[i];
 }
 if(n>1u)n--;
 if(n>=3u&&n<=6u)pot_shape(p,hull,n,HD_ROAD);
 /* A short waistband shadow is anchored to the original hips, not
    an oversized sphere. It is occluded naturally by the tunic. */
 PotPoint a=pot_actor_point(c,pot_mid(q[0],q[5]));
 PotPoint b=pot_actor_point(c,pot_mid(q[1],q[2]));
 if(n>=3u&&pot_sign_dist(a,b)>=4)hd_line(p,a.x,a.y,b.x,b.y,HD_INK);
}
static void pot_dancer(uint8_t *p,const PotCamera *c,unsigned t){
 PotPose pose;
 unsigned source=t>180u?t-180u:0u;
 int cloth_frame=pot_source_cloth_frame(source);
 int leg_frame=pot_source_leg_frame(source);
 int upper_frame=pot_source_upper_frame(source,cloth_frame,leg_frame);
 /* All visible components share the held source frame, including
    source-only and partially installed matte builds. Floor staging below
    also uses this clock so a leg cannot switch depth ahead of its pose. */
 source=pot_source_pose_tick(source,cloth_frame,leg_frame,upper_frame);
 pot_render_sample(source,&pose,cloth_frame>=0||leg_frame>=0||upper_frame>=0);
 unsigned sign=pose.flags&3u;
 for(unsigned i=0;i<4u;i++)if(pose.contact_mask&(1u<<i)){
  PotPoint contact=pot_actor_point(c,pose.contact[i]);
  hd_ellipse(p,contact.x,contact.y,pot_actor_size(c,i<2u?6:3),1,HD_SHADOW);
 }
 if(!pose.contact_mask&&!(pose.flags&POT_GROUNDED)){
  PotJoint mid=pot_mid(pose.joint[POT_LHIP],pose.joint[POT_RHIP]);mid.y=0;
  PotPoint shadow=pot_actor_point(c,mid);
  hd_ellipse(p,shadow.x,shadow.y,pot_actor_size(c,11),1,HD_SHADOW);
 }
 if(pose.bag_state==POT_BAG_RESTING)pot_bag_draw(p,c,&pose);
 if(sign&&!(pose.flags&POT_SIGN_FRONT))pot_sign_draw(p,c,&pose);
 unsigned near_arm=(pose.flags&POT_NEAR_LEFT)?0u:1u;
 unsigned near_leg=(pose.flags&POT_NEAR_LEFT_LEG)?0u:1u;
 int floor_work=source>=570u;
 if(upper_frame<0)pot_arm(p,c,&pose,near_arm^1u);
 /* Private source matte already contains the visible overlap between the
    legs, so do not paint either articulated leg or the old hip stroke. */
 if(leg_frame<0){
  pot_leg(p,c,&pose,near_leg^1u);
  pot_pelvis(p,c,&pose);
  if(!floor_work)pot_leg(p,c,&pose,near_leg);
 }else if(!floor_work)pot_source_leg_draw(p,c,leg_frame);
 if(upper_frame<0)pot_tunic(p,c,&pose,cloth_frame);
 pot_head(p,c,&pose);
 if(floor_work){
  if(leg_frame>=0)pot_source_leg_draw(p,c,leg_frame);
  else pot_leg(p,c,&pose,near_leg);
 }
 int front_sign=sign&&(pose.flags&POT_SIGN_FRONT);
 if(pose.bag_state!=POT_BAG_RESTING&&(upper_frame<0||front_sign))pot_bag_draw(p,c,&pose);
 if(upper_frame<0)pot_arm(p,c,&pose,near_arm);
 if(cloth_frame>=0)pot_source_cloth_draw(p,c,cloth_frame);
 if(front_sign)pot_sign_draw(p,c,&pose);
 // Source matte contains only visible sleeve/skin pixels, including
 // visible grips in front of the sign. No duplicate joint hands.
 if(upper_frame>=0){
  pot_source_upper_draw(p,c,upper_frame);
  if(!front_sign&&pose.bag_state!=POT_BAG_RESTING)pot_bag_draw(p,c,&pose);
 }
 if(sign==POT_SIGN_HELD&&front_sign&&upper_frame<0){
  PotPoint q[5],h[5];unsigned n=pot_sign_projected(c,&pose,q,h);
  for(unsigned i=0;i<2u;i++){
   unsigned wrist=i?POT_RWRIST:POT_LWRIST;
   if(!(pose.visible&(1u<<wrist)))continue;
   PotPoint hand=pot_actor_point(c,pose.joint[wrist]);
   int radius=pot_actor_size(c,3);
   if(!pot_sign_near(h,n,hand,radius))continue;
   hd_ellipse(p,hand.x,hand.y,radius,pot_actor_size(c,2),HD_SKIN);
   hd_line(p,hand.x-1,hand.y-1,hand.x+1,hand.y-1,HD_SKIN_LIT);
  }
 }
}
