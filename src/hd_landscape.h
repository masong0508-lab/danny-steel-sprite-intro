#pragma once
#include "hd_canvas.h"
#include "hd_scene_geometry.h"
/* Shared color language and landmarks for the outdoor heightfield. */
static inline unsigned hd_land_ink(char tile,int h,unsigned shade){
 if(tile=='~')return shade?HD_GLASS:HD_GLASS_LIT;
 if(tile=='=')return shade?HD_MORTAR:HD_STONE;
 if(h>55)return shade?HD_STONE:HD_STONE_LIT;
 if(h>38)return shade?HD_LEAF_DARK:HD_STONE;
 return shade?HD_LEAF_DARK:HD_LEAF;
}
static inline unsigned hd_land_marker(int x,int y){
 return x==44&&y==8?1u:x==54&&y==24?2u:x==56&&y==11?3u:x==43&&y==22?4u:0u;
}
static inline void hd_landscape_sky(uint8_t *p,const Story *s){
 hd_rect(p,0,0,HD_W,36,HD_ROAD_COOL);
 hd_rect(p,0,36,HD_W,28,HD_GLASS);
 hd_rect(p,0,64,HD_W,96,HD_LEAF_DARK);
 /* Continuous, nonrectangular distant silhouette, behind actual mesh. */
 for(int x=0;x<HD_W;x+=3){
  int k=(x/3+(int)s->x*2+(int)s->facing*17)%80;
  int a=k<40?k:80-k,b=(k+19)%39;
  if(b>19)b=38-b;
  int far=65-a/2-b/3,near=75-(40-a)/3-b/2;
  hd_rect(p,x,far,3,76-far,HD_STONE_DARK);
  hd_rect(p,x,near,3,82-near,HD_LEAF_DARK);
 }
}
static inline void hd_land_status(uint8_t *p,const Story *s){
 unsigned action=story_land_action(s);
 if(action==STORY_LAND_PATCH0||action==STORY_LAND_PATCH1){
  char label[]="B LOWER GRADE 0/3";
  label[14]=(char)('0'+story_land_level(s,action-1u));
  hd_text(p,16,152,label,HD_LAMP,1,0);
 }else if(story_rural(s->x,s->y)){
  unsigned h=(unsigned)story_terrain_height(s,s->x*256+128,s->y*256+128);
  char label[]="ELEVATION 00";
  label[10]=(char)('0'+h/10u);label[11]=(char)('0'+h%10u);
  hd_text(p,16,152,label,HD_STONE_LIT,1,0);
 }
}
/* Integer edge stepping, one span per row, no new buffers. */
typedef struct {int x,y;} HdLandPoint;
static inline void hd_land_swap(HdLandPoint *a,HdLandPoint *b){HdLandPoint t=*a;*a=*b;*b=t;}
static inline void hd_land_half(uint8_t *p,HdLandPoint a,HdLandPoint b,
                                HdLandPoint c,HdLandPoint d,unsigned ink){
 int first=a.y>c.y?a.y:c.y,last=b.y<d.y?b.y:d.y;
 if(first<0)first=0;
 if(last>HD_H)last=HD_H;
 if(first>=last||a.y==b.y||c.y==d.y)return;
 int da=(b.x-a.x)*256/(b.y-a.y),db=(d.x-c.x)*256/(d.y-c.y);
 int ax=a.x*256+(first-a.y)*da,bx=c.x*256+(first-c.y)*db;
 for(int y=first;y<last;y++,ax+=da,bx+=db){
  int l=(ax<bx?ax:bx)>>8,r=(ax>bx?ax:bx)>>8;
  hd_rect(p,l,y,r-l+1,1,ink);
 }
}
static inline void hd_land_tri(uint8_t *p,HdLandPoint a,HdLandPoint b,HdLandPoint c,unsigned ink){
 if(a.y>b.y)hd_land_swap(&a,&b);
 if(b.y>c.y)hd_land_swap(&b,&c);
 if(a.y>b.y)hd_land_swap(&a,&b);
 hd_land_half(p,a,b,a,c,ink);
 hd_land_half(p,b,c,a,c,ink);
}
