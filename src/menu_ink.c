#include "menu_ink.h"

/* The menu's pre-existing video palette is tightly quantized. Its last two
   almost-identical charcoal entries are reserved for paper / ink captions;
   encoded footage using those entries is remapped to neighbouring gray 253.
   Keep this file free of hardware references for deterministic host tests. */
enum { INK_SHADOW=254, INK_PAPER=255, INK_RULE=15 };
static const uint8_t font[36][5]={
 {2,5,7,5,5},{6,5,6,5,6},{3,4,4,4,3},{6,5,5,5,6},
 {7,4,6,4,7},{7,4,6,4,4},{3,4,5,5,3},{5,5,7,5,5},
 {7,2,2,2,7},{1,1,1,5,2},{5,5,6,5,5},{4,4,4,4,7},
 {5,7,7,5,5},{5,7,7,7,5},{2,5,5,5,2},{6,5,6,4,4},
 {2,5,5,3,1},{6,5,6,5,5},{3,4,2,1,6},{7,2,2,2,2},
 {5,5,5,5,7},{5,5,5,5,2},{5,5,7,7,5},{5,5,2,5,5},
 {5,5,2,2,2},{7,1,2,4,7},
 {7,5,5,5,7},{2,6,2,2,7},{6,1,2,4,7},{6,1,2,1,6},
 {5,5,7,1,1},{7,4,6,1,6},{3,4,7,5,7},{7,1,1,2,2},
 {7,5,7,5,7},{7,5,7,1,6}
};
static void pixel(uint8_t *p,int x,int y,unsigned c){
 if((unsigned)x<MENU_INK_W && (unsigned)y<MENU_INK_H)
  p[y*MENU_INK_W+x]=(uint8_t)c;
}
static void letter(uint8_t *p,int x,int y,char c,unsigned ink){
 unsigned n;
 if(c>='a'&&c<='z')c=(char)(c-'a'+'A');
 if(c>='A'&&c<='Z')n=(unsigned)(c-'A');
 else if(c>='0'&&c<='9')n=(unsigned)(c-'0'+26);
 else if(c=='.'){pixel(p,x+1,y+4,ink);return;}
 else if(c=='-'){pixel(p,x,y+2,ink);pixel(p,x+1,y+2,ink);pixel(p,x+2,y+2,ink);return;}
 else if(c=='/'){for(int i=0;i<5;i++)pixel(p,x+2-i/2,y+i,ink);return;}
 else if(c==':'){pixel(p,x+1,y+1,ink);pixel(p,x+1,y+3,ink);return;}
 else return;
 for(int j=0;j<5;j++)for(int i=0;i<3;i++)
  if(font[n][j]&(4u>>i))pixel(p,x+i,y+j,ink);
}
static void lettering(uint8_t *p,int x,int y,const char *s,unsigned ink){
 for(int i=0;s[i]&&i<28;i++)letter(p,x+4*i,y,s[i],ink);
}
void menu_ink_draw(uint8_t *p,unsigned level,unsigned mode,
                   unsigned slot,unsigned valid,unsigned held,
                   unsigned feedback){
 /* No full-screen postprocessing; this is one cheap scan of the reused page.
    Ink/paper indices 254/255 are never left in imported footage. */
 for(unsigned i=0;i<MENU_INK_W*MENU_INK_H;i++)
  if(p[i]>=254u)p[i]=253u;
 for(unsigned y=0;y<11u;y++)for(unsigned x=0;x<MENU_INK_W;x++)
  p[y*MENU_INK_W+x]=INK_SHADOW;
 for(unsigned y=68u;y<MENU_INK_H;y++)for(unsigned x=0;x<MENU_INK_W;x++)
  p[y*MENU_INK_W+x]=INK_SHADOW;
 for(int x=4;x<116;x++){
  pixel(p,x,10,INK_RULE);pixel(p,x,68,INK_RULE);
 }
 lettering(p,4,3,"THE LAST DAYS",INK_PAPER);
 if(!level)lettering(p,72,3,"OLD QUARTER",INK_PAPER);
 else lettering(p,76,3,mode==0?"NEW STORY":mode==1?"CONTINUE":"ERASE SAVE",INK_PAPER);
 const char *hint;
 if(!level)hint="LEFT RIGHT  A ENTER  B TITLE";
 else if(feedback==MENU_NOTE_ERASED)hint="SAVE ERASED  B BACK";
 else if(feedback==MENU_NOTE_EMPTY)hint="NO SAVED STORY HERE";
 else if(slot>=2u)hint="B BACK";
 else if(mode==0)hint="A BEGIN  B BACK";
 else if(!valid)hint="EMPTY SLOT  B BACK";
 else if(mode==1)hint="A RESUME  B BACK";
 else hint="HOLD A TO ERASE  B BACK";
 lettering(p,4,73,hint,INK_PAPER);
 if(level&&mode==2&&slot<2u&&valid&&held){
  unsigned width=held*108u/MENU_ERASE_HOLD;
  if(width>108u)width=108u;
  for(unsigned x=5u;x<5u+width;x++)pixel(p,(int)x,69,INK_PAPER);
 }
}
