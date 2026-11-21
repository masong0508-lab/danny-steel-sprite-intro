/*
 * Deterministic picture capture from the exact same C pixel painter
 * as the GBA ROM. These are game-engine frames, not concept mockups.
 * Top-level title/menu frames come from a separate headless mGBA run.
 */
#include "story.h"
#include "story_draw.h"
#include "studio.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static uint8_t canvas[120*80];
static char outdir[512];
static void write_frame(const char *name,const Story *s,unsigned t){
 char filename[700];
 snprintf(filename,sizeof(filename),"%s/%s.ppm",outdir,name);
 memset(canvas,0,sizeof canvas);
 story_draw(s,(int)t,canvas);
 FILE *f=fopen(filename,"wb");if(!f){perror(filename);return;}
 fprintf(f,"P6\n120 80\n255\n");
 for(unsigned i=0;i<120u*80u;i++){
  assert(canvas[i]<32u);
  uint16_t p=story_palette[canvas[i]];
  uint8_t rgb[3]={(uint8_t)((p&31u)*255u/31u),
     (uint8_t)(((p>>5)&31u)*255u/31u),
     (uint8_t)(((p>>10)&31u)*255u/31u)};
  assert(fwrite(rgb,1,3,f)==3);
 }
 fclose(f);
 puts(filename);
}
static void press(Story *s,unsigned key){
 story_tick(s,0u);story_tick(s,(uint16_t)key);story_tick(s,0u);
}
static void done(Story *s){
 for(int i=0;i<50 && s->dialogue;i++)press(s,1u);
 assert(!s->dialogue);
}
static void stand(Story *s,int x,int y,int facing){
 assert(!story_solid(x,y));
 s->x=(uint8_t)x;s->y=(uint8_t)y;s->facing=(uint8_t)facing;
 s->prev_x=s->x;s->prev_y=s->y;s->cooldown=0;
}
int main(int argc,char **argv){
 if(argc!=2)return fprintf(stderr,"usage: gallery OUTDIR\n"),2;
 snprintf(outdir,sizeof(outdir),"%s",argv[1]);
 Story s;
 story_draw_init();
 story_new(&s,0);
 /* Beginning of the real title-to-story transition. */
 assert(s.cinema==CINE_OPEN);
 s.cinema_frame=80;
 write_frame("05_story_arrival",&s,80);
 done(&s);
 s.cinema=CINE_NONE;s.scene=STORY_SCENE_NONE;s.view=0;
 stand(&s,12,13,3);
 write_frame("06_gameplay_overhead",&s,90);
 s.view=1;
 write_frame("07_gameplay_first_person",&s,90);
 s.view=0;
 stand(&s,11,8,3);
 press(&s,1u);
 assert(s.cinema==CINE_BORE);
 s.cinema_frame=95;
 write_frame("08_story_bore_office",&s,95);
 done(&s);
 stand(&s,8,12,0);
 press(&s,1u);
 assert(s.cinema==CINE_MARA);
 s.cinema_frame=85;
 write_frame("09_story_mara_cafe",&s,85);
 done(&s);
 stand(&s,19,4,1);
 press(&s,1u);
 assert(s.cinema==CINE_BUS);
 s.cinema_frame=105;
 write_frame("10_story_last_bus",&s,105);
 /* A separate completed save state, via the same production editor. */
 story_new(&s,1);done(&s);
 s.flags=STORY_FULL;s.life_stage=STORY_LIFE_WAIT;
 s.song_steps=STORY_PROJECT_DONE;
 stand(&s,19,8,3);
 press(&s,1u);
 assert(s.activity=='E');
 press(&s,16u);press(&s,1u);
 assert(s.activity==STORY_ACT_STUDIO);
 s.studio_tab=STUDIO_NOTES;s.studio_cursor=3;
 write_frame("11_advanced_studio_notes",&s,110);
 s.studio_tab=STUDIO_PERFORM;s.activity_choice=1;
 studio_motif_step(18u,&(MotifBeat){0});
 write_frame("12_advanced_studio_perform",&s,110);
 puts("Actual Danny Steel story renderer gallery generated");
 return 0;
}
