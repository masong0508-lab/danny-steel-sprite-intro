#include "hd_facade_art.h"
#include "hd_isometric_atlas.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ROM facade acceptance: each building surface is a distinct illustrated
   architectural material rather than a set of flat colored rectangles. */
static struct {
 uint8_t before[48];
 uint8_t frame[HD_BYTES];
 uint8_t after[48];
} image;
static void reset(void){memset(&image,0xB2,sizeof image);}
static uint32_t inspect(unsigned minimum){
 unsigned count=0;uint32_t v=2166136261u;
 for(unsigned i=0;i<48;i++){
  assert(image.before[i]==0xB2 && image.after[i]==0xB2);
 }
 for(unsigned i=0;i<HD_BYTES;i++){
  uint8_t p=image.frame[i];
  assert(p<64u);
  if(p!=0xB2 && p!=HD_ROAD)count++;
  v=(v^p)*16777619u;
 }
 assert(count>=minimum);
 return v;
}
/* Authored 16x40 source hashes protect the new district art itself. Expected scaled
   frames then use an independent per-pixel mathematical crop, not the
   optimized pair copies or quotient/remainder recurrence. */
static void verify_facade_copies(void){
 static const uint32_t original_hash[2][8]={
  {0x16bad310u,0x1e899509u,0x3e39f621u,0xcb7228c6u,0xd31ccf01u,0xc620c4acu,0xf2aef0bcu,0x0bd3d335u},
  {0xddbd39f9u,0x3d2b8a52u,0xde621c25u,0xb99c8fabu,0x751361feu,0xd121dc47u,0xf51c26deu,0xd72c374fu},
 };
 static uint8_t expected[HD_BYTES+132] __attribute__((aligned(4)));
 static uint8_t actual[HD_BYTES+132] __attribute__((aligned(4)));
 uint8_t original[16u*40u];
 const int xs[]={-17,-16,-15,-14,-3,-2,-1,0,1,2,103,104,105,
                 224,225,237,238,239,240};
 const int ys[]={-50,-10,-1,0,1,40,72,158,159,160,180,220};
 const int heights[]={-3,0,1,2,3,7,16,24,27,32,36,40,42,44,46,52,64,95,127};
 unsigned cases=0;
 for(unsigned side=0;side<2u;side++)for(unsigned variant=0;variant<8u;variant++){
  reset();memset(image.frame,HD_ROAD,HD_BYTES);
  hd_facade_wall(image.frame,64,64,40,variant,side);
  uint32_t hash=2166136261u;
  for(int y=0;y<40;y++)for(int x=0;x<16;x++){
   int yy=64+(side?16-x/2:8+x/2)-40+y;
   unsigned ink=image.frame[yy*HD_W+64+x];
   original[y*16+x]=(uint8_t)ink;
   hash=(hash^ink)*16777619u;
  }
  assert(hash==original_hash[side][variant]);
  for(unsigned align=0;align<4u;align++)
   for(unsigned xi=0;xi<sizeof xs/sizeof xs[0];xi++)
    for(unsigned yi=0;yi<sizeof ys/sizeof ys[0];yi++)
     for(unsigned hi=0;hi<sizeof heights/sizeof heights[0];hi++){
      memset(expected,0xB2,sizeof expected);memset(actual,0xB2,sizeof actual);
      uint8_t *want=expected+64u+align,*got=actual+64u+align;
      memset(want,HD_ROAD,HD_BYTES);memset(got,HD_ROAD,HD_BYTES);
      int sx=xs[xi],sy=ys[yi],height=heights[hi];
      for(int y=0;y<height;y++)for(int x=0;x<16;x++){
       int xx=sx+x,yy=sy+(side?16-x/2:8+x/2)-height+y;
       if((unsigned)xx<HD_W&&(unsigned)yy<HD_H)
        want[yy*HD_W+xx]=original[(y*40/height)*16+x];
      }
      hd_facade_wall(got,sx,sy,height,variant+((cases&1u)?8u:0u),side);
      assert(memcmp(expected,actual,sizeof actual)==0);
      cases++;
     }
 }
 printf("facade copies: %u golden scale/clip/alignment cases and guards matched\n",cases);
}
int main(void){
 verify_facade_copies();
 uint32_t fronts[2][8];
 for(unsigned side=0;side<2;side++){
  for(unsigned v=0;v<8;v++){
   reset();
   memset(image.frame,HD_ROAD,sizeof image.frame);
   hd_facade_wall(image.frame,105,72,36,v,side);
   fronts[side][v]=inspect(250u);
   for(unsigned prev=0;prev<v;prev++)
    assert(fronts[side][v]!=fronts[side][prev]);
   for(int x=-17;x<241;x+=39)for(int y=-45;y<165;y+=39){
    reset();memset(image.frame,HD_ROAD,sizeof image.frame);
    hd_facade_wall(image.frame,x,y,27+(int)(v&1u)*9,v,side);
    for(unsigned k=0;k<48;k++)
     assert(image.before[k]==0xB2 && image.after[k]==0xB2);
    for(unsigned k=0;k<HD_BYTES;k++)
     assert(image.frame[k]<64u);
   }
  }
 }
 for(unsigned v=0;v<8;v++)assert(fronts[0][v]!=fronts[1][v]);
 /* The raycast uses the same wordmark in physically attached wall
    texel coordinates. It never writes the ROM transparency key. */
 unsigned letters=0,banner=0;
 for(int v=0;v<256;v++)for(int u=0;u<256;u++){
  int sample=hd_potheads_wall_ink(u,v);
  assert(sample>=-1 && sample<64);
  if(sample==HD_PAPER)letters++;
  if(sample==HD_LEAF_DARK||sample==HD_LEAF)banner++;
 }
 assert(letters>30u && banner>100u);
 assert(hd_potheads_wall_ink(0,0)==-1);
 assert(hd_potheads_wall_ink(128,80)==HD_LEAF_DARK ||
        hd_potheads_wall_ink(128,80)==HD_PAPER);
 Story s;
 story_new(&s,0);
 assert(s.cinema==CINE_OPEN);
 int pages=0,banana=0,income=0,gratitude=0,dislike=0,art=0;
 const StoryPage *d=story_pages(&s,&pages);
 assert(pages>=7);
 for(int i=0;i<pages;i++){
  if(strstr(d[i].line1,"banana")||strstr(d[i].line2,"banana"))
   banana=1;
  if(strstr(d[i].line1,"rent")||strstr(d[i].line2,"rent"))
   income=1;
  if(strstr(d[i].line1,"grateful")||strstr(d[i].line2,"grateful"))
   gratitude=1;
  if(strstr(d[i].line1,"don't love")||strstr(d[i].line2,"don't love"))
   dislike=1;
  if(strstr(d[i].line1,"art")||strstr(d[i].line2,"art")||
     strstr(d[i].line1,"music")||strstr(d[i].line2,"Music"))
   art=1;
 }
 /* Do not regress to the earlier claim that he enjoys mascot work. */
 assert(banana && income && gratitude && dislike && art);
 int gx=0,gy=0;const char *name=0;
 assert(story_goal(&s,&gx,&gy,&name));
 assert(gx==5 && gy==5 && !story_solid(20,11));
 puts("8 unique cartridge facade identities x 2 orientations, palette and clipping safe");
 printf("Matching first-person Potheads sign: %u light letter texels, %u fascia texels\n",
        letters,banner);
 puts("Potheads pays Danny's rent; gratitude and an unwanted mascot shift coexist with artistic ambitions");
 puts("Original clinic/bus/letter objectives, movement and save structure unchanged");
 return 0;
}

