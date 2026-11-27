#include "play.h"
#include "story.h"
#include "story_draw.h"
#include "gba.h"
#include "music.h"
#include "story_save.h"
#include "rescue_save.h"
#include "neighborhood_save.h"
#include "terrain_save.h"
#include "brush_save.h"
#include "world_save.h"
#include "studio.h"
#include "hd_canvas.h"
#include "frame_runtime.h"
#include "video_present.h"
#include "roto_cache.h"

#define REG_BG2PA R16(0x04000020)
#define REG_BG2PB R16(0x04000022)
#define REG_BG2PC R16(0x04000024)
#define REG_BG2PD R16(0x04000026)
#define REG_BG2X (*(volatile u32 *)0x04000028)
#define REG_BG2Y (*(volatile u32 *)0x0400002C)
#define SAVE_LEN STORY_SAVE_BYTES
#define SAVE_STRIDE STORY_SAVE_STRIDE
#define SAVE_VER STORY_SAVE_VER
static volatile u8 *const save_ram = (volatile u8 *)0x0E000000;
const char story_sram_id[] __attribute__((used)) = "SRAM_V113";
static Story game;
static unsigned clock_ticks;
static int save_cooldown;
static unsigned presented_scene;
/* Distinct locations and actual panel changes may settle visually.
   Dialogue pages within one scene, camera toggles and menus do not. */
static unsigned presentation_scene(void){
    if(game.ended||game.life_stage==STORY_LIFE_FINALE)return 0x300u;
    if(game.battle)return 0x301u;
    if(game.activity==STORY_ACT_STUDIO)return 0x302u;
    int panel=story_neighborhood_panel(&game);
    return panel>=0?0x100u+(unsigned)panel:(unsigned)game.cinema;
}

/* Shared musical language for Danny and BORE. The same cue is heard in the
   associated 2D/3D location and during its corresponding cinematic. */
static unsigned scene_music_cue(void){
    int panel=story_neighborhood_panel(&game);
    if(panel>=0){
        unsigned id=(unsigned)panel/3u;
        return id==8u?MUSIC_CUE_CLINIC:
               (id==2u||id==12u)?MUSIC_CUE_ROOFTOP:
               (id==7u||id==9u)?MUSIC_CUE_FARCE:MUSIC_CUE_STREETS;
    }
    if(game.activity==STORY_ACT_STUDIO)
        return game.activity_choice?MUSIC_CUE_STUDIO:MUSIC_CUE_STUDIO_IDLE;
    if(story_master_showcase(&game))return MUSIC_CUE_STUDIO;
    if(game.ended && game.ending==STORY_END_GOOD)return MUSIC_CUE_LEGACY;
    if(game.life_stage==STORY_LIFE_FINALE ||
       (game.ended && game.ending==STORY_END_NEUTRAL) ||
       game.cinema==CINE_FINALE)return MUSIC_CUE_ROOFTOP;
    switch(game.cinema){
    case CINE_GOOD:return MUSIC_CUE_LEGACY;
    case CINE_NOTICE:case CINE_CLINIC:case CINE_FOLLOWUP:
    case CINE_LAST_CLINIC:return MUSIC_CUE_CLINIC;
    case CINE_BORE:case CINE_MUSIC:case CINE_STAY:
    case CINE_BUS:return MUSIC_CUE_FARCE;
    case CINE_SERVER:case CINE_WITNESS:case CINE_BACKUP:
    case CINE_REBRAND:return MUSIC_CUE_ORCHARD;
    default:break;
    }
    if(game.battle)return MUSIC_CUE_ORCHARD;
    return MUSIC_CUE_STREETS;
}


static unsigned read_world_seed(unsigned slot){
    uint8_t raw[WORLD_EXT_LEN];unsigned seed=0;
    for(unsigned copy=0;copy<WORLD_EXT_COPIES;copy++){
        unsigned off=WORLD_EXT_OFFSET+slot*WORLD_EXT_STRIDE+copy*WORLD_EXT_LEN;
        for(unsigned i=0;i<WORLD_EXT_LEN;i++)raw[i]=save_ram[off+i];
        if(world_save_decode(slot,raw,&seed))return seed;
    }
    return 0; /* old or damaged companion data; retain campaign data */
}
static unsigned new_world_seed(void){
    /* VBlank/menu timing and the live audio timer vary naturally with
       player input; the saved generator state prevents repeated cycles
       even on deterministic emulator boots. Selection is automatic. */
    uint32_t entropy=frame_runtime_video_frames^((uint32_t)REG_TM3CNT_L<<16)^REG_VCOUNT;
    uint8_t raw[WORLD_POOL_LEN];WorldPool pool;
    for(unsigned i=0;i<WORLD_POOL_LEN;i++)raw[i]=save_ram[WORLD_POOL_OFFSET+i];
    if(!world_pool_decode(&pool,raw)){
        uint8_t old[WORLD_EXT_LEN];unsigned last=255u;
        for(unsigned i=0;i<WORLD_EXT_LEN;i++)old[i]=save_ram[WORLD_CURSOR_OFFSET+i];
        (void)world_save_decode(0x7fu,old,&last);
        world_pool_init(&pool,entropy,last);
    }
    unsigned seed=world_pool_pick(&pool,entropy);
    world_pool_encode(&pool,raw);
    for(unsigned i=0;i<WORLD_POOL_LEN;i++)save_ram[WORLD_POOL_OFFSET+i]=raw[i];
    uint8_t cursor[WORLD_EXT_LEN];world_save_encode(0x7fu,seed,cursor);
    for(unsigned i=0;i<WORLD_EXT_LEN;i++)save_ram[WORLD_CURSOR_OFFSET+i]=cursor[i];
    return seed;
}
static int read_slot(int slot,u8 *buf,unsigned *world_seed){
    if((unsigned)slot>1u)return 0;
    u8 raw[SAVE_LEN];
    for(unsigned i=0;i<SAVE_LEN;i++)raw[i]=save_ram[slot*SAVE_STRIDE+i];
    if(!story_save_decode(raw,buf))return 0;
    unsigned seed=read_world_seed((unsigned)slot);
    if(world_seed)*world_seed=seed;
    if(story_world_solid_for_seed(seed,buf[5],buf[6])){
        /* A missing/corrupt layout record never discards story progress.
           Resume's spawn fallback is used when the saved tile is invalid. */
        if(!seed){
            char t=story_world_tile_for_seed(seed,buf[5],buf[6]);
            if(t=='R') ++buf[6];
            else if(t=='P'||t=='V'||t=='Q'||t=='S'||t=='W'||
                    t=='H'||t=='M'||t=='K') --buf[5];
        }
    }
    return 1;
}
int play_slot_valid(int slot){u8 b[SAVE_LEN];return read_slot(slot,b,0);}
void play_erase_slot(int slot){
    if((unsigned)slot>1u)return;
    for(int i=0;i<SAVE_STRIDE;i++)save_ram[slot*SAVE_STRIDE+i]=0xFF;
    for(unsigned i=0;i<RESCUE_EXT_LEN;i++)
        save_ram[RESCUE_EXT_OFFSET+slot*RESCUE_EXT_LEN+i]=0xFF;
    for(unsigned i=0;i<STUDIO_EXT_LEN;i++)
        save_ram[STUDIO_EXT_OFFSET+slot*STUDIO_EXT_LEN+i]=0xFF;
    for(unsigned i=0;i<NEIGHBORHOOD_EXT_LEN;i++)
        save_ram[NEIGHBORHOOD_EXT_OFFSET+slot*NEIGHBORHOOD_EXT_LEN+i]=0xFF;
    for(unsigned i=0;i<TERRAIN_EXT_LEN;i++)
        save_ram[TERRAIN_EXT_OFFSET+slot*TERRAIN_EXT_LEN+i]=0xFF;
    for(unsigned i=0;i<BRUSH_EXT_LEN;i++)save_ram[BRUSH_EXT_OFFSET+slot*BRUSH_EXT_LEN+i]=0xFF;
    for(unsigned i=0;i<WORLD_EXT_STRIDE;i++)save_ram[WORLD_EXT_OFFSET+slot*WORLD_EXT_STRIDE+i]=0xFF;
    /* The global layout cursor intentionally survives deleting a save. */
}
static void write_save(void){
    uint8_t world_bytes[WORLD_EXT_LEN];
    world_save_encode(game.slot,story_world_seed(),world_bytes);
    for(unsigned copy=0;copy<WORLD_EXT_COPIES;copy++){
        unsigned off=WORLD_EXT_OFFSET+game.slot*WORLD_EXT_STRIDE+copy*WORLD_EXT_LEN;
        for(unsigned i=0;i<WORLD_EXT_LEN;i++)save_ram[off+i]=world_bytes[i];
    }
    u8 b[SAVE_LEN];
    story_save_encode(&game,b);
    for(unsigned i=0;i<SAVE_LEN;i++)
        save_ram[game.slot*SAVE_STRIDE+i]=b[i];
    u8 extension[RESCUE_EXT_LEN];
    rescue_save_encode(game.slot,game.bad_mask,game.rescue_stage,extension);
    for(unsigned i=0;i<RESCUE_EXT_LEN;i++)
        save_ram[RESCUE_EXT_OFFSET+game.slot*RESCUE_EXT_LEN+i]=extension[i];
    uint8_t studio_bytes[STUDIO_EXT_LEN];
    studio_save_encode(game.slot,studio_bytes);
    for(unsigned i=0;i<STUDIO_EXT_LEN;i++)
        save_ram[STUDIO_EXT_OFFSET+game.slot*STUDIO_EXT_LEN+i]=studio_bytes[i];
    uint8_t neighborhood_bytes[NEIGHBORHOOD_EXT_LEN];
    neighborhood_save_encode(&game,neighborhood_bytes);
    for(unsigned i=0;i<NEIGHBORHOOD_EXT_LEN;i++)
        save_ram[NEIGHBORHOOD_EXT_OFFSET+game.slot*NEIGHBORHOOD_EXT_LEN+i]=neighborhood_bytes[i];
    uint8_t terrain_bytes[TERRAIN_EXT_LEN];
    terrain_save_encode(&game,terrain_bytes);
    for(unsigned i=0;i<TERRAIN_EXT_LEN;i++)save_ram[TERRAIN_EXT_OFFSET+game.slot*TERRAIN_EXT_LEN+i]=terrain_bytes[i];
    uint8_t brush_bytes[BRUSH_EXT_LEN];
    brush_save_encode(&game,brush_bytes);
    for(unsigned i=0;i<BRUSH_EXT_LEN;i++)save_ram[BRUSH_EXT_OFFSET+game.slot*BRUSH_EXT_LEN+i]=brush_bytes[i];
    game.dirty=0;
}
static void draw_hidden(int page){
    /* The opening's dialogue overlay never touches the film except for
       its fixed top matte. Revoke the persistent canvas on every other
       draw, including world, pause menus, studio, combat and endings. */
    int count=0;
    int film_lease=game.cinema==CINE_OPEN && game.dialogue &&
        !game.ended && !game.battle && game.life_stage!=STORY_LIFE_FINALE &&
        game.activity!=STORY_ACT_STUDIO && !(game.paused && (game.choice & 0x80u)) &&
        story_neighborhood_panel(&game)<0;
    if(film_lease)film_lease=story_pages(&game,&count)!=0 && game.page<(unsigned)count;
    roto_cache_begin_frame(film_lease);
    int native_map=!game.view && game.scene==STORY_SCENE_NONE &&
        game.cinema==CINE_NONE && !game.battle && !game.ended &&
        game.life_stage!=STORY_LIFE_FINALE &&
        game.activity!=STORY_ACT_STUDIO &&
        !(game.paused && (game.choice & 0x80u));
    if(game.life_stage==STORY_LIFE_FINALE ||
       (game.ended&&(game.ending==STORY_END_GOOD ||
                     game.ending==STORY_END_NEUTRAL ||
                     game.ending==STORY_END_BAD))){
        /* Preserve the original 72.5s musical storyboard, while painting
           native scene art and the identical timecoded narrative captions. */
        hd_ending_draw((u8*)hd_shared_frame,&game,game.finale_frame);
    }else if(game.battle){
        /* Full native-resolution Mirror Orchard arena, with physical
           240x160 ROM scenes and original three-lane game mechanics. */
        hd_arena_draw((u8*)hd_shared_frame,&game,(unsigned)clock_ticks);
    }else if(game.activity==STORY_ACT_STUDIO){
        hd_studio_draw((u8*)hd_shared_frame,&game,(unsigned)clock_ticks);
    }else if(native_map){
        hd_world_draw((u8*)hd_shared_frame,&game,(unsigned)clock_ticks);
    }else if(game.view && !game.cinema && !game.battle &&
             !game.ended && game.life_stage!=STORY_LIFE_FINALE &&
             game.scene==STORY_SCENE_NONE &&
             !(game.paused && (game.choice & 0x80u))){
        /* Raycast at true 240-column resolution in the same world. */
        hd_first_person_draw((u8*)hd_shared_frame,&game,
                             (unsigned)clock_ticks);
    }else if(!game.ended && game.life_stage!=STORY_LIFE_FINALE &&
             !game.battle && hd_cinema_supports(game.cinema)){
        hd_cinema_draw((u8*)hd_shared_frame,&game,
                       (unsigned)game.cinema_frame);
    }else{
        /* Specialist ending films, battle, consequence pages and a few
           rare sequences retain their existing proven authored renderer.
           Expand backwards in the same EWRAM page; never a new buffer. */
        Story visual=game;
        if(visual.dialogue && !game.ended && !game.battle &&
           game.life_stage!=STORY_LIFE_FINALE &&
           game.activity!=STORY_ACT_STUDIO)
            visual.dialogue=0;
        story_draw(&visual,clock_ticks,(u8*)hd_shared_frame);
        hd_upscale_inplace((u8*)hd_shared_frame);
    }
    if(!game.ended && !game.battle &&
       game.life_stage!=STORY_LIFE_FINALE &&
       game.activity!=STORY_ACT_STUDIO &&
       !(game.paused && (game.choice & 0x80u)))
        hd_game_overlay((u8*)hd_shared_frame,&game);
    video_copy_frame(hd_shared_frame,(unsigned)page);
}
void play_enter(int want_load,int slot){
    roto_cache_begin_frame(0);
    frame_runtime_stop();
    u8 b[SAVE_LEN];
    studio_reset();
    unsigned seed=0;
    if(want_load&&read_slot(slot,b,&seed)){
        story_resume_seeded(&game,slot,b[5],b[6],b[7],b[8],b[9],b[10],b[11],seed);
        game.cult_stage=b[12]>CULT_RESOLVED?CULT_RESOLVED:b[12];
        game.cult_pre_wins=b[13]&7;
        game.cult_post_wins=b[14]&7;
        game.life_stage=b[15]<=STORY_LIFE_FINALE?b[15]:0;
        game.day_number=b[16]>0?b[16]:1;
        game.day_frame=(u16)(b[17]|((unsigned)b[18]<<8));
        if(game.day_frame>=STORY_DAY_FRAMES)game.day_frame=0;
        game.finale_frame=(u16)(b[19]|((unsigned)b[20]<<8));
        game.daily_mask=story_restore_daily_mask(b[21],game.life_stage);
        game.daily_memories=b[22]>99?99:b[22];
        game.energy=b[23]>STORY_STAT_MAX?STORY_STAT_MAX:b[23];
        game.spark=b[24]>STORY_STAT_MAX?STORY_STAT_MAX:b[24];
        game.bond=b[25]>STORY_STAT_MAX?STORY_STAT_MAX:b[25];
        game.work=b[26]>99?99:b[26];
        game.song_steps=b[27]>STORY_PROJECT_DONE?STORY_PROJECT_DONE:b[27];
        game.art_steps=b[28]>STORY_PROJECT_DONE?STORY_PROJECT_DONE:b[28];
        game.project_style=b[29]&STORY_PROJECT_STYLE_MASK;
        if(b[4]==SAVE_VER){
            u8 extension[RESCUE_EXT_LEN],bad_mask=0,rescue_stage=0;
            for(unsigned i=0;i<RESCUE_EXT_LEN;i++)
                extension[i]=save_ram[RESCUE_EXT_OFFSET+game.slot*RESCUE_EXT_LEN+i];
            if(rescue_save_decode(game.slot,extension,&bad_mask,&rescue_stage)){
                game.bad_mask=bad_mask;game.rescue_stage=rescue_stage;
            }
        }
        /* Studio extension is optional: old saves load the default tune.
           Corrupt or cross-slot extension never invalidates game progress. */
        {u8 studio_bytes[STUDIO_EXT_LEN];
         for(unsigned i=0;i<STUDIO_EXT_LEN;i++)
           studio_bytes[i]=save_ram[STUDIO_EXT_OFFSET+
               game.slot*STUDIO_EXT_LEN+i];
         (void)studio_save_decode(game.slot,studio_bytes);
        }
        {uint8_t neighborhood_bytes[NEIGHBORHOOD_EXT_LEN];
         for(unsigned i=0;i<NEIGHBORHOOD_EXT_LEN;i++)
            neighborhood_bytes[i]=save_ram[NEIGHBORHOOD_EXT_OFFSET+
                game.slot*NEIGHBORHOOD_EXT_LEN+i];
         (void)neighborhood_save_decode(&game,neighborhood_bytes);
        }
        if(game.rescue_stage==STORY_RESCUE_LOST){
            /* LOST repurposes these already-persisted day bytes to count
               the hopeful phone chapter. In particular day zero is valid. */
            game.day_number=b[16]<=STORY_HOPE_END_DAY?b[16]:0u;
            game.life_stage=STORY_LIFE_NONE;
            game.scene=STORY_SCENE_NONE;
        }
        if(game.finale_frame>STORY_GOOD_FINAL_FRAMES)game.finale_frame=0;
        if(game.life_stage==STORY_LIFE_FINALE ||
           (game.ended && (game.ending==STORY_END_GOOD ||
                          game.ending==STORY_END_BAD))) {
            /* The one-shot score starts from the beginning on SRAM resume;
               replay the matching film rather than desynchronizing it. */
            game.finale_frame=0;game.dialogue=0;game.page=0;
            game.cinema=game.ending==STORY_END_GOOD?CINE_GOOD:
                        game.ending==STORY_END_BAD?CINE_BAD:CINE_FINALE;
            game.cinema_frame=0;
        }
        /* Upgrade old slots with the new checksum and version on next autosave. */
        if(b[4]!=SAVE_VER)game.dirty=1;
        uint8_t terrain_bytes[TERRAIN_EXT_LEN];
        for(unsigned i=0;i<TERRAIN_EXT_LEN;i++)terrain_bytes[i]=save_ram[TERRAIN_EXT_OFFSET+game.slot*TERRAIN_EXT_LEN+i];
        (void)terrain_save_decode(&game,terrain_bytes);
        uint8_t brush_bytes[BRUSH_EXT_LEN];
        for(unsigned i=0;i<BRUSH_EXT_LEN;i++)brush_bytes[i]=save_ram[BRUSH_EXT_OFFSET+game.slot*BRUSH_EXT_LEN+i];
        (void)brush_save_decode(&game,brush_bytes);
    } else {story_new_seeded(&game,slot,new_world_seed());write_save();}
    story_draw_init();
    if(game.life_stage==STORY_LIFE_FINALE ||
       (game.ended && game.ending>=STORY_END_GOOD && game.ending<=STORY_END_BAD))
        music_begin_ending(game.ended?game.ending:STORY_END_NEUTRAL);
    else music_set_cue(scene_music_cue());
    clock_ticks=0;save_cooldown=0;
    REG_DISPCNT=0;REG_BLDCNT=0;REG_BLDY=0;
    for(int i=0;i<256;i++)PAL16[i]=i<32?story_palette[i]:
                         i<64?hd_extra_palette[i-32]:0;
    for(int i=0;i<128;i++)((volatile u16*)0x07000000)[i*4]=0x0200;
    REG_BG2PA=256; REG_BG2PB=0; REG_BG2PC=0; REG_BG2PD=256;
    REG_BG2X=0;REG_BG2Y=0;REG_BGCNT(2)=0;
    draw_hidden(0);
    presented_scene=presentation_scene();
    REG_DISPCNT=4|DCNT_BG2;
    /* A held menu confirmation belongs to the menu. Only a release and
       new press may advance the opening after its initial page appears. */
    game.old_keys=(u16)(~REG_KEYINPUT&0x03ffu);
    frame_runtime_start(0u);
    music_set_volume(65);
}
static int play_tick(u16 keys){
    int was_editor=game.activity==STORY_ACT_STUDIO||game.activity==STORY_ACT_PAINT;
    unsigned prior_show=story_show_stage(&game),prior_target=game.neighborhood_target,prior_land=game.terrain_style;
    unsigned prior_brush=story_brush_revision(game.slot),prior_hike=story_hike_stage(game.slot);
    unsigned prior_seed=story_world_seed();
    story_tick(&game,keys);
    /* Leaving the editor commits the edited arrangement immediately,
       instead of waiting two seconds for the periodic SRAM checkpoint. */
    if(game.dirty&&((was_editor&&game.activity!=STORY_ACT_STUDIO&&game.activity!=STORY_ACT_PAINT)||
       prior_show!=story_show_stage(&game)||prior_target!=game.neighborhood_target||prior_land!=game.terrain_style||prior_brush!=story_brush_revision(game.slot)||prior_hike!=story_hike_stage(game.slot)||prior_seed!=story_world_seed())){
        write_save();save_cooldown=0;
    }
    if(game.quit){
        roto_cache_begin_frame(0);
        frame_runtime_stop();
        write_save();music_set_cue(MUSIC_CUE_TITLE);music_set_volume(40);return 1;
    }
    if(game.life_stage==STORY_LIFE_FINALE ||
       (game.ended && game.ending>=STORY_END_GOOD && game.ending<=STORY_END_BAD))
        music_begin_ending(game.ended?game.ending:STORY_END_NEUTRAL);
    else music_set_cue(scene_music_cue());
    /* Cinematic mix: music withdraws during difficult clinic scenes and
       gently fades during the final rooftop sequence. Eight-VBlank updates
       avoid additional per-frame PSG register work or audio allocations. */
    if((clock_ticks & 7)==0){
        int volume=65;
        if(music_ending_phase()==1)volume=62; /* full HERE TODAY, no 7-sec fade */
        else if(music_ending_phase()==2)volume=83; /* good-only coda */
        else if(music_ending_phase()==3)volume=0;
        else if(game.cinema==CINE_CLINIC || game.cinema==CINE_LAST_CLINIC)
            volume=28;
        else if(game.cinema==CINE_GOOD)volume=78;
        music_set_volume(volume);
    }
    /* Periodic clock checkpoint: gameplay time survives resets and reloads. */
    if(((game.life_stage>=STORY_LIFE_WAIT &&
         game.life_stage<=STORY_LIFE_LAST_WORK) ||
         game.rescue_stage==STORY_RESCUE_LOST) &&
         (clock_ticks%600)==599)game.dirty=1;
    if(game.dirty&&++save_cooldown>=120){write_save();save_cooldown=0;}
    ++clock_ticks;
    return 0;
}
int play_update(void){
    u16 keys;
    /* Story mutation is always on main, in the exact order sampled by
       VBlank. A draw can delay processing, but cannot slow the clock or
       turn a short press/release into a permanently missed action. */
    for(unsigned i=0;i<FRAME_RUNTIME_QUEUE_CAPACITY&&frame_runtime_next(&keys);i++)
        if(play_tick(keys))return 1;
    unsigned page;
    if(frame_runtime_begin_render(&page)){
        unsigned scene=presentation_scene();
        /* Preserve brightness and exact timing of source opening,
           active combat and the musical ending. */
        unsigned transition=game.cinema==CINE_OPEN||game.battle==BATTLE_DODGE||
            game.ended||game.life_stage==STORY_LIFE_FINALE?FRAME_RUNTIME_CLEAR:
            scene!=presented_scene?FRAME_RUNTIME_SOFTEN:FRAME_RUNTIME_KEEP;
        draw_hidden((int)page);
        frame_runtime_submit_transition(page,transition);
        presented_scene=scene;
    }
    return 0;
}

