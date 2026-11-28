#pragma once
/* Pure screen-space helpers. The four points are a measured envelope,
   not calibrated corners. The fifth is used only when actually visible. */
static inline int pot_sign_cross(PotPoint a,PotPoint b,PotPoint c){
 return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);
}
static inline int pot_sign_dist(PotPoint a,PotPoint b){
 int x=b.x-a.x,y=b.y-a.y;return (x<0?-x:x)+(y<0?-y:y);
}
static inline unsigned pot_sign_hull(const PotPoint *q,unsigned count,PotPoint out[5]){
 PotPoint sorted[5],h[10];unsigned n=0,k=0;
 if(count>5u)return 0;
 for(unsigned i=0;i<count;i++){
  unsigned j=n;while(j&&(sorted[j-1].x>q[i].x||
       (sorted[j-1].x==q[i].x&&sorted[j-1].y>q[i].y))){sorted[j]=sorted[j-1];j--;}
  sorted[j]=q[i];n++;
 }
 for(unsigned i=0;i<n;i++){
  if(i&&sorted[i].x==sorted[i-1].x&&sorted[i].y==sorted[i-1].y)continue;
  while(k>=2u&&pot_sign_cross(h[k-2],h[k-1],sorted[i])<=0)k--;
  h[k++]=sorted[i];
 }
 if(k<=1u){if(k)out[0]=h[0];return k;}
 unsigned lower=k;
 for(int i=(int)n-2;i>=0;i--){
  if(sorted[i].x==sorted[i+1].x&&sorted[i].y==sorted[i+1].y)continue;
  while(k>lower&&pot_sign_cross(h[k-2],h[k-1],sorted[i])<=0)k--;
  h[k++]=sorted[i];
 }
 if(k>1u)k--;
 for(unsigned i=0;i<k;i++)out[i]=h[i];
 return k;
}
static inline int pot_sign_near(const PotPoint *h,unsigned n,PotPoint p,int radius){
 if(n<3u)return 0;
 for(unsigned i=0;i<n;i++){
  PotPoint a=h[i],b=h[(i+1u)%n];
  if(pot_sign_cross(a,b,p)<-radius*pot_sign_dist(a,b))return 0;
 }
 return 1;
}
static inline int pot_sign_face(const PotPoint q[5],unsigned flags,PotPoint out[4]){
 if(flags&POT_SIGN_CLIPPED)return 0;
 /* Orient artwork toward the observed arrow end, even upside down. */
 int reverse=(flags&POT_SIGN_TIP_VISIBLE)&&
    pot_sign_dist(q[4],q[0])+pot_sign_dist(q[4],q[3])<
    pot_sign_dist(q[4],q[1])+pot_sign_dist(q[4],q[2]);
 for(unsigned i=0;i<4u;i++)out[i]=q[(i+(reverse?2u:0u))&3u];
 int area=0;
 for(unsigned i=0;i<4u;i++){
  int turn=pot_sign_cross(out[i],out[(i+1u)&3u],out[(i+2u)&3u]);
  if(turn<=0)return 0; /* folds and edge-on frames are not texture planes */
  area+=turn;
 }
 int w0=pot_sign_dist(out[0],out[1]),w1=pot_sign_dist(out[3],out[2]);
 int w=w0>w1?w0:w1;
 /* The short projected dimension must accommodate two readable rows;
    avoid turning hundreds of glyph pixels into random one-pixel flecks. */
 int h0=pot_sign_cross(out[0],out[1],out[2]);
 int h1=pot_sign_cross(out[0],out[1],out[3]);
 int h2=pot_sign_cross(out[2],out[3],out[0]);
 int h3=pot_sign_cross(out[2],out[3],out[1]);
 /* Full print, muted print, or material only. The lower detail level
    avoids an abrupt white slab when the board turns through a readable
    but foreshortened angle. Never map a full logo onto a cropped fragment. */
 if(w0>=43&&w1>=43&&area>=w*56&&h0>=w0*10&&h1>=w0*10&&
    h2>=w1*10&&h3>=w1*10)return 2;
 if(w0>=34&&w1>=34&&area>=w*32&&h0>=w0*6&&h1>=w0*6&&
    h2>=w1*6&&h3>=w1*6)return 1;
 return 0;
}

/* Modest display-only correction of the measured face. Preserve centre,
   genuine folds, crops and observed arrow tip. At most one pixel per
   axis per corner; only full-print, near-parallelogram poses qualify. */
static inline int pot_sign_small_shift(int v){
 int a=v<0?-v:v;
 return a<4?0:v<0?-1:1;
}
static inline int pot_sign_stabilize(PotPoint q[5],unsigned flags){
 PotPoint face[4];int detail=pot_sign_face(q,flags,face);
 if(detail!=2||!(flags&POT_SIGN_TIP_VISIBLE))return 0;
 int dx=q[0].x+q[2].x-q[1].x-q[3].x;
 int dy=q[0].y+q[2].y-q[1].y-q[3].y;
 if(dx<-16||dx>16||dy<-16||dy>16)return 0;
 int x=pot_sign_small_shift(dx),y=pot_sign_small_shift(dy);
 if(!x&&!y)return 0;
 PotPoint next[5];
 for(unsigned i=0;i<5u;i++)next[i]=q[i];
 for(unsigned i=0;i<4u;i++){
  int s=(i&1u)?1:-1;
  next[i].x+=s*x;next[i].y+=s*y;
 }
 if(pot_sign_face(next,flags,face)!=detail)return 0;
 for(unsigned i=0;i<4u;i++)q[i]=next[i];
 return 1;
}
/* Single planar inverse projective map, replacing the two incompatible
   affine triangles. Edge weights derive from opposite corner areas.
   Normalize once to Q8 and reject extreme perspective; in-face cross
   products are bounded to 65535, so texel numerators fit uint32_t. */
typedef struct {uint16_t weight[4];} PotSignMap;
static inline int pot_sign_map_init(const PotPoint q[4],PotSignMap *map){
 uint32_t a[4];
 for(unsigned i=0;i<4u;i++){
  int v=pot_sign_cross(q[i],q[(i+1u)&3u],q[(i+2u)&3u]);
  if(v<=0||v>65535)return 0;
  a[i]=(uint32_t)v;
 }
 /* Edge order top,right,bottom,left. a = ABC,BCD,CDA,DAB. */
 uint32_t w[4]={a[1]*a[2],a[3]*a[2],a[0]*a[3],a[0]*a[1]};
 uint32_t max=0,min=UINT32_MAX;
 for(unsigned i=0;i<4u;i++){
  if(w[i]>max)max=w[i];
  if(w[i]<min)min=w[i];
 }
 if(!max||(uint64_t)min*16u<max)return 0;
 for(unsigned i=0;i<4u;i++)map->weight[i]=(uint16_t)(((uint64_t)w[i]*256u+max/2u)/max);
 return 1;
}
/* Rounded rational coordinate update, exact for odd/even denominators.
   Adjacent pixels of a convex projective face are monotone in each
   coordinate; use the previous quotient rather than a software divide. */
static inline unsigned pot_sign_uv_step(unsigned prior,uint32_t n,uint32_t d,unsigned limit){
 uint32_t twice=n*2u;
 while(prior&&twice<(2u*prior-1u)*d)--prior;
 while(prior<limit&&twice>=(2u*prior+1u)*d)++prior;
 return prior;
}
static inline int pot_sign_map_uv_next(const PotSignMap *map,const int edge[4],unsigned *u,unsigned *v,int seeded){
 uint32_t e[4];
 for(unsigned i=0;i<4u;i++){
  if(edge[i]<0||edge[i]>65535)return 0;
  e[i]=(uint32_t)edge[i]*map->weight[i];
 }
 uint32_t du=e[1]+e[3],dv=e[0]+e[2];
 if(!du||!dv)return 0;
 uint32_t nu=e[3]*63u,nv=e[0]*25u;
 if(seeded){
  *u=pot_sign_uv_step(*u,nu,du,63u);
  *v=pot_sign_uv_step(*v,nv,dv,25u);
 }else{
  *u=(nu+du/2u)/du;
  *v=(nv+dv/2u)/dv;
 }
 return 1;
}
static inline int pot_sign_map_uv(const PotSignMap *map,const int edge[4],unsigned *u,unsigned *v){
 return pot_sign_map_uv_next(map,edge,u,v,0);
}
