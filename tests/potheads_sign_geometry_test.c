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
static void check_uv(const PotPoint q[4],PotPoint p,unsigned want_u,unsigned want_v,unsigned error){
 PotSignMap map;assert(pot_sign_map_init(q,&map));
 int e[4];for(unsigned i=0;i<4u;i++)e[i]=pot_sign_cross(q[i],q[(i+1u)&3u],p);
 unsigned u=999,v=999;assert(pot_sign_map_uv(&map,e,&u,&v));
 assert(u<=63u&&v<=25u);
 assert(u+error>=want_u&&u<=want_u+error&&v+error>=want_v&&v<=want_v+error);
}
int main(void){
 /* A single trapezoidal projective surface: every corner maps exactly,
    and the midpoint is the diagonal intersection, not the midpoint of
    either independently affine triangle. */
 const PotPoint rect[4]={{0,0},{80,0},{80,40},{0,40}};
 check_uv(rect,(PotPoint){0,0},0,0,0);
 check_uv(rect,(PotPoint){80,0},63,0,0);
 check_uv(rect,(PotPoint){80,40},63,25,0);
 check_uv(rect,(PotPoint){0,40},0,25,0);
 check_uv(rect,(PotPoint){40,20},32,13,0);
 const PotPoint trapezoid[4]={{0,0},{80,0},{60,40},{20,40}};
 check_uv(trapezoid,(PotPoint){0,0},0,0,0);
 check_uv(trapezoid,(PotPoint){80,0},63,0,0);
 check_uv(trapezoid,(PotPoint){60,40},63,25,0);
 check_uv(trapezoid,(PotPoint){20,40},0,25,0);
 check_uv(trapezoid,(PotPoint){40,27},32,13,1);
 const PotPoint singular[4]={{0,0},{80,0},{80,1},{0,0}};
 PotSignMap bad;assert(!pot_sign_map_init(singular,&bad));
 unsigned stepping=0;
 for(unsigned d=1;d<=33553920u;d=d*3u+1u){
  for(unsigned limit=25u;limit<=63u;limit+=38u){
   /* n <= min(65535*256*limit, d*limit), as in the real edge map. */
   uint32_t max_n=d<16776960u?d*limit:16776960u*limit;
   for(unsigned k=0;k<=128u;k++){
    uint32_t n=(uint32_t)((uint64_t)max_n*k/128u);
    unsigned expected=(n+d/2u)/d;
    for(unsigned prior=0;prior<=limit;prior++)
     assert(pot_sign_uv_step(prior,n,d,limit)==expected);
    stepping++;
   }
  }
 }
 printf("Exact incremental sign quotients: %u denominator/numerator cases with all starting texels\n",stepping);
 unsigned held=0,falling=0,clipped=0,readable=0,hidden=0,planar=0,stabilized=0,uv_samples=0;
 for(unsigned t=0;t<=POT_TRACE_LAST_TICK;t++){
  PotPose p;pot_trace_sample(t,&p);
  if(!(p.flags&3u)){hidden++;continue;}
  if((p.flags&3u)==POT_SIGN_HELD)held++;else falling++;
  if(p.flags&POT_SIGN_CLIPPED)clipped++;
  for(unsigned time=0;time<=180u;time+=30u){
   PotCamera c=pot_camera(time);PotPoint q[5],face[4];
   for(unsigned j=0;j<5u;j++)q[j]=pot_actor_project(&c,p.sign[j].x,p.sign[j].y);
   check(q,p.flags);
   PotPoint old[5];for(unsigned i=0;i<5u;i++)old[i]=q[i];
   int changed=pot_sign_stabilize(q,p.flags);stabilized+=(unsigned)changed;
   int dx=0,dy=0;
   for(unsigned i=0;i<4u;i++){
    int x=q[i].x-old[i].x,y=q[i].y-old[i].y;
    assert(x>=-1&&x<=1&&y>=-1&&y<=1);
    dx+=x;dy+=y;
    if(p.flags&POT_SIGN_CLIPPED)assert(!x&&!y);
   }
   assert(!dx&&!dy&&q[4].x==old[4].x&&q[4].y==old[4].y);
   check(q,p.flags);
   if(pot_sign_face(q,p.flags,face)){
    readable++;
    PotSignMap map;
    if(pot_sign_map_init(face,&map)){
     planar++;
     for(unsigned i=0;i<4u;i++){
      unsigned u,v;int e[4];
      for(unsigned j=0;j<4u;j++)e[j]=pot_sign_cross(face[j],face[(j+1u)&3u],face[i]);
      assert(pot_sign_map_uv(&map,e,&u,&v));
      assert(u==((i==1u||i==2u)?63u:0u)&&v==(i>=2u?25u:0u));uv_samples++;
     }
     /* Check a screen-space grid including rejected pixels outside the
        exact face; no texture access can ever exceed the authored art. */
     for(int y=0;y<135;y+=11){
      unsigned prev_u=0,prev_v=0;int seeded=0;
      for(int x=0;x<240;x++){
       int e[4];for(unsigned j=0;j<4u;j++)e[j]=pot_sign_cross(face[j],face[(j+1u)&3u],(PotPoint){x,y});
       unsigned u=0,v=0;
       int ok=pot_sign_map_uv(&map,e,&u,&v);
       assert(pot_sign_map_uv_next(&map,e,&prev_u,&prev_v,seeded)==ok);
       if(ok){assert(u<64u&&v<26u&&prev_u==u&&prev_v==v);seeded=1;uv_samples++;}
      }
     }
    }
   }
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
 assert(planar>0u&&stabilized>0u&&uv_samples>0u);
 printf("Sign: %u held ticks, %u falling ticks, %u clipped ticks, %u readable projections; %u planar maps, %u bounded stabilizations, %u UV checks\n",held,falling,clipped,readable,planar,stabilized,uv_samples);
 return 0;
}
