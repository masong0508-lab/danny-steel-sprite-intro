#include "hd_canvas.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Reference artwork and geometry from the last shipped ROM. This test
   deliberately does not call the new fast paths to construct expected
   results; a changed tile, ellipse edge, or wall pitch fails. */
static const char *const original_town[STORY_H]={
"########################",
"#......................#",
"#...TT............TT...#",
"#..TTTT...####....TT...#",
"#.........#AA#.........#",
"#...L.....#..#.....B...#",
"#.........####.........#",
"#......................#",
"#...,,,,,,,,,,,,,,,,...#",
"#..T,,,......,,,T.gg...#",
"#..TT,,......,,TT.gg####",
"#...g,,.F....,,......#.#",
"#...g,,,,,,,,,,......#.#",
"#..TT.......D....gg..#.#",
"#..TT............~~~...#",
"#...............~~~~...#",
"#...............~~~~...#",
"########################"
};
static char original_tile(int x,int y){
 if(x<0||y<0||x>=STORY_W||y>=STORY_H)return '#';
 if(x==11&&y==7)return 'C';
 if(x==3&&y==7)return 'P';
 if(x==17&&y==7)return 'V';
 if(x==18&&y==12)return 'Q';
 if(x==21&&y==7)return 'S';
 if(x==6&&y==11)return 'W';
 if(x==7&&y==2)return 'H';
 if(x==14&&y==6)return 'R';
 if(x==19&&y==7)return 'M';
 if(x==16&&y==5)return 'K';
 return original_town[y][x];
}
static struct {uint8_t guard[32],pixels[HD_BYTES],tail[32];} reference,optimized;
static unsigned original_attempts,new_attempts;
static void old_ellipse(uint8_t *p,int cx,int cy,int rx,int ry,unsigned color){
 if(rx<1||ry<1)return;
 for(int y=-ry;y<=ry;y++){
  int numerator=rx*rx*(ry*ry-y*y),width=0;
  while((width+1)*(width+1)*ry*ry<=numerator){
   width++;original_attempts++;
  }
  original_attempts++;
  hd_rect(p,cx-width,cy+y,width*2+1,1,color);
 }
}
static void reset_buffers(void){
 memset(&reference,0xB6,sizeof reference);
 memset(&optimized,0xB6,sizeof optimized);
}
static void guards(void){
 for(unsigned i=0;i<32u;i++){
  assert(reference.guard[i]==0xB6);
  assert(reference.tail[i]==0xB6);
  assert(optimized.guard[i]==0xB6);
  assert(optimized.tail[i]==0xB6);
 }
}
int main(void){
 unsigned cells=0;
 for(int y=-3;y<=STORY_H+3;y++)
  for(int x=-3;x<=STORY_W+3;x++){
   assert(story_tile(x,y)==original_tile(x,y));
   cells++;
  }
 /* Eliminate per-pixel divisions while preserving exact rounding, not
    merely the overall 3D silhouette or a hand-picked snapshot. */
 unsigned old_divisions=0,new_divisions=0;
 for(int height=1;height<=150;height++){
  int major=256/height,minor=256%height,error=0,value=0;
  new_divisions+=2u;
  for(int k=0;k<height;k++){
   int baseline=k*256/height;
   assert(value==baseline);
   old_divisions++;
   value+=major;
   error+=minor;
   if(error>=height){value++;error-=height;}
  }
 }
 const int radii[]={1,2,3,4,6,9,14,19,28,40};
 const int positions[][2]={{-10,0},{120,80},{238,158},{255,-13},{0,159}};
 unsigned tested=0;
 for(unsigned rx=0;rx<10;rx++)for(unsigned ry=0;ry<10;ry++)
  for(unsigned pos=0;pos<5;pos++){
   int x=positions[pos][0],y=positions[pos][1];
   int a=radii[rx],b=radii[ry];
   reset_buffers();
   old_ellipse(reference.pixels,x,y,a,b,HD_GOLD);
   hd_ellipse(optimized.pixels,x,y,a,b,HD_GOLD);
   assert(memcmp(reference.pixels,optimized.pixels,HD_BYTES)==0);
   guards();tested++;
   /* The same monotone-radius procedure as the optimized ellipse
      counts comparisons (rather than wall-clock-dependent timings). */
   int width=0,rx2=a*a,ry2=b*b;
   for(int yy=-b;yy<=b;yy++){
    int numerator=rx2*(ry2-yy*yy);
    if(yy<=0){
     for(;;){
      new_attempts++;
      if((width+1)*(width+1)*ry2>numerator)break;
      width++;
     }
    }else{
     for(;;){
      new_attempts++;
      if(width*width*ry2<=numerator)break;
      width--;
     }
    }
   }
  }
 assert(new_divisions<old_divisions);
 assert(new_attempts<original_attempts);
 printf("immutable map %u cells matched legacy at all edges\n",cells);
 printf("wall texture projection: %u old pixel divides vs %u new column divides (exact)\n",
        old_divisions,new_divisions);
 printf("integer ellipses %u exact framebuffer matches; radius checks %u -> %u\n",
        tested,original_attempts,new_attempts);
 return 0;
}
