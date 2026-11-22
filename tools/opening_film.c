/* Capture the opening from the actual GBA scanout, starting at New Game.
 *
 * Build: gcc -std=c11 -O2 -Wall -Wextra -Werror -Isrc \
 *        tools/opening_film.c -lmgba -o /tmp/danny_opening_film
 * Run:   /tmp/danny_opening_film ROM MATCHING_ELF NEW_DIRECTORY [seconds]
 *        [--read-dialogue] (or use --seconds NUMBER instead of seconds)
 * Encode: python3 tools/encode_opening_film.py NEW_DIRECTORY
 *
 * The default is 24 seconds. Every emulated video interval is retained,
 * including repeated displayed pages. PPM pixels come from mGBA's completed
 * 240x160 video buffer, never from a host version of the game's renderer.
 * ELF symbols are read-only startup/verification probes. This tool neither
 * writes game memory nor loads a save, cheat, patch or user configuration.
 * By default only START, A (New Game), A (slot one), and releases are sent.
 * --read-dialogue adds six one-video-frame A taps after the cinematic clock
 * reaches 150,300,480,810,1020,1200. Each tap advances one caption naturally;
 * none is sent on the final page. Clock continuity is checked throughout.
 * Audio is emulated normally but not included in this film.
 */
#define _XOPEN_SOURCE 700
#include "story.h"
#include <mgba/core/core.h>
#include <mgba/core/blip_buf.h>
#include <mgba/core/log.h>
#include <mgba/core/version.h>
#include <elf.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

enum { WIDTH=240, HEIGHT=160, KEY_A=1, KEY_START=8, DEFAULT_SECONDS=24,
       DIALOGUE_STEPS=6 };
static const unsigned dialogue_beats[DIALOGUE_STEPS]={150u,300u,480u,810u,1020u,1200u};
typedef struct { uint32_t address, size; } Symbol;
typedef struct { Symbol game, ticks, active, read, write, dropped; } Symbols;
typedef struct {
    Story game;
    uint32_t video, ticks, active, depth, dropped, display;
} Sample;
typedef struct {
    struct mCore *core;
    const Symbols *symbols;
    const color_t *pixels;
    color_t completed_pixels[WIDTH*HEIGHT];
    Sample started, completed;
    unsigned ended_count, have_complete, started_open;
} Capture;
typedef struct { uint32_t video, keys, cinema_frame, dialogue_page, cue; } Input;
static Input inputs[24];
static unsigned input_count, emulator_errors;

static void log_errors(struct mLogger *logger, int category,
                       enum mLogLevel level, const char *format, va_list args) {
    (void)logger; (void)category;
    if (level & (mLOG_FATAL|mLOG_ERROR)) {
        ++emulator_errors;
        vfprintf(stderr,format,args); fputc('\n',stderr);
    }
}
static struct mLogger logger={.log=log_errors,.filter=NULL};

/* IEEE CRC32, matching Python zlib.crc32 without another link dependency.
   Fingerprint every ELF byte, including non-ROM symbols and debug data. */
static uint32_t crc32_bytes(const uint8_t *data, size_t size) {
    uint32_t crc=0xffffffffu;
    for (size_t i=0;i<size;i++) {
        crc^=data[i];
        for (unsigned bit=0;bit<8u;bit++)
            crc=(crc>>1)^((crc&1u)?0xedb88320u:0u);
    }
    return ~crc;
}

static uint8_t *read_file(const char *path, size_t *size) {
    FILE *file=fopen(path,"rb");
    if (!file) { perror(path); return NULL; }
    if (fseek(file,0,SEEK_END)) { fclose(file); return NULL; }
    long length=ftell(file);
    if (length<=0 || length>64*1024*1024) { fclose(file); return NULL; }
    rewind(file);
    uint8_t *data=malloc((size_t)length);
    if (!data) { fclose(file); return NULL; }
    int ok=fread(data,1,(size_t)length,file)==(size_t)length;
    fclose(file);
    if (!ok) { free(data); return NULL; }
    *size=(size_t)length;
    return data;
}
static int in_file(size_t size, uint32_t offset, uint32_t length) {
    return (uint64_t)offset+length<=size;
}
static int section(const uint8_t *data, size_t size, const Elf32_Ehdr *header,
                   unsigned index, Elf32_Shdr *out) {
    uint64_t at=(uint64_t)header->e_shoff+(uint64_t)index*header->e_shentsize;
    if (index>=header->e_shnum || at+sizeof(*out)>size) return 0;
    memcpy(out,data+(size_t)at,sizeof(*out));
    return 1;
}
static const char *basename_of(const char *path) {
    const char *slash=strrchr(path,'/');
    return slash?slash+1:path;
}
static Symbol find_symbol(const uint8_t *data, size_t size,
                          const Elf32_Ehdr *header, const char *name,
                          const char *owner) {
    Symbol absent={0,0};
    for (unsigned i=0;i<header->e_shnum;i++) {
        Elf32_Shdr table,strings;
        if (!section(data,size,header,i,&table) || table.sh_type!=SHT_SYMTAB)
            continue;
        if (table.sh_entsize<sizeof(Elf32_Sym) ||
            !in_file(size,table.sh_offset,table.sh_size) ||
            !section(data,size,header,table.sh_link,&strings) ||
            !in_file(size,strings.sh_offset,strings.sh_size)) return absent;
        const char *file="";
        for (unsigned j=0;j<table.sh_size/table.sh_entsize;j++) {
            Elf32_Sym sym;
            memcpy(&sym,data+table.sh_offset+j*table.sh_entsize,sizeof(sym));
            if (sym.st_name>=strings.sh_size) continue;
            const char *s=(const char *)data+strings.sh_offset+sym.st_name;
            if (!memchr(s,0,strings.sh_size-sym.st_name)) return absent;
            if (ELF32_ST_TYPE(sym.st_info)==STT_FILE) file=basename_of(s);
            if (sym.st_shndx!=SHN_UNDEF &&
                ELF32_ST_TYPE(sym.st_info)==STT_OBJECT && !strcmp(s,name) &&
                (!owner || !strcmp(file,owner)))
                return (Symbol){sym.st_value,sym.st_size};
        }
    }
    return absent;
}
static int is_ram(Symbol symbol, uint32_t size) {
    uint64_t end=(uint64_t)symbol.address+symbol.size;
    return symbol.size==size &&
        ((symbol.address>=0x02000000u && end<=0x02040000u) ||
         (symbol.address>=0x03000000u && end<=0x03008000u));
}
static int load_symbols(const char *rom_path, const char *elf_path,
                        Symbols *symbols, size_t *matched_bytes,
                        uint32_t *elf_crc) {
    size_t size=0,rom_size=0;
    uint8_t *data=read_file(elf_path,&size), *rom=read_file(rom_path,&rom_size);
    Elf32_Ehdr header;
    int ok=data && rom && size>=sizeof(header);
    if (ok) {
        memcpy(&header,data,sizeof(header));
        ok=!memcmp(header.e_ident,ELFMAG,SELFMAG) &&
            header.e_ident[EI_CLASS]==ELFCLASS32 &&
            header.e_ident[EI_DATA]==ELFDATA2LSB && header.e_machine==EM_ARM &&
            header.e_shentsize>=sizeof(Elf32_Shdr) &&
            header.e_phentsize>=sizeof(Elf32_Phdr);
    }
    *matched_bytes=0;
    for (unsigned i=0;ok && i<header.e_phnum;i++) {
        Elf32_Phdr segment;
        uint64_t at=(uint64_t)header.e_phoff+(uint64_t)i*header.e_phentsize;
        if (at+sizeof(segment)>size) { ok=0; break; }
        memcpy(&segment,data+(size_t)at,sizeof(segment));
        if (segment.p_type!=PT_LOAD || !segment.p_filesz ||
            segment.p_paddr<0x08000000u || segment.p_paddr>=0x0a000000u)
            continue;
        uint32_t offset=segment.p_paddr-0x08000000u;
        if (!in_file(size,segment.p_offset,segment.p_filesz) ||
            !in_file(rom_size,offset,segment.p_filesz)) { ok=0; break; }
        for (uint32_t j=0;j<segment.p_filesz;j++) {
            /* gbafix fills the official header after ELF-to-ROM conversion.
               The entry instruction and all executable/data bytes must match. */
            if (offset+j>=4u && offset+j<0xc0u) continue;
            if (rom[offset+j]!=data[segment.p_offset+j]) { ok=0; break; }
            ++*matched_bytes;
        }
    }
    if (ok && *matched_bytes>0) {
        symbols->game=find_symbol(data,size,&header,"game","play.c");
        symbols->ticks=find_symbol(data,size,&header,"clock_ticks","play.c");
        symbols->active=find_symbol(data,size,&header,"active","frame_runtime.c");
        symbols->read=find_symbol(data,size,&header,"frame_runtime_read",NULL);
        symbols->write=find_symbol(data,size,&header,"frame_runtime_write",NULL);
        symbols->dropped=find_symbol(data,size,&header,"frame_runtime_dropped",NULL);
        ok=is_ram(symbols->game,sizeof(Story)) && is_ram(symbols->ticks,4u) &&
            is_ram(symbols->active,4u) && is_ram(symbols->read,4u) &&
            is_ram(symbols->write,4u) && is_ram(symbols->dropped,4u);
    } else ok=0;
    if (ok) *elf_crc=crc32_bytes(data,size);
    free(data); free(rom);
    if (!ok) fputs("Need a matching, unstripped ARM ELF with play.c game/clock_ticks "
                   "and frame_runtime.c symbols; ROM load bytes must match.\n",stderr);
    return ok;
}
static uint32_t word(struct mCore *core, Symbol symbol) {
    return core->busRead32(core,symbol.address);
}
static Sample sample(struct mCore *core, const Symbols *symbols) {
    Sample s;
    memset(&s,0,sizeof(s));
    uint8_t *out=(uint8_t *)&s.game;
    for (unsigned i=0;i<sizeof(Story);i++)
        out[i]=(uint8_t)core->busRead8(core,symbols->game.address+i);
    s.video=core->frameCounter(core);
    s.ticks=word(core,symbols->ticks);
    s.active=word(core,symbols->active);
    s.depth=word(core,symbols->write)-word(core,symbols->read);
    s.dropped=word(core,symbols->dropped);
    s.display=core->busRead16(core,0x04000000u);
    return s;
}
static int opening(const Sample *s) {
    return s->active==1u && s->game.cinema==CINE_OPEN && s->game.dialogue &&
        s->game.page<=DIALOGUE_STEPS && (s->display&7u)==4u && (s->display&0x400u) &&
        !(s->display&0x80u);
}
static void frame_started(void *context) {
    Capture *c=context;
    c->started=sample(c->core,c->symbols);
    c->started_open=(unsigned)opening(&c->started);
}
static void frame_ended(void *context) {
    Capture *c=context;
    ++c->ended_count;
    c->completed=c->started;
    c->have_complete=c->started_open;
    if (c->have_complete)
        memcpy(c->completed_pixels,c->pixels,sizeof(c->completed_pixels));
}
static int advance(struct mCore *core, unsigned frames) {
    while (frames--) {
        uint32_t before=core->frameCounter(core);
        core->runFrame(core);
        if (core->frameCounter(core)-before!=1u) {
            fputs("mGBA did not advance exactly one video frame\n",stderr); return 0;
        }
        /* Only drain host audio output; emulated sound/timers keep running. */
        for (int channel=0;channel<2;channel++) {
            struct blip_t *audio=core->getAudioChannel(core,channel);
            if (audio) blip_clear(audio);
        }
    }
    return 1;
}
static void keys(struct mCore *core, const Symbols *symbols, unsigned value,
                 unsigned cue) {
    Sample now=sample(core,symbols);
    core->setKeys(core,value);
    if (input_count<sizeof(inputs)/sizeof(inputs[0]))
        inputs[input_count++]=(Input){now.video,value,now.game.cinema_frame,
                                     now.game.page,cue};
}
static int press(struct mCore *core, const Symbols *symbols, unsigned key) {
    keys(core,symbols,key,0u);
    if (!advance(core,32u)) return 0;
    keys(core,symbols,0u,0u);
    return advance(core,127u);
}
static FILE *output_file(const char *directory, const char *name) {
    char path[PATH_MAX];
    int n=snprintf(path,sizeof(path),"%s/%s",directory,name);
    if (n<0 || (size_t)n>=sizeof(path)) { errno=ENAMETOOLONG; return NULL; }
    /* Refuse to mix a fresh film with a previous run's frames or metadata. */
    FILE *file=fopen(path,"wbx");
    if (!file) perror(path);
    return file;
}
static uint8_t component(color_t color, uint32_t mask) {
    while (!(mask&1u)) { mask>>=1; color>>=1; }
    return (uint8_t)(((uint32_t)color&mask)*255u/mask);
}
static int write_frame(const char *directory, unsigned index,
                       const color_t *pixels) {
    char name[40];
    snprintf(name,sizeof(name),"frame-%06u.ppm",index);
    FILE *file=output_file(directory,name);
    if (!file) return 0;
    uint8_t rgb[WIDTH*HEIGHT*3];
    for (unsigned i=0;i<WIDTH*HEIGHT;i++) {
        rgb[i*3]=component(pixels[i],M_COLOR_RED);
        rgb[i*3+1]=component(pixels[i],M_COLOR_GREEN);
        rgb[i*3+2]=component(pixels[i],M_COLOR_BLUE);
    }
    int ok=fprintf(file,"P6\n%d %d\n255\n",WIDTH,HEIGHT)>0 &&
        fwrite(rgb,1,sizeof(rgb),file)==sizeof(rgb);
    if (fclose(file)) ok=0;
    return ok;
}
static void json_string(FILE *file, const char *text) {
    fputc('"',file);
    for (const unsigned char *p=(const unsigned char *)text;*p;p++) {
        if (*p=='"' || *p=='\\') { fputc('\\',file); fputc(*p,file); }
        else if (*p<32) fprintf(file,"\\u%04x",*p);
        else fputc(*p,file);
    }
    fputc('"',file);
}
static int metadata(const char *directory, const char *rom_path,
                    const char *elf_path, size_t matched_bytes, uint32_t crc,
                    uint32_t elf_crc,
                    int frequency, int cycles, double requested_seconds,
                    unsigned frames, unsigned flips, unsigned changed,
                    unsigned read_dialogue, unsigned dialogue_presses,
                    unsigned max_clock_error,
                    const Sample *first, const Sample *last) {
    FILE *file=output_file(directory,"capture.json");
    if (!file) return 0;
    fputs("{\n  \"format_version\": 1,\n  \"source\": \"mGBA completed GBA video scanout\",\n"
          "  \"emulator_version\": ",file);
    json_string(file,projectVersion);
    fputs(",\n  \"rom\": ",file); json_string(file,rom_path);
    fputs(",\n  \"elf\": ",file); json_string(file,elf_path);
    fprintf(file,",\n  \"rom_crc32\": \"%08x\",\n  \"elf_crc32\": \"%08x\",\n"
            "  \"elf_rom_bytes_verified\": %zu,\n"
            "  \"elf_match_excludes_gbafix_header_bytes\": [4, 191],\n"
            "  \"width\": %d,\n  \"height\": %d,\n"
            "  \"frame_pattern\": \"frame-%%06d.ppm\",\n"
            "  \"frame_count\": %u,\n  \"cpu_frequency_hz\": %d,\n"
            "  \"cycles_per_video_frame\": %d,\n  \"video_rate_hz\": %.12f,\n"
            "  \"requested_seconds\": %.9f,\n  \"duration_seconds\": %.12f,\n"
            "  \"first_video_frame\": %u,\n  \"last_video_frame\": %u,\n"
            "  \"first_cinema_frame\": %u,\n  \"last_cinema_frame\": %u,\n"
            "  \"displayed_page_flips\": %u,\n  \"changed_video_frames\": %u,\n"
            "  \"first_dialogue_page\": %u,\n  \"last_dialogue_page\": %u,\n"
            "  \"read_dialogue\": %s,\n  \"dialogue_presses\": %u,\n"
            "  \"dialogue_cues\": %s,\n  \"max_cinema_clock_error_ticks\": %u,\n"
            "  \"input_samples_dropped\": %u,\n"
            "  \"game_memory_writes\": false,\n  \"audio_included\": false,\n"
            "  \"capture_start\": \"first complete video frame beginning with the opening runtime active\",\n"
            "  \"state_note\": \"cinema_frame and dialogue_page are read at scanout start; they may lead the previously rendered displayed page\",\n"
            "  \"inputs\": [\n",
            crc,elf_crc,matched_bytes,WIDTH,HEIGHT,frames,frequency,cycles,
            (double)frequency/cycles,requested_seconds,(double)frames*cycles/frequency,
            first->video,last->video,first->game.cinema_frame,last->game.cinema_frame,
            flips,changed,first->game.page,last->game.page,read_dialogue?"true":"false",
            dialogue_presses,read_dialogue?"[150, 300, 480, 810, 1020, 1200]":"[]",
            max_clock_error,last->dropped);
    for (unsigned i=0;i<input_count;i++)
        fprintf(file,"    {\"video_frame\": %u, \"keys\": %u, \"cinema_frame\": %u, "
                "\"dialogue_page\": %u, \"cue_cinema_frame\": %u}%s\n",
                inputs[i].video,inputs[i].keys,inputs[i].cinema_frame,
                inputs[i].dialogue_page,inputs[i].cue,i+1u<input_count?",":"");
    fputs("  ]\n}\n",file);
    int ok=!ferror(file);
    if (fclose(file)) ok=0;
    return ok;
}

static void usage(const char *program) {
    fprintf(stderr,"usage: %s ROM MATCHING_ELF NEW_DIRECTORY [seconds] [--read-dialogue]\n"
            "       seconds defaults to 24; --seconds NUMBER is also accepted\n",program);
}
static int options(int argc, char **argv, double *seconds, unsigned *read_dialogue) {
    unsigned duration_seen=0;
    *seconds=DEFAULT_SECONDS; *read_dialogue=0;
    for (int i=4;i<argc;i++) {
        const char *value=argv[i];
        if (!strcmp(value,"--read-dialogue")) {
            if (*read_dialogue) return 0;
            *read_dialogue=1u; continue;
        }
        if (!strcmp(value,"--seconds")) {
            if (++i==argc) return 0;
            value=argv[i];
        } else if (value[0]=='-') return 0;
        if (duration_seen++) return 0;
        char *end=NULL;
        errno=0;
        *seconds=strtod(value,&end);
        if (errno || end==value || *end || !isfinite(*seconds) ||
            *seconds<0.1 || *seconds>180) return 0;
    }
    return 1;
}

int main(int argc, char **argv) {
    if (argc==2 && !strcmp(argv[1],"--help")) { usage(argv[0]); return 0; }
    double seconds=DEFAULT_SECONDS;
    unsigned read_dialogue=0;
    if (argc<4 || !options(argc,argv,&seconds,&read_dialogue)) {
        usage(argv[0]);
        fputs("Use a single duration from 0.1 through 180 and exact option names.\n",stderr);
        return 2;
    }
    char *rom_path=realpath(argv[1],NULL), *elf_path=realpath(argv[2],NULL);
    Symbols symbols={0};
    size_t matched_bytes=0;
    uint32_t elf_crc=0;
    if (!rom_path || !elf_path ||
        !load_symbols(rom_path,elf_path,&symbols,&matched_bytes,&elf_crc)) {
        free(rom_path); free(elf_path); return 2;
    }
    if (mkdir(argv[3],0777) && errno!=EEXIST) {
        perror(argv[3]); free(rom_path); free(elf_path); return 2;
    }
    FILE *timeline=output_file(argv[3],"frames.csv");
    if (!timeline) { free(rom_path); free(elf_path); return 2; }
    fputs("frame,seconds,video_frame,cinema_frame,clock_ticks,dialogue_page,queue_depth,display_page,input_dropped\n",timeline);
    mLogSetDefaultLogger(&logger);
    struct mCore *core=mCoreFind(rom_path);
    if (!core) { fputs("No GBA core for ROM\n",stderr); fclose(timeline); return 2; }
    mCoreInitConfig(core,NULL);
    if (!core->init(core)) { fclose(timeline); return 2; }
    int result=1;
    color_t *pixels=NULL, *previous=NULL;
    Capture *capture=NULL;
    if (core->platform(core)!=mPLATFORM_GBA || !mCoreLoadFile(core,rom_path))
        goto finish;
    unsigned width=0,height=0;
    core->desiredVideoDimensions(core,&width,&height);
    if (width!=WIDTH || height!=HEIGHT) goto finish;
    pixels=calloc(WIDTH*HEIGHT,sizeof(*pixels));
    previous=calloc(WIDTH*HEIGHT,sizeof(*previous));
    capture=calloc(1,sizeof(*capture));
    if (!pixels || !previous || !capture) goto finish;
    core->setVideoBuffer(core,pixels,WIDTH);
    capture->core=core; capture->symbols=&symbols; capture->pixels=pixels;
    struct mCoreCallbacks callbacks={.context=capture,
        .videoFrameStarted=frame_started,.videoFrameEnded=frame_ended};
    core->addCoreCallbacks(core,&callbacks);
    core->reset(core);
    int frequency=core->frequency(core), cycles=core->frameCycles(core);
    if (frequency<=0 || cycles<=0) goto finish;
    double rate=(double)frequency/cycles;
    unsigned frame_count=(unsigned)(seconds*rate);
    if ((double)frame_count*cycles/frequency<seconds) ++frame_count;
    uint32_t crc=0;
    core->checksum(core,&crc,mCHECKSUM_CRC32);
    printf("mGBA %s; %.12f Hz; %u native video frames requested\n",
           projectVersion,rate,frame_count);
    printf("Verified %zu ELF/ROM load bytes; ROM CRC32 %08x; ELF CRC32 %08x\n",
           matched_bytes,crc,elf_crc);
    fflush(stdout);
    if (!advance(core,1650u) || !press(core,&symbols,KEY_START) ||
        !press(core,&symbols,KEY_A))
        goto finish;
    keys(core,&symbols,KEY_A,0u);
    /* Watch each frame while the last menu confirmation is held. Do not
       use the screenshot route's 152-frame wait: it loses the aerial shot. */
    for (unsigned wait=0;wait<600u && !capture->have_complete;wait++)
        if (!advance(core,1u)) goto finish;
    keys(core,&symbols,0u,0u);
    if (!capture->have_complete) {
        fputs("New Game did not reach a complete displayed opening frame\n",stderr);
        goto finish;
    }
    Sample first=capture->completed, last=first, prior=first;
    unsigned flips=0,changed=0,previous_page=first.display&0x10u;
    unsigned next_dialogue=0,held_cue=0,max_clock_error=0;
    if (first.game.cinema_frame>2u || first.game.page!=0u) {
        fprintf(stderr,"Opening capture started late at cinema_frame %u\n",
                first.game.cinema_frame); goto finish;
    }
    for (unsigned frame=0;frame<frame_count;frame++) {
        unsigned ended=capture->ended_count;
        if (frame) {
            if (held_cue) {
                keys(core,&symbols,0u,held_cue); held_cue=0u;
            } else if (read_dialogue && next_dialogue<DIALOGUE_STEPS) {
                Sample now=sample(core,&symbols);
                if (now.game.cinema_frame>=dialogue_beats[next_dialogue]) {
                    if (!opening(&now) || now.game.page!=next_dialogue) {
                        fputs("A previous caption tap did not advance exactly one page\n",stderr);
                        goto finish;
                    }
                    held_cue=dialogue_beats[next_dialogue++];
                    keys(core,&symbols,KEY_A,held_cue);
                }
            }
            if (!advance(core,1u) || capture->ended_count!=ended+1u) goto finish;
        }
        last=capture->completed;
        int64_t clock_error=(int64_t)last.game.cinema_frame-first.game.cinema_frame-
                            (int64_t)(last.ticks-first.ticks);
        unsigned absolute_error=(unsigned)(clock_error<0?-clock_error:clock_error);
        if (absolute_error>max_clock_error) max_clock_error=absolute_error;
        if (!capture->have_complete || !opening(&last) || last.dropped ||
            last.video-first.video!=frame ||
            last.game.cinema_frame<prior.game.cinema_frame || absolute_error>1u ||
            last.game.dialogue!=first.game.dialogue ||
            last.game.page<prior.game.page || last.game.page>prior.game.page+1u ||
            last.game.page>next_dialogue) {
            fprintf(stderr,"Opening/cadence invariant failed at capture frame %u\n",frame);
            goto finish;
        }
        prior=last;
        unsigned page=last.display&0x10u;
        if (frame) {
            if (page!=previous_page) ++flips;
            if (memcmp(previous,capture->completed_pixels,
                       WIDTH*HEIGHT*sizeof(*previous))) ++changed;
        }
        previous_page=page;
        memcpy(previous,capture->completed_pixels,WIDTH*HEIGHT*sizeof(*previous));
        if (!write_frame(argv[3],frame,capture->completed_pixels)) goto finish;
        if (fprintf(timeline,"%u,%.12f,%u,%u,%u,%u,%u,%u,%u\n",frame,
                    (double)frame*cycles/frequency,last.video,last.game.cinema_frame,
                    last.ticks,last.game.page,last.depth,page?1u:0u,last.dropped)<0) goto finish;
    }
    if (held_cue) keys(core,&symbols,0u,held_cue);
    if (read_dialogue && last.game.cinema_frame>=dialogue_beats[DIALOGUE_STEPS-1u]+12u &&
        (next_dialogue!=DIALOGUE_STEPS || last.game.page!=DIALOGUE_STEPS)) {
        fputs("Full review film did not reach its final caption\n",stderr); goto finish;
    }
    if (emulator_errors || !metadata(argv[3],rom_path,elf_path,matched_bytes,crc,elf_crc,
            frequency,cycles,seconds,frame_count,flips,changed,read_dialogue,
            next_dialogue,max_clock_error,&first,&last)) goto finish;
    printf("Captured %u frames, %.9f seconds, video %u..%u, cinema %u..%u\n",
           frame_count,(double)frame_count*cycles/frequency,first.video,last.video,
           first.game.cinema_frame,last.game.cinema_frame);
    printf("Displayed flips %u; changed video frames %u; dialogue %u->%u (%u A taps); "
           "cinema/clock error at most %u tick(s); no dropped inputs or emulator errors\n",
           flips,changed,first.game.page,last.game.page,next_dialogue,max_clock_error);
    result=0;
finish:
    if (fclose(timeline)) result=1;
    core->clearCoreCallbacks(core);
    mCoreConfigDeinit(&core->config); core->deinit(core);
    free(capture); free(previous); free(pixels); free(rom_path); free(elf_path);
    if (result) fputs("Capture incomplete; use a new output directory when retrying.\n",stderr);
    return result;
}
