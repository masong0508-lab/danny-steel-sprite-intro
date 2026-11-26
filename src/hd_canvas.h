#pragma once
#include <stdint.h>
#include "story.h"

/* Mode-4, one physical GBA pixel per painted pixel, double-buffered.
   The original story logic/8-bit frame remains available as a fallback. */
#define HD_W 240
#define HD_H 160
#define HD_BYTES (HD_W*HD_H)
enum {
 HD_ROAD=32,HD_ROAD_LIGHT,HD_ROAD_COOL,HD_WET,HD_MORTAR,HD_STONE,
 HD_STONE_LIT,HD_STONE_DARK,HD_BRICK_DARK,HD_BRICK,HD_BRICK_LIT,
 HD_PLASTER,HD_PLASTER_LIT,HD_SHADOW,HD_LAMP,HD_GLASS,HD_GLASS_LIT,
 HD_REFLECT,HD_COPPER,HD_LEAF_DARK,HD_LEAF,HD_LEAF_LIT,HD_BARK,
 HD_PAPER,HD_INK,HD_CHALK,HD_RED,HD_RED_SHADE,HD_SKIN,
 HD_SKIN_LIT,HD_HAIR,HD_GOLD
};
extern const uint16_t hd_extra_palette[32];
/* Mutually exclusive title/menu and gameplay share this one EWRAM page. */
extern uint32_t hd_shared_frame[HD_BYTES/4];
/* devkitARM's crt0 copies .iwram before main. Advertise both the ARM ISA
   and long-call boundary to every Thumb caller, not just the definitions.
   Host captures execute the same portable painters without attributes. */
#if defined(__arm__)
#define HD_CANVAS_CODE __attribute__((section(".iwram"),long_call,target("arm")))
#else
#define HD_CANVAS_CODE
#endif
HD_CANVAS_CODE void hd_rect(uint8_t *p,int x,int y,int w,int h,unsigned c);
HD_CANVAS_CODE void hd_pixel(uint8_t *p,int x,int y,unsigned c);
HD_CANVAS_CODE void hd_line(uint8_t *p,int x,int y,int xx,int yy,unsigned c);
HD_CANVAS_CODE void hd_ellipse(uint8_t *p,int cx,int cy,int rx,int ry,unsigned c);
HD_CANVAS_CODE void hd_text(uint8_t *p,int x,int y,const char *s,unsigned ink,int scale,int track);
int hd_text_width(const char *s,int scale,int track);
void hd_copy_120_to_240(uint8_t *out,const uint8_t *in);
void hd_upscale_inplace(uint8_t *pixels);
void hd_menu_draw(uint8_t *p,unsigned page,unsigned mode,unsigned selection,
                  unsigned frame,unsigned feedback,unsigned erase_hold,
                  unsigned slot0_valid,unsigned slot1_valid);
void hd_world_draw(uint8_t *p,const Story *s,unsigned frame);
void hd_isometric_draw(uint8_t *p,const Story *s,unsigned frame);
/* A missing or unlocated person must not silently reappear in the town. */
static inline int hd_mara_absent(const Story *s){
 return (s->rescue_stage>=STORY_RESCUE_MISSING &&
         s->rescue_stage<=STORY_RESCUE_TRACE) ||
         s->rescue_stage==STORY_RESCUE_LOST;
}
void hd_first_person_draw(uint8_t *p,const Story *s,unsigned frame);
void hd_studio_draw(uint8_t *p,const Story *s,unsigned frame);
int hd_cinema_supports(unsigned scene);
void hd_cinema_draw(uint8_t *p,const Story *s,unsigned frame);
void hd_game_overlay(uint8_t *p,const Story *s);
void hd_actor_stage(uint8_t *p,int x,int feet,int role,unsigned frame,int emotion);
void hd_actor_closeup(uint8_t *p,int x,int base,int role,unsigned frame,int emotion);
void hd_actor_stage_spoken(uint8_t *p,int x,int feet,int role,unsigned frame,int emotion,int speaking);
void hd_actor_closeup_spoken(uint8_t *p,int x,int base,int role,unsigned frame,int emotion,int speaking);
void hd_actor_stage_directed(uint8_t *p,int x,int feet,int role,unsigned frame,int emotion,int speaking,int gaze);
void hd_actor_closeup_directed(uint8_t *p,int x,int base,int role,unsigned frame,int emotion,int speaking,int gaze);
int hd_special_cinema_draw(uint8_t *p,const Story *s,unsigned frame);
int hd_special_supports(unsigned scene);
void hd_cafe_facade(uint8_t *p,int x,int y,unsigned frame);
void hd_ending_draw(uint8_t *p,const Story *s,unsigned frame);
void hd_arena_draw(uint8_t *p,const Story *s,unsigned frame);

