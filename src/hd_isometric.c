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
static int exposed(int x,int y){
 char c=story_tile(x,y);
 return c!='#'&&c!='A';
}
static unsigned material(char t,unsigned n){
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
static void cutaway(uint8_t *p,int sx,int sy,int front,int side,int shop){
 /* Exposed floor rather than another roof resembling the road. */
 const int h=3;
 unsigned floor=shop?HD_BARK:HD_STONE_DARK;
 for(int col=0;col<16;col++){
  if(front)hd_rect(p,sx+col,sy+8+col/2-h,1,h,HD_BRICK_DARK);
  if(side)hd_rect(p,sx+16+col,sy+16-col/2-h,1,h,HD_SHADOW);
 }
 for(int row=1;row<15;row++){
  int lo=row<8?16-row*2:(row-7)*2;
  hd_rect(p,sx+lo,sy-h+row,32-lo*2,1,floor);
 }
 for(int k=-1;k<=1;k++){
  int x=sx+16+k*6;
  hd_line(p,x-3,sy+6-h,x+3,sy+9-h,shop?HD_BRICK_DARK:HD_ROAD_COOL);
 }
 hd_line(p,sx+2,sy+8-h,sx+16,sy+15-h,HD_INK);
 hd_line(p,sx+17,sy+15-h,sx+29,sy+9-h,HD_INK);
 hd_line(p,sx+3,sy+8-h,sx+12,sy+12-h,HD_MORTAR);
 hd_line(p,sx+20,sy+13-h,sx+27,sy+10-h,HD_STONE);
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
  cutaway(p,sx,sy,front,side,potheads);
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
 /* Compact three-quarter view of the same cream-roofed last bus seen
    in the film. Wheels and shadow remain inside the established tile
    footprint. Four stepped side sections sell depth at 1x resolution. */
 int y=sy+5;
 hd_ellipse(p,sx+16,y+11,17,4,HD_SHADOW);
 hd_rect(p,sx+3,y-10,28,15,HD_RED_SHADE);
 hd_rect(p,sx+4,y-13,25,4,HD_PLASTER);
 hd_rect(p,sx+6,y-14,21,2,HD_PLASTER_LIT);
 hd_rect(p,sx+28,y-11,4,4,HD_STONE);
 hd_rect(p,sx+4,y-9,26,10,HD_RED);
 hd_rect(p,sx+5,y-9,23,6,HD_GLASS);
 hd_rect(p,sx+5,y-9,23,1,HD_ROAD_COOL);
 for(int i=0;i<3;i++){
  int wx=sx+10+i*6;
  hd_rect(p,wx,y-9,1,6,HD_BRICK_DARK);
  hd_pixel(p,wx-2,y-7,HD_GLASS_LIT);
 }
 hd_rect(p,sx+27,y-8,4,7,HD_ROAD_COOL);
 hd_pixel(p,sx+29,y-7,HD_GLASS_LIT);
 hd_rect(p,sx+5,y-2,23,2,HD_PLASTER_LIT);
 hd_rect(p,sx+21,y-8,1,9,HD_RED_SHADE);
 hd_rect(p,sx+5,y+2,26,2,HD_BRICK_DARK);
 hd_ellipse(p,sx+9,y+4,4,4,HD_INK);
 hd_ellipse(p,sx+25,y+4,4,4,HD_INK);
 hd_ellipse(p,sx+9,y+4,2,2,HD_STONE_DARK);
 hd_ellipse(p,sx+25,y+4,2,2,HD_STONE_DARK);
 hd_pixel(p,sx+30,y,(tick/60u)&1u?HD_LAMP:HD_PAPER);
 hd_pixel(p,sx+4,y,HD_RED);
}
/* Miniature architectural landmarks use the existing encounter squares.
   Omit upper architecture when it would cover Danny in the foreground. */
static void cafe_corner(uint8_t *p,int x,int foot,int tall,int absent){
 hd_ellipse(p,x,foot+1,14,3,HD_SHADOW);
 if(tall){
  hd_rect(p,x-15,foot-32,30,29,HD_BRICK_DARK);
  hd_rect(p,x-14,foot-35,28,9,HD_BARK);
  hd_text(p,x-12,foot-34,"CAFE",HD_PAPER,1,0);
  hd_rect(p,x-14,foot-25,28,4,HD_SHADOW);
  for(int j=0;j<5;j++)hd_rect(p,x-13+j*5,foot-25,4,3,j&1?HD_COPPER:HD_BRICK);
  hd_rect(p,x-13,foot-20,26,12,HD_INK);
  hd_rect(p,x-11,foot-19,22,10,absent?HD_ROAD_COOL:HD_BRICK);
  hd_rect(p,x-1,foot-19,2,10,HD_BARK);
  if(!absent){hd_pixel(p,x-8,foot-16,HD_LAMP);hd_pixel(p,x+7,foot-16,HD_GOLD);}
 }
 hd_rect(p,x-14,foot-7,28,3,HD_BARK);
 hd_line(p,x-12,foot-7,x+12,foot-7,HD_COPPER);
 hd_rect(p,x-12,foot-4,3,4,HD_BARK);
 hd_rect(p,x+9,foot-4,3,4,HD_BARK);
 if(absent){
  /* Empty chair and telephone remain in the same physical cafe. */
  hd_rect(p,x-11,foot-12,7,5,HD_BARK);
  hd_rect(p,x-11,foot-7,8,2,HD_COPPER);
  hd_rect(p,x-10,foot-5,2,5,HD_SHADOW);
  hd_rect(p,x-5,foot-5,2,5,HD_SHADOW);
  hd_rect(p,x+4,foot-11,7,4,HD_INK);
  hd_pixel(p,x+9,foot-10,HD_GOLD);
 }
}
static void bore_corner(uint8_t *p,int x,int foot,int tall){
 hd_ellipse(p,x,foot+1,14,3,HD_SHADOW);
 if(tall){
  hd_rect(p,x-14,foot-34,28,28,HD_STONE_DARK);
  hd_rect(p,x-13,foot-35,26,9,HD_BARK);
  hd_text(p,x-11,foot-34,"BORE",HD_PAPER,1,0);
  hd_rect(p,x-12,foot-25,24,14,HD_INK);
  hd_rect(p,x-10,foot-23,20,12,HD_BRICK_DARK);
  for(int j=0;j<5;j++){hd_rect(p,x-9+j*4,foot-22,3,9,HD_BRICK);hd_pixel(p,x-8+j*4,foot-20,HD_GOLD);}
 }
 hd_rect(p,x-13,foot-8,26,4,HD_BARK);
 hd_line(p,x-12,foot-8,x+12,foot-8,HD_COPPER);
 hd_rect(p,x-11,foot-4,3,4,HD_BARK);
 hd_rect(p,x+8,foot-4,3,4,HD_BARK);
 hd_rect(p,x+3,foot-11,7,3,HD_PAPER);
 hd_pixel(p,x+7,foot-10,HD_RED_SHADE);
}
static void object(uint8_t *p,int sx,int sy,char kind,unsigned tick,
                   int wx,int wy,const Story *s,int tall){
 int cx=sx+16,foot=sy+16;
 if(kind=='T'){hd_world_tree_art(p,cx,foot,
                         grain(wx,wy)%3u,(tick/49u)&1u);return;}
 if(kind=='F'){
  int absent=hd_mara_absent(s);
  cafe_corner(p,cx,foot,tall,absent);
  if(!absent)hd_world_person_art(p,cx,foot,1u,1u,0u);
  return;
 }
 if(kind=='C'){
  bore_corner(p,cx,foot,tall);
  hd_world_person_art(p,cx,foot,2u,1u,0u);
  return;
 }
 if(kind=='P'||kind=='V'||kind=='Q'||kind=='W'){
  unsigned role=kind=='P'?3u:kind=='V'?4u:kind=='Q'?5u:6u;
  hd_world_person_art(p,cx,foot,role,1u,0u);
  return;
 }
 if(kind=='B'){bus(p,sx,sy,tick);return;}
 if(kind=='L'){
  /* A grounded mailbox: the family letter no longer floats. */
  hd_ellipse(p,cx,foot+1,9,2,HD_SHADOW);
  hd_rect(p,cx-2,foot-11,4,11,HD_BARK);
  hd_rect(p,cx-8,foot-18,16,9,HD_BRICK_DARK);
  hd_rect(p,cx-7,foot-19,14,8,HD_PAPER);
  hd_line(p,cx-7,foot-19,cx,foot-14,HD_COPPER);
  hd_line(p,cx+6,foot-19,cx,foot-14,HD_COPPER);
  hd_rect(p,cx-8,foot-11,16,2,HD_BARK);
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
  unsigned phase=s->song_steps>4u?4u:s->song_steps;
  for(unsigned j=0;j<4u;j++)hd_pixel(p,cx+1+(int)j*2,foot-5,j<phase?
       (s->project_style&STORY_STYLE_SONG_SOFT?HD_COPPER:HD_LAMP):HD_SHADOW);
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
  /* Small but recognizable framed evening skyline, with a canvas edge
     and easel legs rather than an abstract zig-zag symbol. */
  unsigned phase=s->art_steps,warm=s->project_style&STORY_STYLE_ART_WARM;
  if(!phase){
   hd_rect(p,cx-5,foot-16,10,7,HD_PLASTER);
   hd_line(p,cx-5,foot-10,cx+4,foot-10,HD_STONE);
   hd_line(p,cx-4,foot-12,cx,foot-14,HD_STONE);
   hd_line(p,cx,foot-14,cx+4,foot-12,HD_STONE);
  }else{
   hd_rect(p,cx-5,foot-16,10,4,warm?HD_COPPER:HD_GLASS);
   hd_rect(p,cx-5,foot-12,10,3,warm?HD_BRICK_LIT:HD_ROAD_COOL);
   hd_pixel(p,cx+3,foot-15,HD_LAMP);
   if(phase>=2u){
    hd_rect(p,cx-4,foot-12,3,3,HD_STONE_DARK);
    hd_rect(p,cx,foot-13,3,4,HD_BRICK_DARK);
   }
   if(phase>=3u)hd_pixel(p,cx+1,foot-11,HD_GOLD);
   if(phase>=4u)hd_line(p,cx-5,foot-8,cx+4,foot-8,HD_GOLD);
  }
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
/* Tiny grounded material accents; the same coordinates remain under every
   camera pan. Draw these BEFORE architecture so rain never covers faces. */
static void ground_detail(uint8_t *p,int sx,int sy,unsigned n,
                          char tile,unsigned tick){
 if(tile=='~'){
  int cx=sx+16,cy=sy+8;
  int span=3+(int)((tick/18u+n)%6u);
  hd_line(p,cx-span,cy-1,cx+span,cy-1,HD_GLASS_LIT);
  if((n&3u)==0u)
   hd_line(p,cx-5,cy+2,cx+3,cy+2,HD_REFLECT);
 }else if((tile==','||tile=='=') && (n&15u)==0u){
  hd_line(p,sx+10,sy+9,sx+16,sy+9,HD_REFLECT);
  hd_pixel(p,sx+19,sy+10,HD_STONE_LIT);
 }else if(tile=='.' && (n&31u)==0u){
  hd_line(p,sx+12,sy+8,sx+17,sy+8,HD_STONE_DARK);
  hd_pixel(p,sx+13,sy+9,HD_MORTAR);
  hd_pixel(p,sx+16,sy+9,HD_MORTAR);
 }
}
/* Unlike a top-layer sparkle, this route cue cannot glow through roofs:
   the objective's pavement is decorated at its natural depth. */
static void ground_goal(uint8_t *p,int sx,int sy,unsigned tick){
 unsigned ink=((tick/24u)&1u)?HD_COPPER:HD_GOLD;
 /* Distinct objective diamond with an arrow notch, grounded under props. */
 hd_line(p,sx+5,sy+10,sx+11,sy+13,ink);
 hd_line(p,sx+11,sy+13,sx+16,sy+11,ink);
 hd_line(p,sx+27,sy+10,sx+21,sy+13,ink);
 hd_line(p,sx+21,sy+13,sx+16,sy+11,ink);
 hd_line(p,sx+12,sy+7,sx+16,sy+4,HD_INK);
 hd_line(p,sx+16,sy+4,sx+20,sy+7,HD_INK);
 hd_line(p,sx+13,sy+7,sx+16,sy+5,ink);
 hd_line(p,sx+16,sy+5,sx+19,sy+7,ink);
 hd_pixel(p,sx+16,sy+8,HD_LAMP);
}
/* The next walkable step gets a small painted chevron at natural depth,
   not a floating HUD marker glowing through rooftops. */
static void ground_route(uint8_t *p,int sx,int sy,unsigned tick){
 unsigned ink=((tick/30u)&1u)?HD_GLASS_LIT:HD_GOLD;
 hd_line(p,sx+10,sy+8,sx+16,sy+5,HD_INK);
 hd_line(p,sx+16,sy+5,sx+22,sy+8,HD_INK);
 hd_line(p,sx+11,sy+9,sx+16,sy+6,ink);
 hd_line(p,sx+16,sy+6,sx+21,sy+9,ink);
 hd_pixel(p,sx+16,sy+10,HD_GOLD);
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
 /* A tile row projects to two monotone screen coordinates. Keep the
    exact original draw order while avoiding per-tile projection and
    stopping as soon as the remainder of a row is off-screen. */
 for(int wy=-1;wy<=STORY_H;wy++){
  int sx=104+16*(-1-wy-(int)s->x+(int)s->y);
  int sy=72+8*(-1+wy-(int)s->x-(int)s->y);
  for(int wx=-1;wx<=STORY_W;wx++,sx+=16,sy+=8){
   if(sx<=-32||sy<=-18)continue;
   if(sx>=240||sy>=154)break;
   char c=story_tile(wx,wy);
   unsigned n=grain(wx,wy);
   hd_iso_tile(p,sx,sy,material(c,n));
   if(c=='~'||c==','||c=='='||c=='.')
    ground_detail(p,sx,sy,n,c,tick);
  }
 }
 rain(p,tick);
 int gx=0,gy=0;
 const char *goal=0;
 int has_goal=story_goal(s,&gx,&gy,&goal);
 (void)goal;
 int route_dir=-1,route_steps=-1;
 int has_route=story_route(s,&route_dir,&route_steps);
 static const int8_t dx[4]={1,0,-1,0},dy[4]={0,1,0,-1};
 int next_x=(int)s->x,next_y=(int)s->y;
 if(has_route && route_steps>0 && route_dir>=0 && route_dir<4){
  next_x+=dx[route_dir];next_y+=dy[route_dir];
 }else has_route=0;
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
  /* Depth slices share a screen Y. Clamp the world span first; each
     successive X then advances exactly 32 screen pixels. This avoids
     traversing invisible map rows without touching the painter order. */
  int sy=72+8*(depth-(int)s->x-(int)s->y);
  if(sy<=-55||sy>=166)continue;
  int first=depth-STORY_H,last=depth+1;
  if(first< -1)first=-1;
  if(last>STORY_W)last=STORY_W;
  int sx=104+16*(2*first-depth-(int)s->x+(int)s->y);
  for(int wx=first;wx<=last;wx++,sx+=32){
   if(sx<=-48)continue;
   if(sx>=256)break;
   int wy=depth-wx;
   char c=story_tile(wx,wy);
   if(has_route&&wx==next_x&&wy==next_y)
    ground_route(p,sx,sy,tick);
   if(has_goal&&wx==gx&&wy==gy)
    ground_goal(p,sx,sy,tick);
   if(c=='#'||c=='A')building(p,sx,sy,wx,wy,tick,c,
                            actor_x,actor_foot,player_depth);
   else if(c=='T'||c=='F'||c=='C'||c=='P'||c=='V'||
           c=='Q'||c=='W'||c=='B'||c=='L'||c=='K'||
           c=='M'||c=='S'||c=='H'||c=='R'||c=='D')
    object(p,sx,sy,c,tick,wx,wy,s,
       !(depth*8>=player_depth && sx<actor_x+12 && sx+32>actor_x-12 &&
         sy+16>actor_foot-32 && sy-22<actor_foot+2));
   if(wx==17&&wy==13)lamp(p,sx,sy,tick);
  }
 }
 if(!drew_player)
  hd_world_person_art(p,actor_x,actor_foot,0u,s->facing,0u);
 /* World-space goal cues are already depth sorted. The compass provides
    a reliable fallback when the target is behind buildings. */
}

