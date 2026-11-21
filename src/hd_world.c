#include "hd_canvas.h"
#include "hd_world_atlas.h"

/* The exact 24x18 Old Quarter grid is still authoritative. These tiles are
   painted at native resolution, with different paving, roof/cornice, brick
   and foliage detail, while gameplay/collisions remain unchanged. */
static unsigned noise(unsigned x,unsigned y){
 unsigned n=(x+3u)*374761393u+(y+1u)*668265263u;
 n^=n>>13;n*=1274126177u;return n^(n>>16);
}
static void ground(uint8_t *p,int sx,int sy,int wx,int wy,unsigned frame){
 /* ROM tile art replaces 16x16 per-frame random/grout/brick drawing.
    The exact original story_tile grid still chooses collisions/locations. */
 hd_world_tile_art(p,sx,sy,story_tile(wx,wy),wx,wy);
 (void)frame;
}
static void tree(uint8_t *p,int x,int foot,unsigned seed,unsigned tick){
 hd_world_tree_art(p,x,foot,seed%3u,(tick/47u)&1u);
}
static void streetlamp(uint8_t *p,int x,int y,unsigned frame){
 /* A practical street fixture in the actual map, with restrained glow. */
 hd_ellipse(p,x,y+2,9,3,HD_SHADOW);
 hd_rect(p,x-1,y-34,3,35,HD_INK);
 hd_rect(p,x-3,y-35,7,2,HD_COPPER);
 hd_rect(p,x-5,y-33,11,4,HD_BARK);
 hd_rect(p,x-4,y-32,9,3,HD_LAMP);
 hd_line(p,x-5,y-29,x-10,y-23,HD_ROAD_LIGHT);
 hd_line(p,x+6,y-29,x+10,y-23,HD_ROAD_LIGHT);
 if((frame/43u)&1u)hd_pixel(p,x,y-31,HD_PAPER);
 hd_line(p,x-3,y,x+5,y,HD_COPPER);
}
static void prop(uint8_t *p,int x,int y,char c,unsigned tick){
 if(c=='L'){ /* a physical, folded letter beside the porch */
  hd_ellipse(p,x+1,y+4,7,3,HD_SHADOW);
  hd_rect(p,x-7,y-10,14,10,HD_PAPER);
  hd_line(p,x-6,y-9,x,y-4,HD_STONE);
  hd_line(p,x+6,y-9,x,y-4,HD_STONE);
  hd_rect(p,x-4,y-13,9,2,HD_COPPER);
 }else if(c=='R'){ /* rooftop easel and the painting, not a floating icon */
  hd_rect(p,x-6,y-15,12,15,HD_BARK);
  hd_rect(p,x-5,y-14,10,12,HD_GLASS);
  hd_rect(p,x-4,y-6,8,4,HD_GOLD);
  hd_rect(p,x-7,y-1,14,2,HD_COPPER);
  hd_line(p,x-3,y,x-5,y+6,HD_BARK);
  hd_line(p,x+3,y,x+5,y+6,HD_BARK);
 }else if(c=='M'){ /* record shop: actual vinyl behind glass */
  hd_rect(p,x-9,y-16,18,17,HD_BRICK);
  hd_rect(p,x-8,y-15,16,13,HD_GLASS_LIT);
  hd_ellipse(p,x,y-9,6,6,HD_SHADOW);
  hd_ellipse(p,x,y-9,2,2,HD_PAPER);
  hd_rect(p,x-9,y,18,3,HD_COPPER);
 }else if(c=='K'){
  hd_rect(p,x-7,y-13,14,15,HD_PLASTER);
  hd_rect(p,x-6,y-11,12,10,HD_GLASS);
  hd_rect(p,x-1,y-10,2,8,HD_PAPER);
  hd_rect(p,x-4,y-7,8,2,HD_PAPER);
 }else if(c=='B'){
  int n=(int)(tick/8u%3u);
  hd_rect(p,x-13,y-15+n,27,15,HD_RED_SHADE);
  hd_rect(p,x-12,y-15+n,25,11,HD_RED);
  for(int j=0;j<3;j++)hd_rect(p,x-9+j*8,y-13+n,6,6,HD_GLASS);
  hd_rect(p,x-12,y-1+n,25,3,HD_BRICK_DARK);
  hd_ellipse(p,x-7,y+3+n,3,3,HD_SHADOW);
  hd_ellipse(p,x+9,y+3+n,3,3,HD_SHADOW);
 }else if(c=='S'||c=='H'){
  hd_rect(p,x-8,y-11,17,13,HD_SHADOW);
  hd_rect(p,x-7,y-10,15,11,c=='S'?HD_GLASS:HD_PLASTER);
  hd_rect(p,x-5,y-8,11,7,c=='S'?HD_REFLECT:HD_BRICK);
  hd_rect(p,x-3,y-6,7,2,HD_LAMP);
 }else if(c=='D'){
  hd_ellipse(p,x,y+2,5,2,HD_SHADOW);
  hd_rect(p,x-3,y-9,6,8,HD_PLASTER);
  hd_rect(p,x-2,y-8,4,5,HD_GLASS_LIT);
  }else if(c=='F'||c=='C'||c=='P'||c=='V'||c=='Q'||c=='W'){
  /* An identically anchored 22x34 ROM actor replaces a few rectangles.
     Full boots, hair and sleeves share one stable fixed perspective. */
  unsigned role=c=='F'?1u:c=='C'?2u:c=='P'?3u:
                c=='V'?4u:c=='Q'?5u:6u;
  hd_world_person_art(p,x,y,role,1u,0u);
 }
}
static void player(uint8_t *p,int x,int foot,unsigned facing,unsigned gait){
 hd_world_person_art(p,x,foot,0u,facing,gait&1u);
}
static void heading(uint8_t *p,const Story *s){
 int gx=(int)s->x,gy=(int)s->y;const char *goal="";
 int has=story_goal(s,&gx,&gy,&goal);
 int dx=gx-(int)s->x,dy=gy-(int)s->y;
 const char *direction=dx==0&&dy==0?"HERE":
   (dx<0?-dx:dx)>(dy<0?-dy:dy)?(dx>0?"EAST":"WEST"):(dy>0?"SOUTH":"NORTH");
 hd_rect(p,0,0,240,18,HD_INK);
 hd_rect(p,0,17,240,1,HD_COPPER);
 hd_text(p,8,5,"OLD QUARTER",HD_PAPER,1,1);
 hd_text(p,107,5,has?"NEXT":"FREE TIME",HD_LAMP,1,1);
 if(has){hd_text(p,140,5,goal,HD_PAPER,1,0);}
 hd_text(p,200,5,direction,HD_STONE_LIT,1,0);
 hd_rect(p,0,140,240,20,HD_INK);
 hd_rect(p,0,140,240,1,HD_COPPER);
 hd_text(p,8,145,s->view?"FIRST PERSON":"WALK THE QUARTER",HD_LAMP,1,1);
 hd_text(p,8,153,"A TALK   B CHOICE   L VIEW   R JOURNAL",HD_PAPER,1,0);
}
void hd_world_draw(uint8_t *p,const Story *s,unsigned frame){
 int bx=(int)s->x-7,by=(int)s->y-4;
 /* Draw one extra tile on each edge to avoid camera seams. */
 for(int y=-1;y<=10;y++)for(int x=-1;x<=15;x++)
  ground(p,x*16,y*16-1,bx+x,by+y,frame);
 int dx=0,dy=0;
 if(s->anim){
  dx=(int)(s->prev_x-s->x)*s->anim*4;
  dy=(int)(s->prev_y-s->y)*s->anim*4;
 }
 int player_foot=80+dy;
 int player_drawn=0;
 /* Painter's Y-depth: world characters no longer float on TOP of trees,
    benches and shopfronts standing in front of their feet. */
 for(int y=-1;y<=10;y++){
  for(int x=-1;x<=15;x++){
   int wx=bx+x,wy=by+y,sx=x*16,sy=y*16-1;
   char c=story_tile(wx,wy);
   if(c=='T')tree(p,sx+8,sy+21,
                  noise((unsigned)(wx+100),(unsigned)(wy+100)),frame);
   else if(c=='L'||c=='C'||c=='F'||c=='B'||c=='P'||c=='V'||
           c=='Q'||c=='S'||c=='W'||c=='H'||c=='R'||c=='M'||
           c=='K'||c=='D')prop(p,sx+8,sy+17,c,frame);
   if(wx==17 && wy==13)streetlamp(p,sx+8,sy+15,frame);
  }
  int row_foot=y*16+16;
  if(!player_drawn && player_foot<=row_foot){
   player(p,120+dx,player_foot,s->facing,s->anim?(frame/7u)&1u:0u);
   player_drawn=1;
  }
 }
 if(!player_drawn)
  player(p,120+dx,player_foot,s->facing,s->anim?(frame/7u)&1u:0u);
 int gx,gy;const char *goal;
 if(story_goal(s,&gx,&gy,&goal)){
  int x=(gx-bx)*16, y=(gy-by)*16-1;
  if(x>10&&x<229&&y>18&&y<125){
   int yy=y-3+(int)(frame/20u&1u);
   hd_pixel(p,x+8,yy,HD_LAMP);
   hd_line(p,x+4,yy+2,x+8,yy+5,HD_LAMP);
   hd_line(p,x+12,yy+2,x+8,yy+5,HD_LAMP);
  }
 }
 heading(p,s);
}
