/*
 * Deterministic picture capture from the exact same C pixel painter
 * as the GBA ROM. These are game-engine frames, not concept mockups.
 * Top-level title/menu frames come from a separate headless mGBA run.
 */
#include "story.h"
#include "story_draw.h"
#include "studio.h"
#include "hd_canvas.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static uint8_t canvas[HD_BYTES];
static char outdir[512];
static void write_frame(const char *name,const Story *s,unsigned t){
 char filename[700];
 snprintf(filename,sizeof(filename),"%s/%s.ppm",outdir,name);
 memset(canvas,0,sizeof canvas);
 if(s->battle){
  hd_arena_draw(canvas,s,t);
 }else if(s->life_stage==STORY_LIFE_FINALE ||
    (s->ended && (s->ending==STORY_END_GOOD ||
                  s->ending==STORY_END_NEUTRAL ||
                  s->ending==STORY_END_BAD))){
  hd_ending_draw(canvas,s,s->finale_frame);
 }else if(s->activity==STORY_ACT_STUDIO){
  hd_studio_draw(canvas,s,t);
 }else if(!s->ended && !s->battle && s->life_stage!=STORY_LIFE_FINALE &&
          s->cinema==CINE_NONE && s->scene==STORY_SCENE_NONE){
  if(s->view)hd_first_person_draw(canvas,s,t);
  else hd_world_draw(canvas,s,t);
 }else if(!s->ended && !s->battle && s->life_stage!=STORY_LIFE_FINALE &&
          hd_cinema_supports(s->cinema)){
  hd_cinema_draw(canvas,s,t);
 }else{
  Story visual=*s;
  if(visual.dialogue && !visual.ended && visual.life_stage!=STORY_LIFE_FINALE)
    visual.dialogue=0;
  story_draw(&visual,(int)t,canvas);
  hd_upscale_inplace(canvas);
 }
 hd_game_overlay(canvas,s);
 FILE *f=fopen(filename,"wb");if(!f){perror(filename);return;}
 fprintf(f,"P6\n240 160\n255\n");
 for(unsigned i=0;i<HD_BYTES;i++){
  assert(canvas[i]<64u);
  uint16_t p=canvas[i]<32u?story_palette[canvas[i]]:
                            hd_extra_palette[canvas[i]-32u];
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
 s.cinema_frame=0;
 write_frame("05a_potheads_aerial",&s,0);
 s.cinema_frame=90;
 write_frame("05b_potheads_descent",&s,90);
 s.cinema_frame=180;
 write_frame("05c_potheads_closeup",&s,180);
 s.cinema_frame=180;
 write_frame("05_story_arrival",&s,180);
 s.cinema_frame=420;
 s.page=1;
 write_frame("05d_potheads_dance",&s,420);
 s.cinema_frame=600;
 s.page=2;
 write_frame("05e_potheads_gratitude",&s,600);
 s.cinema_frame=720;
 s.page=3;
 write_frame("05f_potheads_after_hours",&s,720);
 /* Native renderer frames cover the entire authored floor routine and exit.
    The companion opening.mp4 is captured separately from the running ROM. */
 s.cinema_frame=780;
 write_frame("05fa_potheads_inverted",&s,780);
 s.cinema_frame=860;
 s.page=4;
 write_frame("05fb_potheads_bridge",&s,860);
 s.cinema_frame=990;
 write_frame("05fc_potheads_support_change",&s,990);
 s.cinema_frame=1110;
 s.page=5;
 write_frame("05fd_potheads_leg_sweep",&s,1110);
 s.cinema_frame=1230;
 s.page=6;
 write_frame("05fe_potheads_right_exit",&s,1230);
 s.cinema_frame=1273;
 write_frame("05ff_potheads_final",&s,1273);
 done(&s);
 /* Engine-produced performance frames; these are not Photoshop mockups.
    Additional story scenes share camera rigs and all their world props. */
 s.cinema=CINE_LETTER;s.dialogue=0;s.page=2;
 write_frame("05g_letter_reach",&s,140);
 s.cinema=CINE_BORE;s.page=2;
 write_frame("05h_bore_stamp",&s,140);
 s.cinema=CINE_MARA;s.page=2;
 write_frame("05i_mara_table",&s,140);
 s.cinema=CINE_BUS;s.page=2;
 write_frame("05j_bus_boarding",&s,160);
 s.cinema=CINE_ART;s.page=2;
 write_frame("05k_rooftop_brush",&s,215);
 s.cinema=CINE_LAST_CLINIC;s.page=2;
 s.rescue_stage=STORY_RESCUE_NONE;
 write_frame("05l_clinic_last_visit",&s,170);
 s.rescue_stage=STORY_RESCUE_MISSING;
 write_frame("05m_clinic_mara_missing",&s,170);
 s.rescue_stage=STORY_RESCUE_SAVED;
 s.cinema=CINE_RESCUE_SAVED;
 write_frame("05n_rescue_reunion",&s,225);
 write_frame("05t_rescue_contact",&s,265);
 s.rescue_stage=STORY_RESCUE_NONE;
 /* Story-conditioned photographic cuts at full native GBA resolution. */
 s.cinema=CINE_LETTER;s.page=3;
 write_frame("05o_letter_reaction",&s,174);
 s.cinema=CINE_MARA;s.page=3;
 write_frame("05p_mara_reaction",&s,177);
 s.cinema=CINE_BUS;s.page=2;
 write_frame("05q_bus_reaction",&s,195);
 s.cinema=CINE_ART;s.page=1;
 write_frame("05r_rooftop_reaction",&s,227);
 s.cinema=CINE_LAST_CLINIC;s.page=1;
 write_frame("05s_doctor_reaction",&s,160);
 s.page=2;
 write_frame("05u_clinic_mara_reaction",&s,178);
 s.page=3;
 write_frame("05v_clinic_danny_reaction",&s,215);
 s.cinema=CINE_MARA;s.page=3;
 write_frame("05w_vivid_mara_portrait",&s,177);
 s.cinema=CINE_ART;s.page=1;
 write_frame("05x_vivid_rooftop",&s,227);
 s.cinema=CINE_CLINIC;s.page=2;
 write_frame("05y_vivid_clinic",&s,178);
 s.cinema=CINE_FOLLOWUP;s.page=0;
 write_frame("05z_followup_establishing",&s,0);
 s.page=1;
 write_frame("05za_followup_mara",&s,113);
 s.page=2;
 write_frame("05zb_followup_danny",&s,202);
 s.rescue_stage=STORY_RESCUE_LOST;s.page=1;
 write_frame("05zc_followup_alone",&s,113);
 s.rescue_stage=STORY_RESCUE_NONE;
 s.cinema=CINE_NONE;s.scene=STORY_SCENE_NONE;s.view=0;
 /* The first controllable view is now the same storefront as the film. */
 stand(&s,20,11,0);
 write_frame("06_gameplay_overhead",&s,90);
 /* The same real map in four directions, captured from valid walkable
    tiles. Material, silhouette, footprint and foliage differ by location. */
 stand(&s,11,8,1);
 write_frame("33_quarter_archive_plaza",&s,145);
 stand(&s,8,12,3);
 write_frame("34_quarter_mara_street",&s,155);
 stand(&s,19,4,1);
 write_frame("35_quarter_bus_and_facades",&s,162);
 stand(&s,3,8,3);
 write_frame("36_quarter_riverside_residents",&s,177);
 /* The storefront is also a real walkable town facade, not just opening art. */
 stand(&s,20,11,1);
 write_frame("37_potheads_storefront",&s,184);
 stand(&s,10,7,0);
 write_frame("38_civic_facades",&s,191);
 s.view=1;
 stand(&s,20,11,0);
 write_frame("39_potheads_first_person",&s,205);
 s.view=0;
 stand(&s,12,13,3);
 s.view=1;
 write_frame("07_gameplay_first_person",&s,90);
 /* Same navigable grid, additional cardinal views of the native renderer. */
 stand(&s,10,7,0);
 write_frame("13_3d_from_archive",&s,107);
 stand(&s,18,13,2);
 write_frame("14_3d_waterfront",&s,121);
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
 studio_motif_step(17u,&(MotifBeat){0});
 write_frame("12a_advanced_studio_rest",&s,110);
 studio_motif_step(18u,&(MotifBeat){0});
 s.studio_tab=STUDIO_SONG;s.studio_cursor=0;
 write_frame("15_advanced_studio_song",&s,125);
 s.studio_tab=STUDIO_ORDER;s.studio_cursor=2;
 write_frame("16_advanced_studio_order",&s,135);
 s.studio_tab=STUDIO_LYRICS;
 write_frame("17_advanced_studio_lyrics",&s,148);
 s.studio_tab=STUDIO_CHOIR;
 write_frame("17a_advanced_studio_choir",&s,148);
 /* Additional actual in-game plot states exercise the expanded native
    cinematics; no art is fabricated for the screenshot pipeline. */
 s.activity=0;s.activity_choice=0;s.dialogue=0;s.ended=0;
 s.life_stage=STORY_LIFE_NONE;s.scene=STORY_SCENE_NONE;
 s.cinema=CINE_WITNESS;s.page=1;
 write_frame("18_story_witness_handoff",&s,110);
 s.cinema=CINE_SERVER;s.page=1;
 write_frame("19_story_archive_terminal",&s,119);
 s.cinema=CINE_SAFE;s.page=1;
 write_frame("20_story_safehouse",&s,124);
 s.cinema=CINE_MARA_EMPTY;s.page=1;
 write_frame("21_story_mara_missing_cafe",&s,131);
 write_frame("21a_mara_cafe_rain_and_empty_chair",&s,270);
 s.cinema=CINE_RESCUE_DOOR;s.page=1;
 write_frame("22_story_closed_rescue_door",&s,133);
 s.cinema=CINE_RESCUE_SAVED;s.page=1;
 write_frame("23_story_rescue_saved",&s,150);
 write_frame("23a_rescue_hands_meet",&s,270);
 write_frame("23b_rescue_step_out",&s,170);
 write_frame("23c_rescue_cross_threshold",&s,195);
 s.cinema=CINE_NONE;s.ended=1;s.dialogue=0;
 s.ending=STORY_END_NEUTRAL;s.rescue_stage=STORY_RESCUE_NONE;
 s.finale_frame=3790u;
 write_frame("24_neutral_rooftop_memories",&s,3790);
 s.ending=STORY_END_GOOD;s.finale_frame=2900u;
 write_frame("25_good_ending_recordshop",&s,2900);
 s.finale_frame=4750u;
 write_frame("26_good_ending_concert",&s,4750);
 /* Direct native renderer frames of the same real finale locations.
    These expose prop staging before the timecoded subtitle overlay. */
 s.ended=0;s.cinema=CINE_GOOD;s.page=3;
 write_frame("25a_recordstore_cinematic",&s,2910);
 s.page=6;
 write_frame("26a_memorial_concert_cinematic",&s,4760);
 s.ended=1;s.cinema=CINE_NONE;
 s.ending=STORY_END_BAD;s.rescue_stage=STORY_RESCUE_LOST;
 s.finale_frame=990u;
 write_frame("27_bad_unanswered_cafe",&s,990);
 s.rescue_stage=STORY_RESCUE_AFTERSHOCK;
 s.finale_frame=1930u;
 write_frame("28_bad_saved_rescue_door",&s,1930);
 /* All three rival identities, with a playable menu and a genuine
    dodge beat. These captures derive from transient story battle state,
    never from a separate concept-art mockup. */
 s.battle=BATTLE_MENU;s.ended=0;s.dialogue=0;s.life_stage=STORY_LIFE_NONE;
 s.cinema=CINE_NONE;s.activity=0;s.battle_focus=4;
 s.cult_stage=0;s.cult_enemy=0;s.battle_choice=0;
 s.battle_lane=1;s.battle_timer=0;s.battle_progress=0;
 s.battle_parry=0;s.battle_guard=0;
 write_frame("29_orchard_rat_menu",&s,194);
 s.cult_enemy=1;s.battle_choice=1;
 write_frame("30_orchard_badge_care",&s,227);
 s.cult_enemy=2;s.battle=BATTLE_DODGE;
 s.battle_lane=2;s.battle_timer=11;s.battle_progress=1;
 write_frame("31_orchard_warden_dodge",&s,251);
 s.cult_enemy=0;s.cult_stage=CULT_RESOLVED;s.battle_timer=8;
 s.battle_lane=0;s.battle_focus=2;s.battle_parry=5;
 write_frame("32_orchard_rebrand_parry",&s,291);
 s.battle=0;
 puts("Actual Danny Steel story renderer gallery generated");
 return 0;
}

