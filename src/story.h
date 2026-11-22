#pragma once
#include <stdint.h>
#include "cinema.h"
#define STORY_W 24
#define STORY_H 18
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
#define STORY_DAY_ALL 31u
#define STORY_DAY_RECAP 128u
#define STORY_STAT_MAX 5u
#define STORY_ACT_NONE 0u
/* The new advanced editor uses the existing activity menu routing.
   Its two tiny UI bytes remain transient; notes/order live in studio.c. */
#define STORY_ACT_STUDIO 'Z'
/* Four authored stages per creative project. No project gates an ending. */
#define STORY_PROJECT_DONE 4u
#define STORY_STYLE_SONG_SOFT 1u
#define STORY_STYLE_ART_WARM 2u
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
#define STORY_SCENE_NONE 0
#define STORY_SCENE_CLINIC 1
#define STORY_SCENE_NOTICE 2
#define CULT_RESOLVED 4
#define CULT_ENCOUNTERS 3
#define BATTLE_NONE 0
#define BATTLE_MENU 1
#define BATTLE_DODGE 2
/* FACTS rewards accurate counters; CARE offers a forgiving first mistake.
   Windows are VBlank ticks, not variable-duration renderer frames. */
#define STORY_BATTLE_BEAT 30u
#define STORY_BATTLE_STRIKES 4u
#define STORY_FACTS_PARRY_TICKS 15u
#define STORY_CARE_PARRY_TICKS 12u
#define STORY_FACTS_PARRY_COOLDOWN 24u
#define STORY_CARE_PARRY_COOLDOWN 30u

typedef struct {
    uint8_t x, y, facing, view, flags, ended, ending, slot;
    uint8_t dialogue, page, choice, choosing, paused, quit;
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
} Story;

typedef struct { const char *speaker, *line1, *line2; } StoryPage;
void story_new(Story *s, int slot);
void story_resume(Story *s, int slot, int x, int y, int facing, int view, int flags, int ended, int ending);
void story_tick(Story *s, uint16_t keys);
char story_tile(int x, int y);
int story_solid(int x, int y);
/* World-space primary story objective. Returns zero only after an ending.
   Pure query: does not change the optional Mirror Orchard progression. */
int story_goal(const Story *s, int *x, int *y, const char **label);
/* Navigation follows walkable town tiles to a tile adjacent to the goal,
   not a misleading straight line through buildings. Direction: 0=E, 1=S,
   2=W, 3=N, 4=ARRIVED. Zero means no reachable active goal. The query
   never changes saved Story state, and its cached flood fill only rebuilds
   after a player step or objective change. */
int story_route(const Story *s, unsigned *direction, unsigned *steps);
/* Contextual controls and succinct reasons for the next suggestion. */
const char *story_action_hint(const Story *s);
const char *story_goal_reason(const Story *s);
int story_risk_available(const Story *s);
const StoryPage *story_pages(const Story *s, int *count);

/* Deterministic enemy tells and waves: shared by logic, renderer and host tests. */
int story_attack_lane(const Story *s, int beat);
const char *story_enemy_name(const Story *s);

int story_bad_count(const Story *s);
