#pragma once
#include "hd_potheads_set.h"
#include "hd_potheads_actor.h"
#include "hd_potheads_fixed.h"
#include "roto_codec.h"
#include "roto_cache.h"
#define POT_CACHE_DESCENT 180u
#define POT_CACHE_TICKS (POT_CACHE_DESCENT+POT_TRACE_LAST_TICK+1u)
#define POT_CACHE_LAST (POT_CACHE_TICKS-1u)
#define POT_CACHE_SAMPLES (POT_CACHE_LAST/2u+1u+(POT_CACHE_LAST&1u))
/* The original renderer remains the source of truth and fallback. */
static void pot_film_live(uint8_t *p,unsigned t){
 PotCamera camera=pot_camera(t);
 if(t>=POT_CACHE_DESCENT)pot_scene_fixed(p);
 else pot_scene(p,&camera,t);
 pot_dancer(p,&camera,t);
 hd_rect(p,0,135,HD_W,25,HD_INK);
}
#if defined(DANNY_OPENING_CACHE) && defined(__arm__) && !defined(HOST_TEST)
extern const uint32_t pot_film_cache_words[];
static int pot_film_cached(uint8_t *p,unsigned t){
 return roto_cache_render(p,pot_film_cache_words,t,pot_scene_fixed,pot_film_live);
}
#endif
