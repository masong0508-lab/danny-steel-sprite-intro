#include "hd_scene_geometry.h"
#include "hd_facade_art.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void legacy_district(int x,int y,char type,unsigned *height,unsigned *variant){
 int civic=x>=10&&x<=13&&y>=3&&y<=6,shops=x>=21&&y>=10&&y<=12,waterfront=y>=14;
 *height=civic?36u:shops?27u:waterfront?27u:((x/6+y/6)&1)?32u:27u;
 *variant=civic?6u:shops?4u:waterfront?2u:((x/6+y/6)&1)?0u:1u;
 if(!civic&&!shops&&!waterfront&&x<5&&y>=7)*variant=3u;
 if(!civic&&!shops&&!waterfront&&x>=17&&y<=4)*variant=5u;
 if(x==21&&y==11)*variant=7u;
 if(type=='A')*variant=6u;
}
static uint8_t frame[HD_BYTES];
int main(void){
 unsigned districts=0,steps=0,texels=0;
 for(int y=-1;y<=STORY_H;y++)for(int x=-1;x<=STORY_W;x++){
  char t=story_tile(x,y);
  unsigned height,variant;legacy_district(x,y,t,&height,&variant);
  HdDistrict d=hd_district(x,y,t);
  assert(d.height==height&&d.variant==variant);
  for(unsigned side=0;side<2u;side++)assert(hd_facade_at(x,y,t,side)==hd_facade_texture(variant,side));
  districts++;
 }
 for(unsigned side=0;side<2u;side++)for(unsigned variant=0;variant<8u;variant++){
  const uint8_t *tex=hd_facade_texture(variant,side);
  memset(frame,HD_INK,sizeof frame);hd_facade_wall(frame,64,64,40,variant,side);
  for(int v=0;v<40;v++)for(int u=0;u<16;u++){
   unsigned expected=frame[(64+(side?16-u/2:8+u/2)-40+v)*HD_W+64+u];
   assert(tex[v*16+u]==expected&&expected>=32u&&expected<64u);texels++;
  }
 }
 static const int dx[]={1,0,-1,0},dy[]={0,1,0,-1};
 for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++){
  if(story_solid(x,y))continue;
  for(unsigned dir=0;dir<4u;dir++){
   int px=x-dx[dir],py=y-dy[dir];if(story_solid(px,py))continue;
   Story s={0};s.x=(uint8_t)x;s.y=(uint8_t)y;s.prev_x=(uint8_t)px;s.prev_y=(uint8_t)py;
   for(unsigned phase=0;phase<=4u;phase++){
    s.anim=(uint8_t)phase;HdSceneCamera c=hd_scene_camera(&s);
    int qx=x*4+phase*(px-x),qy=y*4+phase*(py-y);
    assert(c.x==qx*64+128&&c.y==qy*64+128);
    assert(c.depth==(qx+qy)*2);
    for(int wy=-1;wy<=STORY_H;wy+=3)for(int wx=-1;wx<=STORY_W;wx+=3){
     int a=104+16*(wx-x-wy+y)-c.iso_x;
     int b=72+8*(wx-x+wy-y)-c.iso_y;
     assert(a==104+4*(wx*4-qx-wy*4+qy));
     assert(b==72+2*(wx*4-qx+wy*4-qy));
     steps++;
    }
   }
  }
 }
 printf("Shared scenery: %u district identities, %u original facade texels, %u exact fractional projections\n",districts,texels,steps);
 return 0;
}
