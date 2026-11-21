#pragma once
#include <stdint.h>
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
#define STORY_END_GOOD 1
#define STORY_END_NEUTRAL 2
#define STORY_END_BAD 3 /* reserved for the later branching chapter */
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
    uint16_t day_frame, finale_frame;
    uint16_t old_keys;
} Story;

typedef struct { const char *speaker, *line1, *line2; } StoryPage;
void story_new(Story *s, int slot);
void story_resume(Story *s, int slot, int x, int y, int facing, int view, int flags, int ended, int ending);
void story_tick(Story *s, uint16_t keys);
char story_tile(int x, int y);
int story_solid(int x, int y);
const StoryPage *story_pages(const Story *s, int *count);

/* Deterministic enemy tells and waves: shared by logic, renderer and host tests. */
int story_attack_lane(const Story *s, int beat);
const char *story_enemy_name(const Story *s);
