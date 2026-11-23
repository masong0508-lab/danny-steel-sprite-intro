#include "hd_canvas.h"
#include "hd_world_atlas.h"
#include "hd_facade_art.h"
#if defined(GBA_PROFILE_RENDER) && defined(__arm__)
#include "gba.h"
/* Debug-build-only counters. Normal release builds allocate NONE of these.
   TM3 is reserved for PSG music; this probe temporarily uses TM0/TM1. */
volatile uint32_t gba_profile_frames;
volatile uint32_t gba_profile_sky_cycles;
volatile uint32_t gba_profile_wall_cycles;
volatile uint32_t gba_profile_wallshade_cycles;
volatile uint32_t gba_profile_floor_cycles;
volatile uint32_t gba_profile_billboard_cycles;
volatile uint32_t gba_profile_weather_cycles;
volatile uint32_t gba_profile_total_cycles;
static void gba_profile_begin(void){
 R16(0x04000102)=0u;R16(0x04000106)=0u;
 R16(0x04000100)=0u;R16(0x04000104)=0u;
 R16(0x04000106)=0x0084u; /* TM1 cascade from TM0 overflow */
 R16(0x04000102)=0x0080u; /* TM0 full-rate CPU clock */
}
static uint32_t gba_profile_clock(void){
 uint16_t hi,low;
 do {
  hi=R16(0x04000104);
  low=R16(0x04000100);
 }while(hi!=R16(0x04000104));
 __asm__ volatile("" ::: "memory");
 return ((uint32_t)hi<<16)|low;
}
#endif

/* A full 240-column perspective renderer for the actual same 24x18
   collision map. All steps and ray offsets are fixed-point, cardinal facing
   exactly matches story_tick(). There is no second depth framebuffer:
   the 240 x uint16 z strip is a small persistent work array.
   Consecutive columns with the SAME original quantized direction share
   ray and wall shading work; every native column and depth entry remains. */
static uint16_t wall_depth[HD_W];
/* Sparse native-column depth/texture cache, not another framebuffer. */
static uint8_t wall_type[HD_W],wall_u[HD_W],wall_tx[HD_W],wall_ty[HD_W];
/* Fixed cartridge-town wall shade memo: only 45 sampled ray directions,
   at most 150 vertical palette entries per ray (6,750 EWRAM bytes).
   This is NOT another framebuffer and never caches moving sky or rain. */
enum { WALL_RAYS=45, WALL_SHADE_ROWS=150 };
static uint8_t wall_shade_cache[WALL_RAYS][WALL_SHADE_ROWS]
 __attribute__((section(".ewram"),aligned(4)));
static int wall_cache_valid,wall_cam_x,wall_cam_y,wall_cam_fx,wall_cam_fy;
#if !defined(__arm__)
void hd_debug_invalidate_wall_cache(void){wall_cache_valid=0;}
#endif
/* Ends of the exact original ((x-120)*3)/16 ray-direction runs. */
static const uint8_t ray_end[45]={
 3,9,14,19,25,30,35,41,46,51,57,62,67,73,78,83,89,94,99,105,110,115,
 126,131,136,142,147,152,158,163,168,174,179,184,190,195,200,206,211,216,
 222,227,232,238,240
};
static int abs_i(int a){return a<0?-a:a;}
static HD_CANVAS_CODE inline unsigned texel(unsigned x,unsigned y){
 uint32_t v=(x+31u)*1664525u+(y+79u)*1013904223u;
 v^=v>>13;v*=2246822519u;return (unsigned)(v^(v>>11));
}
static void dirs(const Story *s,int *fx,int *fy,int *rx,int *ry){
 switch(s->facing&3u){
  case 0:*fx=1;*fy=0;*rx=0;*ry=1;break;
  case 1:*fx=0;*fy=1;*rx=-1;*ry=0;break;
  case 2:*fx=-1;*fy=0;*rx=0;*ry=-1;break;
  default:*fx=0;*fy=-1;*rx=1;*ry=0;break;
 }
}
static void set_sky(uint8_t *p,unsigned tick,int facing,int cx,int cy){
 /* Seven maximal horizontal material runs replace 160 independent row
    painter calls. Pixel colors, cut lines and the horizon stay exact:
    our full-width word-span painter writes each contiguous range once. */
 hd_rect(p,0,0,HD_W,24,19);
 hd_rect(p,0,24,HD_W,30,17);
 hd_rect(p,0,54,HD_W,20,HD_GLASS);
 hd_rect(p,0,74,HD_W,10,HD_GLASS_LIT);
 hd_rect(p,0,84,HD_W,18,HD_ROAD_COOL);
 hd_rect(p,0,102,HD_W,23,HD_ROAD_LIGHT);
 hd_rect(p,0,125,HD_W,35,HD_ROAD);
 /* Layered, place-bound horizon. Neither buildings nor cloud positions
    randomly change when the shot cuts to a dialogue panel. */
 int pan=(facing*71+(cx+cy)/96)%53;
 for(int i=-2;i<19;i++){
  int x=i*23-pan;
  int ht=17+(int)(texel((unsigned)(i+33),23u)%22u);
  hd_rect(p,x,79-ht,23,ht,HD_STONE_DARK);
  hd_rect(p,x+3,79-ht-2,18,2,HD_ROAD);
  /* Sloped overlapping roofs add believable street silhouettes. */
  hd_line(p,x-2,79-ht,x+10,73-ht,HD_STONE_DARK);
  hd_line(p,x+10,73-ht,x+25,79-ht,HD_STONE_DARK);
  hd_line(p,x+1,79-ht,x+21,79-ht,HD_STONE);
  if((i&3)==0){
   hd_rect(p,x+14,69-ht,3,10,HD_ROAD_COOL);
   hd_rect(p,x+13,68-ht,6,2,HD_STONE_DARK);
  }
  for(int w=3;w<21;w+=7)for(int h=7;h<ht-2;h+=9){
   if((texel((unsigned)(i+41+w),(unsigned)h+35u)&3u)!=0u)
    hd_rect(p,x+w,79-ht+h,2,3,(tick/180u+i+w)%5u==0u?HD_LAMP:HD_GLASS);
  }
 }
 for(int k=0;k<4;k++){
  int x=(k*73+35-(int)(tick/42u)%40)-23;
  int y=17+(k&1)*8;
  hd_line(p,x,y,x+32,y,HD_STONE_DARK);
  hd_line(p,x+6,y-2,x+24,y-2,HD_STONE_DARK);
 }
 /* A flat-toned, quiet horizon instead of procedural neon sky. */
 hd_line(p,0,79,239,79,HD_STONE);
}
/* x+y is always in 80..378 in the visible native floor painter.
   A 400-byte ROM mask replaces a constant-divisor test per paved pixel. */
static const uint8_t floor_dither17[HD_W+HD_H] = {
 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0
};
static HD_CANVAS_CODE inline unsigned road_floor_ink(int wx,int wy,int x,int y,
                                                     unsigned noise){
 int fx=wx&255,fy=wy&255;
 if((fx<5||fy<5)&&(wx+wy)%3==0)return HD_ROAD_COOL;
 if((noise&31u)==0u)return HD_REFLECT;
 if((noise&7u)==1u)return HD_ROAD_LIGHT;
 return floor_dither17[x+y]?HD_ROAD_COOL:HD_ROAD;
}
#if !defined(__arm__)
unsigned hd_debug_floor_dither17(int x,int y){return floor_dither17[x+y];}
#endif
static HD_CANVAS_CODE inline unsigned floor_ink(char t,int wx,int wy,int x,int y,unsigned tick){
 int fx=wx&255,fy=wy&255;
 if(t=='~'){
  if(((fy+((int)tick*2))&63)<4)return HD_GLASS_LIT;
  return (texel((unsigned)wx>>5,(unsigned)wy>>5)&3u)==0u?HD_REFLECT:HD_GLASS;
 }
 if(t==','||t=='='){
  if(fx<8||fy<7)return HD_MORTAR;
  if(fx>244||fy>244)return HD_STONE_DARK;
  if(((fx>>5)^(fy>>6))&1)return HD_STONE_LIT;
  return HD_STONE;
 }
 if(t=='g'||t=='T'){
  if((texel((unsigned)(wx>>5),(unsigned)(wy>>5))&3u)==0u)return HD_LEAF;
  return HD_LEAF_DARK;
 }
 if(t=='#'||t=='A')return HD_BRICK_DARK;
 return road_floor_ink(wx,wy,x,y,texel((unsigned)(wx>>4),
                                        (unsigned)(wy>>4)));
}
/* Keep the measured pixel loops in ARM/IWRAM. noinline prevents the
   Thumb entry point from absorbing them back into cartridge code. */
static HD_CANVAS_CODE __attribute__((noinline)) void floor_render(uint8_t *p,const Story *s,int cx,int cy,
                         int fx,int fy,int rx,int ry,unsigned tick){
 (void)s;
 /* One projection calculation per scanline, and half-rate horizontal
    sampling in the far distance only. Near pavement uses each GBA pixel. */
 for(int y=80;y<140;y++){
  int dist=13500/(y-75); if(dist>2304)dist=2304;
  int side_unit=dist*3*256/480;
  int step=y<98?2:1;
  int side_delta=step*side_unit;
  /* These forward terms are constant for the entire floor scanline. */
  int forward_x=cx+fx*dist,forward_y=cy+fy*dist;
  /* Existing wall bottom is 79 + floor(10400/(depth+60)).
     Thus y is covered exactly when depth <= 10400/(y-78)-60.
     At these floor rows the original 3..150 height clamp cannot alter
     that test. Reuse the depth strip instead of an extra coverage buffer. */
  int wall_limit=10400/(y-78)-60;
  int last_tx=-99,last_ty=-99;
  /* Sample successive road pixels often share the exact same 16-unit
     world-material cell. Remember one hash per scanline (stack only). */
  int last_hx=-2147483647,last_hy=-2147483647;
  unsigned road_noise=0u;
  char tile='.';
  int begin=0;
  for(unsigned ray=0;ray<sizeof ray_end;ray++){
   int end=ray_end[ray];
   /* cast_walls gives every column in this original direction run
      the same hit and depth. Test coverage once, not per hidden pixel. */
   if(!wall_type[begin]||wall_depth[begin]>wall_limit){
    int start=begin&~(step-1);
    int side_acc=(start-120)*side_unit;
    for(int x=start;x<end;x+=step,side_acc+=side_delta){
     /* Exact original signed fixed-point projection with additions after
        one run initialization; no geometry wobble or changed row sampling. */
     int side=side_acc>>8;
     int wx=forward_x+rx*side,wy=forward_y+ry*side;
     int tx=wx>>8,ty=wy>>8;
     if(tx!=last_tx||ty!=last_ty){
      last_tx=tx;last_ty=ty;tile=story_tile(tx,ty);
     }
     unsigned ink;
     if(tile=='.'){
      int hx=wx>>4,hy=wy>>4;
      if(hx!=last_hx||hy!=last_hy){
       last_hx=hx;last_hy=hy;
       road_noise=texel((unsigned)hx,(unsigned)hy);
      }
      ink=road_floor_ink(wx,wy,x,y,road_noise);
     }else ink=floor_ink(tile,wx,wy,x,y,tick);
     /* An odd run edge may split a far sample. Keep its original
        even sample x, then paint only this run's visible member(s). */
     if(x>=begin)p[y*HD_W+x]=(uint8_t)ink;
     if(step==2&&x+1<end)p[y*HD_W+x+1]=(uint8_t)ink;
    }
   }
   begin=end;
  }
 }
}
/* Width is constant for a ray, while each row has a palette shade. EWRAM
   supports aligned halfword stores: pack two identical pixels into one
   write instead of performing one byte store + loop branch per column.
   Odd heads and tails remain byte-exact; no VRAM access or extra buffers. */
typedef uint16_t WallHalf __attribute__((may_alias));
static HD_CANVAS_CODE inline void wall_store_span(volatile uint8_t *row,
               unsigned head,unsigned pairs,unsigned tail,uint8_t ink){
 if(head)*row++=ink;
 volatile WallHalf *half=(volatile WallHalf *)(void *)row;
 uint16_t fill=(uint16_t)((unsigned)ink*257u);
 for(unsigned k=0;k<pairs;k++)half[k]=fill;
 if(tail)((volatile uint8_t *)(void *)half)[pairs*2u]=ink;
}
#if !defined(__arm__)
void hd_debug_wall_store_span(uint8_t *canvas,int x,int y,int width,uint8_t ink){
 volatile uint8_t *row=canvas+y*HD_W+x;
 unsigned head=(unsigned)((uintptr_t)row&1u);
 unsigned pairs=((unsigned)width-head)>>1u;
 unsigned tail=((unsigned)width-head)&1u;
 wall_store_span(row,head,pairs,tail,ink);
}
#endif
static HD_CANVAS_CODE void wall_column(uint8_t *p,int x,int width,int top,int bottom,char type,
                        int u,int forward,int tx,int ty,uint8_t *save_ink){
#if defined(GBA_PROFILE_RENDER) && defined(__arm__)
 uint32_t wallshade_start=gba_profile_clock();
#endif
 int height=bottom-top;
 int tree=type=='T';
 unsigned base=tree?HD_LEAF_DARK:type=='A'?HD_BRICK:HD_BRICK_DARK;
 if(forward>1200)base=tree?HD_LEAF_DARK:HD_STONE_DARK;
 /* The older painter performed a software integer division for EVERY
    wall pixel. ARM7TDMI has no hardware divider. Quotient/remainder
    DDA produces precisely floor((y-top)*256/height) per scanline, but
    pays for the division only twice per wall column. */
 int v=0,major=256/height,minor=256%height,error=0;
 /* HD_W is even, so these alignment and span counts are constant
    across every screen row in this wall ray. */
 unsigned head=(unsigned)((uintptr_t)(p+x)&1u);
 unsigned pairs=((unsigned)width-head)>>1u;
 unsigned tail=((unsigned)width-head)&1u;
 for(int y=top;y<bottom;y++){
  if((unsigned)y>=140u)break;
  unsigned c=base;
  if(tree){
   if(v>178 && u>91 && u<162)c=HD_BARK;
   else if((texel((unsigned)(u>>3),(unsigned)(v>>3))&3u)==0u)
    c=HD_LEAF_LIT;
   else c=HD_LEAF;
  }else{
   /* Facade windows fixed to wall texture coordinates, not screen x.
      Floor breaks, mortar and stone relief persist as camera moves. */
   int window=u>=53&&u<=202&&v>=57&&v<=163;
   int door=u>=101&&u<=160&&v>=165;
   if(v<15)c=HD_PLASTER_LIT;
   else if(v<22 || v>238)c=HD_STONE_DARK;
   else if((v&31)<3)c=HD_BRICK_DARK;
   else if((u&31)<3)c=HD_BRICK_LIT;
   else if(door){
    if(u<106||u>155)c=HD_COPPER;
    else if(v<170)c=HD_STONE_DARK;
    else if(u>129&&u<133)c=HD_BRICK_DARK;
    else c=((tx+ty)&1)?HD_BARK:HD_BRICK_DARK;
    if(u>=143&&u<=147&&v>=204&&v<=208)c=HD_LAMP;
   }else if(window){
    if(u<60||u>196||v<65||v>154)c=HD_COPPER;
    else if(u>=124&&u<=132)c=HD_BRICK_DARK;
    else if(v>=102&&v<=109)c=HD_BRICK_DARK;
    else if(v<92&&u>140&&u<153)c=HD_LAMP;
    else if((u&31)<2)c=HD_COPPER;
    else c=((tx+ty)&1)?HD_GLASS_LIT:HD_GLASS;
   }else if((texel((unsigned)(u>>4)+(unsigned)(tx+40),
                         (unsigned)(v>>4)+(unsigned)(ty+40))&7u)==0u)
    c=HD_BRICK_LIT;
  }
  if(v>225&&(u&15)==1)c=HD_STONE;
  /* Same Potheads shop, same town coordinate in both cameras. This is
     environmental branding, not the future handheld/dancing sign. */
  if(!tree && tx==21 && ty==11){
   int signage=hd_potheads_wall_ink(u,v);
   if(signage>=0)c=(unsigned)signage;
  }
  /* These native spans are at most 11 bytes. Keep the short stores
     here: GCC otherwise replaces them with a ROM memset per scanline. */
  volatile uint8_t *out=p+y*HD_W+x;
  wall_store_span(out,head,pairs,tail,(uint8_t)c);
  if(save_ink)save_ink[y-top]=(uint8_t)c;
  v+=major;
  error+=minor;
  if(error>=height){v++;error-=height;}
 }
 if(top>18 && top<140)hd_rect(p,x,top,width,1,tree?HD_LEAF_LIT:HD_STONE_LIT);
#if defined(GBA_PROFILE_RENDER) && defined(__arm__)
 gba_profile_wallshade_cycles+=gba_profile_clock()-wallshade_start;
#endif
}
/* Warm-camera path reads only the exact indexed shades computed by the
   original wall shader on the first draw at this pose. The same row-span
   writer and cap-edge rule keep all 240 native columns pixel-identical. */
static HD_CANVAS_CODE void wall_cached_column(uint8_t *p,int x,int width,
                              int top,int bottom,char type,const uint8_t *shades){
#if defined(GBA_PROFILE_RENDER) && defined(__arm__)
 uint32_t wallshade_start=gba_profile_clock();
#endif
 unsigned head=(unsigned)((uintptr_t)(p+x)&1u);
 unsigned pairs=((unsigned)width-head)>>1u;
 unsigned tail=((unsigned)width-head)&1u;
 for(int y=top;y<bottom;y++){
  if((unsigned)y>=140u)break;
  wall_store_span(p+y*HD_W+x,head,pairs,tail,shades[y-top]);
 }
 if(top>18 && top<140)
  hd_rect(p,x,top,width,1,type=='T'?HD_LEAF_LIT:HD_STONE_LIT);
#if defined(GBA_PROFILE_RENDER) && defined(__arm__)
 gba_profile_wallshade_cycles+=gba_profile_clock()-wallshade_start;
#endif
}
static HD_CANVAS_CODE __attribute__((noinline)) void cast_walls(uint8_t *p,int cx,int cy,int fx,int fy,int rx,int ry){
 int warm=wall_cache_valid&&wall_cam_x==cx&&wall_cam_y==cy&&
          wall_cam_fx==fx&&wall_cam_fy==fy;
 unsigned slot=0;
 for(int x=0;x<HD_W;){
  int side=((x-120)*3)/16; /* 0.75 FOV at each edge, fixed-point */
  /* The original integer projection gives adjacent columns identical
     directions. Reuse that exact result, without reducing sampling. */
  int end=x+1;
  while(end<HD_W && ((end-120)*3)/16==side)end++;
  int dx=fx*32+rx*side,dy=fy*32+ry*side;
  int wx=cx,wy=cy,hit_dist=2400,tx=0,ty=0;
  int previous_tx=cx>>8,previous_ty=cy>>8;
  char hit=0;
  for(int step=1;step<76;step++){
   wx+=dx;wy+=dy;
   tx=wx>>8;ty=wy>>8;
   /* 32-step ray samples often sit in the SAME town tile.
      Only touch the story grid on cell crossings. Same collision set,
      depth and view, but many fewer tile queries and branch chains. */
   if(tx==previous_tx && ty==previous_ty)continue;
   previous_tx=tx;previous_ty=ty;
   char t=story_tile(tx,ty);
   if(t=='#'||t=='A'||t=='T'){hit=t;hit_dist=step*32;break;}
  }
  if(hit_dist>2300)hit_dist=2300;
  for(int col=x;col<end;col++){
   wall_depth[col]=(uint16_t)hit_dist;
   wall_type[col]=(uint8_t)hit;
  }
  if(!hit){x=end;slot++;continue;}
  int height=20800/(hit_dist+60);
  if(height>150)height=150;
  if(height<3)height=3;
  int top=79-height/2,bottom=79+height/2;
  int u=abs_i(dx)>abs_i(dy)?wy&255:wx&255;
  for(int col=x;col<end;col++){
   wall_u[col]=(uint8_t)u;
   wall_tx[col]=(uint8_t)tx;
   wall_ty[col]=(uint8_t)ty;
  }
  if(warm&&slot<WALL_RAYS)
   wall_cached_column(p,x,end-x,top,bottom,hit,wall_shade_cache[slot]);
  else
   wall_column(p,x,end-x,top,bottom,hit,u,hit_dist,tx,ty,
               slot<WALL_RAYS?wall_shade_cache[slot]:0);
  x=end;slot++;
 }
 wall_cache_valid=slot==WALL_RAYS;
 wall_cam_x=cx;wall_cam_y=cy;wall_cam_fx=fx;wall_cam_fy=fy;
}
static void actor(uint8_t *p,int sx,int ground,int h,char kind,unsigned tick){
 if(h<8)return;
 /* World-grounded people use the same authored 4-direction sprites in
    both views. The old rectangles stay only for physical shop markers. */
 if(kind=='F'||kind=='C'||kind=='P'||kind=='V'||kind=='Q'||kind=='W'){
  unsigned role=kind=='F'?1u:kind=='C'?2u:kind=='P'?3u:
                kind=='V'?4u:kind=='Q'?5u:6u;
  hd_world_person_projected(p,sx,ground,h,role,1u,0u);
  return;
 }
 int w=h*4/11;if(w<5)w=5;
 unsigned coat=kind=='F'?HD_GLASS:kind=='C'?HD_GLASS_LIT:
   kind=='P'?HD_COPPER:kind=='V'?HD_BRICK:kind=='Q'?HD_RED_SHADE:
   kind=='W'?HD_LEAF:kind=='B'?HD_RED:kind=='M'?HD_BRICK_LIT:
   kind=='R'?HD_COPPER:kind=='K'?HD_PLASTER:HD_STONE;
 hd_ellipse(p,sx,ground+1,w/2+2,2,HD_SHADOW);
 if(kind=='B'){
  hd_rect(p,sx-w,ground-h/2,w*2,h/3,HD_RED_SHADE);
  hd_rect(p,sx-w+2,ground-h/2+2,w*2-4,h/4,HD_RED);
  hd_rect(p,sx-w+3,ground-h/2+3,w-3,h/6,HD_GLASS);
  hd_rect(p,sx+2,ground-h/2+3,w-4,h/6,HD_GLASS_LIT);
  hd_ellipse(p,sx-w/2,ground-h/7,3,3,HD_INK);
  hd_ellipse(p,sx+w/2,ground-h/7,3,3,HD_INK);
 }else if(kind=='L'||kind=='K'||kind=='R'||kind=='M'||kind=='S'||kind=='H'){
  /* Physical shopfront/letter/terminal silhouette, with legible signage
     on approach instead of a floating seven-pixel quest marker. */
  hd_rect(p,sx-w/2,ground-h,w,h,HD_INK);
  hd_rect(p,sx-w/2+2,ground-h+2,w-4,h-4,coat);
  hd_rect(p,sx-w/2+3,ground-h+4,w-6,3,HD_PAPER);
  if(h>=45){
   const char *title=kind=='K'?"CLINIC":kind=='R'?"ART":
                     kind=='M'?"MUSIC":kind=='S'?"DATA":
                     kind=='H'?"HOME":"LETTER";
   hd_text(p,sx-w/2+3,ground-h+9,title,HD_INK,1,0);
  }
 }else{
  int head=h/4;if(head<4)head=4;
  hd_rect(p,sx-w/4,ground-h+1,head,head+1,HD_SKIN);
  hd_rect(p,sx-w/4,ground-h+1,head,head/3+1,HD_HAIR);
  hd_rect(p,sx-w/2,ground-h+head,w,h/2,coat);
  hd_rect(p,sx-w/2+1,ground-h+head+2,2,h/3,HD_STONE_DARK);
  hd_rect(p,sx+w/2-3,ground-h+head+2,2,h/3,HD_STONE_DARK);
  hd_rect(p,sx-w/4,ground-h/4,3,h/4,HD_INK);
  hd_rect(p,sx+w/8,ground-h/4,3,h/4,HD_INK);
  if(h>25){
   int bob=(int)(tick/27u&1u);
   hd_pixel(p,sx-2,ground-h+head/2+bob,HD_INK);
   hd_pixel(p,sx+2,ground-h+head/2+bob,HD_INK);
   hd_line(p,sx,ground-h+head+1,sx,ground-h/2,HD_COPPER);
  }
  if(kind=='F'&&h>30){
   hd_rect(p,sx-w/4-2,ground-h+1,2,head+8,HD_HAIR);
   hd_rect(p,sx+w/4+2,ground-h+1,2,head+9,HD_HAIR);
  }
 }
}
typedef struct {
 int16_t sx,ground,height,depth;
 uint8_t kind;
} SceneBillboard;
/* Exactly the interactive/nonwall actors in story.c's 24x18 map, in the
   original row-major order. Kept in cartridge ROM, not a dynamic pool.
   The old painter rescanned all 432 tiles for these 13 static entries on
   every first-person frame. Host map/anchor tests keep the list honest. */
typedef struct {
 uint8_t x,y,kind;
} SceneAnchor;
static const SceneAnchor scene_anchors[]={
 {7,2,'H'},
 {4,5,'L'},{16,5,'K'},{19,5,'B'},
 {14,6,'R'},
 {3,7,'P'},{11,7,'C'},{17,7,'V'},{19,7,'M'},{21,7,'S'},
 {6,11,'W'},{8,11,'F'},{18,12,'Q'}
};
#if !defined(__arm__)
/* Host-only guard: refuse a stale ROM anchor table after any map edit. */
int hd_first_person_anchor_map_valid(void){
 unsigned count=0;
 for(int y=0;y<STORY_H;y++)for(int x=0;x<STORY_W;x++){
  char kind=story_tile(x,y);
  if(kind!='F'&&kind!='C'&&kind!='B'&&kind!='L'&&kind!='P'&&
     kind!='V'&&kind!='Q'&&kind!='S'&&kind!='W'&&kind!='H'&&
     kind!='R'&&kind!='M'&&kind!='K')continue;
  if(count>=sizeof scene_anchors/sizeof scene_anchors[0])return 0;
  const SceneAnchor *a=&scene_anchors[count++];
  if((int)a->x!=x||(int)a->y!=y||(char)a->kind!=kind)return 0;
 }
 return count==sizeof scene_anchors/sizeof scene_anchors[0];
}
#endif
#define MAX_VISIBLE_BILLBOARDS 24u
static SceneBillboard visible[MAX_VISIBLE_BILLBOARDS];
/* Restore occluding wall columns AFTER a sprite is drawn.
   This preserves hidden-surface order without a 240x160 sprite buffer. */
static void restore_occluding_wall(uint8_t *p,int sx,int h,int depth){
 int radius=h*4/11+4;
 if(radius<8)radius=8;
 for(int x=sx-radius;x<=sx+radius;x++){
  if((unsigned)x>=HD_W || !wall_type[x] || wall_depth[x]>=depth)
   continue;
  int wh=20800/((int)wall_depth[x]+60);
  if(wh>150)wh=150;
  if(wh<3)wh=3;
  wall_column(p,x,1,79-wh/2,79+wh/2,(char)wall_type[x],wall_u[x],
              wall_depth[x],wall_tx[x],wall_ty[x],0);
 }
}
static void render_billboards(uint8_t *p,int cx,int cy,
                              int fx,int fy,int rx,int ry,unsigned tick){
 unsigned count=0;
 for(unsigned i=0;i<sizeof scene_anchors/sizeof scene_anchors[0];i++){
  const SceneAnchor *a=&scene_anchors[i];
  int x=(int)a->x,y=(int)a->y;
  char kind=(char)a->kind;
  int dx=x*256+128-cx,dy=y*256+128-cy;
  int forward=dx*fx+dy*fy,side=dx*rx+dy*ry;
  if(forward<85||forward>2200||abs_i(side)>forward*4/3)continue;
  int sx=120+side*160/forward;
  int height=24000/(forward+70);
  if(height>115)height=115;
  if(height<8)height=8;
  if(sx<0||sx>=240||forward>wall_depth[sx])continue;
  if(count>=MAX_VISIBLE_BILLBOARDS)continue;
  SceneBillboard item={(int16_t)sx,(int16_t)(80+height/2),
                       (int16_t)height,(int16_t)forward,(uint8_t)kind};
  unsigned j=count++;
  while(j>0u && visible[j-1u].depth<item.depth){
   visible[j]=visible[j-1u];j--;
  }
  visible[j]=item;
 }
 for(unsigned i=0;i<count;i++){
  const SceneBillboard *item=&visible[i];
  actor(p,item->sx,item->ground,item->height,(char)item->kind,tick);
  restore_occluding_wall(p,item->sx,item->height,item->depth);
 }
}
void hd_first_person_draw(uint8_t *p,const Story *s,unsigned tick){
#if defined(GBA_PROFILE_RENDER) && defined(__arm__)
 gba_profile_begin();
 gba_profile_wallshade_cycles=0u;
 uint32_t profile_last=0u,profile_now=0u;
#endif
 int fx,fy,rx,ry;
 dirs(s,&fx,&fy,&rx,&ry);
 int camx=(int)s->x*256+128,camy=(int)s->y*256+128;
 if(s->anim){
  camx+=(s->prev_x-(int)s->x)*s->anim*64;
  camy+=(s->prev_y-(int)s->y)*s->anim*64;
 }
 set_sky(p,tick,s->facing,camx,camy);
#if defined(GBA_PROFILE_RENDER) && defined(__arm__)
 profile_now=gba_profile_clock();
 gba_profile_sky_cycles=profile_now-profile_last;profile_last=profile_now;
#endif
 /* Walls use no destination reads. Paint them first so the floor can
    omit only pixels those same opaque columns already cover. */
 cast_walls(p,camx,camy,fx,fy,rx,ry);
#if defined(GBA_PROFILE_RENDER) && defined(__arm__)
 profile_now=gba_profile_clock();
 gba_profile_wall_cycles=profile_now-profile_last;profile_last=profile_now;
#endif
 floor_render(p,s,camx,camy,fx,fy,rx,ry,tick);
#if defined(GBA_PROFILE_RENDER) && defined(__arm__)
 profile_now=gba_profile_clock();
 gba_profile_floor_cycles=profile_now-profile_last;profile_last=profile_now;
#endif
 render_billboards(p,camx,camy,fx,fy,rx,ry,tick);
#if defined(GBA_PROFILE_RENDER) && defined(__arm__)
 profile_now=gba_profile_clock();
 gba_profile_billboard_cycles=profile_now-profile_last;profile_last=profile_now;
#endif
 /* Sparse rain, never an extra framebuffer or effect allocation. */
 for(unsigned i=0;i<24u;i++){
  unsigned v=texel(i+31u,i*7u+41u);
  int x=(int)((v+tick*2u)%252u)-8;
  int y=20+(int)(((v>>9)+tick*3u)%116u);
  hd_line(p,x,y,x-2,y+4+(int)(i&3u),
          (i&3u)==0u?HD_STONE_LIT:HD_GLASS_LIT);
 }
#if defined(GBA_PROFILE_RENDER) && defined(__arm__)
 profile_now=gba_profile_clock();
 gba_profile_weather_cycles=profile_now-profile_last;
 gba_profile_total_cycles=profile_now;
 R16(0x04000102)=0u;R16(0x04000106)=0u;
 ++gba_profile_frames; /* Publish the completed sample last. */
#endif
}

