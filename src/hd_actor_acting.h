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

/* Exact speaker labels, including archive aliases. Narration, texts and
   off-screen voices never animate a visible character's mouth. */
static inline int hd_acting_label(const char *a,const char *b){
 if(!a)return 0;
 while(*a&&*a==*b){a++;b++;}
 return *a==*b;
}
static inline unsigned hd_acting_speaker_role(const char *label){
 if(hd_acting_label(label,"DANNY"))return 1u;
 if(hd_acting_label(label,"MARA"))return 2u;
 if(hd_acting_label(label,"BORE CLERK")||hd_acting_label(label,"ARCHIVIST"))return 3u;
 if(hd_acting_label(label,"WITNESS"))return 4u;
 if(hd_acting_label(label,"DOCTOR"))return 5u;
 return 0u;
}
