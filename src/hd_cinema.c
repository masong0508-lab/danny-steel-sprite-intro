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
  if((t/80u+i+2u)%3u==0u)hd_pixel(p,xx,y-13,HD_PLASTER_LIT);
 }
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
 if(page>=2u)hd_rect(p,151,83,19,4,HD_RED_SHADE);
 actor(p,ease(64,76,t,8,60),137,1,t,page==0?2:1);
 actor(p,193,135,3,t,page&1u?1:3);
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
 actor(p,38,136,1,t,t<80?2:3);
}
static void rooftop(uint8_t *p,unsigned t){
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
 actor(p,ease(78,101,t,7,65),142,1,t,t<80?2:3);
}
static void clinic(uint8_t *p,unsigned t,int last){
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
 actor(p,68,135,1,t,last?7:1);
 if(last)actor(p,115,136,2,t,5);
 actor(p,198,133,5,t,1);
}
void hd_cinema_draw(uint8_t *p,const Story *s,unsigned t){
 unsigned sc=s->cinema,page=s->page;
 switch(sc){
  case CINE_OPEN:
   street(p,t,0);
   bus(p,ease(258,125,t,0,130),71,t);
   actor(p,ease(68,86,t,12,65),140,1,t,t<75?2:0);
   hd_rect(p,214,94,3,26,HD_BARK);
   hd_rect(p,205,88,22,9,HD_COPPER);
   hd_text(p,209,89,"BUS",HD_INK,1,0);
   break;
  case CINE_LETTER:
   street(p,t,1);
   hd_rect(p,10,69,85,46,HD_BARK);
   hd_rect(p,12,72,80,41,HD_BRICK);
   hd_rect(p,21,78,40,34,HD_INK);
   hd_rect(p,24,81,34,29,HD_GLASS_LIT);
   hd_rect(p,72,81,14,24,HD_PAPER);
   hd_rect(p,74,83,10,2,HD_COPPER);
   actor(p,ease(162,142,t,10,55),145,1,t,page>=1?3:2);
   break;
  case CINE_BORE:archive(p,t,page);break;
  case CINE_MARA:
  case CINE_STAY:
   cafe(p,t);
   table(p,121,110,t);
   actor(p,ease(78,91,t,11,60),141,1,t,t<70?2:page>=1?5:1);
   actor(p,ease(178,164,t,18,63),141,2,t,t<82?2:page>=2?3:1);
   break;
  case CINE_BUS:
   street(p,t,1);
   bus(p,ease(260,74,t,0,116),67,t);
   actor(p,ease(192,165,t,20,78),142,1,t,t<100?2:page>=2?7:0);
   break;
  case CINE_MUSIC:record_room(p,t);break;
  case CINE_ART:rooftop(p,t);break;
  case CINE_NOTICE:{
   street(p,t,1);
   int x=ease(68,91,t,13,71);
   actor(p,x,142,1,t,t<80?2:7);
   hd_rect(p,x+21,96,11,19,HD_INK);
   hd_rect(p,x+23,98,7,15,HD_GLASS_LIT);
   hd_pixel(p,x+26,101,HD_LAMP);
   break;
  }
  case CINE_CLINIC:clinic(p,t,0);break;
  case CINE_LAST_CLINIC:clinic(p,t,1);break;
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
