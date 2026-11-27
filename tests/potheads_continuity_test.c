#include "potheads_continuity.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int bounded(int v,int a,int b){return v>=(a<b?a:b)&&v<=(a>b?a:b);}
int main(void){
 unsigned arithmetic=0,changed=0,tip_changed=0,keys=0,contacts=0;
 for(int a=-128;a<=127;a+=13)for(int b=-128;b<=127;b+=11)
  for(int p=-128;p<=127;p+=37)for(int q=-128;q<=127;q+=41)
   for(unsigned d=2u;d<=16u;d+=2u){
    int last=a;
    for(unsigned n=0;n<=d;n++){
     int v=pot_motion_curve(p,a,b,q,n,d,10u,12u);
     assert(bounded(v,a,b));
     assert((a<=b&&v>=last)||(a>b&&v<=last));
     if(!n)assert(v==a);
     if(n==d)assert(v==b);
     last=v;arithmetic++;
    }
   }
 unsigned lo=0;
 for(unsigned tick=0;tick<=POT_TRACE_LAST_TICK;tick++){
  while(lo+1u<POT_TRACE_COUNT&&pot_trace[lo+1u].tick<=tick)lo++;
  const PotTraceFrame *a=&pot_trace[lo],*b=lo+1u<POT_TRACE_COUNT?&pot_trace[lo+1u]:a;
  PotPose old={0},smooth={0},matte={0};
  pot_trace_sample(tick,&old);pot_render_sample(tick,&smooth,0);pot_render_sample(tick,&matte,1);
  assert(!memcmp(&old,&matte,sizeof old));
  assert(old.flags==smooth.flags&&old.located==smooth.located&&old.visible==smooth.visible);
  assert(old.inferred==smooth.inferred&&old.boundary==smooth.boundary);
  assert(old.contact_mask==smooth.contact_mask&&old.bag_state==smooth.bag_state);
  assert(!memcmp(old.contact,smooth.contact,sizeof old.contact));
  assert(!memcmp(&old.bag,&smooth.bag,sizeof old.bag));
  assert(!memcmp(&old.head_size,&smooth.head_size,sizeof old.head_size));
  assert(!memcmp(old.sign,smooth.sign,4u*sizeof(PotJoint)));
  if(tick==a->tick){assert(!memcmp(&old,&smooth,sizeof old));keys++;}
  for(unsigned j=0;j<POT_JOINTS;j++){
   int same=old.joint[j].x==smooth.joint[j].x&&old.joint[j].y==smooth.joint[j].y;
   if(!same)changed++;
   if(!pot_motion_compatible(a,b,j,pot_motion_support(j)))assert(same);
   else{
    assert(bounded(smooth.joint[j].x,a->joint[j][0],b->joint[j][0]));
    assert(bounded(smooth.joint[j].y,a->joint[j][1],b->joint[j][1]));
   }
   if((a->contact_mask|b->contact_mask)&pot_motion_support(j)){assert(same);contacts++;}
  }
  if(old.sign[4].x!=smooth.sign[4].x||old.sign[4].y!=smooth.sign[4].y){
   assert(a->flags&POT_SIGN_TIP_VISIBLE);
   assert(!(b->flags&POT_SIGN_TIP_VISIBLE)&&(b->flags&3u));
   PotJoint target=pot_motion_tip_target(a,b);
   assert(bounded(smooth.sign[4].x,a->sign[4][0],target.x));
   assert(bounded(smooth.sign[4].y,a->sign[4][1],target.y));tip_changed++;
  }
 }
 assert(keys==POT_TRACE_COUNT&&changed>0&&contacts>0&&tip_changed>0);
 printf("Motion continuity: %u arithmetic checks, %u exact measured keys, %u smoothed joint samples, %u preserved support samples, %u bounded tip-transition samples\n",
        arithmetic,keys,changed,contacts,tip_changed);
 return 0;
}
