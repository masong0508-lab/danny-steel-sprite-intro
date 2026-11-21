#include "hd_canvas.h"

/* A full 240-column perspective renderer for the actual same 24x18
   collision map. All steps and ray offsets are fixed-point, cardinal facing
   exactly matches story_tick(). There is no second depth framebuffer:
   the 240 x uint16 z strip is a small persistent work array. */
static uint16_t wall_depth[HD_W];
static int abs_i(int a){return a<0?-a:a;}
static unsigned texel(unsigned x,unsigned y){
 uint32_t v=(x+31u)*1664525u+(y+79u)*1013904223u;
 v^=v>>13;v*=2246822519u;return (unsigned)(v^(v>>11));
}
static void dirs(const Story *s,int *fx,int *fy,int *rx,int *ry){
 switch(s->facing&3u){
  case 0:*fx=1;*fy=0;*rx=0;*ry=1;break;
  case 1:*fx=0;*fy=1;*rx=-1;*ry=0;break;
  case 2:*fx=-1;*fy=0;*rx=0;*ry=-1;break;
  default:*fx=0;*fy=-1;*rx=1;*ry=0;break;
 }
}
static void set_sky(uint8_t *p,unsigned tick,int facing,int cx,int cy){
 for(int y=0;y<160;y++){
  unsigned c=y<24?19:y<54?17:y<74?HD_GLASS:y<84?HD_GLASS_LIT:
             y<102?HD_ROAD_COOL:y<125?HD_ROAD_LIGHT:HD_ROAD;
  hd_rect(p,0,y,240,1,c);
 }
 /* Layered, place-bound horizon. Neither buildings nor cloud positions
    randomly change when the shot cuts to a dialogue panel. */
 int pan=(facing*71+(cx+cy)/96)%53;
 for(int i=-2;i<19;i++){
  int x=i*23-pan;
  int ht=17+(int)(texel((unsigned)(i+33),23u)%22u);
  hd_rect(p,x,79-ht,23,ht,HD_STONE_DARK);
  hd_rect(p,x+3,79-ht-2,18,2,HD_ROAD);
  for(int w=3;w<21;w+=7)for(int h=7;h<ht-2;h+=9){
   if((texel((unsigned)(i+41+w),(unsigned)h+35u)&3u)!=0u)
    hd_rect(p,x+w,79-ht+h,2,3,(tick/180u+i+w)%5u==0u?HD_LAMP:HD_GLASS);
  }
 }
 for(int k=0;k<4;k++){
  int x=(k*73+35-(int)(tick/42u)%40)-23;
  int y=17+(k&1)*8;
  hd_line(p,x,y,x+32,y,HD_STONE_DARK);
  hd_line(p,x+6,y-2,x+24,y-2,HD_STONE_DARK);
 }
 /* A flat-toned, quiet horizon instead of procedural neon sky. */
 hd_line(p,0,79,239,79,HD_STONE);
}
static unsigned floor_ink(int wx,int wy,int x,int y,unsigned tick){
 char t=story_tile(wx>>8,wy>>8);
 int fx=wx&255,fy=wy&255;
 if(t=='~'){
  if(((fy+((int)tick*2))&63)<4)return HD_GLASS_LIT;
  return (texel((unsigned)wx>>5,(unsigned)wy>>5)&3u)==0u?HD_REFLECT:HD_GLASS;
 }
 if(t==','||t=='='){
  if(fx<8||fy<7)return HD_MORTAR;
  if(fx>244||fy>244)return HD_STONE_DARK;
  if(((fx>>5)^(fy>>6))&1)return HD_STONE_LIT;
  return HD_STONE;
 }
 if(t=='g'||t=='T'){
  if((texel((unsigned)(wx>>5),(unsigned)(wy>>5))&3u)==0u)return HD_LEAF;
  return HD_LEAF_DARK;
 }
 if(t=='#'||t=='A')return HD_BRICK_DARK;
 unsigned n=texel((unsigned)(wx>>4),(unsigned)(wy>>4));
 if((fx<5||fy<5)&&(wx+wy)%3==0)return HD_ROAD_COOL;
 if((n&31u)==0u)return HD_REFLECT;
 if((n&7u)==1u)return HD_ROAD_LIGHT;
 return (x+y)%17==0?HD_ROAD_COOL:HD_ROAD;
}
static void floor_render(uint8_t *p,const Story *s,int cx,int cy,
                         int fx,int fy,int rx,int ry,unsigned tick){
 (void)s;
 /* One projection calculation per scanline, and half-rate horizontal
    sampling in the far distance only. Near pavement uses each GBA pixel. */
 for(int y=80;y<140;y++){
  int dist=13500/(y-75); if(dist>2304)dist=2304;
  int side_unit=dist*3*256/480;
  int step=y<98?2:1;
  for(int x=0;x<240;x+=step){
   int side=((x-120)*side_unit)>>8;
   int wx=cx+fx*dist+rx*side,wy=cy+fy*dist+ry*side;
   unsigned ink=floor_ink(wx,wy,x,y,tick);
   p[y*HD_W+x]=(uint8_t)ink;
   if(step==2)p[y*HD_W+x+1]=(uint8_t)ink;
  }
 }
}
static void wall_column(uint8_t *p,int x,int top,int bottom,char type,
                        int u,int forward,int tx,int ty){
 int height=bottom-top;
 int tree=type=='T';
 unsigned base=tree?HD_LEAF_DARK:type=='A'?HD_BRICK:HD_BRICK_DARK;
 if(forward>1200)base=tree?HD_LEAF_DARK:HD_STONE_DARK;
 for(int y=top;y<bottom;y++){
  if((unsigned)y>=140u)break;
  int v=(y-top)*256/height;
  unsigned c=base;
  if(tree){
   if(v>178 && u>91 && u<162)c=HD_BARK;
   else if((texel((unsigned)(u>>3),(unsigned)(v>>3))&3u)==0u)
    c=HD_LEAF_LIT;
   else c=HD_LEAF;
  }else{
   /* Facade windows fixed to wall texture coordinates, not screen x.
      Floor breaks, mortar and stone relief persist as camera moves. */
   int window=u>=53&&u<=202&&v>=57&&v<=163;
   if(v<15)c=HD_PLASTER_LIT;
   else if(v<22 || v>238)c=HD_STONE_DARK;
   else if(v%51<4)c=HD_BRICK_DARK;
   else if(u%61<4)c=HD_BRICK_LIT;
   else if(window){
    if(u<60||u>196||v<65||v>154)c=HD_COPPER;
    else if(u>=124&&u<=132)c=HD_BRICK_DARK;
    else if(v>=102&&v<=109)c=HD_BRICK_DARK;
    else c=((tx+ty)&1)?HD_GLASS_LIT:HD_GLASS;
   }else if((texel((unsigned)(u>>4)+(unsigned)(tx+40),
                         (unsigned)(v>>4)+(unsigned)(ty+40))&7u)==0u)
    c=HD_BRICK_LIT;
  }
  p[y*HD_W+x]=(uint8_t)c;
 }
 if(top>18 && top<140)hd_pixel(p,x,top,tree?HD_LEAF_LIT:HD_STONE_LIT);
}
static void cast_walls(uint8_t *p,int cx,int cy,int fx,int fy,int rx,int ry){
 for(int x=0;x<HD_W;x++){
  int side=((x-120)*3)/16; /* 0.75 FOV at each edge, fixed-point */
  int dx=fx*32+rx*side,dy=fy*32+ry*side;
  int wx=cx,wy=cy,hit_dist=2400,tx=0,ty=0;
  char hit=0;
  for(int step=1;step<76;step++){
   wx+=dx;wy+=dy;
   tx=wx>>8;ty=wy>>8;
   char t=story_tile(tx,ty);
   if(t=='#'||t=='A'||t=='T'){hit=t;hit_dist=step*32;break;}
  }
  if(hit_dist>2300)hit_dist=2300;
  wall_depth[x]=(uint16_t)hit_dist;
  if(!hit)continue;
  int height=20800/(hit_dist+60);
  if(height>150)height=150;
  if(height<3)height=3;
  int top=79-height/2,bottom=79+height/2;
  int u=abs_i(dx)>abs_i(dy)?wy&255:wx&255;
  wall_column(p,x,top,bottom,hit,u,hit_dist,tx,ty);
 }
}
static void actor(uint8_t *p,int sx,int ground,int h,char kind,unsigned tick){
 if(h<8)return;
 int w=h*4/11;if(w<5)w=5;
 unsigned coat=kind=='F'?HD_GLASS:kind=='C'?HD_GLASS_LIT:
   kind=='P'?HD_COPPER:kind=='V'?HD_BRICK:kind=='Q'?HD_RED_SHADE:
   kind=='W'?HD_LEAF:kind=='B'?HD_RED:kind=='M'?HD_BRICK_LIT:
   kind=='R'?HD_COPPER:kind=='K'?HD_PLASTER:HD_STONE;
 hd_ellipse(p,sx,ground+1,w/2+2,2,HD_SHADOW);
 if(kind=='B'){
  hd_rect(p,sx-w,ground-h/2,w*2,h/3,HD_RED_SHADE);
  hd_rect(p,sx-w+2,ground-h/2+2,w*2-4,h/4,HD_RED);
  hd_rect(p,sx-w+3,ground-h/2+3,w-3,h/6,HD_GLASS);
  hd_rect(p,sx+2,ground-h/2+3,w-4,h/6,HD_GLASS_LIT);
  hd_ellipse(p,sx-w/2,ground-h/7,3,3,HD_INK);
  hd_ellipse(p,sx+w/2,ground-h/7,3,3,HD_INK);
 }else if(kind=='L'||kind=='K'||kind=='R'||kind=='M'||kind=='S'||kind=='H'){
  /* Physical shopfront/letter/terminal silhouette, with legible signage
     on approach instead of a floating seven-pixel quest marker. */
  hd_rect(p,sx-w/2,ground-h,w,h,HD_INK);
  hd_rect(p,sx-w/2+2,ground-h+2,w-4,h-4,coat);
  hd_rect(p,sx-w/2+3,ground-h+4,w-6,3,HD_PAPER);
  if(h>=45){
   const char *title=kind=='K'?"CLINIC":kind=='R'?"ART":
                     kind=='M'?"MUSIC":kind=='S'?"DATA":
                     kind=='H'?"HOME":"LETTER";
   hd_text(p,sx-w/2+3,ground-h+9,title,HD_INK,1,0);
  }
 }else{
  int head=h/4;if(head<4)head=4;
  hd_rect(p,sx-w/4,ground-h+1,head,head+1,HD_SKIN);
  hd_rect(p,sx-w/4,ground-h+1,head,head/3+1,HD_HAIR);
  hd_rect(p,sx-w/2,ground-h+head,w,h/2,coat);
  hd_rect(p,sx-w/2+1,ground-h+head+2,2,h/3,HD_STONE_DARK);
  hd_rect(p,sx+w/2-3,ground-h+head+2,2,h/3,HD_STONE_DARK);
  hd_rect(p,sx-w/4,ground-h/4,3,h/4,HD_INK);
  hd_rect(p,sx+w/8,ground-h/4,3,h/4,HD_INK);
  if(h>25){
   int bob=(int)(tick/27u&1u);
   hd_pixel(p,sx-2,ground-h+head/2+bob,HD_INK);
   hd_pixel(p,sx+2,ground-h+head/2+bob,HD_INK);
   hd_line(p,sx,ground-h+head+1,sx,ground-h/2,HD_COPPER);
  }
  if(kind=='F'&&h>30){
   hd_rect(p,sx-w/4-2,ground-h+1,2,head+8,HD_HAIR);
   hd_rect(p,sx+w/4+2,ground-h+1,2,head+9,HD_HAIR);
  }
 }
}
static void render_billboards(uint8_t *p,int cx,int cy,
                              int fx,int fy,int rx,int ry,unsigned tick){
 for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++){
  char t=story_tile(x,y);
  if(t!='F'&&t!='C'&&t!='B'&&t!='L'&&t!='P'&&t!='V'&&
     t!='Q'&&t!='S'&&t!='W'&&t!='H'&&t!='R'&&t!='M'&&t!='K')
   continue;
  int dx=x*256+128-cx,dy=y*256+128-cy;
  int forward=dx*fx+dy*fy,side=dx*rx+dy*ry;
  if(forward<85||forward>2200||abs_i(side)>forward*4/3)continue;
  int sx=120+side*160/forward;
  int height=24000/(forward+70);
  if(height>115)height=115;
  if(height<8)height=8;
  if(sx<0||sx>=240||forward>wall_depth[sx])continue;
  actor(p,sx,80+height/2,height,t,tick);
 }
}
void hd_first_person_draw(uint8_t *p,const Story *s,unsigned tick){
 int fx,fy,rx,ry;
 dirs(s,&fx,&fy,&rx,&ry);
 int camx=(int)s->x*256+128,camy=(int)s->y*256+128;
 if(s->anim){
  camx+=(s->prev_x-(int)s->x)*s->anim*64;
  camy+=(s->prev_y-(int)s->y)*s->anim*64;
 }
 set_sky(p,tick,s->facing,camx,camy);
 floor_render(p,s,camx,camy,fx,fy,rx,ry,tick);
 cast_walls(p,camx,camy,fx,fy,rx,ry);
 render_billboards(p,camx,camy,fx,fy,rx,ry,tick);
}
