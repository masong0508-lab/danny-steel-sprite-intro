#include "hd_canvas.h"
#include "studio.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static struct {
 uint8_t before[64],pixel[HD_BYTES],after[64];
} frame;
static uint8_t previous[HD_BYTES];
static unsigned render(Story *s,unsigned tick){
 StudioPatch saved=*studio_patch();
 unsigned head=studio_playhead(),part=studio_playpart();
 memset(&frame,0xA5,sizeof frame);
 hd_studio_draw(frame.pixel,s,tick);
 for(unsigned i=0;i<64u;i++){
  assert(frame.before[i]==0xA5&&frame.after[i]==0xA5);
 }
 unsigned hash=2166136261u;
 for(unsigned i=0;i<HD_BYTES;i++){
  assert(frame.pixel[i]<64u);
  hash=(hash^frame.pixel[i])*16777619u;
 }
 assert(!memcmp(&saved,studio_patch(),sizeof saved));
 assert(head==studio_playhead()&&part==studio_playpart());
 return hash;
}
static unsigned pixel(int x,int y){return frame.pixel[y*HD_W+x];}
static void set(unsigned tab,unsigned cursor,unsigned value){
 const StudioPatch *p=studio_patch();
 unsigned old=tab==STUDIO_CHOIR?p->choir:tab==STUDIO_SONG?
  (cursor?p->mood:p->tempo):tab==STUDIO_ORDER?p->order[cursor]:p->notes[cursor];
 if(tab==STUDIO_NOTES){
  if(!value){if(old)studio_toggle(tab,cursor);return;}
  if(!old){studio_toggle(tab,cursor);old=studio_patch()->notes[cursor];}
 }
 studio_change(tab,cursor,(int)value-(int)old);
}
int main(void){
 Story s={0};studio_reset();s.activity=STORY_ACT_STUDIO;s.song_steps=STORY_PROJECT_DONE;
 /* A stopped transport must not animate the record or announce sound. */
 s.studio_tab=STUDIO_SONG;s.activity_choice=0;
 unsigned stopped=render(&s,0);
 assert(stopped==render(&s,26));
 s.activity_choice=1;
 unsigned moving=render(&s,0);
 assert(moving!=render(&s,26));
 assert(pixel(177,10)==HD_LEAF_LIT);
 /* The gap between values and turntable stays physically separate. */
 for(int y=84;y<129;y++)for(int x=160;x<169;x++)assert(pixel(x,y)==HD_INK);

 s.studio_tab=STUDIO_NOTES;s.studio_cursor=3;s.activity_choice=0;
 render(&s,0);
 /* Selection formerly erased these pitch-row lines. */
 for(int y=88;y<118;y+=5)assert(pixel(103,y)==HD_STONE_DARK);
 assert(pixel(103,118)==HD_COPPER);
 assert(pixel(99,104)==HD_PAPER); /* E4 at the fourth step */
 studio_change(STUDIO_NOTES,3,1);
 render(&s,0);
 assert(pixel(99,99)==HD_PAPER); /* UP raises to F4 */
 assert(pixel(99,104)==HD_ROAD_COOL);
 s.activity_choice=1;studio_motif_step(3u,&(MotifBeat){0});
 render(&s,0);
 assert(pixel(96,99)==HD_LEAF_LIT); /* playback beside the note */
 assert(pixel(99,99)==HD_PAPER);   /* note remains unobscured */
 studio_toggle(STUDIO_NOTES,3);render(&s,0);
 assert(pixel(104,120)==HD_STONE); /* REST marker, no false note */

 /* Every preview bar agrees with the real emitted pitch, including
    changed registers and rests. It must not be decorative random bars. */
 s.studio_tab=STUDIO_ORDER;s.studio_cursor=1;s.activity_choice=0;
 for(unsigned variant=0;variant<4u;variant++){
  for(unsigned part=0;part<4u;part++)set(STUDIO_ORDER,part,variant);
  for(unsigned step=0;step<8u;step++)set(STUDIO_NOTES,step,step);
  for(unsigned part=0;part<4u;part++)for(unsigned step=0;step<8u;step++){
   MotifBeat actual;studio_motif_step(part*8u+step,&actual);render(&s,0);
   int x=18+(int)part*53+4+(int)step*5;
   unsigned height=0;
   for(int y=101;y<121;y++)if(pixel(x,y)==HD_GOLD)height++;
   assert(height==(actual.lead>0?(unsigned)actual.lead-57u:0u));
  }
 }

 /* CHOIR pattern lights and LIVE gates match actual PSG events for all
    moods, harmony settings, parts and steps. Drawing is read-only. */
 unsigned gates=0;
 for(unsigned mood=0;mood<3u;mood++)for(unsigned harmony=0;harmony<4u;harmony++){
  set(STUDIO_SONG,1,mood);set(STUDIO_CHOIR,0,harmony);
  for(unsigned part=0;part<4u;part++)for(unsigned step=0;step<8u;step++){
   MotifBeat actual;studio_motif_step(part*8u+step,&actual);
   unsigned active[4]={actual.lead>0,actual.comp!=0,actual.bass!=0,actual.noise!=0};
   s.activity_choice=1;s.studio_tab=STUDIO_PERFORM;render(&s,0);
   for(unsigned voice=0;voice<4u;voice++)
    assert(pixel(22+(int)voice*53,115)==(active[voice]?HD_LAMP:HD_STONE_DARK));
   s.studio_tab=STUDIO_CHOIR;render(&s,0);
   for(unsigned voice=0;voice<4u;voice++)
    assert(pixel(19+(int)voice*53+(int)step*6,118)==
           (active[voice]?HD_GOLD:HD_STONE_DARK));
   assert(pixel(18+(int)harmony*53,84)==HD_LAMP);
   gates++;
  }
 }
 s.studio_tab=STUDIO_PERFORM;s.activity_choice=0;
 unsigned stopped_stage=render(&s,0);
 for(unsigned voice=0;voice<4u;voice++)
  assert(pixel(22+(int)voice*53,115)==HD_STONE_DARK);
 /* All four hand-drawn instrument silhouettes must exist at rest, and
    a stopped transport must not animate or mutate the sequencer. */
 assert(pixel(55,110)==HD_COPPER); /* lead microphone stand */
 assert(pixel(108,108)==HD_PAPER); /* actual keyboard whites */
 assert(pixel(211,106)==HD_STONE_LIT); /* drum cymbal, clear of its stand */
 assert(stopped_stage==render(&s,173));

 s.studio_tab=STUDIO_LYRICS;s.activity_choice=1;
 studio_motif_step(0u,&(MotifBeat){0});render(&s,0);
 memcpy(previous,frame.pixel,HD_BYTES);
 studio_motif_step(7u,&(MotifBeat){0});render(&s,0);
 assert(memcmp(previous,frame.pixel,HD_BYTES)); /* follows phrase */
 s.activity_choice=0;unsigned lyric_still=render(&s,0);
 studio_motif_step(0u,&(MotifBeat){0});assert(lyric_still==render(&s,200));
 for(unsigned tab=0;tab<STUDIO_TAB_COUNT;tab++){
  s.studio_tab=(uint8_t)tab;
  for(unsigned cursor=0;cursor<studio_cursor_count(tab);cursor++){
   s.studio_cursor=(uint8_t)cursor;
   for(unsigned playing=0;playing<2u;playing++){
    s.activity_choice=(uint8_t)playing;render(&s,123u);
   }
  }
 }
 printf("studio renderer: transport, selection, pitch previews and %u voice/event states verified\n",gates);
 puts("all six tabs: guarded palette, read-only sequencer, real controls and phrase cues");
 return 0;
}
