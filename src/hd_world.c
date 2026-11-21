#include "hd_canvas.h"

/* The exact 24x18 Old Quarter grid is still authoritative. These tiles are
   painted at native resolution, with different paving, roof/cornice, brick
   and foliage detail, while gameplay/collisions remain unchanged. */
static unsigned noise(unsigned x,unsigned y){
 unsigned n=(x+3u)*374761393u+(y+1u)*668265263u;
 n^=n>>13;n*=1274126177u;return n^(n>>16);
}
static void ground(uint8_t *p,int xx,int yy,int wx,int wy,unsigned t){
 char c=story_tile(wx,wy);
 unsigned seed=noise((unsigned)(wx+103),(unsigned)(wy+101));
 if(c=='#'||c=='A'){
  hd_rect(p,xx+1,yy+3,16,16,HD_SHADOW);
  hd_rect(p,xx,yy,16,16,(wx+wy)&1?HD_BRICK:HD_BRICK_DARK);
  hd_rect(p,xx,yy,16,2,HD_BRICK_LIT);
  hd_rect(p,xx,yy+2,16,1,HD_BRICK_DARK);
  for(int y=5;y<15;y+=6){
   hd_line(p,xx,y+yy,xx+15,y+yy,HD_BRICK_DARK);
   int start=(wy+y)&2?2:9;
   for(int x=start;x<16;x+=12)hd_line(p,xx+x,yy+y,xx+x,yy+y+4,HD_BRICK_LIT);
  }
  if(((wx+wy)&1)==0){
   hd_rect(p,xx+4,yy+4,8,9,HD_SHADOW);
   hd_rect(p,xx+5,yy+5,6,7,c=='A'?HD_GLASS_LIT:HD_GLASS);
   hd_line(p,xx+8,yy+5,xx+8,yy+11,HD_COPPER);
   hd_rect(p,xx+3,yy+12,10,2,HD_STONE_DARK);
   if(c=='A')hd_rect(p,xx+5,yy+6,3,3,HD_LAMP);
  }
  if(story_tile(wx,wy+1)!='#'&&story_tile(wx,wy+1)!='A'){
   hd_rect(p,xx,yy+13,16,2,HD_PLASTER);
   hd_line(p,xx,yy+15,xx+15,yy+15,HD_SHADOW);
  }
  return;
 }
 if(c=='~'){
  hd_rect(p,xx,yy,16,16,HD_REFLECT);
  hd_rect(p,xx,yy+9,16,7,HD_GLASS);
  if((seed&3u)==0)hd_rect(p,xx+2,yy+4,13,1,HD_GLASS_LIT);
  hd_rect(p,xx+((int)((t/7u+seed)&7u)),yy+11,7,1,HD_GLASS_LIT);
  if(story_tile(wx-1,wy)!='~')hd_rect(p,xx,yy,2,16,HD_STONE);
  if(story_tile(wx,wy-1)!='~')hd_rect(p,xx,yy,16,2,HD_STONE_LIT);
  return;
 }
 if(c=='T'||c=='g'){
  hd_rect(p,xx,yy,16,16,HD_LEAF_DARK);
  for(int i=0;i<7;i++){
   int x=(int)(noise(seed+i*9u,1)&15u),y=(int)(noise(seed+i*7u,3)&15u);
   hd_rect(p,xx+x,yy+y,3,2,(i&1)?HD_LEAF:HD_LEAF_LIT);
  }
  if(c=='g')return;
  hd_ellipse(p,xx+9,yy+11,7,3,HD_SHADOW);
  return;
 }
 if(c==','||c=='='){
  hd_rect(p,xx,yy,16,16,HD_STONE);
  hd_rect(p,xx,yy,16,1,HD_STONE_DARK);
  hd_rect(p,xx,yy,1,16,HD_STONE_DARK);
  for(int y=5;y<16;y+=6){
   hd_line(p,xx+1,yy+y,xx+15,yy+y,HD_MORTAR);
   int split=y==5?7:12;
   hd_line(p,xx+split,yy+y-5,xx+split,yy+y-1,HD_MORTAR);
  }
  for(int n=0;n<4;n++){
   int x=(int)(noise(seed+(unsigned)n,55)&13u);
   int y=(int)(noise(seed+19u+(unsigned)n,66)&13u);
   hd_pixel(p,xx+x,yy+y,(n&1)?HD_PLASTER:HD_STONE_LIT);
  }
  if((wx+wy)%9==0)hd_line(p,xx+3,yy+11,xx+8,yy+13,HD_STONE_DARK);
  return;
 }
 /* Cobblestones, drains, wet chips of asphalt. Not a uniform 8x8 grid. */
 hd_rect(p,xx,yy,16,16,(seed&3u)==0?HD_ROAD_LIGHT:HD_ROAD);
 for(int i=0;i<9;i++){
  unsigned v=noise(seed+(unsigned)i*111u,193u);
  int x=(int)(v&15u),y=(int)((v>>7)&15u);
  hd_rect(p,xx+x,yy+y,2+(int)(v&1u),1,
          (v&4u)?HD_ROAD_COOL:HD_WET);
 }
 if((seed&7u)==0u){
  hd_line(p,xx+3,yy+3,xx+10,yy+7,HD_ROAD_COOL);
  hd_line(p,xx+10,yy+7,xx+12,yy+11,HD_ROAD_COOL);
 }
 if((wx+wy*3)%11==0){
  hd_rect(p,xx+3,yy+6,8,1,HD_STONE_DARK);
  hd_rect(p,xx+3,yy+7,8,1,HD_REFLECT);
 }
}
static void tree(uint8_t *p,int x,int y,unsigned seed,unsigned tick){
 hd_ellipse(p,x+3,y+17,11,4,HD_SHADOW);
 hd_rect(p,x-1,y+5,5,15,HD_BARK);
 hd_rect(p,x,y+10,1,9,HD_COPPER);
 hd_ellipse(p,x-6,y,9,7,HD_LEAF_DARK);
 hd_ellipse(p,x+4,y-5,10,9,HD_LEAF);
 hd_ellipse(p,x+10,y+2,8,7,HD_LEAF);
 hd_ellipse(p,x-1,y-9,8,7,HD_LEAF_LIT);
 for(int i=0;i<13;i++){
  unsigned n=noise(seed+(unsigned)i,seed);
  int dx=(int)(n%20u)-9,dy=(int)((n>>8)%16u)-11;
  hd_pixel(p,x+dx,y+dy,(i&1)?HD_LEAF_LIT:HD_LEAF);
 }
 if((tick/29u+seed)%9u==0)hd_pixel(p,x+5,y-6,HD_STONE_LIT);
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
  (void)tick;
  unsigned coat=c=='F'?HD_GLASS_LIT:c=='C'?HD_GLASS:c=='P'?HD_COPPER:
                c=='V'?HD_BRICK:c=='Q'?HD_BRICK_DARK:HD_LEAF;
  int bob=(int)(tick/17u&1u);
  hd_ellipse(p,x,y+3,8,3,HD_SHADOW);
  hd_rect(p,x-4,y-6+bob,4,9,HD_INK);hd_rect(p,x+1,y-6+bob,4,9,HD_INK);
  hd_rect(p,x-7,y-16+bob,14,13,HD_SHADOW);
  hd_rect(p,x-6,y-15+bob,12,11,coat);
  hd_rect(p,x-4,y-25+bob,9,9,HD_SKIN);
  hd_rect(p,x-4,y-26+bob,10,4,c=='F'?HD_HAIR:HD_SHADOW);
  if(c=='F'){
   hd_rect(p,x-5,y-23+bob,2,10,HD_HAIR);
   hd_rect(p,x+5,y-23+bob,2,9,HD_HAIR);
   hd_rect(p,x-2,y-15+bob,4,3,HD_SKIN_LIT);
  }
  hd_pixel(p,x+2,y-21+bob,HD_INK);
  hd_rect(p,x-8,y-13+bob,2,8,coat);
  hd_rect(p,x+6,y-13+bob,2,8,coat);
  if(c=='C')hd_rect(p,x-1,y-13+bob,3,4,HD_PAPER);
 }
}
static void player(uint8_t *p,int x,int y,unsigned facing,unsigned tick){
 int bob=(int)((tick/10u)&1u);
 hd_ellipse(p,x,y+1,9,3,HD_SHADOW);
 hd_rect(p,x-5,y-7+bob,4,8,HD_INK);
 hd_rect(p,x+2,y-7+bob,4,8,HD_INK);
 hd_rect(p,x-7,y-19+bob,14,13,HD_RED_SHADE);
 hd_rect(p,x-6,y-18+bob,12,11,HD_RED);
 hd_line(p,x,y-17+bob,x,y-9+bob,HD_BRICK_LIT);
 hd_rect(p,x-9,y-15+bob,3,9,HD_RED);
 hd_rect(p,x+7,y-15+bob,3,9,HD_RED_SHADE);
 hd_rect(p,x-4,y-28+bob,9,10,HD_SKIN);
 hd_rect(p,x-5,y-30+bob,10,5,HD_HAIR);
 hd_rect(p,x-5,y-25+bob,2,5,HD_HAIR);
 hd_pixel(p,x-3,y-27+bob,HD_BRICK_LIT);
 hd_rect(p,x-3,y-19+bob,6,2,HD_HAIR);
 if(facing==1){hd_pixel(p,x+3,y-23+bob,HD_INK);}
 else if(facing==3){hd_pixel(p,x-2,y-23+bob,HD_INK);}
 else if(facing==2){
  hd_pixel(p,x-2,y-23+bob,HD_INK);hd_pixel(p,x+3,y-23+bob,HD_INK);
  hd_rect(p,x-1,y-20+bob,3,1,HD_BRICK);
 }else hd_rect(p,x-2,y-27+bob,7,2,HD_HAIR);
 hd_pixel(p,x+6,y-14+bob,HD_COPPER);
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
 for(int y=-1;y<=10;y++)for(int x=-1;x<=15;x++){
  int wx=bx+x,wy=by+y,sx=x*16,sy=y*16-1;
  char c=story_tile(wx,wy);
  if(c=='T')tree(p,sx+8,sy+8,noise((unsigned)(wx+100),(unsigned)(wy+100)),frame);
  else if(c=='L'||c=='C'||c=='F'||c=='B'||c=='P'||c=='V'||
          c=='Q'||c=='S'||c=='W'||c=='H'||c=='R'||c=='M'||
          c=='K'||c=='D')prop(p,sx+8,sy+17,c,frame);
  if(wx==17 && wy==13)streetlamp(p,sx+8,sy+15,frame);
 }
 int dx=0,dy=0;
 if(s->anim){dx=(int)(s->prev_x-s->x)*s->anim*4;dy=(int)(s->prev_y-s->y)*s->anim*4;}
 player(p,120+dx,80+dy,s->facing,frame);
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
