#include "play.h"
#include "story.h"
#include "story_draw.h"
#include "gba.h"
#include "music.h"
#include "story_save.h"
#include "rescue_save.h"
#include "studio.h"

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
static u32 pixels[120*80/4] __attribute__((section(".ewram"), aligned(4)));
static int shown_page, pending, pending_page, clock_ticks, save_cooldown;

/* Shared musical language for Danny and BORE. The same cue is heard in the
   associated 2D/3D location and during its corresponding cinematic. */
static unsigned scene_music_cue(void){
    if(game.activity==STORY_ACT_STUDIO && game.activity_choice)
        return MUSIC_CUE_STUDIO; /* SELECT: hear actual edited pattern */
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


static int read_slot(int slot,u8 *buf){
    if((unsigned)slot>1u)return 0;
    u8 raw[SAVE_LEN];
    for(unsigned i=0;i<SAVE_LEN;i++)raw[i]=save_ram[slot*SAVE_STRIDE+i];
    if(!story_save_decode(raw,buf))return 0;
    if(story_solid(buf[5],buf[6])){
        /* Repair saves from a map before newer NPCs were introduced. */
        char t=story_tile(buf[5],buf[6]);
        if(t=='R') ++buf[6];
        else if(t=='P'||t=='V'||t=='Q'||t=='S'||t=='W'||
                t=='H'||t=='M'||t=='K') --buf[5];
        else return 0;
        if(story_solid(buf[5],buf[6]))return 0;
    }
    return 1;
}
int play_slot_valid(int slot){u8 b[SAVE_LEN];return read_slot(slot,b);}
void play_erase_slot(int slot){
    if((unsigned)slot>1u)return;
    for(int i=0;i<SAVE_STRIDE;i++)save_ram[slot*SAVE_STRIDE+i]=0xFF;
    for(unsigned i=0;i<RESCUE_EXT_LEN;i++)
        save_ram[RESCUE_EXT_OFFSET+slot*RESCUE_EXT_LEN+i]=0xFF;
    for(unsigned i=0;i<STUDIO_EXT_LEN;i++)
        save_ram[STUDIO_EXT_OFFSET+slot*STUDIO_EXT_LEN+i]=0xFF;
}
static void write_save(void){
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
    game.dirty=0;
}
static void draw_hidden(int page){
    story_draw(&game,clock_ticks,(u8*)pixels);
    volatile u32 *dest=(volatile u32*)(VRAM8+(page?0xA000:0));
    for(int y=0;y<80;y++){
        for(int x=0;x<30;x++)dest[y*60+x]=pixels[y*30+x];
    }
}
void play_enter(int want_load,int slot){
    u8 b[SAVE_LEN];
    studio_reset();
    if(want_load&&read_slot(slot,b)){
        story_resume(&game,slot,b[5],b[6],b[7],b[8],b[9],b[10],b[11]);
        game.cult_stage=b[12]>CULT_RESOLVED?CULT_RESOLVED:b[12];
        game.cult_pre_wins=b[13]&7;
        game.cult_post_wins=b[14]&7;
        game.life_stage=b[15]<=STORY_LIFE_FINALE?b[15]:0;
        game.day_number=b[16]>0?b[16]:1;
        game.day_frame=(u16)(b[17]|((unsigned)b[18]<<8));
        if(game.day_frame>=STORY_DAY_FRAMES)game.day_frame=0;
        game.finale_frame=(u16)(b[19]|((unsigned)b[20]<<8));
        game.daily_mask=b[21]&(STORY_DAY_ALL|STORY_DAY_RECAP);
        game.daily_memories=b[22]>99?99:b[22];
        game.energy=b[23]>STORY_STAT_MAX?STORY_STAT_MAX:b[23];
        game.spark=b[24]>STORY_STAT_MAX?STORY_STAT_MAX:b[24];
        game.bond=b[25]>STORY_STAT_MAX?STORY_STAT_MAX:b[25];
        game.work=b[26]>99?99:b[26];
        game.song_steps=b[27]>STORY_PROJECT_DONE?STORY_PROJECT_DONE:b[27];
        game.art_steps=b[28]>STORY_PROJECT_DONE?STORY_PROJECT_DONE:b[28];
        game.project_style=b[29]&(STORY_STYLE_SONG_SOFT|STORY_STYLE_ART_WARM);
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
    } else {story_new(&game,slot);write_save();}
    story_draw_init();
    if(game.life_stage==STORY_LIFE_FINALE ||
       (game.ended && game.ending>=STORY_END_GOOD && game.ending<=STORY_END_BAD))
        music_begin_ending(game.ended?game.ending:STORY_END_NEUTRAL);
    else music_set_cue(scene_music_cue());
    clock_ticks=0;shown_page=0;pending=0;pending_page=0;save_cooldown=0;
    REG_DISPCNT=0;REG_BLDCNT=0;REG_BLDY=0;
    for(int i=0;i<256;i++)PAL16[i]=i<32?story_palette[i]:0;
    for(int i=0;i<128;i++)((volatile u16*)0x07000000)[i*4]=0x0200;
    REG_BG2PA=128; REG_BG2PB=0; REG_BG2PC=0; REG_BG2PD=128;
    REG_BG2X=0;REG_BG2Y=0;REG_BGCNT(2)=0;
    draw_hidden(0);
    REG_DISPCNT=4|DCNT_BG2;
    music_set_volume(65);
}
int play_update(void){
    if(pending){
        shown_page=pending_page;
        REG_DISPCNT=4|DCNT_BG2|(shown_page?0x10:0);
        pending=0;
    }
    story_tick(&game,(u16)(~REG_KEYINPUT&0x03FF));
    if(game.quit){write_save();music_set_cue(MUSIC_CUE_TITLE);music_set_volume(40);return 1;}
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
    if((clock_ticks++&1)==0){
        /* Draw the hidden bitmap; the hardware page flips on the NEXT VBlank. */
        pending_page=shown_page^1;
        draw_hidden(pending_page);
        pending=1;
    }
    return 0;
}
