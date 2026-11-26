#pragma once
#include "potheads_leg_render.h"
/* The upper-body matte excludes pixels from BOTH earlier source layers.
   Only enable the new replacement when those layers are installed. */
#if POTHEADS_CLOTH_ENABLED && POTHEADS_LEG_ENABLED && defined(__has_include)
# if __has_include("potheads_upper_data.h")
#  include "potheads_upper_data.h"
#  define POTHEADS_UPPER_ENABLED 1
# endif
#endif
#ifndef POTHEADS_UPPER_ENABLED
# define POTHEADS_UPPER_ENABLED 0
#endif
static inline int pot_upper_valid(int y,int x,unsigned packed,int height){
 unsigned ink=packed>>5,n=(packed&31u)+1u;
 return ink>=1u&&ink<=5u&&x>=0&&y>=0&&x<240&&y<135&&
        n<=(unsigned)(240-x)&&height>0&&height<=135-y;
}
static inline int pot_source_upper_frame(unsigned tick,int cloth,int leg){
#if POTHEADS_UPPER_ENABLED
 if(cloth<0||leg<0)return -1;
 unsigned a=0,b=POT_UPPER_FRAMES-1u;
 if(tick>=pot_upper_tick[b])a=b;
 else while(a+1u<b){unsigned m=(a+b)/2u;if(pot_upper_tick[m]<=tick)a=m;else b=m;}
 if(a!=(unsigned)cloth||a!=(unsigned)leg||pot_upper_off[a]==pot_upper_off[a+1u])return -1;
 return (int)a;
#else
 (void)tick;(void)cloth;(void)leg;return -1;
#endif
}
static inline unsigned pot_upper_pose_tick(unsigned tick,int frame){
#if POTHEADS_UPPER_ENABLED
 if(frame>=0&&(unsigned)frame<POT_UPPER_FRAMES)return pot_upper_tick[frame];
#else
 (void)frame;
#endif
 return tick;
}
/* Every available matte is a held source-camera frame. Keep the
   procedural face, hands, sign, bag and contacts on that same timestamp,
   even when only the leg or costume matte is available. The generated
   tracks share one timeline; upper is preferred when all layers exist. */
static inline unsigned pot_source_pose_tick(unsigned tick,int cloth,int leg,int upper){
#if POTHEADS_UPPER_ENABLED
 if(upper>=0&&(unsigned)upper<POT_UPPER_FRAMES)return pot_upper_tick[upper];
#else
 (void)upper;
#endif
#if POTHEADS_CLOTH_ENABLED
 if(cloth>=0&&(unsigned)cloth<POT_CLOTH_FRAMES)return pot_cloth_tick[cloth];
#else
 (void)cloth;
#endif
#if POTHEADS_LEG_ENABLED
 if(leg>=0&&(unsigned)leg<POT_LEG_FRAMES)return pot_leg_tick[leg];
#else
 (void)leg;
#endif
 return tick;
}
/* Exact settled rectangle; the moving-camera path expands rows using the
   original per-span projection, preserving the old rounding and clipping. */
static inline int pot_upper_settled_rect(PotClothRect *r,int y,int x,unsigned packed,int height){
 if(!pot_upper_valid(y,x,packed,height))return 0;
 *r=(PotClothRect){x,y,(int)((packed&31u)+1u),height,0};return 1;
}
