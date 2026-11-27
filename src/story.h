#pragma once
#include <stdint.h>
#include "cinema.h"
#define STORY_CORE_W 24
#define STORY_CORE_H 18
#define STORY_CITY_W 40
/* All original 96 layouts retain their IDs. A second bit of parcel
   variation exchanges a whole park and built neighborhood. */
#define STORY_WORLD_BASE_LAYOUTS 96u
#define STORY_WORLD_LAYOUTS 192u
#define STORY_W 64
#define STORY_H 30
#define STORY_LAND_PATCH0 1u
#define STORY_LAND_PATCH1 2u
#define STORY_LAND_SUMMIT 3u
#define STORY_LAND_SHORE 4u
#define STORY_LAND_TRAILHEAD 5u
#define STORY_LAND_WAYPOINT 6u
#define STORY_LAND_FINISH 7u
#define STORY_LAND_DEFAULT 5u
#define STORY_CITY_STOPS 5u
#define STORY_CITY_TOUR_DONE 5u
/* Half-resolution editable control lattice, 2 bits per node. The
   transient brush toggle is used only while freely roaming. */
#define STORY_BRUSH_COLS 13u
#define STORY_BRUSH_ROWS 16u
#define STORY_BRUSH_BYTES 52u
#define STORY_BRUSH_MODE 0x08u
#define STORY_SURVEY_MODE 0x04u
/* The pause debug menu uses the otherwise inactive detail bit; it never
   coexists with field-guide/journal/checklist flags. No new Story bytes. */
#define STORY_DEBUG_MENU 0x10u
/* In unpaused play the same bit authenticates an explicitly selected
   terrain tool; ordinary walking and all save records leave it clear. */
#define STORY_DEBUG_ACCESS STORY_DEBUG_MENU
#define STORY_DEBUG_CONFIRM 0x08u
#define STORY_DEBUG_SELECT_MASK 0x07u
#define STORY_DEBUG_ROWS 5u
static inline int story_rural(int x,int y){
 return x>=STORY_CITY_W&&x<STORY_W&&(unsigned)y<STORY_H;
}
/* The original material banks remain valid at their original coordinates. */
static inline int story_city_extension(int x,int y){
 return (unsigned)x<STORY_W&&(unsigned)y<STORY_H&&
        (x>=STORY_CORE_W||y>=STORY_CORE_H);
}
#define STORY_HOME 1
#define STORY_STAMP 2
#define STORY_MARA 4
#define STORY_ALL (STORY_HOME | STORY_STAMP | STORY_MARA)
#define STORY_MUSIC 8
#define STORY_ART 16
#define STORY_FULL (STORY_ALL | STORY_MUSIC | STORY_ART)
/* Each numbered day lasts four minutes at the GBA's 60Hz VBlank rate. */
#define STORY_DAY_FRAMES 14400
/* Five optional daily memories. The high bit marks the closing vignette;
   these fit the existing reserved SRAM bytes without changing slot size. */
#define STORY_DAY_MARA 1u
#define STORY_DAY_LETTER 2u
#define STORY_DAY_CLERK 4u
#define STORY_DAY_ART 8u
#define STORY_DAY_MUSIC 16u
/* Independent optional Potheads shift: bit 5 was unused in the persisted
   daily_mask byte. It never changes the five existing memory requirements. */
#define STORY_DAY_SHIFT 32u
/* Optional one-time pre-bus friendship scene, using the formerly unused
   bit of the existing persisted daily_mask. Not a numbered-day memory. */
#define STORY_PREBUS_MARA 64u
#define STORY_DAY_ALL 31u
#define STORY_DAY_RECAP 128u
#define STORY_STAT_MAX 5u
#define STORY_ACT_NONE 0u
/* The new advanced editor uses the existing activity menu routing.
   Its two tiny UI bytes remain transient; notes/order live in studio.c. */
#define STORY_ACT_STUDIO 'Z'
#define STORY_ACT_NEIGHBORHOOD 'N'
#define STORY_NEIGHBORHOOD_COUNT 14u
#define STORY_NEIGHBORHOOD_MASK 0x3fffu
#define STORY_NEIGHBORHOOD_JOURNAL 0x20u
/* Four authored stages per creative project. No project gates an ending. */
#define STORY_PROJECT_DONE 4u
#define STORY_STYLE_SONG_SOFT 1u
#define STORY_STYLE_ART_WARM 2u
#define STORY_STYLE_SONG_SHARED 4u
/* Three persisted exhibition-stage bits and two aesthetic choices.
   The story record remains the same 31 bytes, including old saves. */
#define STORY_SHOW_STAGE_MASK 0x38u
#define STORY_SHOW_WARM_POSTER 0x40u
#define STORY_SHOW_WARM_FRAME 0x80u
#define STORY_SHOW_DONE 7u
#define STORY_PROJECT_STYLE_MASK 0xffu
#define STORY_ACT_SHOW 'Y'
#define STORY_ACT_ROOF_REST 'G'
#define STORY_ACT_REHEARSAL 'U'
#define STORY_ACT_PAINT 'T'
#define STORY_PAINT_WARM 0x80u
#define STORY_TRACK_STUDIO 15u
#define STORY_TRACK_ROOF 16u
#define STORY_TRACK_ORCHARD 17u
#define STORY_TRACK_SHOW 18u
#define STORY_TRACK_MAX STORY_TRACK_SHOW
#define STORY_TRACK_COUNT 5u
#define STORY_TRACK_MAIN_INDEX 3u
#define STORY_TRACK_SHOW_INDEX 4u
#define STORY_TRACK_CHOICE_MASK 0x0eu
/* 72.5-second source song, then the good-only 16-second reprise. */
#define STORY_END_SCORE_FRAMES 4350u
#define STORY_END_CODA_FRAMES 960u
#define STORY_GOOD_FINAL_FRAMES (STORY_END_SCORE_FRAMES + STORY_END_CODA_FRAMES)
#define STORY_END_GOOD 1
#define STORY_END_NEUTRAL 2
#define STORY_END_BAD 3
#define STORY_BAD_LETTER 1u
#define STORY_BAD_CLERK 2u
#define STORY_BAD_MUSIC 4u
#define STORY_BAD_SERVER 8u
#define STORY_BAD_THRESHOLD 3u
#define STORY_RESCUE_NONE 0u
#define STORY_RESCUE_MISSING 1u
#define STORY_RESCUE_WITNESS 2u
#define STORY_RESCUE_TRACE 3u
#define STORY_RESCUE_SAVED 4u
#define STORY_RESCUE_LOST 5u
#define STORY_RESCUE_AFTERSHOCK 6u
/* After a failed rescue, an unverified promise is followed by one full
   numbered day of hope, an acknowledgement of no recovery, and one more
   full day before the pre-existing bad-ending film. The old day_number and
   day_frame save bytes are reused ONLY while STORY_RESCUE_LOST. */
#define STORY_HOPE_REALIZE_DAY 1u
#define STORY_HOPE_END_DAY 2u
#define STORY_LIFE_NONE 0
#define STORY_LIFE_WAIT 1
#define STORY_LIFE_CLINIC 2
#define STORY_LIFE_FOLLOWUP_WAIT 3
#define STORY_LIFE_FINAL_CLINIC 4
#define STORY_LIFE_LAST_WORK 5
#define STORY_LIFE_FINALE 6
/* Preserve paid-shift completion on reload. The first-evening marker is
   meaningful only before numbered days start. No save layout changes. */
static inline uint8_t story_restore_daily_mask(uint8_t raw,unsigned life_stage){
    unsigned valid=STORY_DAY_ALL|STORY_DAY_SHIFT|STORY_DAY_RECAP;
    if(life_stage==STORY_LIFE_NONE)valid|=STORY_PREBUS_MARA;
    return (uint8_t)(raw&valid);
}
#define STORY_SCENE_NONE 0
#define STORY_SCENE_CLINIC 1
#define STORY_SCENE_NOTICE 2
#define CULT_RESOLVED 4
#define CULT_ENCOUNTERS 3
#define BATTLE_NONE 0
#define BATTLE_MENU 1
#define BATTLE_DODGE 2
/* A readable half-second attack tell at 60 VBlanks. Four strikes make
   one wave, two waves make one duel. No extra battle or save fields. */
#define STORY_BATTLE_BEAT 30u
#define STORY_BATTLE_STRIKES 4u
/* FACTS rewards precise counters with a longer window and quicker
   recharge; CARE offers one free guarded strike, but its parry is tighter.
   Both modes retain the full 30-tick telegraph and four-focus limit. */
#define STORY_FACTS_PARRY_TICKS 15u
#define STORY_CARE_PARRY_TICKS 12u
#define STORY_FACTS_PARRY_COOLDOWN 24u
#define STORY_CARE_PARRY_COOLDOWN 30u
/* Legacy public constants remain for older playthrough/test clients. */
#define STORY_BATTLE_PARRY STORY_FACTS_PARRY_TICKS
#define STORY_BATTLE_PARRY_CD STORY_CARE_PARRY_COOLDOWN

typedef struct {
    uint8_t x, y, flags, ending;
    /* These values are bounded to 0..3 or 0..1 in gameplay. Pack them
       together to retain the existing sub-64-byte state budget. The SRAM
       codec serializes fields explicitly, never the struct layout. */
    uint8_t facing:2, view:1, ended:1, slot:1, choosing:1, paused:1, quit:1;
    uint8_t dialogue, page, choice;
    uint8_t cooldown, prev_x, prev_y, anim, dirty;
    /* A separate, OPTIONAL quest: persisted independently of Danny's main story. */
    uint8_t cult_stage, cult_pre_wins, cult_post_wins;
    /* Combat uses the existing bitmap. Only a few bytes of transient state. */
    uint8_t battle, cult_enemy, battle_choice, battle_lane, battle_focus;
    uint8_t battle_progress, battle_timer, battle_parry, battle_parry_cd, battle_guard;
    /* Numbered-day life sim, doctor visits and the unskippable last painting. */
    uint8_t day_number, life_stage, scene;
    uint8_t daily_mask, daily_memories; /* optional, persisted; never gates an ending */
    uint8_t energy, spark, bond, work; /* saved resources */
    uint8_t song_steps, art_steps, project_style; /* SRAM v5; stages 0..4 */
    uint8_t activity, activity_choice; /* transient daily choice menu */
    uint8_t studio_tab, studio_cursor; /* only valid in STUDIO mode */
    uint8_t bad_mask, rescue_stage; /* companion SRAM record */
    uint8_t risk_menu, risk_choice; /* confirmed ethical choices */
    uint16_t day_frame, finale_frame;
    /* Cinematic shots are transient; saves retain story flags, not camera frames. */
    uint8_t cinema;
    uint16_t cinema_frame;
    uint16_t old_keys;
    /* Independent companion save; tracked destination persists, cursor is transient. */
    uint16_t neighborhood_seen, neighborhood_right;
    uint8_t neighborhood_cursor:4, terrain_style:4;
    uint8_t neighborhood_target;
} Story;
/* Shared menu and gameplay predicate: an ordinary visit may earn a daily
   memory exactly once, regardless of whether a creative project is finished. */
static inline int story_daily_pending(const Story *s,unsigned bit){
    return (s->life_stage==STORY_LIFE_WAIT||s->life_stage==STORY_LIFE_FOLLOWUP_WAIT)&&
           !(s->daily_mask&bit);
}
static inline unsigned story_show_stage(const Story *s){return (s->project_style&STORY_SHOW_STAGE_MASK)>>3;}
static inline int story_paint_workshop_available(const Story *s){
 return (s->activity=='r'||s->activity=='R')&&s->art_steps<STORY_PROJECT_DONE;
}
static inline unsigned story_paint_targets(const Story *s){
 static const uint8_t layers[4]={0x0fu,0xf0u,0x66u,0x99u};
 return layers[s->art_steps<4u?s->art_steps:3u];
}
static inline int story_paint_ready(const Story *s){
 unsigned targets=story_paint_targets(s);
 return (s->studio_tab&targets)==targets;
}
static inline unsigned story_paint_brush(const Story *s){return s->activity_choice&7u;}
static inline int story_paint_warm(const Story *s){return !!(s->activity_choice&STORY_PAINT_WARM);}
const char *story_paint_layer(const Story *s);
unsigned story_track_id(unsigned choice);
const char *story_show_phrase(unsigned part);
unsigned story_show_expected(const Story *s);
const char *story_show_title(const Story *s);
const char *story_show_option(const Story *s,unsigned right);
int story_show_scene(const Story *s);

typedef struct { const char *speaker, *line1, *line2; } StoryPage;
void story_new(Story *s, int slot);
void story_resume(Story *s, int slot, int x, int y, int facing, int view, int flags, int ended, int ending);
void story_tick(Story *s, uint16_t keys);
char story_tile(int x, int y);
unsigned story_world_seed(void);
unsigned story_world_revision(void);
void story_world_set(unsigned seed);
void story_world_source(int *x,int *y);
void story_world_place(int *x,int *y);
unsigned story_world_facing(unsigned facing);
int story_world_potheads(int x,int y);
char story_world_tile_for_seed(unsigned seed,int x,int y);
void story_world_preview(unsigned seed,uint8_t out[STORY_H][STORY_W]);
int story_world_solid_for_seed(unsigned seed,int x,int y);
void story_new_seeded(Story *s,int slot,unsigned seed);
void story_resume_seeded(Story *s,int slot,int x,int y,int facing,int view,int flags,int ended,int ending,unsigned seed);
const char *story_district_name(int x,int y);
int story_terrain_vertex(const Story *s,int x,int y);
int story_terrain_height(const Story *s,int wx,int wy);
/* Frame-local immutable view; hot renderers acquire it once per frame. */
typedef uint8_t StoryTerrainRow[STORY_W-STORY_CITY_W+1];
const StoryTerrainRow *story_terrain_mesh(const Story *s);
static inline int story_mesh_height(const StoryTerrainRow *mesh,int wx,int wy){
 int x=wx>>8,y=wy>>8,fx=wx&255,fy=wy&255;
 if(x<STORY_CITY_W||x>=STORY_W||(unsigned)y>=STORY_H)return 0;
 unsigned col=(unsigned)(x-STORY_CITY_W);
 int a=mesh[y][col],b=mesh[y][col+1u],c=mesh[y+1][col],d=mesh[y+1][col+1u];
 return ((a*(256-fx)+b*fx)*(256-fy)+(c*(256-fx)+d*fx)*fy)>>16;
}
unsigned story_land_action(const Story *s);
unsigned story_land_level(const Story *s,unsigned patch);
int story_brush_mode(const Story *s);
int story_debug_menu(const Story *s);
int story_debug_survey(const Story *s);
int story_brush_editable(const Story *s);
unsigned story_brush_grade(const Story *s);
unsigned story_brush_revision(unsigned slot);
unsigned story_hike_stage(unsigned slot);
unsigned story_city_tour_stage(unsigned slot);
void story_city_tour_restore(unsigned slot,unsigned stage);
int story_city_site_position(unsigned id,int *x,int *y);
int story_city_tour_visible(const Story *s);
void story_hike_restore(unsigned slot,unsigned stage);
void story_brush_export(unsigned slot,uint8_t out[STORY_BRUSH_BYTES]);
int story_brush_import(unsigned slot,const uint8_t in[STORY_BRUSH_BYTES]);
#if !defined(__arm__)
/* Preserve the shipped map solely for historical golden-render checks. */
void story_debug_core_map(int enabled);
#endif
int story_solid(int x, int y);
/* World-space primary story objective. Returns zero only after an ending.
   Pure query: does not change the optional Mirror Orchard progression. */
int story_goal(const Story *s, int *x, int *y, const char **label);
/* Route to a WALKABLE tile next to the current objective, not straight
   through the wall containing it. Direction 0=E,1=S,2=W,3=N and -1=near.
   steps is the remaining movement count; queries never change save data. */
int story_route(const Story *s, int *direction, int *steps);
const char *story_goal_reason(const Story *s);
/* CORE, EXTRA, DAY, CARE, SAVE, HOPE, LAST or BUS; pure HUD tag. */
const char *story_goal_kind(const Story *s);
/* Exactly the same adjacent target priority as pressing A. */
const char *story_action_hint(const Story *s);
/* Visible prompts describe the actual next input, including commits. */
const char *story_dialogue_accept(const Story *s);
const char *story_dialogue_back(const Story *s);
const char *story_risk_action(const Story *s);
int story_risk_available(const Story *s); /* only at confirmed optional risk sites */
const StoryPage *story_pages(const Story *s, int *count);
int story_master_showcase(const Story *s);
int story_track_available(const Story *s,unsigned choice);
const char *story_track_name(unsigned choice);
unsigned story_track_selected(const Story *s);
int story_neighborhood_enabled(const Story *s);
int story_neighborhood_at(const Story *s);
/* Returns a saved-outcome site only while ordinary exploration is safe. */
int story_neighborhood_revisit(const Story *s);
int story_neighborhood_panel(const Story *s);
const char *story_neighborhood_name(unsigned id);
const char *story_neighborhood_option(unsigned id,unsigned right);
int story_neighborhood_position(unsigned id,int *x,int *y);
unsigned story_neighborhood_count(const Story *s);

/* Deterministic enemy tells and waves: shared by logic, renderer and host tests. */
int story_attack_lane(const Story *s, int beat);
const char *story_enemy_name(const Story *s);

int story_bad_count(const Story *s);
