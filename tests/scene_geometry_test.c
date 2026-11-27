#include "hd_scene_geometry.h"
#include "hd_facade_art.h"
#include "hd_project_props.h"
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
static void verify_wall_samples(void){
 unsigned count=0;
 const int velocity[]={-32,-22,-9,0,9,22,32};
 for(int x=-257;x<=513;x+=7)for(int y=-257;y<=513;y+=11)
  for(unsigned i=0;i<7u;i++)for(unsigned j=0;j<7u;j++){
   int dx=velocity[i],dy=velocity[j];
   if(!dx&&!dy)continue;
   int wx=x+dx,wy=y+dy,tx=wx>>8,ty=wy>>8,ox=x>>8,oy=y>>8;
   if(tx==ox&&ty==oy)continue;
   HdWallSample sample=hd_wall_sample(wx,wy,dx,dy,tx,ty,ox,oy);
   assert(sample.face<2u&&sample.u<=255u);
   if(tx==ox)assert(!sample.face);
   if(ty==oy)assert(sample.face);
   /* Use independent double-precision intersection for the selected
      physical edge. Integer production rounding differs by <=1 texel. */
   double t;
   int edge,expected;
   if(sample.face){
    edge=(tx+(dx<0))*256;
    t=(double)(edge-x)/dx;
    expected=(int)((double)y+dy*t)-ty*256;
   }else{
    edge=(ty+(dy<0))*256;
    t=(double)(edge-y)/dy;
    expected=(int)((double)x+dx*t)-tx*256;
   }
   assert(t>=0.0&&t<=1.0);
   if(expected<0)expected=0;
   if(expected>255)expected=255;
   if(sample.face?dx<0:dy>0)expected=255-expected;
   assert(hd_scene_abs((int)sample.u-expected)<=1);
   count++;
  }
 printf("wall plane texture mapping: %u crossed-grid ray samples, bounded UV\n",count);
}
static void verify_props(void){
 unsigned count=0;
 for(unsigned which=0;which<2u;which++)for(unsigned warm=0;warm<2u;warm++)
  for(int w=16;w<=40;w+=8){
   uint32_t hashes[5];
   for(unsigned phase=0;phase<=4u;phase++){
    memset(frame,HD_INK,sizeof frame);
    if(which)hd_project_desk(frame,120,120,w,w*2,phase,warm);
    else hd_project_easel(frame,120,120,w,w*2,phase,warm);
    uint32_t hash=2166136261u;
    for(int y=0;y<HD_H;y++)for(int x=0;x<HD_W;x++){
     unsigned ink=frame[y*HD_W+x];assert(ink<64u);
     if(x<120-w/2-3||x>120+w/2+3||y>123)assert(ink==HD_INK);
     hash=(hash^ink)*16777619u;
    }
    for(unsigned prior=0;prior<phase;prior++)assert(hashes[prior]!=hash);
    hashes[phase]=hash;count++;
   }
  }
 printf("creative props: %u distinct stage rasters, grounded/clipped bounds\n",count);
}
static void verify_billboard_edges(void){
 uint16_t depth[HD_W];uint8_t type[HD_W];
 unsigned cases=0;
 for(int x=0;x<HD_W;x++){depth[x]=200;type[x]=1;}
 assert(!hd_billboard_visible(120,60,300,depth,type,HD_W));
 depth[105]=350;
 assert(hd_billboard_visible(120,60,300,depth,type,HD_W));
 depth[105]=200;
 type[0]=0;
 assert(hd_billboard_visible(-3,60,300,depth,type,HD_W));
 assert(!hd_billboard_visible(-100,60,300,depth,type,HD_W));
 type[0]=1;
 depth[239]=350;
 assert(hd_billboard_visible(244,60,300,depth,type,HD_W));
 for(int x=0;x<HD_W;x++){depth[x]=(uint16_t)(100+(x*67)%900);type[x]=(uint8_t)(x%11!=0);}
 for(int centre=-48;centre<=288;centre++)for(int h=8;h<=115;h+=3)
  for(int d=85;d<=2200;d+=37){
   int want=0,radius=h*4/11+4;if(radius<8)radius=8;
   for(int x=0;x<HD_W;x++)
    if(x>=centre-radius&&x<=centre+radius&&(!type[x]||d<=depth[x])){want=1;break;}
   assert(hd_billboard_visible(centre,h,d,depth,type,HD_W)==want);cases++;
  }
 printf("billboard occlusion: %u independent visible-interval checks including clipped screen edges\n",cases);
}
int main(void){
 verify_wall_samples();verify_props();verify_billboard_edges();
 unsigned districts=0,steps=0,texels=0;
 for(int y=-1;y<=STORY_H;y++)for(int x=-1;x<=STORY_W;x++){
  char t=story_tile(x,y);
  unsigned height,variant;legacy_district(x,y,t,&height,&variant);
  HdDistrict d=hd_district(x,y,t);
  if(!story_city_extension(x,y))assert(d.height==height&&d.variant==variant);
  else assert(d.height>=32u&&d.height<=52u&&d.variant<8u);
  for(unsigned side=0;side<2u;side++)assert(hd_facade_at(x,y,t,side)==hd_facade_texture(d.variant,side));
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
