#include "music.h"
#ifdef HOST_MUSIC_TEST
#include "../tests/stubs/data.h"
#else
#include "data.h"
#endif
#include "music_motifs.h"
#include "studio.h"

/* One 262144-Hz free-running hardware clock. No streaming, samples,
 * synthesis buffers or per-frame floating-point work. VBlank only extends
 * the clock through slow draws; all note sequencing stays on main. The title uses its
 * existing authored MIDI events; gameplay uses actual BORE-derived motifs. */
static volatile u32 total;
static u32 start, loop_off, cue_next;
static volatile u16 last;
static u32 crash_until;
static int idx;
static unsigned active_cue, pending_cue, cue_step;
/* A one-shot 20-phrase score survives dialogue and camera changes. */
static unsigned ending_kind, ending_phase, here_index;
static u32 ending_start, here_note_end;
static unsigned here_lead_active;
#define TIMER_HZ 262144u
#define TIMER_AT(frame60) ((u32)((frame60)*TIMER_HZ/60u))

/* MIDI 36..96, hardware register values precomputed offline. */
static const u16 square_pitch[61] = { 44, 157, 263, 363, 457, 547, 631, 711, 786, 856, 923, 986, 1046, 1102, 1155, 1205, 1253, 1297, 1339, 1379, 1417, 1452, 1486, 1517, 1547, 1575, 1602, 1627, 1650, 1673, 1694, 1714, 1732, 1750, 1767, 1783, 1798, 1812, 1825, 1837, 1849, 1860, 1871, 1881, 1890, 1899, 1907, 1915, 1923, 1930, 1936, 1943, 1949, 1954, 1959, 1964, 1969, 1974, 1978, 1982, 1985 };
static const u16 wave_pitch[61] = { 1046, 1102, 1155, 1205, 1253, 1297, 1339, 1379, 1417, 1452, 1486, 1517, 1547, 1575, 1602, 1627, 1650, 1673, 1694, 1714, 1732, 1750, 1767, 1783, 1798, 1812, 1825, 1837, 1849, 1860, 1871, 1881, 1890, 1899, 1907, 1915, 1923, 1930, 1936, 1943, 1949, 1954, 1959, 1964, 1969, 1974, 1978, 1982, 1985, 1989, 1992, 1995, 1998, 2001, 2004, 2006, 2009, 2011, 2013, 2015, 2017 };
static unsigned sq_rate(unsigned p){
    if(p<36)p=36;
    if(p>96)p=96;
    return square_pitch[p-36];
}
static unsigned wav_rate(unsigned p){
    if(p<36)p=36;
    if(p>96)p=96;
    return wave_pitch[p-36];
}
static const u8 bass_wave[16] = {
  0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
  0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10
};
static void load_wave(void){
    REG_SND3SEL=0x0000;
    for(int i=0;i<8;i++)WAVE_RAM[i]=bass_wave[2*i]|(bass_wave[2*i+1]<<8);
    REG_SND3SEL=0x0040;
    for(int i=0;i<8;i++)WAVE_RAM[i]=bass_wave[2*i]|(bass_wave[2*i+1]<<8);
    REG_SND3SEL=0x0080;
}
void music_clock_vblank(void){
 u16 n=REG_TM3CNT_L;
 total+=(u16)(n-last);
 last=n;
}
static void time_update(void){
 /* The two clock fields form one sample. Prevent the IRQ from inserting
    its own sample between the main-thread read and write. This section
    does no PSG work and lasts only a few loads/stores. */
 u16 previous=REG_IME;
 REG_IME=0;
 music_clock_vblank();
 REG_IME=previous;
}
void music_stop(void){REG_SND1CNT=0;REG_SND2CNT=0;REG_SND3CNT=0;REG_SND4CNT=0;}
void music_init(void){
    REG_SNDSTAT=0x0080;
    REG_SNDCTRL_L=0xFF77;
    REG_SNDCTRL_H=0x0002;
    REG_SND1SWEEP=0x0008;
    load_wave();music_stop();
    REG_TM3CNT_H=0;REG_TM3CNT_L=0;REG_TM3CNT_H=0x0081;
    last=REG_TM3CNT_L;
    active_cue=pending_cue=MUSIC_CUE_TITLE;
    ending_kind=ending_phase=here_index=here_lead_active=0;
}
void music_start(void){
    music_stop();time_update();
    start=total;loop_off=0;idx=0;crash_until=0;
    active_cue=pending_cue=MUSIC_CUE_TITLE;
    ending_kind=ending_phase=here_index=here_lead_active=0;
}
void music_begin_ending(unsigned ending){
    if(ending<1 || ending>3 || ending_phase)return;
    music_stop();time_update();
    ending_kind=ending;ending_phase=1;here_index=here_lead_active=0;
    ending_start=total;here_note_end=0;
    active_cue=pending_cue=MUSIC_CUE_HERE_TODAY;
    cue_step=0;cue_next=total;
}
unsigned music_ending_phase(void){return ending_phase;}
void music_set_cue(unsigned cue){
    if(cue>MUSIC_CUE_STUDIO_IDLE)cue=MUSIC_CUE_STREETS;
    if(ending_phase){
        /* Explicit return to title is the only way to cancel the ending. */
        if(cue!=MUSIC_CUE_TITLE)return;
        ending_kind=ending_phase=here_index=here_lead_active=0;
        music_stop();time_update();
        active_cue=pending_cue=MUSIC_CUE_TITLE;
        cue_step=0;cue_next=total;start=total;loop_off=0;idx=0;crash_until=0;
        return;
    }
    if(cue==active_cue){pending_cue=cue;return;}
    if(active_cue==MUSIC_CUE_TITLE || cue==MUSIC_CUE_TITLE ||
       active_cue>=MUSIC_CUE_STUDIO || cue>=MUSIC_CUE_STUDIO){
        /* Title transitions and the player's studio transport are immediate.
         * Scene-to-scene music still respects the short phrase boundary. */
        music_stop();time_update();
        active_cue=pending_cue=cue;
        cue_step=0;cue_next=total;
        start=total;loop_off=0;idx=0;crash_until=0;
    } else {
        /* New scene: finish the current short phrase before switching.
         * At most four 8th-notes (about 1 second), preventing hard cuts. */
        pending_cue=cue;
    }
}
unsigned music_get_cue(void){return active_cue;}

static void play(const MusEv *e){
    switch(e->ch){
    case 0:
        if(e->arg){REG_SND1CNT=(e->arg<<12)|(6<<8)|(1<<6);REG_SND1FREQ=0x8000|e->val;}
        else REG_SND1CNT=(1<<6);
        break;
    case 1:
        if(e->arg){REG_SND2CNT=e->arg<<12;REG_SND2FREQ=0x8000|e->val;}
        else REG_SND2CNT=0;
        break;
    case 2:
        if(e->arg){REG_SND3CNT=0x4000;REG_SND3FREQ=0x8000|e->val;}
        else REG_SND3CNT=0;
        break;
    default:{
        int vol=e->arg&15,kind=e->arg>>4;
        if(kind==3){
            REG_SND4CNT=(vol<<12)|(5<<8);
            REG_SND4FREQ=0x8000|0x11;
            crash_until=(total-start)+47000;
            break;
        }
        if((total-start)<crash_until)break;
        REG_SND4CNT=(vol<<12)|(1<<8);
        REG_SND4FREQ=0x8000|(kind==1?0x21:0x5B);
    }}
}
static void play_motif(unsigned cue,unsigned step){
    MotifBeat n;
    if(cue==MUSIC_CUE_STUDIO)studio_motif_step(step,&n);
    else music_motif_step(cue,step,&n);
    if(n.lead>0){
        REG_SND1CNT=((unsigned)n.volume<<12)|(5<<8)|((unsigned)n.duty<<6);
        REG_SND1FREQ=0x8000|sq_rate((unsigned)n.lead);
    } else if(n.lead<0)REG_SND1CNT=((unsigned)n.duty<<6);
    const StudioPatch *mix=cue==MUSIC_CUE_STUDIO?studio_patch():0;
    static const uint8_t track_volume[4]={0,3,5,8};
    if(n.comp){
        unsigned v=mix?track_volume[studio_mix_level(mix,STUDIO_KEYS)]:5u;
        REG_SND2CNT=(v<<12)|(3<<8)|(0<<6);
        REG_SND2FREQ=0x8000|sq_rate(n.comp);
    }else if(cue==MUSIC_CUE_STUDIO)REG_SND2CNT=0;
    if(n.bass){
        /* GBA wave-channel volume: 25%, 50%, 100%. */
        static const uint16_t wave_level[4]={0,0x6000,0x4000,0x2000};
        REG_SND3CNT=mix?wave_level[studio_mix_level(mix,STUDIO_BASS)]:0x4000;
        REG_SND3FREQ=0x8000|wav_rate(n.bass);
    }else if(cue==MUSIC_CUE_STUDIO)REG_SND3CNT=0;
    if(n.noise){
        REG_SND4CNT=mix?((unsigned)track_volume[studio_mix_level(mix,STUDIO_DRUMS)]<<12)|
                    (n.noise==1?0x0700u:0x0300u):(n.noise==1?0x5700u:0x8300u);
        REG_SND4FREQ=0x8000|(n.noise==1?0x21:0x5B);
    }else if(cue==MUSIC_CUE_STUDIO)REG_SND4CNT=0;
    /* A rest in Danny's editor really silences the corresponding channel.
       Other authored motifs retain their existing sustain behavior. */
}
static void play_here_today(void){
    u32 elapsed=total-ending_start;
    if(here_lead_active && elapsed>=here_note_end){
        REG_SND1CNT=(2<<6);
        here_lead_active=0;
    }
    /* Events are sourced from the complete, timed BORE tune, preserving every
       phrase and pause. The per-poll loop is bounded to preserve frame time. */
    for(int guard=0;guard<8 && here_index<MUSIC_HERE_COUNT;guard++){
        const HereNote *n=&music_here_notes[here_index];
        if(elapsed<TIMER_AT(n->at))break;
        unsigned p=n->pitch;
        unsigned volume=n->phrase>16?6:n->phrase>9?11:8;
        REG_SND1CNT=(volume<<12)|(5<<8)|(2<<6);
        REG_SND1FREQ=0x8000|sq_rate(p);
        here_note_end=TIMER_AT((unsigned)n->at+n->length);
        here_lead_active=1;
        if(here_index==0 || n->phrase!=music_here_notes[here_index-1].phrase){
            unsigned bass=music_here_roots[n->phrase];
            REG_SND3CNT=0x4000;
            REG_SND3FREQ=0x8000|wav_rate(bass);
            if(n->phrase<17){
                REG_SND2CNT=(3<<12)|(4<<8);
                REG_SND2FREQ=0x8000|sq_rate(bass+19);
            }else REG_SND2CNT=0;
        }
        here_index++;
    }
    if(elapsed>=TIMER_AT(MUSIC_HERE_END_FRAMES)){
        music_stop();
        here_lead_active=0;
        if(ending_kind==1){
            /* AFTER Here Today's full resolution: the sole upbeat extra
               piece, a 16-second Closer/Whistler legacy coda. */
            ending_phase=2;
            active_cue=pending_cue=MUSIC_CUE_LEGACY;
            cue_step=0;cue_next=total;
        }else{
            ending_phase=3; /* silence, with the credits still on screen */
        }
    }
}
void music_poll(void){
    time_update();
    if(ending_phase==1){play_here_today();return;}
    if(ending_phase==3||active_cue==MUSIC_CUE_STUDIO_IDLE)return;
    if(active_cue==MUSIC_CUE_TITLE){
        u32 now=total-start;
        for(;;){
            if(idx>=MUSIC_COUNT){idx=MUSIC_LOOP_IDX;loop_off+=MUSIC_LOOP_UNITS;}
            const MusEv *e=&music_events[idx];
            if(now<e->t+loop_off)break;
            play(e);idx++;
        }
    }else{
        /* 8th-note sequencer, maximum eight ticks to recover after a blocked
         * frame without stalling music_poll or slowing the renderer. */
        for(int limit=0;limit<8 && (s32)(total-cue_next)>=0;limit++){
            if((cue_step&3u)==0 && pending_cue!=active_cue){
                music_stop();
                active_cue=pending_cue;
                cue_step=0;
            }
            play_motif(active_cue,cue_step++);
            cue_next+=active_cue==MUSIC_CUE_STUDIO?
                      studio_ticks():music_motif_ticks(active_cue);
            if(ending_phase==2 && cue_step>=64u){
                music_stop();ending_phase=3;break;
            }
        }
    }
}
void music_set_volume(int pct){
    if(pct<0)pct=0;
    if(pct>100)pct=100;
    int v=(pct*7+50)/100;
    REG_SNDCTRL_L=(REG_SNDCTRL_L&~0x77)|(v<<4)|v;
}

