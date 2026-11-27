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
