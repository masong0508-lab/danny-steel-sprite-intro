#pragma once
#include "hd_canvas.h"

/* One measured street set throughout the descent. Depth increases toward
   the camera; the shop always occupies x38..210, depth-75..0, height78.
   The camera changes projection, never swaps or wipes between two sets. */
typedef struct {int zoom,ground,rise,skew,ground_y;} PotCamera;
typedef struct {int x,y;} PotPoint;

static int pot_set_round(int value,int scale){
 return (value+(value<0?-scale/2:scale/2))/scale;
}
static PotCamera pot_camera(unsigned t){
 unsigned f=t<180u?t*256u/180u:256u;
 unsigned smooth=(f*f*(768u-2u*f)+32768u)>>16;
 int s=(int)smooth;
 PotCamera c={160+96*s/256,225-177*s/256,56+200*s/256,
              40-40*s/256,88+40*s/256};
 return c;
}
static PotPoint pot_project(const PotCamera *c,int world_x,int depth,int height){
 PotPoint p={120+pot_set_round((world_x-120)*c->zoom+depth*c->skew,256),
             c->ground_y+pot_set_round(depth*c->ground-height*c->rise,256)};
 return p;
}
static HD_CANVAS_CODE void pot_set_line(uint8_t *p,const PotCamera *c,
                          int x,int d,int h,int xx,int dd,int hh,unsigned ink){
 PotPoint a=pot_project(c,x,d,h),b=pot_project(c,xx,dd,hh);
 hd_line(p,a.x,a.y,b.x,b.y,ink);
}
/* A face parallel to the storefront remains an axis-aligned rectangle. */
static HD_CANVAS_CODE void pot_set_front(uint8_t *p,const PotCamera *c,int x,int d,int h,
                           int width,int height,unsigned ink){
 PotPoint a=pot_project(c,x,d,h+height),b=pot_project(c,x+width,d,h);
 int w=b.x-a.x,hh=b.y-a.y;
 hd_rect(p,a.x,a.y,w>0?w:1,hh>0?hh:1,ink);
}
/* Four short fixed-point edges, with clipped scanline fills. Divisions
   occur once per edge, never per pixel; there is no secondary canvas. */
static HD_CANVAS_CODE void pot_set_quad(uint8_t *p,PotPoint a,PotPoint b,
                          PotPoint c,PotPoint d,unsigned ink){
 typedef struct {int low,high,x,step;} Edge;
 PotPoint v[4]={a,b,c,d};Edge edges[4];unsigned n=0;
 int low=HD_H,high=-1,minx=HD_W,maxx=-1;
 for(unsigned i=0;i<4u;i++){
  if(v[i].y<low)low=v[i].y;
  if(v[i].y>high)high=v[i].y;
  if(v[i].x<minx)minx=v[i].x;
  if(v[i].x>maxx)maxx=v[i].x;
 }
 if(high<0||low>=HD_H||maxx<0||minx>=HD_W)return;
 if(low==high){hd_rect(p,minx,low,maxx-minx+1,1,ink);return;}
 if(low<0)low=0;
 if(high>=HD_H)high=HD_H-1;
 for(unsigned i=0;i<4u;i++){
  PotPoint u=v[i],w=v[(i+1u)&3u];
  if(u.y>w.y){PotPoint swap=u;u=w;w=swap;}
  if(u.y==w.y||w.y<=low||u.y>high)continue;
  int first=u.y<low?low:u.y;
  int step=(w.x-u.x)*65536/(w.y-u.y);
  edges[n++]=(Edge){first,w.y,u.x*65536+step*(first-u.y),step};
 }
 for(int y=low;y<=high;y++){
  int left=0x7fffffff,right=(-0x7fffffff-1);
  for(unsigned i=0;i<n;i++)if(y>=edges[i].low&&y<edges[i].high){
   int x=edges[i].x;
   if(x<left)left=x;
   if(x>right)right=x;
   edges[i].x+=edges[i].step;
  }
  if(left<=right){
   int x=pot_set_round(left,65536),xx=pot_set_round(right,65536);
   hd_rect(p,x,y,xx-x+1,1,ink);
  }
 }
}
/* Horizontal or sloped plane, with its height specified at each depth. */
static void pot_set_face(uint8_t *p,const PotCamera *c,int x,int xx,
                          int d,int dd,int h,int hh,unsigned ink){
 pot_set_quad(p,pot_project(c,x,d,h),pot_project(c,xx,d,h),
                pot_project(c,xx,dd,hh),pot_project(c,x,dd,hh),ink);
}
static void pot_set_side(uint8_t *p,const PotCamera *c,int x,int d,int dd,
                          int bottom,int top,unsigned ink){
 pot_set_quad(p,pot_project(c,x,d,bottom),pot_project(c,x,dd,bottom),
                pot_project(c,x,dd,top),pot_project(c,x,d,top),ink);
}
static unsigned pot_set_letter(char ch,unsigned row){
 static const uint8_t glyph[9][7]={
  {30,17,17,30,16,16,16},{14,17,17,17,17,17,14},
  {31,4,4,4,4,4,4},{17,17,17,31,17,17,17},
  {31,16,16,30,16,16,31},{14,17,17,31,17,17,17},
  {30,17,17,17,17,17,30},{15,16,16,14,1,1,30},
  {17,25,25,21,19,19,17}
 };
 const char *letters="POTHEADSN";
 for(unsigned i=0;i<9u;i++)if(ch==letters[i])return glyph[i][row];
 return 0;
}
static void pot_set_label(uint8_t *p,const PotCamera *c,int x,int d,int top,
                           const char *word,int scale,unsigned ink){
 for(unsigned letter=0;word[letter];letter++){
  for(unsigned row=0;row<7u;row++){
   unsigned bits=pot_set_letter(word[letter],row);
   for(unsigned col=0;col<5u;col++)if(bits&(16u>>col))
    pot_set_front(p,c,x+(int)col*scale,d,top-((int)row+1)*scale,
                    scale,scale,ink);
  }
  x+=7*scale;
 }
}
static void pot_set_window(uint8_t *p,const PotCamera *c,int x,int d,
                            int bottom,int width,int height,unsigned lit){
 pot_set_front(p,c,x,d,bottom,width,height,HD_INK);
 pot_set_front(p,c,x+2,d,bottom+2,width-4,height-4,HD_GLASS);
 pot_set_front(p,c,x+3,d,bottom+3,(width-6)/2,height-6,
                 lit?HD_PLASTER:HD_ROAD_COOL);
 pot_set_front(p,c,x+width/2,d,bottom+2,1,height-4,HD_COPPER);
 pot_set_front(p,c,x+1,d,bottom+height-2,width-2,1,HD_STONE);
 pot_set_front(p,c,x-1,d,bottom-2,width+2,2,HD_STONE_DARK);
 if(lit)pot_set_front(p,c,x+4,d,bottom+height-6,4,2,HD_LAMP);
}
static void pot_set_house(uint8_t *p,const PotCamera *c,int x,int width,
                           int front,int rear,int height,unsigned brick){
 pot_set_side(p,c,x,rear,front,0,height,HD_SHADOW);
 pot_set_front(p,c,x,front,0,width,height,HD_INK);
 pot_set_front(p,c,x+2,front,2,width-4,height-4,
                 brick?HD_BRICK_DARK:HD_STONE_DARK);
 for(int h=8;h<height-6;h+=12)
  pot_set_line(p,c,x+2,front,h,x+width-3,front,h,
                  brick?HD_BRICK:HD_MORTAR);
 for(int xx=x+7;xx<x+width-12;xx+=23)
  for(int h=12;h<height-18;h+=28)
   pot_set_window(p,c,xx,front,h,13,17,(unsigned)(xx+h)&1u);
 pot_set_front(p,c,x-2,front,height-4,width+4,4,HD_STONE);
 pot_set_face(p,c,x-3,x+width+3,rear-3,front+2,height+5,height,
                 HD_ROAD_COOL);
 for(int xx=x+8;xx<x+width;xx+=19)
  pot_set_line(p,c,xx,rear-3,height+5,xx,front+2,height,HD_ROAD);
 pot_set_line(p,c,x-3,front+2,height,x+width+3,front+2,height,HD_STONE_LIT);
}

static void pot_scene(uint8_t *p,const PotCamera *c,unsigned t){
 (void)t; /* Architecture and practical lights stay fixed during the shot. */
 hd_rect(p,0,0,HD_W,HD_H,19u);
 hd_rect(p,0,30,HD_W,HD_H-30,HD_ROAD_COOL);
 pot_set_face(p,c,-300,540,-230,440,0,0,HD_ROAD);
 pot_set_face(p,c,-300,540,-110,-83,0,0,HD_ROAD_LIGHT);
 /* Rear blocks and their service lane establish one continuous town. */
 pot_set_house(p,c,-94,102,-92,-160,71,1u);
 pot_set_house(p,c,25,120,-100,-173,86,0u);
 pot_set_house(p,c,164,122,-94,-165,74,1u);
 /* Sidewalk, stone kerb and subdued wet street reflections. */
 pot_set_face(p,c,-280,520,-3,39,0,0,HD_STONE_DARK);
 pot_set_face(p,c,-280,520,0,36,0,0,HD_STONE);
 for(int x=-64;x<320;x+=24)
  pot_set_line(p,c,x,0,0,x,36,0,HD_MORTAR);
 for(int d=10;d<36;d+=12)
  pot_set_line(p,c,-260,d,0,520,d,0,HD_MORTAR);
 pot_set_line(p,c,-260,37,0,520,37,0,HD_STONE_LIT);
 pot_set_line(p,c,-260,40,0,520,40,0,HD_SHADOW);
 for(int x=-40;x<320;x+=48){
  pot_set_face(p,c,x,x+22,82,85,0,0,HD_MORTAR);
  pot_set_face(p,c,x+8,x+27,53,55,0,0,HD_ROAD_LIGHT);
 }
 pot_set_face(p,c,42,216,1,11,0,0,HD_WET);
 for(int i=0;i<5;i++){
  int x=55+i*30;
  pot_set_face(p,c,x,x+15,20+(i&1)*3,22+(i&1)*3,0,0,HD_REFLECT);
  pot_set_face(p,c,x+3,x+9,26+(i&1)*3,27+(i&1)*3,0,0,HD_PLASTER_LIT);
 }
 /* Attached tenements frame the single central footprint. */
 pot_set_house(p,c,-81,113,-5,-77,101,1u);
 pot_set_house(p,c,216,108,-8,-94,113,0u);
 pot_set_side(p,c,38,-75,0,0,78,HD_BRICK_DARK);
 for(int d=-65;d<0;d+=16)
  pot_set_line(p,c,38,d,2,38,d,77,HD_SHADOW);
 pot_set_front(p,c,38,0,0,172,78,HD_INK);
 pot_set_front(p,c,40,0,2,168,74,HD_BRICK_DARK);
 for(int h=8;h<77;h+=8){
  pot_set_line(p,c,40,0,h,208,0,h,HD_BRICK);
  for(int x=44+((h/8)&1)*10;x<207;x+=20)
   pot_set_line(p,c,x,0,h,x,0,h+7,HD_STONE_DARK);
 }
 pot_set_front(p,c,40,0,3,3,72,HD_STONE_DARK);
 pot_set_front(p,c,205,0,3,3,72,HD_SHADOW);
 /* Slate roof slopes share the same ridge and eaves at every angle. */
 pot_set_quad(p,pot_project(c,35,-78,79),pot_project(c,35,-40,93),
                pot_project(c,35,3,79),pot_project(c,35,3,77),HD_BRICK);
 pot_set_face(p,c,34,214,-78,-40,79,93,HD_ROAD);
 pot_set_face(p,c,34,214,-40,3,93,79,HD_ROAD_COOL);
 for(int d=-32;d<3;d+=9){
  int h=79+(3-d)*14/43;
  pot_set_line(p,c,35,d,h,213,d,h,HD_ROAD);
  for(int x=43+((d+32)/9&1)*8;x<210;x+=17)
   pot_set_line(p,c,x,d,h,x,d+7,h-2,HD_STONE_DARK);
 }
 pot_set_line(p,c,34,-40,94,214,-40,94,HD_STONE);
 pot_set_line(p,c,34,3,79,214,3,79,HD_STONE_LIT);
 pot_set_front(p,c,36,3,75,176,3,HD_STONE_DARK);
 /* A glazed roof light and a brick flue give the aerial shot detail. */
 pot_set_face(p,c,91,121,-66,-51,84,89,HD_INK);
 pot_set_face(p,c,94,118,-64,-53,85,89,HD_GLASS);
 pot_set_line(p,c,106,-64,86,106,-53,90,HD_STONE_LIT);
 pot_set_line(p,c,94,-59,88,118,-59,88,HD_REFLECT);
 pot_set_side(p,c,181,-59,-48,88,105,HD_BRICK_DARK);
 pot_set_front(p,c,181,-48,88,13,17,HD_BRICK);
 pot_set_face(p,c,179,196,-61,-46,105,105,HD_STONE_DARK);
 pot_set_front(p,c,183,-48,93,9,1,HD_BRICK_LIT);
 pot_set_front(p,c,183,-48,99,9,1,HD_STONE_DARK);
 /* Forest-green fascia, cream painted letters, restrained brass rules. */
 pot_set_front(p,c,52,0,59,151,16,HD_INK);
 pot_set_front(p,c,54,0,60,147,14,HD_LEAF_DARK);
 pot_set_front(p,c,56,0,72,143,1,HD_LEAF);
 pot_set_front(p,c,56,0,61,143,1,HD_COPPER);
 pot_set_label(p,c,101,0,70,"POTHEADS",1,HD_PAPER);
 for(int side=0;side<2;side++){
  int x=side?185:69;
  pot_set_line(p,c,x,0,64,x,0,70,HD_LEAF_LIT);
  pot_set_line(p,c,x-3,0,69,x,0,66,HD_LEAF_LIT);
  pot_set_line(p,c,x+3,0,69,x,0,66,HD_LEAF_LIT);
 }
 /* Real recessed glazing: mullions, shelves, packets and practical light. */
 pot_set_window(p,c,49,0,4,61,46,0u);
 pot_set_window(p,c,145,0,4,56,46,0u);
 for(int pane=0;pane<2;pane++){
  int x=pane?148:52;
  pot_set_front(p,c,x+2,0,10,50,2,HD_COPPER);
  pot_set_front(p,c,x+2,0,26,50,1,HD_STONE);
  for(int i=0;i<5;i++){
   pot_set_front(p,c,x+4+i*9,0,12,5,5+(i&1)*2,
                   i&1?HD_LEAF_DARK:HD_PLASTER);
   pot_set_front(p,c,x+5+i*9,0,15,3,1,HD_PAPER);
  }
  pot_set_line(p,c,x+5,0,42,x+15,0,34,HD_REFLECT);
  pot_set_line(p,c,x+28,0,43,x+35,0,38,HD_GLASS_LIT);
 }
 pot_set_front(p,c,57,0,31,10,7,HD_PAPER);
 pot_set_front(p,c,59,0,35,6,1,HD_BARK);
 pot_set_front(p,c,59,0,33,4,1,HD_COPPER);
 pot_set_front(p,c,191,0,12,5,5,HD_BARK);
 pot_set_line(p,c,193,0,17,192,0,29,HD_LEAF_DARK);
 pot_set_line(p,c,187,0,25,192,0,21,HD_LEAF);
 pot_set_line(p,c,198,0,27,193,0,23,HD_LEAF_LIT);
 pot_set_front(p,c,69,0,43,6,2,HD_LAMP);
 pot_set_front(p,c,158,0,43,6,2,HD_LAMP);
 pot_set_front(p,c,115,0,0,26,50,HD_INK);
 pot_set_front(p,c,118,0,3,20,44,HD_COPPER);
 pot_set_front(p,c,120,0,5,16,40,HD_GLASS);
 pot_set_front(p,c,121,0,6,6,37,HD_ROAD_COOL);
 pot_set_front(p,c,128,0,5,1,39,HD_STONE_DARK);
 pot_set_front(p,c,132,0,20,2,3,HD_GOLD);
 pot_set_front(p,c,113,1,0,30,2,HD_STONE_LIT);
 pot_set_front(p,c,164,0,32,32,11,HD_LEAF_DARK);
 pot_set_label(p,c,167,0,41,"OPEN",1,HD_LAMP);
 /* Sloped fabric canopy and its visible front valance are real planes. */
 pot_set_face(p,c,48,205,0,12,57,52,HD_LEAF_DARK);
 for(int x=51;x<203;x+=17)
  pot_set_face(p,c,x,x+8,0,12,57,52,HD_LEAF);
 pot_set_front(p,c,48,12,48,157,4,HD_LEAF_DARK);
 pot_set_line(p,c,48,12,48,205,12,48,HD_COPPER);
 pot_set_line(p,c,51,12,48,51,12,1,HD_STONE_DARK);
 pot_set_line(p,c,202,12,48,202,12,1,HD_STONE_DARK);
 pot_set_line(p,c,40,0,1,209,0,1,HD_PLASTER_LIT);
 /* Planter and street lamp sit on the same pavement as the performer. */
 pot_set_front(p,c,29,10,0,11,11,HD_BARK);
 pot_set_front(p,c,28,10,10,13,2,HD_COPPER);
 pot_set_line(p,c,34,10,11,33,10,24,HD_LEAF_DARK);
 pot_set_line(p,c,27,10,20,33,10,15,HD_LEAF);
 pot_set_line(p,c,40,10,22,34,10,16,HD_LEAF_LIT);
 pot_set_front(p,c,222,18,0,2,88,HD_INK);
 pot_set_front(p,c,216,18,85,12,4,HD_STONE_DARK);
 pot_set_front(p,c,218,18,82,8,4,HD_LAMP);
 pot_set_face(p,c,215,231,14,23,0,0,HD_ROAD_LIGHT);
}
