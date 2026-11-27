/* Exercise the actual GBA input/clock pipeline through mGBA.
 * Build: gcc -std=c11 -O2 -Wall -Wextra -Werror -Isrc tools/runtime_probe.c \
 *        src/story.c src/studio.c -lmgba -o /tmp/danny_runtime_probe
 * Run:   /tmp/danny_runtime_probe ROM MATCHING_ELF [existing-picture-directory]
 *
 * The ELF supplies real data-symbol addresses, never guessed memory offsets.
 * The only emulated inputs are button presses. All game-memory access is
 * read-only. A host Story copy predicts an ordinary timed walk; it is never
 * written back into the ROM. No save, cheat or user configuration is loaded.
 */
#define _POSIX_C_SOURCE 200809L
#include "story.h"
#include <mgba/core/core.h>
#include <mgba/core/blip_buf.h>
#include <mgba/core/log.h>
#include <mgba/core/version.h>
#include <mgba-util/vfs.h>
#include <elf.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { KEY_A=1, KEY_B=2, KEY_START=8, KEY_RIGHT=16, KEY_LEFT=32, KEY_L=512 };
typedef struct { uint32_t address, size; } Symbol;
typedef struct {
    Symbol game, ticks, read, write, dropped, world;
} Symbols;
typedef struct { uint32_t ticks, depth; Story game; } Sample;
static unsigned failures, emulator_errors;
static Symbol watched_dropped;
static uint32_t peak_dropped;

static void log_errors(struct mLogger *logger,int category,
                       enum mLogLevel level,const char *format,va_list args){
    (void)logger;(void)category;
    if(level&(mLOG_FATAL|mLOG_ERROR)){
        ++emulator_errors;
        vfprintf(stderr,format,args);fputc('\n',stderr);
    }
}
static struct mLogger logger={.log=log_errors,.filter=NULL};

static int check(int ok,const char *name){
    printf("%s: %s\n",ok?"PASS":"FAIL",name);
    if(!ok)++failures;
    return ok;
}
static int in_file(size_t size,uint32_t offset,uint32_t length){
    return (uint64_t)offset+length<=size;
}
static int section(const uint8_t *data,size_t size,const Elf32_Ehdr *header,
                   unsigned index,Elf32_Shdr *out){
    uint64_t at=(uint64_t)header->e_shoff+(uint64_t)index*header->e_shentsize;
    if(index>=header->e_shnum||at+sizeof(*out)>size)return 0;
    memcpy(out,data+(size_t)at,sizeof(*out));
    return 1;
}
static Symbol find_symbol(const uint8_t *data,size_t size,
                          const Elf32_Ehdr *header,const char *name){
    Symbol absent={0,0};
    for(unsigned i=0;i<header->e_shnum;i++){
        Elf32_Shdr table,strings;
        if(!section(data,size,header,i,&table)||table.sh_type!=SHT_SYMTAB)
            continue;
        if(table.sh_entsize<sizeof(Elf32_Sym)||
           !in_file(size,table.sh_offset,table.sh_size)||
           !section(data,size,header,table.sh_link,&strings)||
           !in_file(size,strings.sh_offset,strings.sh_size))return absent;
        unsigned count=table.sh_size/table.sh_entsize;
        for(unsigned j=0;j<count;j++){
            Elf32_Sym sym;
            memcpy(&sym,data+table.sh_offset+j*table.sh_entsize,sizeof(sym));
            if(sym.st_name>=strings.sh_size||sym.st_shndx==SHN_UNDEF||
               ELF32_ST_TYPE(sym.st_info)!=STT_OBJECT)continue;
            const char *s=(const char *)data+strings.sh_offset+sym.st_name;
            if(!memchr(s,0,strings.sh_size-sym.st_name))return absent;
            if(!strcmp(s,name))return (Symbol){sym.st_value,sym.st_size};
        }
    }
    return absent;
}
static int load_symbols(const char *path,Symbols *s){
    FILE *file=fopen(path,"rb");
    if(!file){perror(path);return 0;}
    if(fseek(file,0,SEEK_END)){fclose(file);return 0;}
    long length=ftell(file);
    if(length<(long)sizeof(Elf32_Ehdr)||length>32*1024*1024){
        fclose(file);return 0;
    }
    rewind(file);
    uint8_t *data=malloc((size_t)length);
    if(!data){fclose(file);return 0;}
    int ok=fread(data,1,(size_t)length,file)==(size_t)length;
    fclose(file);
    Elf32_Ehdr header;
    if(ok){
        memcpy(&header,data,sizeof(header));
        ok=!memcmp(header.e_ident,ELFMAG,SELFMAG)&&
            header.e_ident[EI_CLASS]==ELFCLASS32&&
            header.e_ident[EI_DATA]==ELFDATA2LSB&&header.e_machine==EM_ARM&&
            header.e_shentsize>=sizeof(Elf32_Shdr);
    }
    if(ok){
        s->game=find_symbol(data,(size_t)length,&header,"game");
        s->ticks=find_symbol(data,(size_t)length,&header,"clock_ticks");
        s->read=find_symbol(data,(size_t)length,&header,"frame_runtime_read");
        s->write=find_symbol(data,(size_t)length,&header,"frame_runtime_write");
        s->dropped=find_symbol(data,(size_t)length,&header,"frame_runtime_dropped");
        s->world=find_symbol(data,(size_t)length,&header,"world");
        ok=s->game.address&&s->game.size==sizeof(Story)&&
           s->ticks.address&&s->ticks.size==4u&&s->world.address&&s->world.size>=4u;
    }
    free(data);
    if(!ok)fputs("ELF must be a matching ARM build with game/clock_ticks symbols\n",stderr);
    return ok;
}
static uint32_t read_word(struct mCore *core,Symbol symbol){
    return symbol.address?core->busRead32(core,symbol.address):0u;
}
static Sample sample(struct mCore *core,const Symbols *symbols){
    Sample s;
    memset(&s,0,sizeof(s));
    uint8_t *out=(uint8_t *)&s.game;
    for(unsigned i=0;i<sizeof(Story);i++)
        out[i]=(uint8_t)core->busRead8(core,symbols->game.address+i);
    s.ticks=read_word(core,symbols->ticks);
    s.depth=read_word(core,symbols->write)-read_word(core,symbols->read);
    return s;
}
static void advance(struct mCore *core,unsigned frames){
    while(frames--){
        core->runFrame(core);
        /* New Game resets its session counters. Remember any overflow
           before a later menu/reentry test can clear that evidence. */
        uint32_t dropped=read_word(core,watched_dropped);
        if(dropped>peak_dropped)peak_dropped=dropped;
        for(int channel=0;channel<2;channel++){
            struct blip_t *audio=core->getAudioChannel(core,channel);
            if(audio)blip_clear(audio);
        }
    }
}
static void press(struct mCore *core,unsigned key,unsigned held,unsigned released){
    core->setKeys(core,key);advance(core,held);
    core->setKeys(core,0);advance(core,released);
}
static int mode4(struct mCore *core){
    uint32_t d=core->busRead16(core,0x04000000u);
    return (d&7u)==4u&&(d&0x400u)&&!(d&0x80u);
}
static void picture(struct mCore *core,const char *directory,const char *name){
    if(!directory)return;
    char path[1024];
    int n=snprintf(path,sizeof(path),"%s/runtime-%s.png",directory,name);
    if(n<0||(size_t)n>=sizeof(path)){check(0,"capture path fits");return;}
    struct VFile *file=VFileOpen(path,O_WRONLY|O_CREAT|O_TRUNC);
    if(!file){check(0,"capture file opens");return;}
    int ok=mCoreTakeScreenshotVF(core,file);
    file->close(file);check(ok,"actual ROM capture saved");
}
static void clock_window(struct mCore *core,const Symbols *symbols,
                         const char *name,int cinematic){
    Sample before=sample(core,symbols);
    uint32_t start=core->frameCounter(core);
    advance(core,600u);
    Sample after=sample(core,symbols);
    uint32_t video=core->frameCounter(core)-start;
    uint32_t processed=after.ticks-before.ticks;
    int64_t accounted=(int64_t)processed+after.depth-before.depth;
    int64_t error=accounted-video;
    printf("%s clock: video=%u, processed=%u, queue=%u->%u, accounted=%lld\n",
           name,video,processed,before.depth,after.depth,(long long)accounted);
    check(video==600u,"clock window advances 600 emulated video frames");
    check(before.depth<=64u&&after.depth<=64u,"input queue remains bounded");
    check(error>=-2&&error<=2,"processed plus queued ticks track video time");
    check(processed>=570u&&processed<=630u,"simulation keeps pace during painting");
    if(cinematic){
        unsigned delta=(uint16_t)(after.game.cinema_frame-before.game.cinema_frame);
        printf("  cinematic ticks advanced=%u\n",delta);
        check(before.game.cinema==CINE_OPEN&&after.game.cinema==CINE_OPEN,
              "opening route remains the actual Potheads scene");
        check(delta==processed,"cinematic clock advances with each simulation tick");
    }
    check(mode4(core),"active Mode4 display survives the clock window");
}

int main(int argc,char **argv){
    if(argc<3||argc>4){
        fprintf(stderr,"usage: %s ROM MATCHING_ELF [picture-directory]\n",argv[0]);
        return 2;
    }
    Symbols symbols={0};
    if(!load_symbols(argv[2],&symbols))return 2;
    watched_dropped=symbols.dropped;
    mLogSetDefaultLogger(&logger);
    struct mCore *core=mCoreFind(argv[1]);
    if(!core){fputs("No GBA core for ROM\n",stderr);return 2;}
    mCoreInitConfig(core,NULL);
    if(!core->init(core))return 2;
    if(core->platform(core)!=mPLATFORM_GBA||!mCoreLoadFile(core,argv[1]))return 2;
    unsigned width=0,height=0;
    core->desiredVideoDimensions(core,&width,&height);
    color_t *pixels=calloc((size_t)width*height,sizeof(*pixels));
    if(!pixels||width!=240u||height!=160u)return 2;
    core->setVideoBuffer(core,pixels,width);core->reset(core);
    printf("mGBA %s; read-only runtime verification, Story bytes=%zu\n",
           projectVersion,sizeof(Story));
    check(symbols.read.address&&symbols.read.size==4u&&
          symbols.write.address&&symbols.write.size==4u&&
          symbols.dropped.address&&symbols.dropped.size==4u,
          "built ROM exports its real input-queue counters");
    advance(core,1650u);
    press(core,KEY_START,32u,127u);
    press(core,KEY_A,32u,127u);
    press(core,KEY_A,32u,152u);
    const char *directory=argc==4?argv[3]:NULL;
    Sample initial=sample(core,&symbols);
    unsigned seed=read_word(core,symbols.world);
    if(!check(seed<STORY_WORLD_LAYOUTS,"New Game selects a valid generated layout"))goto finish;
    story_world_set(seed);
    printf("generated world seed: %u\n",seed);
    printf("opening state: scene=%u, dialogue=%u, page=%u, ticks=%u\n",
           initial.game.cinema,initial.game.dialogue,initial.game.page,initial.ticks);
    if(!check(initial.game.cinema==CINE_OPEN&&initial.game.dialogue,
              "New Game reaches the scripted opening"))goto finish;
    check(initial.game.page==0u,"held menu confirm does not skip opening dialogue");
    picture(core,directory,"opening");
    clock_window(core,&symbols,"opening",1);

    Sample before=sample(core,&symbols);
    press(core,KEY_A,1u,32u);
    Sample after=sample(core,&symbols);
    check(after.game.dialogue==before.game.dialogue&&
          after.game.page==(unsigned)before.game.page+1u,
          "one-frame A press advances exactly one dialogue page");
    press(core,KEY_B,32u,96u);
    Sample town=sample(core,&symbols);
    if(!check(!town.game.cinema&&!town.game.dialogue&&!town.game.view,
              "B reaches overhead town gameplay"))goto finish;
    picture(core,directory,"town");
    clock_window(core,&symbols,"town",0);

    unsigned taps=0,min_latency=1000u,max_latency=0u;
    for(unsigned phase=0;phase<12u;phase++){
        advance(core,phase);
        unsigned old_view=sample(core,&symbols).game.view;
        core->setKeys(core,KEY_L);advance(core,1u);core->setKeys(core,0);
        unsigned latency=1u;
        while(latency<48u&&sample(core,&symbols).game.view==old_view){
            advance(core,1u);latency++;
        }
        unsigned expected=old_view^1u;
        if(sample(core,&symbols).game.view==expected){
            taps++;
            if(latency<min_latency)min_latency=latency;
            if(latency>max_latency)max_latency=latency;
        }
        advance(core,32u);
        check(sample(core,&symbols).game.view==expected,
              "one-frame camera tap is captured once while drawing");
    }
    printf("short camera taps: %u/12; observed response %u..%u video frames\n",
           taps,min_latency,max_latency);
    check(taps==12u,"all tested one-frame camera presses are retained");
    check(taps==12u&&max_latency<=12u,
          "tested camera actions apply within twelve video frames");
    before=sample(core,&symbols);
    unsigned camera_changes=0,previous_view=before.game.view;
    core->setKeys(core,KEY_L);
    for(unsigned frame=0;frame<80u;frame++){
        if(frame==40u)core->setKeys(core,0);
        advance(core,1u);
        unsigned view=sample(core,&symbols).game.view;
        if(view!=previous_view){camera_changes++;previous_view=view;}
    }
    after=sample(core,&symbols);
    printf("held camera button: %u observed toggle(s)\n",camera_changes);
    check(camera_changes==1u&&after.game.view==(before.game.view^1u),
          "holding camera button produces one toggle without repeats");
    if(!after.game.view)press(core,KEY_L,1u,48u);
    if(check(sample(core,&symbols).game.view==1u,"first-person camera is active")){
        picture(core,directory,"first-person");
        clock_window(core,&symbols,"first-person",0);
        for(unsigned turn=0;turn<4u;turn++){
            unsigned old_facing=sample(core,&symbols).game.facing;
            core->setKeys(core,KEY_RIGHT);advance(core,1u);core->setKeys(core,0);
            unsigned latency=1u;
            while(latency<48u&&sample(core,&symbols).game.facing==old_facing){
                advance(core,1u);latency++;
            }
            advance(core,32u);
            unsigned facing=sample(core,&symbols).game.facing;
            printf("first-person turn: %u->%u, response=%u video frames\n",
                   old_facing,facing,latency);
            check(facing==((old_facing+1u)&3u)&&latency<=12u,
                  "single-frame turn is retained promptly in each facing");
            char name[40];
            snprintf(name,sizeof(name),"facing-%u",facing);
            picture(core,directory,name);
        }
    }
    press(core,KEY_L,1u,48u);
    if(check(sample(core,&symbols).game.view==0u,"camera returns to overhead")){
        before=sample(core,&symbols);
        Story expected=before.game;
        /* The actual generated storefront may face either east or west.
           Walk away from it along the same relocated street. */
        unsigned walk=story_world_facing(2u)==2u?KEY_LEFT:KEY_RIGHT;
        for(unsigned i=0;i<18u;i++)story_tick(&expected,walk);
        for(unsigned i=0;i<48u;i++)story_tick(&expected,0u);
        press(core,walk,18u,48u);
        after=sample(core,&symbols);
        printf("timed walk: before=(%u,%u), expected=(%u,%u), ROM=(%u,%u)\n",
               before.game.x,before.game.y,expected.x,expected.y,after.game.x,after.game.y);
        check(expected.x!=before.game.x||expected.y!=before.game.y,
              "walk scenario actually moves through the original map");
        check(after.game.x==expected.x&&after.game.y==expected.y&&
              after.game.facing==expected.facing&&after.game.anim==expected.anim,
              "held movement matches its real video-frame duration");
        picture(core,directory,"after-walk");
        /* Secret menu is present in the same ordinary ROM as all other
           tests. Use real pad input, never a debug memory write. */
        unsigned old_seed=read_word(core,symbols.world);
        press(core,KEY_START,1u,48u);
        static const unsigned code[]={64u,64u,128u,128u,32u,16u,32u,16u,2u,1u,8u};
        for(unsigned i=0;i<sizeof code/sizeof code[0];i++){
            press(core,code[i],1u,12u);
            Sample k=sample(core,&symbols);
            check(!!story_debug_menu(&k.game)==(i+1u==sizeof code/sizeof code[0]),
                  "Konami code recognizes only the full pause-screen sequence");
        }
        Sample unlocked=sample(core,&symbols);
        check(story_debug_menu(&unlocked.game)&&!unlocked.game.quit&&read_word(core,symbols.world)==old_seed,
              "regular cartridge opens hidden World Tools without regeneration");
        press(core,KEY_B,1u,48u);
        Sample locked=sample(core,&symbols);
        check(!locked.game.paused&&!story_debug_menu(&locked.game)&&read_word(core,symbols.world)==old_seed,
              "closing the secret menu relocks it without changing the world");
        press(core,KEY_START,1u,48u);
        check(sample(core,&symbols).game.paused==1u,
              "short pause press works after movement");
        press(core,KEY_B,1u,120u);
        check(sample(core,&symbols).game.quit==1u,
              "short back press returns through the pause menu");
        uint32_t stopped=read_word(core,symbols.write);
        advance(core,120u);
        check(read_word(core,symbols.write)==stopped&&sample(core,&symbols).depth==0u,
              "gameplay input capture stops cleanly in the menu");
        picture(core,directory,"returned-menu");
        press(core,KEY_A,32u,127u);
        press(core,KEY_A,32u,152u);
        Sample restarted=sample(core,&symbols);
        check(restarted.game.cinema==CINE_OPEN&&restarted.game.page==0u&&
              !restarted.game.quit&&read_word(core,symbols.write)>0u,
              "New Game restarts the runtime without stale keys or pages");
        unsigned restarted_seed=read_word(core,symbols.world);
        check(restarted_seed<STORY_WORLD_LAYOUTS&&restarted_seed!=seed,
              "another New Game automatically selects a different generated world");
    }
finish:
    printf("input queue dropped samples: current=%u, peak across sessions=%u\n",
           read_word(core,symbols.dropped),peak_dropped);
    check(symbols.dropped.address&&peak_dropped==0u&&
          read_word(core,symbols.dropped)==0u,
          "no captured input samples overflow during the tested routes");
    check(emulator_errors==0u,"emulator reports no fatal/error events");
    printf("Runtime verification: %u failure(s)\n",failures);
    mCoreConfigDeinit(&core->config);core->deinit(core);free(pixels);
    return failures?1:0;
}
