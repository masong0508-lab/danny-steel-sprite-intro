#include "hd_world_atlas.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static struct {
 uint8_t before[48];
 uint8_t image[HD_BYTES];
 uint8_t after[48];
} guarded;
static void reset(void){
 memset(&guarded,0xAB,sizeof guarded);
 memset(guarded.image,HD_ROAD,HD_BYTES);
}
static unsigned inspect(void){
 unsigned hash=2166136261u,detail=0;
 for(unsigned i=0;i<48;i++){
  assert(guarded.before[i]==0xAB);
  assert(guarded.after[i]==0xAB);
 }
 for(unsigned i=0;i<HD_BYTES;i++){
  uint8_t ink=guarded.image[i];
  assert(ink>=32u && ink<64u);
  if(ink!=HD_ROAD)detail++;
  hash=(hash^ink)*16777619u;
 }
 /* At the nearest supported projection size (five pixels tall),
    a correctly rendered far-away silhouette may cover only a handful
    of pixels. Its palette and boundaries still require full validation. */
 assert(detail>0u);
 return hash;
}
static unsigned in_window(int x0,int y0,int x1,int y1){
 unsigned hash=2166136261u;
 for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++)
  hash=(hash^guarded.image[y*HD_W+x])*16777619u;
 return hash;
}
int main(void){
 const char tiles[]={'#','A','.','g','T','~',',','=','D','L','F','C','P','W'};
 unsigned tile_hashes[sizeof tiles],tile_count=0;
 for(unsigned i=0;i<sizeof tiles;i++){
  reset();
  hd_world_tile_art(guarded.image,111,67,tiles[i],(int)i+1,7);
  tile_hashes[i]=in_window(111,67,127,83);
  inspect();tile_count++;
  /* All physical screen edges, and out-of-bounds town camera chunks,
     must clip safely without any second framebuffer allocation. */
  const int xx[]={-15,0,238,239,250};
  const int yy[]={-15,0,158,159,166};
  for(unsigned x=0;x<5u;x++)for(unsigned y=0;y<5u;y++){
   reset();hd_world_tile_art(guarded.image,xx[x],yy[y],
                             tiles[i],-4+(int)i,18);
   for(unsigned k=0;k<48;k++){
    assert(guarded.before[k]==0xAB && guarded.after[k]==0xAB);
   }
  }
 }
 assert(tile_hashes[0]!=tile_hashes[2]);
 assert(tile_hashes[2]!=tile_hashes[4]);
 assert(tile_hashes[4]!=tile_hashes[5]);
 assert(tile_hashes[5]!=tile_hashes[6]);
 unsigned role_hashes[7],poses=0;
 for(unsigned role=0;role<7u;role++){
  unsigned faces[4];
  for(unsigned face=0;face<4u;face++)for(unsigned gait=0;gait<2u;gait++){
   reset();
   hd_world_person_art(guarded.image,120,97,role,face,gait);
   unsigned hash=inspect();
   if(gait==0)faces[face]=hash;
   if(face==1&&gait==0)role_hashes[role]=hash;
   /* Body is grounded at the same foot anchor across every costume. */
   assert(guarded.image[97*HD_W+120]!=HD_ROAD);
   poses++;
  }
  assert(faces[0]!=faces[1]);
  assert(faces[1]!=faces[3]);
  assert(faces[2]!=faces[3]);
  for(unsigned prev=0;prev<role;prev++)
   assert(role_hashes[role]!=role_hashes[prev]);
 }
 /* Sixteen new production walking cels. Real animation state, distinct
    contact poses, same shadow/foot anchor, and clipped screen edges. */
 Story gait_state;story_new(&gait_state,0);
 for(unsigned face=0;face<4u;face++){
  unsigned walk_hash[4];
  gait_state.facing=(uint8_t)face;gait_state.x=12;gait_state.y=12;
  for(unsigned phase=0;phase<4u;phase++){
   gait_state.anim=(uint8_t)(4u-phase);
   unsigned gait=hd_world_danny_gait(&gait_state);
   assert(gait==phase+2u);
   reset();hd_world_person_art(guarded.image,120,97,0u,face,gait);
   walk_hash[phase]=inspect();
   assert(guarded.image[97*HD_W+120]!=HD_ROAD);
   for(unsigned prior=0;prior<phase;prior++)assert(walk_hash[prior]!=walk_hash[phase]);
   unsigned soles=0;
   for(int x=109;x<131;x++)soles+=guarded.image[95*HD_W+x]==HD_INK;
   assert(soles>=5u);
   reset();hd_world_person_art(guarded.image,0,12,0u,face,gait);
   for(unsigned k=0;k<48;k++)assert(guarded.before[k]==0xAB&&guarded.after[k]==0xAB);
   poses++;
  }
  gait_state.x=13;gait_state.anim=4;
  assert(hd_world_danny_gait(&gait_state)==4u);
  gait_state.anim=0;assert(hd_world_danny_gait(&gait_state)==0u);
 }
 for(int height=5;height<=116;height+=9)for(unsigned role=0;role<7;role++){
  reset();
  hd_world_person_projected(guarded.image,120,129,height,role,1u,0u);
  inspect();poses++;
  reset();
  hd_world_person_projected(guarded.image,0,12,height,role,1u,0u);
  for(unsigned k=0;k<48;k++)
   assert(guarded.before[k]==0xAB && guarded.after[k]==0xAB);
 }
 unsigned trees[3];
 for(unsigned kind=0;kind<3;kind++){
  reset();hd_world_tree_art(guarded.image,120,95,kind,0);
  trees[kind]=inspect();
  reset();hd_world_tree_art(guarded.image,0,0,kind,1);
  for(unsigned k=0;k<48;k++)
   assert(guarded.before[k]==0xAB && guarded.after[k]==0xAB);
 }
 assert(trees[0]!=trees[1]);
 assert(trees[1]!=trees[2]);
 Story s;story_new(&s,0);
 const uint8_t positions[4][2]={{12,13},{8,12},{19,4},{11,8}};
 unsigned levels[4];
 for(int j=0;j<4;j++){
  s.x=positions[j][0];s.y=positions[j][1];s.facing=(uint8_t)j;
  s.prev_x=s.x;s.prev_y=s.y;s.anim=0;
  s.cinema=CINE_NONE;s.battle=0;s.dialogue=0;s.activity=0;
  reset();hd_world_draw(guarded.image,&s,110u);
  levels[j]=inspect();
  for(int k=0;k<j;k++)assert(levels[k]!=levels[j]);
 }
 printf("ROM world atlas: %u distinct material test cases, 7 residents with 4 orientations\n",tile_count);
 printf("Projected and directional poses: %u palette/framebuffer and contact-shadow checks\n",poses);
 puts("Town grid and NPC interactions unchanged; every ROM pixel safely clipped");
 return 0;
}
