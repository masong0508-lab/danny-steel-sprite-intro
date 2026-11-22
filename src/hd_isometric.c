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
 hd_rect(p,0,0,240,34,HD_ROAD_COOL);
 hd_rect(p,0,34,240,50,HD_STONE_DARK);
 hd_rect(p,0,84,240,42,HD_ROAD);
 hd_rect(p,0,126,240,34,HD_ROAD_COOL);
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
 /* A bracket-mounted blade sign visible ABOVE the nearby rooflines.
    This is the storefront name, not the future handheld 21+ placard. */
 int x=sx+17,y=sy-h-12;
 hd_line(p,x+3,y+11,x+15,y+12,HD_INK);
 hd_line(p,x+3,y+13,x+15,y+12,HD_COPPER);
 hd_rect(p,x+14,y,53,16,HD_SHADOW);
 hd_rect(p,x+15,y-1,50,14,HD_LEAF_DARK);
 hd_rect(p,x+17,y+1,46,10,HD_BRICK_DARK);
 hd_rect(p,x+17,y+1,46,2,HD_LEAF);
 hd_text(p,x+20,y+4,"POTHEADS",HD_PAPER,1,0);
 hd_rect(p,x+17,y+12,46,2,HD_GOLD);
 /* Green botanical ornament, with no advertising gameplay interaction. */
 hd_line(p,x+12,y+4,x+11,y+10,HD_LEAF);
 hd_line(p,x+12,y+7,x+7,y+5,HD_LEAF_LIT);
 hd_line(p,x+12,y+7,x+16,y+3,HD_LEAF_LIT);
 hd_line(p,x+12,y+8,x+7,y+10,HD_LEAF);
 hd_line(p,x+12,y+8,x+17,y+9,HD_LEAF);
}
/* A foreground building may hide the entire 34-pixel player at the
   storefront spawn. Reveal only the architectural mass intersecting his
   actual moving silhouette. This is a low wall cross-section, never a
   second actor painted on top of a roof. All world footprints stay put. */
static void cutaway(uint8_t *p,int sx,int sy,int front,int side){
 const int h=6;
 for(int col=0;col<16;col++){
  if(front)hd_rect(p,sx+col,sy+8+col/2-h,1,h,HD_BRICK_DARK);
  if(side)hd_rect(p,sx+16+col,sy+16-col/2-h,1,h,HD_SHADOW);
 }
 for(int row=1;row<15;row++){
  int lo=row<8?16-row*2:(row-7)*2;
  hd_rect(p,sx+lo,sy-h+row,32-lo*2,1,HD_WET);
 }
 /* A thick dark section and broken stone lip explain the removed roof. */
 hd_line(p,sx+2,sy+8-h,sx+16,sy+15-h,HD_STONE_DARK);
 hd_line(p,sx+17,sy+15-h,sx+29,sy+9-h,HD_STONE_DARK);
 hd_line(p,sx+3,sy+8-h,sx+12,sy+12-h,HD_BARK);
 hd_line(p,sx+20,sy+13-h,sx+27,sy+10-h,HD_MORTAR);
}
static void building(uint8_t *p,int sx,int sy,int wx,int wy,
                     unsigned tick,char type,int actor_x,int actor_foot,
                     int actor_depth){
 unsigned n=grain(wx,wy);
 /* Shared roof heights are deliberately keyed to 3x3 districts;
    visually contiguous blocks never extrude like shuffled cubes. */
 unsigned h=27u+(unsigned)(((wx/3+wy/3)&1)*9);
 if(type=='A')h=36u;
 int front=exposed(wx,wy+1),side=exposed(wx+1,wy);
 /* Neighboring wall units share an architectural style. Source
    selection is per district, NOT independent random shop wallpaper. */
 unsigned variant=(unsigned)(((wx/3)*5+(wy/3)*3)&7);
 int potheads=(wx==21&&wy==11);
 /* Projected bounds include the small chimney above the roof. The
    fixed-point depth uses the same gait interpolation as the actor. */
 if((wx+wy)*8>actor_depth && sx<actor_x+12 && sx+32>actor_x-12 &&
    sy-(int)h-5<actor_foot+2 && sy+16>actor_foot-32){
  cutaway(p,sx,sy,front,side);
  if(potheads&&side){
   /* Retain the far sign pier, outside the walking silhouette, so the
      existing bracket has a real attachment even with the roof cut. */
   hd_rect(p,sx+30,sy-(int)h,3,(int)h+8,HD_SHADOW);
   hd_rect(p,sx+31,sy-(int)h,1,(int)h+7,HD_LEAF_DARK);
   potheads_fascia(p,sx,sy,(int)h);
  }
  return;
 }
 if(potheads)variant=7u;
 else if(wx>=21&&wy>=10&&wy<=12)variant=4u;
 else if(type=='A')variant=6u;
 else if(wx>=10&&wx<=13&&wy>=3&&wy<=6)variant=3u;
 if(front)hd_facade_wall(p,sx,sy,(int)h,variant,0u);
 if(side)hd_facade_wall(p,sx+16,sy,(int)h,variant,1u);
 hd_iso_tile(p,sx,sy-(int)h,10u+(n&1u));
 /* Only an exposed eave gets a coping edge. Outlining every internal
    roof tile in copper made contiguous blocks look like a glowing grid.
    A dark underside and a short, cool worn edge give the roof thickness. */
 if(front){
  hd_line(p,sx+1,sy+9-(int)h,sx+16,sy+16-(int)h,HD_INK);
  hd_line(p,sx+1,sy+8-(int)h,sx+16,sy+15-(int)h,HD_STONE_DARK);
  hd_line(p,sx+4,sy+9-(int)h,sx+10,sy+12-(int)h,HD_MORTAR);
 }
 if(side){
  hd_line(p,sx+17,sy+16-(int)h,sx+31,sy+9-(int)h,HD_INK);
  hd_line(p,sx+17,sy+15-(int)h,sx+31,sy+8-(int)h,HD_ROAD_LIGHT);
  hd_line(p,sx+23,sy+12-(int)h,sx+29,sy+9-(int)h,HD_MORTAR);
 }
 if(front){
  hd_line(p,sx+1,sy+8,sx+15,sy+15,HD_STONE_DARK);
  hd_rect(p,sx+11,sy+11-(int)h/3,3,5,HD_GLASS);
  hd_pixel(p,sx+12,sy+14-(int)h/3,HD_LAMP);
 }
 if(side)hd_line(p,sx+17,sy+15,sx+31,sy+8,HD_STONE_DARK);
 if((n&3u)==0u){
  hd_rect(p,sx+16,sy-3-(int)h,3,5,HD_STONE_DARK);
  hd_rect(p,sx+16,sy-4-(int)h,5,1,HD_MORTAR);
 }
 if(potheads&&side)potheads_fascia(p,sx,sy,(int)h);
 (void)tick;
}
static void lamp(uint8_t *p,int sx,int sy,unsigned tick){
 int x=sx+16,y=sy+12;
 /* Reflections are short horizontal breaks under the visible source. */
 hd_ellipse(p,x+2,y+3,10,3,HD_WET);
 hd_line(p,x-5,y+1,x+7,y+1,HD_ROAD_LIGHT);
 hd_line(p,x-2,y+3,x+5,y+3,HD_COPPER);
 hd_line(p,x-4,y+5,x+2,y+5,HD_ROAD_COOL);
 hd_rect(p,x-1,y-29,3,29,HD_INK);
 hd_rect(p,x-3,y-31,7,2,HD_COPPER);
 hd_rect(p,x-5,y-28,11,4,HD_PLASTER);
 hd_rect(p,x-4,y-27,9,3,HD_LAMP);
 hd_pixel(p,x,y-27,HD_PAPER);
 if(((tick/45u)&1u)==0u)hd_rect(p,x-1,y+3,3,1,HD_GOLD);
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
 if(kind=='M'){
  /* A grounded record player: spindle, tonearm and separate speakers. */
  hd_ellipse(p,cx,foot+1,13,3,HD_SHADOW);
  hd_rect(p,cx-12,foot-14,24,12,HD_BARK);
  hd_rect(p,cx-10,foot-12,20,8,HD_BRICK);
  hd_ellipse(p,cx-3,foot-8,6,4,HD_INK);
  hd_ellipse(p,cx-3,foot-8,2,2,HD_COPPER);
  hd_pixel(p,cx-3,foot-8,HD_GOLD);
  hd_line(p,cx+6,foot-11,cx+6,foot-7,HD_PAPER);
  hd_line(p,cx+6,foot-7,cx+1,foot-6,HD_PAPER);
  hd_rect(p,cx-14,foot-16,3,10,HD_SHADOW);
  hd_rect(p,cx+11,foot-16,3,10,HD_SHADOW);
  hd_pixel(p,cx+9,foot-5,HD_LAMP);
  return;
 }
 if(kind=='R'){
  /* Freestanding painting with visible three-legged wooden easel. */
  hd_ellipse(p,cx,foot,10,3,HD_SHADOW);
  hd_line(p,cx-7,foot,cx-2,foot-19,HD_BARK);
  hd_line(p,cx+7,foot,cx+2,foot-19,HD_BARK);
  hd_line(p,cx,foot,cx,foot-17,HD_COPPER);
  hd_rect(p,cx-8,foot-19,16,14,HD_BARK);
  hd_rect(p,cx-6,foot-17,12,10,HD_PAPER);
  hd_rect(p,cx-5,foot-16,10,4,HD_GLASS);
  hd_line(p,cx-4,foot-9,cx,foot-13,HD_RED_SHADE);
  hd_line(p,cx,foot-13,cx+5,foot-9,HD_GOLD);
  hd_rect(p,cx-10,foot-5,20,2,HD_COPPER);
  return;
 }
 if(kind=='K'){
  /* Tiled entrance and a real cross rather than an interchangeable sign. */
  hd_ellipse(p,cx,foot+1,12,3,HD_SHADOW);
  hd_rect(p,cx-11,foot-21,22,21,HD_STONE_DARK);
  hd_rect(p,cx-9,foot-19,18,17,HD_PLASTER);
  hd_rect(p,cx-5,foot-16,10,14,HD_GLASS);
  hd_rect(p,cx-1,foot-16,2,14,HD_BRICK_DARK);
  hd_rect(p,cx-7,foot-23,14,5,HD_INK);
  hd_rect(p,cx-1,foot-22,2,4,HD_PAPER);
  hd_rect(p,cx-3,foot-21,6,2,HD_PAPER);
  hd_pixel(p,cx+6,foot-6,HD_LAMP);
  return;
 }
 if(kind=='S'){
  /* Information terminal with a grounded keyboard and active signal. */
  hd_ellipse(p,cx,foot+1,10,3,HD_SHADOW);
  hd_rect(p,cx-9,foot-22,18,17,HD_INK);
  hd_rect(p,cx-7,foot-20,14,12,HD_GLASS);
  hd_rect(p,cx-5,foot-18,10,2,HD_GLASS_LIT);
  hd_rect(p,cx-5,foot-14,7,1,HD_LEAF_LIT);
  hd_pixel(p,cx+4,foot-11,(tick/26u)&1u?HD_LAMP:HD_PAPER);
  hd_rect(p,cx-10,foot-7,20,3,HD_STONE_DARK);
  hd_rect(p,cx-6,foot-4,12,4,HD_BARK);
  return;
 }
 if(kind=='H'){
  /* Heavy rescue threshold with cross-bracing and an actual latch. */
  hd_ellipse(p,cx,foot+1,11,3,HD_SHADOW);
  hd_rect(p,cx-10,foot-24,20,24,HD_STONE_DARK);
  hd_rect(p,cx-8,foot-22,16,21,HD_BRICK_DARK);
  hd_rect(p,cx-6,foot-20,12,18,HD_BARK);
  hd_rect(p,cx-6,foot-20,12,2,HD_COPPER);
  hd_line(p,cx-4,foot-17,cx+4,foot-7,HD_BRICK);
  hd_line(p,cx+4,foot-17,cx-4,foot-7,HD_BRICK);
  hd_rect(p,cx+3,foot-10,2,3,HD_LAMP);
  return;
 }
 if(kind=='D'){
  hd_rect(p,cx-4,foot-9,8,8,HD_PLASTER);
  hd_rect(p,cx-3,foot-7,6,5,HD_GLASS);
 }
}
static void rain(uint8_t *p,unsigned tick){
 /* Fourteen subdued strokes behind the depth-sorted architecture and
    people: rain cannot scribble across Danny's face or a shop sign. */
 for(int k=0;k<14;k++){
  unsigned seed=grain(k+17,k*7+13);
  int x=(int)((seed+tick*2u)%252u)-8;
  int y=19+(int)((seed/97u+tick*3u)%119u);
  int len=(k&3)==0?5:3;
  hd_line(p,x,y,x-1,y+len,k%4u==0u?HD_MORTAR:HD_ROAD_COOL);
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
 rain(p,tick);
 int ox=0,oy=0;
 if(s->anim){
  ox=((int)s->prev_x-(int)s->x -
      ((int)s->prev_y-(int)s->y))*s->anim*4;
  oy=((int)s->prev_x-(int)s->x +
      ((int)s->prev_y-(int)s->y))*s->anim*2;
 }
 int actor_x=120+ox,actor_foot=88+oy;
 int player_depth=((int)s->x+(int)s->y)*8+oy;
 int drew_player=0;
 for(int depth=-2;depth<=STORY_W+STORY_H+1;depth++){
  /* Insert between the actual fractional depth planes while walking;
     drawing at the destination cell early would put him through walls. */
  if(!drew_player&&depth*8>player_depth){
   hd_world_person_art(p,actor_x,actor_foot,0u,s->facing,
                       s->anim?((tick/7u)&1u):0u);
   drew_player=1;
  }
  for(int wx=-1;wx<=STORY_W;wx++){
   int wy=depth-wx;
   if(wy<-1||wy>STORY_H)continue;
   int sx,sy;projection(s,wx,wy,&sx,&sy);
   if(sx<=-48||sx>=256||sy<=-55||sy>=166)continue;
   char c=story_tile(wx,wy);
   if(c=='#'||c=='A')building(p,sx,sy,wx,wy,tick,c,
                            actor_x,actor_foot,player_depth);
   else if(c=='T'||c=='F'||c=='C'||c=='P'||c=='V'||
           c=='Q'||c=='W'||c=='B'||c=='L'||c=='K'||
           c=='M'||c=='S'||c=='H'||c=='R'||c=='D')
    object(p,sx,sy,c,tick,wx,wy);
   if(wx==17&&wy==13)lamp(p,sx,sy,tick);
  }
 }
 if(!drew_player)
  hd_world_person_art(p,actor_x,actor_foot,0u,s->facing,0u);
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
}

