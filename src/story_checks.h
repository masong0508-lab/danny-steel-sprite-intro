#pragma once
#include "story.h"

/* LIVE gameplay conditions, not checksum marks or separately stored trophies.
   Every check is derived from normal story, quest, relationship and project
   state already saved by the game. No new SRAM format or heap allocations. */
enum {
    STORY_CHECK_LETTER=0, STORY_CHECK_BORE, STORY_CHECK_PROMISE,
    STORY_CHECK_MUSIC, STORY_CHECK_CANVAS,
    STORY_CHECK_MASTER_TAPE, STORY_CHECK_FRAMED_ART,
    STORY_CHECK_MARA_TRUST, STORY_CHECK_ORCHARD, STORY_CHECK_CLINIC,
    STORY_CHECK_HIDE_REPLY, STORY_CHECK_FORGE_FORM,
    STORY_CHECK_ERASE_DEMO, STORY_CHECK_LEAK_NAME,
    STORY_CHECK_MARA_SAFE,
    STORY_CHECK_COUNT
};
#define STORY_CHECK_PAGE_COUNT 4u /* three checklists + live field guide */
#define STORY_CHECKS_PER_PAGE 5u
#define STORY_CHECK_PAGE_ACTIVE 0x80u
/* Transient pause-only help overlay. These bits are cleared on resume and
   do not change the Story struct or its two-slot SRAM save format. */
#define STORY_FIELD_GUIDE_ACTIVE 0x40u
#define STORY_FIELD_GUIDE_DETAIL 0x10u
#define STORY_CHECK_PAGE_MASK 0x06u
static inline unsigned story_check_page(const Story *s){
    return (unsigned)((s->choice>>1)&3u);
}
static inline unsigned story_check_live(const Story *s,unsigned check){
    switch(check){
    case STORY_CHECK_LETTER:return !!(s->flags&STORY_HOME);
    case STORY_CHECK_BORE:return !!(s->flags&STORY_STAMP);
    case STORY_CHECK_PROMISE:return !!(s->flags&STORY_MARA);
    case STORY_CHECK_MUSIC:return !!(s->flags&STORY_MUSIC);
    case STORY_CHECK_CANVAS:return !!(s->flags&STORY_ART);
    case STORY_CHECK_MASTER_TAPE:return s->song_steps>=STORY_PROJECT_DONE;
    case STORY_CHECK_FRAMED_ART:return s->art_steps>=STORY_PROJECT_DONE;
    case STORY_CHECK_MARA_TRUST:return s->bond>=4u;
    case STORY_CHECK_ORCHARD:return s->cult_stage>=CULT_RESOLVED;
    case STORY_CHECK_CLINIC:return s->life_stage>=STORY_LIFE_FOLLOWUP_WAIT||
                               (s->ended&&s->ending==STORY_END_NEUTRAL);
    case STORY_CHECK_HIDE_REPLY:return !!(s->bad_mask&STORY_BAD_LETTER);
    case STORY_CHECK_FORGE_FORM:return !!(s->bad_mask&STORY_BAD_CLERK);
    case STORY_CHECK_ERASE_DEMO:return !!(s->bad_mask&STORY_BAD_MUSIC);
    case STORY_CHECK_LEAK_NAME:return !!(s->bad_mask&STORY_BAD_SERVER);
    case STORY_CHECK_MARA_SAFE:return s->rescue_stage==STORY_RESCUE_SAVED||
                                  s->rescue_stage==STORY_RESCUE_AFTERSHOCK;
    default:return 0u;
    }
}
static inline unsigned story_check_count_page(const Story *s,unsigned page){
    if(page>=STORY_CHECK_PAGE_COUNT)return 0u;
    unsigned count=0u,base=page*STORY_CHECKS_PER_PAGE;
    for(unsigned i=0;i<STORY_CHECKS_PER_PAGE;i++)
        count+=story_check_live(s,base+i);
    return count;
}
static inline const char *story_check_name(unsigned check){
    static const char *const names[STORY_CHECK_COUNT]={
        "LETTER FROM HOME", "BORE STAMP", "PROMISE TO MARA",
        "MUSIC ARCHIVE", "ROOFTOP CANVAS",
        "MASTER RECORDING", "FINISHED PAINTING", "MARA TRUST",
        "MIRROR ORCHARD", "CLINIC VISIT",
        "HIDDEN REPLY", "FORGED RECORD", "ERASED DEMO",
        "LEAKED IDENTITY", "MARA RESCUED"
    };
    return check<STORY_CHECK_COUNT?names[check]:"UNKNOWN";
}
