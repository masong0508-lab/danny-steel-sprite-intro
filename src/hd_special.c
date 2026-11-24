#include "hd_canvas.h"

/* Additional story locations use painted native-pixel compositions rather
   than zoomed 120x80 placeholders. City details are period-appropriate:
   worn floorboards, paper records, windows, analog equipment and familiar
   brick, never holograms or faux high-tech chrome. */
static unsigned hash(unsigned x,unsigned y){
 uint32_t n=(x+11u)*1103515245u+(y+33u)*12345u;
 n^=n>>11;n*=2246822519u;return n^(n>>13);
}
int hd_special_supports(unsigned scene){
 switch(scene){
  case CINE_SERVER:case CINE_WITNESS:case CINE_SAFE:
  case CINE_BACKUP:case CINE_REBRAND:case CINE_MARA_EMPTY:
  case CINE_RESCUE_DOOR:case CINE_RESCUE_SAVED:
  case CINE_GOOD:case CINE_BAD:case CINE_FINALE:case CINE_DAY:
   return 1;
  default:return 0;
 }
}
static void inside(uint8_t *p,unsigned t,int window){
 /* Quiet wall masses, a recessed sash and broad floorboards. Light is
    attached to the room; no uniformly bright window or wireframe floor. */
 hd_rect(p,0,0,240,113,window?HD_BRICK_DARK:HD_STONE_DARK);
 hd_rect(p,0,0,240,7,HD_SHADOW);
 hd_rect(p,0,7,240,2,HD_BARK);
 hd_rect(p,0,89,240,20,HD_BARK);
 hd_rect(p,0,89,240,2,HD_RED_SHADE);
 for(int x=3;x<240;x+=40){
  hd_rect(p,x,93,35,13,HD_BRICK_DARK);
  hd_line(p,x+2,94,x+31,94,HD_BARK);
 }
 hd_rect(p,0,109,240,51,HD_BARK);
 hd_rect(p,0,109,240,3,HD_SHADOW);
 for(int row=0;row<5;row++){
  int y=117+row*9;
  hd_line(p,0,y,239,y,HD_BRICK_DARK);
  int x=18+((row*43)%67);
  for(;x<240;x+=83){
   hd_line(p,x,y-7,x-3,y,HD_SHADOW);
   hd_line(p,x+8,y-3,x+24+(row&1)*9,y-3,HD_BRICK_DARK);
  }
 }
 /* Recess depth, not an illuminated cyan panel. */
 hd_rect(p,12,9,116,94,HD_SHADOW);
 hd_rect(p,15,11,110,88,HD_BARK);
 hd_rect(p,19,16,101,76,HD_ROAD);
 hd_rect(p,19,48,101,44,HD_ROAD_COOL);
 for(int i=0;i<8;i++){
  int x=20+i*13,h=21+(int)(hash((unsigned)i,23u)%32u);
  hd_rect(p,x,92-h,13,h,i&1?HD_STONE_DARK:HD_WET);
  hd_rect(p,x+1,91-h,10,1,HD_ROAD_LIGHT);
  for(int k=5;k<h-3;k+=11)
   if((i+k)%3!=0)hd_rect(p,x+4,92-h+k,2,3,
       (i+k)%7==0?HD_GOLD:HD_GLASS);
 }
 hd_line(p,27,19,45,19,HD_ROAD_COOL);
 hd_line(p,22,23,35,23,HD_ROAD_LIGHT);
 hd_line(p,101,19,116,19,HD_ROAD_COOL);
 hd_rect(p,69,16,4,80,HD_SHADOW);
 hd_rect(p,70,16,2,77,HD_BARK);
 hd_rect(p,19,65,101,4,HD_SHADOW);
 hd_rect(p,19,65,101,1,HD_PLASTER);
 hd_rect(p,15,12,5,85,HD_BARK);
 hd_rect(p,119,12,5,85,HD_SHADOW);
 hd_rect(p,14,95,112,4,HD_PLASTER);
 hd_line(p,17,95,122,95,HD_STONE_LIT);
 hd_rect(p,17,99,107,3,HD_SHADOW);
 hd_line(p,15,11,124,11,HD_COPPER);
 /* Short reflected streaks stay inside the window's floor footprint. */
 for(int i=0;i<4;i++){
  int x=28+i*24;
  hd_line(p,x,123+i*4,x+14,123+i*4,HD_BRICK_DARK);
 }
 (void)t;
}
static void rainy_lane(uint8_t *p,unsigned t){
 /* Three depths of unequal roof masses. Far architecture stays quiet;
    the shop, people and physical streetlamp own the high contrast. */
 hd_rect(p,0,0,240,54,19);
 hd_rect(p,0,54,240,52,HD_ROAD_COOL);
 hd_rect(p,0,106,240,54,HD_ROAD);
 for(int i=0;i<7;i++){
  int x=i*39-13,h=22+(int)(hash((unsigned)i,8u)%29u);
  hd_rect(p,x,85-h,35,h+20,HD_STONE_DARK);
  hd_rect(p,x+6,81-h,18,4,HD_STONE_DARK);
  hd_line(p,x+1,85-h,x+33,85-h,HD_MORTAR);
  for(int yy=92-h;yy<98;yy+=12)
   for(int j=0;j<2;j++)
    if(((i+yy+j)&3)!=1)hd_rect(p,x+6+j*14,yy,3,4,
                                      (i+yy+j)%9==0?HD_GOLD:HD_GLASS);
 }
 hd_rect(p,0,87,29,20,HD_WET);
 hd_rect(p,134,70,44,37,HD_WET);
 hd_rect(p,138,67,35,3,HD_STONE_DARK);
 hd_rect(p,181,82,43,25,HD_BRICK_DARK);
 for(int i=0;i<3;i++){
  int x=141+i*12;
  hd_rect(p,x,77,7,13,HD_SHADOW);
  hd_rect(p,x+1,80,3,6,i==1?HD_COPPER:HD_ROAD_COOL);
 }
 hd_line(p,126,60,239,68,HD_SHADOW);
 hd_line(p,127,62,239,70,HD_ROAD_LIGHT);
 hd_rect(p,0,105,240,4,HD_STONE_DARK);
 hd_line(p,0,106,239,106,HD_ROAD_COOL);
 hd_rect(p,0,110,240,2,HD_WET);
 /* Setts have broken joints and unequal clusters, never a full grid. */
 for(int i=0;i<19;i++){
  int x=(i*47+13)%239,y=118+(i*17)%39;
  hd_line(p,x,y,x+6+(i%4)*3,y,HD_ROAD_LIGHT);
  if(i%3==0)hd_line(p,x-1,y+1,x-2,y+4,HD_WET);
 }
 hd_rect(p,217,50,3,65,HD_INK);
 hd_line(p,219,57,219,109,HD_STONE_DARK);
 hd_rect(p,210,49,16,3,HD_BARK);
 hd_rect(p,212,52,12,5,HD_COPPER);
 hd_rect(p,214,53,8,4,HD_LAMP);
 hd_line(p,215,57,221,57,HD_PAPER);
 hd_rect(p,214,112,9,3,HD_SHADOW);
 /* The pool is under its lamp. Broad midtone, broken wet highlights. */
 hd_ellipse(p,216,128,18,5,HD_ROAD_LIGHT);
 hd_line(p,204,125,224,125,HD_MORTAR);
 hd_line(p,212,128,227,128,HD_COPPER);
 hd_line(p,207,131,218,131,HD_STONE);
 hd_line(p,215,135,221,135,HD_GOLD);
 for(int i=0;i<14;i++){
  int x=((i*43+(int)(hash((unsigned)i,47u)%19u)+
          (int)(t/3u))%256)-8;
  int y=6+((i*17+(int)(t/4u))%79);
  hd_line(p,x,y,x-2,y+5,i%4==0?HD_REFLECT:HD_GLASS_LIT);
 }
 for(int i=0;i<5;i++){
  int x=27+i*46,y=133+(i*5)%18;
  hd_line(p,x-7,y,x+6,y,HD_WET);
  if(((t/18u+(unsigned)i)&3u)==0u)
   hd_line(p,x-4,y+2,x+3,y+2,HD_ROAD_COOL);
 }
}
/* Shared facade across the conversation and missing-person chapter.
   The shop has the same cornice, sash, counter and cake stand in both. */
void hd_cafe_facade(uint8_t *p,int x,int y,unsigned t){
 hd_rect(p,x+4,y-10,111,84,HD_SHADOW);
 hd_rect(p,x,y-7,118,81,HD_BRICK_DARK);
 hd_rect(p,x+118,y+2,9,72,HD_SHADOW);
 hd_rect(p,x+1,y-7,116,3,HD_BRICK);
 hd_rect(p,x-2,y-4,122,3,HD_BARK);
 hd_line(p,x,y-4,x+117,y-4,HD_COPPER);
 hd_rect(p,x+2,y,114,16,HD_BRICK_DARK);
 /* Cast-shadow cornice and recessed pilasters: a physical elevation. */
 hd_rect(p,x+3,y-1,112,2,HD_SHADOW);
 hd_rect(p,x+2,y+2,3,12,HD_BARK);
 hd_rect(p,x+112,y+2,3,12,HD_BARK);
 hd_line(p,x+3,y+2,x+3,y+13,HD_BRICK_LIT);
 hd_line(p,x+4,y+14,x+113,y+14,HD_BRICK);
 /* Selective masonry chips and recessed upper vents. */
 for(int i=0;i<8;i++){
  int bx=x+5+(i*31)%105,by=y+3+(i%3)*5;
  hd_line(p,bx,by,bx+5+(i&1)*3,by,HD_BRICK);
 }
 for(int i=0;i<3;i++){
  int wx=x+17+i*34;
  hd_rect(p,wx,y+2,22,11,HD_SHADOW);
  hd_rect(p,wx+2,y+4,18,7,HD_BARK);
  hd_line(p,wx+3,y+10,wx+19,y+10,HD_BRICK);
  hd_line(p,wx+10,y+4,wx+10,y+9,HD_SHADOW);
 }
 hd_rect(p,x+4,y+15,110,12,HD_SHADOW);
 hd_rect(p,x+5,y+16,109,10,HD_BARK);
 hd_line(p,x+6,y+16,x+112,y+16,HD_COPPER);
 hd_line(p,x+6,y+25,x+112,y+25,HD_GOLD);
 hd_text(p,x+16,y+18,"MARA'S CAFE",HD_PAPER,1,1);
 /* Two restrained lamps illuminate the board, not the entire sky. */
 for(int i=0;i<2;i++){
  int lx=x+20+i*76;
  hd_line(p,lx,y+10,lx,y+14,HD_INK);
  hd_rect(p,lx-4,y+13,9,2,HD_COPPER);
  hd_rect(p,lx-2,y+15,5,1,HD_LAMP);
 }
 hd_rect(p,x+8,y+30,102,43,HD_SHADOW);
 hd_rect(p,x+13,y+32,95,39,HD_INK);
 hd_rect(p,x+17,y+35,87,33,HD_BARK);
 hd_rect(p,x+20,y+38,81,17,HD_BRICK_DARK);
 hd_rect(p,x+22,y+41,32,13,HD_BRICK);
 hd_rect(p,x+60,y+41,39,13,HD_BRICK);
 /* Deep glazing, crockery on a ledge and restrained rain reflections. */
 hd_rect(p,x+30,y+42,22,2,HD_BARK);
 hd_rect(p,x+77,y+42,20,2,HD_BARK);
 hd_rect(p,x+32,y+39,6,3,HD_PLASTER);
 hd_rect(p,x+40,y+40,4,2,HD_COPPER);
 hd_rect(p,x+79,y+39,5,3,HD_PLASTER);
 hd_line(p,x+23,y+37,x+33,y+37,HD_ROAD_LIGHT);
 hd_line(p,x+20,y+40,x+26,y+36,HD_PLASTER);
 hd_line(p,x+62,y+37,x+67,y+36,HD_ROAD_LIGHT);
 /* Amber practical light and utensils are seen through dark glazing. */
 hd_rect(p,x+25,y+39,3,10,HD_COPPER);
 hd_rect(p,x+71,y+39,3,10,HD_COPPER);
 hd_rect(p,x+22,y+48,9,3,HD_GOLD);
 hd_rect(p,x+68,y+48,9,3,HD_GOLD);
 hd_rect(p,x+25,y+50,3,2,HD_LAMP);
 hd_rect(p,x+71,y+50,3,2,HD_LAMP);
 hd_rect(p,x+20,y+55,81,11,HD_SHADOW);
 hd_rect(p,x+22,y+53,77,2,HD_COPPER);
 hd_rect(p,x+83,y+45,15,4,HD_GOLD);
 hd_rect(p,x+85,y+43,11,2,HD_PAPER);
 hd_rect(p,x+89,y+41,3,2,HD_RED_SHADE);
 hd_rect(p,x+38,y+49,5,4,HD_PLASTER);
 hd_rect(p,x+45,y+49,4,4,HD_LEAF_DARK);
 hd_line(p,x+49,y+48,x+50,y+45,HD_LEAF);
 hd_rect(p,x+56,y+35,3,33,HD_SHADOW);
 hd_line(p,x+57,y+35,x+57,y+65,HD_COPPER);
 hd_rect(p,x+16,y+52,90,2,HD_BARK);
 hd_line(p,x+18,y+35,x+50,y+35,HD_PLASTER);
 hd_line(p,x+61,y+35,x+102,y+35,HD_PLASTER);
 hd_rect(p,x+12,y+69,98,3,HD_PLASTER);
 hd_line(p,x+14,y+69,x+107,y+69,HD_STONE_LIT);
 hd_rect(p,x+10,y+72,102,3,HD_SHADOW);
 /* Cloth scallops shade the top of the sash. */
 hd_rect(p,x+7,y+27,107,5,HD_SHADOW);
 hd_rect(p,x+8,y+27,104,1,HD_BARK);
 for(int i=0;i<11;i++){
  int ax=x+9+i*9;
  unsigned fabric=i&1?HD_BRICK:HD_BARK;
  hd_rect(p,ax,y+28,8,3,fabric);
  hd_line(p,ax,y+28,ax+7,y+28,i&1?HD_BRICK_LIT:HD_COPPER);
  hd_rect(p,ax+1,y+31,6,2,i&1?HD_COPPER:HD_RED_SHADE);
  hd_pixel(p,ax+1,y+32,HD_SHADOW);
 }
 hd_rect(p,x+11,y+32,2,35,HD_BARK);
 hd_line(p,x+11,y+33,x+11,y+66,HD_COPPER);
 hd_rect(p,x+107,y+33,2,35,HD_BARK);
 /* Rain pipe and a grounded planter finish the silhouette. */
 hd_rect(p,x+114,y+1,3,68,HD_SHADOW);
 hd_line(p,x+115,y+2,x+115,y+64,HD_STONE_DARK);
 hd_rect(p,x+111,y+45,7,2,HD_BARK);
 hd_rect(p,x+110,y+67,10,6,HD_BARK);
 hd_rect(p,x+111,y+65,9,3,HD_COPPER);
 hd_line(p,x+115,y+65,x+111,y+57,HD_LEAF);
 hd_line(p,x+115,y+65,x+120,y+55,HD_LEAF_DARK);
 hd_line(p,x+115,y+63,x+118,y+59,HD_LEAF);
 hd_line(p,x+23,y+82,x+45,y+82,HD_COPPER);
 hd_line(p,x+28,y+85,x+42,y+85,HD_BARK);
 hd_line(p,x+78,y+81,x+98,y+81,HD_COPPER);
 hd_line(p,x+82,y+85,x+94,y+85,HD_BARK);
 (void)t;
}
static void paper(uint8_t *p,int x,int y,int w,int h,unsigned t){
 hd_rect(p,x+2,y+2,w,h,HD_SHADOW);
 hd_rect(p,x,y,w,h,HD_PAPER);
 hd_rect(p,x+1,y+1,w-2,2,HD_STONE_LIT);
 /* A folded note is shorter than a full sheet. Ink belongs inside the
    paper, including the small rooftop note; four fixed lines used to
    spill through the counter and onto the floor. */
 for(int row=0,yy=6;yy<h-2;row++,yy+=4){
  int end=x+w-4-((row&1)?w/5:0);
  hd_line(p,x+3,y+yy,end,y+yy,HD_STONE);
 }
 hd_rect(p,x+3,y+4,w>16?9:w-6,1,HD_RED_SHADE);
 if(w>=20&&h>=14&&((t/120u)&1u))
  hd_rect(p,x+w-8,y+h-6,4,3,HD_COPPER);
}
static void archive_room(uint8_t *p,unsigned t,int backed,int rebrand){
 inside(p,t,0);
 hd_rect(p,138,32,91,73,HD_BRICK_DARK);
 for(int i=0;i<5;i++){
  int x=141+i*17;
  hd_rect(p,x,35,14,57,HD_INK);
  hd_rect(p,x+2,38,10,16,HD_STONE_DARK);
  for(int k=0;k<3;k++)hd_rect(p,x+3,42+k*4,8,1,HD_COPPER);
  hd_rect(p,x+3,59,7,4,(t/60u+i)%7u==0u?HD_LAMP:HD_GLASS_LIT);
  hd_rect(p,x+3,70,9,13,HD_ROAD);
  for(int k=0;k<3;k++)hd_line(p,x+4,73+k*3,x+11,73+k*3,HD_STONE);
 }
 hd_rect(p,21,112,205,10,HD_BARK);
 hd_rect(p,25,122,8,35,HD_BARK);
 hd_rect(p,215,122,8,35,HD_BARK);
 hd_rect(p,59,92,73,24,HD_INK);
 hd_rect(p,62,95,67,17,HD_GLASS);
 for(int i=0;i<6;i++){
  int x=65+i*10,y=101+(i&1)*3;
  hd_rect(p,x,y,7,1,HD_STONE_LIT);
  if((t/19u+i)&1u)hd_rect(p,x+1,y+4,5,1,HD_LAMP);
 }
 paper(p,137,101,28,15,t);
 hd_text(p,148,14,rebrand?"MIRROR / NEW SIGN":
                backed?"ARCHIVED / COPIES":"MIRROR / SERVER",HD_PAPER,1,0);
 if(rebrand){
  hd_rect(p,149,62,65,18,HD_PAPER);
  hd_text(p,156,67,"NEW NAME",HD_INK,1,1);
  hd_rect(p,154,83,55,2,HD_COPPER);
 }
 hd_actor_stage(p,39,146,1,t,backed?0:3);
}
static void witness_lane(uint8_t *p,unsigned t){
 rainy_lane(p,t);
 /* A physical iron bench and a paper envelope between the people. */
 hd_rect(p,95,114,111,6,HD_BARK);
 hd_rect(p,101,121,5,28,HD_INK);
 hd_rect(p,194,121,5,28,HD_INK);
 for(int y=97;y<113;y+=6)
  hd_rect(p,96,y,108,4,HD_COPPER);
 hd_rect(p,99,96,4,24,HD_INK);
 hd_rect(p,198,96,4,24,HD_INK);
 /* Danny closes the distance before accepting the witness's one note.
    The donor's left hand supports it throughout; at the settled pose
    Danny's right hand meets the same sheet, never a second floating
    copy over the bench. Keep the wrists on the actor's breathing clock. */
 unsigned approach=t>12u?t-12u:0u;
 if(approach>84u)approach=84u;
 int danny=59+(int)(approach*78u/84u);
 int settled=approach==84u;
 int breath=(int)((t/38u)&1u);
 hd_actor_stage(p,danny,143,1,t,settled?0:2);
 hd_actor_stage(p,179,143,4,t,t<55u?1:0);
 hd_rect(p,148,107+breath,18,11,HD_BARK);
 hd_rect(p,149,108+breath,16,9,HD_PAPER);
 hd_line(p,150,109+breath,157,113+breath,HD_COPPER);
 hd_line(p,164,109+breath,157,113+breath,HD_COPPER);
 hd_rect(p,162,114+breath,3,4,HD_SKIN);
 hd_pixel(p,162,115+breath,HD_SKIN_LIT);
 if(settled){
  hd_rect(p,151,114+breath,4,4,HD_SKIN);
  hd_line(p,152,115+breath,154,115+breath,HD_SKIN_LIT);
 }
}
static void safehouse(uint8_t *p,unsigned t,int scene){
 inside(p,t,1);
 /* The same window stays in place for the closed-door, saved, and
    ordinary safehouse shots. Fabric folds and sconce make this a home,
    not an empty cyan wall behind two actors. */
 hd_rect(p,17,16,7,74,HD_RED_SHADE);
 hd_rect(p,18,19,3,68,HD_BRICK);
 hd_line(p,18,20,18,85,HD_COPPER);
 hd_rect(p,115,16,7,74,HD_RED_SHADE);
 hd_rect(p,119,19,2,68,HD_BRICK);
 hd_rect(p,19,53,7,4,HD_GOLD);
 hd_rect(p,113,53,9,4,HD_GOLD);
 hd_rect(p,127,40,14,28,HD_BARK);
 hd_rect(p,130,43,10,18,HD_BRICK);
 hd_rect(p,125,38,16,3,HD_COPPER);
 hd_rect(p,130,41,8,3,HD_BARK);
 hd_rect(p,132,44,5,7,HD_LAMP);
 hd_pixel(p,134,46,HD_PAPER);
 hd_rect(p,58,138,76,17,HD_RED_SHADE);
 hd_rect(p,61,140,70,13,HD_BRICK);
 hd_line(p,63,143,129,143,HD_COPPER);
 hd_line(p,63,150,129,150,HD_COPPER);
 /* A real wooden door, hinges, floor threshold and inviting tungsten lamp. */
 hd_rect(p,144,13,83,115,HD_INK);
 hd_rect(p,148,17,75,106,HD_BARK);
 hd_line(p,147,16,222,16,HD_PLASTER);
 hd_rect(p,221,18,3,105,HD_STONE_DARK);
 for(int i=0;i<3;i++)
  hd_rect(p,154,22+i*28,63,24,HD_BRICK_DARK);
 hd_rect(p,156,24,59,19,HD_BARK);
 hd_rect(p,156,52,59,19,HD_BARK);
 hd_rect(p,156,81,59,19,HD_BARK);
 hd_rect(p,193,77,5,6,HD_LAMP);
 hd_rect(p,137,124,98,5,HD_STONE);
 hd_rect(p,139,129,96,3,HD_ROAD);
 hd_rect(p,28,107,94,8,HD_BARK);
 hd_rect(p,33,115,8,25,HD_BARK);
 hd_rect(p,106,115,8,25,HD_BARK);
 hd_rect(p,43,99,15,10,HD_PAPER);
 hd_rect(p,45,96,11,3,HD_COPPER);
 hd_rect(p,64,99,13,9,HD_STONE_LIT);
 hd_rect(p,75,97,7,3,HD_BARK);
 if(scene==CINE_RESCUE_DOOR||scene==CINE_RESCUE_SAVED){
  int open=scene==CINE_RESCUE_SAVED?(int)(t>100u?40u:t*2u/5u):0;
  /* No teleport: the wood leaf rotates into its archway and Mara waits
     behind it until the opening actually reveals the room. */
  hd_rect(p,151,21,65,100,HD_INK);
  if(open){
   /* Practical hall light and reflected threshold share the opening. */
   hd_line(p,156,118,200,118,HD_LAMP);
   hd_rect(p,156,119,44,3,HD_STONE_LIT);
   hd_ellipse(p,180,132,24,5,HD_BRICK_DARK);
   hd_line(p,163,128,198,128,HD_COPPER);
   hd_line(p,170,133,192,133,HD_PLASTER);
   hd_line(p,177,137,188,137,HD_BARK);
  }
  int mara_x=178;
  if(open){
   hd_rect(p,155,25,57,92,HD_GLASS);
   hd_rect(p,160,31,49,85,HD_STONE_DARK);
   hd_rect(p,167,39,32,56,HD_PLASTER);
   hd_rect(p,170,42,26,50,HD_BRICK_DARK);
   if(t>=76u){
    /* Mara steps out from the same revealed room as its door opens.
       Advancing the pose never teleports her across the threshold. */
    unsigned move=t>145u?t-145u:0u;
    if(move>95u)move=95u;
    int mx=178-(int)(move*36u/95u);
    mara_x=mx;
    if(t<145u)hd_actor_stage(p,mx,145,2,t,t>=123u?1:0);
   }
  }
  hd_rect(p,153,23,63-open,96,HD_BARK);
  hd_rect(p,156,27,56-open,88,HD_BRICK_DARK);
  hd_rect(p,155,57,60-open,2,HD_COPPER);
  hd_rect(p,214-open,24,2,94,HD_PLASTER);
  hd_rect(p,153,37,2,7,HD_COPPER);
  hd_rect(p,153,96,2,7,HD_COPPER);
  if(open<20)hd_rect(p,204-open/2,82,5,5,HD_LAMP);
  /* Her feet have crossed the threshold: the complete actor now belongs
     in front of the wood leaf, not sliced into it during the approach. */
  if(open && t>=145u)
   hd_actor_stage(p,mara_x,145,2,t,t<240u?2:0);
  /* After confirming Mara is there, Danny crosses the floor toward her.
     Both characters advance in world-space, not as detached hand cutouts. */
  int danny_x=63;
  if(scene==CINE_RESCUE_SAVED && t>145u){
   unsigned approach=t-145u;
   if(approach>100u)approach=100u;
   danny_x=63+(int)(approach*59u/100u);
  }
  hd_actor_stage(p,danny_x,144,1,t,
                 scene==CINE_RESCUE_SAVED && t>=145u && t<245u?
                 2:t<67u?2:t<107u?3:0);
  if(scene==CINE_RESCUE_SAVED && t>=205u &&
     mara_x-danny_x<=45){
   /* Both hands share the contact point, including in late reaction
      frames, rather than snapping to a separate 'hug' illustration. */
   int left=danny_x+16,right=mara_x-15;
   hd_line(p,left,117,right,117,HD_SKIN);
   hd_line(p,left,118,right,118,HD_SKIN_LIT);
   hd_ellipse(p,(left+right)/2,118,3,2,HD_SKIN_LIT);
  }

 }else{
  hd_actor_stage(p,49,144,1,t,0);
  hd_actor_stage(p,113,144,4,t,t>78u?1:0);
 }
}
static void empty_cafe(uint8_t *p,unsigned t){
 rainy_lane(p,t);
 hd_cafe_facade(p,8,38,t);
 hd_ellipse(p,145,149,48,4,HD_SHADOW);
 hd_rect(p,99,119,92,7,HD_BARK);
 hd_rect(p,101,119,88,2,HD_PLASTER);
 hd_line(p,101,122,187,122,HD_COPPER);
 hd_rect(p,105,125,7,24,HD_BARK);
 hd_rect(p,178,125,7,24,HD_BARK);
 hd_rect(p,115,110,13,8,HD_PLASTER_LIT);
 hd_rect(p,117,117,9,2,HD_PLASTER);
 hd_rect(p,115,110,3,7,HD_PAPER);
 hd_ellipse(p,121,110,7,2,HD_PAPER);
 hd_line(p,118,110,125,110,HD_BARK);
 hd_rect(p,128,112,3,4,HD_PAPER);
 hd_pixel(p,129,113,HD_BARK);
 hd_rect(p,147,114,10,13,HD_INK);
 hd_rect(p,149,116,6,9,HD_GLASS);
 if((t/70u)&1u)hd_rect(p,151,118,2,1,HD_LAMP);
 /* A vacant chair occupies clear space beside the table. Its back,
    seat and separated feet read before its small material highlights. */
 hd_ellipse(p,200,145,14,3,HD_SHADOW);
 hd_rect(p,191,117,3,28,HD_INK);
 hd_rect(p,208,117,3,28,HD_INK);
 hd_rect(p,192,115,17,3,HD_BARK);
 hd_line(p,194,115,207,115,HD_COPPER);
 hd_rect(p,194,120,13,3,HD_BARK);
 hd_rect(p,191,132,21,4,HD_BARK);
 hd_line(p,193,132,209,132,HD_COPPER);
 hd_line(p,194,136,191,148,HD_INK);
 hd_line(p,209,136,212,148,HD_INK);
 /* Mara is absent. No unearned implication that she died or recovered. */
 hd_actor_stage(p,45,143,1,t,7);
}
static void rooftop(uint8_t *p,unsigned t,int paint,int ending_state){
 rainy_lane(p,t);
 hd_rect(p,0,101,240,59,HD_ROAD);
 hd_rect(p,0,99,240,4,HD_STONE_LIT);
 hd_rect(p,0,75,240,4,HD_COPPER);
 for(int x=12;x<240;x+=27)hd_rect(p,x,77,3,28,HD_BARK);
 hd_line(p,0,102,239,102,HD_STONE_DARK);
 for(int i=0;i<6;i++){
  hd_line(p,i*43,105,i*43-13,157,HD_ROAD_COOL);
  hd_rect(p,i*41,147,35,1,HD_ROAD_LIGHT);
 }
 hd_rect(p,145,41,87,67,HD_BARK);
 hd_rect(p,150,46,77,57,HD_PAPER);
 hd_rect(p,154,50,69,44,HD_GLASS);
 hd_ellipse(p,195,62,9,9,HD_LAMP);
 for(int i=0;i<9;i++){
  int x=154+i*8,h=11+(i*13%25);
  hd_rect(p,x,91-h,8,h,HD_STONE_DARK);
  hd_rect(p,x+2,87-h,2,2,HD_LAMP);
 }
 hd_rect(p,154,92,69,8,HD_LEAF);
 if(paint){
  for(int i=0;i<9;i++){
   int x=158+i*7;
   hd_line(p,x,67+i%5,x+4,65+i%7,
           i&1?HD_RED_SHADE:HD_GOLD);
  }
  hd_rect(p,177,101,27,3,HD_COPPER);
 }
 hd_rect(p,180,108,7,34,HD_BARK);
 hd_line(p,182,113,164,147,HD_BARK);
 hd_line(p,184,113,207,147,HD_BARK);
 hd_rect(p,209,140,18,6,HD_BARK);
 hd_rect(p,212,136,12,5,HD_BRICK_DARK);
 hd_rect(p,214,132,8,4,HD_GOLD);
 if(ending_state) {
  hd_rect(p,71,126,20,5,HD_INK);
  hd_rect(p,89,126,6,18,HD_INK);
  if(ending_state==1 && t<400u)
   hd_actor_stage(p,48,144,1,t,t>310u?7:3);
  else if(ending_state==3)
   hd_actor_stage(p,52,146,2,t,t>3000u?0:7);
  else if(ending_state==2)
   hd_actor_stage(p,48,144,1,t,7);
  paper(p,106,138,14,9,t);
  if(ending_state==2){
   /* The unanswered phone stays with Danny. Never render Mara during
      unresolved bad-ending shots. Her condition is not established. */
   hd_rect(p,125,131,9,14,HD_INK);
   hd_rect(p,127,134,5,9,HD_GLASS);
   if((t/81u)&1u)hd_pixel(p,129,136,HD_LAMP);
  }
 }else{
  hd_actor_stage(p,57,145,1,t,t<115u?3:0);
 }
}
/* Four contrasting album-cover identities in the shop, all authored
   as clipped native primitives. The former eight repeated vinyl glyphs
   made the shelves read like an identical debug tile grid. */
static void album_sleeve(uint8_t *p,int x,int y,int id){
 unsigned paper=id%3==0?HD_GLASS:id%3==1?HD_LEAF_DARK:
                HD_BRICK_LIT;
 hd_rect(p,x,y,21,26,HD_BARK);
 hd_rect(p,x+2,y+2,17,22,paper);
 hd_rect(p,x+2,y+2,17,2,(id&1)?HD_COPPER:HD_GOLD);
 switch(id&3){
  case 0: /* Danny's master: the record itself. */
   hd_ellipse(p,x+10,y+12,7,7,HD_INK);
   hd_ellipse(p,x+10,y+12,3,3,HD_PAPER);
   hd_ellipse(p,x+10,y+12,1,1,HD_GOLD);
   break;
  case 1: /* Live in the Old Quarter: asymmetrical midnight skyline. */
   hd_rect(p,x+4,y+13,3,7,HD_INK);
   hd_rect(p,x+8,y+10,4,10,HD_INK);
   hd_rect(p,x+13,y+7,4,13,HD_INK);
   hd_pixel(p,x+15,y+9,HD_LAMP);
   hd_pixel(p,x+10,y+13,HD_GOLD);
   hd_line(p,x+4,y+20,x+17,y+20,HD_STONE_LIT);
   break;
  case 2: /* Unfinished score: waveform and annotated tape mark. */
   hd_line(p,x+4,y+13,x+7,y+13,HD_INK);
   hd_line(p,x+7,y+13,x+9,y+7,HD_INK);
   hd_line(p,x+9,y+7,x+12,y+19,HD_INK);
   hd_line(p,x+12,y+19,x+15,y+10,HD_INK);
   hd_line(p,x+15,y+10,x+17,y+13,HD_INK);
   hd_rect(p,x+6,y+17,3,2,HD_GOLD);
   hd_pixel(p,x+15,y+7,HD_PAPER);
   break;
  default: /* After-hours sketches: plant, window and flowers. */
   hd_line(p,x+10,y+19,x+10,y+9,HD_INK);
   hd_line(p,x+10,y+16,x+5,y+12,HD_INK);
   hd_line(p,x+10,y+15,x+15,y+11,HD_INK);
   hd_ellipse(p,x+10,y+9,5,4,HD_RED_SHADE);
   hd_ellipse(p,x+10,y+9,2,2,HD_GOLD);
   hd_pixel(p,x+6,y+12,HD_LEAF_LIT);
   hd_pixel(p,x+14,y+11,HD_LEAF_LIT);
   break;
 }
 hd_line(p,x+3,y+22,x+17,y+22,HD_STONE_LIT);
 if(id&1)hd_pixel(p,x+5,y+5,HD_LAMP);
}
static void good_record(uint8_t *p,unsigned t){
 inside(p,t,1);
 /* Warm listening-room key light, stock-shelf brackets, and actual
    album-cover identities. Eight identical disc symbols read like a
    debug grid rather than a record shop; the variation is intentional. */
 hd_rect(p,130,10,103,16,HD_SHADOW);
 hd_rect(p,133,12,98,13,HD_BARK);
 hd_rect(p,135,14,94,9,HD_BRICK_DARK);
 hd_text(p,140,15,"DANNY'S MASTER",HD_PAPER,1,0);
 hd_line(p,132,26,231,26,HD_GOLD);
 hd_line(p,188,0,188,10,HD_BARK);
 hd_rect(p,178,10,21,4,HD_COPPER);
 hd_rect(p,181,14,15,3,HD_GOLD);
 hd_rect(p,184,17,9,2,HD_LAMP);
 hd_rect(p,130,30,104,67,HD_SHADOW);
 hd_rect(p,132,31,100,64,HD_STONE_DARK);
 for(int i=0;i<8;i++){
  int x=133+(i%4)*25,y=33+(i/4)*32;
  album_sleeve(p,x,y,i);
 }
 hd_rect(p,133,62,98,2,HD_COPPER);
 hd_rect(p,133,94,98,3,HD_BARK);
 hd_rect(p,18,109,207,9,HD_BARK);
 hd_rect(p,18,109,207,2,HD_COPPER);
 hd_rect(p,25,118,7,36,HD_BARK);
 hd_rect(p,209,118,7,36,HD_BARK);
 hd_rect(p,35,121,164,26,HD_BRICK_DARK);
 for(int i=0;i<6;i++){
  int x=52+i*24;
  hd_rect(p,x,127,18,14,HD_BARK);
  hd_rect(p,x+3,129,12,9,i&1?HD_BRICK:HD_LEAF_DARK);
  hd_line(p,x+4,135,x+12,135,HD_COPPER);
 }
 /* A shallow plinth rests on the counter. The pivot is bolted into that
    deck, and its bent tonearm ends in a cartridge touching the groove;
    neither the arm nor the platter hangs unsupported above the table. */
 hd_rect(p,45,98,99,19,HD_INK);
 hd_rect(p,47,99,95,15,HD_STONE_DARK);
 hd_rect(p,48,114,93,3,HD_BRICK_DARK);
 hd_line(p,49,114,139,114,HD_COPPER);
 hd_rect(p,52,117,8,2,HD_INK);
 hd_rect(p,130,117,8,2,HD_INK);
 hd_ellipse(p,83,104,35,10,HD_INK);
 hd_ellipse(p,83,102,34,9,HD_INK);
 hd_ellipse(p,83,102,28,7,HD_STONE_DARK);
 hd_ellipse(p,83,102,23,6,HD_ROAD_COOL);
 hd_ellipse(p,83,102,5,3,HD_GOLD);
 hd_pixel(p,83,102,HD_PAPER);
 int angle=(int)(t/16u&3u);
 hd_line(p,83,102,83+((angle&1)?7:-7),99,HD_PAPER);
 hd_ellipse(p,134,102,5,3,HD_INK);
 hd_ellipse(p,134,101,4,2,HD_COPPER);
 hd_rect(p,132,94,4,8,HD_BARK);
 hd_line(p,133,94,133,100,HD_GOLD);
 hd_line(p,134,95,116,100,HD_COPPER);
 hd_line(p,116,100,110,104,HD_COPPER);
 hd_rect(p,108,102,7,3,HD_INK);
 hd_line(p,109,103,113,103,HD_PAPER);
 hd_line(p,110,105,110,106,HD_COPPER);
 hd_rect(p,126,109,10,2,HD_INK);
 hd_pixel(p,136,110,HD_LAMP);
 /* The album is HELD by Mara at the end of her raised right arm.
    Draw the arm first, then the sleeve and tiny supporting fingers:
    no separate levitating record over the turntable platter. */
 hd_actor_stage(p,43,143,2,t,5);
 int hand=104+(int)((t/38u)&1u);
 hd_line(p,64,hand,68,102,HD_SKIN_LIT);
 hd_rect(p,65,79,20,25,HD_INK);
 hd_rect(p,67,81,16,21,HD_GLASS_LIT);
 hd_rect(p,67,81,16,3,HD_COPPER);
 hd_ellipse(p,75,91,6,6,HD_INK);
 hd_ellipse(p,75,91,2,2,HD_GOLD);
 hd_line(p,68,99,82,99,HD_PAPER);
 hd_rect(p,67,101,4,3,HD_SKIN);
 hd_pixel(p,68,102,HD_SKIN_LIT);
}
static void concert(uint8_t *p,unsigned t){
 rainy_lane(p,t);
 /* This is an *in-world* memorial performance, not a faceless void.
    Architectural truss, hung banner, powered monitors and a visible
    audience create actual stage depth at 240x160 on one framebuffer. */
 hd_rect(p,0,83,240,46,HD_INK);
 hd_rect(p,17,78,206,42,HD_BRICK_DARK);
 for(int k=0;k<9;k++)
  hd_rect(p,22+k*24,81,19,30,k&1?HD_STONE_DARK:HD_BRICK_DARK);
 hd_rect(p,19,118,202,8,HD_BARK);
 hd_rect(p,20,120,200,4,HD_COPPER);
 hd_rect(p,18,38,205,8,HD_BRICK);
 hd_rect(p,21,42,4,67,HD_BRICK);
 hd_rect(p,215,42,4,67,HD_BRICK);
 hd_rect(p,29,48,182,2,HD_GOLD);
 hd_rect(p,75,20,90,18,HD_INK);
 hd_rect(p,77,22,86,14,HD_BRICK_DARK);
 hd_line(p,79,23,161,23,HD_COPPER);
 hd_text(p,82,26,"ONE MORE SONG",HD_PAPER,1,0);
 /* Broad low-value light shapes establish volume; tiny practical bulbs
    retain the brightest pixels. Background beams never cover actors. */
 for(int beam=0;beam<2;beam++){
  int source=beam?157:71,target=beam?126:78;
  int sway=(int)((t/54u+(unsigned)beam)&3u)-1;
  for(int y=56;y<116;y+=3){
   int d=y-56,cx=source+(target-source)*d/60+sway;
   int w=3+d/5;
   hd_rect(p,cx-w,y,w*2,3,beam?HD_ROAD_LIGHT:HD_BARK);
  }
 }
 for(int i=0;i<5;i++){
  int x=28+i*43;
  hd_rect(p,x,42,7,10,HD_BARK);
  hd_rect(p,x+1,50,5,3,HD_COPPER);
  hd_rect(p,x+2,53,3,3,HD_LAMP);
 }
 /* Stage infrastructure and instruments stay behind the performers. */
 for(int i=0;i<2;i++){
  int x=i?197:22;
  hd_rect(p,x,82,20,35,HD_INK);
  hd_rect(p,x+2,85,16,29,HD_STONE_DARK);
  hd_ellipse(p,x+10,95,6,6,HD_SHADOW);
  hd_ellipse(p,x+10,108,5,5,HD_INK);
  hd_ellipse(p,x+10,108,2,2,HD_COPPER);
 }
 hd_ellipse(p,167,111,21,12,HD_SHADOW);
 hd_ellipse(p,167,106,13,12,HD_BARK);
 hd_ellipse(p,167,106,10,9,HD_GLASS);
 hd_line(p,167,97,167,86,HD_COPPER);
 hd_ellipse(p,167,97,3,3,HD_GOLD);
 hd_rect(p,151,112,3,9,HD_COPPER);
 hd_rect(p,182,112,3,9,HD_COPPER);
 hd_rect(p,42,112,39,6,HD_BARK);
 hd_rect(p,45,108,12,4,HD_STONE_LIT);
 hd_rect(p,63,108,11,4,HD_STONE_LIT);
 /* Purposeful spotlight pools at the same stage coordinates as the two
    actors. Draw them BEFORE people and the foreground audience, so feet
    are visibly grounded instead of glowing through shoes. */
 hd_ellipse(p,75,120,22,4,HD_SHADOW);
 hd_ellipse(p,75,119,17,2,HD_COPPER);
 hd_ellipse(p,121,123,27,4,HD_SHADOW);
 hd_ellipse(p,121,122,21,2,HD_LAMP);
 /* Mara steps into the spotlight to introduce the record in memory
    of Danny. The clerk supports from stage left, rather than making
    Danny inexplicably appear alive at his own posthumous tribute. */
 hd_actor_stage(p,75,119,3,t,0);
 hd_actor_stage(p,121,122,2,t,1);
 /* Mara's animated speech hand is at x140/y85..87. A fixed floor
    microphone at x130 previously floated ten pixels away from that
    hand; key its head at the actual reach while the stand stays put. */
 hd_line(p,140,89,140,117,HD_COPPER);
 hd_ellipse(p,140,86,3,3,HD_INK);
 hd_line(p,139,85,141,85,HD_PAPER);
 hd_line(p,140,118,134,122,HD_STONE);
 hd_rect(p,86,116,21,5,HD_SHADOW);
 hd_line(p,89,116,103,116,HD_STONE_LIT);
 /* The near row of audience heads occupies the *foreground* rather
    than appearing only beneath the final-film subtitles. */
 for(int i=0;i<15;i++){
  int x=i*17-5;
  int bob=(int)(((unsigned)(i*7)+t/26u)&3u);
  int depth=i%3;
  int yy=114+depth*3+bob;
  int radius=5+depth;
  hd_ellipse(p,x,yy,radius,4+depth/2,
             (i&1)?HD_HAIR:HD_STONE_DARK);
  hd_rect(p,x-radius,yy+3,radius*2+1,13+depth*2,
          i%4==0?HD_BRICK:i%4==1?HD_GLASS:
          i%4==2?HD_LEAF_DARK:HD_STONE_LIT);
  if(i%4==0)hd_line(p,x+radius-1,yy+6,x+radius+4,yy-7,HD_SKIN);
  else if(i%5==2)hd_rect(p,x-radius+2,yy+7,5,2,HD_COPPER);
 }
}
int hd_special_cinema_draw(uint8_t *p,const Story *s,unsigned t){
 switch(s->cinema){
  case CINE_SERVER:archive_room(p,t,0,0);break;
  case CINE_BACKUP:archive_room(p,t,1,0);break;
  case CINE_REBRAND:archive_room(p,t,1,1);break;
  case CINE_WITNESS:witness_lane(p,t);break;
  case CINE_SAFE:safehouse(p,t,CINE_SAFE);break;
  case CINE_RESCUE_DOOR:
  case CINE_RESCUE_SAVED:safehouse(p,t,s->cinema);break;
  case CINE_MARA_EMPTY:empty_cafe(p,t);break;
  case CINE_FINALE:rooftop(p,t,1,s->page>=6u?3:1);break;
  case CINE_BAD:rooftop(p,t,1,2);break;
  case CINE_DAY:rooftop(p,t,0,0);break;
  case CINE_GOOD:
   if(s->page>=6u&&s->page<=7u)concert(p,t);
   else if(s->page==5u)rooftop(p,t,1,3);
   else if(s->page<=1u)rooftop(p,t,1,0);
   else good_record(p,t);
   break;
  default:return 0;
 }
 hd_rect(p,0,0,240,2,HD_INK);
 hd_rect(p,0,158,240,2,HD_INK);
 return 1;
}

