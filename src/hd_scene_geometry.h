#pragma once
#include "story.h"
/* One district identity for both cameras, at the original immutable grid.
   Keep the original isometric building silhouettes and roof classes. */
typedef struct {
 unsigned height,variant;
 int civic,shops,waterfront;
} HdDistrict;
/* Canonical coordinates for callers that already resolved the parcel.
   Keep architectural identity attached to the moved lot. */
static inline HdDistrict hd_district_source(int x,int y,char type){
 HdDistrict d;
 if(story_city_extension(x,y)){
  int east=x>=24&&y<14;
  int civic=type=='A'||(x>=24&&y>=14&&y<20);
  int park=x<15&&y>=18;
  int block=(x/8+y/6)&1;
  d.civic=civic;d.shops=east&&y>=9;d.waterfront=y>=20&&x>=20;
  d.height=civic?46u:east?(block?52u:44u):park?32u:block?42u:36u;
  d.variant=civic?6u:east?(y>=9?4u:block?0u:1u):park?3u:d.waterfront?2u:1u;
  return d;
 }
 d.civic=x>=10&&x<=13&&y>=3&&y<=6;
 d.shops=x>=21&&y>=10&&y<=12;
 d.waterfront=y>=14;
 int alternate=(x/6+y/6)&1;
 d.height=d.civic?36u:d.shops?27u:d.waterfront?27u:alternate?32u:27u;
 d.variant=d.civic?6u:d.shops?4u:d.waterfront?2u:alternate?0u:1u;
 if(!d.civic&&!d.shops&&!d.waterfront&&x<5&&y>=7)d.variant=3u;
 if(!d.civic&&!d.shops&&!d.waterfront&&x>=17&&y<=4)d.variant=5u;
 if(x==21&&y==11)d.variant=7u;
 if(type=='A')d.variant=6u;
 return d;
}
static inline HdDistrict hd_district(int x,int y,char type){
 story_world_source(&x,&y);
 return hd_district_source(x,y,type);
}
/* Signed sub-tile offset in units of a quarter tile. The actual
   Story movement clock remains authoritative. */
typedef struct {int x,y,iso_x,iso_y,depth;} HdSceneCamera;
static inline HdSceneCamera hd_scene_camera(const Story *s){
 int phase=s->anim>4u?4:(int)s->anim;
 int dx=((int)s->prev_x-(int)s->x)*phase;
 int dy=((int)s->prev_y-(int)s->y)*phase;
 return (HdSceneCamera){
  (int)s->x*256+128+dx*64,(int)s->y*256+128+dy*64,
  (dx-dy)*4,(dx+dy)*2,((int)s->x+(int)s->y)*8+(dx+dy)*2
 };
}

/* Texture coordinates belong to the actual crossed wall plane, not the
   dominant camera heading. A coarse ray can cross two grid edges during
   one step: the latter edge is the entrance to the final hit tile.
   The wall depth itself remains the original ray step. */
typedef struct {unsigned face,u;} HdWallSample;
static inline int hd_scene_abs(int n){return n<0?-n:n;}
static inline HdWallSample hd_wall_sample(int wx,int wy,int dx,int dy,
                                         int tx,int ty,int old_tx,int old_ty){
 int x0=wx-dx,y0=wy-dy;
 int crossed_x=tx!=old_tx&&dx!=0,crossed_y=ty!=old_ty&&dy!=0;
 int bx=(tx+(dx<0))*256,by=(ty+(dy<0))*256;
 unsigned face;
 if(crossed_x&&crossed_y){
  face=(unsigned)(hd_scene_abs(bx-x0)*hd_scene_abs(dy)>=
                  hd_scene_abs(by-y0)*hd_scene_abs(dx));
 }else if(crossed_x)face=1u;
 else if(crossed_y)face=0u;
 else face=(unsigned)(hd_scene_abs(dx)>hd_scene_abs(dy));
 int along=face?(dx?y0+dy*(bx-x0)/dx:wy)-ty*256:
                 (dy?x0+dx*(by-y0)/dy:wx)-tx*256;
 if(along<0)along=0;
 if(along>255)along=255;
 if(face?dx<0:dy>0)along=255-along;
 return (HdWallSample){face,(unsigned)along};
}

/* A single radius contract for admission and restoring occluding wall
   columns. Wide buses are the limiting existing billboard silhouette. */
static inline int hd_billboard_radius(int height){
 int r=height*4/11+4;return r<8?8:r;
}
static inline int hd_billboard_visible(int sx,int h,int depth,
                                        const uint16_t *wall_depth,
                                        const uint8_t *wall_type,int width){
 int radius=hd_billboard_radius(h);
 int left=sx-radius,right=sx+radius+1;
 if(left<0)left=0;
 if(right>width)right=width;
 for(int x=left;x<right;x++)
  if(!wall_type[x]||depth<=wall_depth[x])return 1;
 return 0;
}
