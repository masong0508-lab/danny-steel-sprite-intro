#include "cinema.h"
/* Story cinematics, 120 x 80 @ 8bpp in the game's existing indexed palette.
   Every scene reuses world-appropriate backgrounds and actors. There are no
   image assets, framebuffer allocations, heap accesses or per-frame trig. */
#define W 120
#define H 80
static void pixel(uint8_t *p,int x,int y,int c){
 if((unsigned)x<W&&(unsigned)y<H)p[y*W+x]=(uint8_t)c;
}
static void rect(uint8_t *p,int x,int y,int w,int h,int c){
 int x1=x+w,y1=y+h;
 if(x<0)x=0;
 if(y<0)y=0;
 if(x1>W)x1=W;
 if(y1>H)y1=H;
 if(x>=x1||y>=y1)return;
 for(int j=y;j<y1;j++)for(int i=x;i<x1;i++)p[j*W+i]=(uint8_t)c;
}
static int abs_i(int a){return a<0?-a:a;}
static void stroke(uint8_t *p,int x0,int y0,int x1,int y1,int c){
 int dx=abs_i(x1-x0),dy=abs_i(y1-y0),sx=x0<x1?1:-1,sy=y0<y1?1:-1,err=dx-dy;
 for(;;){pixel(p,x0,y0,c);if(x0==x1&&y0==y1)break;
 int e=err*2;if(e>-dy){err-=dy;x0+=sx;}if(e<dx){err+=dx;y0+=sy;}}
}
/* Timed easing gives each actor a start, travel beat and held pose. */
static int travel(int from,int to,unsigned t,unsigned start,unsigned duration){
 if(t<=start)return from;
 if(t-start>=duration)return to;
 unsigned q=(t-start)*256u/duration;
 unsigned eased=q*q*(768u-2u*q)>>16;
 return from+(to-from)*(int)eased/256;
}
static int breathe(unsigned t){
 unsigned pos=t%144u;
 return pos<72u?(int)pos/18-2:2-(int)(pos-72u)/18;
}
static void disk(uint8_t *p,int x,int y,int r,int c){
 for(int yy=-r;yy<=r;yy++)for(int xx=-r;xx<=r;xx++)
  if(xx*xx+yy*yy<=r*r)pixel(p,x+xx,y+yy,c);
}
/* Consistent distant skyline, rooftop heights, scrolling clouds and window
   glints. Foreground has independent motion; backgrounds never jump locations. */
static void city(uint8_t *p,unsigned t,int dusk){
 for(int y=0;y<H;y++)rect(p,0,y,W,1,y<19?(dusk?17:26):y<38?(dusk?25:27):y<53?29:9);
 disk(p,97,13,dusk?6:8,dusk?18:20);
 /* Clouds wrap behind the camera: no sudden teleport on a visible frame. */
 unsigned cloud_shift=(t/24u)%160u;
 for(int i=0;i<5;i++){
  int x=(int)(((unsigned)(i*35+45)+160u-cloud_shift)%160u)-29;
  if(x>=W||x+24<=0)continue;
  rect(p,x,9+(i&1)*4,24,2,dusk?29:20);
  rect(p,x+5,7+(i&1)*4,14,2,dusk?29:20);
 }
 /* Repeating skyline has period 16*10=160; no sudden height changes
    at the modulo seam, and invisible buildings are skipped. */
 int skyline_shift=(int)((t/85u)%160u);
 for(int i=-1;i<29;i++){
  int index=i&15;
  int x=i*10-3-skyline_shift;
  if(x>=W||x+10<=0)continue;
  int ht=14+((index*13+3)%22);
  rect(p,x,52-ht,10,ht,29);
  rect(p,x+2,53-ht,2,1,4);
  for(int j=0;j<4;j++){
   int wy=53-ht+5+j*6;
   if(wy<51&&((index+j+(int)(t/80u))&3)!=0)
    pixel(p,x+3,wy,dusk?18:7);
  }
 }
 /* Rooftop water towers, antenna and cabling; static at a given camera angle. */
 for(int i=0;i<3;i++){
  int cx=18+i*45;
  stroke(p,cx,25,cx,45,3);stroke(p,cx-4,27,cx,25,14);
  stroke(p,cx,25,cx+5,28,14);
  rect(p,cx-5,43,10,6,14);rect(p,cx-4,42,8,1,22);
  pixel(p,cx,24,((t/37+i)&1)?18:7);
 }
 int near_shift=(int)((t/110u)%184u);
 for(int i=-1;i<15;i++){
  int index=i&7;
  int x=i*23-12-near_shift;
  if(x>=W||x+22<=0)continue;
  int ht=13+((index*7)%16);
  rect(p,x,57-ht,22,ht,3);
  for(int w=3;w<20;w+=6)
   for(int y=60-ht+4;y<53;y+=7)
    rect(p,x+w,y,2,2,dusk?7:18);
 }
}
static void street(uint8_t *p,unsigned t,int dusk){
 city(p,t,dusk);
 rect(p,0,57,W,23,9);
 rect(p,0,57,W,2,6);
 for(int i=0;i<12;i++)stroke(p,i*15-40,80,60+(i-6)*4,58,10);
 for(int i=0;i<4;i++)stroke(p,0,60+i*i*2,119,60+i*i*2,8);
 /* Street lamp anchored to the same city and cross-street. */
 rect(p,109,19,2,40,14);rect(p,104,19,8,2,14);disk(p,108,20,3,dusk?18:7);
}
/* One identifiable café, whether Mara is present or missing. */
static void cafe(uint8_t *p,unsigned t){
 street(p,t,1);
 rect(p,3,16,33,39,14);rect(p,6,20,27,29,23);
 rect(p,7,18,24,7,18);rect(p,9,25,21,4,22);
 rect(p,9,32,10,15,3);rect(p,21,32,10,15,3);
 rect(p,11,34,6,10,25);rect(p,23,34,6,10,25);
 rect(p,18,28,2,20,14);rect(p,5,52,32,2,7);
 for(int i=0;i<4;i++)rect(p,5+i*8,15,4,3,i&1?18:14);
}
static void cafe_table(uint8_t *p,int cup){
 rect(p,52,59,18,2,14);rect(p,60,61,2,9,14);
 if(cup){rect(p,59,55,6,4,20);pixel(p,66,57,18);
  if(cup==2)pixel(p,61,54,26);}
}
static void interior(uint8_t *p,int mode,unsigned t){
 int wall=mode==0?17:mode==1?21:mode==2?12:8;
 rect(p,0,0,W,H,wall);rect(p,0,54,W,26,mode==2?11:9);
 for(int x=0;x<W;x+=20){rect(p,x,0,1,53,mode==1?23:29);}
 rect(p,8,9,47,37,3);rect(p,11,12,41,31,26);
 /* A genuine view onto the same animated Old Quarter. */
 /* The view from this window stays spatially anchored, while distant
    lights animate quietly instead of shifting the entire street. */
 for(int i=0;i<6;i++){
  int x=11+i*8,h=(i*9+7)%19+7;
  rect(p,x,44-h,8,h,29);
  if(i&1)rect(p,x+3,40-h,2,2,18);
 }
 if((t/180u)&1u)rect(p,24,28,2,2,18);
 rect(p,8,42,47,3,14);rect(p,30,9,2,36,14);
 rect(p,0,52,W,2,mode==2?13:14);
}
static void paper(uint8_t *p,int x,int y,int w,int h,int blink){
 rect(p,x+1,y+1,w,h,3);rect(p,x,y,w,h,20);
 for(int i=0;i<4 && 4+i*4<h;i++)rect(p,x+3,y+4+i*4,w-6,1,i==blink%4?18:22);
}
static void furniture(uint8_t *p,int x,int y,int w,int c){
 rect(p,x+1,y,w-2,4,7);rect(p,x,y,w,3,c);
 rect(p,x+5,y+4,4,22,14);rect(p,x+w-9,y+4,4,22,14);
}
/* Full, expressive 2D silhouettes with hair, coat, eyes, blink and grounded
   feet/shadows; no abstract flying heads or randomly teleporting props. */
/* Six distinct character silhouettes with coordinated body parts.
   Roles 1 Danny / 2 Mara / 3 BORE clerk / 4 witness / 5 heckler / 6 doctor.
   Gesture 0 breathe / 1 talk / 2 walk / 3 reach right / 4 reach left /
   5 listen / 6 relieved / 7 worried. Faces and arms respond together. */
static void actor(uint8_t *p,int x,int foot,int role,unsigned t,int gesture){
 int moving=(gesture==2);
 int gait=moving?((int)(t/7)&1):0;
 /* Listening and emotional reactions settle into a stationary pose. */
 int bob=moving?gait:gesture>=5?0:((int)(t/31)&1);
 int coat=role==1?16:role==2?25:role==3?17:role==4?12:role==5?23:20;
 int trim=role==1?18:role==2?28:role==3?20:role==4?11:role==5?16:26;
 int hair=role==2?24:role==3?29:role==4?14:role==5?23:3;
 int skin=role==4?22:31;
 int shoulder=foot-27+bob;
 int brow=foot-39+bob;
 /* Grounded shadow, stride and separate boots. */
 rect(p,x-9,foot,18,1,3);
 if(moving){
  stroke(p,x-4,foot-12,x-6+gait*3,foot-2,4);
  stroke(p,x+4,foot-12,x+6-gait*3,foot-2,4);
  rect(p,x-9+gait*3,foot-2,7,2,3);
  rect(p,x+2-gait*3,foot-2,7,2,3);
 }else{
  rect(p,x-6,foot-13,5,11,4);rect(p,x+2,foot-13,5,11,4);
  rect(p,x-7,foot-2,7,2,3);rect(p,x+2,foot-2,7,2,3);
 }
 /* Tapered garment, cuffs and contrasting lapels. */
 rect(p,x-8,shoulder+2,16,12,coat);
 rect(p,x-7,shoulder+13,14,5,coat);
 rect(p,x-6,shoulder+18,12,2,trim);
 rect(p,x-10,shoulder+4,3,10,coat);
 rect(p,x+7,shoulder+4,3,10,coat);
 pixel(p,x-7,shoulder+4,trim);pixel(p,x+7,shoulder+4,trim);
 stroke(p,x-4,shoulder+2,x,shoulder+7,trim);
 stroke(p,x+4,shoulder+2,x,shoulder+7,trim);
 stroke(p,x,shoulder+7,x,shoulder+17,role==3?20:23);
 if(role==1){
  /* Danny's diagonal satchel and red jacket. */
  stroke(p,x-6,shoulder+4,x+4,shoulder+16,18);
  rect(p,x+4,shoulder+14,4,3,14);
  rect(p,x-4,shoulder+16,2,1,7);
 }else if(role==2){
  /* Mara's magenta neckline and hair framing. */
  rect(p,x-6,shoulder+12,12,2,28);
  pixel(p,x-3,shoulder+9,20);pixel(p,x+3,shoulder+9,20);
 }else if(role==3){
  /* Archive clerk ID badge and tie. */
  rect(p,x-5,shoulder+6,3,2,20);rect(p,x+3,shoulder+6,3,2,20);
  stroke(p,x,shoulder+6,x+1,shoulder+14,16);
 }else if(role==4){
  /* Witness' hood and zipper, no identifying details. */
  stroke(p,x-4,shoulder+2,x-4,shoulder+17,11);
  rect(p,x+3,shoulder+13,3,2,14);
 }else if(role==5)rect(p,x-5,shoulder+7,11,2,16);
 /* Shaded jaw and hair, ears, blinking eyes, spoken expression. */
 disk(p,x,foot-34+bob,8,3);
 disk(p,x,foot-35+bob,7,skin);
 rect(p,x-6,foot-32+bob,13,2,skin);
 pixel(p,x+7,foot-34+bob,22);
 rect(p,x-6,brow,12,4,hair);
 rect(p,x-7,brow+2,3,5,hair);
 if(role==2){
  rect(p,x-8,brow+3,3,12,24);rect(p,x+6,brow+3,3,13,24);
  pixel(p,x+1,brow-1,28);
 }else if(role==3){
  rect(p,x+4,brow+3,4,5,29);rect(p,x-5,brow,5,1,7);
 }else if(role==4){
  rect(p,x-8,brow+1,3,12,14);rect(p,x+6,brow+2,3,11,14);
  rect(p,x-6,brow-2,13,2,11);
 }else if(role==1){
  rect(p,x-5,brow-2,9,3,14);
  rect(p,x-7,brow,14,3,14);
  rect(p,x-7,brow+2,3,5,14);
  rect(p,x-6,foot-32+bob,2,4,14);
  rect(p,x+5,foot-32+bob,2,4,14);
  rect(p,x-5,foot-30+bob,11,3,23);
  rect(p,x-3,foot-27+bob,7,2,14);
 }else rect(p,x+4,brow+3,4,6,hair);
 if((t%97)<92){
  rect(p,x-4,foot-35+bob,2,1,3);
  rect(p,x+3,foot-35+bob,2,1,3);
 }else{
  rect(p,x-4,foot-34+bob,2,1,3);
  rect(p,x+3,foot-34+bob,2,1,3);
 }
 /* Facial acting: concerned brows and mouth, relieved smile, or speech.
    Every feature stays attached to the same character's face. */
 if(gesture==7){
  stroke(p,x-5,foot-38+bob,x-2,foot-36+bob,3);
  stroke(p,x+2,foot-36+bob,x+5,foot-38+bob,3);
 }else if(gesture==6){
  pixel(p,x-4,foot-36+bob,7);pixel(p,x+3,foot-36+bob,7);
 }
 pixel(p,x+1,foot-32+bob,22);
 if(gesture==7){
  rect(p,x-2,foot-28+bob,4,1,23);
  pixel(p,x-3,foot-27+bob,23);pixel(p,x+2,foot-27+bob,23);
 }else if(gesture==6){
  pixel(p,x-3,foot-30+bob,23);rect(p,x-2,foot-29+bob,4,1,23);
  pixel(p,x+2,foot-30+bob,23);
 }else if(gesture==1&&((t/9)&1))rect(p,x-2,foot-29+bob,4,2,23);
 else rect(p,x-2,foot-29+bob,4,1,role==2?16:23);
 /* Arms are attached to shoulders; hand never floats at head level. */
 if(gesture==3){
   int wrist=shoulder+11+((int)(t/18)&1);
   stroke(p,x+8,shoulder+4,x+16,wrist-2,coat);
   stroke(p,x+16,wrist-2,x+20,wrist,skin);
   disk(p,x+20,wrist,2,skin);
   stroke(p,x-8,shoulder+4,x-11,shoulder+15,coat);
   disk(p,x-11,shoulder+15,2,skin);
  }else if(gesture==4){
   int wrist=shoulder+11+((int)(t/18)&1);
   stroke(p,x-8,shoulder+4,x-16,wrist-2,coat);
   stroke(p,x-16,wrist-2,x-20,wrist,skin);
   disk(p,x-20,wrist,2,skin);
   stroke(p,x+8,shoulder+4,x+11,shoulder+15,coat);
   disk(p,x+11,shoulder+15,2,skin);
  
 }else if(gesture==5){
  /* Listening: quiet shoulders and hands hanging at the waist. */
  stroke(p,x-8,shoulder+4,x-11,shoulder+14,coat);
  stroke(p,x+8,shoulder+4,x+11,shoulder+14,coat);
  disk(p,x-11,shoulder+14,2,skin);
  disk(p,x+11,shoulder+14,2,skin);
 }else if(gesture==6){
  /* Relief: one hand against the chest; a small open reach. */
  stroke(p,x-8,shoulder+4,x-11,shoulder+12,coat);
  stroke(p,x-11,shoulder+12,x-2,shoulder+12,skin);
  disk(p,x-2,shoulder+12,2,skin);
  stroke(p,x+8,shoulder+4,x+13,shoulder+13,coat);
  disk(p,x+13,shoulder+13,2,skin);
 }else if(gesture==7){
  /* Concern: a shoulder/elbow/hand chain rises to the forehead. */
  stroke(p,x+8,shoulder+4,x+15,shoulder+1,coat);
  stroke(p,x+15,shoulder+1,x+5,brow+6,skin);
  disk(p,x+5,brow+6,2,skin);
  stroke(p,x-8,shoulder+4,x-11,shoulder+14,coat);
  disk(p,x-11,shoulder+14,2,skin);
 }else if(gesture==1){
  stroke(p,x+8,shoulder+4,x+13,shoulder-2+((int)(t/12)&3),coat);
  disk(p,x+13,shoulder-2+((int)(t/12)&3),2,skin);
  stroke(p,x-8,shoulder+4,x-10,shoulder+14,coat);
  disk(p,x-10,shoulder+14,2,skin);
 }else{
  int sway=moving?(gait?2:-2):(int)((t/20)&1u);
  stroke(p,x-8,shoulder+4,x-10+sway,shoulder+14,coat);
  stroke(p,x+8,shoulder+4,x+10-sway,shoulder+14,coat);
  disk(p,x-10+sway,shoulder+14,2,skin);
  disk(p,x+10-sway,shoulder+14,2,skin);
 }
}
static void bus(uint8_t *p,int x,int y,unsigned t){
 rect(p,x,y,73,29,18);rect(p,x+2,y+3,69,19,16);
 rect(p,x+6,y+5,28,12,25);rect(p,x+38,y+5,27,12,25);
 rect(p,x+34,y+3,2,22,3);rect(p,x+2,y+23,69,4,14);
 disk(p,x+17,y+29,7,3);disk(p,x+56,y+29,7,3);
 disk(p,x+17,y+29,3,22);disk(p,x+56,y+29,3,22);
 rect(p,x+69,y+13,3,4,((t/19)&1)?7:18);
}
static void roof(uint8_t *p,unsigned t,int dusk){
 city(p,t,dusk);
 rect(p,0,56,W,24,9);rect(p,0,54,W,3,14);
 for(int x=0;x<120;x+=22){rect(p,x,58,1,9,10);rect(p,x+11,68,1,8,10);}
 stroke(p,0,67,119,67,10);
 /* Tar seams, masonry, and an anchored rooftop drainpipe. */
 for(int i=0;i<8;i++){
  int rx=(i*27+9)%120;
  stroke(p,rx,66+(i&1)*3,rx+8,66+(i&1)*3,10);
 }
 rect(p,109,53,3,20,14);rect(p,107,70,8,2,3);
 /* Rail across the back, never in front of the canvas. */
 rect(p,0,42,120,2,14);for(int x=5;x<120;x+=23)rect(p,x,42,2,15,14);
}
static void canvas(uint8_t *p,int x,int y,unsigned t,int painted){
 rect(p,x-3,y-4,36,31,14);rect(p,x,y,30,23,20);
 /* The painting depicts the actual Old Quarter skyline. */
 rect(p,x+2,y+2,26,10,26);disk(p,x+21,y+6,3,7);
 for(int i=0;i<6;i++){
  int ht=4+(i*7)%7;rect(p,x+2+i*5,y+14-ht,5,ht,29);
  if(i%2==0)pixel(p,x+3+i*5,y+13-ht,7);
 }
 rect(p,x+1,y+17,28,5,11);
 if(painted){
  /* Paint accumulates on the actual canvas rather than randomly changing. */
  int strokes=(int)(t/55);if(strokes>7)strokes=7;
  for(int i=0;i<strokes;i++){
   int px=x+5+i*3,py=y+7+(i%3)*2;
   stroke(p,px,py,px+5,py+2,i&1?18:13);
  }
  pixel(p,x+17,y+9,7);
 }
 /* Wooden stretcher, struts, fasteners and cast shadow. */
 rect(p,x+13,y+27,3,21,14);rect(p,x+1,y+42,31,2,14);
 stroke(p,x+14,y+30,x+2,y+50,14);stroke(p,x+15,y+30,x+28,y+50,14);
}
static void studio(uint8_t *p,unsigned t){
 interior(p,1,t);
 rect(p,0,19,6,28,3);rect(p,1,21,4,24,22);
 rect(p,73,13,37,38,4);rect(p,75,15,33,34,17);
 rect(p,82,22,18,20,14);disk(p,91,31,9,3);disk(p,91,31,5,10);
 disk(p,91,31,2,20);
 for(int i=0;i<4;i++){
  unsigned period=(t/7u+(unsigned)i*5u)%24u;
  int level=(int)(period>12u?24u-period:period);
  rect(p,80+i*6,45-level/3,3,2+level/3,level>6?18:7);
 }
 furniture(p,15,51,94,14);
 disk(p,45,49,13,3);disk(p,45,49,9,22);
 static const int8_t spokes[8][2]={
  {6,0},{4,4},{0,6},{-4,4},{-6,0},{-4,-4},{0,-6},{4,-4}
 };
 unsigned phase=(t/9u)&7u;
 int dx=spokes[phase][0],dy=spokes[phase][1];
 stroke(p,45-dx,49-dy,45+dx,49+dy,20);
 disk(p,45,49,2,20);
 stroke(p,61,44,55+(int)((t/25u)%3u),46,20);
}
static void clinic(uint8_t *p,unsigned t,int last){
 interior(p,0,t);
 /* Same clinic in both visits: window, exam chair, chart and diagnosis. */
 rect(p,66,17,45,26,20);rect(p,68,19,41,22,26);
 for(int i=0;i<4;i++){
  int ht=2+(i*5)%10;stroke(p,72+i*8,33,72+i*8+5,33-ht,16);
 }
 rect(p,58,50,58,3,14);
 rect(p,67,45,11,5,22);rect(p,106,45,10,5,22);
 /* The medical form is physical and receives a hand gesture. */
 paper(p,88,43,15,10,last?2:0);
 if(last)rect(p,0,0,120,3,24);
}
static void server(uint8_t *p,unsigned t,int after){
 interior(p,1,t);
 rect(p,18,19,85,39,3);rect(p,21,22,79,32,25);
 rect(p,23,24,75,5,after?16:18);
 for(int i=0;i<5;i++){
  int x=25+(i*11+(int)t/15)%70;
  rect(p,x,33+(i*7)%15,7,2,i==4?16:20);
 }
 if(after){rect(p,56,32,24,12,17);rect(p,59,34,18,8,24);}
 else{rect(p,46,35,25,10,17);rect(p,48,37,21,6,20);}
 furniture(p,5,57,110,14);
}
static void safehouse(uint8_t *p,unsigned t){
 interior(p,2,t);
 rect(p,58,18,50,37,14);rect(p,61,22,44,30,12);
 rect(p,64,27,10,24,7);rect(p,80,27,10,24,7);rect(p,96,27,7,24,7);
 furniture(p,16,56,66,14);
 rect(p,37,46,12,8,18);rect(p,36,45,14,2,20);rect(p,40,43,4,2,31);
 for(int i=0;i<2;i++)pixel(p,41+(int)(t/12)%5,40-i*4,26);
}
static void recordshop(uint8_t *p,unsigned t){
 interior(p,1,t);
 for(int x=5;x<120;x+=16){
  rect(p,x,19,14,30,14);rect(p,x+2,21,10,25,22);
  rect(p,x+4,23,6,19,(x%3)?16:25);
 }
 furniture(p,8,56,104,14);
 disk(p,60,38,20,3);disk(p,60,38,17,22);
 disk(p,60,38,12,3);disk(p,60,38,7,18);
 static const int8_t label[8][2]={
  {9,0},{6,6},{0,9},{-6,6},{-9,0},{-6,-6},{0,-9},{6,-6}
 };
 unsigned phase=(t/17u)&7u;
 /* A moving lacquer glint is long enough to read at native GBA
    resolution, and runs across the record's actual groove circle. */
 stroke(p,60,38,60+label[phase][0],38+label[phase][1],7);
 disk(p,60,38,3,20);
 rect(p,94,31,4,20,7);rect(p,94,30,12,2,20);
 rect(p,9,8,100,5,21);
 rect(p,19,9,7,3,(t/160u)&1u?18:7);
}
/* Camera framing: wide -> two-shot -> actor reaction -> detail. The world and
   object positions remain coherent: panning is limited to a few pixels. */
/* World clock continues across every cut. The local blocking beat resets
   only for performers entering a new shot; it never restarts clouds, vinyl,
   studio meters or the city clock. The public API uses a shared clock. */
static void cinema_draw_stage(uint8_t *p,unsigned scene,unsigned tick,
                              unsigned beat,unsigned page,unsigned final){
 if(scene>=CINE_COUNT)scene=CINE_NONE;
 unsigned t=tick;
 int focus=(int)(page&3u);
 /* Sky motion continues even when the finale actor pose holds. */
 switch(scene){
 case CINE_OPEN: {
  street(p,t,0);
  /* The last bus ARRIVES and idles at the same kerb, rather than
     teleporting back to the entrance on a modulo counter. */
  int busx=travel(134,17,beat,0,126);
  bus(p,busx,26,t);
  actor(p,travel(39,45,beat,8,62),74,1,t,beat<70?2:0);
  /* Permanently anchored sign and stop post. */
  rect(p,104,39,2,28,14);
  rect(p,99,37,12,8,14);rect(p,101,39,8,4,18);
  break;
 }
  case CINE_LETTER: {
   street(p,t,1);
   rect(p,3,15,56,43,14);rect(p,5,18,52,38,23);
   rect(p,13,24,18,29,3);rect(p,16,26,12,24,17);
   rect(p,41,24,10,11,4);rect(p,43,26,6,7,26);
   rect(p,36,51,34,3,14);rect(p,39,54,3,13,14);
   rect(p,60,54,3,13,14);
   paper(p,46,50,13,7,focus);
   int x=travel(81,76,beat,8,38);
   actor(p,x,72,1,t,focus>=1?4:beat<48?2:0);
   if(focus>=1){stroke(p,x-20,56,57,53,31);rect(p,49,51,7,1,18);}
   break;
  }
 case CINE_BORE:
   interior(p,1,t);
   rect(p,65,4,47,8,3);rect(p,67,6,43,4,18);
   furniture(p,20,53,91,14);
   paper(p,58,49,18,7,focus);
   rect(p,78,49,6,5,14);rect(p,79,47,4,3,18);
   if(focus>=2){rect(p,64,50,9,4,16);rect(p,66,51,5,2,20);}
   actor(p,travel(28,33,beat,10,36),68,1,t,focus&1?1:0);
   actor(p,96,66,3,t,focus>=1?4:0);
   break;
 case CINE_MARA: {
   cafe(p,t);cafe_table(p,2);
   int dx=travel(34,43,beat,12,55),mx=travel(95,83,beat,18,57);
   actor(p,dx,72,1,t,beat<65?2:focus==1?5:focus>=2?3:1);
   actor(p,mx,72,2,t,beat<70?2:focus==0?1:focus>=2?4:5);
   if(focus>=2){rect(p,62,57,5,3,20);rect(p,63,58,3,1,18);}
   break;
  }
 case CINE_BUS: {
  street(p,t,1);
  bus(p,travel(136,16,beat,0,100),25,t);
  /* Danny approaches the parked bus and pauses before deciding. */
  int x=travel(108,92,beat,32,85);
  actor(p,x,74,1,t,beat<119?2:focus==2?0:1);
  break;
 }
  case CINE_STAY: {
  cafe(p,t);cafe_table(p,1);
  actor(p,travel(37,45,beat,10,45),72,1,t,beat<56?2:1);
  actor(p,travel(95,87,beat,25,61),72,2,t,beat<87?2:0);
  break;
 }
  case CINE_MUSIC: {
  studio(p,t);
  /* The engineer moves from the tape reel to the mixing desk, leaving
     the physical reel and controls fixed inside the same room. */
  int x=travel(17,30,beat,12,54);
  actor(p,x,73,1,t,beat<68?2:focus>=2?3:1);
  for(int i=0;i<5;i++){
   unsigned pulse=(t/8u+(unsigned)i*4u)%16u;
   int level=(int)(pulse>8u?16u-pulse:pulse);
   rect(p,81+i*5,19-level/3,2,2+level/3,level>4?18:7);
  }
  break;
 }
  case CINE_ART: {
    roof(p,t,0);canvas(p,70,20,t,1);
    int x=travel(37,46,beat,7,45);
    actor(p,x,76,1,t,beat<53?2:3);
    if(beat>=53){stroke(p,x+20,60,76,35,14);pixel(p,76,35+(int)((t/13)&1),18);}
    break;
   }
 case CINE_SERVER:
  server(p,t,0);actor(p,17,70,1,t,focus>=2?3:0);
  break;
 case CINE_WITNESS:
    street(p,t,1);
    rect(p,52,54,61,3,14);rect(p,58,57,4,9,14);
    rect(p,103,57,4,9,14);
    actor(p,travel(28,39,beat,12,51),72,1,t,beat<63?2:focus&1?1:0);
    actor(p,78,72,4,t,focus>=1?4:1);
    /* The witness's physical written clue is handed to Danny. */
    rect(p,58,58,7,4,20);
    if(focus>=1){rect(p,60,59,3,1,18);}
    break;
 case CINE_SAFE:
  safehouse(p,t);
  actor(p,33,67,1,t,focus>=1?0:1);
  actor(p,83,67,4,t,focus>=1?1:0);
  break;
 case CINE_BACKUP:
  server(p,t,1);actor(p,19,70,1,t,focus>=2?0:3);
  break;
 case CINE_NOTICE: {
    street(p,t,0);
    int x=travel(32,43,beat,9,65);
    actor(p,x,72,1,t,beat<70?2:3);
    /* Held phone follows his hand, not a floating large overlay. */
    int px=x+21,py=52;
    rect(p,px,py,9,14,3);rect(p,px+2,py+2,5,10,25);
    rect(p,px+3,py+3,3,2,20);
    if((t/18)&1)pixel(p,px+4,py+8,18);
    break;
   }
 case CINE_CLINIC:
    clinic(p,t,0);
    actor(p,travel(35,43,beat,8,59),72,1,t,beat<65?2:focus==2?7:5);
    actor(p,86,72,6,t,focus>=1?4:1);
    rect(p,81,53,17,2,20);
    if(focus>=1)stroke(p,87,52,89,48,31);
    break;
 case CINE_FOLLOWUP: {
  street(p,t,1);
  /* Identifiable outpatient entrance: fixed doorway, sign and steps. */
  rect(p,11,14,67,44,21);rect(p,17,19,54,32,20);
  rect(p,25,24,38,4,17);rect(p,34,32,20,15,25);
  rect(p,16,17,56,2,14);rect(p,40,50,19,2,14);
  rect(p,46,52,28,3,14);
  int dx=travel(63,80,beat,7,75);
  int mx=travel(91,104,beat,24,78);
  actor(p,dx,74,1,t,beat<82?2:focus&1?1:0);
  actor(p,mx,74,2,t,beat<102?2:focus&1?0:1);
  break;
 }
  case CINE_LAST_CLINIC:
    clinic(p,t,1);
    actor(p,30,72,1,t,focus>=2?7:5);
    actor(p,58,72,2,t,focus>=2?6:5);
    actor(p,101,72,6,t,focus&1?4:1);
    break;
 case CINE_GOOD:
  if(page<2){roof(p,t,1);canvas(p,72,19,t,1);actor(p,45,72,1,t,1);}
  else if(page<5){
     recordshop(p,t);
     if(page==4){
       int x=travel(110,96,beat,10,85);
       /* The physical sleeve remains on a real shelf while Mara's left
          hand arrives at exactly the sleeve's lower-right edge. */
       rect(p,74,51,9,9,14);rect(p,76,53,5,5,20);
       disk(p,78,55,2,22);
       actor(p,x,73,2,t,beat<96?2:4);
     }
   }
  else if(page==5){
    /* Someone stays with Danny's artwork after he is gone. */
    roof(p,t,1);canvas(p,67,19,t>=400u?400u:t,1);
     actor(p,travel(41,50,beat,14,79),73,2,t,beat<94?2:0);
  }else if(page==6){
     /* Moving lights remain anchored to a visible concert truss. */
     city(p,t,1);rect(p,0,35,120,45,3);
     for(int i=0;i<5;i++){
       int x=8+i*24;
       rect(p,x,16,5,29,21);
       int swing=breathe(t+(unsigned)i*35u);
       stroke(p,x+2,19,59+swing*4,43,18);
     }
     rect(p,20,45,80,3,16);rect(p,26,28,68,17,14);
     rect(p,49,41,24,4,22);
     actor(p,60,50,3,t,1);
     for(int i=0;i<17;i++){
       int x=i*8-3,y=68+(i%3)*3+((int)(t/16+i)&1);
       disk(p,x,y,4,i%4?24:16);
       rect(p,x-3,y+2,7,12,i%3?3:21);
     }
  }else{
    city(p,t,1);rect(p,0,59,120,21,9);
    for(int i=0;i<6;i++){
      int x=9+i*19, y=47+(i%3)*2;
      rect(p,x,y,11,13,21);rect(p,x+2,y+2,7,9,20);
      disk(p,x+5,y+6,3,3);disk(p,x+5,y+6,1,26);
      if((t/10+i)&1)rect(p,x+2,y+2,2,1,18);
    }
  }
  break;
 case CINE_DAY:
  roof(p,t,0);canvas(p,72,19,t,0);actor(p,49,72,1,t,3);
  break;
 case CINE_REBRAND:
  server(p,t,1);actor(p,24,70,1,t,0);
  break;
 case CINE_MARA_EMPTY:
   cafe(p,t);cafe_table(p,1);
   /* Mara's unanswered phone rests beside her actual cafe cup. */
   rect(p,66,55,4,4,3);rect(p,67,56,2,2,25);
   if((t/150u)&1u)pixel(p,68,56,18);
   actor(p,travel(36,42,beat,20,58),72,1,t,beat<68?2:7);
   rect(p,81,62,13,2,14);rect(p,84,64,2,7,14);
   rect(p,91,64,2,7,14);
   break;
 case CINE_RESCUE_DOOR: {
    safehouse(p,t);
    rect(p,76,22,29,35,3);rect(p,78,24,25,31,14);
    rect(p,80,25,21,28,22);pixel(p,97,43,18);
    int x=travel(38,59,beat,6,70);
    actor(p,x,72,1,t,beat<76?2:focus>=2?7:3);
    if(beat>84 && ((t/24)&1))pixel(p,97,43,7);
    break;
   }
 case CINE_RESCUE_SAVED: {
    safehouse(p,t);
    rect(p,76,22,29,35,3);
    int open=travel(0,16,beat,15,47);
    rect(p,78,24,25,31,20);
    rect(p,78+open,24,25-open,31,14);
    rect(p,79+open,25,20-open,29,22);
    /* They approach from separated, grounded positions: only once the door
       has opened does Mara enter the shot. Hands meet without crossing. */
    int dx=travel(30,50,beat,12,79);
    int mx=travel(93,89,beat,50,72);
    actor(p,dx,72,1,t,beat<92?2:beat>=124?6:focus>=1?3:5);
    if(beat>=45u)
        actor(p,mx,72,2,t,beat<118?2:beat>=124?6:focus>=1?4:5);
    if(beat>=123u)stroke(p,dx+19,57,mx-20,57,31);
    break;
   }
 case CINE_BAD:
   roof(p,t,1);canvas(p,71,21,t,0);
   actor(p,41,76,1,t,7);
   rect(p,14,66,11,4,14);rect(p,17,63,5,3,18);
   /* The unresolved phone / returned note is selected by the ending
      director so rescued Mara is never shown as missing. */
   break;
  case CINE_FINALE: {
  /* Twelve coherent shots along one fixed rooftop composition. The camera
     drifts but never teleports objects or changes the skyline direction. */
  int f=(int)final;
  roof(p,t/2,1);
  canvas(p,69,18,f>295?295u:final,f>=20);
  /* Exterior paint supplies, stationary in every shot. */
  rect(p,13,69,10,5,14);rect(p,15,66,6,3,18);
  pixel(p,15,67,26);pixel(p,18,67,16);pixel(p,20,67,13);
  stroke(p,18,68,25,62,14);
  rect(p,97,69,15,3,3);rect(p,101,67,9,2,7);
  if(f<310){
   int x=43+(f>200?(f-200)/50:0);
   actor(p,x,76,1,final,f<230?3:0);
   if(f<235){
    /* Brush connects the hand to the canvas; last colored stroke appears. */
    int dy=((f/13)&1);
    stroke(p,x+18,49+dy,69+8,32+dy,31);
    stroke(p,77,32+dy,79,29+dy,14);
   }
   if(f>270){
    rect(p,60,67,9,1,14);
   }
  }else if(f<354){
   /* A restrained loss of balance, shown only as a silhouette. */
   int y=(f-310)/4;
   rect(p,49,37+y,14,9,24);rect(p,51,44+y,13,18,16);
   stroke(p,60,57+y,71,48+y,16);
  }else{
   /* The aftermath uses a wider framing, the finished painting remains. */
   rect(p,50,67,23,4,24);
   rect(p,51,70,27,5,16);
   if(f>395){rect(p,0,72,120,8,21);}
  }
  break;
 }
 default:street(p,t,0);break;
 }
 /* Sparse grain stays at the film border, never across character eyes. */
  for(int i=0;i<2;i++){
   int x=(int)((t/12u+i*47u)%120u);
   pixel(p,x,i?1:0,i?29:22);
  }
  
 /* A two-frame top/bottom eyelid wipe on scene introductions (not flicker). */
 if(t<8&&scene!=CINE_FINALE){
  int bar=(8-(int)t)/2;
  rect(p,0,0,120,bar,3);rect(p,0,80-bar,120,bar,3);
 }
}


void cinema_draw(uint8_t *p,unsigned scene,unsigned tick,
                 unsigned page,unsigned final){
 cinema_draw_stage(p,scene,tick,tick,page,final);
}


/* Final five neutral shots are a connected aftermath, not five repeated
   copies of the motionless roof. The real painting remains at the same
   coordinates in EVERY shot; Mara moves in from the actual left stairs,
   notices Danny's folded note and carries it toward the finished canvas.
   The note changes hands at musical cuts, never floats over anyone. */
static void neutral_rooftop_beat(uint8_t *p,unsigned shot,unsigned local,
                                 unsigned world){
 if(shot<6u)return;
 int x=shot==6u?travel(-13,25,local,24u,132u):
       shot==7u?29:shot==8u?31:shot==9u?35:38;
 int pose=shot==6u?(local<160u?2:7):
          shot==7u?(local<90u?7:4):
          shot==8u?6:shot==9u?5:6;
 /* The note lies where Danny left his supplies; the hand meets it.
    Subsequent montage cuts show it tucked against Mara's chest. */
 if(shot<=7u)paper(p,10,58,11,6,1);
 actor(p,x,73,2,world,pose);
 if(shot>=8u){
   paper(p,x-7,53,9,5,2);
   if(shot==10u)pixel(p,x-3,55,18);
 }
 /* Camera emphasis changes through lamps/railings at the edge,
    without erasing the painting or altering the rooftop geography. */
 if(shot>=9u){
   rect(p,2,54,3,21,14);
   rect(p,2,54,15,2,14);
   pixel(p,4,53,(world/70u)&1u?18:7);
 }
}

/* The good ending revisits one real record shop at three different moments:
   discovery, growing audience and the quiet final coda. Add only small
   foreground props to the LEFT of the turntable so continuous record
   rotation at x60 remains untouched across a musical shot boundary. */
static void good_recordshop_beat(uint8_t *p,unsigned shot,unsigned local,
                                 unsigned world){
 if(shot==4u){
   /* The discovered master gets a sleeve and a dedicated shelf. */
   rect(p,6,45,29,3,14);rect(p,8,47,3,12,14);rect(p,31,47,3,12,14);
   paper(p,14,38,14,7,2);
 }else if(shot==6u){
   /* A growing rack of copies, all on the SAME lower shop shelf. */
   rect(p,6,54,37,3,14);
   for(int i=0;i<5;i++){
     rect(p,9+i*6,44,5,10,(i&1)?25:20);
     rect(p,10+i*6,46,3,4,14);
   }
 }else if(shot==8u){
   /* After the first part of the score, Mara listens in the shop. */
   rect(p,6,55,28,3,14);
   paper(p,9,48,14,7,2);
   actor(p,travel(-12,28,local,12u,98u),74,2,world,
         local<115u?2:5);
 }else if(shot==11u){
   /* Bookend the concert with an intimate, stationary listener. */
   rect(p,6,55,32,3,14);
   paper(p,12,46,14,7,2);
   actor(p,30,74,2,world,6);
 }
}

/* Failure implies unanswered contact; successful rescue must NEVER depict
   Mara as missing. The same rooftop set instead holds a returned letter. */
static void bad_rooftop_beat(uint8_t *p,unsigned shot,unsigned local,
                             unsigned world,unsigned mara_safe){
 if(mara_safe){
   /* A folded thank-you card stands on the same art-supply shelf. */
   paper(p,16,57,11,8,1);
   rect(p,17,64,9,1,14);
   if(shot>=8u){rect(p,30,65,16,2,14);pixel(p,34,64,20);}
 }else{
   /* The unanswered phone glows beside an empty chair. It provides no
      explicit information about Mara's physical fate. */
   rect(p,17,57,8,11,3);rect(p,19,59,4,6,25);
   if((world/90u)&1u)pixel(p,21,61,18);
   if(shot>=5u){
     rect(p,30,61,13,2,14);
     rect(p,32,63,2,8,14);rect(p,40,63,2,8,14);
   }
 }
 if(shot>=8u && local<85u){
   /* A held reaction beat; no random shake or dramatic teleport. */
   rect(p,7,58,5,11,14);
 }
}

/* Long-form ending storyboard. Each shot holds an identifiable location and
   slowly moves within it. Scene changes follow sections of the 20-phrase
   HERE TODAY score; do not pin the last 65 seconds to a static image. */
typedef struct { uint16_t until; uint8_t scene, page; } EndingShot;
static const EndingShot neutral_shots[] = {
 {370,CINE_FINALE,0}, {810,CINE_FINALE,0},
 {1130,CINE_LETTER,1}, {1590,CINE_MARA,2},
 {2010,CINE_BORE,1}, {2490,CINE_MUSIC,0},
 {2870,CINE_FINALE,0}, {3250,CINE_FINALE,0},
 {3650,CINE_FINALE,0}, {4050,CINE_FINALE,0},
 {4350,CINE_FINALE,0}
};
static const EndingShot good_shots[] = {
 {530,CINE_ART,1}, {1080,CINE_MUSIC,1},
 {1560,CINE_MARA,2}, {2050,CINE_BORE,2},
 {2600,CINE_GOOD,3}, {3200,CINE_GOOD,4},
 {3730,CINE_GOOD,3}, {4350,CINE_GOOD,5},
 /* Here Today resolves at 4350: a separate 16-second coda begins. */
 {4610,CINE_GOOD,3}, {4890,CINE_GOOD,6},
 {5140,CINE_GOOD,7}, {5310,CINE_GOOD,3}
};

static const EndingShot bad_shots[] = {
 {350,CINE_BAD,0}, {780,CINE_LETTER,1},
 {1190,CINE_MARA,2}, {1650,CINE_SERVER,1},
 {2070,CINE_RESCUE_DOOR,1}, {2490,CINE_BAD,0},
 {2900,CINE_BORE,1}, {3320,CINE_MUSIC,1},
 {3670,CINE_BAD,0}, {4030,CINE_REBRAND,1},
 {4350,CINE_BAD,0}
};
static unsigned pick_shot(const EndingShot *v,unsigned count,unsigned frame){
 for(unsigned i=0;i<count;i++)if(frame<v[i].until)return i;
 return count-1;
}
unsigned cinema_ending_shot(unsigned ending,unsigned frame){
 if(ending==1)return pick_shot(good_shots,
        sizeof(good_shots)/sizeof(good_shots[0]),frame);
 if(ending==3)return pick_shot(bad_shots,
        sizeof(bad_shots)/sizeof(bad_shots[0]),frame);
 return pick_shot(neutral_shots,
        sizeof(neutral_shots)/sizeof(neutral_shots[0]),frame);
}
/* A rescue-success bad ending cannot show Mara as missing, and a failed
   rescue cannot accidentally show Mara present at the cafe. */
unsigned cinema_ending_scene(unsigned ending,unsigned frame,unsigned mara_safe){
 unsigned shot=cinema_ending_shot(ending,frame);
 if(ending==3u){
    if(shot==2u)return mara_safe?CINE_MARA:CINE_MARA_EMPTY;
    if(shot==4u)return mara_safe?CINE_RESCUE_SAVED:CINE_RESCUE_DOOR;
 }
 return ending==1u?good_shots[shot].scene:
        ending==3u?bad_shots[shot].scene:neutral_shots[shot].scene;
}
void cinema_ending_draw_context(uint8_t *p,unsigned ending,unsigned frame,
                                unsigned mara_safe){
 unsigned good=ending==1u;
 const EndingShot *shots=good?good_shots:ending==3?bad_shots:neutral_shots;
 unsigned shot=cinema_ending_shot(ending,frame);
 unsigned begin=shot?shots[shot-1u].until:0u;
 unsigned local=frame>begin?frame-begin:0u;
 unsigned scene=cinema_ending_scene(ending,frame,mara_safe);
 if(!good && scene==CINE_FINALE){
   /* Sky and city continue moving; Danny stays in the last pose reached. */
   cinema_draw_stage(p,scene,frame,frame,0,frame<420u?frame:420u);
   if(ending==2u)neutral_rooftop_beat(p,shot,local,frame);
 }else{
   /* Do not restart city or record spindle on a musical cut. */
   cinema_draw_stage(p,scene,100u+frame,local,shots[shot].page,0);
   if(ending==1u && scene==CINE_GOOD && shots[shot].page>=3u &&
      shots[shot].page<=4u)
      good_recordshop_beat(p,shot,local,frame);
   if(ending==3u && scene==CINE_BAD)
      bad_rooftop_beat(p,shot,local,frame,mara_safe);
 }
 /* Film-gate cut at score section boundaries: no fake pixel zoom or extra
    framebuffer. The same authored world location persists inside each shot. */
 if(local<14u&&shot>0u){
   int bar=(14-(int)local)*9/14;
   rect(p,0,0,120,bar,3);
   rect(p,0,80-bar,120,bar,3);
 }
}
void cinema_ending_draw(uint8_t *p,unsigned ending,unsigned frame){
 cinema_ending_draw_context(p,ending,frame,0u);
}

/* Tiny world-space accents, only during ordinary life-sim scenes.
   Work affects an actual painting and tape monitor, not just the HUD. */
void cinema_moment_detail(uint8_t *p,unsigned scene,unsigned work,unsigned bond){
    if(scene==CINE_ART){
        unsigned strokes=(work+1u)/2u;
        if(strokes>9u)strokes=9u;
        for(unsigned i=0;i<strokes;i++){
            int x=75+(int)i*2,y=29+(int)(i%3u)*2;
            stroke(p,x,y,x+3,y+1,(i&1u)?18:13);
        }
    }else if(scene==CINE_MUSIC){
        unsigned bars=(work+1u)/3u;
        if(bars>6u)bars=6u;
        for(unsigned i=0;i<bars;i++){
            int x=79+(int)i*4;
            rect(p,x,35-(int)(i%3u)*2,3,2,20);
        }
    }else if(scene==CINE_MARA && bond>=4u){
        /* Two small cups between the friends on a familiar street bench. */
        rect(p,56,56,4,3,20);rect(p,67,56,4,3,20);
        pixel(p,55,57,18);pixel(p,71,57,18);
    }
}

/* Physical project milestones, not a separate static "success" screen.
   Scene interiors, characters and props remain continuous. */
void cinema_project_detail(uint8_t *p,unsigned scene,unsigned music,
                           unsigned art,unsigned style,unsigned tod){
 if(scene==CINE_ART){
    if(art>4u)art=4u;
    for(unsigned i=0;i<art;i++){
        int x=73+(int)i*6;
        int y=28+(int)(i%2u)*3;
        stroke(p,x,y,x+4,y+1,(style&2u)?18:20);
        stroke(p,x,y+1,x+3,y+3,13);
    }
    if(art==4u){
        rect(p,70,19,31,2,14);rect(p,72,42,27,3,14);
        rect(p,81,43,9,2,20); /* brass title plate on canvas */
        if(style&2u)disk(p,91,26,3,18);
    }
 }else if(scene==CINE_MUSIC){
    if(music>4u)music=4u;
    for(unsigned i=0;i<music;i++){
        int x=80+(int)i*6;
        rect(p,x,24+(int)(i%2u)*3,4,3,(style&1u)?26:20);
        rect(p,x+2,30,3,2,18);
    }
    if(music==4u){
        rect(p,81,39,25,5,14);
        rect(p,84,41,20,1,20);
        disk(p,46,49,8,(style&1u)?26:22); /* boxed master reel */
        disk(p,46,49,2,20);
    }
 }else if(scene==CINE_MARA && tod==2u){
    /* The cafe is still available after dark: lit shop windows and lamp. */
    rect(p,8,29,20,15,18);
    rect(p,9,30,18,13,23);
    rect(p,17,30,2,13,14);
    rect(p,8,37,20,1,14);
 }else if(scene==CINE_BORE && tod==2u){
    /* Shift-change desk lamp, without moving the one reachable clerk. */
    disk(p,103,16,5,18);
    rect(p,101,20,5,7,14);
 }
}
