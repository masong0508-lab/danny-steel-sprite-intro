#include "hd_canvas.h"
#include "studio.h"
/* One compact tape-room interface. Selection is amber; playback is green.
   Patterns, phrases and voice indicators use the saved patch and sequencer.
   Painting never advances or changes the sequencer. */
static const char *const phrase_names[4]={"BASE","UP","LOW","FILL"};
static const char *const voice_names[4]={"LEAD","HARM","BASS","BEAT"};
static void title(uint8_t *p,unsigned tab,int playing,int basic,unsigned take){
 /* These three bands cover the page exactly once. The panel interior
    is already ink; paint its thin border without two full-panel fills. */
 hd_rect(p,0,23,HD_W,113,HD_INK);
 hd_rect(p,0,0,HD_W,23,HD_SHADOW);
 hd_rect(p,0,21,HD_W,2,HD_COPPER);
 hd_text(p,9,5,basic?"BASIC STUDIO":"ADVANCED STUDIO",HD_LAMP,1,1);
 if(basic){
  char next[]="TAKE 1/4 - START SAVE";next[5]=(char)('1'+take);
  hd_text(p,9,13,next,HD_PAPER,1,0);
 }else hd_text(p,9,13,"YOUR MASTER TAPE",HD_PAPER,1,1);
 hd_rect(p,176,9,3,5,playing?HD_LEAF_LIT:HD_STONE_DARK);
 hd_text(p,184,8,playing?"PLAYING":"STOPPED",
         playing?HD_LEAF_LIT:HD_STONE_LIT,1,0);
 static const char *const names[STUDIO_TAB_COUNT]={
  "SONG","NOTES","ORDER","LYRICS","CHOIR","LIVE"
 };
 for(unsigned j=0;j<(basic?3u:STUDIO_TAB_COUNT);j++){
  unsigned i=basic?(j==2u?STUDIO_PERFORM:j):j;
  int w=basic?76:37,x=4+(int)j*(basic?78:39),selected=i==tab;
  hd_rect(p,x,27,w,20,selected?HD_ROAD_COOL:HD_SHADOW);
  hd_rect(p,x,27,w,selected?3:1,selected?HD_LAMP:HD_STONE_DARK);
  hd_rect(p,x,46,w,1,selected?HD_COPPER:HD_STONE_DARK);
  int width=hd_text_width(names[i],1,0);
  hd_text(p,x+(w-width)/2,35,names[i],
          selected?HD_PAPER:HD_STONE_LIT,1,0);
 }
 hd_rect(p,5,51,230,2,HD_STONE_DARK);
 hd_rect(p,5,131,230,2,HD_STONE_DARK);
 hd_rect(p,5,53,2,78,HD_STONE_DARK);
 hd_rect(p,233,53,2,78,HD_STONE_DARK);
 hd_rect(p,7,53,3,78,HD_COPPER);
 hd_rect(p,0,136,240,24,HD_SHADOW);
 hd_rect(p,0,136,240,1,HD_COPPER);
}
static void label(uint8_t *p,const char *name,const char *detail){
 hd_text(p,17,58,name,HD_PAPER,1,1);
 hd_text(p,17,68,detail,HD_STONE_LIT,1,0);
 hd_line(p,17,79,224,79,HD_STONE_DARK);
}
static void value_row(uint8_t *p,int y,const char *name,const char *value,int chosen){
 /* Field and turntable bounds are disjoint. */
 hd_rect(p,18,y,141,19,chosen?HD_ROAD_COOL:HD_SHADOW);
 hd_rect(p,18,y,chosen?3:1,19,chosen?HD_LAMP:HD_STONE_DARK);
 hd_text(p,27,y+6,name,chosen?HD_LAMP:HD_STONE_LIT,1,1);
 hd_text(p,97,y+6,value,HD_PAPER,1,1);
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
static void song(uint8_t *p,const StudioPatch *a,unsigned cursor,
                 int playing,unsigned tick){
 static const char *const tempos[4]={"SLOW","MID","FAST","BRISK"};
 static const char *const moods[3]={"SOFT","WARM","BRIGHT"};
 label(p,"SONG / MASTER TAPE","PULSE LEAD / WAVE BASS / FOUR PARTS");
 value_row(p,84,"TEMPO",tempos[a->tempo],cursor==0u);
 value_row(p,107,"FEEL",moods[a->mood],cursor==1u);
 hd_rect(p,169,84,55,45,HD_SHADOW);
 hd_rect(p,170,85,53,2,HD_COPPER);
 record(p,194,104,17,14,playing?tick:0u);
 hd_rect(p,217,89,2,19,HD_STONE_LIT);
 hd_line(p,218,90,207,98,HD_COPPER);
 hd_rect(p,205,97,3,3,HD_PAPER);
 hd_rect(p,174,123,3,2,playing?HD_LEAF_LIT:HD_STONE_DARK);
 hd_text(p,182,121,"MASTER",HD_STONE_LIT,1,0);
}
static void notes(uint8_t *p,const StudioPatch *a,unsigned cursor,int playing){
 label(p,"NOTES / MELODY","8 STEPS / 7 PITCHES / REST");
 const int left=23,top=83,stepw=24,rowh=5;
 unsigned selected=cursor&7u,head=studio_playhead();
 unsigned level=a->mood==0u?7u:a->mood==1u?10u:12u;
 int level_width=(int)(19u*level/12u);
 /* Selection precedes the grid: every pitch row stays visible. */
 hd_rect(p,left+(int)selected*stepw+1,top+1,stepw-2,39,HD_ROAD_COOL);
 for(int row=0;row<8;row++){
  int y=top+row*rowh;
  hd_rect(p,left,y,192,1,row==7?HD_COPPER:HD_STONE_DARK);
  if(row<7){
   hd_rect(p,left-10,y+1,8,4,HD_PLASTER_LIT);
   hd_rect(p,left-10,y+4,8,1,HD_STONE_DARK);
  }
 }
 for(unsigned i=0;i<8u;i++){
  int x=left+(int)i*stepw,chosen=i==selected;
  hd_rect(p,x,top,i==4u?2:1,40,i==4u?HD_COPPER:HD_STONE_DARK);
  if(a->notes[i]){
   int row=7-(int)a->notes[i];
   hd_rect(p,x+3,top+row*rowh+1,stepw-5,3,chosen?HD_PAPER:HD_GOLD);
   hd_rect(p,x+3,top+row*rowh+4,stepw-5,1,HD_COPPER);
   /* Actual global sound level, not a nonexistent velocity edit. */
   hd_rect(p,x+3,120,level_width,2,HD_COPPER);
  }else hd_rect(p,x+9,top+37,5,1,HD_STONE);
  if(playing&&i==head){
   hd_rect(p,x+1,top+1,1,38,HD_LEAF_LIT);
   hd_rect(p,x+9,80,5,2,HD_LEAF_LIT);
  }
  if(chosen)hd_rect(p,x+3,82,19,1,HD_LAMP);
  char count[2]={(char)('1'+i),0};
  hd_text(p,x+10,124,count,chosen?HD_PAPER:HD_STONE_LIT,1,0);
 }
 char current[]="STEP 1";
 current[5]=(char)('1'+selected);
 hd_text(p,165,59,current,HD_LAMP,1,0);
 hd_text(p,165,69,studio_note_label(a->notes[selected]),HD_PAPER,1,1);
}
/* Mirrors the MIDI register changes in studio.c. The renderer contract
   test compares these preview heights with studio_motif_step output. */
static int pitch_height(unsigned note,unsigned mode){
 static const uint8_t pitch[8]={0,60,62,64,65,67,69,71};
 static const int8_t transpose[4]={0,2,-2,5};
 return note?(int)pitch[note]+transpose[mode]-57:0;
}
static void order(uint8_t *p,const StudioPatch *a,unsigned cursor,int playing){
 label(p,"ORDER / FOUR PHRASES","EACH PART PLAYS YOUR EIGHT STEPS");
 unsigned playpart=studio_playpart();
 for(unsigned i=0;i<4u;i++){
  int x=18+(int)i*53,chosen=i==(cursor&3u);
  unsigned mode=a->order[i];
  hd_rect(p,x,84,48,42,chosen?HD_ROAD_COOL:HD_SHADOW);
  hd_rect(p,x,84,48,2,chosen?HD_LAMP:HD_STONE_DARK);
  if(chosen)hd_rect(p,x,86,2,40,HD_LAMP);
  char count[2]={(char)('1'+i),0};
  hd_text(p,x+4,90,count,HD_COPPER,1,0);
  hd_text(p,x+13,90,phrase_names[mode],HD_PAPER,1,0);
  hd_line(p,x+4,100,x+43,100,HD_STONE_DARK);
  for(unsigned k=0;k<8u;k++){
   int h=pitch_height(a->notes[k],mode);
   hd_rect(p,x+4+(int)k*5,121-(h?h:1),3,h?h:1,
           h?HD_GOLD:HD_STONE_DARK);
  }
  if(playing&&i==playpart)hd_rect(p,x+4,123,40,2,HD_LEAF_LIT);
 }
}
static void lyrics(uint8_t *p,const StudioPatch *a,int playing){
 label(p,"LYRICS / REFRAIN","CHOOSE A LINE / FOLLOW THE PHRASE");
 hd_rect(p,18,84,204,43,HD_STONE_DARK);
 hd_rect(p,20,86,200,39,HD_PAPER);
 hd_rect(p,24,91,1,29,HD_COPPER);
 for(unsigned i=0;i<4u;i++)
  hd_rect(p,32+(int)i*13,90,9,3,i==a->lyric?HD_COPPER:HD_STONE);
 char cue[]="CUE 1/4";cue[4]=(char)('1'+a->lyric);
 hd_text(p,171,90,cue,HD_BRICK_DARK,1,0);
 const char *line=studio_lyric(a->lyric);
 hd_text(p,32,103,line,HD_INK,1,1);
 if(playing){
  /* A word cue follows the actual eight-step phrase. Drawing does not
     consume a musical event or imply a recorded vocal track. */
  unsigned words=1;
  for(unsigned i=0;line[i];i++)if(line[i]==' ')words++;
  unsigned target=studio_playhead()*words/8u,word=0,start=0,end=0;
  while(line[start]&&word<target){if(line[start]==' ')word++;start++;}
  end=start;while(line[end]&&line[end]!=' ')end++;
  hd_rect(p,32+(int)start*6,113,(int)(end-start)*6-1,2,HD_BRICK);
 }
}
static unsigned voice_events(const StudioPatch *a,unsigned step){
 return (a->notes[step&7u]?1u:0u)|
        (a->choir&&!(step&1u)?2u:0u)|
        (!(step&3u)?4u:0u)|
        (a->mood==2u&&(step&1u)?8u:0u);
}
static void choir(uint8_t *p,const StudioPatch *a,int playing){
 static const char *const harmony[4]={"OFF","FIFTH","OCTAVE","HIGH"};
 label(p,"CHOIR / HARMONY","SET THE HARMONY / SEE EACH VOICE");
 unsigned head=studio_playhead();
 for(unsigned i=0;i<4u;i++){
  int x=18+(int)i*53,chosen=i==a->choir;
  hd_rect(p,x,84,48,16,chosen?HD_ROAD_COOL:HD_SHADOW);
  hd_rect(p,x,84,48,chosen?2:1,chosen?HD_LAMP:HD_STONE_DARK);
  int w=hd_text_width(harmony[i],1,0);
  hd_text(p,x+(48-w)/2,89,harmony[i],chosen?HD_PAPER:HD_STONE_LIT,1,0);
  hd_text(p,x+4,106,voice_names[i],HD_STONE_LIT,1,0);
  if(playing)hd_rect(p,x+1+(int)head*6,124,4,2,HD_LEAF_LIT);
 }
 for(unsigned step=0;step<8u;step++){
  unsigned events=voice_events(a,step);
  for(unsigned voice=0;voice<4u;voice++){
   int active=(events&(1u<<voice))!=0;
   hd_rect(p,19+(int)voice*53+(int)step*6,117,4,4,
           active?HD_GOLD:HD_STONE_DARK);
  }
 }
}
/* Four recognizable miniature instruments share one 48x21 stage module.
   The cues are driven by emitted PSG event bits, not random animation. */
static void stage_symbol(uint8_t *p,int x,int y,unsigned voice,int lit,
                         unsigned tick){
 unsigned shine=lit?((tick/12u)&1u?HD_LAMP:HD_PAPER):HD_STONE_LIT;
 int cx=x+37;
 switch(voice){
 case 0: /* LEAD - a microphone and full stand */
  hd_ellipse(p,cx,y-5,3,2,shine);
  hd_line(p,cx,y-3,cx,y+3,HD_COPPER);
  hd_line(p,cx-3,y+3,cx+3,y+3,HD_STONE_LIT);
  break;
 case 1: /* HARM - twin rows of a tiny electric keyboard */
  hd_rect(p,x+31,y-8,15,8,HD_INK);
  hd_rect(p,x+32,y-7,13,6,HD_PAPER);
  hd_rect(p,x+34,y-7,2,3,HD_SHADOW);
  hd_rect(p,x+39,y-7,2,3,HD_SHADOW);
  hd_rect(p,x+43,y-7,1,3,HD_SHADOW);
  hd_rect(p,x+32,y-1,13,1,shine);
  break;
 case 2: /* BASS - rounded instrument body and straight strings */
  hd_ellipse(p,cx-2,y-1,4,3,shine);
  hd_line(p,cx,y-4,cx+5,y-9,HD_COPPER);
  hd_line(p,cx-2,y-3,cx+4,y-9,HD_STONE_LIT);
  hd_pixel(p,cx-2,y-1,HD_INK);
  break;
 default: /* BEAT - drum, rim and mounted cymbal */
  hd_ellipse(p,cx,y-1,5,3,HD_STONE_LIT);
  hd_rect(p,cx-4,y-1,8,4,HD_BARK);
  hd_line(p,cx-5,y-5,cx+5,y-5,shine);
  hd_line(p,cx,y-5,cx,y-9,HD_COPPER);
  hd_line(p,cx,y+3,cx,y+4,HD_COPPER);
  break;
 }
}
static void perform(uint8_t *p,const StudioPatch *a,int playing,
                    unsigned tick){
 char position[]="PART 1/4   STEP 1/8";
 unsigned head=studio_playhead(),playpart=studio_playpart();
 position[5]=(char)('1'+playpart);
 position[16]=(char)('1'+head);
 label(p,"PERFORM / LIVE MIX",playing?position:"SELECT OR A TO PLAY YOUR MASTER");
 unsigned events=playing?voice_events(a,head):0u;
 for(unsigned part=0;part<4u;part++){
  int x=18+(int)part*53;
  int current=playing&&part==playpart;
  int lit=(events&(1u<<part))!=0u;
  hd_rect(p,x,84,48,13,current?HD_ROAD_COOL:HD_SHADOW);
  hd_rect(p,x,84,48,2,current?HD_LEAF_LIT:HD_STONE_DARK);
  char num[2]={(char)('1'+part),0};
  hd_text(p,x+3,89,num,current?HD_PAPER:HD_STONE_LIT,1,0);
  hd_text(p,x+13,89,phrase_names[a->order[part]],HD_STONE_LIT,1,0);
  /* Each voice lives on a physical stage lane, not a generic meter box. */
  hd_rect(p,x,101,48,21,HD_SHADOW);
  hd_rect(p,x,101,48,1,lit?HD_COPPER:HD_STONE_DARK);
  hd_rect(p,x,101,1,21,HD_STONE_DARK);
  hd_rect(p,x+47,101,1,21,HD_STONE_DARK);
  hd_rect(p,x+1,120,46,2,HD_ROAD_COOL);
  if(lit){
   unsigned ray=(tick/15u)&1u?HD_COPPER:HD_ROAD_COOL;
   hd_line(p,x+27,102,x+33,111,ray);
   hd_line(p,x+44,102,x+41,110,ray);
  }
  hd_text(p,x+4,103,voice_names[part],HD_PAPER,1,0);
  /* Keep the instrument identity above its gate strip at native scale. */
  stage_symbol(p,x,111,part,lit,tick);
  /* Existing test-proven PSG gate remains at exactly y=114..117. */
  hd_rect(p,x+4,114,40,4,lit?HD_LAMP:HD_STONE_DARK);
 }
 hd_text(p,18,124,studio_lyric(a->lyric),HD_STONE_LIT,1,0);
}
void hd_studio_draw(uint8_t *p,const Story *s,unsigned tick){
 const StudioPatch *a=studio_patch();
 unsigned tab=s->studio_tab<STUDIO_TAB_COUNT?s->studio_tab:0u;
 unsigned cursor=s->studio_cursor;
 int playing=s->activity_choice!=0;
 int basic=s->song_steps<STORY_PROJECT_DONE;
 title(p,tab,playing,basic,s->song_steps);
 switch(tab){
  case STUDIO_SONG:song(p,a,cursor,playing,tick);break;
  case STUDIO_NOTES:notes(p,a,cursor,playing);break;
  case STUDIO_ORDER:order(p,a,cursor,playing);break;
  case STUDIO_LYRICS:lyrics(p,a,playing);break;
  case STUDIO_CHOIR:choir(p,a,playing);break;
  default:perform(p,a,playing,tick);break;
 }
 /* Match story_tick: LEFT/RIGHT selects fields; UP/DOWN edits. Keep
    the actual preview toggle visible on every page, including NOTES. */
 const char *row1=tab==STUDIO_NOTES?
  "L/R TABS LEFT/RIGHT STEP SEL PLAY":tab==STUDIO_ORDER?
  "L/R TABS LEFT/RIGHT PART SEL PLAY":tab==STUDIO_SONG?
  "L/R TABS LEFT/RIGHT FIELD SEL PLAY":"L/R TABS SELECT PLAY/STOP";
 const char *row2=tab==STUDIO_NOTES?
  "UP/DOWN PITCH A REST B EXIT":tab==STUDIO_ORDER?
  "UP/DOWN VARIANT A NEXT B EXIT":tab==STUDIO_PERFORM?
  (playing?"A STOP B EXIT":"A PLAY B EXIT"):
  "UP/DOWN CHANGE A NEXT B EXIT";
 if(basic){
  row1=tab==STUDIO_NOTES?"L/R TABS D-PAD EDIT A REST":
       tab==STUDIO_PERFORM?"L/R TABS A OR SELECT PLAY":
       "L/R TABS D-PAD CHANGE TEMPO/FEEL";
  row2="SELECT PLAY  START SAVE TAKE  B EXIT";
 }
 hd_text(p,8,139,row1,HD_STONE_LIT,1,0);
 hd_text(p,8,150,row2,HD_PAPER,1,0);
}
