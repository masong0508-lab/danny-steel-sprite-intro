#pragma once
#include "potheads_cloth_render.h"
/* Source-derived lower-body spans use five authored palette inks.
   A zero-span frame is a real crop, not an instruction to invent legs. */
#if defined(__has_include)
# if __has_include("potheads_leg_data.h")
#  include "potheads_leg_data.h"
#  define POTHEADS_LEG_ENABLED 1
# endif
#endif
#ifndef POTHEADS_LEG_ENABLED
# define POTHEADS_LEG_ENABLED 0
#endif
static inline int pot_source_leg_frame(unsigned tick){
#if POTHEADS_LEG_ENABLED
 unsigned a=0,b=POT_LEG_FRAMES-1u;
 if(tick>=pot_leg_tick[b])return (int)b;
 while(a+1u<b){unsigned m=(a+b)/2u;if(pot_leg_tick[m]<=tick)a=m;else b=m;}
 return (int)a;
#else
 (void)tick;return -1;
#endif
}
static inline int pot_leg_rect(PotClothRect *r,unsigned *ink,int x,int y,unsigned packed,
 int zoom,int ground,int rise,int skew,int ground_y){
 unsigned color=packed>>5;
 if(color<1u||color>5u)return 0;
 if(!pot_cloth_rect(r,x,y,(int)((packed&31u)+1u),zoom,ground,rise,skew,ground_y))return 0;
 *ink=color;return 1;
}
