#include "hd_canvas.h"
#include "hd_potheads_set.h"
#include "potheads_motion.h"
#include "potheads_sign_geometry.h"
#include <assert.h>
#include <stdio.h>
static void check(PotPoint q[5],unsigned flags){
 PotPoint h[5],face[4];unsigned n=pot_sign_hull(q,flags&POT_SIGN_TIP_VISIBLE?5u:4u,h);
 assert(n>0&&n<=5u);
 if(n>=3u){
  for(unsigned i=0;i<n;i++){
   assert(pot_sign_cross(h[i],h[(i+1u)%n],h[(i+2u)%n])>0);
   for(unsigned j=0;j<(flags&POT_SIGN_TIP_VISIBLE?5u:4u);j++)
    assert(pot_sign_cross(h[i],h[(i+1u)%n],q[j])>=0);
  }
 }
 if(flags&POT_SIGN_CLIPPED)assert(!pot_sign_face(q,flags,face));
}
int main(void){
 unsigned held=0,falling=0,clipped=0,readable=0,hidden=0;
 for(unsigned t=0;t<=POT_TRACE_LAST_TICK;t++){
  PotPose p;pot_trace_sample(t,&p);
  if(!(p.flags&3u)){hidden++;continue;}
  if((p.flags&3u)==POT_SIGN_HELD)held++;else falling++;
  if(p.flags&POT_SIGN_CLIPPED)clipped++;
  for(unsigned time=0;time<=180u;time+=30u){
   PotCamera c=pot_camera(time);PotPoint q[5],face[4];
   for(unsigned j=0;j<5u;j++)q[j]=pot_actor_project(&c,p.sign[j].x,p.sign[j].y);
   check(q,p.flags);
   if(pot_sign_face(q,p.flags,face))readable++;
  }
 }
 PotPoint collapsed[5]={{1,1},{2,2},{3,3},{4,4},{100,100}};
 PotPoint h[5],face[4];
 assert(pot_sign_hull(collapsed,4u,h)==2u);
 assert(!pot_sign_face(collapsed,0u,face));
 PotPoint square[5]={{0,0},{80,0},{80,40},{0,40},{100,20}};
 assert(pot_sign_hull(square,4u,h)==4u);
 assert(pot_sign_hull(square,5u,h)==5u);
 assert(pot_sign_near(h,5u,(PotPoint){50,20},0));
 assert(!pot_sign_near(h,5u,(PotPoint){120,20},2));
 assert(pot_sign_face(square,POT_SIGN_TIP_VISIBLE,face)==2);
 PotPoint mid[5]={{0,0},{40,0},{40,12},{0,12},{50,6}};
 assert(pot_sign_face(mid,POT_SIGN_TIP_VISIBLE,face)==1);
 assert(!pot_sign_face(mid,POT_SIGN_CLIPPED|POT_SIGN_TIP_VISIBLE,face));
 PotPoint thin[5]={{0,0},{40,0},{40,3},{0,3},{50,1}};
 assert(!pot_sign_face(thin,POT_SIGN_TIP_VISIBLE,face));
 assert(held==428u&&falling==24u&&hidden>0&&clipped>0&&readable>0);
 printf("Sign: %u held ticks, %u falling ticks, %u clipped ticks, %u readable camera projections; envelope, observed tip, crop and grip bounds OK\n",held,falling,clipped,readable);
 return 0;
}
