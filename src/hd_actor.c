#include "hd_canvas.h"

/* Native character drawing uses a handful of filled planes, not a scaled
   bitmap. Endpoints are the same world-space acting anchors used by the
   props and camera rigs. A face is a silhouette before it is a collection
   of features; light is grouped into planes rather than outline noise. */
static void plane(uint8_t *p,int x,int y,int left,int right,
                  int end_left,int end_right,int height,unsigned ink){
 if(height<1)return;
 int l=left*256,r=right*256;
 int dl=height>1?(end_left-left)*256/(height-1):0;
 int dr=height>1?(end_right-right)*256/(height-1):0;
 int start=0,previous_left=left,previous_right=right;
 for(int row=1;row<height;row++){
  l+=dl;r+=dr;
  int a=l/256,b=r/256;
  if(a==previous_left&&b==previous_right)continue;
  hd_rect(p,x+previous_left,y+start,previous_right-previous_left+1,
          row-start,ink);
  start=row;previous_left=a;previous_right=b;
 }
 hd_rect(p,x+previous_left,y+start,previous_right-previous_left+1,
         height-start,ink);
}
static void limb(uint8_t *p,int sx,int sy,int ex,int ey,int wx,int wy,
                 int width,unsigned shadow,unsigned body,unsigned light){
 /* A sleeve bends at its elbow. Parallel continuous strokes keep the
    wrist attached without black ribs at every rasterized segment. */
 for(int k=-width/2;k<=width/2;k++){
  unsigned c=k==-width/2?shadow:k==width/2?light:body;
  hd_line(p,sx+k,sy,ex+k,ey,c);
  hd_line(p,ex+k,ey,wx+k,wy,c);
 }
}

/* Roles: Danny 1, Mara 2, archivist 3, witness 4, doctor 5.
   Gestures: listening 0, speaking 1, walking 2, reaching 3, reflective 5,
   worried 7. The foot and hand positions remain shared with the scenes. */
void hd_actor_stage(uint8_t *p,int x,int feet,int role,unsigned frame,int emotion){
 int walk=emotion==2;
 unsigned phase=(frame/8u)&3u;
 int stride=walk?(phase==0u?-4:phase==2u?4:0):0;
 int lift=walk?(phase&1u?1:0):0;
 int fy=feet+(walk?0:(int)((frame/38u)&1u));
 unsigned coat=role==1?HD_RED:role==2?HD_GLASS:
               role==3?HD_STONE_LIT:role==4?HD_LEAF_DARK:HD_PLASTER_LIT;
 unsigned dark=role==1?HD_RED_SHADE:role==2?HD_ROAD_COOL:
               role==3?HD_STONE_DARK:role==4?HD_SHADOW:HD_STONE;
 unsigned light=role==1?HD_BRICK_LIT:role==2?HD_GLASS_LIT:
                role==3?HD_PAPER:role==4?HD_LEAF:HD_PAPER;
 unsigned hair=role==2?HD_BARK:role==3?HD_STONE_DARK:
               role==4?HD_BARK:HD_INK;
 hd_ellipse(p,x+1,feet+1,15,2,HD_SHADOW);
 /* Full trouser legs, bent knees and planted soles replace stick ankles. */
 limb(p,x-5,fy-24,x-6+stride/2,fy-13,x-6+stride,fy-4-lift,
      5,HD_INK,HD_BRICK_DARK,HD_ROAD_COOL);
 limb(p,x+5,fy-24,x+6-stride/2,fy-13,x+6-stride,
      fy-4-(walk?1-lift:0),5,HD_INK,HD_STONE_DARK,HD_ROAD);
 hd_rect(p,x-10+stride,fy-4-lift,10,4,HD_INK);
 hd_rect(p,x+2-stride,fy-4-(walk?1-lift:0),11,4,HD_INK);
 hd_rect(p,x-9+stride,fy-2-lift,6,1,HD_STONE_DARK);
 hd_rect(p,x+4-stride,fy-2-(walk?1-lift:0),7,1,HD_STONE_DARK);
 /* Sloping shoulders, a narrow waist and uneven hem define the garment. */
 plane(p,x,fy-47,-5,5,-13,12,6,HD_INK);
 plane(p,x,fy-42,-13,12,-10,10,21,HD_INK);
 plane(p,x,fy-46,-5,5,-11,11,5,coat);
 plane(p,x,fy-41,-11,11,-9,9,18,coat);
 plane(p,x,fy-41,5,11,3,9,18,dark);
 plane(p,x,fy-43,-9,-6,-8,-5,18,light);
 hd_rect(p,x-9,fy-24,18,3,dark);
 hd_line(p,x-8,fy-22,x-1,fy-22,light);
 hd_line(p,x-5,fy-44,x,fy-36,dark);
 hd_line(p,x+4,fy-44,x,fy-36,light);
 hd_line(p,x,fy-35,x,fy-25,dark);
 hd_pixel(p,x+1,fy-32,HD_COPPER);
 hd_pixel(p,x+1,fy-28,HD_COPPER);
 hd_rect(p,x-7,fy-30,4,1,dark);
 /* Articulated sleeves maintain the existing prop-contact endpoints. */
 int wx=x+16,wy=fy-28,elx=x+14,ely=fy-34;
 if(emotion==3){wx=x+23;wy=fy-49;elx=x+20;ely=fy-38;}
 else if(emotion==7){wx=x+19;wy=fy-57;elx=x+23;ely=fy-42;}
 else if(emotion==5){wx=x+21;wy=fy-39;elx=x+18;ely=fy-31;}
 else if(walk){wx=x+16-stride;wy=fy-31-lift;elx=x+15-stride/2;}
 else if(emotion==1){wx=x+19;wy=fy-37+(int)((frame/10u)&1u)*2;
                     elx=x+17;ely=fy-30;}
 limb(p,x+10,fy-42,elx,ely,wx,wy,4,dark,coat,light);
 hd_rect(p,wx-2,wy-1,5,4,HD_SKIN);
 hd_rect(p,wx-1,wy,3,1,HD_SKIN_LIT);
 int other_x=x-17+(walk?stride:0),other_y=fy-28+(walk?lift*2:0);
 limb(p,x-10,fy-42,x-14+(walk?stride/2:0),fy-35,
      other_x,other_y,4,dark,coat,light);
 hd_rect(p,other_x-2,other_y-1,5,5,HD_SKIN);
 hd_pixel(p,other_x-1,other_y,HD_SKIN_LIT);
 /* Neck, collar and hair are behind the tapered jaw. */
 hd_rect(p,x-3,fy-51,7,7,HD_BRICK_LIT);
 hd_rect(p,x-2,fy-49,4,4,HD_SKIN);
 hd_line(p,x-5,fy-46,x-2,fy-43,light);
 hd_line(p,x+4,fy-46,x+2,fy-43,dark);
 if(role==2){
  plane(p,x,fy-65,-7,7,-11,10,5,hair);
  plane(p,x,fy-60,-11,-7,-9,-7,22,hair);
  plane(p,x,fy-60,7,10,9,12,22,hair);
  hd_line(p,x+10,fy-56,x+11,fy-41,HD_HAIR);
 }else if(role==4){
  plane(p,x,fy-68,-5,5,-12,11,9,HD_SHADOW);
  plane(p,x,fy-59,-12,11,-13,13,16,HD_LEAF_DARK);
 }
 plane(p,x,fy-63,-6,6,-8,8,4,HD_BARK);
 plane(p,x,fy-59,-8,8,-7,7,8,HD_BARK);
 plane(p,x,fy-51,-7,7,-3,4,5,HD_BARK);
 plane(p,x,fy-62,-5,5,-7,6,4,HD_SKIN);
 plane(p,x,fy-58,-7,6,-6,6,8,HD_SKIN);
 plane(p,x,fy-50,-6,6,-2,3,4,HD_SKIN);
 plane(p,x,fy-60,-6,-1,-5,0,9,HD_SKIN_LIT);
 hd_rect(p,x+5,fy-58,2,7,HD_BRICK_LIT);
 hd_rect(p,x-9,fy-57,2,4,HD_SKIN);
 hd_rect(p,x+7,fy-57,2,4,HD_BRICK_LIT);
 /* Short asymmetrical hair clusters; no rectangular helmet silhouette. */
 if(role==2){
  plane(p,x,fy-66,-5,4,-10,8,5,hair);
  plane(p,x,fy-62,-10,8,-8,-1,4,hair);
  hd_line(p,x-7,fy-64,x+1,fy-64,HD_HAIR);
  hd_rect(p,x-9,fy-58,2,9,hair);
  hd_rect(p,x+8,fy-60,2,12,hair);
  hd_rect(p,x-1,fy-45,3,3,HD_GOLD);
 }else if(role==3){
  plane(p,x,fy-65,-5,5,-9,8,4,hair);
  hd_rect(p,x-9,fy-61,3,7,hair);
  hd_rect(p,x+7,fy-61,3,6,HD_STONE);
  hd_rect(p,x-3,fy-42,6,11,HD_PAPER);
  hd_rect(p,x-1,fy-42,2,12,HD_BARK);
  hd_rect(p,x+5,fy-34,4,6,HD_PAPER);
 }else if(role==4){
  hd_rect(p,x-7,fy-63,14,3,hair);
  hd_line(p,x-4,fy-44,x-5,fy-29,HD_PLASTER);
  hd_line(p,x+5,fy-44,x+6,fy-32,HD_PLASTER);
 }else if(role==5){
  plane(p,x,fy-65,-5,5,-9,7,4,hair);
  hd_rect(p,x-9,fy-61,2,8,HD_STONE_DARK);
  hd_rect(p,x+7,fy-61,2,5,HD_STONE);
  hd_line(p,x-6,fy-63,x+4,fy-63,HD_STONE_DARK);
 }else{
  plane(p,x,fy-67,-4,2,-9,7,4,hair);
  plane(p,x,fy-63,-10,7,-7,5,3,hair);
  hd_rect(p,x-9,fy-61,2,8,hair);
  hd_rect(p,x+7,fy-61,2,5,hair);
  hd_line(p,x-6,fy-65,x+1,fy-65,role==1?HD_ROAD_COOL:HD_STONE);
 }
 if(role==1){
  hd_line(p,x-6,fy-42,x+7,fy-24,HD_BARK);
  hd_line(p,x-5,fy-42,x+8,fy-24,HD_COPPER);
  hd_rect(p,x+5,fy-26,7,8,HD_BARK);
  hd_rect(p,x+6,fy-25,5,2,HD_COPPER);
 }else if(role==5){
  hd_line(p,x-5,fy-44,x-1,fy-36,HD_PAPER);
  hd_line(p,x+5,fy-44,x+1,fy-36,HD_PAPER);
  hd_rect(p,x-2,fy-39,4,8,HD_GLASS);
  hd_rect(p,x+4,fy-33,4,4,HD_PAPER);
  hd_pixel(p,x+5,fy-32,HD_RED);
 }
 int blink=(frame%111u)>=105u;
 if(!blink){
  hd_rect(p,x-5,fy-57,3,1,HD_INK);
  hd_rect(p,x+3,fy-57,3,1,HD_INK);
  hd_pixel(p,x-4,fy-56,HD_PAPER);
  hd_pixel(p,x+4,fy-56,HD_SKIN_LIT);
 }else{
  hd_line(p,x-5,fy-56,x-2,fy-56,HD_BARK);
  hd_line(p,x+3,fy-56,x+5,fy-56,HD_BARK);
 }
 if(role==3){
  hd_rect(p,x-6,fy-58,5,1,HD_INK);
  hd_rect(p,x+2,fy-58,5,1,HD_INK);
  hd_line(p,x-1,fy-57,x+2,fy-57,HD_INK);
 }
 hd_line(p,x+1,fy-56,x+1,fy-53,HD_BRICK_LIT);
 hd_pixel(p,x,fy-53,HD_SKIN_LIT);
 hd_pixel(p,x+2,fy-52,HD_BARK);
 if(emotion==7){
  hd_line(p,x-6,fy-60,x-2,fy-59,HD_BARK);
  hd_line(p,x+2,fy-59,x+6,fy-60,HD_BARK);
 }
 hd_rect(p,x-2,fy-50,5,1,HD_BARK);
 if(emotion==1&&((frame/11u)&1u))hd_rect(p,x-1,fy-50,3,2,HD_BRICK_DARK);
 else hd_rect(p,x-1,fy-49,3,1,HD_SKIN_LIT);
 if(emotion==5){
  hd_line(p,x+21,fy-39,x+9,fy-42,HD_SKIN);
  hd_line(p,x+21,fy-38,x+9,fy-41,HD_SKIN_LIT);
 }else if(emotion==7){
  hd_line(p,x+19,fy-57,x+6,fy-61,HD_SKIN);
  hd_line(p,x+19,fy-56,x+6,fy-60,HD_SKIN_LIT);
 }
}

void hd_actor_closeup(uint8_t *p,int x,int base,int role,
                      unsigned frame,int emotion){
 unsigned jacket=role==1?HD_RED:role==2?HD_GLASS:
                 role==3?HD_STONE_LIT:role==4?HD_LEAF_DARK:HD_PLASTER_LIT;
 unsigned shadow=role==1?HD_RED_SHADE:role==2?HD_ROAD_COOL:
                 role==3?HD_STONE_DARK:role==4?HD_SHADOW:HD_STONE;
 unsigned light=role==1?HD_BRICK_LIT:role==2?HD_GLASS_LIT:
                role==3?HD_PAPER:role==4?HD_LEAF:HD_PAPER;
 unsigned hair=role==2?HD_BARK:role==3?HD_STONE_DARK:
               role==4?HD_BARK:HD_INK;
 int y=base-(int)((frame/27u)&1u);
 /* The torso reaches the lower matte. Broad shadow shapes describe the
    shoulder and chest; a few seams are kept off the lit fabric planes. */
 plane(p,x,y-51,-12,12,-32,31,13,HD_INK);
 plane(p,x,y-38,-33,32,-29,29,33,HD_INK);
 plane(p,x,y-5,-29,29,-25,27,43,HD_INK);
 plane(p,x,y-49,-12,12,-30,29,12,shadow);
 plane(p,x,y-37,-31,30,-27,27,32,shadow);
 plane(p,x,y-5,-27,27,-23,25,43,shadow);
 plane(p,x,y-48,-12,7,-28,10,12,jacket);
 plane(p,x,y-36,-29,10,-24,6,74,jacket);
 plane(p,x,y-43,-23,-17,-21,-17,23,light);
 hd_line(p,x-21,y-16,x-17,y-4,shadow);
 hd_line(p,x+13,y-25,x+20,y-20,HD_INK);
 hd_rect(p,x+11,y-18,11,2,shadow);
 /* Full sleeves keep their old wrist contact zones. The outer contour
    stays continuous across bands instead of turning into ribbed tubes. */
 if(emotion!=5){
  plane(p,x,y-44,-31,-18,-36,-21,23,HD_INK);
  plane(p,x,y-21,-36,-21,-36,-21,18,HD_INK);
  plane(p,x,y-44,-30,-19,-35,-22,23,jacket);
  hd_rect(p,x-35,y-21,14,18,jacket);
  plane(p,x,y-42,-30,-28,-35,-32,21,light);
  hd_rect(p,x-35,y-21,3,16,light);
  hd_line(p,x-31,y-20,x-25,y-16,shadow);
  hd_rect(p,x-36,y-4,15,3,HD_STONE_LIT);
  hd_rect(p,x-35,y-1,11,6,HD_SKIN);
  hd_rect(p,x-33,y,6,2,HD_SKIN_LIT);
 }else{
  limb(p,x-25,y-39,x-29,y-20,x-15,y-42,10,HD_INK,jacket,light);
  limb(p,x-15,y-42,x-10,y-49,x-7,y-53,6,HD_BRICK_LIT,HD_SKIN,HD_SKIN_LIT);
 }
 if(emotion!=7){
  plane(p,x,y-44,18,31,21,36,23,HD_INK);
  hd_rect(p,x+21,y-21,16,18,HD_INK);
  plane(p,x,y-44,19,30,22,35,23,jacket);
  hd_rect(p,x+22,y-21,14,18,jacket);
  plane(p,x,y-42,28,30,32,35,21,shadow);
  hd_rect(p,x+32,y-21,4,17,shadow);
  hd_line(p,x+24,y-16,x+31,y-20,shadow);
  hd_rect(p,x+22,y-4,15,3,HD_STONE_LIT);
  hd_rect(p,x+24,y-1,11,6,HD_SKIN);
  hd_rect(p,x+25,y,6,2,HD_SKIN_LIT);
 }else{
  limb(p,x+25,y-40,x+31,y-25,x+23,y-47,10,HD_INK,jacket,shadow);
  limb(p,x+23,y-47,x+20,y-56,x+14,y-61,6,HD_BRICK_LIT,HD_SKIN,HD_SKIN_LIT);
 }
 /* Neck and collars remain behind the chin. */
 plane(p,x,y-57,-7,7,-6,8,17,HD_BRICK_LIT);
 plane(p,x,y-55,-5,1,-4,3,16,HD_SKIN);
 plane(p,x,y-44,-6,7,-2,2,9,HD_INK);
 plane(p,x,y-47,-14,-6,-6,-1,15,light);
 plane(p,x,y-47,7,14,1,7,15,shadow);
 hd_line(p,x-1,y-31,x-2,y+19,shadow);
 hd_rect(p,x,y-24,2,2,HD_COPPER);
 hd_rect(p,x,y-11,2,2,HD_COPPER);
 if(role==1){
  limb(p,x+23,y-41,x+8,y-16,x-10,y+18,3,HD_INK,HD_BARK,HD_COPPER);
  hd_rect(p,x+11,y+5,16,14,HD_BARK);
  hd_rect(p,x+12,y+6,14,3,HD_COPPER);
  hd_rect(p,x+17,y+9,4,3,HD_GOLD);
 }else if(role==2){
  hd_line(p,x-6,y-41,x+1,y-29,HD_COPPER);
  hd_line(p,x+8,y-41,x+1,y-29,HD_COPPER);
  hd_rect(p,x,y-29,3,4,HD_GOLD);
  hd_rect(p,x+13,y-27,3,3,HD_GOLD);
 }else if(role==3){
  plane(p,x,y-40,-4,4,-3,3,23,HD_PAPER);
  plane(p,x,y-36,-1,1,-3,2,19,HD_BARK);
  hd_rect(p,x+12,y-25,8,10,HD_PAPER);
  hd_rect(p,x+14,y-22,4,2,HD_COPPER);
 }else if(role==5){
  plane(p,x,y-42,-5,5,-4,4,29,HD_GLASS);
  hd_line(p,x-16,y-42,x-4,y-25,HD_PAPER);
  hd_line(p,x+16,y-42,x+4,y-25,HD_PAPER);
  hd_line(p,x-11,y-41,x-10,y-29,HD_INK);
  hd_line(p,x-10,y-29,x+12,y-24,HD_INK);
  hd_line(p,x+12,y-24,x+14,y-39,HD_INK);
  hd_ellipse(p,x+12,y-24,3,3,HD_INK);
  hd_ellipse(p,x+12,y-24,2,2,HD_STONE_LIT);
  hd_rect(p,x+19,y-24,7,8,HD_PAPER);
  hd_pixel(p,x+22,y-21,HD_RED);
 }
 /* Back hair / hood silhouettes are deliberately different by role. */
 if(role==2){
  plane(p,x,y-102,-8,9,-21,21,13,hair);
  plane(p,x,y-89,-22,22,-20,26,38,hair);
  plane(p,x,y-51,-20,-14,-15,-13,17,hair);
  plane(p,x,y-51,19,26,18,22,17,hair);
  plane(p,x,y-84,18,22,21,25,39,HD_HAIR);
 }else if(role==4){
  plane(p,x,y-105,-5,5,-25,23,21,HD_SHADOW);
  plane(p,x,y-84,-25,23,-26,26,40,HD_SHADOW);
  plane(p,x,y-98,-6,6,-22,20,16,HD_LEAF_DARK);
  plane(p,x,y-82,-23,21,-22,23,35,HD_LEAF_DARK);
 }
 /* Hand-authored forehead, cheek and jaw planes replace the balloon
    face. One broad lit plane and one quiet shadow preserve the volumes. */
 plane(p,x,y-96,-8,8,-18,17,12,HD_BARK);
 plane(p,x,y-84,-18,17,-16,18,24,HD_BARK);
 plane(p,x,y-60,-16,18,-6,8,15,HD_BARK);
 plane(p,x,y-94,-8,7,-16,15,11,HD_SKIN);
 plane(p,x,y-83,-16,15,-14,16,23,HD_SKIN);
 plane(p,x,y-60,-14,16,-5,7,13,HD_SKIN);
 plane(p,x,y-89,-8,0,-14,1,13,HD_SKIN_LIT);
 plane(p,x,y-76,-14,0,-10,0,18,HD_SKIN_LIT);
 plane(p,x,y-58,-10,1,-5,2,9,HD_SKIN_LIT);
 plane(p,x,y-83,11,15,9,16,24,HD_BRICK_LIT);
 plane(p,x,y-59,9,16,4,7,12,HD_BRICK_LIT);
 hd_rect(p,x-19,y-74,4,9,HD_SKIN);
 hd_rect(p,x-19,y-71,2,5,HD_BRICK_LIT);
 hd_rect(p,x+16,y-73,4,8,HD_BRICK_LIT);
 hd_pixel(p,x+17,y-70,HD_SKIN);
 if(role==2){
  plane(p,x,y-102,-8,9,-20,18,10,hair);
  plane(p,x,y-92,-20,18,-18,-3,11,hair);
  plane(p,x,y-81,-18,-11,-16,-14,17,hair);
  plane(p,x,y-91,13,19,17,21,23,hair);
  hd_line(p,x-13,y-96,x-2,y-99,HD_HAIR);
  hd_line(p,x-16,y-91,x-8,y-94,HD_HAIR);
  hd_line(p,x+19,y-71,x+22,y-53,HD_HAIR);
  hd_rect(p,x-17,y-65,2,3,HD_GOLD);
 }else if(role==3){
  plane(p,x,y-100,-7,8,-19,16,8,hair);
  plane(p,x,y-92,-19,16,-17,12,8,hair);
  hd_rect(p,x-19,y-86,4,15,hair);
  plane(p,x,y-90,13,18,15,19,17,HD_STONE);
  hd_line(p,x-13,y-96,x-1,y-98,HD_STONE);
  hd_line(p,x-15,y-93,x-6,y-95,HD_STONE_LIT);
 }else if(role==4){
  plane(p,x,y-96,-7,7,-16,15,9,hair);
  plane(p,x,y-87,-16,15,-14,6,5,hair);
  hd_rect(p,x-17,y-83,3,13,hair);
 }else if(role==5){
  plane(p,x,y-101,-7,6,-17,15,8,hair);
  plane(p,x,y-93,-17,15,-18,16,7,hair);
  plane(p,x,y-86,-18,-13,-17,-15,15,HD_STONE_DARK);
  plane(p,x,y-86,13,17,15,18,12,HD_STONE);
  hd_line(p,x-12,y-96,x+4,y-97,HD_STONE_DARK);
  hd_line(p,x-14,y-91,x-4,y-93,HD_ROAD_COOL);
 }else{
  plane(p,x,y-104,-4,3,-14,12,7,hair);
  plane(p,x,y-97,-14,12,-21,17,8,hair);
  plane(p,x,y-89,-21,17,-16,9,7,hair);
  hd_rect(p,x-19,y-84,4,15,hair);
  hd_rect(p,x+14,y-86,4,9,hair);
  unsigned glint=HD_ROAD_COOL;
  hd_line(p,x-9,y-99,x+1,y-101,glint);
  hd_line(p,x-15,y-94,x-4,y-98,glint);
 }
 /* Narrow lids and displaced pupils provide a gaze without giant white
    square eyes. Highlights occupy one pixel, never an entire eyeball. */
 int gaze=emotion==7?-1:emotion==5?1:0;
 int blink=((frame+(unsigned)role*17u)%127u)>=122u;
 if(blink){
  hd_line(p,x-12,y-74,x-5,y-73,HD_BARK);
  hd_line(p,x+5,y-73,x+12,y-74,HD_BARK);
 }else{
  hd_rect(p,x-12,y-75,8,3,HD_PAPER);
  hd_rect(p,x+5,y-75,8,3,HD_SKIN_LIT);
  hd_rect(p,x-9+gaze,y-75,3,3,HD_INK);
  hd_rect(p,x+8+gaze,y-75,3,3,HD_INK);
  hd_line(p,x-13,y-76,x-5,y-76,HD_INK);
  hd_line(p,x+5,y-76,x+13,y-75,HD_INK);
  hd_pixel(p,x-8+gaze,y-74,HD_GLASS);
  hd_pixel(p,x+9+gaze,y-74,HD_GLASS);
  hd_line(p,x-12,y-71,x-6,y-71,HD_BRICK_LIT);
 }
 if(emotion==7){
  hd_line(p,x-13,y-81,x-5,y-79,HD_BARK);
  hd_line(p,x+5,y-79,x+13,y-82,HD_BARK);
 }else{
  hd_line(p,x-13,y-81,x-5,y-81,HD_BARK);
  hd_line(p,x+5,y-81,x+13,y-80,HD_BARK);
 }
 if(role==3){
  hd_rect(p,x-14,y-78,11,1,HD_INK);
  hd_rect(p,x-14,y-78,1,9,HD_INK);
  hd_rect(p,x-13,y-69,9,1,HD_INK);
  hd_rect(p,x-4,y-77,1,8,HD_INK);
  hd_rect(p,x+3,y-78,12,1,HD_INK);
  hd_rect(p,x+3,y-77,1,8,HD_INK);
  hd_rect(p,x+14,y-77,1,8,HD_INK);
  hd_rect(p,x+4,y-69,10,1,HD_INK);
  hd_line(p,x-3,y-75,x+3,y-75,HD_INK);
 }
 hd_line(p,x+2,y-73,x+4,y-63,HD_BRICK_LIT);
 hd_line(p,x+3,y-63,x-1,y-62,HD_BARK);
 hd_rect(p,x,y-65,2,2,HD_SKIN_LIT);
 hd_pixel(p,x+5,y-63,HD_BARK);
 if(emotion==1&&((frame/10u)&1u)){
  hd_line(p,x-5,y-56,x+6,y-56,HD_BARK);
  hd_rect(p,x-3,y-55,8,3,HD_BRICK_DARK);
  hd_rect(p,x-1,y-52,5,1,HD_SKIN_LIT);
 }else if(emotion==5){
  hd_line(p,x-5,y-55,x+6,y-54,HD_BARK);
  hd_pixel(p,x+7,y-55,HD_BRICK_LIT);
  hd_rect(p,x-2,y-52,5,1,HD_SKIN_LIT);
 }else{
  hd_line(p,x-5,y-55,x+6,y-55,HD_BARK);
  hd_rect(p,x-2,y-52,5,1,HD_SKIN_LIT);
 }
 if(role==1){
  hd_line(p,x-10,y-55,x-7,y-50,HD_BRICK_LIT);
  hd_line(p,x+8,y-52,x+11,y-57,HD_BARK);
  hd_rect(p,x-1,y-48,5,1,HD_BRICK_LIT);
 }
 /* Fingers lie in front of the cheek at the end of a real forearm. */
 if(emotion==5){
  plane(p,x,y-56,-9,-4,-11,-5,7,HD_SKIN);
  hd_line(p,x-8,y-56,x-8,y-51,HD_SKIN_LIT);
 }else if(emotion==7){
  plane(p,x,y-66,12,15,13,18,9,HD_SKIN);
  hd_line(p,x+13,y-65,x+14,y-59,HD_SKIN_LIT);
 }
}
