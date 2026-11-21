#pragma once
#include <stdint.h>
#define STORY_W 24
#define STORY_H 18
#define STORY_HOME 1
#define STORY_STAMP 2
#define STORY_MARA 4
#define STORY_ALL (STORY_HOME | STORY_STAMP | STORY_MARA)

typedef struct {
    uint8_t x, y, facing, view, flags, ended, ending, slot;
    uint8_t dialogue, page, choice, choosing, paused, quit;
    uint8_t cooldown, prev_x, prev_y, anim, dirty;
    uint16_t old_keys;
} Story;

typedef struct { const char *speaker, *line1, *line2; } StoryPage;
void story_new(Story *s, int slot);
void story_resume(Story *s, int slot, int x, int y, int facing, int view, int flags, int ended, int ending);
void story_tick(Story *s, uint16_t keys);
char story_tile(int x, int y);
int story_solid(int x, int y);
const StoryPage *story_pages(const Story *s, int *count);
