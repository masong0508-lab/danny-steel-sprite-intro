#include "play.h"
#include "story.h"
#include "story_draw.h"
#include "gba.h"
#include "music.h"

#define REG_BG2PA R16(0x04000020)
#define REG_BG2PB R16(0x04000022)
#define REG_BG2PC R16(0x04000024)
#define REG_BG2PD R16(0x04000026)
#define REG_BG2X (*(volatile u32 *)0x04000028)
#define REG_BG2Y (*(volatile u32 *)0x0400002C)
#define SAVE_LEN 24
#define LEGACY_SAVE_LEN 13
#define V2_SAVE_LEN 16
#define SAVE_STRIDE 32
#define SAVE_VER 3
static volatile u8 *const save_ram = (volatile u8 *)0x0E000000;
const char story_sram_id[] __attribute__((used)) = "SRAM_V113";
static Story game;
static u32 pixels[120*80/4] __attribute__((section(".ewram"), aligned(4)));
static int shown_page, pending, pending_page, clock_ticks, save_cooldown;

static int read_slot(int slot,u8 *buf){
    if((unsigned)slot>1u)return 0;
    unsigned hash=0x6D;
    for(int i=0;i<SAVE_LEN;i++)buf[i]=save_ram[slot*SAVE_STRIDE+i];
    if(buf[0]!='D'||buf[1]!='S'||buf[2]!='T'||buf[3]!='2')return 0;
    /* Preserve original saves: V1 has a 13-byte record; V2 adds optional
       cult quest milestones without touching Danny's primary quest flags. */
    int len=buf[4]==1?LEGACY_SAVE_LEN:buf[4]==2?V2_SAVE_LEN:buf[4]==SAVE_VER?SAVE_LEN:0;
    if(!len)return 0;
    for(int i=0;i<len-1;i++)hash^=buf[i];
    if(buf[len-1]!=hash)return 0;
    if(story_solid(buf[5],buf[6])) {
        /* A previous valid save can occupy a newly-added NPC location.
           Relocate only those known new actors, never accept a bad wall tile. */
        char t=story_tile(buf[5],buf[6]);
        if(t=='R') ++buf[6];
        else if(t=='P'||t=='V'||t=='Q'||t=='S'||t=='W'||t=='H'||t=='M'||t=='K') --buf[5];
        else return 0;
        if(story_solid(buf[5],buf[6]))return 0;
    }
    if(len==LEGACY_SAVE_LEN){buf[12]=buf[13]=buf[14]=0;}
    /* Version 1/2 finales were immediate choices, unlike the new stories;
       keep all quest progress and let old players choose again. */
    if(len<SAVE_LEN){
        buf[10]=0;buf[11]=0;
        for(int i=15;i<SAVE_LEN;i++)buf[i]=0;
        buf[16]=1; /* day 1 */
    }
    return 1;
}
int play_slot_valid(int slot){u8 b[SAVE_LEN];return read_slot(slot,b);}
void play_erase_slot(int slot){
    if((unsigned)slot>1u)return;
    for(int i=0;i<SAVE_STRIDE;i++)save_ram[slot*SAVE_STRIDE+i]=0xFF;
}
static void write_save(void){
    u8 b[SAVE_LEN]={'D','S','T','2',SAVE_VER,game.x,game.y,game.facing,game.view,game.flags,
        game.ended,game.ending,game.cult_stage,game.cult_pre_wins,game.cult_post_wins,
        game.life_stage,game.day_number,(u8)game.day_frame,(u8)(game.day_frame>>8),
        (u8)game.finale_frame,(u8)(game.finale_frame>>8),0,0,0};
    unsigned hash=0x6D;
    for(int i=0;i<SAVE_LEN-1;i++)hash^=b[i];
    b[SAVE_LEN-1]=(u8)hash;
    for(int i=0;i<SAVE_LEN;i++)save_ram[game.slot*SAVE_STRIDE+i]=b[i];
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
        if(game.finale_frame>450)game.finale_frame=450;
        if(game.life_stage==STORY_LIFE_FINALE && !game.ended) {
            /* A resumed final scene starts cleanly without duplicated dialogue. */
            game.dialogue=0;
        }
        /* Upgrade old slots with the new checksum and version on next autosave. */
        if(b[4]!=SAVE_VER)game.dirty=1;
    } else {story_new(&game,slot);write_save();}
    story_draw_init();
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
    if(game.quit){write_save();music_set_volume(40);return 1;}
    /* Periodic clock checkpoint: gameplay time survives resets and reloads. */
    if(game.life_stage>=STORY_LIFE_WAIT && game.life_stage<=STORY_LIFE_LAST_WORK
       && (clock_ticks%600)==599)game.dirty=1;
    if(game.dirty&&++save_cooldown>=120){write_save();save_cooldown=0;}
    if((clock_ticks++&1)==0){
        /* Draw the hidden bitmap; the hardware page flips on the NEXT VBlank. */
        pending_page=shown_page^1;
        draw_hidden(pending_page);
        pending=1;
    }
    return 0;
}
