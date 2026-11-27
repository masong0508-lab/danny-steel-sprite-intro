#include "hd_canvas.h"
#include "hd_world_atlas.h"
#include "hd_facade_art.h"
#include "hd_scene_geometry.h"
#include "hd_project_props.h"
#include "hd_road_material_rom.inc"
#include "hd_road_columns.inc"
#include "hd_ray_trace.h"
#include "hd_wall_shades.h"
#include "hd_floor_runs.inc"
#include "hd_skyline_rom.h"
#include "hd_extra_floor_atlas.h"
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
static uint8_t wall_face[WALL_RAYS];
static uint8_t wall_shade_cache[WALL_RAYS][WALL_SHADE_ROWS]
 __attribute__((section(".ewram"),aligned(4)));
static int wall_cache_valid,wall_cam_x,wall_cam_y,wall_cam_fx,wall_cam_fy;
#if !defined(__arm__)
static int legacy_scenery,reference_restore,reference_floor,reference_ray,reference_shades;
void hd_debug_invalidate_wall_cache(void){wall_cache_valid=0;}
/* Shipped art remains a host-only baseline; do not weaken the existing
   3,456 golden-frame regression to bless unrelated rendering changes. */
void hd_debug_legacy_scenery(int enabled){legacy_scenery=!!enabled;wall_cache_valid=0;}
void hd_debug_reference_restore(int enabled){reference_restore=!!enabled;}
void hd_debug_reference_floor(int enabled){reference_floor=!!enabled;}
void hd_debug_reference_ray(int enabled){reference_ray=!!enabled;wall_cache_valid=0;}
void hd_debug_reference_shades(int enabled){reference_shades=!!enabled;wall_cache_valid=0;}
#else
#define legacy_scenery 0
#define reference_restore 0
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
#if !defined(__arm__)
static void set_sky_reference(uint8_t *p,unsigned tick,int facing,int cx,int cy){
 /* Floor and walls jointly cover EVERY column in rows 80..139.
    Never prefill their opaque region: 14,400 dead stores per frame.
    Retain the historical full clear on the host comparison path. */
 hd_rect(p,0,0,HD_W,24,19);
 hd_rect(p,0,24,HD_W,30,17);
 hd_rect(p,0,54,HD_W,20,HD_GLASS);
#if !defined(__arm__)
 if(reference_floor){
  hd_rect(p,0,74,HD_W,10,HD_GLASS_LIT);
  hd_rect(p,0,84,HD_W,18,HD_ROAD_COOL);
  hd_rect(p,0,102,HD_W,23,HD_ROAD_LIGHT);
  hd_rect(p,0,125,HD_W,35,HD_ROAD);
 }else
#endif
 {
  hd_rect(p,0,74,HD_W,6,HD_GLASS_LIT);
  hd_rect(p,0,140,HD_W,20,HD_ROAD);
 }
 /* Layered, place-bound horizon. Neither buildings nor cloud positions
    randomly change when the shot cuts to a dialogue panel. */
 int pan=(facing*71+(cx+cy)/96)%53;
 unsigned light_phase=(tick/180u)%5u;
 for(int i=-2;i<19;i++){
  int x=i*23-pan;
  if(x+25<0||x-2>=HD_W)continue;
  const uint8_t *material=hd_skyline_material[i+2];
  int ht=material[0];
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
  for(unsigned col=0;col<3u;col++){
   unsigned bits=material[col+1u],mask=bits&15u;
   unsigned ink=light_phase==(bits>>4)?HD_LAMP:HD_GLASS;
   int xx=x+3+(int)col*7;
   for(int h=7;mask;h+=9,mask>>=1)
    if(mask&1u)hd_rect(p,xx,79-ht+h,2,3,ink);
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
#endif
static void set_sky(uint8_t *p,unsigned tick,int facing,int cx,int cy){
#if !defined(__arm__)
 if(reference_floor){set_sky_reference(p,tick,facing,cx,cy);return;}
#endif
 hd_rect(p,0,0,HD_W,24,19);
 hd_rect(p,0,24,HD_W,8,17);
 hd_rect(p,0,140,HD_W,20,HD_ROAD);
 int pan=(facing*71+(cx+cy)/96)%53;
 hd_skyline_draw(p,pan,(tick/180u)%5u);
 /* Only the clouds are drawn procedurally. They remain above the
    skyline band, with the same original time and movement. */
 int drift=(int)(tick/42u)%40;
 for(int k=0;k<4;k++){
  int x=k*73+12-drift,y=17+(k&1)*8;
  hd_line(p,x,y,x+32,y,HD_STONE_DARK);
  hd_line(p,x+6,y-2,x+24,y-2,HD_STONE_DARK);
 }
}
/* 400 ROM bytes replace the repeating 17-column road dither modulo. */
static const uint8_t hd_floor_dither17[HD_W+HD_H] = {
 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0
};
/* One aligned cartridge-ROM word holds all 16 two-bit shades for
   one material Y row. Load once when the projected row changes, then
   extract a category directly from the scalar without unpacking a
   16-byte array or reading separate ROM bytes per floor pixel. */
typedef uint32_t RoadShadeWord __attribute__((may_alias));
static HD_CANVAS_CODE inline unsigned read_packed_road_row(
 unsigned road_tile,unsigned material_y){
 const uint8_t *data=hd_road_material_2bits+road_tile*64u+material_y*4u;
 return *(const RoadShadeWord *)(const void *)data;
}
static HD_CANVAS_CODE inline unsigned road_shade_color(
 int wx,int wy,int x,int y,unsigned category){
 int fx=wx&255,fy=wy&255;
 if((fx<5||fy<5)&&(wx+wy)%3==0)return HD_ROAD_COOL;
 if(category==0u)return HD_REFLECT;
 if(category==1u)return HD_ROAD_LIGHT;
 return hd_floor_dither17[x+y]?HD_ROAD_COOL:HD_ROAD;
}
#if !defined(__arm__)
unsigned hd_debug_packed_road_column_category(int tx,int ty,unsigned cx,unsigned cy){
 if((unsigned)tx>=24u||(unsigned)ty>=18u||cx>=16u||cy>=16u)return 255u;
 unsigned id=hd_road_tile_index[(unsigned)ty*24u+(unsigned)tx];
 if(id==255u)return 255u;
 return (hd_road_columns[id*16u+cx]>>(cy*2u))&3u;
}
unsigned hd_debug_packed_road_row_category(int tx,int ty,unsigned cx,unsigned cy){
 if((unsigned)tx>=24u||(unsigned)ty>=18u||cx>=16u||cy>=16u)return 255u;
 unsigned id=hd_road_tile_index[(unsigned)ty*24u+(unsigned)tx];
 if(id==255u)return 255u;
 unsigned word=read_packed_road_row(id,cy);
 return (word>>(cx*2u))&3u;
}
#endif
#if !defined(__arm__)
unsigned hd_debug_road_rom_category(int tx,int ty,unsigned cx,unsigned cy){
 if((unsigned)tx>=24u||(unsigned)ty>=18u||cx>=16u||cy>=16u)return 255u;
 unsigned id=hd_road_tile_index[ty*24+tx];
 if(id==255u)return 255u;
 unsigned byte=hd_road_material_2bits[id*64u+cy*4u+(cx>>2)];
 return (byte>>((cx&3u)*2u))&3u;
}
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
 unsigned n=texel((unsigned)(wx>>4),(unsigned)(wy>>4));
 if((fx<5||fy<5)&&(wx+wy)%3==0)return HD_ROAD_COOL;
 if((n&31u)==0u)return HD_REFLECT;
 if((n&7u)==1u)return HD_ROAD_LIGHT;
 return (x+y)%17==0?HD_ROAD_COOL:HD_ROAD;
}
/* Keep the measured pixel loops in ARM/IWRAM. noinline prevents the
   Thumb entry point from absorbing them back into cartridge code. */
#if !defined(__arm__)
static void floor_render_reference(uint8_t *p,const Story *s,int cx,int cy,
                         int fx,int fy,int rx,int ry,unsigned tick){
 (void)s;
 /* One projection calculation per scanline, and half-rate horizontal
    sampling in the far distance only. Near pavement uses each GBA pixel. */
 for(int y=80;y<140;y++){
  int dist=13500/(y-75); if(dist>2304)dist=2304;
  int side_unit=dist*3*256/480;
  int step=y<98?2:1;
  /* Existing wall bottom is 79 + floor(10400/(depth+60)).
     Thus y is covered exactly when depth <= 10400/(y-78)-60.
     At these floor rows the original 3..150 height clamp cannot alter
     that test. Reuse the depth strip instead of an extra coverage buffer. */
  int wall_limit=10400/(y-78)-60;
  int last_tx=-99,last_ty=-99;
  char tile='.';
  unsigned road_tile=0u,packed_row=0u;
  unsigned material_row=16u; /* Sentinel invalidates the packed word. */
  int begin=0;
  for(unsigned ray=0;ray<sizeof ray_end;ray++){
   int end=ray_end[ray];
   /* cast_walls gives every column in this original direction run
      the same hit and depth. Test coverage once, not per hidden pixel. */
   if(!wall_type[begin]||wall_depth[begin]>wall_limit){
    for(int x=begin&~(step-1);x<end;x+=step){
     int side=((x-120)*side_unit)>>8;
     int wx=cx+fx*dist+rx*side,wy=cy+fy*dist+ry*side;
     int tx=wx>>8,ty=wy>>8;
     if(tx!=last_tx||ty!=last_ty){
      last_tx=tx;last_ty=ty;tile=story_tile(tx,ty);
      if(tile=='.'){
       road_tile=hd_road_tile_index[(unsigned)ty*24u+(unsigned)tx];
       material_row=16u;
      }
     }
     unsigned ink;
     if(tile=='.'){
      unsigned cy=((unsigned)wy>>4)&15u;
      if(cy!=material_row){
       packed_row=read_packed_road_row(road_tile,cy);
       material_row=cy;
      }
      unsigned cx=((unsigned)wx>>4)&15u;
      unsigned category=(packed_row>>(cx*2u))&3u;
      ink=road_shade_color(wx,wy,x,y,category);
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
#endif
/* Fixed 60-row geometry removes three software divisions per row.
   The table is exactly the previous projection and wall-bottom rule. */
typedef struct {uint16_t distance,side;int16_t wall_limit;} FloorProjection;
static const FloorProjection floor_projection[60]={
 {2304,3686,5140},
 {2250,3600,3406},
 {1928,3084,2540},
 {1687,2699,2020},
 {1500,2400,1673},
 {1350,2160,1425},
 {1227,1963,1240},
 {1125,1800,1095},
 {1038,1660,980},
 {964,1542,885},
 {900,1440,806},
 {843,1348,740},
 {794,1270,682},
 {750,1200,633},
 {710,1136,590},
 {675,1080,551},
 {642,1027,517},
 {613,980,487},
 {586,937,460},
 {562,899,435},
 {540,864,412},
 {519,830,392},
 {500,800,373},
 {482,771,356},
 {465,744,340},
 {450,720,325},
 {435,696,311},
 {421,673,298},
 {409,654,286},
 {397,635,275},
 {385,616,265},
 {375,600,255},
 {364,582,245},
 {355,568,237},
 {346,553,228},
 {337,539,221},
 {329,526,213},
 {321,513,206},
 {313,500,200},
 {306,489,193},
 {300,480,187},
 {293,468,181},
 {287,459,176},
 {281,449,171},
 {275,440,166},
 {270,432,161},
 {264,422,156},
 {259,414,152},
 {254,406,148},
 {250,400,143},
 {245,392,140},
 {241,385,136},
 {236,377,132},
 {232,371,129},
 {228,364,125},
 {225,360,122},
 {221,353,119},
 {217,347,116},
 {214,342,113},
 {210,336,110},
};
/* Paint the same even sample on both far pixels, without crossing
   an occluded ray edge. Near samples retain every native column. */
static inline __attribute__((always_inline)) void floor_store(uint8_t *row,int x,
                                                 int begin,int end,int step,unsigned ink){
 if(x>=begin)row[x]=(uint8_t)ink;
 if(step==2&&x+1<end)row[x+1]=(uint8_t)ink;
}
static HD_CANVAS_CODE __attribute__((noinline)) void floor_render(uint8_t *p,const Story *s,int cx,int cy,
                         int fx,int fy,int rx,int ry,unsigned tick){
#if !defined(__arm__)
 if(reference_floor){floor_render_reference(p,s,cx,cy,fx,fy,rx,ry,tick);return;}
#endif
 (void)s;
 /* A cardinal camera has exactly ONE varying world coordinate per
    scanline. Keep it in Q8, advancing by addition. Negative directions
    add 255 before negation to preserve the original arithmetic shift. */
 int along_x=rx!=0,sign=along_x?rx:ry;
 for(int y=80;y<140;y++){
  const FloorProjection *pr=&floor_projection[y-80];
  int dist=pr->distance,unit=pr->side,limit=pr->wall_limit;
  int fixed=along_x?cy+fy*dist:cx+fx*dist;
  int origin=along_x?cx+fx*dist:cy+fy*dist;
  int ft=fixed>>8;
  unsigned sub=((unsigned)fixed>>4)&15u;
  int step=y<98?2:1,signed_unit=sign*unit,dq=signed_unit*step;
  /* The run boundaries repeat every 16 world units. A real camera is
     quarter-tile aligned; arbitrary external origins keep the exact
     existing single-sample path. */
  const uint8_t *runs=(origin&15)?0:hd_floor_run_next[sign<0][y-80];
  const uint8_t *dither=hd_floor_dither_skip[step-1];
  uint8_t *row=p+y*HD_W;
  int begin=0;
  unsigned ray=0;
  while(ray<WALL_RAYS){
   int end=ray_end[ray++];
   if(wall_type[begin]&&wall_depth[begin]<=limit){begin=end;continue;}
   /* Adjacent uncovered rays are a continuous floor span, not a new
      texture setup and duplicate odd half-sample for every ray. */
   while(ray<WALL_RAYS&&(!wall_type[end]||wall_depth[end]>limit))
    end=ray_end[ray++];
   int x=begin&~(step-1);
   int q=origin*256+sign*(x-120)*unit+(sign<0?255:0);
   while(x<end){
    int tv=q>>16,tx=along_x?tv:ft,ty=along_x?ft:tv;
    char tile=story_tile(tx,ty);
    unsigned landmark=tile=='.'?255u:hd_landmark_floor_id(tx,ty);
    if(tile=='.'||landmark!=255u){
     /* All walkable road-like materials use exact original categories,
        including the small patches beneath the letter, bus and NPCs. */
     unsigned word;
     if(tile=='.'){
      unsigned id=hd_road_tile_index[(unsigned)ty*24u+(unsigned)tx];
      word=along_x?read_packed_road_row(id,sub):hd_road_columns[id*16u+sub];
     }else word=hd_landmark_floor_word(landmark,along_x,sub);
     do{
      int v=q>>8;
      unsigned cell=((unsigned)v>>4)&15u;
      unsigned category=(word>>(cell*2u))&3u;
      /* Road texel color stays constant within a run, except the
         17-pixel dither and the narrow original tile seams. Batch long
         non-seam spans through the existing aligned IWRAM painter.
         The original floor sample parity is retained at odd ray edges. */
      if(runs&&cell&&(fixed&255)>=5){
       int next=runs[x];
       if(next>end)next=(end+step-1)&~(step-1);
       int left=x>begin?x:begin,right=next<end?next:end;
       if(right-left>=5){
        unsigned ink=category==0u?HD_REFLECT:category==1u?HD_ROAD_LIGHT:HD_ROAD;
        hd_rect(row,left,0,right-left,1,ink);
        if(category>=2u){
         int mark=x+dither[x+y];
         for(;mark<right;mark+=17*step)
          floor_store(row,mark,left,right,step,HD_ROAD_COOL);
        }
        q+=signed_unit*(next-x);x=next;
        continue;
       }
      }
      unsigned ink=road_shade_color(v,fixed,x,y,category);
      floor_store(row,x,begin,end,step,ink);
      x+=step;q+=dq;
     }while(x<end&&(q>>16)==tv);
    }else{
     unsigned soft=hd_soft_floor_id(tx,ty);
     if(soft!=255u){
      unsigned word=hd_soft_floor_word(soft,along_x,((unsigned)fixed>>5)&7u);
      /* Only the original four-pixel moving water glint depends on
         time. The underlying material hash is already in cartridge ROM. */
      do{
       int v=q>>8;
       unsigned light=(word>>(((unsigned)v>>5)&7u))&1u;
       unsigned ink;
       if(tile=='~'){
        int wy=along_x?fixed:v;
        ink=((wy+((int)tick*2))&63)<4?HD_GLASS_LIT:light?HD_REFLECT:HD_GLASS;
       }else ink=light?HD_LEAF:HD_LEAF_DARK;
       floor_store(row,x,begin,end,step,ink);
       x+=step;q+=dq;
      }while(x<end&&(q>>16)==tv);
     }else{
      do{
       int v=q>>8;
       int wx=along_x?v:fixed,wy=along_x?fixed:v;
       unsigned ink=floor_ink(tile,wx,wy,x,y,tick);
       floor_store(row,x,begin,end,step,ink);
       x+=step;q+=dq;
      }while(x<end&&(q>>16)==tv);
     }
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
#if !defined(__arm__)
static void wall_column_reference(uint8_t *p,int x,int width,int top,int bottom,char type,
                        int u,int forward,int tx,int ty,unsigned face,uint8_t *save_ink){
#if defined(GBA_PROFILE_RENDER) && defined(__arm__)
 uint32_t wallshade_start=gba_profile_clock();
#endif
 int height=bottom-top;
 int tree=type=='T';
 unsigned base=tree?HD_LEAF_DARK:type=='A'?HD_BRICK:HD_BRICK_DARK;
 if(forward>1200)base=tree?HD_LEAF_DARK:HD_STONE_DARK;
 const uint8_t *surface=(!tree&&!legacy_scenery)?hd_facade_at(tx,ty,type,face):0;
 unsigned column=(unsigned)u>>4;
 /* The authored side lighting stays visible in the distance, without
    turning the far windows and roof trim into bright noise. */
 static const uint8_t distant[32]={
  HD_ROAD,HD_ROAD,HD_ROAD_COOL,HD_WET,HD_MORTAR,HD_STONE,
  HD_STONE,HD_STONE_DARK,HD_BRICK_DARK,HD_BRICK_DARK,HD_BRICK,
  HD_STONE,HD_PLASTER,HD_SHADOW,HD_GOLD,HD_GLASS,HD_GLASS,
  HD_ROAD_COOL,HD_BARK,HD_LEAF_DARK,HD_LEAF_DARK,HD_LEAF,
  HD_BARK,HD_PLASTER,HD_INK,HD_STONE_LIT,HD_RED_SHADE,HD_RED_SHADE,
  HD_SKIN,HD_SKIN,HD_HAIR,HD_COPPER
 };
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
  }else if(surface){
   /* Native 16x40 atlas. The same exact facing and district appears in
      overhead and street views. No newly decoded texture or framebuffer. */
   c=surface[((unsigned)v*40u>>8)*16u+column];
   if(forward>1200)c=distant[c-32u];
  }
#if !defined(__arm__)
  else{
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
#endif
  if(!surface&&v>225&&(u&15)==1)c=HD_STONE;
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
#endif
static HD_CANVAS_CODE void wall_column(uint8_t *p,int x,int width,int top,int bottom,char type,
                        int u,int forward,int tx,int ty,unsigned face,uint8_t *save_ink){
#if !defined(__arm__)
 if(legacy_scenery||reference_shades){
  wall_column_reference(p,x,width,top,bottom,type,u,forward,tx,ty,face,save_ink);return;
 }
#endif
#if defined(GBA_PROFILE_RENDER) && defined(__arm__)
 uint32_t wallshade_start=gba_profile_clock();
#endif
 const uint8_t *strip=hd_wall_strip(type,tx,ty,face,(unsigned)u,forward>1200);
 const uint8_t *v=hd_wall_v_samples((unsigned)(bottom-top));
 unsigned head=(unsigned)((uintptr_t)(p+x)&1u);
 unsigned pairs=((unsigned)width-head)>>1u,tail=((unsigned)width-head)&1u;
 uint8_t *row=p+top*HD_W+x;
 int count=bottom-top;
 if(bottom>140)count=140-top;
 for(int i=0;i<count;i++,row+=HD_W){
  uint8_t ink=strip[v[i]];
  wall_store_span(row,head,pairs,tail,ink);
  if(save_ink)save_ink[i]=ink;
 }
 if(top>18&&top<140)hd_rect(p,x,top,width,1,type=='T'?HD_LEAF_LIT:HD_STONE_LIT);
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
/* 72 bytes replace the height division in cold rays, warm rays,
   and restored occluding columns. Depth 2272 shares the last bucket
   with 2300: both project to exactly four half-height pixels. */
static const uint8_t wall_half_height[72]={
 75,75,75,66,55,47,41,36,32,29,27,25,23,21,20,19,18,17,
 16,15,14,14,13,13,12,12,11,11,10,10,10,9,9,9,9,8,
 8,8,8,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,
 5,5,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,
};
static HD_CANVAS_CODE __attribute__((noinline)) void cast_walls(uint8_t *p,int cx,int cy,int fx,int fy,int rx,int ry){
 int warm=wall_cache_valid&&wall_cam_x==cx&&wall_cam_y==cy&&
          wall_cam_fx==fx&&wall_cam_fy==fy;
 if(warm){
  /* Cache contains BOTH geometry and shade data for this exact pose.
     Do not traverse the map or recompute UVs on a stationary frame. */
  int x=0;
  for(unsigned slot=0;slot<WALL_RAYS;slot++){
   int end=ray_end[slot];
   if(wall_type[x]){
    int half=wall_half_height[wall_depth[x]>>5];
    wall_cached_column(p,x,end-x,79-half,79+half,(char)wall_type[x],wall_shade_cache[slot]);
   }
   x=end;
  }
  return;
 }
 unsigned slot=0;
 for(int x=0;x<HD_W;){
  /* Exact, immutable original ray directions and column boundaries. */
  int side=(int)slot-22;
  int end=ray_end[slot];
  int dx=fx*32+rx*side,dy=fy*32+ry*side;
  HdRayHit sample;
#if !defined(__arm__)
  if(reference_ray)hd_ray_reference(cx,cy,dx,dy,&sample);
  else
#endif
  hd_ray_trace(cx,cy,dx,dy,&sample);
  int wx=sample.wx,wy=sample.wy,tx=sample.tx,ty=sample.ty;
  int hit_from_x=sample.from_x,hit_from_y=sample.from_y;
  int hit_dist=sample.depth;
  char hit=sample.type;
  for(int col=x;col<end;col++){
   wall_depth[col]=(uint16_t)hit_dist;
   wall_type[col]=(uint8_t)hit;
  }
  if(!hit){x=end;slot++;continue;}
  int half=wall_half_height[(unsigned)hit_dist>>5];
  int top=79-half,bottom=79+half;
  unsigned face=abs_i(dx)>abs_i(dy)?1u:0u;
  int u=face?wy&255:wx&255;
  if(!legacy_scenery){
   HdWallSample sample=hd_wall_sample(wx,wy,dx,dy,tx,ty,hit_from_x,hit_from_y);
   face=sample.face;u=(int)sample.u;
  }
  if(slot<WALL_RAYS)wall_face[slot]=(uint8_t)face;
  for(int col=x;col<end;col++){
   wall_u[col]=(uint8_t)u;
   wall_tx[col]=(uint8_t)tx;
   wall_ty[col]=(uint8_t)ty;
  }
  wall_column(p,x,end-x,top,bottom,hit,u,hit_dist,tx,ty,face,
              wall_shade_cache[slot]);
  x=end;slot++;
 }
 wall_cache_valid=slot==WALL_RAYS;
 wall_cam_x=cx;wall_cam_y=cy;wall_cam_fx=fx;wall_cam_fy=fy;
}
static void actor(uint8_t *p,int sx,int ground,int h,char kind,unsigned tick,const Story *s){
 if(h<8)return;
 if(!legacy_scenery&&(kind=='R'||kind=='M')){
  int w=h*4/11;if(w<5)w=5;
  if(kind=='R')hd_project_easel(p,sx,ground,w,h*3/4,s->art_steps,
                               !!(s->project_style&STORY_STYLE_ART_WARM));
  else hd_project_desk(p,sx,ground,w,h*3/4,s->song_steps,
                       !!(s->project_style&STORY_STYLE_SONG_SOFT));
  return;
 }
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
 int radius=hd_billboard_radius(h);
 int left=sx-radius,right=sx+radius+1;
 if(left<0)left=0;
 if(right>HD_W)right=HD_W;
 /* Restore the SAME shaded wall already drawn for this camera, clipped
    to the billboard's extent. Each ray's equal columns are one span;
    never redo texture math just because a person crossed a facade. */
 for(int x=left;x<right;){
  unsigned slot=(unsigned)(((x-120)*3)/16+22);
  int end=wall_cache_valid&&!reference_restore&&slot<WALL_RAYS?ray_end[slot]:x+1;
  if(end>right)end=right;
  if(wall_type[x]&&wall_depth[x]<depth){
   int half=wall_half_height[wall_depth[x]>>5];
   int top=79-half,bottom=79+half;
   if(wall_cache_valid&&!reference_restore&&slot<WALL_RAYS)
    wall_cached_column(p,x,end-x,top,bottom,(char)wall_type[x],wall_shade_cache[slot]);
   else
    wall_column(p,x,1,top,bottom,(char)wall_type[x],wall_u[x],
                wall_depth[x],legacy_scenery?wall_tx[x]:(int)(int8_t)wall_tx[x],
                legacy_scenery?wall_ty[x]:(int)(int8_t)wall_ty[x],
                slot<WALL_RAYS?wall_face[slot]:(unsigned)(wall_cam_fx!=0),0);
  }
  x=end;
 }
}
static void render_billboards(uint8_t *p,int cx,int cy,
                              int fx,int fy,int rx,int ry,unsigned tick,const Story *s){
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
  if(legacy_scenery){
   if(sx<0||sx>=240||forward>wall_depth[sx])continue;
  }else if(!hd_billboard_visible(sx,height,forward,wall_depth,wall_type,HD_W))continue;
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
  if(item->kind=='F' && hd_mara_absent(s)){
   int x=item->sx,y=item->ground,h=item->height;
   int w=h*4/11;if(w<5)w=5;
   int top=y-h/3;
   hd_ellipse(p,x,y+1,w/2+2,2,HD_SHADOW);
   hd_rect(p,x-w/2,top,w,h/7+2,HD_BARK);
   hd_rect(p,x-w/2,top,w,1,HD_COPPER);
   hd_rect(p,x-w/2+1,top+2,2,y-top-2,HD_BARK);
   hd_rect(p,x+w/2-2,top+2,2,y-top-2,HD_BARK);
   hd_rect(p,x,top-h/10,w/3+1,h/10+1,HD_INK);
   hd_pixel(p,x+1,top-h/10,HD_GOLD);
  }else actor(p,item->sx,item->ground,item->height,(char)item->kind,tick,s);
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
 HdSceneCamera camera=hd_scene_camera(s);
 int camx=camera.x,camy=camera.y;
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
 render_billboards(p,camx,camy,fx,fy,rx,ry,tick,s);
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

