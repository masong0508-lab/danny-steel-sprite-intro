#include "hd_canvas.h"
#include "menu_ink.h"

/* Danny's neighborhood, not an operating system. This composition renders
   in actual GBA pixels; the menu is a printed story-card foreground over an
   illustrated night bus stop. The original music keeps playing. */
static unsigned hash(unsigned a,unsigned b){
 unsigned x=a*40503u+b*19073u+673u;
 x^=x>>7;x*=199u;x^=x>>9;return x;
}
static void sky(uint8_t *p,unsigned t){
 for(int y=0;y<116;y++){
  unsigned ink=y<24?19:y<49?24:y<77?17:y<101?25:HD_ROAD_COOL;
  hd_rect(p,0,y,240,1,ink);
 }
 /* Quiet bands like brushed graphite and distant sodium-lit haze */
 for(int j=0;j<8;j++)
  hd_line(p,0,20+j*9,240,17+j*9,(j&1)?29:17);
 for(int i=0;i<7;i++){
  int x=135+i*19-((int)(t/220)%7);
  int y=49+(i*13%25);
  hd_rect(p,x,y,20,107-y,HD_SHADOW);
  hd_rect(p,x+2,y-2,15,2,HD_STONE_DARK);
  for(int yy=y+8;yy<115;yy+=10)for(int xx=x+4;xx<x+18;xx+=7)
    if((hash((unsigned)xx,(unsigned)yy)%5u)!=0)
      hd_rect(p,xx,yy,3,3,((t/90u+(unsigned)xx+(unsigned)yy)%7u)==0u?HD_LAMP:HD_GLASS);
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
 for(int y=117;y<160;y++)
   hd_rect(p,0,y,240,1,y<124?HD_STONE_DARK:y<144?HD_WET:HD_ROAD);
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
 /* Bus passing beyond the pedestrian stop; controlled, not a generic grid. */
 hd_rect(p,199,108,41,18,HD_RED_SHADE);
 hd_rect(p,200,109,40,12,HD_BRICK_LIT);
 hd_rect(p,203,110,17,7,HD_GLASS);
 hd_rect(p,222,110,14,7,HD_GLASS);
 hd_rect(p,204,111,15,2,HD_GLASS_LIT);
 hd_rect(p,221,120,13,2,HD_BRICK_DARK);
 hd_ellipse(p,208,126,4,4,HD_SHADOW);
 hd_ellipse(p,231,126,4,4,HD_SHADOW);
 hd_pixel(p,240-3,118,HD_LAMP);
 /* The street lamp creates the warm vertical anchor for the figure. */
 hd_rect(p,181,60,2,70,HD_INK);
 hd_rect(p,177,59,12,3,HD_COPPER);
 hd_rect(p,179,62,8,3,HD_LAMP);
 hd_line(p,181,127,181,139,HD_INK);
 /* Danny in the same red coat seen in the opening scene. */
 int bob=(t/16u&1u)?1:0;
 hd_ellipse(p,177,140,12,3,HD_SHADOW);
 hd_rect(p,171,116+bob,12,15,HD_RED_SHADE);
 hd_rect(p,173,116+bob,9,13,HD_RED);
 hd_rect(p,170,120+bob,3,10,HD_RED_SHADE);
 hd_rect(p,181,120+bob,3,10,HD_RED_SHADE);
 hd_rect(p,173,130+bob,4,9,HD_INK);
 hd_rect(p,179,130+bob,4,9,HD_INK);
 hd_rect(p,171,136+bob,8,3,HD_STONE_DARK);
 hd_rect(p,179,136+bob,6,3,HD_STONE_DARK);
 hd_rect(p,173,106+bob,9,10,HD_SKIN);
 hd_rect(p,173,106+bob,9,3,HD_HAIR);
 hd_rect(p,172,108+bob,2,6,HD_HAIR);
 hd_pixel(p,175,112+bob,HD_INK);
 hd_pixel(p,179,112+bob,HD_INK);
 hd_pixel(p,178,115+bob,HD_SKIN_LIT);
 /* Light touches the pavement rather than turning it neon. */
 for(int i=0;i<8;i++){
  int x=169+i*3;
  hd_line(p,x,142+(i&3),x,150+(i&1),i&1?HD_REFLECT:HD_ROAD_LIGHT);
 }
}
static void paper_panel(uint8_t *p){
 hd_rect(p,10,42,140,100,HD_SHADOW);
 hd_rect(p,12,44,136,96,HD_ROAD);
 hd_rect(p,14,46,132,92,HD_INK);
 hd_rect(p,15,47,130,90,HD_SHADOW);
 hd_rect(p,20,49,126,1,HD_COPPER);
 hd_line(p,20,133,145,133,HD_STONE_DARK);
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
 hd_text(p,190,26,"CHAPTER 01",HD_STONE_LIT,1,1);
 paper_panel(p);
 const char *const top[3]={"NEW GAME","CONTINUE","ERASE SAVE"};
 const char *const sub[3]={"A FRESH BEGINNING","PICK UP THE STORY","CLEAR A RECORD"};
 const char *const slots[3]={"SLOT ONE","SLOT TWO","RETURN"};
 const char *const help[3]={"BEGIN A NEW STORY","CONTINUE THIS STORY","RETURN TO OPTIONS"};
 for(unsigned i=0;i<3;i++){
  int y=54+(int)i*25,active=i==selection;
  if(active){
   hd_rect(p,20,y-3,121,23,HD_ROAD_COOL);
   hd_rect(p,20,y-3,2,23,HD_LAMP);
   hd_line(p,27,y+17,138,y+17,HD_COPPER);
  }else hd_line(p,26,y+19,136,y+19,HD_STONE_DARK);
  hd_text(p,27,y, page?slots[i]:top[i],active?HD_PAPER:HD_STONE_LIT,1,1);
  const char *caption=page?help[i]:sub[i];
  if(page && i<2){
   caption=i==0?(valid0?"SAVED STORY":"EMPTY RECORD"):
                 (valid1?"SAVED STORY":"EMPTY RECORD");
  }
  hd_text(p,27,y+10,caption,active?HD_LAMP:HD_STONE,1,0);
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
 hd_text(p,7,151,page?"LEFT/RIGHT SLOT  A OPEN  B BACK":
                         "LEFT/RIGHT CHOOSE  A OPEN  B TITLE",
         HD_PAPER,1,1);
}
