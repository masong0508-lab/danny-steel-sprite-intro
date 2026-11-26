#pragma once
/* A screen-down gravity blend rather than a binary inverted costume.
   down_y is the measured torso direction in signed Q8. */
static inline int pot_drape_weight(int down_y){
 if(down_y>=128)return 0;
 if(down_y<=-128)return 256;
 return 128-down_y;
}
static inline int pot_drape_mix(int a,int b,int w){
 return (a*(256-w)+b*w)/256;
}
