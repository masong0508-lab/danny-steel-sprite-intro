#pragma once
#include "hd_city_canvas.h"
/* The same artwork and progress language at both world camera scales.
   Every silhouette fits the old billboard width and is grounded at foot.
   Small distant props use a cheap material silhouette. */
static inline void hd_project_easel(uint8_t *p,int x,int foot,int w,int h,
                                    unsigned phase,int warm){
 if(w<5||h<5)return;
 if(phase>4u)phase=4u;
 int l=x-w/2,top=foot-h,board=h*3/5,edge=w>=12?2:1;
 if(board<3)board=3;
 hd_ellipse(p,x,foot+1,w/2+1,2,HD_SHADOW);
 hd_line(p,x,top+board,x,foot,HD_COPPER);
 hd_line(p,x-1,top+board,l,foot,HD_BARK);
 hd_line(p,x+1,top+board,l+w-1,foot,HD_BARK);
 hd_rect(p,l,top,w,board,HD_BARK);
 hd_rect(p,l+edge,top+edge,w-edge*2,board-edge*2,HD_PAPER);
 if(w>=10&&board>=7)
  hd_city_painting(p,l+edge,top+edge,w-edge*2,board-edge*2,phase,warm,0u);
 else if(phase)
  hd_rect(p,l+edge,top+edge,w-edge*2,board-edge*2,warm?HD_COPPER:HD_GLASS);
 hd_rect(p,l-1,top+board,w+2,1,HD_COPPER);
 if(w>=10){
  /* Four tiny frame-edge inlays stay distinct at miniature resolution. */
  for(unsigned i=0;i<4u;i++)
   hd_pixel(p,l+2+(int)i*2,top+board-1,i<phase?HD_GOLD:HD_STONE_DARK);
 }
}
static inline void hd_project_desk(uint8_t *p,int x,int foot,int w,int h,
                                   unsigned phase,int soft){
 if(w<5||h<5)return;
 if(phase>4u)phase=4u;
 int l=x-w/2,base=foot-h/4,top=base-h/3;
 int inset=w>=12?2:1;
 int speaker=w/7;if(speaker<1)speaker=1;
 int sx=l+w/3,sy=top+h/7;
 int rx=w/5,ry=h/12;if(rx<1)rx=1;if(ry<1)ry=1;
 hd_ellipse(p,x,foot+1,w/2+2,2,HD_SHADOW);
 hd_rect(p,l+inset,base,w-inset*2,foot-base,HD_BARK);
 hd_rect(p,l,top,w,base-top,HD_BARK);
 hd_rect(p,l+inset,top+inset,w-inset*2,base-top-inset*2,HD_BRICK);
 hd_ellipse(p,sx,sy,rx,ry,HD_INK);
 if(w>=10){
  hd_pixel(p,sx,sy,HD_GOLD);
  hd_line(p,l+w-3,top+1,l+w-3,sy+1,HD_PAPER);
  hd_line(p,l+w-3,sy+1,sx+rx,sy,HD_PAPER);
 }
 hd_rect(p,l,top-h/6,speaker,h/6+1,HD_SHADOW);
 hd_rect(p,l+w-speaker,top-h/6,speaker,h/6+1,HD_SHADOW);
 if(w>=10){
  int meter=w-4,cell=meter/4;if(cell<1)cell=1;
  hd_rect(p,l+2,top-h/5,meter,h/7+2,HD_INK);
  for(unsigned i=0;i<4u;i++)
   hd_rect(p,l+2+(int)i*cell,top-h/5+1,cell>1?cell-1:1,h/7,
           i<phase?(soft?HD_COPPER:HD_LAMP):HD_STONE_DARK);
 }
 if(w>=36){
  const char *name="STUDIO";
  int tw=hd_text_width(name,1,0);
  if(tw<=w){int yy=top-h/5-10;
   hd_rect(p,l,yy,w,9,HD_SHADOW);
   hd_text(p,x-tw/2,yy+1,name,HD_LAMP,1,0);
  }
 }
}
