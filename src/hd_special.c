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
 unsigned wall=window?HD_BRICK_DARK:HD_STONE_DARK;
 hd_rect(p,0,0,240,114,wall);
 hd_rect(p,0,110,240,50,HD_BARK);
 hd_rect(p,0,107,240,4,HD_COPPER);
 for(int y=118;y<160;y+=8)hd_line(p,0,y,239,y,HD_BRICK_DARK);
 for(int x=13;x<240;x+=29)
  hd_line(p,x,112,x-9,159,HD_BRICK_DARK);
 /* The same skyline through each reachable window. */
 hd_rect(p,16,12,108,84,HD_INK);
 hd_rect(p,19,16,101,76,HD_GLASS);
 for(int i=0;i<9;i++){
  int x=20+i*12,h=26+(int)(hash((unsigned)i,23u)%31u);
  hd_rect(p,x,92-h,12,h,HD_STONE_DARK);
  for(int k=4;k<h-2;k+=10)
   hd_rect(p,x+4,92-h+k,3,3,
          (((unsigned)i+(unsigned)k+t/100u)%7u)==0u?HD_LAMP:HD_GLASS_LIT);
 }
 hd_rect(p,70,16,3,80,HD_BARK);
 hd_rect(p,19,66,105,3,HD_BARK);
 hd_rect(p,15,12,5,85,HD_BARK);
 hd_rect(p,119,12,5,85,HD_BARK);
 hd_rect(p,15,95,110,5,HD_BRICK);
 hd_line(p,14,11,124,11,HD_GOLD);
 hd_rect(p,130,21,100,4,HD_COPPER);
}
static void rainy_lane(uint8_t *p,unsigned t){
 for(int y=0;y<160;y++)
  hd_rect(p,0,y,240,1,y<30?19:y<75?HD_GLASS:
                               y<105?HD_STONE_DARK:HD_ROAD);
 for(int i=0;i<14;i++){
  int x=i*19-7,h=26+(int)(hash((unsigned)i,8u)%28u);
  hd_rect(p,x,109-h,19,h,HD_BRICK_DARK);
  hd_rect(p,x+2,109-h,15,3,HD_BRICK);
  for(int yy=109-h+8;yy<106;yy+=10)
   if(((i+yy)&3)!=1)hd_rect(p,x+6,yy,4,4,HD_GLASS_LIT);
 }
 hd_rect(p,0,106,240,4,HD_STONE);
 hd_rect(p,0,110,240,3,HD_BRICK_DARK);
 for(int i=-1;i<16;i++){
  int x=i*19;
  hd_line(p,x,114,x-6,159,HD_ROAD_COOL);
 }
 for(int y=115;y<160;y+=7)
  for(int x=3;x<240;x+=31){
   hd_line(p,x+(y&7),y,x+11+(y&7),y,HD_ROAD_LIGHT);
   hd_pixel(p,x+5,y+2,HD_REFLECT);
  }
 hd_line(p,0,80,239,80,HD_ROAD_COOL);
 hd_rect(p,216,50,3,67,HD_INK);
 hd_rect(p,210,50,16,4,HD_COPPER);
 hd_rect(p,213,54,10,4,HD_LAMP);
 for(int k=0;k<4;k++)
  hd_line(p,204+k*7,126,198+k*9,146,
           k==1?HD_REFLECT:HD_ROAD_LIGHT);
 (void)t;
}
static void paper(uint8_t *p,int x,int y,int w,int h,unsigned t){
 hd_rect(p,x+2,y+2,w,h,HD_SHADOW);
 hd_rect(p,x,y,w,h,HD_PAPER);
 hd_rect(p,x+1,y+1,w-2,2,HD_STONE_LIT);
 for(int i=0;i<4;i++)
  hd_line(p,x+4,y+7+i*5,x+w-5-(i&1)*7,y+7+i*5,HD_STONE);
 hd_rect(p,x+6,y+5,9,1,HD_RED_SHADE);
 if((t/120u)&1u)hd_rect(p,x+w-10,y+h-8,5,4,HD_COPPER);
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
 paper(p,126,112,20,12,t);
 hd_actor_stage(p,59,143,1,t,t<78u?2:3);
 hd_actor_stage(p,179,143,4,t,t<65u?0:3);
 if(t>90u){
  hd_rect(p,129,104,13,9,HD_PAPER);
  hd_rect(p,131,105,9,1,HD_COPPER);
 }
}
static void safehouse(uint8_t *p,unsigned t,int scene){
 inside(p,t,1);
 /* A real wooden door, hinges, floor threshold and inviting tungsten lamp. */
 hd_rect(p,144,13,83,115,HD_INK);
 hd_rect(p,148,17,75,106,HD_BARK);
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
    hd_ellipse(p,mx,145,16,3,HD_SHADOW);
    hd_actor_stage(p,mx,145,2,t,t>=145u?2:t>=123u?1:0);
   }
  }
  hd_rect(p,153,23,63-open,96,HD_BARK);
  hd_rect(p,156,27,56-open,88,HD_BRICK_DARK);
  hd_rect(p,155,57,60-open,2,HD_COPPER);
  if(open<20)hd_rect(p,204-open/2,82,5,5,HD_LAMP);
  hd_actor_stage(p,63,144,1,t,t<67u?2:t<107u?3:0);
  if(open){
   /* Practical hall light and reflected threshold share the opening. */
   hd_line(p,156,118,200,118,HD_LAMP);
   hd_rect(p,156,119,44,3,HD_STONE_LIT);
   for(int i=0;i<4;i++)
    hd_line(p,161+i*9,122,154+i*11,141,HD_REFLECT);
  }
 }else{
  hd_actor_stage(p,49,144,1,t,0);
  hd_actor_stage(p,113,144,4,t,t>78u?1:0);
 }
}
static void empty_cafe(uint8_t *p,unsigned t){
 rainy_lane(p,t);
 hd_rect(p,8,38,118,76,HD_BRICK_DARK);
 for(int y=45;y<112;y+=8){
  hd_line(p,9,y,124,y,HD_BRICK);
  for(int x=16+(y/8%2)*8;x<124;x+=19)
   hd_line(p,x,y,x,y+6,HD_BRICK);
 }
 hd_rect(p,13,54,109,10,HD_GOLD);
 hd_rect(p,14,55,107,8,HD_BARK);
 hd_text(p,24,56,"MARA'S CAFE",HD_PAPER,1,1);
 hd_rect(p,21,70,95,39,HD_INK);
 hd_rect(p,25,73,87,33,HD_GLASS_LIT);
 hd_rect(p,64,73,3,33,HD_BARK);
 hd_rect(p,24,90,90,3,HD_COPPER);
 hd_rect(p,99,119,92,7,HD_BARK);
 hd_rect(p,105,125,7,24,HD_BARK);
 hd_rect(p,178,125,7,24,HD_BARK);
 hd_rect(p,114,110,15,9,HD_PAPER);
 hd_rect(p,113,109,17,2,HD_LAMP);
 hd_rect(p,147,114,10,13,HD_INK);
 hd_rect(p,149,116,6,9,HD_GLASS);
 if((t/70u)&1u)hd_rect(p,151,118,2,1,HD_LAMP);
 /* Mara is absent. No unearned implication that she died or recovered. */
 hd_rect(p,198,112,27,4,HD_BARK);
 hd_rect(p,201,115,3,26,HD_BARK);
 hd_rect(p,219,115,3,26,HD_BARK);
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
static void good_record(uint8_t *p,unsigned t){
 inside(p,t,1);
 for(int i=0;i<8;i++){
  int x=133+(i%4)*25,y=33+(i/4)*32;
  hd_rect(p,x,y,21,26,HD_BARK);
  hd_rect(p,x+2,y+2,17,22,HD_GLASS);
  hd_ellipse(p,x+10,y+12,7,7,HD_INK);
  hd_ellipse(p,x+10,y+12,2,2,HD_GOLD);
 }
 hd_rect(p,18,109,207,9,HD_BARK);
 hd_rect(p,25,118,7,36,HD_BARK);
 hd_rect(p,209,118,7,36,HD_BARK);
 hd_ellipse(p,83,108,37,13,HD_INK);
 hd_ellipse(p,83,108,30,10,HD_STONE_DARK);
 hd_ellipse(p,83,108,6,3,HD_GOLD);
 int angle=(int)(t/16u&3u);
 hd_line(p,83,108,83+((angle&1)?7:-7),105,HD_PAPER);
 hd_line(p,132,87,117,100,HD_COPPER);
 hd_rect(p,132,84,4,14,HD_COPPER);
 hd_actor_stage(p,43,143,2,t,5);
 hd_text(p,144,15,"DANNY'S MASTER",HD_PAPER,1,0);
}
static void concert(uint8_t *p,unsigned t){
 rainy_lane(p,t);
 hd_rect(p,0,82,240,53,HD_INK);
 hd_rect(p,22,38,196,8,HD_BRICK);
 hd_rect(p,24,43,4,52,HD_BRICK);
 hd_rect(p,212,43,4,52,HD_BRICK);
 for(int i=0;i<5;i++){
  int x=31+i*43;
  hd_rect(p,x,42,5,10,HD_GOLD);
  hd_line(p,x+2,52,116+(int)(t/50u&3u)*2,101,
          i&1?HD_GLASS_LIT:HD_LAMP);
 }
 hd_rect(p,18,119,202,10,HD_BRICK_DARK);
 for(int i=0;i<19;i++){
  int x=i*14-6;
  int y=141+(int)(((unsigned)(i*7)+t/23u)&3u);
  hd_ellipse(p,x,y,7,7,i%3u?HD_STONE_DARK:HD_HAIR);
  hd_rect(p,x-6,y+4,13,19,i%4u?HD_GLASS:HD_BRICK);
 }
 hd_actor_stage(p,121,121,3,t,1);
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
