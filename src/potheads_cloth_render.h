#pragma once
#include <stdint.h>
/* Private source-derived costume track is optional for source-only builds. */
#if defined(__has_include)
# if __has_include("potheads_cloth_data.h")
#  include "potheads_cloth_data.h"
#  define POTHEADS_CLOTH_ENABLED 1
# endif
#endif
#ifndef POTHEADS_CLOTH_ENABLED
# define POTHEADS_CLOTH_ENABLED 0
#endif

typedef struct {int x,y,w,h,lit;} PotClothRect;
static inline int pot_cloth_round(int v){return (v+(v<0?-128:128))/256;}
/* Exact settled projection; shares the camera's scale and rise on descent.
   Clip to the 240x135 film, never paint the caption rows. */
static inline int pot_cloth_rect(PotClothRect *r,int x,int y,int packed,
    int zoom,int ground,int rise,int skew,int ground_y){
    int n=packed&127;
    if(!n||x<0||x+n>240||y<0||y>=135)return 0;
    /* Settled camera is the exact source raster. Avoid projecting every
       span of every source frame, including the optional lower body. */
    if(zoom==256&&ground==48&&rise==256&&skew==0&&ground_y==128){
        *r=(PotClothRect){x,y,n,1,(packed&128)!=0};return 1;
    }
    int a=120+pot_cloth_round((x-120)*zoom+14*skew);
    int b=120+pot_cloth_round((x+n-120)*zoom+14*skew);
    int c=ground_y+pot_cloth_round(14*ground-(131-y)*rise);
    int d=ground_y+pot_cloth_round(14*ground-(130-y)*rise);
    if(b<=a)b=a+1;
    if(d<=c)d=c+1;
    if(a<0)a=0;
    if(b>240)b=240;
    if(c<0)c=0;
    if(d>135)d=135;
    if(b<=a||d<=c)return 0;
    *r=(PotClothRect){a,c,b-a,d-c,(packed&128)!=0};return 1;
}
static inline int pot_source_cloth_frame(unsigned tick){
#if POTHEADS_CLOTH_ENABLED
    unsigned a=0,b=POT_CLOTH_FRAMES-1;
    if(tick>=pot_cloth_tick[b])a=b;
    else while(a+1<b){unsigned m=(a+b)/2;if(pot_cloth_tick[m]<=tick)a=m;else b=m;}
    /* An almost completely occluded or unreliable contour has no spans.
       Retain the old authored painter for only those frames. */
    return pot_cloth_off[a]<pot_cloth_off[a+1]?(int)a:-1;
#else
    (void)tick;return -1;
#endif
}
