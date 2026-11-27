#include "hd_canvas.h"
#include "hd_facade_art.h"
#include "hd_wall_shades.h"
#include <assert.h>
#include <stdio.h>
static const uint8_t dim[32]={HD_ROAD,HD_ROAD,HD_ROAD_COOL,HD_WET,HD_MORTAR,HD_STONE,
 HD_STONE,HD_STONE_DARK,HD_BRICK_DARK,HD_BRICK_DARK,HD_BRICK,
 HD_STONE,HD_PLASTER,HD_SHADOW,HD_GOLD,HD_GLASS,HD_GLASS,
 HD_ROAD_COOL,HD_BARK,HD_LEAF_DARK,HD_LEAF_DARK,HD_LEAF,
 HD_BARK,HD_PLASTER,HD_INK,HD_STONE_LIT,HD_RED_SHADE,HD_RED_SHADE,
 HD_SKIN,HD_SKIN,HD_HAIR,HD_COPPER};
static unsigned tree(unsigned u,unsigned v){
 uint32_t n=((u>>3)+31u)*1664525u+((v>>3)+79u)*1013904223u;
 n^=n>>13;n*=2246822519u;n^=n>>11;
 if(v>225&&(u&15u)==1u)return HD_STONE;
 if(v>178&&u>91&&u<162)return HD_BARK;
 return (n&3u)==0u?HD_LEAF_LIT:HD_LEAF;
}
int main(void){
 unsigned checks=0;
 for(unsigned face=0;face<2;face++)for(unsigned variant=0;variant<8;variant++)
  for(unsigned far=0;far<2;far++)for(unsigned u=0;u<256;u++){
   const uint8_t *a=hd_facade_texture(variant,face);
   const uint8_t *b=hd_wall_shades[hd_wall_strip_index[((face*8u+variant)*2u+far)*16u+(u>>4)]];
   for(unsigned v=0;v<256;v++){
    unsigned c=a[(v*40u>>8)*16u+(u>>4)];
    assert(b[v]==(far?dim[c-32u]:c));checks++;
   }
  }
 for(unsigned u=0;u<256;u++){
  const uint8_t *b=hd_wall_strip('T',4,2,0,u,0);
  for(unsigned v=0;v<256;v++){assert(b[v]==tree(u,v));checks++;}
 }
 for(unsigned face=0;face<2;face++)for(unsigned far=0;far<2;far++)
  for(unsigned u=0;u<256;u++){
   const uint8_t *a=hd_facade_texture(7,face),*b=hd_wall_strip('#',21,11,face,u,far);
   for(unsigned v=0;v<256;v++){
    unsigned c=a[(v*40u>>8)*16u+(u>>4)];
    int sign=hd_potheads_wall_ink((int)u,(int)v);
    assert(b[v]==(sign>=0?(unsigned)sign:far?dim[c-32u]:c));checks++;
   }
  }
 for(unsigned depth=32;depth<=2304;depth+=32){
  unsigned d=depth>2300?2300:depth,h=20800/(d+60);
  if(h>150)h=150;
  h=2u*(h/2u);
  const uint8_t *v=hd_wall_v_samples(h);
  for(unsigned y=0;y<h;y++){assert(v[y]==y*256u/h);checks++;}
 }
 printf("Wall shading: %u exact source shade and vertical projection comparisons\n",checks);
 return 0;
}
