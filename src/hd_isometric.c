#include "hd_canvas.h"
#include "hd_isometric_atlas.h"
#include "hd_world_atlas.h"
#include "hd_facade_art.h"

/* A real isometric projection of the ACTUAL 24x18 walkable town:
   x grows southeast and y southwest, so D-pad logic stays cardinal
   and all quests, collisions and save slots are unchanged.
   One shared 240x160 mode-4 buffer; no heap, second frame or Z buffer.
   ROM prepaints the textured ground diamonds; only vertical architecture,
   people, lights and restrained rain are composed at runtime. */
static unsigned grain(int x,int y){
 uint32_t n=(uint32_t)(x+101)*374761393u+
            (uint32_t)(y+131)*668265263u;
 n^=n>>13;n*=1274126177u;return (unsigned)(n^(n>>16));
}
static void projection(const Story *s,int wx,int wy,int *sx,int *sy){
 int dx=wx-(int)s->x,dy=wy-(int)s->y;
 *sx=104+16*(dx-dy); /* left edge: center of player's diamond is x=120 */
 *sy=72+8*(dx+dy);   /* feet stand at y=88, under the scene overlay */
}
static int exposed(int x,int y){
 char c=story_tile(x,y);
 return c!='#'&&c!='A';
}
static unsigned material(char t,int x,int y){
 unsigned n=grain(x,y);
 if(t=='~')return 8u+(n&1u);
 if(t=='g'||t=='T')return 6u+(n&1u);
 if(t==','||t=='=')return 4u+(n&1u);
 if(t=='#'||t=='A')return 10u+(n&1u);
 if(t=='D'||t=='L')return 12u+(n&1u);
 return n&3u;
}
static void night(uint8_t *p){
 for(int y=0;y<160;y++){
  unsigned c=y<34?HD_ROAD_COOL:
             y<84?HD_STONE_DARK:
             y<126?HD_ROAD:HD_ROAD_COOL;
  hd_rect(p,0,y,240,1,c);
 }
 /* Rooftop silhouettes at the margins make clipped edge-of-map camera
    views feel like a larger city; no claims that these are walkable. */
 for(int i=-1;i<13;i++){
  int x=i*22-8,h=8+(i*i*7+11)%19;
  hd_rect(p,x,32-h,22,h,HD_STONE_DARK);
  hd_rect(p,x+2,32-h-2,18,2,HD_ROAD_COOL);
  for(int w=5;w<19;w+=7)
   hd_rect(p,x+w,32-h+5,2,3,HD_GLASS);
 }
}
/* Potheads is a regular business with a dependable employee, not a
   quest trap. The banner is attached to the shop's side facade rather
   than drawn as a floating HUD caption over the pavement. */
static void potheads_fascia(uint8_t *p,int sx,int sy,int h){
 int y=sy-h+22;
 hd_rect(p,sx+19,y,52,15,HD_SHADOW);
 hd_rect(p,sx+18,y-1,54,13,HD_LEAF_DARK);
 hd_rect(p,sx+20,y+1,50,9,HD_BRICK_DARK);
 hd_rect(p,sx+20,y+1,50,2,HD_LEAF);
 hd_text(p,sx+23,y+4,"POTHEADS",HD_PAPER,1,0);
 hd_rect(p,sx+22,y+12,45,2,HD_GOLD);
 /* A restrained botanical emblem, styled like the reference shop's
    green insignia but rendered in physical world coordinates. */
 hd_line(p,sx+19,y-5,sx+18,y,HD_LEAF);
 hd_line(p,sx+19,y-4,sx+16,y-6,HD_LEAF_LIT);
 hd_line(p,sx+19,y-4,sx+23,y-7,HD_LEAF_LIT);
 hd_line(p,sx+19,y-3,sx+15,y-3,HD_LEAF);
 hd_line(p,sx+19,y-3,sx+23,y-3,HD_LEAF);
}
static void building(uint8_t *p,int sx,int sy,int wx,int wy,
                     unsigned tick,char type){
 unsigned n=grain(wx,wy);
 /* Shared roof heights are deliberately keyed to 3x3 districts;
    visually contiguous blocks never extrude like shuffled cubes. */
 unsigned h=27u+(unsigned)(((wx/3+wy/3)&1)*9);
 if(type=='A')h=36u;
 int front=exposed(wx,wy+1),side=exposed(wx+1,wy);
 unsigned variant=(n>>10)&7u;
 /* An actual Potheads storefront resides on the accessible edge of
    the south-east block, not at a randomly moving map location. */
 int potheads=(wx==21&&wy==11);
 if(potheads)variant=7u;
 else if(type=='A')variant=6u;
 else if(wx>=10&&wx<=13&&wy>=3&&wy<=6)variant=3u;
 if(front)hd_facade_wall(p,sx,sy,(int)h,variant,0u);
 if(side)hd_facade_wall(p,sx+16,sy,(int)h,variant,1u);
 hd_iso_tile(p,sx,sy-(int)h,10u+(n&1u));
 /* Roof edges, gutters and shadowed coping belong to the building
    footprint; do not paint an unrelated rectangle over the street. */
 hd_line(p,sx+1,sy+8-(int)h,sx+16,sy+15-(int)h,HD_COPPER);
 hd_line(p,sx+17,sy+15-(int)h,sx+31,sy+8-(int)h,HD_STONE_LIT);
 if(front){
  hd_line(p,sx+1,sy+8,sx+15,sy+15,HD_STONE_DARK);
  hd_rect(p,sx+11,sy+11-(int)h/3,3,5,HD_GLASS);
  hd_pixel(p,sx+12,sy+14-(int)h/3,HD_LAMP);
 }
 if(side)hd_line(p,sx+17,sy+15,sx+31,sy+8,HD_STONE_DARK);
 if((n&3u)==0u){
  hd_rect(p,sx+16,sy-3-(int)h,3,5,HD_STONE_DARK);
  hd_rect(p,sx+16,sy-4-(int)h,5,1,HD_PLASTER);
 }
 if(potheads&&side)potheads_fascia(p,sx,sy,(int)h);
 (void)tick;
}
static void lamp(uint8_t *p,int sx,int sy,unsigned tick){
 int x=sx+16,y=sy+12;
 hd_ellipse(p,x+4,y+2,9,3,HD_ROAD_COOL);
 hd_rect(p,x-1,y-29,3,29,HD_INK);
 hd_rect(p,x-3,y-31,7,2,HD_COPPER);
 hd_rect(p,x-5,y-28,11,4,HD_PLASTER);
 hd_rect(p,x-4,y-27,9,3,HD_LAMP);
 hd_pixel(p,x,y-27,HD_PAPER);
 for(int k=0;k<5;k++){
  int off=k*3;
  hd_pixel(p,x-off,y+off/2,HD_ROAD_LIGHT);
  if(((tick/45u)&1u)==0u)
   hd_pixel(p,x+off,y+off/3,HD_REFLECT);
 }
}
static void bus(uint8_t *p,int sx,int sy,unsigned tick){
 int y=sy+5;
 hd_ellipse(p,sx+16,y+11,17,4,HD_SHADOW);
 hd_rect(p,sx+3,y-12,29,16,HD_RED_SHADE);
 hd_rect(p,sx+4,y-13,28,12,HD_RED);
 hd_rect(p,sx+7,y-11,21,7,HD_GLASS);
 for(int i=0;i<3;i++)hd_rect(p,sx+11+i*7,y-11,1,7,HD_BRICK_DARK);
 hd_rect(p,sx+8,y-3,24,3,HD_BRICK_DARK);
 hd_ellipse(p,sx+9,y+4,4,4,HD_INK);
 hd_ellipse(p,sx+27,y+4,4,4,HD_INK);
 hd_pixel(p,sx+31,y-6,(tick/60u)&1u?HD_LAMP:HD_PAPER);
}
static void object(uint8_t *p,int sx,int sy,char kind,unsigned tick,int wx,int wy){
 int cx=sx+16,foot=sy+16;
 if(kind=='T'){hd_world_tree_art(p,cx,foot,
                         grain(wx,wy)%3u,(tick/49u)&1u);return;}
 if(kind=='F'||kind=='C'||kind=='P'||kind=='V'||
    kind=='Q'||kind=='W'){
  unsigned role=kind=='F'?1u:kind=='C'?2u:kind=='P'?3u:
                kind=='V'?4u:kind=='Q'?5u:6u;
  hd_world_person_art(p,cx,foot,role,1u,0u);
  return;
 }
 if(kind=='B'){bus(p,sx,sy,tick);return;}
 if(kind=='L'){
  hd_rect(p,cx-7,foot-9,14,9,HD_PAPER);
  hd_line(p,cx-7,foot-9,cx,foot-3,HD_COPPER);
  hd_line(p,cx+6,foot-9,cx,foot-3,HD_COPPER);
  return;
 }
 if(kind=='M'||kind=='K'||kind=='S'||kind=='H'||kind=='R'){
  /* Reachable landmark, not an impassable new building. A physically
     placed shop sign leaves story collisions and objectives intact. */
  hd_rect(p,cx-10,foot-20,20,19,HD_BRICK_DARK);
  hd_rect(p,cx-8,foot-18,16,13,HD_GLASS);
  hd_rect(p,cx-9,foot-21,18,3,HD_COPPER);
  hd_rect(p,cx-6,foot-16,12,2,HD_PAPER);
  if(kind=='M'){
   hd_ellipse(p,cx,foot-10,5,5,HD_INK);
   hd_ellipse(p,cx,foot-10,2,2,HD_GOLD);
  }else if(kind=='K'){
   hd_rect(p,cx-1,foot-14,2,8,HD_PAPER);
   hd_rect(p,cx-4,foot-11,8,2,HD_PAPER);
  }else if(kind=='R'){
   hd_rect(p,cx-4,foot-14,9,7,HD_GOLD);
  }else{
   hd_rect(p,cx-3,foot-14,7,5,HD_GLASS_LIT);
  }
  hd_rect(p,cx-9,foot-1,18,2,HD_STONE_DARK);
  return;
 }
 if(kind=='D'){
  hd_rect(p,cx-4,foot-9,8,8,HD_PLASTER);
  hd_rect(p,cx-3,foot-7,6,5,HD_GLASS);
 }
}
static void rain(uint8_t *p,unsigned tick){
 /* Screen-space weather is bounded and intentionally subtle: only
    24 light strokes per frame, unaffected by the time-of-day scripts. */
 for(int k=0;k<24;k++){
  unsigned seed=grain(k+17,k*7+13);
  int x=(int)((seed+tick*2u)%252u)-8;
  int y=19+(int)((seed/97u+tick*3u)%119u);
  int len=(k&3)==0?7:4;
  hd_line(p,x,y,x-2,y+len,k%3u==0u?HD_STONE_LIT:HD_GLASS_LIT);
 }
}
void hd_isometric_draw(uint8_t *p,const Story *s,unsigned tick){
 night(p);
 /* Terrain first. Occlusion and architectural depth sorted by world
    x+y, the exact order of isometric foot placement. */
 for(int wy=-1;wy<=STORY_H;wy++)
  for(int wx=-1;wx<=STORY_W;wx++){
   int sx,sy;projection(s,wx,wy,&sx,&sy);
   if(sx<=-32||sx>=240||sy<=-18||sy>=154)continue;
   char c=story_tile(wx,wy);
   hd_iso_tile(p,sx,sy,material(c,wx,wy));
  }
 unsigned player_depth=(unsigned)s->x+(unsigned)s->y;
 int drew_player=0;
 for(int depth=-2;depth<=STORY_W+STORY_H+1;depth++){
  for(int wx=-1;wx<=STORY_W;wx++){
   int wy=depth-wx;
   if(wy<-1||wy>STORY_H)continue;
   int sx,sy;projection(s,wx,wy,&sx,&sy);
   if(sx<=-48||sx>=256||sy<=-55||sy>=166)continue;
   char c=story_tile(wx,wy);
   if(c=='#'||c=='A')building(p,sx,sy,wx,wy,tick,c);
   else if(c=='T'||c=='F'||c=='C'||c=='P'||c=='V'||
           c=='Q'||c=='W'||c=='B'||c=='L'||c=='K'||
           c=='M'||c=='S'||c=='H'||c=='R'||c=='D')
    object(p,sx,sy,c,tick,wx,wy);
   if(wx==17&&wy==13)lamp(p,sx,sy,tick);
  }
  if(!drew_player&&depth>=(int)player_depth){
   int ox=0,oy=0;
   if(s->anim){
    ox=((int)s->prev_x-(int)s->x -
        ((int)s->prev_y-(int)s->y))*s->anim*4;
    oy=((int)s->prev_x-(int)s->x +
        ((int)s->prev_y-(int)s->y))*s->anim*2;
   }
   hd_world_person_art(p,120+ox,88+oy,0u,s->facing,
                       s->anim?((tick/7u)&1u):0u);
   drew_player=1;
  }
 }
 if(!drew_player)
  hd_world_person_art(p,120,88,0u,s->facing,0u);
 /* The original story_goal is a logical coordinate, not a new quest.
    Render a restrained in-world cue only if the target is visible. */
 int gx,gy;
 const char *title;
 if(story_goal(s,&gx,&gy,&title)){
  int sx,sy;projection(s,gx,gy,&sx,&sy);
  if(sx>4&&sx<208&&sy>45&&sy<123){
   int cx=sx+16,top=sy-9-(int)((tick/24u)&1u);
   hd_pixel(p,cx,top,HD_GOLD);
   hd_line(p,cx-4,top+3,cx,top+6,HD_LAMP);
   hd_line(p,cx+4,top+3,cx,top+6,HD_LAMP);
  }
 }
 rain(p,tick);
}
