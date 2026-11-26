#pragma once
#include "potheads_motion.h"
#include "potheads_drape.h"
#include "potheads_cloth_render.h"
#include "potheads_leg_render.h"
#include "potheads_upper_render.h"

/* Source-timed articulated cutout. The anatomy and motion use measured
   landmarks; material planes and pixel lighting are authored native art.
   The final projection maps the complete source camera to 240x135. */
static int pot_abs(int value){return value<0?-value:value;}
static unsigned pot_sqrt(unsigned value){
 unsigned root=0,bit=1u<<30;
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
 return pot_project(c,113+pot_set_round(point.x*9,8),14,
                       -pot_set_round(point.y*9,8));
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
static HD_CANVAS_CODE void pot_shape(uint8_t *p,const PotPoint *v,unsigned count,unsigned ink){
 /* Scan-convert actual polygon spans, including the loose concave hem.
    Divide once per edge; never divide once per painted pixel or row. */
 typedef struct {int low,high,x,step;} Edge;
 Edge edges[8];unsigned n=0;
 int low=135,high=-1,minx=HD_W,maxx=-1;
 if(count>8u)return;
 for(unsigned i=0;i<count;i++){
  if(v[i].y<low)low=v[i].y;
  if(v[i].y>high)high=v[i].y;
  if(v[i].x<minx)minx=v[i].x;
  if(v[i].x>maxx)maxx=v[i].x;
 }
 if(high<0||low>=135||maxx<0||minx>=HD_W)return;
 if(low==high){hd_rect(p,minx,low,maxx-minx+1,1,ink);return;}
 if(low<0)low=0;
 if(high>134)high=134;
 for(unsigned i=0;i<count;i++){
  PotPoint a=v[i],b=v[(i+1u)%count];
  if(a.y>b.y){PotPoint swap=a;a=b;b=swap;}
  if(a.y==b.y||b.y<=low||a.y>high)continue;
  int first=a.y<low?low:a.y;
  int step=(b.x-a.x)*65536/(b.y-a.y);
  edges[n++]=(Edge){first,b.y,a.x*65536+step*(first-a.y),step};
 }
 for(int y=low;y<=high;y++){
  int hits[8];unsigned used=0;
  for(unsigned i=0;i<n;i++)if(y>=edges[i].low&&y<edges[i].high){
   int x=edges[i].x;unsigned at=used;
   while(at&&hits[at-1u]>x){hits[at]=hits[at-1u];at--;}
   hits[at]=x;used++;edges[i].x+=edges[i].step;
  }
  for(unsigned i=0;i+1u<used;i+=2u){
   int x=pot_set_round(hits[i],65536),xx=pot_set_round(hits[i+1u],65536);
   hd_rect(p,x,y,xx-x+1,1,ink);
  }
 }
}
static void pot_shape_edge(uint8_t *p,const PotPoint *v,unsigned n,unsigned ink){
 for(unsigned i=0;i<n;i++)
  hd_line(p,v[i].x,v[i].y,v[(i+1u)%n].x,v[(i+1u)%n].y,ink);
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
static unsigned pot_caps(const PotPose *pose,unsigned a,unsigned b){
 return ((pose->boundary&(1u<<a))?0u:1u)|((pose->boundary&(1u<<b))?0u:2u);
}
static void pot_arm(uint8_t *p,const PotCamera *c,const PotPose *pose,unsigned side){
 unsigned shoulder=side?POT_RSHOULDER:POT_LSHOULDER;
 unsigned elbow=side?POT_RELBOW:POT_LELBOW;
 unsigned wrist=side?POT_RWRIST:POT_LWRIST;
 int width=pot_actor_size(c,pose->head_size.x+1);
 if(pot_has(pose,shoulder)&&pot_has(pose,elbow))
  pot_stroke(p,pot_actor_point(c,pose->joint[shoulder]),
   pot_actor_point(c,pose->joint[elbow]),width,HD_ROAD,HD_ROAD_COOL,pot_caps(pose,shoulder,elbow));
 if(pot_has(pose,elbow)&&pot_has(pose,wrist))
  pot_stroke(p,pot_actor_point(c,pose->joint[elbow]),
   pot_actor_point(c,pose->joint[wrist]),width-1,HD_ROAD,HD_STONE_DARK,pot_caps(pose,elbow,wrist)&(pot_has(pose,shoulder)?2u:3u));
 if((pose->visible&(1u<<wrist))!=0u){
  PotPoint hand=pot_actor_point(c,pose->joint[wrist]);
  int r=pot_actor_size(c,3);
  unsigned contact=side+2u;
  if(pose->contact_mask&(1u<<contact)){
   PotPoint palm=pot_actor_point(c,pose->contact[contact]);
   int dx=palm.x-hand.x,dy=palm.y-hand.y;
   if(dx*dx+dy*dy<=r*r*16){
    pot_stroke(p,hand,palm,r*2,HD_SKIN,HD_SKIN_LIT,3u);
    hd_ellipse(p,palm.x,palm.y,r+1,r>1?r-1:1,HD_SKIN);
   }else hd_ellipse(p,hand.x,hand.y,r,r>1?r-1:1,HD_SKIN);
  }else{
   hd_ellipse(p,hand.x,hand.y,r,r>1?r-1:1,HD_SKIN);
   hd_line(p,hand.x-1,hand.y-1,hand.x+1,hand.y-1,HD_SKIN_LIT);
  }
 }
}
static void pot_leg(uint8_t *p,const PotCamera *c,const PotPose *pose,unsigned side){
 unsigned hip=side?POT_RHIP:POT_LHIP,knee=side?POT_RKNEE:POT_LKNEE;
 unsigned ankle=side?POT_RANKLE:POT_LANKLE,toe=side?POT_RTOE:POT_LTOE;
 int thick=pose->head_size.x*2;
 unsigned near=(pose->flags&POT_NEAR_LEFT_LEG)?0u:1u;
 /* Foreshortened foreground thighs broaden during the supported floor
    turns. Inverted airborne legs retain the standing proportions. */
 if(side==near&&(pose->contact_mask&12u)&&pot_has(pose,POT_NECK)&&
    pot_has(pose,hip)&&pose->joint[hip].y>pose->joint[POT_NECK].y)
  thick=pose->head_size.x*3+2;
 int width=pot_actor_size(c,thick);
 if(pot_has(pose,hip)&&pot_has(pose,knee))
  pot_stroke(p,pot_actor_point(c,pose->joint[hip]),pot_actor_point(c,pose->joint[knee]),
   width,HD_INK,HD_ROAD,pot_caps(pose,hip,knee));
 if(pot_has(pose,knee)&&pot_has(pose,ankle))
  pot_stroke(p,pot_actor_point(c,pose->joint[knee]),pot_actor_point(c,pose->joint[ankle]),
   width-2,HD_INK,HD_STONE_DARK,pot_caps(pose,knee,ankle)&(pot_has(pose,hip)?2u:3u));
 if(pot_has(pose,ankle)&&pot_has(pose,toe)&&
    !(pose->boundary&((1u<<ankle)|(1u<<toe)))){
  PotPoint foot=pot_actor_point(c,pose->joint[ankle]);
  PotPoint tip=pot_actor_point(c,pose->joint[toe]);
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
static HD_CANVAS_CODE PotPoint pot_sign_point(const PotPoint *q,int u,int v){
 /* Two measured edges retain the board's bend instead of assuming an
    upright HUD rectangle. Rows of lettering share the board's perspective. */
 int tx=q[0].x+(q[1].x-q[0].x)*u/64;
 int ty=q[0].y+(q[1].y-q[0].y)*u/64;
 int bx=q[3].x+(q[2].x-q[3].x)*u/64;
 int by=q[3].y+(q[2].y-q[3].y)*u/64;
 return (PotPoint){tx+(bx-tx)*v/26,ty+(by-ty)*v/26};
}
static const uint8_t *pot_sign_glyph(char ch){
 /* Ten tiny 5x7 ink forms, 70 bytes of ROM. They are drawn as strokes on
    the measured board, never into a second bitmap or decoded image. */
 static const uint8_t glyph[10][7]={
  {31,16,16,30,16,16,16},{30,17,17,30,20,18,17},
  {31,16,16,30,16,16,31},{17,17,17,21,21,27,17},
  {30,17,17,17,17,17,30},{14,17,1,2,4,8,31},
  {4,12,4,4,4,4,14},{12,18,20,8,21,18,13},
  {17,17,17,17,17,17,14},{30,17,17,30,16,16,16}
 };
 const char *letters="FREWD21&UP";
 for(unsigned i=0;i<10u;i++)if(ch==letters[i])return glyph[i];
 return 0;
}
static void pot_sign_word(uint8_t *p,const PotPoint *q,const char *word,int x,int y){
 for(unsigned i=0;word[i];i++){
  const uint8_t *glyph=pot_sign_glyph(word[i]);
  if(!glyph)continue;
  for(unsigned row=0;row<7u;row++){
   unsigned bits=glyph[row];
   for(unsigned col=0;col<5u;){
    if(!(bits&(16u>>col))){col++;continue;}
    unsigned start=col;
    while(col<5u&&(bits&(16u>>col)))col++;
    PotPoint a=pot_sign_point(q,x+(int)i*6+(int)start,y+(int)row);
    PotPoint b=pot_sign_point(q,x+(int)i*6+(int)col-1,y+(int)row);
    hd_line(p,a.x,a.y,b.x,b.y,HD_INK);
   }
  }
 }
}
static void pot_sign_draw(uint8_t *p,const PotCamera *c,const PotPose *pose){
 PotPoint q[5],outline[5],shadow[5];
 for(unsigned i=0;i<5u;i++)q[i]=pot_actor_point(c,pose->sign[i]);
 outline[0]=q[0];outline[1]=q[1];outline[2]=q[4];
 outline[3]=q[2];outline[4]=q[3];
 for(unsigned i=0;i<5u;i++)shadow[i]=(PotPoint){outline[i].x+2,outline[i].y+2};
 pot_shape(p,shadow,5u,HD_SHADOW);
 pot_shape(p,outline,5u,HD_PAPER);
 pot_shape_edge(p,outline,5u,HD_STONE_DARK);
 PotPoint a=pot_sign_point(q,2,2),b=pot_sign_point(q,61,2);
 hd_line(p,a.x,a.y,b.x,b.y,HD_PLASTER_LIT);
 a=pot_sign_point(q,46,1);b=pot_sign_point(q,46,25);
 hd_line(p,a.x,a.y,b.x,b.y,HD_STONE_LIT);
 if(c->rise>=128&&pot_abs(q[1].x-q[0].x)+pot_abs(q[1].y-q[0].y)>35){
  pot_sign_word(p,q,"FREE WEED",5,4);
  pot_sign_word(p,q,"21 & UP",11,15);
  /* Small hand-painted leaf marks sit on the arrow and lower corner. */
  PotPoint stem=pot_sign_point(q,4,23);
  for(int i=-2;i<=2;i++){
   PotPoint leaf=pot_sign_point(q,4+i*2,19+pot_abs(i));
   hd_line(p,stem.x,stem.y,leaf.x,leaf.y,HD_LEAF_DARK);
  }
 }
}
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
 for(int side=-1;side<=1;side+=2){
  PotPoint cup=pot_actor_point(c,pot_local(centre,across,down,side*(rx+1),0));
  hd_ellipse(p,cup.x,cup.y,pot_actor_size(c,2),pot_actor_size(c,3),HD_INK);
  hd_pixel(p,cup.x,cup.y-1,HD_ROAD_COOL);
 }
 if(facing!=3u&&c->rise>140){
  int look=facing==1u?-1:facing==2u?1:0;
  for(int side=-1;side<=1;side+=2)if(!look||look==side){
   PotPoint eye=pot_actor_point(c,pot_local(centre,across,down,side*2,0));
   hd_pixel(p,eye.x,eye.y,HD_INK);
  }
  PotPoint a=pot_actor_point(c,pot_local(centre,across,down,-3,ry-2));
  PotPoint b=pot_actor_point(c,pot_local(centre,across,down,3,ry-2));
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
static void pot_tunic(uint8_t *p,const PotCamera *c,const PotPose *pose,int source_cloth){
 unsigned body=(1u<<POT_LSHOULDER)|(1u<<POT_RSHOULDER)|(1u<<POT_LHIP)|(1u<<POT_RHIP);
 if((pose->located&body)!=body)return;
 PotJoint ls=pose->joint[POT_LSHOULDER],rs=pose->joint[POT_RSHOULDER];
 PotJoint lh=pose->joint[POT_LHIP],rh=pose->joint[POT_RHIP];
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
 hem=pot_local(hem,across,down,2,pot_drape_mix(9,-4,drape));
 hem.y+=pot_drape_mix(4,13,drape);
 PotJoint suit[7]={pot_local(ls,across,down,3,-2),
  pot_local(rs,across,down,-3,-2),pot_local(rs,across,down,-5,6),
  pot_local(low_right,across,down,-4,3),hem,
  pot_local(low_left,across,down,4,4),pot_local(ls,across,down,5,6)};
 pot_cloth(p,c,suit,7u,HD_GOLD,0);
 PotJoint light[5]={pot_local(ls,across,down,1,-1),pot_local(rs,across,down,0,0),
  pot_local(low_right,across,down,1,2),hem,pot_local(low_left,across,down,0,2)};
 pot_cloth(p,c,light,5u,HD_LAMP,0);
 PotJoint fold[3]={pot_local(ls,across,down,0,5),
  pot_local(low_left,across,down,0,2),pot_local(hem,across,down,-2,-2)};
 pot_cloth(p,c,fold,3u,HD_GOLD,0);
 if(pot_has(pose,POT_NECK)){
  PotJoint hood[3]={pose->joint[POT_NECK],pot_local(rs,across,down,-7,-6),
                    pot_local(rs,across,down,-1,7)};
  pot_cloth(p,c,hood,3u,HD_GOLD,0);
  hood[1]=pot_local(ls,across,down,7,-7);hood[2]=pot_local(ls,across,down,1,7);
  pot_cloth(p,c,hood,3u,HD_LAMP,0);
  PotPoint tip=pot_actor_point(c,hood[1]);
  hd_ellipse(p,tip.x,tip.y,1,1,HD_BARK);
 }
}
static void pot_dancer(uint8_t *p,const PotCamera *c,unsigned t){
 PotPose pose;
 unsigned source=t>180u?t-180u:0u;
 int cloth_frame=pot_source_cloth_frame(source);
 int leg_frame=pot_source_leg_frame(source);
 int upper_frame=pot_source_upper_frame(source,cloth_frame,leg_frame);
 // Sample authored face, props and contacts at the same source timestamp
 // as all three private mattes; avoid one-tick head/hand seam drift.
 pot_trace_sample(pot_upper_pose_tick(source,upper_frame),&pose);
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
  if(pot_has(&pose,POT_LHIP)&&pot_has(&pose,POT_RHIP))
   pot_stroke(p,pot_actor_point(c,pose.joint[POT_LHIP]),
    pot_actor_point(c,pose.joint[POT_RHIP]),pot_actor_size(c,pose.head_size.x*2),HD_INK,HD_ROAD,
    pot_caps(&pose,POT_LHIP,POT_RHIP));
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
 if(sign==POT_SIGN_HELD&&upper_frame<0){
  for(unsigned i=0;i<2u;i++){
   unsigned wrist=i?POT_RWRIST:POT_LWRIST;
   if(!(pose.visible&(1u<<wrist)))continue;
   PotPoint hand=pot_actor_point(c,pose.joint[wrist]);
   hd_ellipse(p,hand.x,hand.y,pot_actor_size(c,3),pot_actor_size(c,2),HD_SKIN);
  }
 }
}
