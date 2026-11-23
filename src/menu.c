#include "menu.h"
#include "hd_canvas.h"
#include "story_draw.h"
#include "menu_ink.h"
#include "menu_nav.h"
#include "music.h"
#include "play.h"
#define R_BG2PA R16(0x04000020)
#define R_BG2PB R16(0x04000022)
#define R_BG2PC R16(0x04000024)
#define R_BG2PD R16(0x04000026)
#define R_BG2X (*(volatile u32*)0x04000028)
#define R_BG2Y (*(volatile u32*)0x0400002C)
#define FRAMES_IN 25
#define TRANS_FRAMES 38
#define HALF_TRANS 19
#define TITLE_VOLUME 40
enum { M_IDLE, M_TRANS };
static unsigned page,pending,clock_frames,scene_frame;
static int state,level,sel,mode,prev_main,dir,transition,switched;
static MenuNav menu_nav;
static int was_b,erase_hold,menu_note,note_ticks,menu_launch;
static void ink_page(unsigned pg){
 hd_menu_draw((uint8_t*)hd_shared_frame,(unsigned)level,(unsigned)mode,(unsigned)sel,
              scene_frame,(unsigned)menu_note,(unsigned)erase_hold,
              (unsigned)play_slot_valid(0),(unsigned)play_slot_valid(1));
 volatile u32 *dest=(volatile u32*)(VRAM8+(pg?0xA000:0));
 for(unsigned i=0;i<HD_BYTES/4u;i++)dest[i]=hd_shared_frame[i];
}
static void refresh(void){
 page^=1u;ink_page(page);pending=1u;
}
static void start_transition(int direction){
 state=M_TRANS;dir=direction;transition=0;switched=0;erase_hold=0;
 menu_note=MENU_NOTE_NONE;note_ticks=0;
}
static void goto_sel(int selection){
 sel=selection;erase_hold=0;menu_note=MENU_NOTE_NONE;note_ticks=0;
}
static void enter_slot(int slot){
 if(mode==1 && !play_slot_valid(slot)){
  menu_note=MENU_NOTE_EMPTY;note_ticks=85;
 }else menu_launch=(mode==0?2:4)+slot;
}
void menu_enter(void){
 REG_DISPCNT=0;
 REG_BLDCNT=0x00FF;REG_BLDY=16;
 for(int i=0;i<32;i++)PAL16[i]=story_palette[i];
 for(int i=0;i<32;i++)PAL16[i+32]=hd_extra_palette[i];
 R_BG2PA=256;R_BG2PB=0;R_BG2PC=0;R_BG2PD=256;
 R_BG2X=0;R_BG2Y=0;REG_BGCNT(2)=0;
 state=M_IDLE;level=0;sel=0;mode=0;prev_main=0;transition=0;
 switched=0;scene_frame=0;clock_frames=0;page=0;pending=0;
 menu_nav_reset(&menu_nav);was_b=1;
 erase_hold=0;menu_note=MENU_NOTE_NONE;note_ticks=0;menu_launch=0;
 ink_page(0);
 REG_DISPCNT=4|DCNT_BG2;
}
int menu_update(void){
 if(pending){
  REG_DISPCNT=4|DCNT_BG2|(page?0x10:0);
  pending=0;
 }
 int redraw=0;
 int k=(int)~REG_KEYINPUT;
 int a=(k&KEY_A)!=0,b=(k&KEY_B)!=0;
 /* Both D-pad axes can navigate the three menu choices. Opposing
    directions cancel, even when entered on different axes. */
 int l=(k&(KEY_LEFT|KEY_UP))!=0,r=(k&(KEY_RIGHT|KEY_DOWN))!=0;
 int start=(k&KEY_START)!=0;
 MenuNavEvents nav=menu_nav_step(&menu_nav,a,start,l,r);
 int ea=nav.a_press,eb=b&&!was_b,el=nav.left_press,er=nav.right_press;
 int confirm=ea||nav.start_press;
 was_b=b;
 if(clock_frames<FRAMES_IN)
  REG_BLDY=(u16)(16-clock_frames*16/FRAMES_IN);
 else if(state==M_IDLE)REG_BLDY=0;
 if(clock_frames<90u)music_set_volume(100-(60*(int)clock_frames)/90);
 else music_set_volume(TITLE_VOLUME);
 ++clock_frames;++scene_frame;
 if(state==M_TRANS){
  int phase=transition<=HALF_TRANS?transition:TRANS_FRAMES-transition;
  if(phase<0)phase=0;
  REG_BLDY=(u16)(phase*15/HALF_TRANS);
  if(!switched&&transition>=HALF_TRANS){
   switched=1;level^=1;
   sel=dir>0?0:prev_main;
   redraw=1;
  }
  if(++transition>TRANS_FRAMES){
   state=M_IDLE;REG_BLDY=0;redraw=1;
  }
 }else{
  if(er&&sel<2){goto_sel(sel+1);redraw=1;}
  else if(el&&sel>0){goto_sel(sel-1);redraw=1;}
  else if(confirm){
   if(level==0){prev_main=mode=sel;start_transition(1);}
   else if(sel==2){start_transition(-1);}
   else if(mode!=2){enter_slot(sel);redraw=1;}
   else if(!play_slot_valid(sel)){
    menu_note=MENU_NOTE_EMPTY;note_ticks=85;redraw=1;
   }
  }else if(eb){
   if(level==1)start_transition(-1);
   else {music_set_volume(100);return 1;}
  }
  if(level==1&&mode==2&&sel<2){
   unsigned previous=(unsigned)erase_hold;
   erase_hold=(int)menu_erase_step(previous,ea,a&&play_slot_valid(sel));
   if((unsigned)erase_hold>=MENU_ERASE_HOLD){
    play_erase_slot(sel);erase_hold=0;
    menu_note=MENU_NOTE_ERASED;note_ticks=100;redraw=1;
   }else if(erase_hold!= (int)previous || (previous && !erase_hold))redraw=1;
  }
 }
 if(note_ticks&&!--note_ticks){menu_note=MENU_NOTE_NONE;redraw=1;}
 /* Animate the light/rain at 10 FPS; input/transition frames still redraw
    immediately. No LZ77 stream or hundreds of kilobytes of placeholder FMV. */
 if((scene_frame%6u)==0)redraw=1;
 if(redraw)refresh();
 if(menu_launch){int action=menu_launch;menu_launch=0;return action;}
 return 0;
}
