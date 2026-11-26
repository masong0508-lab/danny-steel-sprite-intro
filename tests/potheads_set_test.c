#include "hd_canvas.h"
#include "hd_potheads_set.h"
#include "hd_potheads_fixed.h"
#include "potheads_shape.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* An incorrect word count, stale generated set or
   unaligned word write must fail before this data reaches a cartridge. */
static uint8_t projected[HD_BYTES];
static uint8_t decoded[HD_BYTES+132];

static void verify_words(void){
 assert(sizeof pot_set_words==HD_BYTES);
 for(unsigned i=0;i<HD_BYTES/4u;i++){
  uint32_t four=pot_set_words[i];
  for(unsigned pixel=0;pixel<4u;pixel++){
   assert((four&255u)<64u);four>>=8;
  }
 }
}
static void crop(uint8_t *p){
 hd_rect(p,0,135,HD_W,25,HD_INK);
 hd_rect(p,0,0,240,2,HD_INK);
 hd_rect(p,0,158,240,2,HD_INK);
}
static void reference_shape(uint8_t *p,const PotPoint *v,unsigned count,unsigned ink){
 /* Scan-convert actual polygon spans, including the loose concave hem.
    Divide once per edge; never divide once per painted pixel or row. */
 typedef struct {int low,high,x,step;} Edge;
 Edge edges[8];unsigned n=0;
 int low=135,high=-1,minx=HD_W,maxx=-1;
 if(count>8u)return;
 for(unsigned i=0;i<count;i++){
  if(v[i].y<low)low=v[i].y;
  if(v[i].y>high)high=v[i].y;
  if(v[i].x<minx)minx=v[i].x;
  if(v[i].x>maxx)maxx=v[i].x;
 }
 if(high<0||low>=135||maxx<0||minx>=HD_W)return;
 if(low==high){hd_rect(p,minx,low,maxx-minx+1,1,ink);return;}
 if(low<0)low=0;
 if(high>134)high=134;
 for(unsigned i=0;i<count;i++){
  PotPoint a=v[i],b=v[(i+1u)%count];
  if(a.y>b.y){PotPoint swap=a;a=b;b=swap;}
  if(a.y==b.y||b.y<=low||a.y>high)continue;
  int first=a.y<low?low:a.y;
  int step=(b.x-a.x)*65536/(b.y-a.y);
  edges[n++]=(Edge){first,b.y,a.x*65536+step*(first-a.y),step};
 }
 for(int y=low;y<=high;y++){
  int hits[8];unsigned used=0;
  for(unsigned i=0;i<n;i++)if(y>=edges[i].low&&y<edges[i].high){
   int x=edges[i].x;unsigned at=used;
   while(at&&hits[at-1u]>x){hits[at]=hits[at-1u];at--;}
   hits[at]=x;used++;edges[i].x+=edges[i].step;
  }
  for(unsigned i=0;i+1u<used;i+=2u){
   int x=pot_set_round(hits[i],65536),xx=pot_set_round(hits[i+1u],65536);
   hd_rect(p,x,y,xx-x+1,1,ink);
  }
 }
}
static uint32_t shape_rng=123456789u;
static unsigned shape_random(void){
 shape_rng=shape_rng*1664525u+1013904223u;
 return shape_rng>>8;
}
static void verify_shapes(void){
 for(unsigned k=0;k<6000u;k++){
  PotPoint v[8];unsigned n=3u+shape_random()%6u;
  for(unsigned j=0;j<n;j++){
   v[j].x=(int)(shape_random()%330u)-45;
   v[j].y=(int)(shape_random()%210u)-40;
  }
  if(k%4u==0u){
   n=4u;int x=(int)(shape_random()%250u)-5;
   int y=(int)(shape_random()%140u)-5;
   int w=1+(int)(shape_random()%35u),h=1+(int)(shape_random()%70u);
   v[0]=(PotPoint){x,y};v[1]=(PotPoint){x+w,y};
   v[2]=(PotPoint){x+w,y+h};v[3]=(PotPoint){x,y+h};
  }
  memset(projected,HD_ROAD_LIGHT,HD_BYTES);
  memset(decoded,0xB7,sizeof decoded);
  uint8_t *pixels=decoded+64u+(k&3u);
  memset(pixels,HD_ROAD_LIGHT,HD_BYTES);
  reference_shape(projected,v,n,HD_HAIR);
  pot_shape(pixels,v,n,HD_HAIR);
  assert(!memcmp(projected,pixels,HD_BYTES));
  for(unsigned i=0;i<64u+(k&3u);i++)assert(decoded[i]==0xB7);
  for(unsigned i=64u+(k&3u)+HD_BYTES;i<sizeof decoded;i++)assert(decoded[i]==0xB7);
 }
 puts("6000 articulated polygons match reference pixels, crop and guards");
}
int main(void){
 verify_shapes();
 verify_words();
 static const unsigned settled[]={180u,181u,450u,790u,1274u,65535u};
 for(unsigned t=0;t<sizeof settled/sizeof settled[0];t++){
  PotCamera camera=pot_camera(settled[t]);
  memset(projected,0xA6,sizeof projected);
  pot_scene(projected,&camera,settled[t]);
  for(unsigned alignment=0;alignment<4u;alignment++){
   memset(decoded,0xB7,sizeof decoded);
   uint8_t *pixels=decoded+64u+alignment;
   pot_scene_fixed(pixels);
   assert(memcmp(projected,pixels,HD_BYTES)==0);
   for(unsigned i=0;i<64u+alignment;i++)assert(decoded[i]==0xB7);
   for(unsigned i=64u+alignment+HD_BYTES;i<sizeof decoded;i++)assert(decoded[i]==0xB7);
   for(unsigned i=0;i<HD_BYTES;i++)assert(pixels[i]<64u);
  }
  uint8_t *pixels=decoded+64u;
  pot_scene_fixed(pixels);crop(pixels);crop(projected);
  assert(memcmp(projected,pixels,HD_BYTES)==0);
 }
 printf("Potheads set: all 38400 pixels match the projected camera, palette, guards, four alignments and exact word count OK (%zu ROM bytes)\n",sizeof pot_set_words);
 return 0;
}
