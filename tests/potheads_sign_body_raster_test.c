#include "hd_canvas.h"
#include "hd_potheads_set.h"
#include "hd_potheads_actor.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static struct {uint8_t before[32],pixels[HD_BYTES],after[32];} frame;
static void reset(void){
 memset(&frame,0xA6,sizeof frame);
 memset(frame.pixels,HD_WET,HD_BYTES);
}
static void guards(void){
 for(unsigned i=0;i<32u;i++)assert(frame.before[i]==0xA6&&frame.after[i]==0xA6);
 for(unsigned i=0;i<HD_BYTES;i++)assert(frame.pixels[i]<64u);
}
static int inside(PotPoint p){return p.x>=0&&p.x<HD_W&&p.y>=0&&p.y<135;}
static int painted_near(PotPoint p){
 for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){
  PotPoint q={p.x+dx,p.y+dy};
  if(inside(q)&&frame.pixels[q.y*HD_W+q.x]!=HD_WET)return 1;
 }
 return 0;
}
int main(void){
 unsigned compact_ink=0,full_ink=0;
 for(unsigned v=0;v<26u;v++)for(unsigned u=0;u<64u;u++){
  char a=pot_sign_texel(u,v,1u),b=pot_sign_texel(u,v,2u);
  assert(a=='X'||a==' ');assert(b==pot_sign_art[v][u]);
  compact_ink+=(a=='X');full_ink+=(b=='X');
  assert(pot_sign_texel(u,v,0u)==' ');
 }
 assert(compact_ink>full_ink&&compact_ink<64u*26u/2u);
 assert(pot_sign_texel(64u,0u,2u)==' '&&pot_sign_texel(0u,26u,1u)==' ');
 const PotJoint raw[7]={{0,0},{20,0},{23,4},{17,20},{10,30},{3,20},{-3,4}};
 PotJoint rounded[8];pot_costume_bevel(raw,rounded);
 assert(rounded[4].y<raw[4].y&&rounded[5].y<raw[4].y);
 assert(rounded[4].x>rounded[5].x);
 for(unsigned i=0;i<8u;i++)assert(rounded[i].x>=-3&&rounded[i].x<=23&&rounded[i].y>=0&&rounded[i].y<=30);
 static const unsigned bone[4][3]={
  {POT_LSHOULDER,POT_LELBOW,POT_LWRIST},{POT_RSHOULDER,POT_RELBOW,POT_RWRIST},
  {POT_LHIP,POT_LKNEE,POT_LANKLE},{POT_RHIP,POT_RKNEE,POT_RANKLE}
 };
 unsigned signs=0,limbs=0,joints=0,palms=0,soles=0,muted=0,torsos=0,material=0,pelvis=0,tip_planes=0;
 for(unsigned t=0;t<=POT_TRACE_LAST_TICK;t+=2u){
  PotPose pose;pot_render_sample(t,&pose,0);
  PotJoint ls=pose.joint[POT_LSHOULDER],rs=pose.joint[POT_RSHOULDER];
  PotJoint lh=pose.joint[POT_LHIP],rh=pose.joint[POT_RHIP];
  PotJoint s=pot_mid(ls,rs),h=pot_mid(lh,rh);
  if(pot_torso_sections(&pose,&ls,&rs,&lh,&rh)){
   PotJoint ns=pot_mid(ls,rs),nh=pot_mid(lh,rh);
   assert(ns.x==s.x&&ns.y==s.y&&nh.x==h.x&&nh.y==h.y);
   int ax=ls.x-rs.x,ay=ls.y-rs.y,bx=lh.x-rh.x,by=lh.y-rh.y;
   assert(ax*bx+ay*by>0); /* same side ordering, never a bow tie */
   assert(ax*ax+ay*ay<=4*(pose.head_size.x*2+4)*(pose.head_size.x*2+4));
   torsos++;
  }
  PotJoint hip_contour[6];
  unsigned pelvis_n=pot_pelvis_outline(&pose,hip_contour);
  if(pelvis_n){
   assert(pelvis_n==6u);
   assert(!(pose.boundary&((1u<<POT_LHIP)|(1u<<POT_RHIP))));
   for(unsigned j=0;j<pelvis_n;j++){
    PotJoint anchor=pose.joint[(j==0u||j>=4u)?POT_LHIP:POT_RHIP];
    int dx=hip_contour[j].x-anchor.x,dy=hip_contour[j].y-anchor.y;
    assert(dx*dx+dy*dy<=100);
   }
   pelvis++;
  }
  for(unsigned camera=90;camera<=180;camera+=90){
   PotCamera c=pot_camera(camera);
   reset();pot_pelvis(frame.pixels,&c,&pose);guards();
   if(!pelvis_n)for(unsigned j=0;j<HD_BYTES;j++)assert(frame.pixels[j]==HD_WET);
   if(pose.flags&3u){
    PotPoint q[5],h[5],face[4];
    unsigned n=pot_sign_projected(&c,&pose,q,h);
    int readable=pot_sign_face(q,pose.flags,face);
    PotPoint plane[3];
    if(pot_sign_tip_plane(&pose,q,h,n,plane)){
     assert(pose.flags&POT_SIGN_TIP_VISIBLE);
     assert(!(pose.flags&POT_SIGN_CLIPPED));
     for(unsigned k=0;k<3u;k++)assert(pot_sign_near(h,n,plane[k],0));
     tip_planes++;
    }
    PotPoint strip[4];
    if(!readable&&pot_sign_material_plane(&pose,q,h,n,strip)){
     for(unsigned k=0;k<4u;k++)assert(pot_sign_near(h,n,strip[k],0));
     material++;
    }
    reset();pot_sign_draw(frame.pixels,&c,&pose);guards();signs++;
    muted+=(readable==1);
    for(int y=0;y<135;y++)for(int x=0;x<HD_W;x++){
     unsigned color=frame.pixels[y*HD_W+x];
     if(color==HD_WET)continue;
     /* The integer edge rasterizer can round at a contour by a pixel. */
     if(n>=3u)assert(pot_sign_near(h,n,(PotPoint){x,y},2));
     if(!readable)assert(color==HD_PAPER||color==HD_STONE_DARK||
                         color==HD_PLASTER_LIT||color==HD_STONE_LIT);
     if(readable<2)assert(color!=HD_INK&&color!=HD_LEAF_DARK);
    }
   }
   for(unsigned side=0;side<2u;side++){
    unsigned wrist=side?POT_RWRIST:POT_LWRIST;
    if(pose.contact_mask&(1u<<(side+2u))){
     PotPoint contact=pot_actor_point(&c,pose.contact[side+2u]);
     PotPoint start=pot_has(&pose,wrist)?pot_actor_point(&c,pose.joint[wrist]):contact;
     reset();pot_support_palm(frame.pixels,start,contact,pot_actor_size(&c,3),pot_has(&pose,wrist));guards();
     if(inside(contact)){assert(painted_near(contact));palms++;}
    }
    unsigned ankle=side?POT_RANKLE:POT_LANKLE,toe=side?POT_RTOE:POT_LTOE;
    if((pose.contact_mask&(1u<<side))&&pot_has(&pose,ankle)&&pot_has(&pose,toe)&&
       !(pose.boundary&((1u<<ankle)|(1u<<toe)))){
     PotPoint contact=pot_actor_point(&c,pose.contact[side]);
     reset();pot_ground_shoe(frame.pixels,pot_actor_point(&c,pose.joint[ankle]),
        pot_actor_point(&c,pose.joint[toe]),contact,pot_actor_size(&c,3));guards();
     if(inside(contact)){assert(painted_near(contact));soles++;}
    }
   }
   for(unsigned k=0;k<4u;k++){
    unsigned a=bone[k][0],b=bone[k][1],d=bone[k][2];
    if(!pot_has(&pose,a)||!pot_has(&pose,b)||!pot_has(&pose,d)||(pose.boundary&(1u<<b)))continue;
    PotPoint x=pot_actor_point(&c,pose.joint[a]),y=pot_actor_point(&c,pose.joint[b]),z=pot_actor_point(&c,pose.joint[d]);
    int width=pot_actor_size(&c,k<2u?pose.head_size.x+1:pose.head_size.x*2);
    int bend=width>3?width-1:2,tip=width>5?width-3:2;
    reset();
    pot_jointed(frame.pixels,x,y,z,width+1,bend,tip,
                HD_ROAD,HD_ROAD_COOL,HD_STONE_DARK,pot_caps(&pose,a,d));
    guards();limbs++;
    if(inside(y)){assert(painted_near(y));joints++;}
    /* No long miter outside the measured limb plus a bounded fabric width. */
    int lo=x.x,hi=x.x,top=x.y,bot=x.y;
    PotPoint v[2]={y,z};
    for(unsigned i=0;i<2u;i++){
     if(v[i].x<lo)lo=v[i].x;
     if(v[i].x>hi)hi=v[i].x;
     if(v[i].y<top)top=v[i].y;
     if(v[i].y>bot)bot=v[i].y;
    }
    int margin=width+3;
    for(int yy=0;yy<135;yy++)for(int xx=0;xx<HD_W;xx++){
     if(frame.pixels[yy*HD_W+xx]==HD_WET)continue;
     assert(xx>=lo-margin&&xx<=hi+margin&&yy>=top-margin&&yy<=bot+margin);
    }
   }
  }
 }
 assert(signs>400u&&limbs>1000u&&joints>1000u&&palms>100u&&soles>100u&&muted>0u&&torsos>200u);
 assert(material>0u&&pelvis>200u&&tip_planes>0u);
 printf("Silhouette polish: %u bounded pelvic contours, %u observed arrow-extension shade planes; clipped hips and tips remain unpainted\n",pelvis,tip_planes);
 printf("Sign material and lettering: %u clipped/oblique shade planes, compact %u ink samples, full %u ink samples; bevel bounded\n",material,compact_ink,full_ink);
 printf("Body contours: %u normalized torso sections preserve measured centres and consistent side ordering\n",torsos);
 printf("Sign/body raster: %u sign poses, %u muted-print angles, %u connected limbs, %u bends, %u palms, %u soles; crop, contact and guards OK\n",signs,muted,limbs,joints,palms,soles);
 return 0;
}
