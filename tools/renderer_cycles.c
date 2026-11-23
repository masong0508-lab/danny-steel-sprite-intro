/* Profiling-only mGBA probe for an ARM GBA build with GBA_PROFILE_RENDER.
   Unlike host microbenchmarks, each value is read from cascaded ARM timers
   while the real ROM draws sky, walls, floor, billboards and weather.
   No emulator memory writes, cheats, saves or tampered frame clocks. */
#define _POSIX_C_SOURCE 200809L
#include <elf.h>
#include <mgba/core/core.h>
#include <mgba/core/blip_buf.h>
#include <mgba-util/vfs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { KEY_START=8u,KEY_A=1u,KEY_B=2u,KEY_UP=64u,KEY_DOWN=128u,
       KEY_L=512u,WINDOW_FRAMES=600u,STAGES=7u };
static const char *const names[STAGES+1u]={
 "gba_profile_frames",
 "gba_profile_sky_cycles",
 "gba_profile_wall_cycles",
 "gba_profile_floor_cycles",
 "gba_profile_billboard_cycles",
 "gba_profile_weather_cycles",
 "gba_profile_total_cycles",
 "gba_profile_wallshade_cycles"
};
static const char *const labels[STAGES]={
 "sky and skyline","walls and depth","floor and materials",
 "billboards/occlusion","weather/rain","total renderer",
 "wall texture paint (part of walls)"
};
static int range_ok(size_t size,uint64_t offset,uint64_t length){
 return offset<=size&&length<=size-offset;
}
static int section(const uint8_t *bytes,size_t size,const Elf32_Ehdr *h,
                   unsigned id,Elf32_Shdr *out){
 if(id>=h->e_shnum||h->e_shentsize<sizeof(*out))return 0;
 uint64_t where=(uint64_t)h->e_shoff+(uint64_t)id*h->e_shentsize;
 if(!range_ok(size,where,sizeof(*out)))return 0;
 memcpy(out,bytes+where,sizeof(*out));return 1;
}
static int locate_symbols(const char *path,uint32_t out[STAGES+1u]){
 FILE *in=fopen(path,"rb");if(!in){perror(path);return 0;}
 if(fseek(in,0,SEEK_END)){fclose(in);return 0;}
 long n=ftell(in);
 if(n<(long)sizeof(Elf32_Ehdr)||n>32*1024*1024){
  fclose(in);fputs("Invalid profiler ELF size\n",stderr);return 0;
 }
 rewind(in);
 uint8_t *bytes=malloc((size_t)n);if(!bytes){fclose(in);return 0;}
 int ok=fread(bytes,1,(size_t)n,in)==(size_t)n;
 fclose(in);
 Elf32_Ehdr h;
 if(ok){
  memcpy(&h,bytes,sizeof(h));
  ok=!memcmp(h.e_ident,ELFMAG,SELFMAG)&&h.e_ident[EI_CLASS]==ELFCLASS32&&
      h.e_ident[EI_DATA]==ELFDATA2LSB&&h.e_machine==EM_ARM;
 }
 if(ok)for(unsigned i=0;i<h.e_shnum;i++){
  Elf32_Shdr symtab,strings;
  if(!section(bytes,(size_t)n,&h,i,&symtab)) {ok=0;break;}
  if(symtab.sh_type!=SHT_SYMTAB)continue;
  if(symtab.sh_entsize<sizeof(Elf32_Sym)||
     !range_ok((size_t)n,symtab.sh_offset,symtab.sh_size)||
     !section(bytes,(size_t)n,&h,symtab.sh_link,&strings)||
     !range_ok((size_t)n,strings.sh_offset,strings.sh_size)){ok=0;break;}
  for(unsigned k=0;k<symtab.sh_size/symtab.sh_entsize;k++){
   Elf32_Sym sym;
   memcpy(&sym,bytes+symtab.sh_offset+k*symtab.sh_entsize,sizeof(sym));
   if(sym.st_name>=strings.sh_size||sym.st_shndx==SHN_UNDEF||
      ELF32_ST_TYPE(sym.st_info)!=STT_OBJECT||sym.st_size!=4u)continue;
   const char *name=(const char *)bytes+strings.sh_offset+sym.st_name;
   if(!memchr(name,0,strings.sh_size-sym.st_name)){ok=0;break;}
   for(unsigned j=0;j<STAGES+1u;j++)
    if(!strcmp(name,names[j]))out[j]=sym.st_value;
  }
  if(!ok)break;
 }
 free(bytes);
 if(!ok)return 0;
 for(unsigned i=0;i<STAGES+1u;i++)if(!out[i]){
  fprintf(stderr,"Profile ELF is missing %s\n",names[i]);return 0;
 }
 return 1;
}
static void advance(struct mCore *core,unsigned frames){
 while(frames--){
  core->runFrame(core);
  for(int channel=0;channel<2;channel++){
   struct blip_t *audio=core->getAudioChannel(core,channel);
   if(audio)blip_clear(audio);
  }
 }
}
static void press(struct mCore *core,unsigned key){
 core->setKeys(core,key);advance(core,32u);
 core->setKeys(core,0u);advance(core,32u);
}
int main(int argc,char **argv){
 if(argc!=3 && argc!=4){
  fprintf(stderr,"Usage: %s profiled.gba matching.elf [walking]\n",argv[0]);
  return 2;
 }
 int walking=argc==4;
 if(walking && strcmp(argv[3],"walking")){
  fputs("The only optional mode is walking\n",stderr);return 2;
 }
 uint32_t symbol[STAGES+1u]={0};
 if(!locate_symbols(argv[2],symbol))return 2;
 struct mCore *core=mCoreFind(argv[1]);if(!core)return 2;
 mCoreInitConfig(core,NULL);
 if(!core->init(core))return 2;
 if(core->platform(core)!=mPLATFORM_GBA||!mCoreLoadFile(core,argv[1])){
  fputs("Expected a valid GBA ROM\n",stderr);core->deinit(core);return 2;
 }
 unsigned w=0,h=0;core->desiredVideoDimensions(core,&w,&h);
 color_t *video=calloc((size_t)w*h,sizeof(*video));
 if(!video||w!=240u||h!=160u){
  fputs("Expected native GBA video\n",stderr);free(video);core->deinit(core);return 2;
 }
 core->setVideoBuffer(core,video,w);core->reset(core);
 advance(core,1650u);
 press(core,KEY_START);advance(core,95u);
 press(core,KEY_A);advance(core,95u);
 press(core,KEY_A);advance(core,120u);
 press(core,KEY_B);advance(core,120u);
 core->setKeys(core,KEY_L);advance(core,96u);
 core->setKeys(core,0u);advance(core,192u);
 unsigned display=core->busRead16(core,0x04000000u);
 if((display&7u)!=4u||(display&0x0400u)==0u){
  fputs("Failed to reach the native Mode4 scene\n",stderr);
  free(video);core->deinit(core);return 1;
 }
 uint32_t last=core->busRead32(core,symbol[0]);
 uint64_t sum[STAGES]={0};uint32_t lo[STAGES],hi[STAGES];
 for(unsigned i=0;i<STAGES;i++){lo[i]=UINT32_MAX;hi[i]=0;}
 unsigned samples=0u,missed=0u;
 for(unsigned frame=0;frame<WINDOW_FRAMES;frame++){
  /* Match tools/frame_cadence.c: hold reverse/forward for 48 frames
     each on the physically walkable Potheads pavement. This changes
     controller inputs only, never emulator time or game memory. */
  if(walking)
   core->setKeys(core,(frame%96u)<48u?KEY_DOWN:KEY_UP);
  advance(core,1u);
  uint32_t current=core->busRead32(core,symbol[0]);
  if(current==last)continue;
  if(current-last>1u)missed+=current-last-1u;
  last=current;samples++;
  for(unsigned i=0;i<STAGES;i++){
   uint32_t elapsed=core->busRead32(core,symbol[i+1u]);
   sum[i]+=elapsed;
   if(elapsed<lo[i])lo[i]=elapsed;
   if(elapsed>hi[i])hi[i]=elapsed;
  }
 }
 if(walking)core->setKeys(core,0u);
 unsigned hz=(unsigned)core->frequency(core);
 printf("Native ARM renderer stage profiler (%s): %u sample(s) in %u video frames; ",walking?"held walking":"stationary"
        "%u skipped sampled generations; CPU=%u Hz\n",samples,WINDOW_FRAMES,missed,hz);
 if(samples>=20u){
  for(unsigned i=0;i<STAGES;i++){
   double mean=(double)sum[i]/samples;
   printf("%-22s mean %.0f cycles (%.3f ms), min %u, max %u\n",
          labels[i],mean,mean*1000.0/hz,lo[i],hi[i]);
  }
  double paint=(double)sum[6]/samples;
  double allwalls=(double)sum[1]/samples;
  printf("ray traversal plus other wall overhead: %.0f cycles (%.3f ms)\n",
         allwalls-paint,(allwalls-paint)*1000.0/hz);
  printf("Profiled GBA source: complete first-person renderer, "
         "not host execution time or physical hardware timing.\n");
 }
 free(video);mCoreConfigDeinit(&core->config);core->deinit(core);
 return samples<20u?1:0;
}
