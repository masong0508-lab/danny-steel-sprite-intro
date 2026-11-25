#include "hd_canvas.h"
#include "hd_world_atlas.h"
#include "interior_tasks.h"
#include <string.h>
/* 28 independently painted, actual navigable room backgrounds and 14
   workbench close-ups. Every byte is displayed in the native painter. */
enum {ROOM_BYTES=240*140,STRIP_BYTES=240*32,BUILDING_BYTES=2*ROOM_BYTES+STRIP_BYTES};
extern const uint8_t hd_interior_art[14*BUILDING_BYTES];
__asm__(
 ".pushsection .rodata.interiors,\"a\"\n"
 ".balign 4\n"
 ".global hd_interior_art\n"
 "hd_interior_art:\n"
 ".incbin \"assets/interiors/00.bin\"\n"
 ".incbin \"assets/interiors/01.bin\"\n"
 ".incbin \"assets/interiors/02.bin\"\n"
 ".incbin \"assets/interiors/03.bin\"\n"
 ".incbin \"assets/interiors/04.bin\"\n"
 ".incbin \"assets/interiors/05.bin\"\n"
 ".incbin \"assets/interiors/06.bin\"\n"
 ".incbin \"assets/interiors/07.bin\"\n"
 ".incbin \"assets/interiors/08.bin\"\n"
 ".incbin \"assets/interiors/09.bin\"\n"
 ".incbin \"assets/interiors/10.bin\"\n"
 ".incbin \"assets/interiors/11.bin\"\n"
 ".incbin \"assets/interiors/12.bin\"\n"
 ".incbin \"assets/interiors/13.bin\"\n"
 ".popsection\n");
static void card(uint8_t *p,int x,int y,int w,int h){
 hd_rect(p,x+2,y+2,w,h,HD_SHADOW);
 hd_rect(p,x,y,w,h,HD_INK);
 hd_rect(p,x,y,w,1,HD_COPPER);hd_rect(p,x,y,2,h,HD_COPPER);
}
static void label(uint8_t *p,int x,int y,const char *s,unsigned ink){
 hd_text(p,x,y,s,ink,1,1);
}
static void progress(uint8_t *p,int x,int y,unsigned done){
 for(unsigned i=0;i<3u;i++){
  hd_rect(p,x+(int)i*12,y,9,7,i<done?HD_GOLD:HD_STONE_DARK);
  if(i==done)hd_rect(p,x+(int)i*12,y+8,9,1,HD_LAMP);
 }
}
static void workstation(uint8_t *p,const Story *s,unsigned id){
 const InteriorTask *t=&interior_tasks[id];
 unsigned step=interior_step(s),mode=interior_mode(id,step);
 memcpy(p,hd_interior_art+id*BUILDING_BYTES+2*ROOM_BYTES,STRIP_BYTES);
 hd_rect(p,0,32,240,128,HD_ROAD);
 hd_rect(p,0,32,240,2,HD_COPPER);
 label(p,12,39,story_neighborhood_name(id),HD_PAPER);
 label(p,90,39,t->steps[step],HD_LAMP);
 progress(p,190,39,s->activity_choice);
 card(p,12,54,216,86);
 if(mode==IN_DIAL){
  unsigned target=interior_dial_target(id,step);
  char prompt[]="SET THE DIAL TO 0";
  prompt[16]=(char)('0'+target);
  label(p,22,65,prompt,HD_LAMP);
  for(unsigned i=0;i<8u;i++){
   int x=22+(int)i*24;
   unsigned active=s->choice==i;
   hd_rect(p,x,88,20,23,active?HD_COPPER:HD_STONE_DARK);
   if(i==target)hd_rect(p,x,87,20,2,HD_GOLD);
   char digit[]={'0'+(char)i,0};
   hd_text(p,x+7,96,digit,active?HD_PAPER:HD_STONE_LIT,1,0);
  }
  label(p,22,121,"LEFT/RIGHT THEN A",HD_PAPER);
 }else if(mode==IN_TIMING){
  unsigned target=interior_timing_target(id,step),pos=interior_timing_position(s);
  label(p,22,65,"STOP THE NEEDLE IN GOLD",HD_LAMP);
  for(unsigned i=0;i<16u;i++){
   int x=24+(int)i*12;
   unsigned gold=i+1u>=target&&i<=target+1u;
   hd_rect(p,x,90,10,18,gold?HD_COPPER:HD_STONE_DARK);
   if(i==pos){hd_rect(p,x+3,85,4,28,HD_PAPER);hd_rect(p,x+1,85,8,2,HD_LAMP);}
  }
  label(p,22,121,"A TO STOP / NO TIME LIMIT",HD_PAPER);
 }else{
  static const char *const dirs[]={"RIGHT","DOWN","LEFT","UP"};
  label(p,22,65,"FOLLOW THE THREE DIRECTIONS",HD_LAMP);
  for(unsigned i=0;i<3u;i++){
   int x=22+(int)i*65;
   unsigned done=i<s->choice;
   hd_rect(p,x,88,59,23,done?HD_COPPER:HD_STONE_DARK);
   if(i==s->choice)hd_rect(p,x,88,59,1,HD_LAMP);
   label(p,x+5,96,dirs[interior_pattern(id,step,i)],done?HD_PAPER:HD_STONE_LIT);
  }
  label(p,22,121,"PRESS THE D-PAD IN ORDER",HD_PAPER);
 }
 if(s->page==IN_RETRY){
  card(p,18,111,204,28);
  label(p,25,115,"TRY AGAIN - NO LOST PROGRESS",HD_LAMP);
  label(p,25,125,"A RETRY   B CANCEL",HD_PAPER);
 }
 hd_rect(p,0,142,240,18,HD_INK);
 label(p,12,147,"B CANCEL / RETURN TO ROOM",HD_STONE_LIT);
}
void hd_interior_draw(uint8_t *p,const Story *s,unsigned frame){
 (void)frame;
 int found=story_interior_id(s);
 if(found<0||s->interior_room>=2u||s->interior_pos>=60u){
  hd_rect(p,0,0,240,160,HD_INK);label(p,12,70,"RETURN TO THE STREET",HD_PAPER);return;
 }
 unsigned id=(unsigned)found,room=s->interior_room;
 const InteriorTask *t=&interior_tasks[id];
 if(s->page==IN_GAME||s->page==IN_RETRY){workstation(p,s,id);return;}
 memcpy(p,hd_interior_art+id*BUILDING_BYTES+room*ROOM_BYTES,ROOM_BYTES);
 /* Slot labels occupy the little dark badge built into every equipment
    cel. Mark progress on the actual fixture, never on an empty tile. */
 for(int y=0;y<6;y++)for(int x=0;x<10;x++){
  char c=story_interior_tile(id,room,x,y);
  if(c>='1'&&c<='3'){
   unsigned n=(unsigned)(c-'1');
   int xx=10+22*x,yy=22+19*y;
   char num[]={c,0};unsigned active=n==s->activity_choice;
   if(active){
    hd_rect(p,xx+1,yy+1,20,1,HD_LAMP);
    hd_rect(p,xx+1,yy+17,20,1,HD_LAMP);
   }
   hd_text(p,xx+1,yy+1,num,n<s->activity_choice?HD_GOLD:active?HD_PAPER:HD_STONE_LIT,1,0);
  }
 }
 int x=(int)(s->interior_pos%10u),y=(int)(s->interior_pos/10u);
 static const int8_t dx[4]={1,0,-1,0},dy[4]={0,1,0,-1};
 unsigned facing=s->facing&3u,anim=s->anim>4u?4u:s->anim;
 int px=21+x*22-dx[facing]*(int)anim*22/4;
 int foot=39+y*19-dy[facing]*(int)anim*19/4;
 unsigned gait=anim?2u+((4u-anim+((unsigned)(x+y)&1u)*2u)&3u):0u;
 hd_ellipse(p,px,foot-1,7,2,HD_SHADOW);
 hd_world_person_projected(p,px,foot,20,0u,facing,gait);
 hd_rect(p,0,140,240,20,HD_INK);hd_rect(p,0,140,240,1,HD_COPPER);
 label(p,8,143,story_neighborhood_name(id),HD_PAPER);
 label(p,88,143,room?"WORKROOM":"ENTRY",HD_STONE_LIT);
 progress(p,190,143,s->activity_choice);
 char action=story_interior_action(s);
 const char *hint=action=='D'?"A EXIT":action=='>'?"A WORKROOM":
   action=='<'?"A ENTRY":action=='C'?"A TALK":"A LOOK";
 label(p,8,152,(action>='1'&&action<='3')?"A WORK":hint,HD_LAMP);
 if(action>='1'&&action<='3')label(p,53,152,t->steps[(unsigned)(action-'1')],HD_PAPER);
 label(p,137,152,"R HELP  B EXIT",HD_STONE_LIT);
 if(s->page==IN_WALK)return;
 card(p,10,69,220,69);
 if(s->page==IN_BRIEF){
  label(p,18,75,t->speaker,HD_LAMP);
  label(p,18,87,t->intro,HD_PAPER);
  label(p,18,98,t->order,HD_PAPER);
  label(p,18,109,"1-2-3 IN WORKROOM",HD_GOLD);
 }else if(s->page==IN_WRONG){
  label(p,18,75,"ONE STEP AT A TIME",HD_LAMP);
  label(p,18,88,"NEXT STATION:",HD_PAPER);
  label(p,110,88,t->steps[interior_step(s)],HD_GOLD);
  label(p,18,100,t->order,HD_PAPER);
 }else if(s->page==IN_RESULT&&s->activity_choice>=3u){
  label(p,18,75,t->speaker,HD_LAMP);
  label(p,18,87,t->finish,HD_PAPER);
  unsigned bit=1u<<id;
  label(p,18,98,(s->neighborhood_seen&bit)?
        ((s->neighborhood_right&bit)?t->right:t->left):
        "You made something useful.",HD_PAPER);
  label(p,18,109,"JOB SAVED / ONE-TIME BONUS",HD_GOLD);
 }else if(s->page==IN_RESULT){
  label(p,18,75,"STEP COMPLETE",HD_LAMP);
  label(p,18,87,"NEXT:",HD_PAPER);
  label(p,65,87,t->steps[interior_step(s)],HD_GOLD);
  label(p,18,100,t->order,HD_PAPER);
 }
 label(p,18,126,"A CONTINUE  B BACK",HD_STONE_LIT);
}
