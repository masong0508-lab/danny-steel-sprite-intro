#pragma once
#include "story.h"
/* One district identity for both cameras, at the original immutable grid.
   Keep the original isometric building silhouettes and roof classes. */
typedef struct {
 unsigned height,variant;
 int civic,shops,waterfront;
} HdDistrict;
static inline HdDistrict hd_district(int x,int y,char type){
 HdDistrict d;
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
