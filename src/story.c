#include "story.h"
#include "story_checks.h"
#include "studio.h"

#include "city_layout.h"
#if !defined(__arm__)
/* Original immutable map retained for the original material generators
   and independent shipped-image regression, never selected on cartridge. */
static int core_map_reference;
void story_debug_core_map(int enabled){core_map_reference=!!enabled;}
static const char *const town[STORY_CORE_H] = {
"########################",
"#......................#",
"#...TT.H..........TT...#",
"#..TTTT...####....TT...#",
"#.........#AA#.........#",
"#...L.....#..#..K..B...#",
"#.........####R........#",
"#..P.......C.....V.M.S.#",
"#...,,,,,,,,,,,,,,,,...#",
"#..T,,,......,,,T.gg...#",
"#..TT,,......,,TT.gg####",
"#...g,W.F....,,......#.#",
"#...g,,,,,,,,,,...Q..#.#",
"#..TT.......D....gg..#.#",
"#..TT............~~~...#",
"#...............~~~~...#",
"#...............~~~~...#",
"########################"
};
#endif

char story_tile(int x, int y) {
#if !defined(__arm__)
    if(core_map_reference)return (unsigned)x<STORY_CORE_W&&(unsigned)y<STORY_CORE_H?town[y][x]:'#';
#endif
    if ((unsigned)x >= STORY_W || (unsigned)y >= STORY_H) return '#';
    return city_layout[y][x];
}
const char *story_district_name(int x,int y){
    if(!story_city_extension(x,y))return "OLD QUARTER";
    if(y<14)return "EAST AVENUE";
    if(y<20&&x>=24)return "CIVIC ROW";
    if(x<15)return "SOUTH PARK";
    if(x<25)return "CANAL WALK";
    return "RIVER PLAZA";
}
/* Fourteen optional, existing walkable sites; ordering matches ROM panels. */
static const struct {uint8_t x,y;const char *name,*left,*right;}
neighborhood[STORY_NEIGHBORHOOD_COUNT]={
 {2,2,"LAUNDRY","MEND","RETURN"},
 {9,2,"PHOTO","SIGN","NO NAME"},
 {14,2,"STAIR","SKETCH","LISTEN"},
 {21,3,"REPAIR","REPAIR","DONATE"},
 {2,5,"POST","WRITE","KEEP"},
 {6,5,"STOOP","KNOCK","NOTE"},
 {8,8,"MARKET","HELP","BUY"},
 {13,8,"ANNEX","LABEL","RESTORE"},
 {15,9,"GARDEN","PLAN","REST"},
 {20,9,"SHIFT","FOLD","THANK"},
 {2,12,"PARK","SHARE","SIT"},
 {10,13,"PHONE","CALL","MESSAGE"},
 {14,14,"CANAL","SKETCH","CLEAN"},
 {22,15,"SHELTER","MARK","WAIT"}
};
int story_neighborhood_enabled(const Story *s){
 return !s->ended&&(s->flags&STORY_ALL)==STORY_ALL&&
 (s->life_stage==STORY_LIFE_NONE||s->life_stage==STORY_LIFE_WAIT||
  s->life_stage==STORY_LIFE_FOLLOWUP_WAIT)&&
 (s->rescue_stage==STORY_RESCUE_NONE||s->rescue_stage==STORY_RESCUE_SAVED);
}
int story_neighborhood_position(unsigned id,int *x,int *y){
 if(id>=STORY_NEIGHBORHOOD_COUNT)return 0;
 if(x)*x=neighborhood[id].x;
 if(y)*y=neighborhood[id].y;
 return 1;
}
int story_neighborhood_at(const Story *s){
 if(!story_neighborhood_enabled(s))return -1;
 for(unsigned i=0;i<STORY_NEIGHBORHOOD_COUNT;i++)
  if(s->x==neighborhood[i].x&&s->y==neighborhood[i].y)return (int)i;
 return -1;
}
int story_neighborhood_revisit(const Story *s){
 int at=story_neighborhood_at(s);
 return at>=0&&s->rescue_stage==STORY_RESCUE_NONE&&
        (s->neighborhood_seen&(1u<<(unsigned)at))?at:-1;
}
const char *story_neighborhood_name(unsigned id){
 return id<STORY_NEIGHBORHOOD_COUNT?neighborhood[id].name:"STORIES";
}
const char *story_neighborhood_option(unsigned id,unsigned right){
 if(id>=STORY_NEIGHBORHOOD_COUNT)return "";
 return right?neighborhood[id].right:neighborhood[id].left;
}
unsigned story_neighborhood_count(const Story *s){
 unsigned n=0;
 for(unsigned i=0;i<STORY_NEIGHBORHOOD_COUNT;i++)n+=(s->neighborhood_seen>>i)&1u;
 return n;
}
static int neighborhood_tracking(const Story *s){
 return story_neighborhood_enabled(s)&&s->neighborhood_target>0u&&
        s->neighborhood_target<=STORY_NEIGHBORHOOD_COUNT;
}
const char *story_track_name(unsigned choice){
 static const char *const names[STORY_TRACK_COUNT]={"MUSIC STUDIO","ROOFTOP ART","MIRROR ORCHARD","MAIN STORY","CAFE SHOW"};
 return choice<STORY_TRACK_COUNT?names[choice]:"MAIN STORY";
}
unsigned story_track_selected(const Story *s){
 unsigned n=(s->choice&STORY_TRACK_CHOICE_MASK)>>1;
 return n<STORY_TRACK_COUNT?n:0u;
}
unsigned story_track_id(unsigned choice){
 return choice==STORY_TRACK_SHOW_INDEX?STORY_TRACK_SHOW:
        choice<3u?STORY_TRACK_STUDIO+choice:0u;
}
static int show_available(const Story *s){
 return story_neighborhood_enabled(s)&&!story_bad_count(s)&&story_show_stage(s)<STORY_SHOW_DONE;
}
static int show_tracking(const Story *s){
 return s->neighborhood_target==STORY_TRACK_SHOW&&show_available(s);
}
static void show_goal(const Story *s,int *x,int *y,const char **label){
 switch(story_show_stage(s)){
 case 0:*x=8;*y=11;*label="PLAN SHOW";break;
 case 1:*x=9;*y=2;*label="POSTER";break;
 case 2:*x=19;*y=7;*label=s->song_steps<STORY_PROJECT_DONE?"MAKE TAPE":"REHEARSE";break;
 case 3:*x=14;*y=6;*label=s->art_steps<STORY_PROJECT_DONE?"PAINT":"FRAME ART";break;
 case 4:*x=13;*y=8;*label="SET UP";break;
 case 5:*x=8;*y=11;*label="CAFE SHOW";break;
 default:*x=11;*y=7;*label="ARCHIVE";break;
 }
}
static unsigned show_here(const Story *s){
 if(!show_tracking(s))return 0;
 int x,y;const char *label;show_goal(s,&x,&y,&label);
 int dx=x-(int)s->x,dy=y-(int)s->y;
 int distance=(dx<0?-dx:dx)+(dy<0?-dy:dy);
 return distance==(story_solid(x,y)?1:0)?story_show_stage(s)+1u:0u;
}
const char *story_paint_layer(const Story *s){
 static const char *const names[]={"SKY WASH","CITY BLOCKS","WINDOW LIGHT","FINISH EDGES"};
 return names[s->art_steps<4u?s->art_steps:3u];
}
const char *story_show_phrase(unsigned part){
 static const char *const names[]={"BASE","UP","LOW","FILL"};
 return names[part&3u];
}
unsigned story_show_expected(const Story *s){return studio_patch()->order[s->studio_tab&3u];}
const char *story_show_title(const Story *s){
 static const char *const names[]={"CAFE / PLAN","PHOTO / POSTER","STUDIO / REHEARSAL",
  "ROOF / FRAME","ANNEX / SETUP","CAFE / LIVE SHOW","BORE / ARCHIVE","SHOW COMPLETE"};
 return names[story_show_stage(s)];
}
const char *story_show_option(const Story *s,unsigned right){
 switch(story_show_stage(s)){
 case 0:return right?"LATER":"PLAN";
 case 1:return right?"SUNSET":"BLUE HOUR";
 case 3:return right?"WOOD":"BLACK";
 case 4:return right?"LATER":"SET UP";
 case 5:return right?"LATER":"PLAY";
 case 6:return right?"LATER":"FILE";
 default:return "BACK";
 }
}
int story_track_available(const Story *s,unsigned choice){
 if(choice==STORY_TRACK_MAIN_INDEX)return 1;
 if(choice==STORY_TRACK_SHOW_INDEX)return show_available(s);
 if(choice>=3u||s->ended||
    !(s->life_stage==STORY_LIFE_NONE||s->life_stage==STORY_LIFE_WAIT||
      s->life_stage==STORY_LIFE_FOLLOWUP_WAIT)||
    !(s->rescue_stage==STORY_RESCUE_NONE||s->rescue_stage==STORY_RESCUE_SAVED))return 0;
 return choice!=2u||(s->rescue_stage==STORY_RESCUE_NONE&&s->cult_stage<CULT_RESOLVED);
}
static int creative_tracking(const Story *s){
 unsigned id=s->neighborhood_target;
 return id>=STORY_TRACK_STUDIO&&id<=STORY_TRACK_ORCHARD&&story_track_available(s,id-STORY_TRACK_STUDIO);
}
static int share_pending(const Story *s){
 return s->song_steps>=STORY_PROJECT_DONE&&(s->flags&STORY_MARA)&&
        !story_bad_count(s)&&!(s->project_style&STORY_STYLE_SONG_SHARED);
}
int story_goal(const Story *s, int *x, int *y, const char **label) {
    if (s->ended || s->life_stage==STORY_LIFE_FINALE) return 0;
    if(s->rescue_stage==STORY_RESCUE_MISSING){
        *x=6;*y=11;*label="WITNESS";return 1;
    }
    if(s->rescue_stage==STORY_RESCUE_WITNESS){
        *x=21;*y=7;*label="TERMINAL";return 1;
    }
    if(s->rescue_stage==STORY_RESCUE_TRACE){
        *x=7;*y=2;*label="SAVE MARA";return 1;
    }
    if(s->rescue_stage==STORY_RESCUE_LOST){
        /* No rescue deadline remains, but the still-reachable cafe phone
           is the explicit center of Danny's two-day hope subplot. */
        *x=8;*y=11;*label="PHONE";return 1;
    }
    if(show_tracking(s)){show_goal(s,x,y,label);return 1;}
    if(creative_tracking(s)){
        switch(s->neighborhood_target){
        case STORY_TRACK_STUDIO:
            if(share_pending(s)){*x=8;*y=11;*label="SHARE SONG";}
            else {*x=19;*y=7;*label="STUDIO";}
            return 1;
        case STORY_TRACK_ROOF:*x=14;*y=6;*label="PAINT";return 1;
        default:
            if(s->cult_stage==1u){*x=6;*y=11;*label="WITNESS";}
            else if(s->cult_stage==2u){*x=7;*y=2;*label="SAFE HOUSE";}
            else {*x=21;*y=7;*label="TERMINAL";}
            return 1;
        }
    }
    if(neighborhood_tracking(s)){
        unsigned id=s->neighborhood_target-1u;
        *x=neighborhood[id].x;*y=neighborhood[id].y;
        *label=neighborhood[id].name;return 1;
    }
    /* A medical appointment takes priority; there is deliberately no timer
       forcing a final rooftop interaction after the last visit. */
    if (s->life_stage==STORY_LIFE_CLINIC ||
        s->life_stage==STORY_LIFE_FINAL_CLINIC) {
        *x=16; *y=5; *label="CLINIC";
    } else if (s->life_stage==STORY_LIFE_LAST_WORK) {
        *x=14; *y=6; *label="CANVAS";
    } else if (s->life_stage==STORY_LIFE_WAIT ||
               s->life_stage==STORY_LIFE_FOLLOWUP_WAIT) {
        /* Suggestions, not mandatory objectives; days continue regardless. */
        if (!(s->daily_mask&STORY_DAY_MARA)) {
            *x=8; *y=11; *label="MARA";
        } else if (!(s->daily_mask&STORY_DAY_LETTER)) {
            *x=5; *y=5; *label="LETTER";
        } else if (!(s->daily_mask&STORY_DAY_CLERK)) {
            *x=11; *y=7; *label="BORE";
        } else if (!(s->daily_mask&STORY_DAY_ART)) {
            *x=14; *y=6; *label="PAINT";
        } else if (!(s->daily_mask&STORY_DAY_MUSIC)) {
            *x=19; *y=7; *label="MUSIC";
        } else {
            if(s->art_steps<STORY_PROJECT_DONE){
                *x=14; *y=6; *label="ART WORK";
            }else if(s->song_steps<STORY_PROJECT_DONE){
                *x=19; *y=7; *label="MIX TAPE";
            }else{
                *x=14; *y=6; *label="REST";
            }
        }
    } else if (!(s->flags&STORY_HOME)) {
        *x=5; *y=5; *label="LETTER";
    } else if (!(s->flags&STORY_STAMP)) {
        *x=11; *y=7; *label="STAMP";
    } else if (!(s->flags&STORY_MARA)) {
        *x=8; *y=11; *label="MARA";
    } else if (!(s->flags&STORY_MUSIC)) {
        *x=19; *y=7; *label="MUSIC";
    } else if (!(s->flags&STORY_ART)) {
        *x=14; *y=6; *label="CANVAS";
    } else {
        *x=19; *y=5; *label="LAST BUS";
    }
    return 1;
}
int story_solid(int x, int y) {
    const char c = story_tile(x, y);
    return c == '#' || c == 'A' || c == 'T' || c == '~' || c == 'L' || c == 'C' || c == 'F' || c == 'B' || c == 'P' || c == 'V' || c == 'Q' || c == 'S' || c == 'W' || c == 'H' || c == 'R' || c == 'M' || c == 'K';
}
/* Reverse flood from the current goal ONCE, not a new city search
   every step. All four routes use walkable tiles; a solid goal seeds its
   adjacent approaches. E/S/W/N tie order matches the original forward BFS.
   4,800 bytes fixed scratch for 1,200 cells; zero heap or save growth. */
int story_route(const Story *s, int *direction, int *steps){
    static uint16_t distance[STORY_W*STORY_H],queue[STORY_W*STORY_H];
    static int last_x=-1,last_y=-1;
#if !defined(__arm__)
    static int last_reference=-1;
#endif
    static const int8_t dx[4]={1,0,-1,0},dy[4]={0,1,0,-1};
    int gx=0,gy=0;const char *name=0;
    if(direction)*direction=-1;
    if(steps)*steps=-1;
    if(!story_goal(s,&gx,&gy,&name)||
       (unsigned)gx>=STORY_W||(unsigned)gy>=STORY_H||
       story_solid(s->x,s->y))return 0;
    if(gx!=last_x||gy!=last_y
#if !defined(__arm__)
       ||last_reference!=core_map_reference
#endif
       ){
        last_x=gx;last_y=gy;
#if !defined(__arm__)
        last_reference=core_map_reference;
#endif
        for(unsigned i=0;i<STORY_W*STORY_H;i++)distance[i]=UINT16_MAX;
        unsigned read=0,write=0;
        int solid=story_solid(gx,gy);
        for(unsigned d=0;d<(solid?4u:1u);d++){
            int x=gx+(solid?dx[d]:0),y=gy+(solid?dy[d]:0);
            if(story_solid(x,y))continue;
            unsigned id=(unsigned)y*STORY_W+(unsigned)x;
            distance[id]=0;queue[write++]=(uint16_t)id;
        }
        while(read<write){
            unsigned id=queue[read++];
            int x=(int)(id%STORY_W),y=(int)(id/STORY_W);
            for(unsigned d=0;d<4u;d++){
                int nx=x+dx[d],ny=y+dy[d];
                if(story_solid(nx,ny))continue;
                unsigned next=(unsigned)ny*STORY_W+(unsigned)nx;
                if(distance[next]!=UINT16_MAX)continue;
                distance[next]=(uint16_t)(distance[id]+1u);
                queue[write++]=(uint16_t)next;
            }
        }
    }
    unsigned id=(unsigned)s->y*STORY_W+s->x;
    unsigned n=distance[id];
    if(n==UINT16_MAX)return 0;
    if(steps)*steps=(int)n;
    if(n&&direction){
        for(unsigned d=0;d<4u;d++){
            int x=(int)s->x+dx[d],y=(int)s->y+dy[d];
            if(story_solid(x,y))continue;
            if(distance[(unsigned)y*STORY_W+(unsigned)x]+1u==n){
                *direction=(int)d;break;
            }
        }
    }
    return 1;
}
const char *story_risk_action(const Story *s){
    switch(s->risk_menu){
    case 'H':return "LEAVE";
    case 'L':return "HIDE REPLY";
    case 'C':return "FORGE RECORD";
    case 'M':return "ERASE DEMO";
    case 'S':return "LEAK NAME";
    default:return "BACK";
    }
}
const char *story_goal_reason(const Story *s){
    if(s->ended||s->life_stage==STORY_LIFE_FINALE)
        return "YOUR STORY IS STILL HERE";
    switch(s->rescue_stage){
    case STORY_RESCUE_MISSING:return "ASK THE WITNESS FOR CLUES";
    case STORY_RESCUE_WITNESS:return "TRACE THE VEHICLE WITH CARE";
    case STORY_RESCUE_TRACE:return "HELP MARA AT THE SAFE HOUSE";
    case STORY_RESCUE_LOST:return "NO VERIFIED RECOVERY YET";
    default:break;
    }
    if(show_tracking(s)){
        switch(story_show_stage(s)){
        case 0:return "ASK MARA ABOUT A SHOW";
        case 1:return "DESIGN A SHOW POSTER";
        case 2:return s->song_steps<4u?"FINISH FOUR MUSIC TAKES":"MATCH YOUR FOUR PHRASES";
        case 3:return s->art_steps<4u?"FINISH FOUR ART LAYERS":"CHOOSE THE FRAME";
        case 4:return "INSTALL THE POSTER";
        case 5:return "PLAY YOUR SAVED SONG";
        default:return "LEAVE A COPY AT BORE";
        }
    }
    if(creative_tracking(s)){
        if(s->neighborhood_target==STORY_TRACK_STUDIO)
            return share_pending(s)?"PLAY YOUR SONG FOR MARA":s->song_steps>=4u?
                "ADVANCED / EDIT OR PLAY":"EAST PLAZA / SAVE A TAKE";
        if(s->neighborhood_target==STORY_TRACK_ROOF)return "ROOFTOP / MAKE ART";
        return s->cult_stage==1u?"LISTEN TO THE WITNESS":s->cult_stage==2u?
            "VISIT THE SAFE HOUSE":"OPTIONAL SERVER STORY";
    }
    if(neighborhood_tracking(s))return "OPTIONAL STORY / A ON ARRIVAL";
    switch(s->life_stage){
    case STORY_LIFE_CLINIC:case STORY_LIFE_FINAL_CLINIC:
        return "THE DOCTOR IS WAITING";
    case STORY_LIFE_LAST_WORK:return "FINISH THE LAST PAINTING";
    case STORY_LIFE_WAIT:case STORY_LIFE_FOLLOWUP_WAIT:
        if((s->daily_mask&STORY_DAY_ALL)==STORY_DAY_ALL&&
           s->song_steps>=STORY_PROJECT_DONE&&s->art_steps>=STORY_PROJECT_DONE)
            return "REST OR EXPLORE UNTIL CALL";
        return "MAKE MEMORIES AT YOUR PACE";
    default:break;
    }
    if(!(s->flags&STORY_HOME))return "LETTER / WEST OLD QUARTER";
    if(!(s->flags&STORY_STAMP))return "PROVE ADDRESS AT BORE";
    if(!(s->flags&STORY_MARA))return "TELL MARA ABOUT THE CALL";
    if(!(s->flags&STORY_MUSIC))return "OPTIONAL TAPE / BUS OPEN";
    if(!(s->flags&STORY_ART))return "OPTIONAL ROOF / BUS OPEN";
    return "BOARD THE BUS OR STAY";
}
/* Pure, short chapter tag shared by both GBA HUD modes. The first three
   tasks unlock the bus; creating art and music changes the legacy ending
   but never falsely appears to be a mandatory gate. */
const char *story_goal_kind(const Story *s){
    if(s->ended || s->life_stage==STORY_LIFE_FINALE)return "";
    if(s->rescue_stage==STORY_RESCUE_LOST)return "HOPE";
    if(s->rescue_stage>=STORY_RESCUE_MISSING &&
       s->rescue_stage<=STORY_RESCUE_TRACE)return "SAVE";
    if(s->life_stage==STORY_LIFE_CLINIC ||
       s->life_stage==STORY_LIFE_FINAL_CLINIC)return "CARE";
    if(s->life_stage==STORY_LIFE_LAST_WORK)return "LAST";
    if(show_tracking(s))return "SHOW";
    if(creative_tracking(s)||neighborhood_tracking(s))return "SIDE";
    if(s->life_stage==STORY_LIFE_WAIT ||
       s->life_stage==STORY_LIFE_FOLLOWUP_WAIT)return "DAY";
    if((s->flags&STORY_ALL)!=STORY_ALL)return "CORE";
    if((s->flags&STORY_FULL)!=STORY_FULL)return "EXTRA";
    return "BUS";
}

enum { D_NONE, D_OPEN, D_LETTER, D_LETTER_AGAIN, D_CLERK_LOCKED, D_CLERK, D_CLERK_AGAIN,
       D_MARA_LOCKED, D_MARA, D_MARA_AGAIN, D_BUS_LOCKED, D_BUS_READY, D_END_GO,
       D_END_STAY, D_JOURNAL_0, D_JOURNAL_1, D_JOURNAL_2, D_JOURNAL_3, D_NOTHING, D_RETURN,
       D_CULT_FIND, D_CULT_LOGS, D_CULT_WAIT, D_CULT_WITNESS, D_CULT_SAFETY,
       D_CULT_BACKUP, D_CULT_AFTER, D_CULT_NOT_YET, D_CULT_WITNESS_AFTER,
       D_CULT_SAFE_AGAIN, D_CULT_PRE_WIN, D_CULT_DURING_WIN, D_CULT_POST_WIN,
       D_CULT_RUN, D_CULT_LOSE, D_CULT_NOTES_1, D_CULT_NOTES_2,
       D_CULT_NOTES_3, D_CULT_NOTES_4,
       D_MUSIC, D_MUSIC_AGAIN, D_ART, D_ART_AGAIN, D_ART_DAY, D_ART_LAST_LOCKED,
       D_NEUTRAL_START, D_MEDICAL_NOTICE, D_DOCTOR_FIRST,
       D_DOCTOR_RETURN, D_DOCTOR_LAST, D_NEUTRAL_PAINT, D_NEUTRAL_END,
       D_GOOD_END, D_DAY_NOTES, D_CLINIC_LOCKED,
       D_NOTE_VISIT, D_NOTE_FOLLOWUP, D_NOTE_CANVAS,
       D_DAY_MARA, D_DAY_MARA_AFTER, D_DAY_LETTER, D_DAY_LETTER_AFTER,
       D_DAY_CLERK, D_DAY_CLERK_AFTER, D_DAY_MUSIC, D_DAY_MUSIC_AFTER,
       D_DAY_PAINT, D_DAY_PAINT_AFTER, D_DAY_RECAP, D_DAY_RECAP_AFTER,
       D_ACT_MARA_TALK, D_ACT_LETTER_KEEP, D_ACT_CLERK_JOKE,
       D_ACT_MUSIC_LISTEN, D_ACT_ART_REST,
       D_ACT_MARA_CLOSE, D_ACT_MUSIC_PROGRESS, D_ACT_ART_PROGRESS,
       D_PROJECT_SONG_2, D_PROJECT_SONG_3, D_PROJECT_SONG_DONE,
       D_PROJECT_ART_2, D_PROJECT_ART_3, D_PROJECT_ART_DONE,
       D_MARA_MORNING, D_MARA_CLOSING, D_CLERK_CLOSING,
       D_MARA_GALLERY, D_CLERK_SONG,
       D_BAD_LETTER, D_BAD_CLERK, D_BAD_MUSIC, D_BAD_SERVER,
       D_MARA_MISSING, D_MARA_ABSENT, D_RESCUE_WITNESS,
       D_RESCUE_TRACE, D_RESCUE_SAVED, D_RESCUE_LOST,
       D_RESCUE_AFTERSHOCK, D_BAD_END, D_BAD_END_SAVED, D_RESCUE_NOTES_1,
       D_RESCUE_NOTES_2, D_RESCUE_NOTES_3, D_RESCUE_NOTES_SAVED,
       D_BUS_RESCUE, D_BAD_WARNING_2, D_CLINIC_RESCUE,
       D_RESCUE_WITNESS_TRUST, D_RESCUE_SAVED_TRUST,
       D_RESCUE_SAVED_ART,
       D_HOPE_REALIZE, D_HOPE_PHONE_WAIT, D_HOPE_PHONE_LATE,
       D_HOPE_WITNESS, D_HOPE_TERMINAL, D_HOPE_DOOR,
       D_HOPE_NOTE_WAIT, D_HOPE_NOTE_LATE,
       D_MARA_CONCERN, D_MARA_WARNING, D_BAD_MUSIC_MASTER,
       D_GOOD_END_SCARS, D_RESCUE_LOST_REMOTE,
       D_SHIFT_EARLY, D_SHIFT_DONE, D_SHIFT_REPEAT,
       D_SHIFT_TIRED, D_SHIFT_PAUSED,
       D_DOCTOR_RETURN_ALONE,
       D_LOOK_POTHEADS, D_LOOK_CANAL, D_LOOK_MARKET,
       D_LOOK_NORTH, D_LOOK_STREET,
       D_PROJECT_SONG_START, D_PROJECT_ART_START, D_MARA_PREBUS_WALK,
       D_MARA_PREBUS_TALK, D_MARA_MASTER, D_MARA_CREATIONS,
       D_BUS_READY_LEGACY, D_BUS_STAY_OPEN, D_DOCTOR_LAST_ALONE,
       D_NB_0_INTRO,
       D_NB_0_LEFT,
       D_NB_0_RIGHT,
       D_NB_1_INTRO,
       D_NB_1_LEFT,
       D_NB_1_RIGHT,
       D_NB_2_INTRO,
       D_NB_2_LEFT,
       D_NB_2_RIGHT,
       D_NB_3_INTRO,
       D_NB_3_LEFT,
       D_NB_3_RIGHT,
       D_NB_4_INTRO,
       D_NB_4_LEFT,
       D_NB_4_RIGHT,
       D_NB_5_INTRO,
       D_NB_5_LEFT,
       D_NB_5_RIGHT,
       D_NB_6_INTRO,
       D_NB_6_LEFT,
       D_NB_6_RIGHT,
       D_NB_7_INTRO,
       D_NB_7_LEFT,
       D_NB_7_RIGHT,
       D_NB_8_INTRO,
       D_NB_8_LEFT,
       D_NB_8_RIGHT,
       D_NB_9_INTRO,
       D_NB_9_LEFT,
       D_NB_9_RIGHT,
       D_NB_10_INTRO,
       D_NB_10_LEFT,
       D_NB_10_RIGHT,
       D_NB_11_INTRO,
       D_NB_11_LEFT,
       D_NB_11_RIGHT,
       D_NB_12_INTRO,
       D_NB_12_LEFT,
       D_NB_12_RIGHT,
       D_NB_13_INTRO,
       D_NB_13_LEFT,
       D_NB_13_RIGHT, D_MARA_MASTER_SHARED,
       D_SHOW_PLAN,D_SHOW_POSTER_BLUE,D_SHOW_POSTER_WARM,D_SHOW_REHEARSED,
       D_SHOW_FRAME_BLACK,D_SHOW_FRAME_WOOD,D_SHOW_SETUP,D_SHOW_PLAY,D_SHOW_ARCHIVE,
       D_NB_RETURN_0_0,
       D_NB_RETURN_0_1,
       D_NB_RETURN_1_0,
       D_NB_RETURN_1_1,
       D_NB_RETURN_2_0,
       D_NB_RETURN_2_1,
       D_NB_RETURN_3_0,
       D_NB_RETURN_3_1,
       D_NB_RETURN_4_0,
       D_NB_RETURN_4_1,
       D_NB_RETURN_5_0,
       D_NB_RETURN_5_1,
       D_NB_RETURN_6_0,
       D_NB_RETURN_6_1,
       D_NB_RETURN_7_0,
       D_NB_RETURN_7_1,
       D_NB_RETURN_8_0,
       D_NB_RETURN_8_1,
       D_NB_RETURN_9_0,
       D_NB_RETURN_9_1,
       D_NB_RETURN_10_0,
       D_NB_RETURN_10_1,
       D_NB_RETURN_11_0,
       D_NB_RETURN_11_1,
       D_NB_RETURN_12_0,
       D_NB_RETURN_12_1,
       D_NB_RETURN_13_0,
       D_NB_RETURN_13_1,D_CITY_EAST,D_CITY_CIVIC,D_CITY_PARK,D_CITY_CANAL,D_CITY_PLAZA };
_Static_assert(D_CITY_PLAZA<256,"dialogue ID width");
#define SCRIPT(name, ...) static const StoryPage name[] = { __VA_ARGS__ }
/* The banana-sign shift is practical income, not Danny's calling. He is
   grateful for the stability without pretending to enjoy mascot work. */
SCRIPT(d_open,
 {"DANNY", "Another afternoon", "in the banana suit."},
 {"DANNY", "I don't love this gig.", "But it pays the rent."},
 {"DANNY", "That counts for a lot.", "I'm grateful it does."},
 {"DANNY", "After hours, I get", "to make my own art."},
 {"DANNY", "Last bus tonight.", "Clinic called again."},
 {"DANNY", "I missed their calls.", "I'm scared of the news."},
 {"DANNY", "First, the letter.", "Then BORE. Then Mara."});
SCRIPT(d_letter,
 {"OLD LETTER", "I left the porch", "light on for you."},
 {"OLD LETTER", "The bill went up.", "I left it on anyway."},
 {"DANNY", "I left without a", "proper goodbye."},
 {"DANNY", "I'll take this proof", "to BORE. Then reply."});
SCRIPT(d_letter_again,
 {"DANNY", "That little letter", "weighs a whole ton."});
SCRIPT(d_clerk_locked,
 {"BORE CLERK", "To verify your name", "I need an old address."},
 {"BORE CLERK", "A letter from home", "will do. West street."});
SCRIPT(d_clerk,
 {"BORE CLERK", "Your letter proves", "your old address."},
 {"DANNY", "So I exist?", "That's a relief."},
 {"BORE CLERK", "STAMPED. Your name", "is on the record."},
 {"DANNY", "And the game music", "I left in storage?"},
 {"BORE CLERK", "Music desk. East plaza.", "Try the basic studio."});
SCRIPT(d_clerk_again,
 {"BORE CLERK", "Do not fold the", "universe in half."},
 {"BORE CLERK", "The warranty is", "very specific."});
SCRIPT(d_mara_locked,
 {"MARA", "You're leaving?", "Without saying why?"},
 {"MARA", "Settle your records", "at BORE. Then talk."});
SCRIPT(d_mara,
 {"MARA", "You only joke like", "that when scared."},
 {"DANNY", "The clinic called.", "I didn't answer."},
 {"MARA", "You can leave. Just", "don't vanish on me."},
 {"DANNY", "I'll call before", "I take the bus."},
 {"MARA", "And save that song.", "You still owe me one."});
SCRIPT(d_mara_again,
 {"MARA", "Do you still have", "time for a walk?"},
 {"DANNY", "I do now.", "No rush today."});
/* One or two deliberately harmful choices affect Mara's trust *before*
   disappearance. Neither line presumes which site the player damaged. */
SCRIPT(d_mara_concern,
 {"MARA", "You've been talking", "around something."},
 {"DANNY", "I hurt someone's", "trust. I know."},
 {"MARA", "Tell me the truth.", "I can listen."},
 {"DANNY", "I have to start", "by not hiding."});
SCRIPT(d_mara_warning,
 {"MARA", "People are getting", "hurt by these edits."},
 {"DANNY", "I called them small", "until they weren't."},
 {"MARA", "Stop making others", "pay for your fear."},
 {"DANNY", "You're right.","No more shortcuts."});
SCRIPT(d_bus_locked,
 {"DRIVER", "Last bus tonight.", "Seats still open."},
 {"DRIVER", "Your letter, stamp", "and Mara come first."});
SCRIPT(d_bus_ready,
 {"DRIVER", "Your stamp checks out.", "One seat. One choice."},
 {"DRIVER", "Can you leave the", "unfinished work here?"},
 {"DANNY", "And the clinic call", "I keep avoiding?"});
SCRIPT(d_end_go,
 {"DANNY", "The city got small", "behind the window."},
 {"DANNY", "I didn't lose it.", "I brought it with me."},
 {"DRIVER", "Mind the gap in", "the space-time fare."},
 {"THE END", "Tomorrow is still", "a place to begin."});
SCRIPT(d_end_stay,
 {"DANNY", "I stayed. The bus", "left without a fuss."},
 {"MARA", "Good. The old cafe", "needs a new sign."},
 {"DANNY", "I call it THE END.", "We're open Tuesday."},
 {"THE END", "Tomorrow came on", "foot after all."});
SCRIPT(d_journal_0,
 {"POCKET NOTEBOOK", "1 LETTER / WEST", "2 BORE CLERK / CENTER"},
 {"DANNY", "Then tell Mara why", "I am afraid to go."});
SCRIPT(d_journal_1,
 {"POCKET NOTEBOOK", "LETTER FOUND. NEXT:", "BORE CLERK / CENTER"});
SCRIPT(d_journal_2,
 {"POCKET NOTEBOOK", "STAMPED. NEXT:", "MARA / WEST CAFE"});
SCRIPT(d_journal_3,
 {"POCKET NOTEBOOK", "BUS IS OPEN. EXTRAS:", "MUSIC / ROOF PAINT"},
 {"DANNY", "START then L for help.", "Right page tracks goals."},
 {"DANNY", "East plaza: music desk.", "A again for BASIC."});
SCRIPT(d_nothing,
 {"DANNY", "Nothing here for me.", "Where was I headed?"},
 {"POCKET NOTEBOOK", "The gold mark guides me.", "START opens the map."});
SCRIPT(d_look_potheads,
 {"DANNY", "Potheads. The suit", "still smells of rain."},
 {"DANNY", "I can be grateful", "and still want more."});
SCRIPT(d_look_canal,
 {"DANNY", "The canal keeps", "its own kind of time."},
 {"DANNY", "A hundred windows", "in every ripple."});
SCRIPT(d_look_market,
 {"DANNY", "Someone patched the", "awning by hand."},
 {"DANNY", "Not every repair", "has to look new."});
SCRIPT(d_look_north,
 {"DANNY", "All these lit rooms.", "Lives I never knew."},
 {"DANNY", "My song could reach", "one of them someday."});
SCRIPT(d_look_street,
 {"DANNY", "The old paving", "holds the rain."},
 {"DANNY", "I should keep going.", "One block at a time."});
SCRIPT(d_return,
 {"DANNY", "The city is still", "here. So am I."});
/* Entirely fictional setting and people. The harm is serious; the bureaucracy is not. */
SCRIPT(d_cult_find,
 {"TERMINAL", "MIRROR ORCHARD", "WELCOME / RULES: 900"},
 {"DANNY", "The art server has", "a ministry of fonts."},
 {"UNKNOWN USER", "The jokes hide harm.", "Please take care."},
 {"DANNY", "A real person asked", "for help. I can try."});
SCRIPT(d_cult_logs,
 {"TERMINAL", "Screenshots are not", "a rescue plan."},
 {"DANNY", "Find the witness.", "Ask what THEY need."});
SCRIPT(d_cult_wait,
 {"TERMINAL", "Talk to the witness", "before posting."});
SCRIPT(d_cult_witness,
 {"WITNESS", "I kept the logs.", "Please don't post me."},
 {"DANNY", "Your name stays out.", "What would help?"},
 {"WITNESS", "Help my friend get", "somewhere safe."},
 {"DANNY", "No spectacle. We", "listen and help."});
SCRIPT(d_cult_safety,
 {"NEIGHBOR", "The door is open.", "They chose to come."},
 {"NEIGHBOR", "Trusted people know.", "No cameras here."},
 {"DANNY", "One person is safe.", "That is not small."});
SCRIPT(d_cult_backup,
 {"TERMINAL", "Evidence preserved.", "Reports submitted."},
 {"TERMINAL", "Server reopened as", "MIRROR ORCHARD 2."},
 {"DANNY", "We didn't erase it.", "We helped someone."});
SCRIPT(d_cult_after,
 {"TERMINAL", "Server still active.", "New logo. Same mess."},
 {"DANNY", "They can't erase", "who got out."});
SCRIPT(d_cult_not_yet,
 {"WITNESS", "Please read the", "message first."});
SCRIPT(d_cult_witness_after,
 {"WITNESS", "Thank you for not", "making me a meme."});
SCRIPT(d_cult_safe_again,
 {"NEIGHBOR", "The kettle is on.", "No server rules."});
SCRIPT(d_cult_pre_win,
 {"DANNY", "The reply squad", "muted itself. Huh."},
 {"HECKLER", "YOU WON A POINT.", "NOT THE INTERNET."});
SCRIPT(d_cult_during_win,
 {"DANNY", "A loud argument", "isn't real help."},
 {"HECKLER", "My mod badge has", "emotional dents."});
SCRIPT(d_cult_post_win,
 {"HECKLER", "We changed names!", "YOU CAN'T WIN!"},
 {"DANNY", "I wasn't trying to", "win a website."},
 {"DANNY", "The people we helped", "still matter."});
SCRIPT(d_cult_run,
 {"DANNY", "I can walk away.", "No need to win this."});
SCRIPT(d_cult_lose,
 {"DANNY", "Too much noise.", "I'll try another way."},
 {"DANNY", "I can try again later.", "Or keep walking."});
SCRIPT(d_cult_notes_1,
 {"POCKET NOTEBOOK", "MIRROR ORCHARD:", "Find the witness."});
SCRIPT(d_cult_notes_2,
 {"POCKET NOTEBOOK", "Witness contacted.", "Find the safe house."});
SCRIPT(d_cult_notes_3,
 {"POCKET NOTEBOOK", "Someone got safe.", "Back up the truth."});
SCRIPT(d_cult_notes_4,
 {"POCKET NOTEBOOK", "People helped. Logs", "saved. Server lives."});
SCRIPT(d_music,
 {"ARCHIVE TAPE", "BORE REEL 07", "ROUGH TRACK INTACT."},
 {"DANNY", "I wrote this when", "I still called home."},
 {"DANNY", "Mara always liked", "this silly song."},
 {"ARCHIVE TAPE", "COPY REGISTERED.", "YOUR NAME IS ON IT."},
 {"ARCHIVE TAPE", "A AGAIN: MUSIC DESK.", "CHOOSE BASIC STUDIO."},
 {"DANNY", "Edit notes. Hear them.", "START saves a take."},
 {"ARCHIVE TAPE", "FOUR TAKES TO UNLOCK", "THE ADVANCED STUDIO."});
SCRIPT(d_music_again,
 {"DANNY", "The tape survived.", "Not the cassette case."});
SCRIPT(d_art,
 {"DANNY", "Mara's cafe is", "down there somewhere."},
 {"DANNY", "I'll paint this view.", "A AGAIN FOR A SESSION."});
SCRIPT(d_art_again,
 {"DANNY", "First colors are down.", "It needs more time."});
SCRIPT(d_art_day,
 {"DANNY", "One more layer of", "paint. One more day."},
 {"DANNY", "The city keeps on", "being a city."});
SCRIPT(d_art_last_locked,
 {"DANNY", "The doctors haven't", "finished talking."},
 {"DANNY", "My canvas can wait", "a little longer."});
SCRIPT(d_neutral_start,
 {"DAY ONE", "I reached the bus.", "My phone rang."},
 {"CLINIC", "We need to discuss", "your earlier scans."},
 {"DANNY", "I let the bus go.", "I have to face this."},
 {"DANNY", "Mara was right.", "I can't vanish now."},
 {"DANNY", "I'll spend today here.", "The clinic calls later."});
SCRIPT(d_medical_notice,
 {"DAY TWO", "The clinic left", "another message."},
 {"DANNY", "I heard them this", "time. I'll go in."},
 {"CLINIC", "Please come in.", "We need to talk."});
SCRIPT(d_doctor_first,
 {"DOCTOR", "Earlier scans raise", "real concerns."},
 {"DOCTOR", "We need more tests.", "No fixed timetable."},
 {"DANNY", "I've been dodging", "your calls all week."},
 {"DOCTOR", "Bring someone you", "trust next time."});
SCRIPT(d_doctor_return,
 {"DAY THREE", "Your follow-up", "is scheduled today."},
 {"MARA", "I'll come in.", "Or wait outside."},
 {"DANNY", "Come in. I don't", "want to hear alone."});
/* When the search has ended without verified recovery, the doctor still
   needs Danny, but Mara cannot promise to accompany him. Her condition
   is deliberately UNKNOWN, not assumed fatal. */
SCRIPT(d_doctor_return_alone,
 {"DAY THREE", "The follow-up", "is still today."},
 {"DANNY", "No confirmed word", "about Mara. Not yet."},
 {"DANNY", "I'll see the doctor", "and keep looking."});
SCRIPT(d_doctor_last,
 {"DOCTOR", "These tests confirm", "advanced cancer."},
 {"DOCTOR", "We can plan care", "and ease the pain."},
 {"MARA", "I'll stay with you.", "If you want."},
 {"DANNY", "For now. Later,", "I want to paint."});
SCRIPT(d_neutral_paint,
 {"ROOFTOP", "Danny came back", "to the cafe skyline."},
 {"DANNY", "I told Mara where", "I'd be this time."},
 {"DANNY", "Let the last line", "be a little light."});
SCRIPT(d_neutral_end,
 {"NARRATOR", "The last brush fell", "on the quiet roof."},
 {"NARRATOR", "Danny never stood", "up again."},
 {"MARA", "He finally called.", "I wish we'd had more."},
 {"NEUTRAL END", "He left us a sky", "we could recognize."});
SCRIPT(d_good_end,
 {"YEARS LATER", "The bus was only", "the beginning."},
 {"MARA", "He called. He faced", "the clinic too."},
 {"BORE CLERK", "His tape and canvas", "were kept safe."},
 {"RECORD STORE", "Mara released them", "after he was gone."},
 {"RECORD STORE", "The songs reached", "millions of people."},
 {"GOOD END", "He never heard", "their final chorus."});
/* A positive artistic legacy cannot erase harms from optional bad checks.
   The title of the ending remains unchanged; its moral context does not. */
SCRIPT(d_good_end_scars,
 {"YEARS LATER", "Danny lived beyond", "that last bus ride."},
 {"MARA", "I kept the tapes.", "Not his excuses."},
 {"BORE CLERK", "The archive kept", "the original copies."},
 {"RECORD STORE", "After he was gone,", "the songs took off."},
 {"MARA", "He made real music.", "He did harm too."},
 {"GOOD END", "One did not erase", "the other."});
SCRIPT(d_day_notes,
 {"POCKET NOTEBOOK", "The bus can wait.", "The roof cannot."},
 {"POCKET NOTEBOOK", "Read, walk, paint.", "The day continues."},
 {"POCKET NOTEBOOK", "At familiar places", "A choose  B cancel."},
 {"POCKET NOTEBOOK", "E energy  S spark", "B bond  W work."});
SCRIPT(d_clinic_locked,
 {"CLINIC", "OPEN BY APPOINTMENT", "NEXT NOTICE PENDING."});
SCRIPT(d_note_visit,
 {"POCKET NOTEBOOK", "CLINIC ON EAST ST.", "They asked for me."},
 {"DANNY", "I should see them", "before I paint."});
SCRIPT(d_note_followup,
 {"POCKET NOTEBOOK", "WAIT FOR THE NEXT", "DOCTOR APPOINTMENT."},
 {"DANNY", "Mara offered to", "walk there with me."});
SCRIPT(d_note_canvas,
 {"POCKET NOTEBOOK", "LAST ROOFTOP CANVAS", "NORTH PLAZA STAIRS."},
 {"DANNY", "The last detail is", "a corner of sky."});

/* Small, world-grounded scenes, staged in the existing cinematic sets. */
SCRIPT(d_day_mara,
 {"MARA", "Want the long way", "past the old shops?"},
 {"DANNY", "My legs complain.", "My mouth says yes."},
 {"MARA", "Then we can walk", "as slowly as we like."});
SCRIPT(d_day_mara_after,
 {"MARA", "I can walk with", "you for a while."},
 {"DANNY", "Let's pass the shop", "with the bad sign."},
 {"MARA", "Still spelled wrong.", "Some things endure."});
SCRIPT(d_day_letter,
 {"DANNY", "The letter smells", "of old cupboards."},
 {"DANNY", "I owe a reply.", "Ink should be enough."},
 {"OLD LETTER", "A small note placed", "behind the frame."});
SCRIPT(d_day_letter_after,
 {"DANNY", "There was a lamp", "in that old house."},
 {"DANNY", "Maybe my painting", "can hold its light."},
 {"OLD LETTER", "He folded the page", "and kept it near."});
SCRIPT(d_day_clerk,
 {"BORE CLERK", "Form W: proof of", "an ordinary weekday."},
 {"DANNY", "I had breakfast.", "Does that count?"},
 {"BORE CLERK", "Only if you kept", "the receipt."});
SCRIPT(d_day_clerk_after,
 {"BORE CLERK", "Today's stamp is", "not a deadline."},
 {"DANNY", "That's the nicest", "thing you've said."},
 {"BORE CLERK", "Please don't file", "a compliment."});
SCRIPT(d_day_music,
 {"ARCHIVE TAPE", "A melody from", "the old BORE games."},
 {"DANNY", "This chord should", "be one step lower."},
 {"DANNY", "There. Now it's", "awkward on purpose."});
SCRIPT(d_day_music_after,
 {"ARCHIVE TAPE", "A second take.", "The reel turns."},
 {"DANNY", "Leave the quiet", "part of the song."},
 {"DANNY", "A pause can say", "a whole lot."});
SCRIPT(d_day_paint,
 {"ROOFTOP", "The paint catches", "the afternoon sun."},
 {"DANNY", "Another thin line", "behind the skyline."},
 {"DANNY", "It's coming along.", "So am I, maybe."});
SCRIPT(d_day_paint_after,
 {"ROOFTOP", "The city changes", "while he paints."},
 {"DANNY", "Leave some space", "for tomorrow's sky."},
 {"DANNY", "No need to fill", "every corner yet."});
SCRIPT(d_day_recap,
 {"AN ORDINARY DAY", "A walk, a letter", "and a crooked tune."},
 {"DANNY", "I didn't solve", "the whole world."},
 {"DANNY", "I did live in", "a little of it."});
SCRIPT(d_day_recap_after,
 {"ANOTHER DAY", "The music played.", "The sky moved on."},
 {"MARA", "Tomorrow isn't", "a receipt to keep."},
 {"DANNY", "No. Just another", "morning, if it comes."});
/* The right-hand daily options restore energy rather than punishing fatigue. */
SCRIPT(d_act_mara_talk,
 {"MARA", "You do not have to", "be funny today."},
 {"DANNY", "Well, that saves", "my worst material."},
 {"MARA", "We can just sit", "and talk awhile."});
SCRIPT(d_act_letter_keep,
 {"DANNY", "Some memories are", "not letters to send."},
 {"OLD LETTER", "He kept the page", "where it could dry."},
 {"DANNY", "I still know the", "shape of that light."});
SCRIPT(d_act_clerk_joke,
 {"DANNY", "Is this the line", "for existing loudly?"},
 {"BORE CLERK", "Wrong queue. That", "one has a bell."},
 {"DANNY", "You laughed. I'll", "file that under joy."});
SCRIPT(d_act_music_listen,
 {"ARCHIVE TAPE", "An old BORE theme", "plays from your take."},
 {"DANNY", "The rest between", "notes is part of it."},
 {"DANNY", "Maybe I can just", "listen this time."});
SCRIPT(d_act_art_rest,
 {"ROOFTOP", "Danny sets the", "brush beside him."},
 {"DANNY", "I could paint all", "the hours away."},
 {"DANNY", "Or watch the sky", "paint itself awhile."});
SCRIPT(d_act_mara_close,
 {"MARA", "You never have to", "perform for me."},
 {"DANNY", "Then let's watch", "the shops close."},
 {"MARA", "We can take our", "time getting home."});
SCRIPT(d_act_music_progress,
 {"ARCHIVE TAPE", "The new arrangement", "has taken shape."},
 {"DANNY", "That off-beat line", "is finally mine."},
 {"DANNY", "Keep a copy in", "the BORE archive."});
SCRIPT(d_act_art_progress,
 {"ROOFTOP", "The old skyline", "has more color now."},
 {"DANNY", "It's not perfect.", "It feels like mine."});

/* Projects continue between numbered-day interactions. Every milestone is
   a playable scene tied to the same physical workplace and BORE music. */
SCRIPT(d_project_song_2,
 {"ARCHIVE TAPE", "The bass finds the", "old crooked melody."},
 {"DANNY", "Whistler Man comes", "back in the gaps."},
 {"DANNY", "Two takes down.", "Next, shape the rhythm."});
SCRIPT(d_project_song_3,
 {"DANNY", "The rhythm is", "finally breathing."},
 {"ARCHIVE TAPE", "One final mix.", "Then it is ready."},
 {"DANNY", "Hear the whole take.", "Leave space to breathe."});
SCRIPT(d_project_song_done,
 {"ARCHIVE TAPE", "MASTER COPY SAVED.", "THE LAST BORE REEL."},
 {"DANNY", "The quiet parts", "made it in too."},
 {"DANNY", "Let someone else", "hear the whole song."},
 {"BORE ARCHIVE", "ADVANCED UNLOCKED.", "A AT THE MUSIC DESK."},
 {"BORE ARCHIVE", "ORDER, LYRICS, MIXER.", "EDIT WHEN YOU WANT."},
 {"DANNY", "I'll take it to Mara.", "She heard the rough one."});
SCRIPT(d_project_art_2,
 {"ROOFTOP", "Buildings rise over", "the painted horizon."},
 {"DANNY", "I know those", "windows by heart."});
SCRIPT(d_project_art_3,
 {"ROOFTOP", "A thin brush adds", "the street lights."},
 {"DANNY", "Almost the same", "as the view outside."});
SCRIPT(d_project_art_done,
 {"ROOFTOP", "THE OLD QUARTER", "FINISHED ON CANVAS."},
 {"DANNY", "I left one corner", "for the real sky."},
 {"DANNY", "That feels about", "right for a painting."});
/* Shops follow morning / evening rhythms while remaining reachable:
   schedules never hide story NPCs or disable their interactions. */
SCRIPT(d_mara_morning,
 {"MARA", "The cafe's opening.", "The kettle is loud."},
 {"DANNY", "So am I before", "coffee, allegedly."});
SCRIPT(d_mara_closing,
 {"MARA", "Evening's here.", "We can take a walk."},
 {"DANNY", "The city lights", "look less tired."});
SCRIPT(d_clerk_closing,
 {"BORE CLERK", "Five minutes until", "the stamps go home."},
 {"DANNY", "Does your stamp", "get paid overtime?"});
SCRIPT(d_mara_gallery,
 {"MARA", "You finished the", "rooftop picture!"},
 {"DANNY", "Just in time for", "the light to change."},
 {"MARA", "Let me see it", "without the frame."});
SCRIPT(d_clerk_song,
 {"BORE CLERK", "This recording", "is officially BORE."},
 {"DANNY", "I finally made", "a usable master."},
 {"BORE CLERK", "Filed under", "SONGS WORTH KEEPING."});

/* Four optional harmful decisions require deliberate B + RIGHT + A.
   Mara's fate is never shown or described explicitly if rescue fails. */
SCRIPT(d_bad_letter,
 {"DANNY", "I hid the reply", "when it mattered."},
 {"DANNY", "Nobody deserves", "to wait for a lie."});
SCRIPT(d_bad_clerk,
 {"BORE CLERK", "That forged record", "has a real owner."},
 {"DANNY", "I thought it was", "only paperwork."});
SCRIPT(d_bad_music,
 {"ARCHIVE TAPE", "EARLY DEMO ERASED.", "SESSION CREDITS LOST."},
 {"DANNY", "Other people made", "that track with me."});
/* Mastering the current recording does not retroactively recreate an
   erased *early* demo; a master and lost session are distinct artifacts. */
SCRIPT(d_bad_music_master,
 {"ARCHIVE TAPE", "EARLY TAKE ERASED.", "MASTER STILL SAFE."},
 {"DANNY", "A final mix can't", "replace that take."});
SCRIPT(d_bad_server,
 {"TERMINAL", "PRIVATE NAME POSTED.", "RETRACTING IS HARD."},
 {"DANNY", "I just made life", "harder for someone."});
SCRIPT(d_clinic_rescue,
 {"RECEPTION", "Your appointment", "can be rescheduled."},
 {"DANNY", "Mara is missing.", "I have to find her."},
 {"RECEPTION", "We'll keep your", "place for you."});
SCRIPT(d_bad_warning_2,
 {"DANNY", "Two small wrongs.", "They didn't stay small."},
 {"UNKNOWN TEXT", "MARA'S CAFE IS", "ON THEIR LIST."},
 {"POCKET NOTEBOOK", "TWO BAD CHECKS.", "STOP BEFORE THREE."});
SCRIPT(d_mara_missing,
 {"MARA'S CAFE", "Her chair is empty.", "The cup is untouched."},
 {"DANNY", "She would have left", "some kind of note."},
 {"PHONE MESSAGE", "A WITNESS SAW A VAN.", "NEAR MIRROR ORCHARD."},
 {"DANNY", "A lead, not proof.", "I'll ask in person."});
SCRIPT(d_mara_absent,
 {"MARA'S CAFE", "The kettle has", "gone cold again."},
 {"DANNY", "I'll check the trail", "before I guess."});
SCRIPT(d_rescue_witness,
 {"WITNESS", "I saw a van.", "Someone forced her in."},
 {"WITNESS", "I lost the plate.", "Try the terminal."},
 {"DANNY", "I won't put your", "name online."});
SCRIPT(d_rescue_trace,
 {"TERMINAL", "LAST VAN SIGNAL:", "OLD SAFE HOUSE."},
 {"TERMINAL", "VEHICLE LOCATION.", "NOT PROOF OF MARA."},
 {"DANNY", "A lead, not proof.", "I'm going there."});
SCRIPT(d_rescue_saved,
 {"MARA", "You found me.", "I heard your steps."},
 {"DANNY", "I thought I'd be", "too late."},
 {"MARA", "Don't make a speech.", "Just help me stand."},
 {"DANNY", "I'm sorry.", "Let's get you home."});
SCRIPT(d_rescue_lost,
 {"SAFE HOUSE", "The light went out.", "No answer came."},
 {"UNKNOWN CALLER", "She will call you.", "Just give it a day."},
 {"DANNY", "You sound so sure.", "I need you right."},
 {"DANNY", "I'll keep the phone", "on. She can call."});
/* Fourth harm can occur from any desk, before Danny ever reaches the
   house. It must NOT teleport him to a safe-house failure scene. */
/* Shop scenes reuse the real Potheads set; these words are about making
   ends meet, never a claim Danny loves being a sandwich-board mascot. */
SCRIPT(d_shift_early,
 {"DANNY", "Already did my", "hours in the suit."},
 {"DANNY", "I can be grateful", "and still be tired."});
SCRIPT(d_shift_done,
 {"DANNY", "One more shift done.", "Rent will get paid."},
 {"DANNY", "Now I can get back", "to making something."});
SCRIPT(d_shift_repeat,
 {"DANNY", "I put my hours in.", "That\'s plenty today."});
SCRIPT(d_shift_tired,
 {"DANNY", "Too worn down for", "another sign shift."});
SCRIPT(d_shift_paused,
 {"DANNY", "The sign can wait.", "There are bigger things."});
SCRIPT(d_rescue_lost_remote,
 {"PHONE", "A lead went quiet.", "Mara is still gone."},
 {"UNKNOWN CALLER", "She will call you.", "Just give it a day."},
 {"DANNY", "You sound so sure.", "I need you right."},
 {"DANNY", "I'll keep the phone", "on. She can call."});
/* The unverified caller sounds reassuring, but the game does not portray
   an unreturned call as evidence of death or confirmed harm. */
SCRIPT(d_hope_realize,
 {"PHONE", "A day has passed.", "No return call."},
 {"UNKNOWN CALLER", "I passed on a rumor.", "I never checked."},
 {"WITNESS", "Mara still has not", "been recovered."},
 {"DANNY", "You sounded certain.", "I wanted you right."},
 {"DANNY", "The trail ends here.", "Not her story."},
 {"DANNY", "I'll keep the line", "open. Just in case."});
SCRIPT(d_hope_phone_wait,
 {"PHONE", "CALL STILL PENDING.", "No reply yet."},
 {"DANNY", "They sounded sure.", "I needed that."});
SCRIPT(d_hope_phone_late,
 {"PHONE", "NO VERIFIED UPDATE.", "Calls unanswered."},
 {"DANNY", "I know a promise", "isn't proof."},
 {"DANNY", "But keep ringing.", "I'll be here."});
SCRIPT(d_hope_witness,
 {"WITNESS", "They promised news.", "I can't confirm it."},
 {"WITNESS", "No signed recovery", "report exists."});
SCRIPT(d_hope_terminal,
 {"TERMINAL", "LOCATION UNKNOWN.", "NO VERIFIED TRACE."},
 {"DANNY", "The last van ping", "wasn't Mara's voice."});
SCRIPT(d_hope_door,
 {"SAFE HOUSE", "The door is shut.", "The light is off."},
 {"DANNY", "I still have no", "answer. Not yet."});
SCRIPT(d_hope_note_wait,
 {"POCKET NOTEBOOK", "AWAIT MARA'S CALL.", "PROMISE UNVERIFIED."},
 {"DANNY", "Keep the phone on.", "No guessing."});
SCRIPT(d_hope_note_late,
 {"POCKET NOTEBOOK", "MARA NOT RECOVERED.", "NO VERIFIED UPDATE."},
 {"DANNY", "Hope isn't proof.", "Neither is silence."});
SCRIPT(d_rescue_aftershock,
 {"DANNY", "I crossed that line", "again, on purpose."},
 {"MARA", "I'm home. I'm safe.", "It isn't all right."},
 {"DANNY", "You were right", "to expect better."});
SCRIPT(d_bad_end,
 {"NARRATOR", "There was no", "certain word of Mara."},
 {"DANNY", "I waited because", "someone said she'd call."},
 {"NARRATOR", "No answer came.", "That settled nothing."},
 {"BAD END", "The line stayed open", "past the last frame."});
SCRIPT(d_bad_end_saved,
 {"NARRATOR", "Mara returned home.", "She was safe."},
 {"MARA", "I was found. That", "didn't fix everything."},
 {"DANNY", "I asked for trust", "and spent it again."},
 {"BAD END", "Not every second", "chance gets a third."});
SCRIPT(d_rescue_notes_1,
 {"POCKET NOTEBOOK", "MARA IS MISSING.", "WITNESS: WEST CAFE."});
SCRIPT(d_rescue_notes_2,
 {"POCKET NOTEBOOK", "TERMINAL: EAST", "MIRROR ORCHARD."});
SCRIPT(d_rescue_notes_3,
 {"POCKET NOTEBOOK", "SAFE HOUSE: NORTH", "OPEN THE DOOR."});
SCRIPT(d_rescue_notes_saved,
 {"POCKET NOTEBOOK", "MARA IS SAFE.", "BAD CHECKS REMAIN."},
 {"DANNY", "One more bad choice", "could end it."});
SCRIPT(d_bus_rescue,
 {"DRIVER", "Back for a ticket?", "There is still time."},
 {"DANNY", "Mara made it home.", "That mattered more."},
 {"DRIVER", "So did what you did", "before she vanished."});
/* Good marks alter the rescue itself: deeds change what NPCs know and say.
   These are pure consequences of existing progress, never grind gates. */
SCRIPT(d_rescue_witness_trust,
 {"WITNESS", "You helped us with", "Mirror Orchard."},
 {"WITNESS", "I trust you with", "Mara's trail now."},
 {"WITNESS", "The terminal has", "the last known stop."});
SCRIPT(d_rescue_saved_trust,
 {"MARA", "You kept looking.", "I heard your steps."},
 {"DANNY", "I should've listened", "before all this."},
 {"MARA", "We can talk later.", "Please take me home."});
SCRIPT(d_rescue_saved_art,
 {"MARA", "You found me.", "I want to go home."},
 {"DANNY", "The roof painting", "is waiting for you."},
 {"MARA", "I'd like to see it", "when we're ready."});
/* First-evening scenes join the existing creative and relationship systems
   without requiring Danny to enter a medical crisis to play them. */
SCRIPT(d_project_song_start,
 {"DANNY", "Here is the melody.", "It still sounds rough."},
 {"DANNY", "I'll build a version", "someone can keep."},
 {"ARCHIVE TAPE", "TAKE ONE SAVED.", "THREE STEPS REMAIN."},
 {"DANNY", "I can try new notes", "at the basic studio."});
SCRIPT(d_project_art_start,
 {"DANNY", "Start with the sky.", "The rest can wait."},
 {"DANNY", "The cafe lights will", "go in later."},
 {"ROOFTOP", "FIRST LAYER DRYING.", "THREE STEPS REMAIN."});
SCRIPT(d_mara_prebus_walk,
 {"MARA", "Let's take the long", "way past Potheads."},
 {"DANNY", "I won't miss the suit.", "I will miss you."},
 {"MARA", "Call me even if", "you don't feel brave."},
 {"DANNY", "All right. No more", "vanishing acts."});
SCRIPT(d_mara_prebus_talk,
 {"MARA", "You can be honest.", "We have time."},
 {"DANNY", "What if the clinic", "has bad news for me?"},
 {"MARA", "Then we make a plan.", "Not a performance."},
 {"DANNY", "I needed to hear", "that from someone."});
SCRIPT(d_mara_master,
 {"MARA", "You finished our song.", "Can I keep a copy?"},
 {"DANNY", "Yes. BORE has one.", "You should have one."},
 {"MARA", "Then I can play it", "when the cafe closes."},
 {"MARA", "Not another promise.", "Something you finished."},
 {"DANNY", "I'll keep making songs.", "Even after tonight."},
 {"MARA", "This one stays here.", "Thank you."});
SCRIPT(d_mara_creations,
 {"MARA", "The tape and painting.", "You really did it."},
 {"DANNY", "I wanted you to have", "more than a goodbye."},
 {"MARA", "Keep making things.", "And keep calling."});
SCRIPT(d_bus_ready_legacy,
 {"DRIVER", "Your stamp checks out.", "One seat. One choice."},
 {"DRIVER", "You kept your tape", "and painted the view."},
 {"DANNY", "I'll call Mara.", "I'll face the clinic."});
SCRIPT(d_bus_stay_open,
 {"DRIVER", "No rush to board.", "Come back when ready."},
 {"DANNY", "A little more time.", "I know where to go."},
 {"DANNY", "Mara, the reel,", "and the rooftop."},
 {"DANNY", "Maybe a little show.","START, A: CAFE SHOW."});
SCRIPT(d_doctor_last_alone,
 {"DOCTOR", "These tests confirm", "advanced cancer."},
 {"DOCTOR", "We can plan care", "and ease the pain."},
 {"DANNY", "I wish Mara were here.", "I still have no news."},
 {"DOCTOR", "We can talk more.", "You don't need to rush."});
SCRIPT(d_nb_0_intro,
 {"ELSIE", "That coat again?", "The seam is coming out."},
 {"DANNY", "It used to be my best.", "I can spare a moment."});
SCRIPT(d_nb_0_left,
 {"ELSIE", "Hold the cloth taut.", "There. A proper stitch."},
 {"DANNY", "I thought it was done.", "It still has some life."});
SCRIPT(d_nb_0_right,
 {"DANNY", "Here are your towels.", "You take care of us."},
 {"ELSIE", "And you came back.", "That's what matters."});
SCRIPT(d_nb_1_intro,
 {"PHOTOGRAPHER", "Found an old negative.", "You stood by the cafe."},
 {"DANNY", "I remember that day.", "What should it say?"});
SCRIPT(d_nb_1_left,
 {"DANNY", "Put my name under it.", "And the date, please."},
 {"PHOTOGRAPHER", "A name and a place.", "Now it belongs here."});
SCRIPT(d_nb_1_right,
 {"DANNY", "Leave this one blank.", "Let the faces speak."},
 {"PHOTOGRAPHER", "A picture can live", "without an explanation."});
SCRIPT(d_nb_2_intro,
 {"DANNY", "This stair used to go", "up to a water tower."},
 {"DANNY", "The city's still here.", "One good view left."});
SCRIPT(d_nb_2_left,
 {"DANNY", "Keep the rooftops low.", "Let the sky do the work."},
 {"DANNY", "A small study for", "the painting upstairs."});
SCRIPT(d_nb_2_right,
 {"DANNY", "Rain on the rail.", "A tram somewhere far."},
 {"DANNY", "There's a rhythm here.", "I could use that."});
SCRIPT(d_nb_3_intro,
 {"REPAIRMAN", "Old radio. Dead switch.", "Still a good speaker."},
 {"DANNY", "Used to take these", "apart for the noise."});
SCRIPT(d_nb_3_left,
 {"REPAIRMAN", "Try the contact now.", "That's the station!"},
 {"DANNY", "Not everything broken", "has to be thrown out."});
SCRIPT(d_nb_3_right,
 {"DANNY", "Use it for spare parts.", "Someone needs a knob."},
 {"REPAIRMAN", "I'll keep what works.", "Nothing wasted."});
SCRIPT(d_nb_4_intro,
 {"POST CLERK", "Last collection soon.", "Something to send?"},
 {"DANNY", "A note for home.", "Nothing complicated."});
SCRIPT(d_nb_4_left,
 {"DANNY", "I'm still in town.", "I miss the old place."},
 {"POST CLERK", "In the evening bag.", "It's on its way."});
SCRIPT(d_nb_4_right,
 {"DANNY", "Not ready to send it.", "I'll keep the words."},
 {"POST CLERK", "Paper can wait.", "People can call."});
SCRIPT(d_nb_5_intro,
 {"DANNY", "I knew this doorway.", "Used to wait out rain."},
 {"DANNY", "The light's on.", "Maybe someone remembers."});
SCRIPT(d_nb_5_left,
 {"NEIGHBOR", "Oh! You're the singer.", "You grew up fast."},
 {"DANNY", "I didn't mean to lose", "touch. Thanks for this."});
SCRIPT(d_nb_5_right,
 {"DANNY", "I'll leave a note.", "No need to interrupt."},
 {"DANNY", "Thank you for letting", "me wait in the rain."});
SCRIPT(d_nb_6_intro,
 {"VENDOR", "Rain got the crates.", "Closing in ten minutes."},
 {"DANNY", "I have a little time.", "What do you need?"});
SCRIPT(d_nb_6_left,
 {"VENDOR", "These two to the dry.", "That saves my evening."},
 {"DANNY", "Small enough to do.", "Big enough to matter."});
SCRIPT(d_nb_6_right,
 {"DANNY", "A bag of apples.", "Something for later."},
 {"VENDOR", "Take the bruised one", "too. Still sweet."});
SCRIPT(d_nb_7_intro,
 {"ARCHIVIST", "Found the loose reels.", "No names on two boxes."},
 {"DANNY", "I know that handwriting.", "Let me have a look."});
SCRIPT(d_nb_7_left,
 {"DANNY", "That one's rehearsals.", "Put it under BORE."},
 {"ARCHIVIST", "Now we can find it.", "A name is a beginning."});
SCRIPT(d_nb_7_right,
 {"DANNY", "This reel's case is", "split down the side."},
 {"ARCHIVIST", "A new sleeve, then.", "The recording stays."});
SCRIPT(d_nb_8_intro,
 {"DANNY", "The clinic garden.", "No one rushes here."},
 {"DANNY", "I don't have answers.", "I can plan a question."});
SCRIPT(d_nb_8_left,
 {"DANNY", "What do the scans show?", "What can we do next?"},
 {"DANNY", "Write it down now.", "Ask when I'm there."});
SCRIPT(d_nb_8_right,
 {"DANNY", "I'll sit for a while.", "Let the phone be quiet."},
 {"DANNY", "Rest isn't giving up.", "It's part of the day."});
SCRIPT(d_nb_9_intro,
 {"ELSIE", "The suit needs drying.", "What a silly uniform."},
 {"DANNY", "It paid my rent.", "It wasn't my whole life."});
SCRIPT(d_nb_9_left,
 {"DANNY", "Fold the sleeves in.", "Keep the badge on top."},
 {"ELSIE", "Someone else will use", "it for their first day."});
SCRIPT(d_nb_9_right,
 {"DANNY", "Thanks for the hours.", "For the steady checks."},
 {"ELSIE", "You're more than", "what you do for work."});
SCRIPT(d_nb_10_intro,
 {"OLD MAN", "Mind the wet bench.", "The dry end is yours."},
 {"DANNY", "I have a little bread.", "Or we could just sit."});
SCRIPT(d_nb_10_left,
 {"DANNY", "Here, split it in two.", "Nothing fancy."},
 {"OLD MAN", "Good bread and company.", "That's an afternoon."});
SCRIPT(d_nb_10_right,
 {"DANNY", "No need to fill", "every quiet moment."},
 {"OLD MAN", "That's the trouble.", "We forget how to sit."});
SCRIPT(d_nb_11_intro,
 {"DANNY", "A public phone booth.", "Still takes a coin."},
 {"DANNY", "Someone from home", "should hear from me."});
SCRIPT(d_nb_11_left,
 {"DANNY", "Hello. Yes, it's me.", "No, I'm still in town."},
 {"DANNY", "We don't need to fix", "everything tonight."});
SCRIPT(d_nb_11_right,
 {"DANNY", "It's Danny. I'm here.", "Call when you can."},
 {"DANNY", "A small message.", "Better than silence."});
SCRIPT(d_nb_12_intro,
 {"DANNY", "The canal carries", "the street lights."},
 {"DANNY", "There's a good shape.", "And trash by the rail."});
SCRIPT(d_nb_12_left,
 {"DANNY", "The rail cuts across", "the water like a line."},
 {"DANNY", "I'll save this view", "for the big canvas."});
SCRIPT(d_nb_12_right,
 {"DANNY", "Two bottles and paper.", "Into the bin they go."},
 {"DANNY", "It's still a good view.", "A little better now."});
SCRIPT(d_nb_13_intro,
 {"DRIVER", "The old shelter's map", "has no late bus route."},
 {"DANNY", "People still wait here.", "We could help them."});
SCRIPT(d_nb_13_left,
 {"DANNY", "Draw the late route.", "Put the stop name here."},
 {"DRIVER", "Now nobody has to", "guess in the rain."});
SCRIPT(d_nb_13_right,
 {"DANNY", "Leave the seat clear.", "I'll stand a while."},
 {"DRIVER", "You don't have to go", "before you're ready."});
SCRIPT(d_nb_return_0_0,
 {"ELSIE","The seam held up.","Even in this rain."},
 {"ELSIE","That coat has years","left if you mend it."},
 {"ELSIE","You saw the work suit.","Both needed care."});
SCRIPT(d_nb_return_0_1,
 {"ELSIE","The towels went out.","The shop is stocked."},
 {"ELSIE","You can stop here","without doing chores."},
 {"ELSIE","The suit is dry too.","A small day's work."});
SCRIPT(d_nb_return_1_0,
 {"PHOTOGRAPHER","Your name is printed.","The date is legible."},
 {"PHOTOGRAPHER","People remember you","without the costume."},
 {"PHOTOGRAPHER","The annex has labels.","Names help us find art."});
SCRIPT(d_nb_return_1_1,
 {"PHOTOGRAPHER","The print stays blank.","Just as you asked."},
 {"PHOTOGRAPHER","A stranger can see","their own story here."},
 {"PHOTOGRAPHER","The annex keeps reels.","Not all have names."});
SCRIPT(d_nb_return_2_0,
 {"DANNY","I kept the roof study.","The slope was right."},
 {"DANNY","Next time I paint","I'll leave more sky."},
 {"DANNY","The canal sketch adds","a different horizon."});
SCRIPT(d_nb_return_2_1,
 {"DANNY","That rail had a tune.","I tapped it walking."},
 {"DANNY","Maybe the pause is","the best part of it."},
 {"DANNY","The canal has its own","rhythm in the rain."});
SCRIPT(d_nb_return_3_0,
 {"REPAIRMAN","The radio still works.","No more dead switch."},
 {"REPAIRMAN","I left it on low.","Good for long shifts."},
 {"REPAIRMAN","The shelter needs","a little care too."});
SCRIPT(d_nb_return_3_1,
 {"REPAIRMAN","I used the old knob.","Another set plays."},
 {"REPAIRMAN","It didn't go to waste.","Thanks for the parts."},
 {"REPAIRMAN","The shelter could use","a light like this."});
SCRIPT(d_nb_return_4_0,
 {"POST CLERK","Your note went out.","It's on the route."},
 {"POST CLERK","You don't have to","send perfect words."},
 {"POST CLERK","The phone is nearby.","A voice helps too."});
SCRIPT(d_nb_return_4_1,
 {"POST CLERK","The note is yours.","Keep it until ready."},
 {"POST CLERK","No deadline here.","Come back any time."},
 {"POST CLERK","The phone booth lets","you leave words too."});
SCRIPT(d_nb_return_5_0,
 {"NEIGHBOR","I told the family.","Danny came to visit."},
 {"NEIGHBOR","Nobody asked for","a big explanation."},
 {"NEIGHBOR","The park is quiet.","Go sit if you want."});
SCRIPT(d_nb_return_5_1,
 {"NEIGHBOR","I found your note.","It's by the kettle."},
 {"NEIGHBOR","You didn't interrupt.","That was kind of you."},
 {"NEIGHBOR","The park bench is","there for a pause."});
SCRIPT(d_nb_return_6_0,
 {"VENDOR","The crates stayed dry.","You saved the apples."},
 {"VENDOR","I can close on time","for once this week."},
 {"VENDOR","The clinic garden","has room to breathe."});
SCRIPT(d_nb_return_6_1,
 {"VENDOR","Did the apples last?","I kept a good crate."},
 {"VENDOR","Next time take one","for the road."},
 {"VENDOR","The garden would be","a nice lunch spot."});
SCRIPT(d_nb_return_7_0,
 {"ARCHIVIST","We found that reel.","Your label held."},
 {"ARCHIVIST","It isn't famous yet.","But it can be heard."},
 {"ARCHIVIST","The photo shop keeps","memories like these."});
SCRIPT(d_nb_return_7_1,
 {"ARCHIVIST","The sleeve is fixed.","The tape is safe."},
 {"ARCHIVIST","Nobody sees the case","when the song plays."},
 {"ARCHIVIST","The photo shop knows","how paper survives."});
SCRIPT(d_nb_return_8_0,
 {"DANNY","I kept my questions.","No need to guess."},
 {"DANNY","I'll ask the doctor","when it's my turn."},
 {"DANNY","The market is busy.","Here I can slow down."});
SCRIPT(d_nb_return_8_1,
 {"DANNY","The bench is still dry.","A little luck today."},
 {"DANNY","It's all right to","have nothing to do."},
 {"DANNY","The market can wait.","So can the errands."});
SCRIPT(d_nb_return_9_0,
 {"ELSIE","The suit is folded.","The next kid has it."},
 {"ELSIE","You had a job here.","You have a life too."},
 {"ELSIE","Your coat looks ready","for the next chapter."});
SCRIPT(d_nb_return_9_1,
 {"ELSIE","I kept your thanks.","Not every worker does."},
 {"ELSIE","You made us laugh.","Don't forget that."},
 {"ELSIE","Your coat survived.","Keep taking care."});
SCRIPT(d_nb_return_10_0,
 {"OLD MAN","The bread was good.","Better with company."},
 {"OLD MAN","Come sit any time.","No need to bring food."},
 {"OLD MAN","The folks at the stoop","keep a light on too."});
SCRIPT(d_nb_return_10_1,
 {"OLD MAN","The bench is dry now.","Sit if you like."},
 {"OLD MAN","We can leave the","silence as it is."},
 {"OLD MAN","The stoop is quiet.","Another good pause."});
SCRIPT(d_nb_return_11_0,
 {"DANNY","I made that call.","It wasn't perfect."},
 {"DANNY","But they heard me.","That was enough."},
 {"DANNY","The post has a place","for the slower words."});
SCRIPT(d_nb_return_11_1,
 {"DANNY","I left my message.","They can call later."},
 {"DANNY","No need to wait here","for the bell to ring."},
 {"DANNY","The letter can carry","what I couldn't say."});
SCRIPT(d_nb_return_12_0,
 {"DANNY","The sketch is safe.","The rail is too dark."},
 {"DANNY","I'll lighten that line","when I get upstairs."},
 {"DANNY","The stair study shows","where the sky goes."});
SCRIPT(d_nb_return_12_1,
 {"DANNY","The bank looks better.","Just a little cleaner."},
 {"DANNY","I can see the water","between the lights."},
 {"DANNY","The stair has another","view of this quarter."});
SCRIPT(d_nb_return_13_0,
 {"DRIVER","The map is still up.","People read it now."},
 {"DRIVER","A few less questions","on the late route."},
 {"DRIVER","The repair shop keeps","old things useful too."});
SCRIPT(d_nb_return_13_1,
 {"DRIVER","The seat stayed clear.","Someone needed it."},
 {"DRIVER","Not every wait has","to be uncomfortable."},
 {"DRIVER","The repairman's radio","could fill the wait."});
SCRIPT(d_city_east,
 {"DANNY","These shops go on","for another block."},
 {"DANNY","I used to rush past.","Today I can walk."});
SCRIPT(d_city_civic,
 {"DANNY","The old public hall.","Windows like a piano."},
 {"DANNY","All those little rooms.","Everyone has a story."});
SCRIPT(d_city_park,
 {"DANNY","The park is quiet.","Rain on the benches."},
 {"DANNY","A whole city outside.","A little room to think."});
SCRIPT(d_city_canal,
 {"DANNY","Two bridges over water.","The lights drift slowly."},
 {"DANNY","I could paint this too.","Not every hour is work."});
SCRIPT(d_city_plaza,
 {"DANNY","The river plaza.","Room to see the sky."},
 {"DANNY","Maybe I'll bring Mara","here another evening."});
SCRIPT(d_mara_master_shared,
 {"MARA", "Your song is here.", "I kept the copy."},
 {"DANNY", "I can still change", "the arrangement."},
 {"MARA", "Make another version.", "The door is open."});
SCRIPT(d_show_plan,
 {"MARA","The cafe has a wall.","It has no good art."},
 {"DANNY","You want me to paint","a bigger coffee cup?"},
 {"MARA","Your painting. Your","song. One small show."},
 {"DANNY","No costume this time?","I think I can do it."},
 {"MARA","Ask the photo shop","for a poster first."},
 {"DANNY","Then finish my tape","and frame the canvas."});
SCRIPT(d_show_poster_blue,
 {"PHOTOGRAPHER","Blue hour it is.","Rain and window light."},
 {"DANNY","Put the street first.","Not my face this time."},
 {"PHOTOGRAPHER","One copy for the cafe.","One for your archive."},
 {"DANNY","Next, the music desk.","I need a real set."});
SCRIPT(d_show_poster_warm,
 {"PHOTOGRAPHER","Sunset it is.","A warmer little city."},
 {"DANNY","Make it look inviting.","Not like a sales pitch."},
 {"PHOTOGRAPHER","One copy for the cafe.","One for your archive."},
 {"DANNY","Next, the music desk.","I need a real set."});
SCRIPT(d_show_rehearsed,
 {"ARCHIVE TAPE","ALL FOUR PARTS READY.","REHEARSAL COMPLETE."},
 {"DANNY","I know what comes next.","I can play this now."},
 {"DANNY","The song needs company.","Back to the rooftop."},
 {"ARCHIVE TAPE","BRING YOUR PAINTING.","THEN VISIT THE ANNEX."});
SCRIPT(d_show_frame_black,
 {"DANNY","A black frame.","Let the colors speak."},
 {"DANNY","These little windows","all belonged to someone."},
 {"DANNY","I'll carry it down.","The annex has hooks."});
SCRIPT(d_show_frame_wood,
 {"DANNY","A wooden frame.","It feels more like home."},
 {"DANNY","These little windows","all belonged to someone."},
 {"DANNY","I'll carry it down.","The annex has hooks."});
SCRIPT(d_show_setup,
 {"ANNEX KEEPER","A poster, a painting,","and a tape. Good."},
 {"DANNY","It's not quite a tour.","It's one room."},
 {"ANNEX KEEPER","Every show starts","with one room."},
 {"DANNY","I'll tell Mara we're","ready. No more excuses."});
SCRIPT(d_show_play,
 {"MARA","There. The chairs","are facing your art."},
 {"DANNY","No stage lights?","No terrible banana?"},
 {"MARA","Just neighbors.","Play what you made."},
 {"DANNY","This is our street.","Every crooked window."},
 {"MARA","I can hear the song","you kept working on."},
 {"DANNY","I thought nobody","would ever hear it."},
 {"MARA","We did. That's enough","for tonight."},
 {"DANNY","One more thing.","BORE should have a copy."});
SCRIPT(d_show_archive,
 {"BORE CLERK","A live show record.","And the poster."},
 {"DANNY","Please keep both.","Someone might want them."},
 {"BORE CLERK","Filed under your name.","Not your work uniform."},
 {"DANNY","For once, a record","I wanted to make."},
 {"BORE CLERK","The original stays.","You can make another."},
 {"DANNY","I still have choices.","That's a good feeling."});
#undef SCRIPT
#define SEQ(name) {name, (uint8_t)(sizeof(name) / sizeof((name)[0]))}
static const struct { const StoryPage *pages; uint8_t count; } scripts[] = {
 {0,0}, SEQ(d_open), SEQ(d_letter), SEQ(d_letter_again), SEQ(d_clerk_locked),
 SEQ(d_clerk), SEQ(d_clerk_again), SEQ(d_mara_locked), SEQ(d_mara), SEQ(d_mara_again),
 SEQ(d_bus_locked), SEQ(d_bus_ready), SEQ(d_end_go), SEQ(d_end_stay),
 SEQ(d_journal_0), SEQ(d_journal_1), SEQ(d_journal_2), SEQ(d_journal_3),
 SEQ(d_nothing), SEQ(d_return), SEQ(d_cult_find), SEQ(d_cult_logs),
 SEQ(d_cult_wait), SEQ(d_cult_witness), SEQ(d_cult_safety), SEQ(d_cult_backup),
 SEQ(d_cult_after), SEQ(d_cult_not_yet), SEQ(d_cult_witness_after),
 SEQ(d_cult_safe_again), SEQ(d_cult_pre_win), SEQ(d_cult_during_win),
 SEQ(d_cult_post_win), SEQ(d_cult_run), SEQ(d_cult_lose),
 SEQ(d_cult_notes_1), SEQ(d_cult_notes_2), SEQ(d_cult_notes_3),
 SEQ(d_cult_notes_4),
 SEQ(d_music), SEQ(d_music_again), SEQ(d_art), SEQ(d_art_again),
 SEQ(d_art_day), SEQ(d_art_last_locked), SEQ(d_neutral_start),
 SEQ(d_medical_notice), SEQ(d_doctor_first), SEQ(d_doctor_return),
 SEQ(d_doctor_last), SEQ(d_neutral_paint), SEQ(d_neutral_end),
 SEQ(d_good_end), SEQ(d_day_notes), SEQ(d_clinic_locked),
 SEQ(d_note_visit), SEQ(d_note_followup), SEQ(d_note_canvas),
 SEQ(d_day_mara), SEQ(d_day_mara_after), SEQ(d_day_letter), SEQ(d_day_letter_after),
 SEQ(d_day_clerk), SEQ(d_day_clerk_after), SEQ(d_day_music), SEQ(d_day_music_after),
 SEQ(d_day_paint), SEQ(d_day_paint_after), SEQ(d_day_recap), SEQ(d_day_recap_after),
  SEQ(d_act_mara_talk), SEQ(d_act_letter_keep), SEQ(d_act_clerk_joke),
  SEQ(d_act_music_listen), SEQ(d_act_art_rest),
  SEQ(d_act_mara_close), SEQ(d_act_music_progress), SEQ(d_act_art_progress),
  SEQ(d_project_song_2), SEQ(d_project_song_3), SEQ(d_project_song_done),
  SEQ(d_project_art_2), SEQ(d_project_art_3), SEQ(d_project_art_done),
  SEQ(d_mara_morning), SEQ(d_mara_closing), SEQ(d_clerk_closing),
  SEQ(d_mara_gallery), SEQ(d_clerk_song),
  SEQ(d_bad_letter), SEQ(d_bad_clerk), SEQ(d_bad_music), SEQ(d_bad_server),
  SEQ(d_mara_missing), SEQ(d_mara_absent), SEQ(d_rescue_witness),
  SEQ(d_rescue_trace), SEQ(d_rescue_saved), SEQ(d_rescue_lost),
  SEQ(d_rescue_aftershock), SEQ(d_bad_end), SEQ(d_bad_end_saved),
  SEQ(d_rescue_notes_1),
  SEQ(d_rescue_notes_2), SEQ(d_rescue_notes_3), SEQ(d_rescue_notes_saved),
  SEQ(d_bus_rescue), SEQ(d_bad_warning_2), SEQ(d_clinic_rescue),
  SEQ(d_rescue_witness_trust), SEQ(d_rescue_saved_trust),
  SEQ(d_rescue_saved_art),
  SEQ(d_hope_realize), SEQ(d_hope_phone_wait), SEQ(d_hope_phone_late),
  SEQ(d_hope_witness), SEQ(d_hope_terminal), SEQ(d_hope_door),
  SEQ(d_hope_note_wait), SEQ(d_hope_note_late),
  SEQ(d_mara_concern), SEQ(d_mara_warning), SEQ(d_bad_music_master),
  SEQ(d_good_end_scars), SEQ(d_rescue_lost_remote),
  SEQ(d_shift_early), SEQ(d_shift_done), SEQ(d_shift_repeat),
  SEQ(d_shift_tired), SEQ(d_shift_paused),
  SEQ(d_doctor_return_alone),
  SEQ(d_look_potheads), SEQ(d_look_canal), SEQ(d_look_market),
  SEQ(d_look_north), SEQ(d_look_street),
  SEQ(d_project_song_start), SEQ(d_project_art_start),
  SEQ(d_mara_prebus_walk), SEQ(d_mara_prebus_talk),
  SEQ(d_mara_master), SEQ(d_mara_creations),
  SEQ(d_bus_ready_legacy), SEQ(d_bus_stay_open),
  SEQ(d_doctor_last_alone),
  SEQ(d_nb_0_intro),
  SEQ(d_nb_0_left),
  SEQ(d_nb_0_right),
  SEQ(d_nb_1_intro),
  SEQ(d_nb_1_left),
  SEQ(d_nb_1_right),
  SEQ(d_nb_2_intro),
  SEQ(d_nb_2_left),
  SEQ(d_nb_2_right),
  SEQ(d_nb_3_intro),
  SEQ(d_nb_3_left),
  SEQ(d_nb_3_right),
  SEQ(d_nb_4_intro),
  SEQ(d_nb_4_left),
  SEQ(d_nb_4_right),
  SEQ(d_nb_5_intro),
  SEQ(d_nb_5_left),
  SEQ(d_nb_5_right),
  SEQ(d_nb_6_intro),
  SEQ(d_nb_6_left),
  SEQ(d_nb_6_right),
  SEQ(d_nb_7_intro),
  SEQ(d_nb_7_left),
  SEQ(d_nb_7_right),
  SEQ(d_nb_8_intro),
  SEQ(d_nb_8_left),
  SEQ(d_nb_8_right),
  SEQ(d_nb_9_intro),
  SEQ(d_nb_9_left),
  SEQ(d_nb_9_right),
  SEQ(d_nb_10_intro),
  SEQ(d_nb_10_left),
  SEQ(d_nb_10_right),
  SEQ(d_nb_11_intro),
  SEQ(d_nb_11_left),
  SEQ(d_nb_11_right),
  SEQ(d_nb_12_intro),
  SEQ(d_nb_12_left),
  SEQ(d_nb_12_right),
  SEQ(d_nb_13_intro),
  SEQ(d_nb_13_left),
  SEQ(d_nb_13_right), SEQ(d_mara_master_shared),
  SEQ(d_show_plan),SEQ(d_show_poster_blue),SEQ(d_show_poster_warm),SEQ(d_show_rehearsed),
  SEQ(d_show_frame_black),SEQ(d_show_frame_wood),SEQ(d_show_setup),SEQ(d_show_play),SEQ(d_show_archive),
  SEQ(d_nb_return_0_0),
  SEQ(d_nb_return_0_1),
  SEQ(d_nb_return_1_0),
  SEQ(d_nb_return_1_1),
  SEQ(d_nb_return_2_0),
  SEQ(d_nb_return_2_1),
  SEQ(d_nb_return_3_0),
  SEQ(d_nb_return_3_1),
  SEQ(d_nb_return_4_0),
  SEQ(d_nb_return_4_1),
  SEQ(d_nb_return_5_0),
  SEQ(d_nb_return_5_1),
  SEQ(d_nb_return_6_0),
  SEQ(d_nb_return_6_1),
  SEQ(d_nb_return_7_0),
  SEQ(d_nb_return_7_1),
  SEQ(d_nb_return_8_0),
  SEQ(d_nb_return_8_1),
  SEQ(d_nb_return_9_0),
  SEQ(d_nb_return_9_1),
  SEQ(d_nb_return_10_0),
  SEQ(d_nb_return_10_1),
  SEQ(d_nb_return_11_0),
  SEQ(d_nb_return_11_1),
  SEQ(d_nb_return_12_0),
  SEQ(d_nb_return_12_1),
  SEQ(d_nb_return_13_0),
  SEQ(d_nb_return_13_1),SEQ(d_city_east),SEQ(d_city_civic),SEQ(d_city_park),SEQ(d_city_canal),SEQ(d_city_plaza)
};
_Static_assert(sizeof(scripts)/sizeof(scripts[0])==D_CITY_PLAZA+1u,"script table order");
static int neighborhood_return_id(unsigned id){
 return id>=D_NB_RETURN_0_0&&id<=D_NB_RETURN_13_1?(int)(id-D_NB_RETURN_0_0):-1;
}
static const uint8_t neighborhood_pair[STORY_NEIGHBORHOOD_COUNT]={9,7,12,13,11,10,8,1,6,0,5,4,2,3};
static int neighborhood_dialogue(unsigned id){
 return id>=D_NB_0_INTRO&&id<=D_NB_13_RIGHT?(int)(id-D_NB_0_INTRO):-1;
}
int story_neighborhood_panel(const Story *s){
 int revisit=neighborhood_return_id(s->dialogue);
 if(revisit>=0)return (revisit/2)*3+1+(revisit%2);
 if(s->dialogue==D_SHOW_POSTER_BLUE)return 4;
 if(s->dialogue==D_SHOW_POSTER_WARM)return 5;
 if(s->dialogue==D_SHOW_SETUP)return 22;
 if(s->activity==STORY_ACT_SHOW){
  if(story_show_stage(s)==1u)return 3;
  if(story_show_stage(s)==4u)return 21;
 }
 int id=neighborhood_dialogue(s->dialogue);
 if(id>=0)return id;
 if(s->activity==STORY_ACT_NEIGHBORHOOD){
  int at=story_neighborhood_at(s);
  return at>=0?at*3:-1;
 }
 return -1;
}
int story_show_scene(const Story *s){
 return s->dialogue==D_SHOW_PLAY||
        (s->activity==STORY_ACT_SHOW&&story_show_stage(s)==5u);
}
int story_master_showcase(const Story *s){
 return s->song_steps>=STORY_PROJECT_DONE&&
        (s->dialogue==D_MARA_MASTER||s->dialogue==D_MARA_MASTER_SHARED||
         s->dialogue==D_MARA_CREATIONS||s->dialogue==D_ACT_MUSIC_LISTEN||
         s->dialogue==D_SHOW_PLAY||s->activity==STORY_ACT_REHEARSAL);
}
const StoryPage *story_pages(const Story *s, int *count) {
    unsigned id = s->dialogue;
    if (id >= sizeof(scripts) / sizeof(scripts[0])) id = 0;
    *count = scripts[id].count;
    int revisit=neighborhood_return_id(id);
    /* The third page connects the pair only after Danny has actually
       completed the other encounter. Existing saves hold both bits. */
    if(revisit>=0&&!(s->neighborhood_seen&(1u<<neighborhood_pair[revisit/2])))
        *count=2;
    return scripts[id].pages;
}
/* Each major interaction receives a world-grounded, animated shot sequence.
   The same script pages act as camera beats; no replay cutscene flags needed. */
/* Deterministic edit points on selected authored conversations. These are
   camera cues, not unskippable cutscene delays: B still dismisses, and
   the VBlank clock never runs backwards. Longer live reads continue at
   their naturally advanced frame without replaying a shot. */
static uint16_t dialogue_shot_beat(unsigned script,unsigned page){
 static const uint16_t opening[]={0u,90u,180u,450u,780u,990u,1188u};
 static const uint16_t letter[]={0u,55u,108u,160u};
 static const uint16_t mara[]={0u,66u,108u,151u,190u};
 static const uint16_t bus[]={0u,118u,183u};
 static const uint16_t canvas[]={0u,198u};
 static const uint16_t doctor[]={0u,124u,158u,200u};
 static const uint16_t followup[]={0u,98u,183u};
 static const uint16_t reunion4[]={0u,100u,170u,250u};
 static const uint16_t reunion3[]={0u,128u,250u};
 const uint16_t *beats=0;unsigned length=0;
 switch(script){
  case D_OPEN:beats=opening;length=7u;break;
  case D_LETTER:beats=letter;length=4u;break;
  case D_MARA:beats=mara;length=5u;break;
  case D_BUS_READY:case D_BUS_READY_LEGACY:
   case D_BUS_STAY_OPEN:beats=bus;length=3u;break;
  case D_ART:beats=canvas;length=2u;break;
  case D_DOCTOR_FIRST:case D_DOCTOR_LAST:case D_DOCTOR_LAST_ALONE:
   beats=doctor;length=4u;break;
  case D_DOCTOR_RETURN:case D_DOCTOR_RETURN_ALONE:
   beats=followup;length=3u;break;
  case D_RESCUE_SAVED:beats=reunion4;length=4u;break;
  case D_RESCUE_SAVED_TRUST:case D_RESCUE_SAVED_ART:
   beats=reunion3;length=3u;break;
  default:break;
 }
 return beats&&page<length?beats[page]:0u;
}
/* A final A can commit a promise, save an outcome, or open a decision.
   Expose the actual intent rather than describing everything as NEXT. */
static int dialogue_commits(const Story *s){
    int episode=neighborhood_dialogue(s->dialogue);
    return episode>=0||s->dialogue==D_BUS_READY||s->dialogue==D_BUS_READY_LEGACY||
           (s->dialogue==D_MARA&&!(s->flags&STORY_MARA))||
           ((s->dialogue==D_MARA_MASTER||s->dialogue==D_MARA_CREATIONS)&&share_pending(s))||
           (s->dialogue==D_SHOW_PLAY&&story_show_stage(s)==5u);
}
const char *story_dialogue_accept(const Story *s){
    int count=0;
    story_pages(s,&count);
    if(s->page+1u<(unsigned)count)return "A NEXT";
    int episode=neighborhood_dialogue(s->dialogue);
    if(episode>=0)return episode%3==0?"A CHOOSE":"A SAVE";
    if(s->dialogue==D_BUS_READY||s->dialogue==D_BUS_READY_LEGACY)return "A CHOOSE";
    if(s->dialogue==D_MARA&&!(s->flags&STORY_MARA))return "A PROMISE";
    if((s->dialogue==D_MARA_MASTER||s->dialogue==D_MARA_CREATIONS)&&share_pending(s))
        return "A SHARE";
    if(s->dialogue==D_SHOW_PLAY&&story_show_stage(s)==5u)return "A FINISH";
    if(s->dialogue==D_OPEN)return "A WALK";
    return s->ended?"A CLOSE":"A RETURN";
}
const char *story_dialogue_back(const Story *s){
    int count=0;
    story_pages(s,&count);
    if(dialogue_commits(s))return "B CANCEL";
    return s->page+1u<(unsigned)count?"B SKIP":"B CLOSE";
}
static uint8_t cinematics_for_dialogue(uint8_t d){
    if(neighborhood_dialogue(d)>=0||neighborhood_return_id(d)>=0)return CINE_DAY;
    if(d==D_SHOW_POSTER_BLUE||d==D_SHOW_POSTER_WARM||d==D_SHOW_SETUP)return CINE_BORE;
    switch(d){
    case D_OPEN: case D_SHIFT_EARLY: case D_SHIFT_DONE:
    case D_SHIFT_REPEAT: case D_SHIFT_TIRED: return CINE_OPEN;
    case D_SHIFT_PAUSED: return CINE_NOTICE;
    case D_LETTER: case D_LETTER_AGAIN: return CINE_LETTER;
    case D_CLERK_LOCKED: case D_CLERK: case D_CLERK_AGAIN: return CINE_BORE;
    case D_MARA_LOCKED: case D_MARA: case D_MARA_AGAIN:
    case D_MARA_CONCERN: case D_MARA_WARNING: return CINE_MARA;
    case D_BUS_LOCKED: case D_BUS_READY: case D_BUS_READY_LEGACY:
    case D_BUS_STAY_OPEN: case D_END_GO: return CINE_BUS;
    case D_END_STAY: return CINE_STAY;
    case D_MUSIC: case D_MUSIC_AGAIN: return CINE_MUSIC;
    case D_ART: case D_ART_AGAIN: case D_ART_DAY: case D_ART_LAST_LOCKED:
       return CINE_ART;
    case D_CULT_FIND: case D_CULT_LOGS: case D_CULT_WAIT: return CINE_SERVER;
    case D_CULT_WITNESS: case D_CULT_NOT_YET: case D_CULT_WITNESS_AFTER:
       return CINE_WITNESS;
    case D_CULT_SAFETY: case D_CULT_SAFE_AGAIN: return CINE_SAFE;
    case D_CULT_BACKUP: return CINE_BACKUP;
    case D_CULT_AFTER: case D_CULT_POST_WIN: return CINE_REBRAND;
    case D_CULT_PRE_WIN: case D_CULT_DURING_WIN:
    case D_CULT_RUN: case D_CULT_LOSE: return CINE_SERVER;
    case D_MEDICAL_NOTICE: return CINE_NOTICE;
    case D_DOCTOR_FIRST: return CINE_CLINIC;
    case D_DOCTOR_RETURN: case D_DOCTOR_RETURN_ALONE:
       return CINE_FOLLOWUP;
    case D_DOCTOR_LAST: case D_DOCTOR_LAST_ALONE: return CINE_LAST_CLINIC;
    case D_GOOD_END: case D_GOOD_END_SCARS: return CINE_GOOD;
    case D_NEUTRAL_START: return CINE_DAY;
    case D_NEUTRAL_PAINT: case D_NEUTRAL_END: return CINE_FINALE;
    case D_DAY_MARA: case D_DAY_MARA_AFTER: return CINE_MARA;
    case D_DAY_LETTER: case D_DAY_LETTER_AFTER: return CINE_LETTER;
    case D_DAY_CLERK: case D_DAY_CLERK_AFTER: return CINE_BORE;
    case D_DAY_MUSIC: case D_DAY_MUSIC_AFTER: return CINE_MUSIC;
    case D_DAY_PAINT: case D_DAY_PAINT_AFTER: return CINE_ART;
    case D_DAY_RECAP: case D_DAY_RECAP_AFTER: return CINE_DAY;
    case D_ACT_MARA_TALK: return CINE_MARA;
    case D_ACT_LETTER_KEEP: return CINE_LETTER;
    case D_ACT_CLERK_JOKE: return CINE_BORE;
    case D_ACT_MUSIC_LISTEN: return CINE_MUSIC;
    case D_ACT_ART_REST: case D_ACT_ART_PROGRESS: return CINE_ART;
    case D_ACT_MARA_CLOSE: return CINE_MARA;
    case D_ACT_MUSIC_PROGRESS: return CINE_MUSIC;
    case D_PROJECT_SONG_START: case D_PROJECT_SONG_2: case D_PROJECT_SONG_3:
    case D_PROJECT_SONG_DONE: case D_CLERK_SONG: return CINE_MUSIC;
    case D_PROJECT_ART_START: case D_PROJECT_ART_2: case D_PROJECT_ART_3:
    case D_PROJECT_ART_DONE: return CINE_ART;
    case D_MARA_MORNING: case D_MARA_CLOSING:
    case D_MARA_GALLERY: case D_MARA_PREBUS_WALK: case D_MARA_PREBUS_TALK:
    case D_MARA_MASTER: case D_MARA_CREATIONS: case D_MARA_MASTER_SHARED:
    case D_SHOW_PLAN:case D_SHOW_PLAY:return CINE_MARA;
    case D_SHOW_REHEARSED:return CINE_MUSIC;
    case D_SHOW_FRAME_BLACK:case D_SHOW_FRAME_WOOD:return CINE_ART;
    case D_SHOW_ARCHIVE:return CINE_BORE;
    case D_CLERK_CLOSING: return CINE_BORE;
    case D_BAD_LETTER: return CINE_LETTER;
    case D_BAD_CLERK: return CINE_BORE;
    case D_BAD_MUSIC: case D_BAD_MUSIC_MASTER: return CINE_MUSIC;
    case D_BAD_SERVER: case D_RESCUE_TRACE: case D_RESCUE_AFTERSHOCK:
    case D_HOPE_TERMINAL: return CINE_SERVER;
    case D_MARA_MISSING: case D_MARA_ABSENT:
    case D_HOPE_PHONE_WAIT: case D_HOPE_PHONE_LATE:
        return CINE_MARA_EMPTY;
    case D_RESCUE_WITNESS: case D_RESCUE_WITNESS_TRUST:
    case D_HOPE_WITNESS: return CINE_WITNESS;
    case D_RESCUE_LOST: case D_HOPE_DOOR: return CINE_RESCUE_DOOR;
    case D_RESCUE_SAVED: case D_RESCUE_SAVED_TRUST:
    case D_RESCUE_SAVED_ART: return CINE_RESCUE_SAVED;
    case D_BAD_END: case D_BAD_END_SAVED: return CINE_BAD;
    case D_BUS_RESCUE: return CINE_BUS;
    case D_BAD_WARNING_2: case D_HOPE_REALIZE:
    case D_RESCUE_LOST_REMOTE: return CINE_NOTICE;
    case D_CLINIC_RESCUE: return CINE_CLINIC;
    default: return CINE_NONE;
    }
}
static void talk(Story *s, uint8_t id) {
    s->dialogue=id;s->page=0;
    s->cinema=cinematics_for_dialogue(id);s->cinema_frame=0;
}
void story_new(Story *s, int slot) {
    *s = (Story){0};
    studio_reset(); /* a new game starts with a clean optional tune */
    /* The Potheads opening now lands at its ACTUAL storefront; the first
       controllable frame starts beside the same facade seen in the film.
       Only a fresh game's spawn changes; old saves preserve coordinates. */
    s->x = 20; s->y = 11; s->prev_x = s->x; s->prev_y = s->y;
    s->facing = 0; s->slot = (uint8_t)(slot & 1);
    s->day_number = 1;
    s->energy=3;s->spark=1;s->bond=1;
    talk(s, D_OPEN); s->dirty = 1;
}
void story_resume(Story *s, int slot, int x, int y, int facing, int view, int flags, int ended, int ending) {
    story_new(s, slot);
    if (!story_solid(x,y)) { s->x = (uint8_t)x; s->y = (uint8_t)y; }
    s->prev_x = s->x; s->prev_y = s->y;
    s->facing = (uint8_t)(facing & 3); s->view = (uint8_t)(view & 1);
    s->flags = (uint8_t)(flags & STORY_FULL);
    s->ended = (uint8_t)(!!ended); s->ending = (uint8_t)(ending <= STORY_END_BAD ? ending : 0);
    s->dirty = 0;
    if(s->ended && (s->ending==STORY_END_GOOD || s->ending==STORY_END_BAD)){
        /* Replay the cinematic in sync with the one-shot score after loading. */
        s->finale_frame=0;s->dialogue=0;s->page=0;
        s->cinema=s->ending==STORY_END_BAD?CINE_BAD:CINE_GOOD;
    } else if(s->ended && s->ending==STORY_END_NEUTRAL){
        s->finale_frame=0;s->dialogue=0;s->page=0;s->cinema=CINE_FINALE;
        s->life_stage=STORY_LIFE_FINALE;
    } else talk(s,D_RETURN);
}
/* Two waves of four telegraphed hazards each, delivered in 30 VBlanks.
   A parries for 15 ticks in FACTS or 12 in CARE; CARE also absorbs one
   missed attack. The real simulation clock, not draw FPS, sets timing. */
int story_attack_lane(const Story *s, int beat) {
    return (beat * 2 + s->cult_enemy + (s->cult_stage >= CULT_RESOLVED ? 1 : 0)) % 3;
}
const char *story_enemy_name(const Story *s) {
    static const char *const pre[3] = {"RATIO RAT", "BADGE LORD", "LORE WARDEN"};
    static const char *const post[3] = {"REBRAND RAT", "EX MODERATOR", "REPLY ARCHIVE"};
    return s->cult_stage >= CULT_RESOLVED ? post[s->cult_enemy % 3] : pre[s->cult_enemy % 3];
}
static void cult_start(Story *s, unsigned enemy) {
    s->battle = BATTLE_MENU;
    s->cult_enemy = (uint8_t)(enemy % CULT_ENCOUNTERS);
    s->battle_choice = 0;
    s->battle_lane = 1;
    s->battle_focus = 4;
    s->battle_progress = 0;
    s->battle_timer = 0;
    s->battle_parry = s->battle_parry_cd = s->battle_guard = 0;
}
static void cult_finish(Story *s, int result) {
    s->battle = BATTLE_NONE;
    if (result > 0) {
        if (s->cult_stage == 0) s->cult_pre_wins |= (uint8_t)(1u << s->cult_enemy);
        else if (s->cult_stage >= CULT_RESOLVED) s->cult_post_wins |= (uint8_t)(1u << s->cult_enemy);
        talk(s, s->cult_stage == 0 ? D_CULT_PRE_WIN :
             s->cult_stage >= CULT_RESOLVED ? D_CULT_POST_WIN : D_CULT_DURING_WIN);
        s->dirty = 1;
    } else talk(s, result == 0 ? D_CULT_RUN : D_CULT_LOSE);
}
static void cult_tick(Story *s, uint16_t pressed) {
    enum { A=1, B=2, RIGHT=16, LEFT=32 };
    if (s->battle == BATTLE_MENU) {
        if (pressed & LEFT) s->battle_choice = (uint8_t)((s->battle_choice + 2) % 3);
        if (pressed & RIGHT) s->battle_choice = (uint8_t)((s->battle_choice + 1) % 3);
        if (pressed & B) { cult_finish(s, 0); return; }
        if (pressed & A) {
            if (s->battle_choice == 2) { cult_finish(s, 0); return; }
            s->battle = BATTLE_DODGE;
            s->battle_timer = 0;
            s->battle_lane = 1;
            s->battle_guard = s->battle_choice == 1;
            s->battle_parry = s->battle_parry_cd = 0;
        }
        return;
    }
    if (pressed & LEFT && s->battle_lane > 0) --s->battle_lane;
    if (pressed & RIGHT && s->battle_lane < 2) ++s->battle_lane;
    if (s->battle_parry) --s->battle_parry;
    if (s->battle_parry_cd) --s->battle_parry_cd;
    if ((pressed & A) && !s->battle_parry_cd) {
        s->battle_parry = (uint8_t)(s->battle_choice==0u?
                         STORY_FACTS_PARRY_TICKS:STORY_CARE_PARRY_TICKS);
        s->battle_parry_cd = (uint8_t)(s->battle_choice==0u?
                         STORY_FACTS_PARRY_COOLDOWN:STORY_CARE_PARRY_COOLDOWN);
    }
    ++s->battle_timer;
    if (s->battle_timer % STORY_BATTLE_BEAT == 0) {
        int beat = (int)(s->battle_timer / STORY_BATTLE_BEAT) - 1;
        if(s->battle_lane==story_attack_lane(s,beat)){
            if(s->battle_parry){
                if(s->battle_choice==0u && s->battle_focus<4u)
                    ++s->battle_focus; /* Accurate FACTS counter */
            }else if(s->battle_guard) s->battle_guard=0;
            else if(s->battle_focus) --s->battle_focus;
        }
    }
    if (s->battle_timer >= STORY_BATTLE_BEAT*STORY_BATTLE_STRIKES) {
        if (!s->battle_focus) { cult_finish(s, -1); return; }
        if (++s->battle_progress >= 2) { cult_finish(s, 1); return; }
        s->battle = BATTLE_MENU;
        s->battle_choice = 0;
    }
}
/* Optional memories are not quest gates, scores, health, or deadlines. */
static int can_make_memory(const Story *s){
    return s->life_stage==STORY_LIFE_WAIT ||
           s->life_stage==STORY_LIFE_FOLLOWUP_WAIT;
}
static int remember(Story *s,unsigned bit){
    if(!can_make_memory(s) || (s->daily_mask&bit))return 0;
    s->daily_mask=(uint8_t)(s->daily_mask|bit);
    if(s->daily_memories<99)++s->daily_memories;
    s->dirty=1;
    return 1;
}
/* Choices carry small, visible benefits, never failure or ending gates. */
static unsigned day_period(const Story *s){
    if(s->day_frame<STORY_DAY_FRAMES/3u)return 0u; /* morning */
    if(s->day_frame<STORY_DAY_FRAMES*2u/3u)return 1u; /* afternoon */
    return 2u; /* evening */
}
static void moment_menu(Story *s,unsigned station){
    s->activity=(uint8_t)station;s->activity_choice=0;
    s->cinema=(uint8_t)((station=='F'||station=='f')?CINE_MARA:station=='L'?CINE_LETTER:
        station=='C'?CINE_BORE:
        (station=='M'||station=='m'||station=='E')?CINE_MUSIC:
        station=='J'?CINE_OPEN:CINE_ART);
    s->cinema_frame=station=='J'?184u:0u;
}
static void stat_gain(uint8_t *v,unsigned gain){
    unsigned sum=(unsigned)*v+gain;
    *v=(uint8_t)(sum>STORY_STAT_MAX?STORY_STAT_MAX:sum);
}
static void work_gain(Story *s,unsigned gain){
    unsigned sum=(unsigned)s->work+gain;
    s->work=(uint8_t)(sum>99u?99u:sum);
}
static int exert(Story *s){
    if(!s->energy)return 0;
    --s->energy;return 1;
}

/* A completed project is a small tangible creation, not a new gate.
   The second choice produces a gentler warm/soft alternative arrangement. */
static void step_project(Story *s,unsigned station,unsigned soft){
    uint8_t *phase=station=='m'?&s->song_steps:&s->art_steps;
    if(*phase>=STORY_PROJECT_DONE){
        talk(s,station=='m'?D_PROJECT_SONG_DONE:D_PROJECT_ART_DONE);
        return;
    }
    if(soft){
        stat_gain(&s->spark,1);
        stat_gain(&s->energy,1);
        s->project_style|=(uint8_t)(station=='m'?
            STORY_STYLE_SONG_SOFT:STORY_STYLE_ART_WARM);
        work_gain(s,1u);
    }else{
        int enough=exert(s);
        work_gain(s,1u+(unsigned)enough+(s->spark>=4u));
        stat_gain(&s->spark,1);
    }
    ++*phase;
    s->dirty=1;
    if(station=='m'){
        talk(s,*phase>=4u?D_PROJECT_SONG_DONE:
            *phase==3u?D_PROJECT_SONG_3:
            *phase==2u?D_PROJECT_SONG_2:
            s->life_stage==STORY_LIFE_NONE?D_PROJECT_SONG_START:
            s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?D_DAY_MUSIC_AFTER:D_DAY_MUSIC);
        if(!(s->flags&STORY_MUSIC)){s->flags|=STORY_MUSIC;s->dirty=1;}
    }else{
        talk(s,*phase>=4u?D_PROJECT_ART_DONE:
            *phase==3u?D_PROJECT_ART_3:
            *phase==2u?D_PROJECT_ART_2:
            s->life_stage==STORY_LIFE_NONE?D_PROJECT_ART_START:
            s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?D_DAY_PAINT_AFTER:D_DAY_PAINT);
        /* Actual painting qualifies as the legacy objective; resting does not. */
        s->flags|=STORY_ART;
    }
}
static void enter_music_editor(Story *s){
 s->activity=STORY_ACT_STUDIO;s->activity_choice=0;
 s->studio_tab=STUDIO_SONG;s->studio_cursor=0;
 s->cinema=CINE_MUSIC;s->cinema_frame=0;
}
static void show_advance(Story *s,unsigned stage){
 s->project_style=(uint8_t)((s->project_style&~STORY_SHOW_STAGE_MASK)|(stage<<3));
 s->dirty=1;
}
static void show_menu(Story *s,unsigned stage){
 s->activity=STORY_ACT_SHOW;s->activity_choice=0;
 s->cinema=(uint8_t)((stage==0u||stage==5u)?CINE_MARA:stage==3u?CINE_ART:CINE_BORE);
 s->cinema_frame=0;
}
static void finish_show(Story *s,unsigned option){
 unsigned stage=story_show_stage(s);
 s->activity=0;s->activity_choice=0;s->cinema=CINE_NONE;
 if(!show_available(s))return;
 if(stage!=1u&&stage!=3u&&option)return;
 switch(stage){
 case 0:show_advance(s,1u);talk(s,D_SHOW_PLAN);break;
 case 1:
  s->project_style=(uint8_t)((s->project_style&~STORY_SHOW_WARM_POSTER)|(option?STORY_SHOW_WARM_POSTER:0));
  show_advance(s,2u);talk(s,option?D_SHOW_POSTER_WARM:D_SHOW_POSTER_BLUE);break;
 case 3:
  if(s->art_steps<4u)return;
  s->project_style=(uint8_t)((s->project_style&~STORY_SHOW_WARM_FRAME)|(option?STORY_SHOW_WARM_FRAME:0));
  show_advance(s,4u);talk(s,option?D_SHOW_FRAME_WOOD:D_SHOW_FRAME_BLACK);break;
 case 4:show_advance(s,5u);talk(s,D_SHOW_SETUP);break;
 case 5:
  if(s->song_steps>=4u&&s->art_steps>=4u)talk(s,D_SHOW_PLAY);
  break;
 case 6:show_advance(s,7u);s->neighborhood_target=0;
  work_gain(s,3u);stat_gain(&s->spark,1u);talk(s,D_SHOW_ARCHIVE);break;
 default:break;
 }
}
static void finish_moment(Story *s){
    unsigned station=s->activity,option=s->activity_choice;
    if(station==STORY_ACT_SHOW){finish_show(s,option);return;}
    if(station==STORY_ACT_NEIGHBORHOOD){
        int at=story_neighborhood_at(s);
        s->activity=0;s->activity_choice=0;
        if(at<0){s->cinema=CINE_NONE;return;}
        talk(s,(uint8_t)(D_NB_0_INTRO+at*3+1u+(option?1u:0u)));
        return;
    }
    if(station=='f'){
        s->activity=STORY_ACT_NONE;s->activity_choice=0;
        if(s->life_stage!=STORY_LIFE_NONE || !(s->flags&STORY_MARA) ||
           (s->daily_mask&STORY_PREBUS_MARA) || story_bad_count(s)){
            s->cinema=CINE_NONE;return;
        }
        s->daily_mask|=STORY_PREBUS_MARA;
        /* A once-only relationship choice, not a repeatedly farmable stat. */
        if(option){
            stat_gain(&s->energy,1);stat_gain(&s->bond,1);
            talk(s,D_MARA_PREBUS_TALK);
        }else{
            exert(s);stat_gain(&s->bond,2);
            talk(s,D_MARA_PREBUS_WALK);
        }
        s->dirty=1;
        return;
    }
    if(station=='J'){
        /* Opening the menu spends nothing. WORK must be explicitly confirmed. */
        s->activity=STORY_ACT_NONE;s->activity_choice=0;
        if(option){s->cinema=CINE_NONE;return;}
        if(!can_make_memory(s))talk(s,D_SHIFT_EARLY);
        else if(s->daily_mask&STORY_DAY_SHIFT)talk(s,D_SHIFT_REPEAT);
        else if(!s->energy)talk(s,D_SHIFT_TIRED);
        else {
            --s->energy;
            work_gain(s,2u);
            s->daily_mask|=STORY_DAY_SHIFT;
            s->dirty=1;
            talk(s,D_SHIFT_DONE);
        }
        s->cinema_frame=184u; /* avoid replaying the full aerial intro */
        return;
    }
    if(station==STORY_ACT_ROOF_REST){
        s->activity=STORY_ACT_NONE;s->activity_choice=0;
        if(option){s->cinema=CINE_NONE;return;}
        /* A finished canvas still leaves a place to sit. Rest is renewable
           recovery, not another painting, paycheck or daily memory. */
        if(can_make_memory(s)){
            if(s->energy<STORY_STAT_MAX){stat_gain(&s->energy,1);s->dirty=1;}
            talk(s,D_ACT_ART_REST);
        }else s->cinema=CINE_NONE;
        return;
    }
    if(station=='E'){
        if(option){
            s->activity=STORY_ACT_STUDIO;
            s->activity_choice=0; /* transport stopped, no auto play */
            s->studio_tab=STUDIO_SONG;s->studio_cursor=0;
            s->cinema=CINE_MUSIC;s->cinema_frame=0;
        }else{
            s->activity=STORY_ACT_NONE;
            /* Listening to a completed master is still a real daily moment.
               Do not pay or advance the project a second time. */
            if(remember(s,STORY_DAY_MUSIC)){
                stat_gain(&s->energy,1);stat_gain(&s->spark,1);
            }
            talk(s,D_ACT_MUSIC_LISTEN);
        }
        return;
    }
    if((station=='m'||station=='M')&&option==2u){enter_music_editor(s);return;}
    if(story_paint_workshop_available(s)&&option==2u){
        s->activity=STORY_ACT_PAINT;
        s->activity_choice=(s->project_style&STORY_STYLE_ART_WARM)?STORY_PAINT_WARM:0u;
        s->studio_tab=s->studio_cursor=0;s->cinema=CINE_ART;s->cinema_frame=0;return;
    }
    if(station=='m'||station=='r'){
        s->activity=STORY_ACT_NONE;s->activity_choice=0;
        step_project(s,station,option);
        return;
    }
    unsigned bit=station=='F'?STORY_DAY_MARA:station=='L'?STORY_DAY_LETTER:
        station=='C'?STORY_DAY_CLERK:station=='M'?STORY_DAY_MUSIC:STORY_DAY_ART;
    s->activity=STORY_ACT_NONE;s->activity_choice=0;
    if(!remember(s,bit))return;
    switch(station){
    case 'F':
        if(option){stat_gain(&s->energy,1);stat_gain(&s->bond,1);talk(s,D_ACT_MARA_TALK);}
        else {exert(s);stat_gain(&s->bond,2);talk(s,s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?
              D_DAY_MARA_AFTER:D_DAY_MARA);}
        break;
    case 'L':
        if(option){stat_gain(&s->energy,1);stat_gain(&s->spark,1);talk(s,D_ACT_LETTER_KEEP);}
        else {exert(s);stat_gain(&s->spark,2);talk(s,s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?
              D_DAY_LETTER_AFTER:D_DAY_LETTER);}
        break;
    case 'C':
        if(option){stat_gain(&s->energy,1);stat_gain(&s->bond,1);talk(s,D_ACT_CLERK_JOKE);}
        else {exert(s);stat_gain(&s->spark,1);talk(s,s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?
              D_DAY_CLERK_AFTER:D_DAY_CLERK);}
        break;
    case 'M':
        if(option){
            stat_gain(&s->energy,1);stat_gain(&s->spark,1);
            talk(s,D_ACT_MUSIC_LISTEN);
        }else step_project(s,'m',0u);
        if(!(s->flags&STORY_MUSIC)){s->flags|=STORY_MUSIC;s->dirty=1;}
        break;
    default:
        if(option){
            stat_gain(&s->energy,2);stat_gain(&s->spark,1);
            talk(s,D_ACT_ART_REST);
        }else step_project(s,'r',0u);
        break;
    }
    s->dirty=1;
}

/* Only explicitly confirmed harmful actions earn checks.
   No countdown, energy deficit or ordinary conversation can harm Mara. */
int story_bad_count(const Story *s){
    unsigned mask=s->bad_mask&15u,n=0;
    for(int i=0;i<4;i++)n+=(mask>>i)&1u;
    return (int)n;
}
static int actor_tile(char c){
    return c=='L'||c=='C'||c=='F'||c=='B'||c=='P'||c=='V'||
           c=='Q'||c=='S'||c=='W'||c=='H'||c=='R'||c=='M'||c=='K';
}
static char nearby_actor(const Story *s){
    static const int dx[4]={1,0,-1,0},dy[4]={0,1,0,-1};
    char c=story_tile(s->x+dx[s->facing],s->y+dy[s->facing]);
    if(!actor_tile(c)){
        /* At a four-way crossroads the old E/S/W/N fallback could select
           the MUSIC station when the player had followed the LAST BUS
           waypoint to its neighboring tile. Prefer the current objective
           only when the player is not explicitly facing another actor. */
        int gx=0,gy=0;const char *label=0;
        if(story_goal(s,&gx,&gy,&label) &&
           ((gx==(int)s->x &&
              (gy==(int)s->y-1||gy==(int)s->y+1)) ||
            (gy==(int)s->y &&
              (gx==(int)s->x-1||gx==(int)s->x+1))) &&
           actor_tile(story_tile(gx,gy))){
            c=story_tile(gx,gy);
        }else{
            for(int i=0;i<4;i++){
                char a=story_tile(s->x+dx[i],s->y+dy[i]);
                if(actor_tile(a)){c=a;break;}
            }
        }
    }
    return c;
}
const char *story_action_hint(const Story *s){
    unsigned show=show_here(s);
    if(show){
     static const char *const names[]={"A PLAN","A POSTER","A REHEARSE","A FRAME","A SET UP","A SHOW","A ARCHIVE"};
     if(show==3u&&s->song_steps<4u)return (s->flags&STORY_MUSIC)?"A STUDIO":"A DEMO";
     if(show==4u&&s->art_steps<4u)return "A CANVAS";
     return names[show-1u];
    }
    /* This special storefront interaction is checked BEFORE all actors. */
    if(s->x==20u&&s->y==11u&&s->facing==0u)return "A SHIFT";
    if(story_neighborhood_at(s)>=0)return "A STORY";
    switch(nearby_actor(s)){
    case 'L':return "A READ";
    case 'C':return "A CLERK";
    case 'F':return s->rescue_stage==STORY_RESCUE_LOST?"A PHONE":
          (s->rescue_stage>=STORY_RESCUE_MISSING&&
           s->rescue_stage<=STORY_RESCUE_TRACE)?"A EMPTY CAFE":"A MARA";
    case 'B':return "A BUS";
    case 'M':return (s->flags&STORY_MUSIC)?"A STUDIO":"A DEMO";
    case 'R':return "A CANVAS";
    case 'K':return "A CLINIC";
    case 'P':case 'V':case 'Q':return "A DUEL";
    case 'S':return "A TERMINAL";
    case 'W':return "A WITNESS";
    case 'H':return "A DOOR";
    default:return "A LOOK";
    }
}
int story_risk_available(const Story *s){
    char target=nearby_actor(s);
    unsigned bit=target=='L'?STORY_BAD_LETTER:
                 target=='C'?STORY_BAD_CLERK:
                 target=='M'?STORY_BAD_MUSIC:
                 target=='S'?STORY_BAD_SERVER:0u;
    return bit && !(s->bad_mask&bit) &&
           s->rescue_stage!=STORY_RESCUE_LOST &&
           s->rescue_stage!=STORY_RESCUE_AFTERSHOCK;
}
static void begin_bad_film(Story *s){
    s->ended=1;s->ending=STORY_END_BAD;s->life_stage=STORY_LIFE_NONE;
    s->finale_frame=0;s->dialogue=0;s->page=0;
    s->scene=STORY_SCENE_NONE;s->cinema=CINE_BAD;s->cinema_frame=0;
    s->risk_menu=0;s->risk_choice=0;s->dirty=1;
}
/* The failed rescue does NOT instantly decide Mara's fate. Reuse the
   already-persisted day clock for this chapter; no new SRAM flags required.
   These days are independent of the clinic's original numbered days. */
static void begin_wait_for_call(Story *s,unsigned away_from_house){
    s->rescue_stage=STORY_RESCUE_LOST;
    s->life_stage=STORY_LIFE_NONE;
    s->day_number=0;s->day_frame=0;
    s->daily_mask=0;s->scene=STORY_SCENE_NONE;s->dirty=1;
    talk(s,away_from_house?D_RESCUE_LOST_REMOTE:D_RESCUE_LOST);
}
static void commit_bad(Story *s,char target){
    unsigned bit=target=='L'?STORY_BAD_LETTER:target=='C'?STORY_BAD_CLERK:
                 target=='M'?STORY_BAD_MUSIC:STORY_BAD_SERVER;
    if(s->bad_mask&bit)return; /* non-repeatable */
    s->bad_mask|=(uint8_t)bit;s->dirty=1;
    unsigned count=(unsigned)story_bad_count(s);
    if(count==4u && s->rescue_stage>=STORY_RESCUE_MISSING){
        if(s->rescue_stage==STORY_RESCUE_SAVED){
            s->rescue_stage=STORY_RESCUE_AFTERSHOCK;
            talk(s,D_RESCUE_AFTERSHOCK);
        }else begin_wait_for_call(s,1u);
    }else if(count==STORY_BAD_THRESHOLD &&
             s->rescue_stage==STORY_RESCUE_NONE){
        s->rescue_stage=STORY_RESCUE_MISSING;talk(s,D_MARA_MISSING);
    }else if(count==2u && s->rescue_stage==STORY_RESCUE_NONE){
        talk(s,D_BAD_WARNING_2);
    }else talk(s,target=='L'?D_BAD_LETTER:target=='C'?D_BAD_CLERK:
               target=='M'?(s->song_steps>=STORY_PROJECT_DONE?
                   D_BAD_MUSIC_MASTER:D_BAD_MUSIC):D_BAD_SERVER);
}
static void rescue_reunion(Story *s){
    if(story_check_live(s,STORY_CHECK_MARA_TRUST))
        talk(s,D_RESCUE_SAVED_TRUST);
    else if(story_check_live(s,STORY_CHECK_FRAMED_ART))
        talk(s,D_RESCUE_SAVED_ART);
    else talk(s,D_RESCUE_SAVED);
}
static void resolve_risk(Story *s){
    char target=(char)s->risk_menu;
    unsigned selected=s->risk_choice;
    s->risk_menu=0;s->risk_choice=0;s->cinema=CINE_NONE;
    if(target=='H'){
        if(s->rescue_stage!=STORY_RESCUE_TRACE)return;
        if(selected)begin_wait_for_call(s,0u);
        else{
            s->rescue_stage=STORY_RESCUE_SAVED;s->dirty=1;
            rescue_reunion(s);
        }
    }else if(selected)commit_bad(s,target);
}
static void interact(Story *s) {
    unsigned show=show_here(s);char forced=0;
    if(show){
     unsigned stage=show-1u;
     if(stage==2u){
      if(s->song_steps<4u)forced='M';
      else{
       s->activity=STORY_ACT_REHEARSAL;s->activity_choice=0;
       s->studio_tab=s->studio_cursor=0;s->cinema=CINE_MUSIC;s->cinema_frame=0;return;
      }
     }else if(stage==3u&&s->art_steps<4u)forced='R';
     else {show_menu(s,stage);return;}
    }
    /* The isometric shop occupies the solid (21,11) facade. Facing it
       from its adjacent walkable tile opens an OPTIONAL work interaction.
       No collision changes, new flags in SRAM, or mandatory job gate. */
    if(s->x==20u && s->y==11u && s->facing==0u){
        if(s->rescue_stage==STORY_RESCUE_LOST ||
           (s->rescue_stage>=STORY_RESCUE_MISSING &&
            s->rescue_stage<=STORY_RESCUE_TRACE)){
            talk(s,D_SHIFT_PAUSED);return;
        }
        if(!can_make_memory(s))talk(s,D_SHIFT_EARLY);
        else if(s->daily_mask&STORY_DAY_SHIFT)talk(s,D_SHIFT_REPEAT);
        else if(!s->energy)talk(s,D_SHIFT_TIRED);
        else moment_menu(s,'J'); /* WORK / LEAVE, no automatic cost */
        /* Repeated visits never replay the whole aerial approach. */
        s->cinema_frame=184u;
        return;
    }
    int episode=forced?-1:story_neighborhood_at(s);
    if(episode>=0){talk(s,(uint8_t)(D_NB_0_INTRO+episode*3));return;}
    char c=forced?forced:nearby_actor(s);
    switch (c) {
    case 'L':
        if(can_make_memory(s) && !(s->daily_mask&STORY_DAY_LETTER))moment_menu(s,'L');
        else talk(s,(s->flags & STORY_HOME) ? D_LETTER_AGAIN : D_LETTER);
        if (!(s->flags & STORY_HOME)) { s->flags |= STORY_HOME; s->dirty = 1; }
        break;
    case 'C':
        if (!(s->flags & STORY_HOME)) talk(s, D_CLERK_LOCKED);
        else {
            if(can_make_memory(s) && !(s->daily_mask&STORY_DAY_CLERK))moment_menu(s,'C');
            else if((!s->life_stage||can_make_memory(s)) &&
                    s->song_steps>=STORY_PROJECT_DONE)
                talk(s,D_CLERK_SONG);
            else if(can_make_memory(s) && day_period(s)==2u)
                talk(s,D_CLERK_CLOSING);
            else talk(s,(s->flags & STORY_STAMP) ? D_CLERK_AGAIN : D_CLERK);
            if (!(s->flags & STORY_STAMP)) { s->flags |= STORY_STAMP; s->dirty = 1; }
        }
        break;
    case 'F':
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,s->day_number>=STORY_HOPE_REALIZE_DAY?
                 D_HOPE_PHONE_LATE:D_HOPE_PHONE_WAIT);
            break;
        }
        if(s->rescue_stage>=STORY_RESCUE_MISSING &&
           s->rescue_stage<=STORY_RESCUE_TRACE){
            talk(s,D_MARA_ABSENT);break;
        }
        if (!(s->flags & STORY_STAMP)) talk(s, D_MARA_LOCKED);
        else {
            /* The Promise checkmark is granted only after the actual
               promise scene, never from small talk or a menu. */
            if (!(s->flags & STORY_MARA)) {
                /* The promise is not granted until the player actually
                   finishes and confirms Danny's answer with A. */
                talk(s,D_MARA);
            } else if(creative_tracking(s)&&s->neighborhood_target==STORY_TRACK_STUDIO&&share_pending(s))
                talk(s,s->art_steps>=STORY_PROJECT_DONE?D_MARA_CREATIONS:D_MARA_MASTER);
            else if(can_make_memory(s) &&
                      !(s->daily_mask&STORY_DAY_MARA))moment_menu(s,'F');
            else if(story_bad_count(s)>=2)talk(s,D_MARA_WARNING);
            else if(story_bad_count(s)==1)talk(s,D_MARA_CONCERN);
            else if(!s->life_stage && !(s->daily_mask&STORY_PREBUS_MARA))
                moment_menu(s,'f');
            else if(!s->life_stage && s->song_steps>=STORY_PROJECT_DONE &&
                    s->art_steps>=STORY_PROJECT_DONE)talk(s,D_MARA_CREATIONS);
            else if((!s->life_stage||can_make_memory(s)) &&
                    s->art_steps>=STORY_PROJECT_DONE)talk(s,D_MARA_GALLERY);
            else if((!s->life_stage||can_make_memory(s)) && s->song_steps>=STORY_PROJECT_DONE)
                talk(s,(s->project_style&STORY_STYLE_SONG_SHARED)?D_MARA_MASTER_SHARED:D_MARA_MASTER);
            else if(can_make_memory(s) && s->bond>=4)talk(s,D_ACT_MARA_CLOSE);
            else if(can_make_memory(s) && day_period(s)==0u)talk(s,D_MARA_MORNING);
            else if(can_make_memory(s) && day_period(s)==2u)talk(s,D_MARA_CLOSING);
            else talk(s,D_MARA_AGAIN);
        }
        break;
    case 'B':
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,s->day_number>=STORY_HOPE_REALIZE_DAY?
                 D_HOPE_NOTE_LATE:D_HOPE_NOTE_WAIT);
            break;
        }
        if(s->rescue_stage>=STORY_RESCUE_MISSING &&
           s->rescue_stage<=STORY_RESCUE_TRACE)talk(s,D_MARA_MISSING);
        else if((s->flags&STORY_ALL)==STORY_ALL &&
             (!s->life_stage ||
              (s->rescue_stage==STORY_RESCUE_SAVED &&
               s->life_stage<=STORY_LIFE_FOLLOWUP_WAIT)))
             talk(s,(s->flags&STORY_FULL)==STORY_FULL?
                  D_BUS_READY_LEGACY:D_BUS_READY);
        else talk(s,D_BUS_LOCKED);
        break;
    case 'M':
        if(s->rescue_stage>=STORY_RESCUE_MISSING &&
           s->rescue_stage<=STORY_RESCUE_TRACE){
            talk(s,D_MARA_MISSING); /* no cheerful studio work during rescue */
        }else if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,s->day_number>=STORY_HOPE_REALIZE_DAY?
                 D_HOPE_NOTE_LATE:D_HOPE_NOTE_WAIT);
        }else if(s->song_steps>=STORY_PROJECT_DONE){
            moment_menu(s,'E'); /* LISTEN or the unlocked full editor */
        }else if(can_make_memory(s)){
            if(!(s->daily_mask&STORY_DAY_MUSIC))moment_menu(s,'M');
            else moment_menu(s,'m');
        }else if(!(s->flags & STORY_MUSIC)){
            s->flags |= STORY_MUSIC;s->dirty=1;talk(s,D_MUSIC);
        }else if(!s->life_stage)moment_menu(s,'m');
        else talk(s,s->work>=8?D_ACT_MUSIC_PROGRESS:D_MUSIC_AGAIN);
        break;
    case 'R':
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,s->day_number>=STORY_HOPE_REALIZE_DAY?
                 D_HOPE_NOTE_LATE:D_HOPE_NOTE_WAIT);
            break;
        }
        /* The missing-person rescue takes priority even after the last
           clinic visit; the player cannot accidentally bypass the crisis. */
        if(s->rescue_stage>=STORY_RESCUE_MISSING &&
           s->rescue_stage<=STORY_RESCUE_TRACE){
            talk(s,s->rescue_stage==STORY_RESCUE_MISSING?D_RESCUE_NOTES_1:
                   s->rescue_stage==STORY_RESCUE_WITNESS?D_RESCUE_NOTES_2:
                   D_RESCUE_NOTES_3);
            break;
        }
        if (s->life_stage == STORY_LIFE_LAST_WORK) {
            s->life_stage=STORY_LIFE_FINALE;s->finale_frame=0;
            s->scene=STORY_SCENE_NONE;s->dirty=1;
            /* A timed in-scene subtitle replaces the blocking opening dialogue.
               The music and animation therefore start together even if nobody
               presses A for the next 72 seconds. */
            s->dialogue=0;s->page=0;s->cinema=CINE_FINALE;s->cinema_frame=0;
        } else if (s->life_stage == STORY_LIFE_FINALE) {
            /* The final cutscene is already playing: no restarting it. */
        } else if (s->life_stage == STORY_LIFE_CLINIC ||
                   s->life_stage == STORY_LIFE_FINAL_CLINIC) {
            talk(s,D_ART_LAST_LOCKED);
        } else if (s->life_stage == STORY_LIFE_WAIT ||
                   s->life_stage == STORY_LIFE_FOLLOWUP_WAIT) {
            if(!(s->daily_mask&STORY_DAY_ART))moment_menu(s,'R');
            else if(s->art_steps<STORY_PROJECT_DONE)moment_menu(s,'r');
            else moment_menu(s,STORY_ACT_ROOF_REST);
        } else if (!(s->flags & STORY_ART)) {
            s->flags |= STORY_ART;s->dirty=1;talk(s,D_ART);
        } else if(!s->life_stage && s->art_steps<STORY_PROJECT_DONE)
            moment_menu(s,'r');
        else if(s->art_steps>=STORY_PROJECT_DONE)talk(s,D_PROJECT_ART_DONE);
        else talk(s,D_ART_AGAIN);
        break;
    case 'K':
        if(s->rescue_stage>=STORY_RESCUE_MISSING &&
           s->rescue_stage<=STORY_RESCUE_TRACE){
            talk(s,D_CLINIC_RESCUE);break;
        }
        if (s->life_stage == STORY_LIFE_CLINIC) {
            s->life_stage=STORY_LIFE_FOLLOWUP_WAIT;
            s->scene=STORY_SCENE_CLINIC;s->dirty=1;talk(s,D_DOCTOR_FIRST);
        } else if (s->life_stage == STORY_LIFE_FINAL_CLINIC) {
            s->life_stage=STORY_LIFE_LAST_WORK;
            s->scene=STORY_SCENE_CLINIC;s->dirty=1;
            talk(s,s->rescue_stage==STORY_RESCUE_LOST?
                 D_DOCTOR_LAST_ALONE:D_DOCTOR_LAST);
        } else talk(s,D_CLINIC_LOCKED);
        break;
    case 'P': cult_start(s, 0); break;
    case 'V': cult_start(s, 1); break;
    case 'Q': cult_start(s, 2); break;
    case 'S':
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,D_HOPE_TERMINAL);break;
        }
        if(s->rescue_stage==STORY_RESCUE_WITNESS){
            s->rescue_stage=STORY_RESCUE_TRACE;s->dirty=1;
            talk(s,D_RESCUE_TRACE);break;
        }
        if (s->cult_stage == 0) { s->cult_stage = 1; talk(s, D_CULT_FIND); s->dirty = 1; }
        else if (s->cult_stage == 3) { s->cult_stage = CULT_RESOLVED; talk(s, D_CULT_BACKUP); s->dirty = 1; }
        else talk(s, s->cult_stage >= CULT_RESOLVED ? D_CULT_AFTER :
                  s->cult_stage == 1 ? D_CULT_LOGS : D_CULT_WAIT);
        break;
    case 'W':
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,D_HOPE_WITNESS);break;
        }
        if(s->rescue_stage==STORY_RESCUE_MISSING){
            s->rescue_stage=STORY_RESCUE_WITNESS;s->dirty=1;
            talk(s,story_check_live(s,STORY_CHECK_ORCHARD)?
                 D_RESCUE_WITNESS_TRUST:D_RESCUE_WITNESS);break;
        }
        if (s->cult_stage == 1) { s->cult_stage = 2; talk(s, D_CULT_WITNESS); s->dirty = 1; }
        else talk(s, s->cult_stage > 1 ? D_CULT_WITNESS_AFTER : D_CULT_NOT_YET);
        break;
    case 'H':
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,D_HOPE_DOOR);break;
        }
        if(s->rescue_stage==STORY_RESCUE_TRACE){
            s->risk_menu='H';s->risk_choice=0;
            s->cinema=CINE_RESCUE_DOOR;s->cinema_frame=0;break;
        }
        if(s->rescue_stage==STORY_RESCUE_SAVED){
            rescue_reunion(s);break;
        }
        if (s->cult_stage == 2) { s->cult_stage = 3; talk(s, D_CULT_SAFETY); s->dirty = 1; }
        else talk(s, D_CULT_SAFE_AGAIN);
        break;
    default:
        /* Standing still is an invitation to notice the neighborhood,
           not a dead control. Geography decides the short observation. */
        if(story_city_extension(s->x,s->y)){
            if(s->y<14u)talk(s,D_CITY_EAST);
            else if(s->y<20u&&s->x>=24u)talk(s,D_CITY_CIVIC);
            else if(s->x<15u)talk(s,D_CITY_PARK);
            else if(s->x<25u)talk(s,D_CITY_CANAL);
            else talk(s,D_CITY_PLAZA);
        }else if(s->x>=17u && s->y>=9u && s->y<=13u)
            talk(s,D_LOOK_POTHEADS);
        else if(s->x>=15u && s->y>=13u)
            talk(s,D_LOOK_CANAL);
        else if(s->x<=7u && s->y>=8u)
            talk(s,D_LOOK_MARKET);
        else if(s->y<=7u)
            talk(s,D_LOOK_NORTH);
        else talk(s,D_LOOK_STREET);
        break;
    }
}
void story_tick(Story *s, uint16_t keys) {
    /* Use the same key-bit convention as REG_KEYINPUT, after inversion. */
    enum { A=1, B=2, SELECT=4, START=8, RIGHT=16, LEFT=32, UP=64, DOWN=128, R=256, L=512 };
    uint16_t pressed = (uint16_t)(keys & ~s->old_keys);
    s->old_keys = keys;
    if (s->anim) --s->anim;
    /* The fourth violation after a SUCCESSFUL rescue retains its original
       immediate-bad-ending consequence. A failed rescue now has a playable,
       two-day unanswered-phone epilogue with no assumed death. */
    if(!s->dialogue&&!s->ended&&
       s->rescue_stage==STORY_RESCUE_AFTERSHOCK)
        begin_bad_film(s);
    if(s->rescue_stage==STORY_RESCUE_LOST && !s->ended &&
       !s->paused && !s->dialogue && !s->battle &&
       !s->choosing && !s->risk_menu && !s->activity){
        if(++s->day_frame>=STORY_DAY_FRAMES){
            s->day_frame=0;
            if(s->day_number<STORY_HOPE_END_DAY)++s->day_number;
            s->dirty=1;
            if(s->day_number==STORY_HOPE_REALIZE_DAY)
                talk(s,D_HOPE_REALIZE);
            else if(s->day_number>=STORY_HOPE_END_DAY)
                begin_bad_film(s);
        }
    }
    if (s->cinema && !s->paused && s->cinema_frame < 65530) ++s->cinema_frame;
    /* One step is exactly one VBlank, including dialogue. Pause freezes the
       clock. Day transitions never steal controls during a fight or textbox. */
    if (!s->ended && s->life_stage >= STORY_LIFE_WAIT &&
        s->life_stage <= STORY_LIFE_LAST_WORK && !s->paused &&
        s->activity!=STORY_ACT_STUDIO&&s->activity!=STORY_ACT_REHEARSAL&&
        s->activity!=STORY_ACT_PAINT) {
        if (++s->day_frame >= STORY_DAY_FRAMES) {
            s->day_frame=0;
            if (s->day_number<99) ++s->day_number;
            s->daily_mask=0; /* optional moments reset every morning */
            stat_gain(&s->energy,2);
            s->dirty=1;
        }
        if (!s->dialogue && !s->battle && !s->choosing && !s->activity && !s->risk_menu) {
            if(s->rescue_stage>=STORY_RESCUE_MISSING &&
               s->rescue_stage<=STORY_RESCUE_TRACE){
                /* Appointments stay on the calendar, without scenes that
                   incorrectly show Mara while she is missing. */
            }else if (s->life_stage==STORY_LIFE_WAIT && s->day_number>=2) {
                s->life_stage=STORY_LIFE_CLINIC;
                s->scene=STORY_SCENE_NOTICE;s->dirty=1;talk(s,D_MEDICAL_NOTICE);
            } else if (s->life_stage==STORY_LIFE_FOLLOWUP_WAIT && s->day_number>=3) {
                s->life_stage=STORY_LIFE_FINAL_CLINIC;
                s->scene=STORY_SCENE_NOTICE;s->dirty=1;
                talk(s,s->rescue_stage==STORY_RESCUE_LOST?
                     D_DOCTOR_RETURN_ALONE:D_DOCTOR_RETURN);
            } else if(can_make_memory(s) &&
                      (s->daily_mask&STORY_DAY_ALL)==STORY_DAY_ALL &&
                      !(s->daily_mask&STORY_DAY_RECAP)) {
                s->daily_mask|=STORY_DAY_RECAP;s->dirty=1;
                talk(s,s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?
                    D_DAY_RECAP_AFTER:D_DAY_RECAP);
            }
        }
    }
    /* Ending pictures are authored over the COMPLETE score, not just the
       original 450-frame animation. Keep one shot clock through the chorus,
       verse returns and the good-only coda. Input cannot accidentally skip
       the performance or leave the camera on a static credit card. */
    if (!s->dialogue && (
          (s->life_stage==STORY_LIFE_FINALE && !s->ended) ||
          (s->ended && ((s->ending==STORY_END_GOOD &&
           s->finale_frame<STORY_GOOD_FINAL_FRAMES) ||
           (s->ending==STORY_END_BAD &&
           s->finale_frame<STORY_END_SCORE_FRAMES))))) {
        unsigned limit=s->ending==STORY_END_GOOD?
            STORY_GOOD_FINAL_FRAMES:STORY_END_SCORE_FRAMES;
        if (s->finale_frame<limit) ++s->finale_frame;
        if (s->finale_frame>=limit) {
            if (s->ending==STORY_END_GOOD)
                talk(s,story_bad_count(s)?D_GOOD_END_SCARS:D_GOOD_END);
            else if(s->ending==STORY_END_BAD)
                talk(s,s->rescue_stage==STORY_RESCUE_AFTERSHOCK?
                     D_BAD_END_SAVED:D_BAD_END);
            else {
                s->ended=1;s->ending=STORY_END_NEUTRAL;
                talk(s,D_NEUTRAL_END);
            }
            s->dirty=1;
        }
        return;
    }
    /* Combat takes priority over exploration, so toggling 2D/3D cannot evade a hit. */
    if (s->battle) { cult_tick(s, pressed); return; }
    if (s->dialogue) {
        int episode=neighborhood_dialogue(s->dialogue);
        if(episode>=0&&(pressed&(A|B))){
            int count;story_pages(s,&count);
            if(pressed&B){s->dialogue=0;s->page=0;s->cinema=CINE_NONE;}
            else if(s->page+1u<(unsigned)count)++s->page;
            else{
                s->dialogue=0;s->page=0;
                if(episode%3==0 && story_neighborhood_at(s)==episode/3){
                    s->activity=STORY_ACT_NEIGHBORHOOD;
                    s->activity_choice=0;s->cinema=CINE_DAY;s->cinema_frame=0;
                }else{
                    if(episode%3!=0){
                        unsigned bit=1u<<(unsigned)(episode/3);
                        s->neighborhood_seen=(uint16_t)(s->neighborhood_seen|bit);
                        if(episode%3==2)s->neighborhood_right=(uint16_t)(s->neighborhood_right|bit);
                        else s->neighborhood_right=(uint16_t)(s->neighborhood_right&~bit);
                        if(s->neighborhood_target==(unsigned)(episode/3)+1u)s->neighborhood_target=0;
                        s->dirty=1;
                    }
                    s->cinema=CINE_NONE;
                }
            }
            return;
        }
        if (pressed & (A|B)) {
            int count; story_pages(s, &count);
            if ((pressed & A) && !(pressed & B) && s->page + 1 < count) {
                ++s->page;
                /* The opening is editorially cut to seven spoken beats.
                   Fast readers may advance, but each line then gets its
                   intended visible camera pose instead of skipping the
                   entire storefront reveal and dance. The scene clock
                   remains monotonic; B can still dismiss the dialogue. */
                uint16_t beat=dialogue_shot_beat(s->dialogue,s->page);
                if(s->cinema_frame<beat)s->cinema_frame=beat;
            }
            else {
                int ready = (s->dialogue==D_BUS_READY ||
                             s->dialogue==D_BUS_READY_LEGACY) &&
                            (pressed&A) && !(pressed&B);
                /* B skips an unfinished conversation; no promise check.
                   Only A on Danny's final spoken answer earns it. */
                if(s->dialogue==D_MARA && (pressed&A) && !(pressed&B) &&
                   !(s->flags&STORY_MARA)){
                    s->flags|=STORY_MARA;s->dirty=1;
                }
                if((s->dialogue==D_MARA_MASTER||s->dialogue==D_MARA_CREATIONS)&&
                   (pressed&A)&&!(pressed&B)&&share_pending(s)){
                    s->project_style|=STORY_STYLE_SONG_SHARED;
                    stat_gain(&s->bond,1);s->dirty=1;
                    if(s->neighborhood_target==STORY_TRACK_STUDIO)s->neighborhood_target=0;
                }
                if(s->dialogue==D_SHOW_PLAY&&(pressed&A)&&!(pressed&B)&&story_show_stage(s)==5u){
                    show_advance(s,6u);
                    if(!(s->project_style&STORY_STYLE_SONG_SHARED)){
                     s->project_style|=STORY_STYLE_SONG_SHARED;stat_gain(&s->bond,1);
                    }
                }
                s->dialogue = 0; s->page = 0; s->scene=STORY_SCENE_NONE;
                if(!ready && !s->ended && s->life_stage!=STORY_LIFE_FINALE)s->cinema=CINE_NONE;
                if (ready) { s->choosing = 1; s->choice = 0; }
            }
        }
        return;
    }
    if (s->ended) {
        /* An ending is a stable scene, not a doorway back into gameplay. */
        if (pressed & START) s->quit=1;
        return;
    }
    if(s->risk_menu){
        if(pressed&(LEFT|RIGHT|UP|DOWN))s->risk_choice^=1u;
        if(pressed&B){s->risk_menu=0;s->risk_choice=0;s->cinema=CINE_NONE;}
        else if(pressed&A)resolve_risk(s);
        return;
    }
    if(s->activity==STORY_ACT_PAINT){
        /* Brush location and palette are transient. Only START on a
           completed layer changes the saved artwork. No timed failure. */
        if(pressed&B){
            s->activity=0;s->activity_choice=0;s->studio_tab=s->studio_cursor=0;
            s->cinema=CINE_NONE;return;
        }
        if(pressed&START){
            if(!story_paint_ready(s)){s->studio_cursor=2u;return;}
            unsigned warm=(unsigned)story_paint_warm(s);
            s->activity=0;s->activity_choice=0;s->studio_tab=s->studio_cursor=0;
            s->project_style=(uint8_t)((s->project_style&~STORY_STYLE_ART_WARM)|
                                       (warm?STORY_STYLE_ART_WARM:0u));
            remember(s,STORY_DAY_ART);step_project(s,'r',warm);return;
        }
        unsigned brush=story_paint_brush(s),tone=s->activity_choice&STORY_PAINT_WARM;
        if(pressed&LEFT){if(brush&3u)brush--;}
        else if(pressed&RIGHT){if((brush&3u)<3u)brush++;}
        else if(pressed&UP){if(brush>=4u)brush-=4u;}
        else if(pressed&DOWN){if(brush<4u)brush+=4u;}
        if(pressed&(SELECT|L|R))tone^=STORY_PAINT_WARM;
        s->activity_choice=(uint8_t)(tone|brush);
        if(pressed&(LEFT|RIGHT|UP|DOWN|SELECT|L|R))s->studio_cursor=0;
        if(pressed&A){
            unsigned bit=1u<<brush;
            if(story_paint_targets(s)&bit){s->studio_tab|=(uint8_t)bit;s->studio_cursor=0;}
            else s->studio_cursor=1;
        }
        return;
    }
    if(s->activity==STORY_ACT_REHEARSAL){
        /* Four untimed cue cards use the actual arrangement order.
           A wrong choice gives feedback; B cancels without a penalty. */
        if(pressed&(B|START)){s->activity=0;s->activity_choice=0;s->cinema=CINE_NONE;return;}
        if(pressed&(LEFT|UP)){s->activity_choice=(uint8_t)((s->activity_choice+3u)&3u);s->studio_cursor=0;}
        else if(pressed&(RIGHT|DOWN)){s->activity_choice=(uint8_t)((s->activity_choice+1u)&3u);s->studio_cursor=0;}
        if(pressed&A){
         if(s->activity_choice!=story_show_expected(s))s->studio_cursor=1;
         else if(++s->studio_tab>=4u){
          s->activity=0;s->activity_choice=0;s->studio_tab=s->studio_cursor=0;
          if(show_available(s)&&story_show_stage(s)==2u){
           show_advance(s,3u);talk(s,D_SHOW_REHEARSED);
          }else s->cinema=CINE_NONE;
         }else {s->activity_choice=0;s->studio_cursor=0;}
        }
        return;
    }
    if(s->activity==STORY_ACT_STUDIO){
        /* Easy enough to play entirely with the D-pad and face buttons.
           L/R switch pages, D-pad edits the current setting, SELECT
           previews the *actual* PSG notes, B exits without losing work. */
        int basic=s->song_steps<STORY_PROJECT_DONE;
        if(pressed&(B|START)){
            s->activity=STORY_ACT_NONE;s->activity_choice=0;s->cinema=CINE_NONE;
            if((pressed&START)&&!(pressed&B)){
                if(basic){remember(s,STORY_DAY_MUSIC);step_project(s,'m',studio_patch()->mood==0u);}
                else s->paused=1;
            }
            return;
        }
        if(pressed&(L|R)){
            if(basic){
                static const uint8_t tabs[4]={STUDIO_SONG,STUDIO_NOTES,STUDIO_MIX,STUDIO_PERFORM};
                unsigned pos=s->studio_tab==STUDIO_NOTES?1u:s->studio_tab==STUDIO_MIX?2u:s->studio_tab==STUDIO_PERFORM?3u:0u;
                s->studio_tab=tabs[(pos+((pressed&L)?3u:1u))%4u];
            }else s->studio_tab=(uint8_t)((s->studio_tab+((pressed&L)?STUDIO_TAB_COUNT-1u:1u))%STUDIO_TAB_COUNT);
            s->studio_cursor=0;
        }
        unsigned count=studio_cursor_count(s->studio_tab);
        if(pressed&LEFT)
            s->studio_cursor=(uint8_t)((s->studio_cursor+count-1u)%count);
        if(pressed&RIGHT)
            s->studio_cursor=(uint8_t)((s->studio_cursor+1u)%count);
        if(pressed&UP){
            studio_change(s->studio_tab,s->studio_cursor,1);
            s->dirty=1;
        }else if(pressed&DOWN){
            studio_change(s->studio_tab,s->studio_cursor,-1);
            s->dirty=1;
        }
        if(pressed&A){
            if(s->studio_tab==STUDIO_PERFORM)
                s->activity_choice^=1u;
            else {
                studio_toggle(s->studio_tab,s->studio_cursor);
                s->dirty=1;
            }
        }
        if(pressed&SELECT)s->activity_choice^=1u;
        return;
    }
    if(s->activity){
        if(s->activity=='m'||s->activity=='M'||story_paint_workshop_available(s)){
            if(pressed&(LEFT|UP))s->activity_choice=(uint8_t)((s->activity_choice+2u)%3u);
            else if(pressed&(RIGHT|DOWN))s->activity_choice=(uint8_t)((s->activity_choice+1u)%3u);
        }else if(pressed&(LEFT|RIGHT|UP|DOWN))s->activity_choice^=1u;
        if(pressed&B){
            s->activity=STORY_ACT_NONE;s->activity_choice=0;
            s->cinema=CINE_NONE;
        }else if(pressed&A)finish_moment(s);
        return;
    }
    if (s->choosing) {
        if (pressed & (LEFT|RIGHT|UP|DOWN)) s->choice ^= 1;
        if (pressed & B) {
            s->choosing=0;s->choice=0;s->cinema=CINE_NONE;
            return; /* cancel wins when A and B arrive on the same tick */
        }
        if (pressed & A) {
            s->choosing=0;
            if (s->choice==0 && (s->flags & STORY_FULL)==STORY_FULL) {
                s->ended=1;s->ending=STORY_END_GOOD;
                /* An early happy-route choice after a rescue must stop the
                   numbered-day clinic clock during the complete ending film. */
                s->life_stage=STORY_LIFE_NONE;s->scene=STORY_SCENE_NONE;
                s->finale_frame=0;s->dialogue=0;s->page=0;
                s->cinema=CINE_GOOD;s->cinema_frame=0;
            } else if (s->choice==0) {
                /* Adequate progress, but not 100%: keep living and crafting.
                   The neutral ending is delayed by active, numbered game days. */
                if(s->life_stage){
                    talk(s,D_BUS_RESCUE);
                }else{
                    s->life_stage=STORY_LIFE_WAIT;
                    s->day_number=1;s->day_frame=0;
                    s->daily_mask=(uint8_t)(s->daily_mask&~STORY_PREBUS_MARA);
                    talk(s,D_NEUTRAL_START);
                }
            } else {
                /* Continue at the bus stop, not an inexplicable rooftop cut. */
                talk(s,D_BUS_STAY_OPEN);
            }
            s->dirty=1;
        }
        return;
    }
    if(pressed&START){
        s->paused^=1u;
        /* The journal is transient UI, never part of a bus decision. */
        if(!s->paused)s->choice&=1u;
        return;
    }
    if(s->paused){
        if(pressed&SELECT){
            s->choice=(uint8_t)((s->choice&STORY_NEIGHBORHOOD_JOURNAL)?
                (s->choice&1u):((s->choice&1u)|STORY_NEIGHBORHOOD_JOURNAL));
            return;
        }
        if(s->choice&STORY_NEIGHBORHOOD_JOURNAL){
            if(pressed&B){s->choice&=1u;return;}
            if(pressed&(UP|LEFT))
                s->neighborhood_cursor=(uint8_t)((s->neighborhood_cursor+13u)%14u);
            else if(pressed&(DOWN|RIGHT))
                s->neighborhood_cursor=(uint8_t)((s->neighborhood_cursor+1u)%14u);
            else if((pressed&A)&&story_neighborhood_enabled(s)){
                unsigned target=s->neighborhood_cursor+1u;
                s->neighborhood_target=(uint8_t)(s->neighborhood_target==target?0u:target);
                s->dirty=1;s->paused=0;s->choice&=1u;
            }
            return;
        }
        /* A directly opens destinations from the pause root. */
        if((pressed&A)&&!(s->choice&(STORY_CHECK_PAGE_ACTIVE|STORY_FIELD_GUIDE_ACTIVE))){
            s->choice=(uint8_t)((s->choice&1u)|STORY_FIELD_GUIDE_ACTIVE|STORY_FIELD_GUIDE_DETAIL);
            return;
        }
        /* L is a direct two-page help guide. R remains the four-page
           consequences journal; both are transient and never save. */
        if(pressed&L){
            if(s->choice&STORY_FIELD_GUIDE_ACTIVE)s->choice&=1u;
            else s->choice=(uint8_t)((s->choice&1u)|
                                      STORY_FIELD_GUIDE_ACTIVE);
        }else if((pressed&(LEFT|RIGHT)) &&
                 (s->choice&STORY_FIELD_GUIDE_ACTIVE)){
            s->choice^=STORY_FIELD_GUIDE_DETAIL;
        }else if((s->choice&(STORY_FIELD_GUIDE_ACTIVE|STORY_FIELD_GUIDE_DETAIL))==
                 (STORY_FIELD_GUIDE_ACTIVE|STORY_FIELD_GUIDE_DETAIL)){
            unsigned selected=story_track_selected(s);
            if(pressed&B)s->choice&=1u;
            else if(pressed&R)s->choice=(uint8_t)((s->choice&1u)|STORY_CHECK_PAGE_ACTIVE);
            else if(pressed&(UP|DOWN)){
                selected=(selected+((pressed&UP)?STORY_TRACK_COUNT-1u:1u))%STORY_TRACK_COUNT;
                s->choice=(uint8_t)((s->choice&~STORY_TRACK_CHOICE_MASK)|(selected<<1));
            }else if((pressed&A)&&story_track_available(s,selected)){
                s->neighborhood_target=(uint8_t)story_track_id(selected);
                s->dirty=1;s->paused=0;s->choice&=1u;
            }
        }else if(pressed&R){
            unsigned page=(s->choice&STORY_CHECK_PAGE_ACTIVE)?
                (story_check_page(s)+1u)%STORY_CHECK_PAGE_COUNT:0u;
            s->choice=(uint8_t)((s->choice&1u)|STORY_CHECK_PAGE_ACTIVE|
                                (page<<1));
        }else if(pressed&B){
            if(s->choice&(STORY_CHECK_PAGE_ACTIVE|STORY_FIELD_GUIDE_ACTIVE))
                s->choice&=1u;
            else s->quit=1;
        }
        return;
    }
    if (pressed & (L|SELECT)) { s->view ^= 1; s->dirty = 1; }
    if (pressed & R) {
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,s->day_number>=STORY_HOPE_REALIZE_DAY?
                 D_HOPE_NOTE_LATE:D_HOPE_NOTE_WAIT);return;
        }
        if(s->rescue_stage==STORY_RESCUE_MISSING){
            talk(s,D_RESCUE_NOTES_1);return;
        }
        if(s->rescue_stage==STORY_RESCUE_WITNESS){
            talk(s,D_RESCUE_NOTES_2);return;
        }
        if(s->rescue_stage==STORY_RESCUE_TRACE){
            talk(s,D_RESCUE_NOTES_3);return;
        }
        if(s->rescue_stage==STORY_RESCUE_SAVED){
            talk(s,D_RESCUE_NOTES_SAVED);return;
        }
        int revisit=story_neighborhood_revisit(s);
        if(revisit>=0){
            unsigned right=(s->neighborhood_right>>(unsigned)revisit)&1u;
            talk(s,(uint8_t)(D_NB_RETURN_0_0+2u*(unsigned)revisit+right));return;
        }
        if (s->life_stage) {
            talk(s,s->life_stage==STORY_LIFE_CLINIC||s->life_stage==STORY_LIFE_FINAL_CLINIC?D_NOTE_VISIT:
                 s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?D_NOTE_FOLLOWUP:
                 s->life_stage>=STORY_LIFE_LAST_WORK?D_NOTE_CANVAS:D_DAY_NOTES);
            return;
        }
        if (s->cult_stage) {
            talk(s, (uint8_t)(D_CULT_NOTES_1 + s->cult_stage - 1));
            return;
        }
        talk(s, (s->flags & STORY_ALL)==STORY_ALL ? D_JOURNAL_3 :
             (s->flags & STORY_STAMP) ? D_JOURNAL_2 :
             (s->flags & STORY_HOME) ? D_JOURNAL_1 : D_JOURNAL_0);
        return;
    }
    if (pressed & A) { interact(s); return; }
    if(pressed&B){
        char target=nearby_actor(s);
        unsigned bit=target=='L'?STORY_BAD_LETTER:
                     target=='C'?STORY_BAD_CLERK:
                     target=='M'?STORY_BAD_MUSIC:
                     target=='S'?STORY_BAD_SERVER:0u;
        if(bit && story_risk_available(s)){
            s->risk_menu=(uint8_t)target;s->risk_choice=0;
            s->cinema=target=='L'?CINE_LETTER:
                target=='C'?CINE_BORE:
                target=='M'?CINE_MUSIC:CINE_SERVER;
            s->cinema_frame=0;
        }
        return;
    }
    if (s->cooldown) --s->cooldown;
    if (!s->cooldown) {
        int d = -1;
        if (s->view) {
            if (pressed & LEFT) { s->facing = (uint8_t)((s->facing+3)&3); s->cooldown = 5; s->dirty=1; }
            else if (pressed & RIGHT) { s->facing=(uint8_t)((s->facing+1)&3); s->cooldown=5; s->dirty=1; }
            else if (keys & UP) d = s->facing;
            else if (keys & DOWN) d = (s->facing+2)&3;
        } else {
            if (keys & UP) d = 3;
            else if (keys & DOWN) d = 1;
            else if (keys & LEFT) d = 2;
            else if (keys & RIGHT) d = 0;
            if (d >= 0) s->facing = (uint8_t)d;
        }
        if (d >= 0) {
            static const int dx[4] = {1,0,-1,0}, dy[4]={0,1,0,-1};
            int x = s->x + dx[d], y = s->y + dy[d];
            if (!story_solid(x,y)) {
                s->prev_x=s->x; s->prev_y=s->y; s->x=(uint8_t)x; s->y=(uint8_t)y;
                s->anim=4; s->dirty=1;
            }
            s->cooldown=6;
        }
    }
}

