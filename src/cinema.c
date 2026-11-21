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
static void disk(uint8_t *p,int x,int y,int r,int c){
 for(int yy=-r;yy<=r;yy++)for(int xx=-r;xx<=r;xx++)
  if(xx*xx+yy*yy<=r*r)pixel(p,x+xx,y+yy,c);
}
/* Consistent distant skyline, rooftop heights, scrolling clouds and window
   glints. Foreground has independent motion; backgrounds never jump locations. */
static void city(uint8_t *p,unsigned t,int dusk){
 for(int y=0;y<H;y++)rect(p,0,y,W,1,y<19?(dusk?17:26):y<38?(dusk?25:27):y<53?29:9);
 disk(p,97,13,dusk?6:8,dusk?18:20);
 for(int i=0;i<4;i++){
  int x=12+i*31-(int)((t/18+i*6)%41);
  rect(p,x,9+(i&1)*4,24,2,dusk?29:20);
  rect(p,x+5,7+(i&1)*4,14,2,dusk?29:20);
 }
 for(int i=0;i<16;i++){
  int x=i*10-3-((int)(t/85)%11),ht=14+((i*13+3)%22);
  rect(p,x,52-ht,10,ht,29);
  rect(p,x+2,53-ht,2,1,4);
  for(int j=0;j<4;j++){
   int wy=53-ht+5+j*6;
   if(wy<51&&((i+j+(int)(t/80))&3)!=0)pixel(p,x+3,wy,dusk?18:7);
  }
 }
 for(int i=0;i<7;i++){
  int x=i*23-12-((int)(t/110)%8),ht=13+((i*7)%16);
  rect(p,x,57-ht,22,ht,3);
  for(int w=3;w<20;w+=6)for(int y=60-ht+4;y<53;y+=7)
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
static void interior(uint8_t *p,int mode,unsigned t){
 int wall=mode==0?17:mode==1?21:mode==2?12:8;
 rect(p,0,0,W,H,wall);rect(p,0,54,W,26,mode==2?11:9);
 for(int x=0;x<W;x+=20){rect(p,x,0,1,53,mode==1?23:29);}
 rect(p,8,9,47,37,3);rect(p,11,12,41,31,26);
 /* A genuine view onto the same animated Old Quarter. */
 for(int i=0;i<6;i++){
  int x=11+i*8-((int)t/95%5),h=(i*9+7)%19+7;
  rect(p,x,44-h,8,h,29);
  if(i&1)rect(p,x+3,40-h,2,2,18);
 }
 rect(p,8,42,47,3,14);rect(p,30,9,2,36,14);
 rect(p,0,52,W,2,mode==2?13:14);
}
static void paper(uint8_t *p,int x,int y,int w,int h,int blink){
 rect(p,x+1,y+1,w,h,3);rect(p,x,y,w,h,20);
 for(int i=0;i<4;i++)rect(p,x+3,y+4+i*4,w-6,1,i==blink%4?18:22);
}
static void furniture(uint8_t *p,int x,int y,int w,int c){
 rect(p,x+1,y,w-2,4,7);rect(p,x,y,w,3,c);
 rect(p,x+5,y+4,4,22,14);rect(p,x+w-9,y+4,4,22,14);
}
/* Full, expressive 2D silhouettes with hair, coat, eyes, blink and grounded
   feet/shadows; no abstract flying heads or randomly teleporting props. */
static void actor(uint8_t *p,int x,int foot,int role,unsigned t,int gesture){
 int walk=(gesture==2)?((int)(t/5)&1):0;
 int bob=(gesture==2)?((int)(t/5)&1):((int)(t/22)&1);
 int coat=role==1?16:role==2?25:role==3?17:role==4?12:role==5?23:28;
 int skin=role==4?22:31;
 /* Shadow, shoes, trouser split and collar. */
 rect(p,x-9,foot,19,2,3);
 rect(p,x-6,foot-6+walk,5,7,3);rect(p,x+2,foot-6-walk,5,7,3);
 rect(p,x-8,foot-24+bob,17,18,coat);
 rect(p,x-7,foot-24+bob,15,2,role==3?20:18);
 rect(p,x-8,foot-24+bob,2,15,23);rect(p,x+7,foot-24+bob,2,15,23);
 /* Head, ear, distinct hairstyles and expressions. */
 disk(p,x,foot-32+bob,8,skin);
 rect(p,x-8,foot-40+bob,16,5,role==2?24:role==3?29:role==4?14:3);
 if(role==2){rect(p,x-9,foot-37+bob,3,14,24);}
 if(role==3){rect(p,x+6,foot-35+bob,4,12,29);}
 if((t%90)<86){pixel(p,x-3,foot-33+bob,3);pixel(p,x+3,foot-33+bob,3);}
 else{rect(p,x-4,foot-33+bob,3,1,3);rect(p,x+2,foot-33+bob,3,1,3);}
 pixel(p,x,foot-29+bob,23);
 if(role==2)rect(p,x-3,foot-27+bob,7,1,23);
 /* Gestures: speaking, handing over, painting, or idle weight shifts. */
 if(gesture==1){
  stroke(p,x+7,foot-20+bob,x+15,foot-29+((int)(t/14)&1),coat);
  disk(p,x+15,foot-29+((int)(t/14)&1),2,skin);
 } else if(gesture==3){
  stroke(p,x+7,foot-22+bob,x+20,foot-27+bob,coat);
  disk(p,x+20,foot-27+bob,2,skin);
 } else {
  stroke(p,x-7,foot-21+bob,x-11,foot-12+bob,coat);
  stroke(p,x+8,foot-21+bob,x+11,foot-12+bob,coat);
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
  int q=(int)(t/17)%6;
  stroke(p,x+5+q,y+6,x+15+q/2,y+11,18);
  pixel(p,x+11+q,y+8,7);
 }
 rect(p,x+13,y+27,3,21,14);rect(p,x+1,y+42,31,2,14);
 stroke(p,x+14,y+30,x+2,y+50,14);stroke(p,x+15,y+30,x+28,y+50,14);
}
static void studio(uint8_t *p,unsigned t){
 interior(p,1,t);
 rect(p,0,19,6,28,3);rect(p,1,21,4,24,22);
 rect(p,73,13,37,38,4);rect(p,75,15,33,34,17);
 rect(p,82,22,18,20,14);disk(p,91,31,9,3);disk(p,91,31,5,10);
 disk(p,91,31,2,20);
 for(int i=0;i<4;i++)rect(p,80+i*6,45,3,2,((t/12+i)&1)?18:7);
 furniture(p,15,51,94,14);
 disk(p,45,49,13,3);disk(p,45,49,9,22);disk(p,45,49,2,20);
 stroke(p,61,44,55+((int)t/25%3),46,20);
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
 disk(p,60,38,20,3);disk(p,60,38,17,22);disk(p,60,38,12,3);disk(p,60,38,7,18);disk(p,60,38,3,20);
 rect(p,94,31,4,20,7);rect(p,94,30,12,2,20);
 rect(p,9,8,100,5,21);
 if((t/15)&1)rect(p,19,9,7,3,18);
}
/* Camera framing: wide -> two-shot -> actor reaction -> detail. The world and
   object positions remain coherent: panning is limited to a few pixels. */
void cinema_draw(uint8_t *p,unsigned scene,unsigned tick,unsigned page,unsigned final){
 if(scene>=CINE_COUNT)scene=CINE_NONE;
 unsigned t=tick;
 int a=(int)(t/24)%3;
 int pan=(int)(t/70)%7-3;
 int focus=(int)(page&3u);
 if(scene==CINE_FINALE)t=final;
 switch(scene){
 case CINE_OPEN:
  street(p,t,0);
  bus(p,92-(int)(t/7)%145,27,t);
  actor(p,32,67,1,t,2);
  rect(p,2,39,26,12,14);rect(p,3,40,24,10,22);
  break;
 case CINE_LETTER:
  street(p,t,1);
  rect(p,3,15,56,43,14);rect(p,5,18,52,38,23);
  rect(p,13,24,18,29,3);rect(p,16,26,12,24,17);
  rect(p,41,24,10,11,4);rect(p,43,26,6,7,26);
  rect(p,36,43,17,2,14);
  actor(p,69+pan/2,72,1,t,focus>=2?1:0);
  paper(p,40+(a&1),50-(int)focus,12,8,focus);
  break;
 case CINE_BORE:
  interior(p,1,t);
  rect(p,65,4,47,8,3);rect(p,67,6,43,4,18);
  furniture(p,20,53,91,14);
  paper(p,52,49,22,10,focus);
  actor(p,31,65,1,t,focus&1?1:0);
  actor(p,96,60,3,t,focus==2?3:1);
  if(focus>=2){rect(p,59,43,13,7,16);rect(p,61,45,9,3,20);}
  break;
 case CINE_MARA:
  street(p,t,1);
  rect(p,3,16,32,38,14);rect(p,6,20,26,29,23);
  rect(p,7,18,23,7,18);rect(p,9,25,21,4,22);
  actor(p,44+pan/2,67,1,t,focus&1?1:0);
  actor(p,79-pan/2,67,2,t,focus&1?0:1);
  if(focus>=2){stroke(p,57,45,66,44,20);disk(p,66,44,2,31);}
  break;
 case CINE_BUS:
  street(p,t,1);
  bus(p,15+((t/60)%2),21,t);
  actor(p,90+pan/2,69,1,t,focus==2?2:1);
  break;
 case CINE_STAY:
  street(p,t,1);
  rect(p,5,13,75,42,14);rect(p,7,17,71,36,23);
  rect(p,22,25,36,13,18);rect(p,24,27,32,9,20);
  actor(p,51,68,1,t,1);actor(p,87,68,2,t,1);
  break;
 case CINE_MUSIC:
  studio(p,t);actor(p,20,69,1,t,focus==2?3:1);
  for(int i=0;i<5;i++)rect(p,81+i*5,16,2,2+((int)(t/9)+i)%6,((t/17+i)&1)?7:18);
  break;
 case CINE_ART:
  roof(p,t,0);canvas(p,70,20,t,1);
  actor(p,45,76,1,t,3);
  stroke(p,60,50,73,41,31);stroke(p,73,41,77,36,14);
  break;
 case CINE_SERVER:
  server(p,t,0);actor(p,17,70,1,t,focus>=2?3:0);
  break;
 case CINE_WITNESS:
  street(p,t,1);
  rect(p,52,54,61,3,14);rect(p,58,57,4,9,14);rect(p,103,57,4,9,14);
  actor(p,38,69,1,t,focus&1?0:1);
  actor(p,78,69,4,t,focus==0?0:1);
  break;
 case CINE_SAFE:
  safehouse(p,t);
  actor(p,33,67,1,t,focus>=1?0:1);
  actor(p,83,67,4,t,focus>=1?1:0);
  break;
 case CINE_BACKUP:
  server(p,t,1);actor(p,19,70,1,t,focus>=2?0:3);
  break;
 case CINE_NOTICE:
  street(p,t,0);
  actor(p,44,70,1,t,focus>=1?0:1);
  /* Notification appears on a tangible phone, held by Danny. */
  rect(p,64,24,19,28,3);rect(p,66,26,15,24,25);
  rect(p,68,30,11,4,20);rect(p,69,36,9,1,16);
  if((t/13)&1)rect(p,69,41,8,2,18);
  stroke(p,53,47,64,44,31);
  break;
 case CINE_CLINIC:
  clinic(p,t,0);actor(p,40,69,1,t,focus==2?0:1);
  actor(p,86,67,3,t,focus&1?3:1);
  break;
 case CINE_FOLLOWUP:
  street(p,t,1);
  rect(p,11,14,67,44,21);rect(p,17,19,54,32,20);
  rect(p,25,24,38,4,17);rect(p,34,32,20,15,25);
  actor(p,79,71,1,t,focus&1?1:0);
  actor(p,104,71,2,t,focus&1?0:1);
  break;
 case CINE_LAST_CLINIC:
  clinic(p,t,1);
  actor(p,30,69,1,t,focus>=2?0:1);
  actor(p,58,69,2,t,focus>=2?3:0);
  actor(p,100,68,3,t,focus&1?3:0);
  break;
 case CINE_GOOD:
  if(page<2){roof(p,t,1);canvas(p,72,19,t,1);actor(p,45,72,1,t,1);}
  else if(page<5){recordshop(p,t);if(page==4)actor(p,101,73,2,t,0);}
  else{roof(p,t,1);canvas(p,67,19,t,0);actor(p,49,73,2,t,0);}
  break;
 case CINE_DAY:
  roof(p,t,0);canvas(p,72,19,t,0);actor(p,49,72,1,t,3);
  break;
 case CINE_REBRAND:
  server(p,t,1);actor(p,24,70,1,t,0);
  break;
 case CINE_FINALE: {
  /* Twelve coherent shots along one fixed rooftop composition. The camera
     drifts but never teleports objects or changes the skyline direction. */
  int f=(int)final;
  roof(p,final/2,1);
  canvas(p,69,18,final,f>=20&&f<295);
  /* Exterior paint supplies, stationary in every shot. */
  rect(p,13,69,10,5,14);rect(p,15,66,6,3,18);
  rect(p,97,69,15,3,3);rect(p,101,67,9,2,7);
  if(f<310){
   int x=43+(f>200?(f-200)/30:0);
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
 /* Sparse, subtle film grain: no random sprite shake, no wasted VRAM. */
 for(int i=0;i<4;i++){
  int x=((int)t/6+i*29+13)%120, y=(i*13+7)%46;
  pixel(p,x,y,(i&1)?29:22);
 }
 /* A two-frame top/bottom eyelid wipe on scene introductions (not flicker). */
 if(t<8&&scene!=CINE_FINALE){
  int bar=(8-(int)t)/2;
  rect(p,0,0,120,bar,3);rect(p,0,80-bar,120,bar,3);
 }
}
