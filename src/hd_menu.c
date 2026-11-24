#include "hd_canvas.h"
#include "menu_ink.h"
#include "hd_world_atlas.h"

/* Danny's neighborhood, not an operating system. This composition renders
   in actual GBA pixels; the menu is a printed story-card foreground over an
   illustrated night bus stop. The original music keeps playing. */
static unsigned hash(unsigned a,unsigned b){
 unsigned x=a*40503u+b*19073u+673u;
 x^=x>>7;x*=199u;x^=x>>9;return x;
}
static void sky(uint8_t *p,unsigned t){
 /* Five contiguous fills replace 116 individual scanline calls.
    The record-sleeve illustration stays quietly behind the title. */
 hd_rect(p,0,0,240,24,19);
 hd_rect(p,0,24,240,25,24);
 hd_rect(p,0,49,240,28,17);
 hd_rect(p,0,77,240,24,25);
 hd_rect(p,0,101,240,15,HD_ROAD_COOL);
 for(int i=0;i<5;i++){
  int x=124+i*27,y=47+(i*17)%23;
  hd_rect(p,x,y,26,109-y,HD_ROAD_COOL);
  hd_rect(p,x+4,y-2,14,2,HD_ROAD_COOL);
 }
 for(int i=0;i<6;i++){
  int x=140+i*21,y=58+(i*13%21);
  hd_rect(p,x,y,20,107-y,HD_SHADOW);
  hd_rect(p,x+2,y-2,15,2,HD_STONE_DARK);
  hd_rect(p,x+1,y+2,2,104-y,HD_STONE_DARK);
  for(int yy=y+8;yy<106;yy+=12)for(int xx=x+4;xx<x+18;xx+=7){
   unsigned v=hash((unsigned)xx,(unsigned)yy)%5u;
   if(v!=0)hd_rect(p,xx,yy,3,4,v==1?HD_COPPER:HD_GLASS);
   if(v==1)hd_pixel(p,xx+1,yy+1,HD_LAMP);
  }
 }
 /* One coherent lit apartment frontage and a street awning. */
 hd_rect(p,170,71,56,46,HD_BRICK_DARK);
 for(int y=74;y<114;y+=8){
  hd_line(p,171,y,225,y,HD_BRICK);
  for(int x=175+(y&7);x<223;x+=13)hd_line(p,x,y,x,y+6,HD_BRICK);
 }
 for(int x=177;x<224;x+=14){
  hd_rect(p,x,80,10,18,HD_SHADOW);
  hd_rect(p,x+1,81,8,15,HD_GLASS);
  hd_line(p,x+5,82,x+5,95,HD_COPPER);
  hd_line(p,x+2,89,x+8,89,HD_COPPER);
  if(x==191||x==219)hd_rect(p,x+2,83,3,4,HD_LAMP);
 }
 hd_rect(p,162,113,78,6,HD_BRICK);
 hd_line(p,162,112,240,112,HD_STONE_LIT);
 /* Wet pavement, curb and irregular granite: no giant flat chessboard. */
 hd_rect(p,0,116,240,8,HD_STONE_DARK);
 hd_rect(p,0,124,240,20,HD_WET);
 hd_rect(p,0,144,240,16,HD_ROAD);
 hd_line(p,0,123,239,123,HD_STONE);
 hd_line(p,0,125,239,125,HD_ROAD_COOL);
 for(int y=130;y<160;y+=9)for(int x=(y&7)-10;x<242;x+=27){
  unsigned m=hash((unsigned)(x+20),(unsigned)y);
  hd_line(p,x,y,x+9+(int)(m&7u),y,HD_ROAD_COOL);
  hd_pixel(p,x+3,y+2,HD_REFLECT);
 }
 for(int k=0;k<10;k++){
  int x=138+k*11;
  hd_line(p,x,137,x-4,156,(k&1)?HD_ROAD_COOL:HD_STONE_DARK);
 }
 /* The last bus wears the same cream roof and red body as the
    cinematic vehicle. It sits behind the foreground street furniture. */
 hd_ellipse(p,216,127,24,3,HD_SHADOW);
 hd_rect(p,195,107,45,20,HD_RED_SHADE);
 hd_rect(p,198,105,42,4,HD_PLASTER);
 hd_rect(p,198,109,42,13,HD_RED);
 hd_rect(p,201,110,37,7,HD_GLASS);
 hd_rect(p,201,110,37,2,HD_ROAD_COOL);
 hd_line(p,202,116,237,116,HD_COPPER);
 hd_rect(p,214,110,1,8,HD_INK);
 hd_rect(p,228,110,1,8,HD_INK);
 hd_rect(p,198,119,42,2,HD_PLASTER_LIT);
 hd_rect(p,198,123,41,2,HD_BRICK_DARK);
 hd_ellipse(p,204,126,4,4,HD_INK);
 hd_ellipse(p,231,126,4,4,HD_INK);
 hd_pixel(p,204,126,HD_STONE);
 hd_pixel(p,231,126,HD_STONE);
 hd_pixel(p,237,120,HD_LAMP);
 /* Lamp, timetable and a broken pool of reflected light establish a
    believable place to wait. The menu uses the actual gameplay sprite. */
 hd_rect(p,181,60,2,69,HD_INK);
 hd_rect(p,177,59,12,3,HD_BARK);
 hd_rect(p,179,62,8,3,HD_LAMP);
 hd_line(p,182,65,182,128,HD_STONE_DARK);
 hd_rect(p,176,75,12,17,HD_SHADOW);
 hd_rect(p,177,76,10,4,HD_COPPER);
 hd_text(p,178,76,"1",HD_PAPER,1,0);
 hd_rect(p,177,82,10,9,HD_PLASTER);
 hd_line(p,179,84,184,84,HD_STONE);
 hd_line(p,179,87,185,87,HD_STONE);
 hd_ellipse(p,181,137,18,6,HD_WET);
 hd_line(p,170,134,188,134,HD_ROAD_LIGHT);
 hd_line(p,178,136,190,136,HD_COPPER);
 hd_line(p,173,140,189,140,HD_REFLECT);
 if((t/23u)&1u)hd_line(p,181,136,185,136,HD_GOLD);
 hd_world_person_art(p,164,139,0u,1u,0u);
 for(unsigned i=0;i<6u;i++){
  unsigned seed=hash(i,7u);
  int x=152+(int)((seed+t*2u)%86u),y=43+(int)((seed/19u+t*3u)%98u);
  hd_line(p,x,y,x-1,y+3,HD_ROAD_COOL);
 }
}
static void paper_panel(uint8_t *p){
 /* A quiet, slightly offset sleeve of dark paper, not a framed GUI. */
 hd_rect(p,10,42,140,101,HD_SHADOW);
 hd_rect(p,13,44,133,96,HD_BRICK_DARK);
 hd_rect(p,14,45,131,93,HD_INK);
 hd_rect(p,16,47,126,89,HD_SHADOW);
 hd_line(p,20,49,139,49,HD_COPPER);
 hd_line(p,20,133,139,133,HD_STONE_DARK);
}
void hd_menu_draw(uint8_t *p,unsigned page,unsigned mode,unsigned selection,
                  unsigned frame,unsigned feedback,unsigned held,
                  unsigned valid0,unsigned valid1){
 sky(p,frame);
 /* A deliberate editorial title, in native 240x160 pixels. */
 hd_text(p,15,7,"THE LAST DAYS OF",HD_STONE_LIT,1,2);
 hd_text(p,15,19,"DANNY STEEL",HD_PAPER,2,1);
 hd_line(p,15,38,230,38,HD_COPPER);
 hd_text(p,159,9,"OLD QUARTER",HD_LAMP,1,1);
 hd_text(p,179,26,"SIDE A / 01",HD_STONE_LIT,1,0);
 paper_panel(p);
 const char *const top[3]={"NEW GAME","CONTINUE","ERASE SAVE"};
 const char *const sub[3]={"A FRESH BEGINNING","PICK UP THE STORY","CLEAR A RECORD"};
 const char *const slots[3]={"SLOT ONE","SLOT TWO","RETURN"};
 const char *const help[3]={"BEGIN A NEW STORY","CONTINUE THIS STORY","RETURN TO OPTIONS"};
 for(unsigned i=0;i<3;i++){
  int y=54+(int)i*25,active=i==selection;
  if(active){
   hd_rect(p,20,y-3,121,22,HD_BRICK_DARK);
   hd_rect(p,20,y-3,2,22,HD_LAMP);
   hd_line(p,27,y+17,138,y+17,HD_COPPER);
  }else hd_line(p,26,y+19,136,y+19,HD_STONE_DARK);
  hd_text(p,27,y, page?slots[i]:top[i],active?HD_PAPER:HD_STONE_LIT,1,1);
  const char *caption=page?help[i]:sub[i];
  if(page && i<2){
   caption=i==0?(valid0?"SAVED STORY":"EMPTY RECORD"):
                 (valid1?"SAVED STORY":"EMPTY RECORD");
  }
  hd_text(p,27,y+10,caption,active?HD_STONE_LIT:HD_STONE,1,0);
 }
 if(page && mode==2 && selection<2){
  hd_rect(p,23,132,116,2,HD_STONE_DARK);
  unsigned n=held*116u/MENU_ERASE_HOLD;if(n>116u)n=116u;
  hd_rect(p,23,132,(int)n,2,HD_LAMP);
 }
 if(feedback){
  hd_rect(p,11,139,136,9,HD_INK);
  hd_text(p,16,140,feedback==MENU_NOTE_ERASED?"SLOT ERASED":
                                   "NO SAVED STORY",HD_LAMP,1,1);
 }
 /* Navigation belongs within the image, not in a shouting footer bar. */
 hd_rect(p,0,149,240,11,HD_SHADOW);
 hd_line(p,0,148,240,148,HD_COPPER);
 hd_text(p,7,151,page?"D-PAD SLOT   A/START OPEN   B BACK":
                         "D-PAD CHOOSE  A/START OPEN  B TITLE",
         HD_PAPER,1,0);
}
