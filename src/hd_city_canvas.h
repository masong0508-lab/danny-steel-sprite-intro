#pragma once
#include "hd_canvas.h"

/* One authored 64x40 skyline, used at every scale. The eight painting
   cells are logical CLIP regions, not separate pictures. A saved phase
   and a transient mark mask are all the data required. */
typedef struct {int x,y,w,h,left,top,right,bottom;} HdCityCell;
static inline void hd_city_rect(uint8_t *p,const HdCityCell *c,int x,int y,int w,int h,unsigned ink){
 int x1=x+w,y1=y+h;
 if(x<c->left)x=c->left;
 if(y<c->top)y=c->top;
 if(x1>c->right)x1=c->right;
 if(y1>c->bottom)y1=c->bottom;
 if(x1<=x||y1<=y)return;
 int a=c->x+x*c->w/64,b=c->y+y*c->h/40;
 int aa=c->x+x1*c->w/64,bb=c->y+y1*c->h/40;
 if(aa>a&&bb>b)hd_rect(p,a,b,aa-a,bb-b,ink);
}
static inline unsigned hd_city_layer(unsigned phase,unsigned marks,unsigned bit,unsigned layer,unsigned target){
 return (target&bit)&&(phase>layer||(phase==layer&&(marks&bit)));
}
static inline void hd_city_painting(uint8_t *p,int x,int y,int w,int h,unsigned phase,int warm,unsigned marks){
 if(w<=0||h<=0)return;
 if(phase>4u)phase=4u;
 hd_rect(p,x,y,w,h,HD_PLASTER);
 unsigned sky=warm?HD_COPPER:HD_GLASS,light=warm?HD_GOLD:HD_GLASS_LIT;
 unsigned ground=warm?HD_BRICK_LIT:HD_REFLECT;
 static const uint8_t roofs[8]={14,10,16,12,17,11,15,9};
 for(unsigned i=0;i<8u;i++){
  unsigned bit=1u<<i;
  int left=(int)(i&3u)*16,top=(int)(i>>2)*20;
  HdCityCell c={x,y,w,h,left,top,left+16,top+20};
  if(hd_city_layer(phase,marks,bit,0u,0x0fu)){
   hd_city_rect(p,&c,0,0,64,20,sky);
   hd_city_rect(p,&c,0,0,64,5,light);
   hd_city_rect(p,&c,0,15,64,5,ground);
  }
  if(hd_city_layer(phase,marks,bit,1u,0xf0u)){
   hd_city_rect(p,&c,0,20,64,20,ground);
   for(int b=0;b<8;b++){
    int height=roofs[b],bx=b*8;
    hd_city_rect(p,&c,bx,37-height,7,height,HD_STONE_DARK);
    hd_city_rect(p,&c,bx,37-height,6,1,warm?HD_COPPER:HD_STONE_LIT);
   }
   hd_city_rect(p,&c,0,37,64,3,HD_LEAF_DARK);
  }
  if(hd_city_layer(phase,marks,bit,2u,0x66u)){
   if(i<4u){
    hd_city_rect(p,&c,left+3,8,4,1,HD_PAPER);
    hd_city_rect(p,&c,left+10,12,2,2,HD_LAMP);
   }else{
    for(int b=2;b<6;b++){
     int bx=b*8,by=39-roofs[b];
     hd_city_rect(p,&c,bx+2,by,2,2,HD_LAMP);
     hd_city_rect(p,&c,bx+4,by+4,2,2,HD_GOLD);
    }
    hd_city_rect(p,&c,left+3,38,10,1,HD_GOLD);
   }
  }
  if(hd_city_layer(phase,marks,bit,3u,0x99u)){
   if(i<4u){
    int mx=i==0u?6:54;
    hd_city_rect(p,&c,mx-3,8,7,5,HD_LAMP);
    hd_city_rect(p,&c,mx-2,7,5,7,HD_LAMP);
    hd_city_rect(p,&c,left,19,16,1,HD_GOLD);
   }else{
    for(int b=0;b<8;b++){
     int bx=b*8,by=39-roofs[b];
     hd_city_rect(p,&c,bx+2,by,2,2,HD_LAMP);
     hd_city_rect(p,&c,bx+4,by+4,2,2,HD_GOLD);
    }
    hd_city_rect(p,&c,left+2,38,12,1,HD_GOLD);
   }
  }
 }
}
