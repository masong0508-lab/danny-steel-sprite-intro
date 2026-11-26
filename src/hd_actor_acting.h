#pragma once
/* Fixed ground plane; the torso alone follows the existing prop clock. */
typedef struct {int torso,ground;} HdActingAnchor;
static inline HdActingAnchor hd_acting_anchor(int feet,unsigned frame,int walk){
 return (HdActingAnchor){feet+(walk?0:(int)((frame/38u)&1u)),feet};
}
/* Open, half-lidded, closed. Offset each role to avoid ensemble blinks. */
static inline int hd_acting_eye(unsigned frame,unsigned role){
 unsigned phase=(frame%127u+role*17u)%127u;
 return phase>=122u&&phase<=124u?2:phase>=120u&&phase<=126u?1:0;
}
/* Authored animation cadence, not a claim of phoneme-level lip sync. */
static inline int hd_acting_mouth(unsigned frame,unsigned role,int emotion){
 static const unsigned char beat[12]={0,1,2,2,1,0,0,2,1,1,0,0};
 if(emotion!=1)return 0;
 return beat[(frame/4u+role*3u)%12u];
}
