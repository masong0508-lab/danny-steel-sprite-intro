#include "hd_canvas.h"

/* Carefully staged Old Quarter locations use the SAME narrative scene and
   page state as the original 120x80 director. They replace the most-seen
   cinema sets at real 240x160 while leaving specialized endings untouched. */
int hd_cinema_supports(unsigned scene){
 switch(scene){
  case CINE_OPEN:case CINE_LETTER:case CINE_BORE:
  case CINE_MARA:case CINE_BUS:case CINE_STAY:
  case CINE_MUSIC:case CINE_ART:case CINE_NOTICE:
  case CINE_CLINIC:case CINE_FOLLOWUP:case CINE_LAST_CLINIC:
   return 1;
  default:return hd_special_supports(scene);
 }
}
static unsigned hash(int x,int y){
 uint32_t n=(uint32_t)(x+137)*1664525u+(uint32_t)(y+101)*1013904223u;
 n^=n>>13;n*=2246822519u;return n^(n>>14);
}
static int ease(int from,int to,unsigned t,unsigned begin,unsigned span){
 if(t<=begin)return from;
 if(t>=begin+span)return to;
 unsigned f=(t-begin)*256u/span;
 unsigned smooth=f*f*(768u-2u*f)>>16;
 return from+(to-from)*(int)smooth/256;
}
static void dusk(uint8_t *p,unsigned frame,int night){
 for(int y=0;y<160;y++){
  unsigned c=y<37?(night?19:17):y<71?(night?24:25):
    y<91?(night?HD_GLASS:HD_GLASS_LIT):
    y<117?HD_STONE_DARK:y<135?HD_ROAD_COOL:HD_ROAD;
  hd_rect(p,0,y,HD_W,1,c);
 }
 hd_ellipse(p,186,34,9,9,night?HD_COPPER:HD_LAMP);
 for(int i=0;i<18;i++){
  int x=i*20-12;
  int h=18+(int)(hash(i,11)%31u);
  hd_rect(p,x,112-h,20,h,HD_STONE_DARK);
  hd_rect(p,x,112-h,20,2,HD_SHADOW);
  for(int j=4;j<19;j+=7)for(int yy=112-h+8;yy<107;yy+=9)
   if((hash(i+j,yy)&3u)!=0)
    hd_rect(p,x+j,yy,3,3,(i+j+yy+(int)(frame/120u))%11==0?HD_LAMP:HD_GLASS);
 }
 /* Atmospheric power lines and a few real rooftops, not decorative HUD. */
 hd_line(p,0,84,239,84,HD_BRICK_DARK);
 hd_line(p,7,79,186,89,HD_STONE_DARK);
 for(int i=0;i<3;i++){
  int x=13+i*99;
  hd_rect(p,x,60,2,55,HD_SHADOW);
  hd_rect(p,x-4,61,9,2,HD_COPPER);
 }
}
static void sidewalk(uint8_t *p,unsigned frame){
 hd_rect(p,0,111,240,49,HD_ROAD);
 hd_rect(p,0,111,240,4,HD_STONE_LIT);
 hd_rect(p,0,115,240,2,HD_STONE_DARK);
 for(int i=-1;i<14;i++){
  int x=i*23+(i*7%11);
  hd_line(p,x,119,x-8,159,HD_ROAD_COOL);
  hd_line(p,x+2,117,x+2,121,HD_MORTAR);
 }
 for(int y=122;y<160;y+=9)
  for(int i=0;i<12;i++){
   int x=i*21+((int)hash(i,y)%7);
   hd_rect(p,x,y,8,1,HD_ROAD_LIGHT);
   hd_pixel(p,x+3,y+3,HD_REFLECT);
  }
 /* The bus stop's actual lamp, stable in every exterior scene. */
 hd_rect(p,213,42,3,74,HD_INK);
 hd_rect(p,209,42,12,3,HD_COPPER);
 hd_rect(p,210,45,10,4,HD_LAMP);
 hd_rect(p,211,116,9,2,HD_SHADOW);
 for(int i=0;i<3;i++){
  int x=202+i*7;
  hd_line(p,x,122,x-4,137,i==1?HD_REFLECT:HD_ROAD_COOL);
 }
 (void)frame;
}
static void street(uint8_t *p,unsigned frame,int night){
 dusk(p,frame,night);
 hd_rect(p,16,55,53,60,HD_BRICK_DARK);
 for(int y=60;y<110;y+=9){
  hd_line(p,17,y,68,y,HD_BRICK);
  for(int x=23+((y/9)&1)*10;x<67;x+=17)
   hd_line(p,x,y,x,y+7,HD_BRICK);
 }
 hd_rect(p,23,70,17,26,HD_SHADOW);
 hd_rect(p,25,72,13,22,HD_GLASS);
 hd_rect(p,47,70,15,26,HD_SHADOW);
 hd_rect(p,49,72,11,22,HD_GLASS_LIT);
 hd_line(p,31,73,31,94,HD_COPPER);
 hd_line(p,54,73,54,94,HD_COPPER);
 hd_rect(p,17,98,51,4,HD_STONE_LIT);
 hd_rect(p,0,104,240,8,HD_BRICK_DARK);
 sidewalk(p,frame);
}
static void bus(uint8_t *p,int x,int y,unsigned t){
 /* Larger authored bus with window muntins, reflectors and shadows. */
 hd_ellipse(p,x+56,y+40,62,5,HD_SHADOW);
 hd_rect(p,x,y,112,39,HD_RED_SHADE);
 hd_rect(p,x+3,y+2,106,29,HD_RED);
 hd_rect(p,x+6,y+7,97,17,HD_GLASS);
 for(int k=0;k<4;k++){
  int xx=x+7+k*25;
  hd_rect(p,xx,y+7,1,18,HD_COPPER);
  hd_rect(p,xx+3,y+9,15,2,HD_GLASS_LIT);
 }
 hd_rect(p,x+63,y+25,2,13,HD_BRICK_LIT);
 hd_rect(p,x+1,y+31,108,5,HD_BRICK_DARK);
 hd_rect(p,x+6,y+33,102,1,HD_COPPER);
 hd_ellipse(p,x+24,y+40,8,8,HD_INK);
 hd_ellipse(p,x+85,y+40,8,8,HD_INK);
 hd_ellipse(p,x+24,y+40,4,4,HD_STONE);
 hd_ellipse(p,x+85,y+40,4,4,HD_STONE);
 hd_rect(p,x+104,y+23,6,5,(t/32u)&1u?HD_LAMP:HD_PLASTER_LIT);
 hd_rect(p,x+9,y+29,18,2,HD_LAMP);
}
/* All narrative characters share one connected skeletal renderer. */
static void actor(uint8_t *p,int x,int feet,int who,unsigned t,int emotion){
 hd_actor_stage(p,x,feet,who,t,emotion);
}

/* POTHEADS OPENING
   The same location as town cell (21,11), seen through a descending camera.
   GBA-friendly pasted-paper art: no video frames, heap, extra framebuffer or
   new OBJ tiles. The choreography is an authored eight-pose rotoscope-ready
   timeline; a source-video trace can replace its joint measurements later. */
static int pot_pt(int center,int coord,int zoom){
 return center+(coord*zoom+(coord>=0?128:-128))/256;
}
static int pot_size(int size,int zoom){
 int v=(size*zoom+128)/256;return v<1?1:v;
}
static void pot_rect(uint8_t *p,int cx,int cy,int z,
                     int x,int y,int w,int h,unsigned ink){
 int xx=pot_pt(cx,x-120,z), yy=pot_pt(cy,y-80,z);
 hd_rect(p,xx,yy,pot_size(w,z),pot_size(h,z),ink);
}
static void pot_line(uint8_t *p,int cx,int cy,int z,
                     int x,int y,int xx,int yy,unsigned ink){
 hd_line(p,pot_pt(cx,x-120,z),pot_pt(cy,y-80,z),
         pot_pt(cx,xx-120,z),pot_pt(cy,yy-80,z),ink);
}
static void pot_city(uint8_t *p,unsigned t,int z,int cx,int cy){
 hd_rect(p,0,0,240,160,HD_STONE_DARK);
 for(int y=0;y<102;y+=7)
  hd_rect(p,0,y,240,1,y&1?HD_ROAD_COOL:HD_STONE);
 /* Behind the location: compressed roof rows retain the bird's-eye
    footprint while the camera closes in toward its street-level awning. */
 for(int i=0;i<7;i++){
  int x=i*43-85,roof=16+(i*17)%15;
  pot_rect(p,cx,cy,z,x,-32,41,roof+40,HD_BRICK_DARK);
  pot_line(p,cx,cy,z,x,-32,x+20,-44,HD_STONE_LIT);
  pot_line(p,cx,cy,z,x+20,-44,x+41,-32,HD_COPPER);
  for(int j=0;j<3;j++)
   pot_rect(p,cx,cy,z,x+6+j*12,9,5,7,
            ((i+j+(int)(t/75u))&3)?HD_GLASS:HD_LAMP);
 }
 /* Pavement perspective: diagonals converge on Danny, never on the HUD. */
 pot_rect(p,cx,cy,z,-25,100,293,65,HD_ROAD_COOL);
 pot_rect(p,cx,cy,z,-25,98,293,3,HD_COPPER);
 /* The canopy casts an actual contact shadow across a lit sidewalk.
    A restrained, repeated wet-stone reflection ties storefront and ground. */
 pot_rect(p,cx,cy,z,43,101,170,4,HD_SHADOW);
 for(int i=0;i<8;i++){
  int x=47+i*22;
  pot_rect(p,cx,cy,z,x,108+(i&1)*3,12,1,HD_REFLECT);
  pot_rect(p,cx,cy,z,x+3,112+(i&1)*3,5,1,HD_STONE_LIT);
 }
 for(int i=0;i<11;i++){
  int x=-34+i*31;
  pot_line(p,cx,cy,z,x,101,x-19,161,HD_ROAD);
  pot_line(p,cx,cy,z,x,123,x+24,123,HD_STONE);
 }
 /* Three-dimensional facade / pitched roof / attached business sign. */
 pot_rect(p,cx,cy,z,38,18,174,83,HD_INK);
 pot_rect(p,cx,cy,z,42,21,166,77,HD_BRICK_DARK);
 for(int y=31;y<97;y+=8){
  pot_line(p,cx,cy,z,43,y,207,y,HD_BRICK);
  for(int x=49+(y/8%2)*11;x<206;x+=22)
   pot_line(p,cx,cy,z,x,y,x,y+7,HD_STONE_DARK);
 }
 pot_rect(p,cx,cy,z,34,15,185,10,HD_STONE_DARK);
 pot_line(p,cx,cy,z,34,15,120,-2,HD_STONE_LIT);
 pot_line(p,cx,cy,z,120,-2,219,15,HD_STONE_LIT);
 for(int i=0;i<8;i++){
  int roofx=39+i*23;
  pot_line(p,cx,cy,z,roofx,15,68+i*16,3,HD_COPPER);
  pot_rect(p,cx,cy,z,roofx+3,12,13,1,HD_ROAD_LIGHT);
 }
 /* A single coherent masonry ledge, awning brackets and drainage. */
 pot_rect(p,cx,cy,z,42,27,167,2,HD_BRICK_LIT);
 pot_line(p,cx,cy,z,42,28,42,97,HD_STONE_DARK);
 pot_line(p,cx,cy,z,207,28,207,97,HD_SHADOW);
 /* Forest-green shop fascia is built INTO the brickwork. */
 pot_rect(p,cx,cy,z,72,30,116,25,HD_INK);
 pot_rect(p,cx,cy,z,75,32,110,21,HD_LEAF_DARK);
 pot_rect(p,cx,cy,z,77,34,106,17,HD_LEAF);
 pot_rect(p,cx,cy,z,80,37,100,11,HD_BRICK_DARK);
 if(z>=184)
  hd_text(p,pot_pt(cx,81-120,z),pot_pt(cy,39-80,z),
          "POTHEADS",HD_PAPER,1,1);
 else
  for(int i=0;i<8;i++)
   pot_rect(p,cx,cy,z,102+i*7,39,4,6,HD_PAPER);
 /* Window frames, inventory silhouettes, real door and hanging canopy. */
 for(int i=0;i<2;i++){
  int x=i?145:59;
  pot_rect(p,cx,cy,z,x,67,52,31,HD_INK);
  pot_rect(p,cx,cy,z,x+3,70,46,25,HD_GLASS);
  pot_rect(p,cx,cy,z,x+6,72,15,17,HD_LEAF_DARK);
  pot_rect(p,cx,cy,z,x+25,75,17,14,HD_STONE);
  pot_line(p,cx,cy,z,x+25,69,x+25,96,HD_COPPER);
  pot_line(p,cx,cy,z,x+2,91,x+50,91,HD_COPPER);
 }
 pot_rect(p,cx,cy,z,114,65,26,35,HD_INK);
 pot_rect(p,cx,cy,z,118,69,18,30,HD_GLASS_LIT);
 pot_rect(p,cx,cy,z,119,71,16,28,HD_GLASS);
 pot_line(p,cx,cy,z,127,70,127,99,HD_COPPER);
 pot_rect(p,cx,cy,z,131,84,2,2,HD_GOLD);
 pot_rect(p,cx,cy,z,52,57,153,7,HD_LEAF_DARK);
 for(int x=55;x<200;x+=18){
  pot_rect(p,cx,cy,z,x,59,10,4,HD_LEAF_LIT);
  pot_line(p,cx,cy,z,x+10,60,x+15,62,HD_LEAF_DARK);
 }
 pot_rect(p,cx,cy,z,51,65,154,2,HD_COPPER);
 pot_line(p,cx,cy,z,54,65,47,101,HD_STONE);
 pot_line(p,cx,cy,z,202,65,209,101,HD_STONE);
 /* A window-local OPEN light, rather than floating interface lettering. */
 if(z>=218){
  hd_rect(p,pot_pt(cx,156-120,z),pot_pt(cy,77-80,z),27,11,HD_LEAF_DARK);
  hd_text(p,pot_pt(cx,159-120,z),pot_pt(cy,79-80,z),
          "OPEN",HD_LAMP,1,0);
 }
 pot_line(p,cx,cy,z,45,101,211,101,HD_PLASTER_LIT);
 pot_line(p,cx,cy,z,43,103,214,103,HD_SHADOW);
 /* Two tiny street details make the sign job feel inhabited, not staged. */
 pot_rect(p,cx,cy,z,202,80,3,21,HD_BARK);
 pot_rect(p,cx,cy,z,198,78,11,5,HD_GOLD);
 pot_rect(p,cx,cy,z,35,90,10,10,HD_BARK);
 pot_rect(p,cx,cy,z,37,87,6,4,HD_LEAF);
 /* The job is somebody's ordinary afternoon: the shop is lit, the
    sidewalk is used, and nothing is magically suspended in space. */
 for(int i=0;i<2;i++){
  int bx=17+i*216+((int)((t/8u)%41u))*(i?-1:1);
  int yy=94+i*2;
  pot_rect(p,cx,cy,z,bx-7,yy+3,15,2,HD_SHADOW);
  pot_rect(p,cx,cy,z,bx-4,yy-15,9,15,i?HD_GLASS:HD_BRICK);
  pot_rect(p,cx,cy,z,bx-3,yy-20,7,6,HD_SKIN);
  pot_line(p,cx,cy,z,bx-2,yy,bx-5+((t/12u)&1u)*3,yy+3,HD_INK);
  pot_line(p,cx,cy,z,bx+2,yy,bx+5-((t/12u)&1u)*3,yy+3,HD_INK);
 }
}
typedef struct {
 signed char lean,bob,lx,ly,rx,ry,lf,lh,rf,rh;
} PotPose;
/* Frame-to-frame hand, heel and body measurements may be replaced by
   exact reference-video rotoscope samples without altering the renderer. */
static const PotPose pot_dance[8]={
 { 0, 0,-21,-29,26,-43,-10, 0, 10, 0},
 { 3,-2,-27,-35,30,-46,-14,-5, 13, 0},
 { 7,-3,-17,-46,32,-43, -5, 0, 18,-6},
 { 6, 0,-27,-24,28,-37,-13, 0, 11, 0},
 { 0,-2,-29,-37,26,-45,-17,-5,  9, 0},
 {-6,-3,-19,-45,25,-44, -8, 0, 18,-6},
 {-7, 0,-29,-26,28,-40,-14, 0, 10, 0},
 {-2,-1,-18,-32,26,-44,-11,-4, 13, 0}
};
static int pot_lerp(int a,int b,unsigned m){
 /* Hermite ease: the sampled pose lands without a one-frame snap.
    Fixed-point integer arithmetic keeps the exact same joint geometry on
    host and GBA and does not evaluate floating point on every frame. */
 unsigned f=m*256u/12u;
 unsigned smooth=(f*f*(768u-2u*f))>>16;
 return a+((b-a)*(int)smooth)/256;
}
static PotPose pot_motion(unsigned t){
 if(t<76u)return pot_dance[0];
 unsigned beat=(t-76u)/12u,i=beat&7u,f=(t-76u)%12u;
 PotPose a=pot_dance[i],b=pot_dance[(i+1u)&7u];
 PotPose r={
  (signed char)pot_lerp(a.lean,b.lean,f),
  (signed char)pot_lerp(a.bob,b.bob,f),
  (signed char)pot_lerp(a.lx,b.lx,f),
  (signed char)pot_lerp(a.ly,b.ly,f),
  (signed char)pot_lerp(a.rx,b.rx,f),
  (signed char)pot_lerp(a.ry,b.ry,f),
  (signed char)pot_lerp(a.lf,b.lf,f),
  (signed char)pot_lerp(a.lh,b.lh,f),
  (signed char)pot_lerp(a.rf,b.rf,f),
  (signed char)pot_lerp(a.rh,b.rh,f)
 };
 return r;
}
static int pot_local(int origin,int v,int scale){
 return origin+(v*scale+(v>=0?128:-128))/256;
}
static void pot_bline(uint8_t *p,int x,int foot,int scale,
                      int ax,int ay,int bx,int by,unsigned ink){
 hd_line(p,pot_local(x,ax,scale),pot_local(foot,ay,scale),
         pot_local(x,bx,scale),pot_local(foot,by,scale),ink);
}
static void pot_brect(uint8_t *p,int x,int foot,int scale,
                      int px,int py,int w,int h,unsigned ink){
 hd_rect(p,pot_local(x,px,scale),pot_local(foot,py,scale),
         pot_size(w,scale),pot_size(h,scale),ink);
}
static void pot_bellipse(uint8_t *p,int x,int foot,int scale,
                         int px,int py,int rx,int ry,unsigned ink){
 hd_ellipse(p,pot_local(x,px,scale),pot_local(foot,py,scale),
            pot_size(rx,scale),pot_size(ry,scale),ink);
}
/* Deliberately irregular painted characters, not one mechanically
   perfect screen-font strip. No additional font atlas or ROM image. */
static void pot_handletter(uint8_t *p,int x,int y,const char *word,
                           unsigned ink){
 for(int i=0;word[i]&&i<9;i++){
  char letter[2]={word[i],0};
  hd_text(p,x+i*6,y+(i*7+1)%3-1,letter,ink,1,0);
 }
}
static void pot_banana(uint8_t *p,int cx,int cy,int z,unsigned t){
 PotPose a=pot_motion(t);
 int x=pot_pt(cx,98-120,z),foot=pot_pt(cy,101-80,z);
 int sc=z; /* every limb and paper corner uses one shared camera anchor */
 pot_bellipse(p,x,foot,sc,0,1,20,3,HD_SHADOW);
 /* Feet stay fixed to the sidewalk; small lifted heels describe steps. */
 pot_bline(p,x,foot,sc,-5,-17,a.lf,a.lh-2,HD_INK);
 pot_bline(p,x,foot,sc, 5,-17,a.rf,a.rh-2,HD_INK);
 pot_brect(p,x,foot,sc,a.lf-6,a.lh-3,13,4,HD_INK);
 pot_brect(p,x,foot,sc,a.rf-6,a.rh-3,13,4,HD_INK);
 /* A tall, asymmetrical curved BANANA rather than an egg-shaped mascot.
    The central face aperture is inset into the yellow outer shell. */
 pot_bellipse(p,x,foot,sc,a.lean/2+2,-37+a.bob,16,23,HD_INK);
 pot_bellipse(p,x,foot,sc,a.lean/2+2,-37+a.bob,13,21,HD_GOLD);
 pot_bellipse(p,x,foot,sc,a.lean/2-5,-36+a.bob,4,17,HD_LAMP);
 pot_bline(p,x,foot,sc,a.lean/2-10,-57+a.bob,
           a.lean/2-16,-43+a.bob,HD_INK);
 pot_bline(p,x,foot,sc,a.lean/2-16,-43+a.bob,
           a.lean/2-13,-26+a.bob,HD_INK);
 pot_bline(p,x,foot,sc,a.lean/2-13,-26+a.bob,
           a.lean/2-4,-15+a.bob,HD_INK);
 pot_bline(p,x,foot,sc,a.lean/2+11,-59+a.bob,
           a.lean/2+18,-44+a.bob,HD_INK);
 pot_bline(p,x,foot,sc,a.lean/2+18,-44+a.bob,
           a.lean/2+15,-27+a.bob,HD_INK);
 pot_bline(p,x,foot,sc,a.lean/2+15,-27+a.bob,
           a.lean/2+6,-16+a.bob,HD_INK);
 pot_bline(p,x,foot,sc,a.lean/2+11,-58+a.bob,
           a.lean/2+16,-41+a.bob,HD_LAMP);
 pot_bline(p,x,foot,sc,a.lean/2+16,-41+a.bob,
           a.lean/2+12,-28+a.bob,HD_LAMP);
 /* Seamed, asymmetrical banana silhouette. The dark seam tracks the
    connected torso, rather than wobbling independently of the skeleton. */
 pot_bline(p,x,foot,sc,a.lean/2+8,-50+a.bob,
           a.lean/2+12,-27+a.bob,HD_BARK);
 pot_bline(p,x,foot,sc,a.lean/2+12,-27+a.bob,
           a.lean/2+5,-17+a.bob,HD_GOLD);
 pot_bline(p,x,foot,sc,a.lean/2-11,-31+a.bob,
           a.lean/2-8,-19+a.bob,HD_LAMP);
 pot_brect(p,x,foot,sc,a.lean/2-2,-23+a.bob,4,2,HD_COPPER);
 pot_bline(p,x,foot,sc,-8+a.lean/2,-49+a.bob,
           -18+a.lean/2,-41+a.bob,HD_PAPER);
 pot_bline(p,x,foot,sc, 8+a.lean/2,-48+a.bob,
            15+a.lean/2,-42+a.bob,HD_LAMP);
 pot_bellipse(p,x,foot,sc,a.lean,-56+a.bob,11,13,HD_INK);
 pot_bellipse(p,x,foot,sc,a.lean,-56+a.bob,9,12,HD_LAMP);
 pot_bellipse(p,x,foot,sc,a.lean,-54+a.bob,6,7,HD_SKIN_LIT);
 /* Curled dark stem makes the fruit shape legible at 1x LCD scale. */
 pot_bline(p,x,foot,sc,a.lean-3,-66+a.bob,a.lean-8,-75+a.bob,HD_BARK);
 pot_bline(p,x,foot,sc,a.lean-8,-75+a.bob,a.lean-4,-77+a.bob,HD_INK);
 pot_bline(p,x,foot,sc,a.lean-1,-68+a.bob,a.lean+3,-72+a.bob,HD_GOLD);
 /* A little sweat and a neutral, weathered expression: he's working,
    not delighted with the suit. The face remains recognizable in profile. */
 pot_bline(p,x,foot,sc,a.lean-5,-60+a.bob,
           a.lean-2,-59+a.bob,HD_BARK);
 pot_bline(p,x,foot,sc,a.lean+2,-59+a.bob,
           a.lean+5,-60+a.bob,HD_BARK);
 pot_brect(p,x,foot,sc,a.lean-4,-57+a.bob,2,2,HD_INK);
 pot_brect(p,x,foot,sc,a.lean+3,-57+a.bob,2,2,HD_INK);
 pot_bline(p,x,foot,sc,a.lean-2,-50+a.bob,a.lean+3,-51+a.bob,HD_BRICK_DARK);
 pot_brect(p,x,foot,sc,a.lean+8,-59+a.bob,1,3,HD_SKIN);
 /* The sign arm is visibly connected: the paper overlay never floats. */
 pot_bline(p,x,foot,sc,-9,-44+a.bob,a.lx,a.ly,HD_INK);
 pot_bline(p,x,foot,sc,-8,-43+a.bob,a.lx,a.ly,HD_GOLD);
 pot_bline(p,x,foot,sc, 9,-44+a.bob,a.rx,a.ry,HD_INK);
 pot_bline(p,x,foot,sc,10,-43+a.bob,a.rx,a.ry,HD_GOLD);
 pot_bellipse(p,x,foot,sc,a.lx,a.ly,3,3,HD_PAPER);
 int bx=pot_local(x,a.rx+3,sc),by=pot_local(foot,a.ry-21,sc);
 int bw=pot_size(56,sc),bh=pot_size(34,sc);
 /* Screenprinted cardboard, deliberately superimposed like a 2-D
    collage, but attached to Danny's grip and shadowed into the shot. */
 hd_rect(p,bx+pot_size(3,sc),by+pot_size(3,sc),bw,bh,HD_SHADOW);
 hd_rect(p,bx-1,by-1,bw+2,bh+2,HD_INK);
 hd_rect(p,bx,by,bw,bh,HD_PAPER);
 hd_rect(p,bx+1,by+1,bw-2,pot_size(3,sc),HD_GOLD);
 /* Scuffed corrugation, inked corners and a subtle paper-cut edge make
    the sign clearly superimposed without losing its physical grip. */
 for(int i=1;i<4;i++)
  hd_line(p,bx+2,by+bh-2-i*3,bx+4,by+bh-3-i*3,HD_STONE_LIT);
 hd_line(p,bx,by+bh-1,bx+bw-1,by+bh-2,HD_BARK);
 hd_line(p,bx+bw-2,by+3,bx+bw-2,by+bh-4,HD_COPPER);
 hd_pixel(p,bx+2,by+bh-3,HD_RED_SHADE);
 hd_pixel(p,bx+bw-4,by+3,HD_RED_SHADE);
 hd_line(p,bx+1,by+1,bx+1,by+bh-2,HD_COPPER);
 if(z>=215){
  hd_rect(p,bx+5,by+5,bw-10,8,HD_LAMP);
  pot_handletter(p,bx+5,by+5,"POTHEADS",HD_INK);
  pot_handletter(p,bx+8,by+18,"COME IN",HD_INK);
  hd_line(p,bx+7,by+14,bx+43,by+13,HD_COPPER);
  hd_pixel(p,bx+42,by+17,HD_RED_SHADE);
  hd_line(p,bx+43,by+26,bx+49,by+26,HD_RED_SHADE);
  hd_line(p,bx+46,by+23,bx+49,by+26,HD_RED_SHADE);
  hd_line(p,bx+46,by+29,bx+49,by+26,HD_RED_SHADE);
  hd_rect(p,bx+4,by+1,7,3,HD_STONE_LIT); /* masking tape */
  hd_rect(p,bx+bw-11,by+bh-3,7,2,HD_STONE_LIT);
 }else{
  for(int j=0;j<3;j++)
   hd_rect(p,bx+pot_size(6,sc),by+pot_size(6+j*8,sc),
           bw-pot_size(17,sc),pot_size(2,sc),j==1?HD_BARK:HD_INK);
 }
 /* Grip is in FRONT of the sign, and its shoulder stays behind the board. */
 pot_bellipse(p,x,foot,sc,a.rx,a.ry,3,3,HD_PAPER);
 pot_bline(p,x,foot,sc,a.rx,a.ry,a.rx+5,a.ry-2,HD_INK);
}

/* A genuinely overhead establishing view of the same Potheads block.
   It reveals the storefront as the camera cranes down, using a bounded
   scanline wipe instead of a second framebuffer or per-pixel alpha blend. */
static void pot_air_rect(uint8_t *p,int top,int x,int y,int w,int h,unsigned ink){
 if(y<0){h+=y;y=0;}
 if(y+h>top)h=top-y;
 if(h>0&&w>0)hd_rect(p,x,y,w,h,ink);
}
static void pot_air_scale(uint8_t *p,int top,int z,
                          int x,int y,int w,int h,unsigned ink){
 pot_air_rect(p,top,pot_pt(120,x-120,z),pot_pt(80,y-80,z),
              pot_size(w,z),pot_size(h,z),ink);
}
static void pot_aerial(uint8_t *p,unsigned t){
 if(t>=98u)return;
 /* The roof/ground split sweeps upward as the camera tilts toward street
    level. At frame 0 it is an actual top-down street, not a tiny facade. */
 int top=t<46u?160:160-ease(0,160,t,46,52);
 if(top<=0)return;
 int z=ease(168,276,t,0,98);
 pot_air_rect(p,top,0,0,240,160,HD_ROAD_COOL);
 for(int i=0;i<14;i++){
  int x=i*21-16;
  pot_air_rect(p,top,x,0,2,160,HD_ROAD);
  pot_air_rect(p,top,0,i*13,240,1,HD_STONE_DARK);
 }
 /* Cross street and pavements are measured around a single central
    roof: neighboring blocks are contiguous, never random voxels. */
 pot_air_scale(p,top,z,-20,94,280,15,HD_STONE);
 pot_air_scale(p,top,z,-20,109,280,61,HD_ROAD);
 pot_air_scale(p,top,z,-20,92,280,3,HD_PLASTER_LIT);
 for(int i=0;i<10;i++){
  pot_air_scale(p,top,z,13+i*26,142,13,2,HD_STONE_LIT);
  pot_air_scale(p,top,z,14+i*25,101,12,1,HD_ROAD_LIGHT);
 }
 for(int i=0;i<3;i++){
  int bx=i==0?-38:i==1?181:4;
  int by=i==2?-47:12;
  pot_air_scale(p,top,z,bx+3,by+4,64,70,HD_SHADOW);
  pot_air_scale(p,top,z,bx,by,64,70,HD_BRICK_DARK);
  pot_air_scale(p,top,z,bx+3,by+3,57,64,i==2?HD_STONE:HD_BRICK);
  for(int v=0;v<4;v++){
   pot_air_scale(p,top,z,bx+8+v*13,by+8,7,15,HD_ROAD_COOL);
   pot_air_scale(p,top,z,bx+8+v*13,by+38,7,9,HD_GLASS);
  }
  pot_air_scale(p,top,z,bx+8,by+53,48,3,HD_COPPER);
 }
 /* The Potheads roof, light-wells, sign-facing green parapet and curb. */
 pot_air_scale(p,top,z,75,30,104,68,HD_INK);
 pot_air_scale(p,top,z,79,34,96,58,HD_BRICK_DARK);
 for(int k=0;k<7;k++){
  pot_air_scale(p,top,z,83+k*13,40,7,42,
                k&1?HD_BRICK:HD_STONE_DARK);
  pot_air_scale(p,top,z,85+k*13,37,3,2,HD_COPPER);
 }
 pot_air_scale(p,top,z,83,68,88,9,HD_STONE);
 pot_air_scale(p,top,z,91,71,22,3,HD_ROAD_LIGHT);
 pot_air_scale(p,top,z,137,71,26,3,HD_ROAD_LIGHT);
 pot_air_scale(p,top,z,78,88,99,8,HD_LEAF_DARK);
 pot_air_scale(p,top,z,82,91,91,3,HD_LEAF);
 /* At this altitude the banana suit is a tiny yellow silhouette with
    a matching pavement shadow and a separate paper sign. */
 pot_air_scale(p,top,z,110,111,9,3,HD_SHADOW);
 pot_air_scale(p,top,z,113,104,5,8,HD_GOLD);
 pot_air_scale(p,top,z,113,103,3,3,HD_LAMP);
 pot_air_scale(p,top,z,119,105,12,7,HD_PAPER);
 pot_air_scale(p,top,z,120,107,10,1,HD_BARK);
 if(top<160)pot_air_rect(p,top,0,top-2,240,2,HD_COPPER);
}

static void potheads_open(uint8_t *p,unsigned t){
 /* 0-132: a continuous bird's-eye descent; then an unbroken dance.
    Page advance never resets the camera or sign's position. */
 int z=ease(86,256,t,0,132);
 int cx=ease(97,120,t,0,132),cy=ease(106,80,t,0,132);
 pot_city(p,t,z,cx,cy);
 pot_banana(p,cx,cy,z,t);
 pot_aerial(p,t); /* overhead-to-street tilt, no extra canvas */
}

static void cafe(uint8_t *p,unsigned t){
 street(p,t,1);
 hd_rect(p,4,42,102,70,HD_BRICK_DARK);
 for(int y=48;y<109;y+=8){
  hd_line(p,5,y,103,y,HD_BRICK);
  for(int x=12+(y/8%2)*8;x<102;x+=19)
   hd_line(p,x,y,x,y+6,HD_BRICK);
 }
 hd_rect(p,8,57,94,10,HD_GOLD);
 hd_rect(p,11,60,88,5,HD_BRICK_DARK);
 hd_text(p,19,59,"MARA'S CAFE",HD_PAPER,1,1);
 for(int i=0;i<2;i++){
  int x=14+i*42;
  hd_rect(p,x,71,35,35,HD_INK);
  hd_rect(p,x+3,74,29,28,HD_GLASS_LIT);
  hd_rect(p,x+3,96,29,6,HD_COPPER);
  hd_rect(p,x+16,74,2,27,HD_BRICK_DARK);
  hd_rect(p,x+4,87,27,2,HD_COPPER);
 }
 hd_line(p,8,110,105,110,HD_PLASTER_LIT);
 /* Café shade, textured awning, lit service interior and rain reflections. */
 hd_rect(p,5,67,98,4,HD_BARK);
 for(int k=0;k<11;k++)
  hd_rect(p,7+k*9,68,5,3,(k&1)?HD_GOLD:HD_RED_SHADE);
 hd_rect(p,17,78,23,5,HD_STONE_LIT);
 hd_rect(p,59,77,21,4,HD_STONE_LIT);
 hd_rect(p,27,84,8,12,HD_LEAF_DARK);
 hd_rect(p,67,82,7,15,HD_BARK);
 hd_rect(p,73,85,5,5,HD_SKIN); /* silhouette behind the glass */
 hd_rect(p,72,90,7,12,HD_STONE_DARK);
 for(int i=0;i<3;i++)
  hd_line(p,18+i*28,116,30+i*28,118,HD_REFLECT);
}
static void table(uint8_t *p,int x,int y,unsigned t){
 hd_ellipse(p,x,y+21,36,6,HD_SHADOW);
 hd_rect(p,x-31,y,62,6,HD_BARK);
 hd_rect(p,x-28,y+1,56,3,HD_COPPER);
 hd_rect(p,x-25,y+6,5,19,HD_BARK);
 hd_rect(p,x+19,y+6,5,19,HD_BARK);
 for(int i=-1;i<=1;i++){
  int xx=x+i*19;
  hd_rect(p,xx-6,y-10,12,10,HD_PAPER);
  hd_rect(p,xx-6,y-10,12,2,HD_GOLD);
  hd_rect(p,xx-6,y-11,12,1,HD_COPPER);
  if((t/80u+(unsigned)(i+2))%3u==0u)
   hd_pixel(p,xx,y-13,HD_PLASTER_LIT);
 }
 hd_ellipse(p,x-43,y+25,11,3,HD_SHADOW);
 hd_ellipse(p,x+43,y+25,11,3,HD_SHADOW);
 hd_rect(p,x-47,y+8,9,2,HD_BARK);
 hd_rect(p,x+39,y+8,9,2,HD_BARK);
 hd_rect(p,x+9,y-3,16,8,HD_PAPER); /* receipt */
 hd_line(p,x+11,y,x+22,y,HD_STONE_DARK);
 hd_pixel(p,x+20,y+2,HD_RED_SHADE);
}
static void interior(uint8_t *p,unsigned t,int variant){
 unsigned wall=variant==1?HD_STONE_DARK:variant==2?HD_BRICK_DARK:HD_ROAD_COOL;
 hd_rect(p,0,0,HD_W,112,wall);
 for(int x=0;x<HD_W;x+=38)hd_rect(p,x,0,2,111,HD_BRICK);
 hd_rect(p,0,110,HD_W,50,HD_STONE_DARK);
 for(int y=111;y<160;y+=10)
  hd_line(p,0,y,239,y,(y&1)?HD_STONE:HD_BRICK_DARK);
 hd_rect(p,15,17,90,73,HD_SHADOW);
 hd_rect(p,18,20,84,67,HD_GLASS);
 for(int i=0;i<7;i++){
  int xx=19+i*12,h=20+(int)(hash(i,39)%19u);
  hd_rect(p,xx,86-h,12,h,HD_STONE_DARK);
  hd_rect(p,xx+2,86-h+6,3,3,HD_LAMP);
 }
 hd_rect(p,59,20,3,69,HD_COPPER);
 hd_rect(p,18,64,85,2,HD_COPPER);
 hd_rect(p,14,87,93,4,HD_BRICK_DARK);
 hd_rect(p,140,25,85,6,HD_SHADOW);
 hd_rect(p,141,25,83,2,HD_GOLD);
 (void)t;
}
static void archive(uint8_t *p,unsigned t,unsigned page){
 interior(p,t,0);
 hd_rect(p,144,37,77,63,HD_INK);
 for(int i=0;i<7;i++){
  hd_rect(p,149+i*10,43,7,32,HD_BRICK);
  hd_rect(p,149+i*10,46,7,2,HD_GOLD);
  hd_rect(p,149+i*10,78,7,3,HD_STONE_LIT);
 }
 hd_rect(p,118,93,119,9,HD_BARK);
 hd_rect(p,127,99,7,33,HD_BARK);
 hd_rect(p,213,99,7,33,HD_BARK);
 hd_rect(p,134,86,32,11,HD_PAPER);
 hd_rect(p,138,89,23,1,HD_STONE_DARK);
 hd_rect(p,141,91,21,1,HD_STONE_DARK);
 hd_rect(p,182,87,16,12,HD_COPPER);
 hd_rect(p,186,83,8,5,HD_PAPER);
 /* The form, approval stamp and connected clerk hand share a desk. */
 hd_rect(p,150,81,25,13,HD_PAPER);
 hd_rect(p,152,83,18,1,HD_STONE);
 hd_rect(p,152,86,16,1,HD_STONE);
 hd_rect(p,152,89,11,1,HD_STONE);
 if(page>=2u){
  hd_ellipse(p,163,89,5,3,HD_RED_SHADE);
  hd_pixel(p,163,88,HD_GOLD);
 }
 hd_rect(p,178,84,10,7,HD_INK);
 hd_rect(p,180,82,6,5,HD_BARK);
 actor(p,ease(64,76,t,8,60),137,1,t,page==0?2:page>=2?3:1);
 actor(p,193,135,3,t,page&1u?1:3);
 if(page>=1u){
  hd_line(p,176,107,165,97,HD_STONE_LIT);
  hd_line(p,175,106,165,96,HD_PLASTER_LIT);
  hd_ellipse(p,164,96,3,2,HD_SKIN);
 }
 hd_line(p,126,32,237,32,HD_STONE_DARK);
 hd_text(p,143,14,"BORE / THE ARCHIVE",HD_PAPER,1,0);
}
static void record_room(uint8_t *p,unsigned t){
 interior(p,t,1);
 hd_rect(p,127,29,102,71,HD_INK);
 hd_rect(p,130,32,95,64,HD_STONE_DARK);
 for(int i=0;i<6;i++){
  int x=134+i*15;int level=(int)((t/7u+(unsigned)i*3u)%20u);
  if(level>10)level=20-level;
  hd_rect(p,x,66-level,8,level+5,level>5?HD_LAMP:HD_STONE_LIT);
  hd_rect(p,x,81,8,1,HD_STONE);
 }
 hd_rect(p,19,102,208,10,HD_BARK);
 hd_rect(p,26,112,8,35,HD_BARK);
 hd_rect(p,212,112,8,35,HD_BARK);
 hd_ellipse(p,70,100,31,10,HD_INK);
 hd_ellipse(p,70,100,26,8,HD_BRICK_DARK);
 hd_ellipse(p,70,100,20,6,HD_STONE_DARK);
 hd_ellipse(p,70,100,7,4,HD_GOLD);
 int px=(int)(t/7u&3u);
 hd_line(p,70,100,76+px,97,HD_PAPER);
 hd_rect(p,97,80,3,16,HD_COPPER);
 hd_line(p,99,82,87,92,HD_COPPER);
 hd_rect(p,150,87,46,12,HD_BARK);
 hd_rect(p,153,85,40,9,HD_PAPER); /* a real illustrated sleeve */
 hd_rect(p,155,87,13,5,HD_GOLD);
 hd_line(p,171,88,190,88,HD_STONE_DARK);
 hd_line(p,171,90,187,90,HD_STONE);
 hd_rect(p,122,31,3,68,HD_COPPER);
 hd_rect(p,130,28,94,2,HD_STONE_LIT);
 for(int j=0;j<5;j++)
  hd_rect(p,136+j*17,76,11,2,HD_RED_SHADE);
 actor(p,38,136,1,t,t<80?2:3);
}
static void rooftop(uint8_t *p,unsigned t,unsigned page){
 dusk(p,t,0);
 hd_rect(p,0,103,HD_W,57,HD_ROAD);
 hd_rect(p,0,102,HD_W,4,HD_STONE);
 for(int i=0;i<6;i++){
  int x=i*41;
  hd_line(p,x,109,x+14,156,HD_ROAD_COOL);
  hd_rect(p,x,145,36,1,HD_ROAD_LIGHT);
 }
 hd_rect(p,0,75,HD_W,4,HD_COPPER);
 for(int x=12;x<240;x+=27)hd_rect(p,x,77,2,37,HD_COPPER);
 hd_line(p,0,105,239,105,HD_STONE_DARK);
 /* Painting depicts the same city rather than anonymous colored squares. */
 hd_rect(p,151,52,67,51,HD_BARK);
 hd_rect(p,155,56,59,43,HD_PAPER);
 hd_rect(p,158,59,53,32,HD_GLASS);
 hd_ellipse(p,195,67,7,7,HD_LAMP);
 for(int i=0;i<9;i++){
  int x=158+i*6,h=(i*13)%17+7;
  hd_rect(p,x,91-h,6,h,HD_STONE_DARK);
  hd_rect(p,x+2,87-h,2,2,HD_LAMP);
 }
 hd_rect(p,157,90,55,7,HD_LEAF);
 hd_rect(p,169,105,7,35,HD_BARK);
 hd_line(p,171,113,156,147,HD_BARK);
 hd_line(p,175,113,189,147,HD_BARK);
 hd_rect(p,208,135,12,9,HD_BRICK_DARK);
 hd_rect(p,211,130,6,6,HD_RED_SHADE);
 /* Advance to the same canvas edge before touching it. The brush
    extends from the raised hand and pigments appear only on contact. */
 int painter=ease(78,128,t,7,78);
 int reaction=page>=3u&&t>=192u;
 if(reaction)hd_actor_closeup(p,106,143,1,t,5);
 else actor(p,painter,142,1,t,t<92u?2:3);
 if(!reaction&&t>=92u){
  int hand=painter+23;
  hd_line(p,hand,93,hand+8,87,HD_BARK);
  hd_pixel(p,hand+8,87,HD_RED_SHADE);
  int strokes=(int)((t-92u)/32u);
  if(strokes>6)strokes=6;
  for(int j=0;j<strokes;j++){
   int px=160+j*7;
   hd_line(p,px,78-(j&1)*3,px+5,76+(j&1)*3,
           j&1?HD_COPPER:HD_RED_SHADE);
  }
 }
}
static void clinic(uint8_t *p,const Story *s,unsigned t,int last){
 interior(p,t,2);
 hd_rect(p,146,31,77,68,HD_PLASTER);
 hd_rect(p,150,35,69,60,HD_GLASS_LIT);
 hd_line(p,151,64,217,64,HD_STONE);
 hd_line(p,177,36,177,94,HD_STONE);
 hd_rect(p,150,105,77,11,HD_PAPER);
 hd_rect(p,163,102,16,7,HD_STONE);
 hd_rect(p,181,103,29,4,HD_BRICK_DARK);
 hd_rect(p,184,96,21,10,HD_PAPER);
 for(int i=0;i<4;i++)hd_line(p,185,98+i*2,202,98+i*2,HD_ROAD_COOL);
 /* Consultation chart, desk and patient-clinician line of action. */
 hd_rect(p,83,107,123,5,HD_BARK);
 hd_rect(p,87,111,6,27,HD_BARK);
 hd_rect(p,197,111,6,27,HD_BARK);
 hd_rect(p,115,98,39,11,HD_PAPER);
 hd_line(p,120,101,146,101,HD_STONE);
 hd_line(p,120,104,140,104,HD_STONE);
 hd_rect(p,126,93,15,5,HD_COPPER);
 hd_rect(p,142,96,14,8,HD_GLASS_LIT);
 hd_pixel(p,146,98,HD_RED_SHADE);
 hd_rect(p,154,36,60,51,HD_PAPER);
 hd_rect(p,156,38,56,47,HD_GLASS_LIT);
 for(int i=0;i<5;i++)
  hd_line(p,160,47+i*7,207,47+i*7,HD_STONE_LIT);
 hd_line(p,162,75,169,69,HD_RED_SHADE);
 hd_line(p,169,69,178,76,HD_RED_SHADE);
 hd_line(p,178,76,189,61,HD_RED_SHADE);
 /* No unverified living-person appearance while Mara is missing. */
 int missing=(s->rescue_stage>=STORY_RESCUE_MISSING&&
              s->rescue_stage<=STORY_RESCUE_TRACE)||
              s->rescue_stage==STORY_RESCUE_LOST;
 actor(p,68,135,1,t,last?7:1);
 if(last&&!missing)actor(p,115,136,2,t,5);
 if(s->page>=3u&&t>=160u)
  hd_actor_closeup(p,184,142,5,t,1);
 else actor(p,198,133,5,t,1);
 hd_line(p,181,106,157,102,HD_PLASTER_LIT);
 hd_ellipse(p,157,101,3,2,HD_SKIN);
}
void hd_cinema_draw(uint8_t *p,const Story *s,unsigned t){
 unsigned sc=s->cinema,page=s->page;
 switch(sc){
  case CINE_OPEN:
   potheads_open(p,t);
   break;
  case CINE_LETTER:{
   street(p,t,1);
   hd_rect(p,9,56,91,59,HD_BARK);
   hd_rect(p,12,59,85,53,HD_BRICK);
   for(int i=0;i<5;i++)
    hd_line(p,14,67+i*9,96,67+i*9,HD_BRICK_DARK);
   hd_rect(p,18,75,43,37,HD_INK);
   hd_rect(p,21,78,37,31,HD_GLASS_LIT);
   hd_line(p,38,78,38,109,HD_BARK);
   hd_rect(p,65,75,23,36,HD_STONE_DARK);
   hd_rect(p,67,76,20,4,HD_COPPER);
   hd_rect(p,69,81,16,16,HD_SHADOW);
   int dx=ease(174,141,t,10,70);
   /* Same counter and letter for all dialogue pages. */
   hd_rect(p,153,111,70,7,HD_BARK);
   hd_rect(p,155,117,5,29,HD_BARK);
   hd_rect(p,217,117,5,29,HD_BARK);
   hd_rect(p,165,91,24,17,HD_PAPER);
   hd_rect(p,166,92,22,2,HD_STONE_LIT);
   hd_line(p,166,94,177,103,HD_COPPER);
   hd_line(p,188,94,177,103,HD_COPPER);
   hd_rect(p,176,97,3,2,HD_RED_SHADE);
   int reaction=page>=3u&&t>=150u;
   if(reaction)hd_actor_closeup(p,132,143,1,t,7);
   else actor(p,dx,145,1,t,t<85u?2:page==0?1:3);
   if(!reaction&&t>=85u&&page>=1u){
    hd_line(p,dx+23,96,166,97,HD_SKIN);
    hd_pixel(p,166,97,HD_SKIN_LIT);
   }
   break;
  }
  case CINE_BORE:archive(p,t,page);break;
  case CINE_MARA:
  case CINE_STAY:{
   cafe(p,t);
   table(p,121,110,t);
   int danny=ease(78,91,t,11,60);
   int mara=ease(178,164,t,18,63);
   actor(p,danny,141,1,t,t<70?2:page>=1?5:1);
   int reaction=page>=3u&&t>=145u;
   if(reaction)hd_actor_closeup(p,178,145,2,t,5);
   else actor(p,mara,141,2,t,t<82?2:page>=2?3:1);
   /* Body gestures have real table targets instead of floating cups. */
   if(t>=86u&&page>=1u){
    hd_line(p,danny+18,108,114,99,HD_SKIN);
    hd_pixel(p,114,99,HD_SKIN_LIT);
   }
   if(!reaction&&t>=104u&&page>=2u){
    hd_line(p,mara-18,108,141,99,HD_SKIN);
    hd_pixel(p,141,99,HD_SKIN_LIT);
   }
   /* Noninteractive pedestrians keep the cafe block lived-in. */
   int passer=235-(int)((t/5u)%80u);
   hd_ellipse(p,passer,113,7,2,HD_SHADOW);
   hd_rect(p,passer-3,93,7,15,HD_STONE_DARK);
   hd_rect(p,passer-2,89,5,6,HD_SKIN);
   break;
  }
  case CINE_BUS:{
   street(p,t,1);
   /* Arrival, settle and opening door are sequential physical events. */
   int bx=ease(260,74,t,0,116);
   bus(p,bx,67,t);
   hd_rect(p,201,68,30,19,HD_INK); /* bus route timetable */
   hd_rect(p,203,70,26,14,HD_PAPER);
   hd_line(p,207,74,224,74,HD_STONE);
   hd_line(p,207,78,220,78,HD_STONE);
   hd_rect(p,200,88,3,25,HD_BARK);
   if(t>=116u){
    int gap=ease(0,12,t,116,36);
    hd_rect(p,bx+66,92,20,12,HD_INK);
    hd_rect(p,bx+68,94,16,9,HD_STONE_DARK);
    hd_rect(p,bx+68,94,8-gap/3,9,HD_RED);
    hd_rect(p,bx+84-gap/3,94,2,9,HD_COPPER);
    hd_rect(p,bx+85,103,4,9,HD_STONE_LIT);
    hd_line(p,bx+68,113,bx+92,113,HD_LAMP);
   }
   int danny=ease(192,165,t,20,78);
   if(page>=3u&&t>=170u)hd_actor_closeup(p,162,143,1,t,7);
   else actor(p,danny,142,1,t,t<100u?2:page>=2?7:0);
   break;
  }
  case CINE_MUSIC:record_room(p,t);break;
  case CINE_ART:rooftop(p,t,page);break;
  case CINE_NOTICE:{
   street(p,t,1);
   int x=ease(68,91,t,13,71);
   actor(p,x,142,1,t,t<80?2:7);
   /* Phone aligns with Danny's raised hand at (x+19,85). */
   hd_rect(p,x+18,75,11,20,HD_INK);
   hd_rect(p,x+20,77,7,16,HD_GLASS_LIT);
   hd_pixel(p,x+23,80,HD_LAMP);
   hd_line(p,x+19,85,x+20,87,HD_SKIN);
   break;
  }
  case CINE_CLINIC:clinic(p,s,t,0);break;
  case CINE_LAST_CLINIC:clinic(p,s,t,1);break;
  case CINE_FOLLOWUP:
   street(p,t,1);
   hd_rect(p,16,66,102,48,HD_PLASTER);
   hd_rect(p,22,74,90,37,HD_STONE);
   hd_rect(p,57,82,42,29,HD_GLASS);
   hd_text(p,35,69,"CLINIC",HD_INK,1,1);
   hd_rect(p,72,84,2,26,HD_COPPER);
   actor(p,ease(139,156,t,12,74),142,1,t,t<89?2:1);
   actor(p,ease(193,212,t,23,78),142,2,t,t<104?2:5);
   break;
  default:
   if(!hd_special_cinema_draw(p,s,t))street(p,t,1);
   break;
 }
 /* Filmic edge line only; the native subtitle layer owns the readable
    bottom panel. Original scene script governs which line is spoken. */
 hd_rect(p,0,0,240,2,HD_INK);
 hd_rect(p,0,158,240,2,HD_INK);
}
