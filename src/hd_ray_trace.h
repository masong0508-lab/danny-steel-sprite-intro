#pragma once
#include "story.h"
#include "hd_ray_paths.inc"

/* The reference and compressed paths report the exact original
   hit sample and previously visited tile, including corner crossings.
   No visibility, collision or wall-depth changes. */
typedef struct {
 int wx,wy,tx,ty,from_x,from_y,depth;
 char type;
} HdRayHit;
static inline int hd_ray_wall(char tile){return tile=='#'||tile=='A'||tile=='T';}
static inline int hd_ray_abs(int n){return n<0?-n:n;}
static inline void hd_ray_finish(HdRayHit *h,int cx,int cy,int dx,int dy,
                                unsigned step,int tx,int ty,int from_x,int from_y,char type){
 h->wx=cx+dx*(int)step;h->wy=cy+dy*(int)step;
 h->tx=tx;h->ty=ty;h->from_x=from_x;h->from_y=from_y;
 h->depth=step>=72u?2300:(int)step*32;h->type=type;
}
/* Arbitrary subpixel cameras (not used by ordinary gameplay) retain
   the full original marcher rather than rounding the starting point. */
static __attribute__((noinline)) void hd_ray_reference(int cx,int cy,int dx,int dy,HdRayHit *h){
 int wx=cx,wy=cy,tx=cx>>8,ty=cy>>8;
 for(unsigned step=1u;step<=75u;step++){
  wx+=dx;wy+=dy;
  int nx=wx>>8,ny=wy>>8;
  if(nx==tx&&ny==ty)continue;
  char type=story_tile(nx,ny);
  if(hd_ray_wall(type)){hd_ray_finish(h,cx,cy,dx,dy,step,nx,ny,tx,ty,type);return;}
  tx=nx;ty=ny;
 }
 hd_ray_finish(h,cx,cy,dx,dy,75u,tx,ty,tx,ty,0);
}
/* Reflection is about the integer tile edge (255-local), not 256-local:
   this exactly preserves floor() for negative directions and ensures
   a diagonal crossing advances both axes on the same original sample. */
static __attribute__((noinline)) void hd_ray_trace(int cx,int cy,int dx,int dy,HdRayHit *h){
 int ax=hd_ray_abs(dx),ay=hd_ray_abs(dy),major_x=ax==32;
 int major=major_x?dx:dy,minor=major_x?dy:dx;
 int speed=hd_ray_abs(minor);
 if(((cx|cy)&63)||hd_ray_abs(major)!=32||speed>22||
    (ax!=32&&ay!=32)){
  hd_ray_reference(cx,cy,dx,dy,h);return;
 }
 unsigned a=((unsigned)(major_x?cx:cy)>>6)&3u;
 unsigned b=((unsigned)(major_x?cy:cx)>>6)&3u;
 if(major<0)a+=4u;
 if(minor<0)b+=4u;
 const uint8_t *path=hd_ray_paths+hd_ray_offsets[(a*8u+b)*23u+(unsigned)speed];
 int tx=cx>>8,ty=cy>>8;
 int mx=major_x?(major<0?-1:1):0;
 int my=major_x?0:(major<0?-1:1);
 int nx=major_x?0:(minor<0?-1:1);
 int ny=major_x?(minor<0?-1:1):0;
 unsigned step=0;
 for(unsigned code=*path++;code;code=*path++){
  step+=code&15u;
  int from_x=tx,from_y=ty;
  if(code&16u){tx+=mx;ty+=my;}
  if(code&32u){tx+=nx;ty+=ny;}
  char type=story_tile(tx,ty);
  if(hd_ray_wall(type)){hd_ray_finish(h,cx,cy,dx,dy,step,tx,ty,from_x,from_y,type);return;}
 }
 hd_ray_finish(h,cx,cy,dx,dy,75u,tx,ty,tx,ty,0);
}
