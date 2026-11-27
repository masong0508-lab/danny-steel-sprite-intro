#include "hd_ray_trace.h"
#include "hd_scene_geometry.h"
#include <assert.h>
#include <stdio.h>
static void same(const HdRayHit *a,const HdRayHit *b){
 assert(a->type==b->type&&a->depth==b->depth);
 if(a->type){
  assert(a->wx==b->wx&&a->wy==b->wy&&a->tx==b->tx&&a->ty==b->ty);
  assert(a->from_x==b->from_x&&a->from_y==b->from_y);
 }
}
int main(void){
 static const int fx[]={1,0,-1,0},fy[]={0,1,0,-1};
 unsigned checks=0;
 for(int y=-1;y<=STORY_H;y++)for(int x=-1;x<=STORY_W;x++){
  for(unsigned facing=0;facing<4u;facing++)
   for(int a=0;a<4;a++)for(int b=0;b<4;b++)for(int side=-22;side<=22;side++){
    int cx=x*256+a*64,cy=y*256+b*64;
    int dx=fx[facing]*32-fy[facing]*side;
    int dy=fy[facing]*32+fx[facing]*side;
    HdRayHit ref,fast;
    hd_ray_reference(cx,cy,dx,dy,&ref);hd_ray_trace(cx,cy,dx,dy,&fast);same(&ref,&fast);
    if(ref.type){
     HdWallSample p=hd_wall_sample(ref.wx,ref.wy,dx,dy,ref.tx,ref.ty,ref.from_x,ref.from_y);
     HdWallSample q=hd_wall_sample(fast.wx,fast.wy,dx,dy,fast.tx,fast.ty,fast.from_x,fast.from_y);
     assert(p.face==q.face&&p.u==q.u);
    }
    checks++;
   }
 }
 for(int x=-129;x<600;x+=19)for(int y=-129;y<600;y+=29)for(int side=-22;side<=22;side+=7){
  HdRayHit ref,fast;hd_ray_reference(x,y,32,side,&ref);hd_ray_trace(x,y,32,side,&fast);
  same(&ref,&fast);checks++;
 }
 printf("ray crossings: %u exact hit/depth/plane/UV comparisons, quarter-tile phases and off-grid fallback\n",checks);
 return 0;
}
