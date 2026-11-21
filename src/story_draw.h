#pragma once
#include "story.h"
#include <stdint.h>
#define STORY_SCREEN_W 120
#define STORY_SCREEN_H 80
extern const uint16_t story_palette[32];
void story_draw_init(void);
void story_draw(const Story *s, int frame, uint8_t *pixels);
