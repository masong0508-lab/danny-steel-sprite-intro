#pragma once
#include "potheads_motion.h"

/* Interpolation for the authored, source-only renderer. The measured
   table remains immutable: every key, contact and visibility mask is
   still selected by pot_trace_sample. No extrapolated anatomy. */
static inline int pot_motion_abs(int n){return n<0?-n:n;}
static inline int pot_motion_round(int v,int d){
 return (v+(v<0?-d/2:d/2))/d;
}
static inline unsigned pot_motion_status(const PotTraceFrame *f,unsigned j){
 unsigned bit=1u<<j;
 return (f->boundary&bit)?3u:(f->visible&bit)?2u:(f->inferred&bit)?1u:0u;
}
static inline unsigned pot_motion_support(unsigned j){
 switch(j){
 case POT_LWRIST:return POT_LEFT_PALM;
 case POT_RWRIST:return POT_RIGHT_PALM;
 case POT_LANKLE:case POT_LTOE:return POT_LEFT_SOLE;
 case POT_RANKLE:case POT_RTOE:return POT_RIGHT_SOLE;
 default:return 0u;
 }
}
/* Limit a tangent to the smaller adjacent signed secant. The endpoint
   tangent is expressed as Q8 displacement over this segment. Opposing
   secants stop the tangent at a reversal instead of ringing past it. */
static inline int pot_motion_tangent(int delta,int other,unsigned d,unsigned dt){
 if(!delta||!other||(delta<0)!=(other<0))return 0;
 int v=pot_motion_abs(other)*(int)d*256/(int)dt;
 int cap=pot_motion_abs(delta)*256;
 if(v>cap)v=cap;
 return delta<0?-v:v;
}
static inline int pot_motion_curve(int prev,int a,int b,int next,unsigned n,
                                    unsigned d,unsigned before,unsigned after){
 if(!n||!d||a==b)return a;
 if(n>=d)return b;
 int t=(int)((n*4096u+d/2u)/d);
 int t2=(t*t+2048)>>12,t3=(t2*t+2048)>>12;
 int h=3*t2-2*t3,k0=t-2*t2+t3,k1=t2-t3;
 int delta=b-a;
 int m0=before?pot_motion_tangent(delta,a-prev,d,before):delta*256;
 int m1=after?pot_motion_tangent(delta,next-b,d,after):delta*256;
 int v=a+pot_motion_round(delta*256*h+m0*k0-m1*k1,4096*256);
 int low=a<b?a:b,high=a>b?a:b;
 return v<low?low:v>high?high:v;
}
/* A visible arrow tip that becomes unobserved should approach the
   nearest corresponding measured board edge, not a fabricated tip.
   Select that edge and its clamped segment fraction at the observed
   key. Interpolate to the corresponding point on the next envelope. */
static inline PotJoint pot_motion_tip_target(const PotTraceFrame *a,const PotTraceFrame *b){
 int tx=a->sign[4][0],ty=a->sign[4][1];
 int best=0x7fffffff, fraction=0;unsigned edge=0;
 for(unsigned i=0;i<4u;i++){
  unsigned j=(i+1u)&3u;
  int x=a->sign[i][0],y=a->sign[i][1];
  int dx=a->sign[j][0]-x,dy=a->sign[j][1]-y;
  int len=dx*dx+dy*dy;
  int u=len?((tx-x)*dx+(ty-y)*dy)*256/len:0;
  if(u<0)u=0;
  if(u>256)u=256;
  int ex=tx-x-pot_motion_round(dx*u,256);
  int ey=ty-y-pot_motion_round(dy*u,256);
  int distance=ex*ex+ey*ey;
  if(distance<best){best=distance;edge=i;fraction=u;}
 }
 unsigned j=(edge+1u)&3u;
 return (PotJoint){
  b->sign[edge][0]+pot_motion_round((b->sign[j][0]-b->sign[edge][0])*fraction,256),
  b->sign[edge][1]+pot_motion_round((b->sign[j][1]-b->sign[edge][1])*fraction,256)
 };
}
static inline int pot_motion_compatible(const PotTraceFrame *a,const PotTraceFrame *b,
                                        unsigned j,unsigned support){
 unsigned state=pot_motion_status(a,j);
 return state&&state<3u&&state==pot_motion_status(b,j)&&
        !((a->contact_mask|b->contact_mask)&support)&&
        !((a->flags^b->flags)&0xfcu);
}
static inline void pot_render_sample(unsigned tick,PotPose *out,int exact_matte){
 pot_trace_sample(tick,out);
 /* Private source mattes are aligned to the original held pose.
    Their contour and landmarks must never be shifted independently. */
 if(exact_matte||tick>=POT_TRACE_LAST_TICK)return;
 unsigned lo=0u,hi=POT_TRACE_COUNT-1u;
 while(lo+1u<hi){
  unsigned mid=(lo+hi)/2u;
  if(pot_trace[mid].tick<=tick)lo=mid;else hi=mid;
 }
 const PotTraceFrame *a=&pot_trace[lo],*b=&pot_trace[hi];
 unsigned n=tick-a->tick,d=b->tick-a->tick;
 if(!n||!d)return;
 const PotTraceFrame *prev=lo?&pot_trace[lo-1u]:a;
 const PotTraceFrame *next=hi+1u<POT_TRACE_COUNT?&pot_trace[hi+1u]:b;
 for(unsigned j=0;j<POT_JOINTS;j++){
  unsigned support=pot_motion_support(j);
  if(!pot_motion_compatible(a,b,j,support))continue;
  unsigned before=lo&&pot_motion_compatible(prev,a,j,support)?a->tick-prev->tick:0u;
  unsigned after=hi+1u<POT_TRACE_COUNT&&pot_motion_compatible(b,next,j,support)?next->tick-b->tick:0u;
  out->joint[j].x=pot_motion_curve(prev->joint[j][0],a->joint[j][0],b->joint[j][0],
                            next->joint[j][0],n,d,before,after);
  out->joint[j].y=pot_motion_curve(prev->joint[j][1],a->joint[j][1],b->joint[j][1],
                            next->joint[j][1],n,d,before,after);
 }
 if((a->flags&3u)&&(b->flags&3u)&&
    (a->flags&POT_SIGN_TIP_VISIBLE)&&!(b->flags&POT_SIGN_TIP_VISIBLE)){
  PotJoint target=pot_motion_tip_target(a,b);
  out->sign[4].x=pot_trace_lerp(a->sign[4][0],target.x,n,d);
  out->sign[4].y=pot_trace_lerp(a->sign[4][1],target.y,n,d);
 }
}
