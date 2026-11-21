#include "hd_canvas.h"
#include "studio.h"
/* The studio is a warm, physical tape-room interface, not a toy piano roll
   or a fake DAW. The six real editable tabs and actual PSG playback state
   drive everything painted below. The original save sidecar is unchanged. */
static void title(uint8_t *p,unsigned tab,int playing){
 hd_rect(p,0,0,HD_W,HD_H,HD_INK);
 for(int y=48;y<135;y+=9)hd_line(p,0,y,239,y,(y&1)?HD_SHADOW:HD_ROAD);
 hd_rect(p,0,0,HD_W,23,HD_SHADOW);
 hd_rect(p,0,21,HD_W,2,HD_COPPER);
 hd_text(p,9,5,"BORE ARCHIVE",HD_LAMP,1,2);
 hd_text(p,9,13,"THE MASTER TAPE",HD_PAPER,1,1);
 hd_text(p,184,8,playing?"PLAYING":"STOPPED",playing?HD_LEAF_LIT:HD_STONE_LIT,1,0);
 static const char *const names[STUDIO_TAB_COUNT]={
  "SONG","NOTES","ORDER","LYRICS","CHOIR","LIVE"
 };
 for(unsigned i=0;i<STUDIO_TAB_COUNT;i++){
  int x=4+(int)i*39;
  int selected=i==tab;
  hd_rect(p,x,27,37,20,selected?HD_ROAD_COOL:HD_SHADOW);
  hd_rect(p,x,27,37,selected?2:1,selected?HD_LAMP:HD_STONE_DARK);
  hd_rect(p,x,46,37,1,selected?HD_COPPER:HD_STONE_DARK);
  int width=hd_text_width(names[i],1,0);
  hd_text(p,x+(37-width)/2,34,names[i],selected?HD_PAPER:HD_STONE_LIT,1,0);
 }
 hd_rect(p,5,51,230,82,HD_SHADOW);
 hd_rect(p,7,53,226,78,HD_INK);
 hd_rect(p,7,53,3,78,HD_COPPER);
 hd_rect(p,0,136,240,24,HD_SHADOW);
 hd_rect(p,0,136,240,1,HD_COPPER);
 hd_text(p,8,140,"L/R TABS       SELECT TO PREVIEW",HD_STONE_LIT,1,0);
 hd_text(p,8,150,"D-PAD EDIT     A SET    B LEAVE",HD_PAPER,1,0);
}
static void label(uint8_t *p,const char *name,const char *detail){
 hd_text(p,17,58,name,HD_PAPER,1,1);
 hd_text(p,17,68,detail,HD_STONE_LIT,1,0);
 hd_line(p,17,79,224,79,HD_STONE_DARK);
}
static void value_row(uint8_t *p,int y,const char *name,const char *value,int chosen){
 hd_rect(p,18,y,204,20,chosen?HD_ROAD_COOL:HD_SHADOW);
 hd_rect(p,18,y,chosen?3:1,20,chosen?HD_LAMP:HD_STONE_DARK);
 hd_text(p,27,y+6,name,chosen?HD_LAMP:HD_STONE_LIT,1,1);
 hd_text(p,133,y+6,value,HD_PAPER,1,1);
}
static void record(uint8_t *p,int cx,int cy,int rx,int ry,unsigned t){
 hd_ellipse(p,cx,cy,rx,ry,HD_ROAD_COOL);
 hd_ellipse(p,cx,cy,rx-3,ry-3,HD_STONE_DARK);
 hd_ellipse(p,cx,cy,rx-6,ry-6,HD_INK);
 hd_ellipse(p,cx,cy,rx/2,ry/2,HD_BRICK_DARK);
 hd_ellipse(p,cx,cy,rx/2-2,ry/2-2,HD_LAMP);
 hd_ellipse(p,cx,cy,3,3,HD_PAPER);
 static const int8_t coords[8][2]={
  {1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}
 };
 unsigned angle=(t/13u)&7u;
 hd_line(p,cx,cy,cx+coords[angle][0]*(rx-7),cy+coords[angle][1]*(ry-6),
         HD_STONE_LIT);
 hd_ellipse(p,cx,cy,2,2,HD_INK);
}
static void notes(uint8_t *p,const StudioPatch *a,unsigned cursor,int playing){
 label(p,"NOTES / MELODY","EIGHT STEPS / SEVEN PITCHES / REST");
 const int left=23,top=83,stepw=24,rowh=5;
 for(int row=0;row<8;row++){
  int y=top+row*5;
  hd_rect(p,left,y,192,1,row==7?HD_COPPER:HD_STONE_DARK);
  if(row<7)hd_rect(p,left-10,y+2,7,1,row%2?HD_STONE:HD_PAPER);
 }
 for(unsigned i=0;i<8u;i++){
  int x=left+(int)i*stepw,selected=i==cursor;
  hd_rect(p,x,top,1,40,i==4u?HD_COPPER:HD_STONE_DARK);
  if(selected){
   hd_rect(p,x+1,top+1,stepw-2,38,HD_ROAD_COOL);
   hd_rect(p,x+1,top,stepw-2,1,HD_LAMP);
  }
  if(a->notes[i]){
   int row=7-(int)a->notes[i];
   hd_rect(p,x+3,top+row*rowh+1,stepw-5,4,selected?HD_PAPER:HD_GOLD);
   hd_rect(p,x+3,top+row*rowh+4,stepw-5,1,HD_COPPER);
  }else hd_rect(p,x+9,top+37,5,1,HD_STONE);
  if(playing&&i==studio_playhead())
   hd_rect(p,x+stepw/2,top,1,40,HD_LEAF_LIT);
  char count[2]={(char)('1'+i),0};
  hd_text(p,x+10,124,count,selected?HD_PAPER:HD_STONE_LIT,1,0);
 }
 hd_text(p,18,58,"NOTES / MELODY",HD_PAPER,1,1);
 char current[12]="STEP 1";
 current[5]=(char)('1'+(cursor&7u));
 hd_text(p,165,59,current,HD_LAMP,1,0);
 hd_text(p,165,69,studio_note_label(a->notes[cursor&7u]),HD_PAPER,1,1);
 hd_text(p,18,140,"A REST/NOTE    UP/DOWN PITCH",HD_STONE_LIT,1,0);
 hd_text(p,18,150,"LEFT/RIGHT STEP     B LEAVE",HD_PAPER,1,0);
}
static void order(uint8_t *p,const StudioPatch *a,unsigned cursor){
 static const char *const names[4]={"BASE","UP","LOW","FILL"};
 label(p,"ARRANGEMENT / ORDER","FOUR PHRASES / EIGHT BEATS EACH");
 for(unsigned i=0;i<4u;i++){
  int x=18+(int)i*53;
  int selected=i==cursor;
  hd_rect(p,x,86,48,36,selected?HD_ROAD_COOL:HD_SHADOW);
  hd_rect(p,x,86,48,2,selected?HD_LAMP:HD_STONE_DARK);
  char count[2]={(char)('1'+i),0};
  hd_text(p,x+4,92,count,HD_COPPER,1,0);
  hd_text(p,x+10,102,names[a->order[i]],HD_PAPER,1,0);
  for(int k=0;k<5;k++){
   int h=1+(int)((a->order[i]+i+k*3)%7u);
   hd_rect(p,x+7+k*8,117-h,5,h,selected?HD_LAMP:HD_STONE_LIT);
  }
 }
}
static void lyrics(uint8_t *p,const StudioPatch *a){
 label(p,"LYRICS / CUE CARD","TEXT ON SCREEN / NO RECORDED VOCALS");
 hd_rect(p,18,84,204,41,HD_STONE_DARK);
 hd_rect(p,20,86,200,37,HD_PAPER);
 hd_rect(p,24,90,1,29,HD_COPPER);
 hd_text(p,32,91,"BORE / ARCHIVE NOTE",HD_BRICK_DARK,1,1);
 hd_line(p,31,101,210,101,HD_STONE);
 hd_text(p,31,109,studio_lyric(a->lyric),HD_INK,1,0);
}
static void choir(uint8_t *p,const StudioPatch *a,unsigned cursor){
 (void)cursor;
 label(p,"CHOIR / PSG LAYERS","REAL CHIP CHANNELS / FOUR VOICE MIX");
 static const char *const voices[4]={"LEAD","HARMONY","BASS","BEAT"};
 const unsigned active[4]={1,a->choir!=0,1,a->mood==2};
 for(int i=0;i<4;i++){
  int x=17+i*53;
  hd_rect(p,x,84,48,40,active[i]?HD_ROAD_COOL:HD_SHADOW);
  hd_rect(p,x,84,48,2,active[i]?HD_LAMP:HD_STONE_DARK);
  hd_text(p,x+3,91,voices[i],active[i]?HD_PAPER:HD_STONE_LIT,1,0);
  for(int v=0;v<6;v++){
   int h=(v*4+i*3+(int)a->choir*2)%17+2;
   if(!active[i])h=2;
   hd_rect(p,x+5+v*7,119-h,4,h,active[i]?HD_GOLD:HD_STONE_DARK);
  }
 }
}
static void perform(uint8_t *p,const StudioPatch *a,int playing){
 label(p,"PERFORM / MASTER PREVIEW","LIVE PSG PLAYBACK / YOUR ARRANGEMENT");
 int current=(int)studio_playpart();
 for(unsigned part=0;part<4u;part++){
  int x=19+(int)part*53;
  int highlighted=playing&&part==(unsigned)current;
  hd_rect(p,x,87,49,34,highlighted?HD_ROAD_COOL:HD_SHADOW);
  hd_rect(p,x,87,49,2,highlighted?HD_LAMP:HD_STONE_DARK);
  for(int i=0;i<8;i++){
   int h=a->notes[i]?3+(int)a->notes[i]*2:1;
   int xx=x+3+i*5;
   hd_rect(p,xx,113-h,4,h,highlighted&&i==(int)studio_playhead()?
                          HD_PAPER:a->notes[i]?HD_GOLD:HD_STONE);
  }
  char num[2]={(char)('1'+part),0};
  hd_text(p,x+22,90,num,highlighted?HD_PAPER:HD_STONE_LIT,1,0);
 }
 hd_text(p,17,125,studio_lyric(a->lyric),HD_STONE_LIT,1,0);
 hd_text(p,8,150,playing?"A STOP    SELECT PREVIEW    B LEAVE":
                          "A PLAY    SELECT PREVIEW    B LEAVE",HD_PAPER,1,0);
}
void hd_studio_draw(uint8_t *p,const Story *s,unsigned tick){
 const StudioPatch *a=studio_patch();
 unsigned tab=s->studio_tab<STUDIO_TAB_COUNT?s->studio_tab:0u;
 unsigned cursor=s->studio_cursor;
 int playing=s->activity_choice!=0;
 title(p,tab,playing);
 switch(tab){
  case STUDIO_SONG:{
   static const char *const tempos[4]={"SLOW","MID","FAST","BRISK"};
   static const char *const moods[3]={"SOFT","WARM","BRIGHT"};
   label(p,"SONG / RECORDING","EDIT TEMPO AND MUSICAL COLOR");
   value_row(p,84,"TEMPO",tempos[a->tempo],cursor==0u);
   value_row(p,107,"FEEL",moods[a->mood],cursor==1u);
   break;
  }
  case STUDIO_NOTES:notes(p,a,cursor,playing);break;
  case STUDIO_ORDER:order(p,a,cursor);break;
  case STUDIO_LYRICS:lyrics(p,a);break;
  case STUDIO_CHOIR:choir(p,a,cursor);break;
  default:perform(p,a,playing);break;
 }
 /* The moving vinyl runs only in SONG mode: one cinematic detail, not an
    expensive spinner on every page. 100% fixed-point draw instructions. */
 if(tab==STUDIO_SONG){
  record(p,196,105,16,16,tick);
  hd_rect(p,208,87,2,19,HD_STONE_LIT);
  hd_line(p,209,88,201,97,HD_COPPER);
  hd_text(p,18,127,"A CHANGE   UP/DOWN SELECT",HD_STONE_LIT,1,0);
 }
 if(tab!=STUDIO_NOTES && tab!=STUDIO_PERFORM)
  hd_text(p,8,150,"A SET   LEFT/RIGHT EDIT   B LEAVE",HD_PAPER,1,0);
}
