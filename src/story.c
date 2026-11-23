#include "story.h"
#include "story_checks.h"
#include "studio.h"

/* A quiet neighborhood, not a random maze. Every story anchor is reachable on foot. */
static const char *const town[STORY_H] = {
"########################",
"#......................#",
"#...TT.H..........TT...#",
"#..TTTT...####....TT...#",
"#.........#AA#.........#",
"#...L.....#..#..K..B...#",
"#.........####R........#",
"#..P.......C.....V.M.S.#",
"#...,,,,,,,,,,,,,,,,...#",
"#..T,,,......,,,T.gg...#",
"#..TT,,......,,TT.gg####",
"#...g,W.F....,,......#.#",
"#...g,,,,,,,,,,...Q..#.#",
"#..TT.......D....gg..#.#",
"#..TT............~~~...#",
"#...............~~~~...#",
"#...............~~~~...#",
"########################"
};

char story_tile(int x, int y) {
    if ((unsigned)x >= STORY_W || (unsigned)y >= STORY_H) return '#';
    /* All gameplay entities have fixed tile locations. Store the exact
       original decorated grid directly: the old ten coordinate branches
       ran once for EVERY wall ray and floor sample. Save RAM and cycles
       while preserving the existing interaction and collision positions. */
    return town[y][x];
}
int story_goal(const Story *s, int *x, int *y, const char **label) {
    if (s->ended || s->life_stage==STORY_LIFE_FINALE) return 0;
    if(s->rescue_stage==STORY_RESCUE_MISSING){
        *x=6;*y=11;*label="WITNESS";return 1;
    }
    if(s->rescue_stage==STORY_RESCUE_WITNESS){
        *x=21;*y=7;*label="TERMINAL";return 1;
    }
    if(s->rescue_stage==STORY_RESCUE_TRACE){
        *x=7;*y=2;*label="SAVE MARA";return 1;
    }
    if(s->rescue_stage==STORY_RESCUE_LOST){
        /* No rescue deadline remains, but the still-reachable cafe phone
           is the explicit center of Danny's two-day hope subplot. */
        *x=8;*y=11;*label="PHONE";return 1;
    }
    /* A medical appointment takes priority; there is deliberately no timer
       forcing a final rooftop interaction after the last visit. */
    if (s->life_stage==STORY_LIFE_CLINIC ||
        s->life_stage==STORY_LIFE_FINAL_CLINIC) {
        *x=16; *y=5; *label="CLINIC";
    } else if (s->life_stage==STORY_LIFE_LAST_WORK) {
        *x=14; *y=6; *label="CANVAS";
    } else if (s->life_stage==STORY_LIFE_WAIT ||
               s->life_stage==STORY_LIFE_FOLLOWUP_WAIT) {
        /* Suggestions, not mandatory objectives; days continue regardless. */
        if (!(s->daily_mask&STORY_DAY_MARA)) {
            *x=8; *y=11; *label="MARA";
        } else if (!(s->daily_mask&STORY_DAY_LETTER)) {
            *x=5; *y=5; *label="LETTER";
        } else if (!(s->daily_mask&STORY_DAY_CLERK)) {
            *x=11; *y=7; *label="BORE";
        } else if (!(s->daily_mask&STORY_DAY_ART)) {
            *x=14; *y=6; *label="PAINT";
        } else if (!(s->daily_mask&STORY_DAY_MUSIC)) {
            *x=19; *y=7; *label="MUSIC";
        } else {
            if(s->art_steps<STORY_PROJECT_DONE){
                *x=14; *y=6; *label="ART WORK";
            }else if(s->song_steps<STORY_PROJECT_DONE){
                *x=19; *y=7; *label="MIX TAPE";
            }else{
                *x=14; *y=6; *label="REST";
            }
        }
    } else if (!(s->flags&STORY_HOME)) {
        *x=5; *y=5; *label="LETTER";
    } else if (!(s->flags&STORY_STAMP)) {
        *x=11; *y=7; *label="STAMP";
    } else if (!(s->flags&STORY_MARA)) {
        *x=8; *y=11; *label="MARA";
    } else if (!(s->flags&STORY_MUSIC)) {
        *x=19; *y=7; *label="MUSIC";
    } else if (!(s->flags&STORY_ART)) {
        *x=14; *y=6; *label="CANVAS";
    } else {
        *x=19; *y=5; *label="LAST BUS";
    }
    return 1;
}
int story_solid(int x, int y) {
    const char c = story_tile(x, y);
    return c == '#' || c == 'A' || c == 'T' || c == '~' || c == 'L' || c == 'C' || c == 'F' || c == 'B' || c == 'P' || c == 'V' || c == 'Q' || c == 'S' || c == 'W' || c == 'H' || c == 'R' || c == 'M' || c == 'K';
}
/* A tiny cached pathfinder follows the ORIGINAL 24x18 walkable town.
   A direct compass bearing is useless when a building is in the way. The
   shortest route ends at a walkable interaction square beside a solid
   objective; it never tells Danny to walk INSIDE the counter or clinic.
   Fixed ~1.3 KiB static scratch, no heap, no framebuffer, no save data.
   Repeated HUD draws cost only a coordinate cache check; recalculate on
   movement or changed objective, never once per pixel or ray. */
int story_route(const Story *s, int *direction, int *steps){
    static struct {
        int sx,sy,gx,gy,dir,steps,valid,found;
    } cache;
    int gx=0,gy=0;const char *name=0;
    if(!story_goal(s,&gx,&gy,&name)){
        if(direction)*direction=-1;
        if(steps)*steps=-1;
        return 0;
    }
    int sx=(int)s->x,sy=(int)s->y;
    if(!cache.valid||cache.sx!=sx||cache.sy!=sy||
       cache.gx!=gx||cache.gy!=gy){
        cache.valid=1;cache.sx=sx;cache.sy=sy;
        cache.gx=gx;cache.gy=gy;cache.found=0;
        cache.dir=-1;cache.steps=-1;
        if(!story_solid(sx,sy)){
            int gxsolid=story_solid(gx,gy);
            int distance=(sx>gx?sx-gx:gx-sx)+
                         (sy>gy?sy-gy:gy-sy);
            if((gxsolid&&distance==1)||(!gxsolid&&distance==0)){
                cache.found=1;cache.steps=0;
            }else{
                static uint16_t queue[STORY_W*STORY_H];
                static uint8_t first[STORY_W*STORY_H];
                static const int8_t dx[4]={1,0,-1,0};
                static const int8_t dy[4]={0,1,0,-1};
                for(unsigned i=0;i<STORY_W*STORY_H;i++)first[i]=255u;
                unsigned read=0,write=0,count=1,depth=0;
                unsigned start=(unsigned)sy*STORY_W+(unsigned)sx;
                first[start]=4u;queue[write++]=(uint16_t)start;
                while(read<write && !cache.found){
                    unsigned next_count=0;
                    for(unsigned k=0;k<count&&!cache.found;k++){
                        unsigned cell=queue[read++];
                        int cx=(int)(cell%STORY_W),cy=(int)(cell/STORY_W);
                        for(unsigned d=0;d<4u;d++){
                            int nx=cx+dx[d],ny=cy+dy[d];
                            if((unsigned)nx>=STORY_W||
                               (unsigned)ny>=STORY_H||story_solid(nx,ny))
                                continue;
                            unsigned nid=(unsigned)ny*STORY_W+(unsigned)nx;
                            if(first[nid]!=255u)continue;
                            first[nid]=(uint8_t)(cell==start?d:first[cell]);
                            queue[write++]=(uint16_t)nid;
                            ++next_count;
                            int near=(nx>gx?nx-gx:gx-nx)+
                                     (ny>gy?ny-gy:gy-ny);
                            if((gxsolid&&near==1)||(!gxsolid&&near==0)){
                                cache.found=1;cache.dir=(int)first[nid];
                                cache.steps=(int)depth+1;
                                break;
                            }
                        }
                    }
                    count=next_count;++depth;
                }
            }
        }
    }
    if(direction)*direction=cache.dir;
    if(steps)*steps=cache.steps;
    return cache.found;
}
const char *story_goal_reason(const Story *s){
    if(s->ended||s->life_stage==STORY_LIFE_FINALE)
        return "YOUR STORY IS STILL HERE";
    switch(s->rescue_stage){
    case STORY_RESCUE_MISSING:return "ASK THE WITNESS FOR CLUES";
    case STORY_RESCUE_WITNESS:return "TRACE THE VEHICLE WITH CARE";
    case STORY_RESCUE_TRACE:return "HELP MARA AT THE SAFE HOUSE";
    case STORY_RESCUE_LOST:return "NO VERIFIED RECOVERY YET";
    default:break;
    }
    switch(s->life_stage){
    case STORY_LIFE_CLINIC:case STORY_LIFE_FINAL_CLINIC:
        return "THE DOCTOR IS WAITING";
    case STORY_LIFE_LAST_WORK:return "FINISH THE LAST PAINTING";
    case STORY_LIFE_WAIT:case STORY_LIFE_FOLLOWUP_WAIT:
        return "MAKE MEMORIES AT YOUR PACE";
    default:break;
    }
    if(!(s->flags&STORY_HOME))return "START WITH THE OLD LETTER";
    if(!(s->flags&STORY_STAMP))return "THE CLERK NEEDS YOUR LETTER";
    if(!(s->flags&STORY_MARA))return "SPEAK HONESTLY WITH MARA";
    if(!(s->flags&STORY_MUSIC))return "THE BORE ARCHIVE HAS A TAPE";
    if(!(s->flags&STORY_ART))return "PUT SOME COLOR ON THE ROOF";
    return "THE LAST BUS IS WAITING";
}

enum { D_NONE, D_OPEN, D_LETTER, D_LETTER_AGAIN, D_CLERK_LOCKED, D_CLERK, D_CLERK_AGAIN,
       D_MARA_LOCKED, D_MARA, D_MARA_AGAIN, D_BUS_LOCKED, D_BUS_READY, D_END_GO,
       D_END_STAY, D_JOURNAL_0, D_JOURNAL_1, D_JOURNAL_2, D_JOURNAL_3, D_NOTHING, D_RETURN,
       D_CULT_FIND, D_CULT_LOGS, D_CULT_WAIT, D_CULT_WITNESS, D_CULT_SAFETY,
       D_CULT_BACKUP, D_CULT_AFTER, D_CULT_NOT_YET, D_CULT_WITNESS_AFTER,
       D_CULT_SAFE_AGAIN, D_CULT_PRE_WIN, D_CULT_DURING_WIN, D_CULT_POST_WIN,
       D_CULT_RUN, D_CULT_LOSE, D_CULT_NOTES_1, D_CULT_NOTES_2,
       D_CULT_NOTES_3, D_CULT_NOTES_4,
       D_MUSIC, D_MUSIC_AGAIN, D_ART, D_ART_AGAIN, D_ART_DAY, D_ART_LAST_LOCKED,
       D_NEUTRAL_START, D_MEDICAL_NOTICE, D_DOCTOR_FIRST,
       D_DOCTOR_RETURN, D_DOCTOR_LAST, D_NEUTRAL_PAINT, D_NEUTRAL_END,
       D_GOOD_END, D_DAY_NOTES, D_CLINIC_LOCKED,
       D_NOTE_VISIT, D_NOTE_FOLLOWUP, D_NOTE_CANVAS,
       D_DAY_MARA, D_DAY_MARA_AFTER, D_DAY_LETTER, D_DAY_LETTER_AFTER,
       D_DAY_CLERK, D_DAY_CLERK_AFTER, D_DAY_MUSIC, D_DAY_MUSIC_AFTER,
       D_DAY_PAINT, D_DAY_PAINT_AFTER, D_DAY_RECAP, D_DAY_RECAP_AFTER,
       D_ACT_MARA_TALK, D_ACT_LETTER_KEEP, D_ACT_CLERK_JOKE,
       D_ACT_MUSIC_LISTEN, D_ACT_ART_REST,
       D_ACT_MARA_CLOSE, D_ACT_MUSIC_PROGRESS, D_ACT_ART_PROGRESS,
       D_PROJECT_SONG_2, D_PROJECT_SONG_3, D_PROJECT_SONG_DONE,
       D_PROJECT_ART_2, D_PROJECT_ART_3, D_PROJECT_ART_DONE,
       D_MARA_MORNING, D_MARA_CLOSING, D_CLERK_CLOSING,
       D_MARA_GALLERY, D_CLERK_SONG,
       D_BAD_LETTER, D_BAD_CLERK, D_BAD_MUSIC, D_BAD_SERVER,
       D_MARA_MISSING, D_MARA_ABSENT, D_RESCUE_WITNESS,
       D_RESCUE_TRACE, D_RESCUE_SAVED, D_RESCUE_LOST,
       D_RESCUE_AFTERSHOCK, D_BAD_END, D_BAD_END_SAVED, D_RESCUE_NOTES_1,
       D_RESCUE_NOTES_2, D_RESCUE_NOTES_3, D_RESCUE_NOTES_SAVED,
       D_BUS_RESCUE, D_BAD_WARNING_2, D_CLINIC_RESCUE,
       D_RESCUE_WITNESS_TRUST, D_RESCUE_SAVED_TRUST,
       D_RESCUE_SAVED_ART,
       D_HOPE_REALIZE, D_HOPE_PHONE_WAIT, D_HOPE_PHONE_LATE,
       D_HOPE_WITNESS, D_HOPE_TERMINAL, D_HOPE_DOOR,
       D_HOPE_NOTE_WAIT, D_HOPE_NOTE_LATE,
       D_MARA_CONCERN, D_MARA_WARNING, D_BAD_MUSIC_MASTER,
       D_GOOD_END_SCARS, D_RESCUE_LOST_REMOTE,
       D_SHIFT_EARLY, D_SHIFT_DONE, D_SHIFT_REPEAT,
       D_SHIFT_TIRED, D_SHIFT_PAUSED,
       D_DOCTOR_RETURN_ALONE,
       D_LOOK_POTHEADS, D_LOOK_CANAL, D_LOOK_MARKET,
       D_LOOK_NORTH, D_LOOK_STREET };
#define SCRIPT(name, ...) static const StoryPage name[] = { __VA_ARGS__ }
/* The banana-sign shift is practical income, not Danny's calling. He is
   grateful for the stability without pretending to enjoy mascot work. */
SCRIPT(d_open,
 {"DANNY", "Another afternoon", "in the banana suit."},
 {"DANNY", "I don't love this gig.", "But it pays the rent."},
 {"DANNY", "That counts for a lot.", "I'm grateful it does."},
 {"DANNY", "After hours, I get", "to make my own art."},
 {"DANNY", "Last bus tonight.", "Doctor called again."},
 {"DANNY", "I can face a letter", "before test results."},
 {"DANNY", "Then Mara. Music.", "One thing at a time."});
SCRIPT(d_letter,
 {"OLD LETTER", "I left the porch", "light on for you."},
 {"OLD LETTER", "The bill went up.", "I left it on anyway."},
 {"DANNY", "Somebody made room", "for me. I left."},
 {"DANNY", "I can at least", "write back."});
SCRIPT(d_letter_again,
 {"DANNY", "That little letter", "weighs a whole ton."});
SCRIPT(d_clerk_locked,
 {"BORE CLERK", "Form B-2 requires", "proof you existed."},
 {"BORE CLERK", "Try looking for", "a letter from home."});
SCRIPT(d_clerk,
 {"BORE CLERK", "We keep records", "when people forget."},
 {"DANNY", "You saved a blank", "file for me?"},
 {"BORE CLERK", "One existence,", "provisionally okay."},
 {"DANNY", "What about the old", "BORE music reels?"},
 {"BORE CLERK", "In the archive.", "Names still attached."});
SCRIPT(d_clerk_again,
 {"BORE CLERK", "Do not fold the", "universe in half."},
 {"BORE CLERK", "The warranty is", "very specific."});
SCRIPT(d_mara_locked,
 {"MARA", "You're leaving?", "You always do that."},
 {"MARA", "Talk to the clerk.", "Make it official."});
SCRIPT(d_mara,
 {"MARA", "You only joke like", "that when scared."},
 {"DANNY", "Then I'm a very", "funny man tonight."},
 {"MARA", "You can leave. Just", "don't vanish on me."},
 {"DANNY", "I'll tell you first.", "You have my word."},
 {"MARA", "And keep the song.", "The silly one."});
SCRIPT(d_mara_again,
 {"MARA", "Do you still have", "time for a walk?"},
 {"DANNY", "I do now.", "No rush today."});
/* One or two deliberately harmful choices affect Mara's trust *before*
   disappearance. Neither line presumes which site the player damaged. */
SCRIPT(d_mara_concern,
 {"MARA", "You've been talking", "around something."},
 {"DANNY", "I hurt someone's", "trust. I know."},
 {"MARA", "Tell me the truth.", "I can listen."},
 {"DANNY", "I have to start", "by not hiding."});
SCRIPT(d_mara_warning,
 {"MARA", "People are getting", "hurt by these edits."},
 {"DANNY", "I called them small", "until they weren't."},
 {"MARA", "Stop making others", "pay for your fear."},
 {"DANNY", "You're right.","No more shortcuts."});
SCRIPT(d_bus_locked,
 {"DRIVER", "Last bus tonight.", "Seats still open."},
 {"DRIVER", "Your ticket is fine.", "Your goodbyes aren't."});
SCRIPT(d_bus_ready,
 {"DRIVER", "Your stamp checks out.", "One seat. One choice."},
 {"DRIVER", "Leaving won't erase", "what happened here."},
 {"DANNY", "No. But I'll take", "the truth with me."});
SCRIPT(d_end_go,
 {"DANNY", "The city got small", "behind the window."},
 {"DANNY", "I didn't lose it.", "I brought it with me."},
 {"DRIVER", "Mind the gap in", "the space-time fare."},
 {"THE END", "Tomorrow is still", "a place to begin."});
SCRIPT(d_end_stay,
 {"DANNY", "I stayed. The bus", "left without a fuss."},
 {"MARA", "Good. The old cafe", "needs a new sign."},
 {"DANNY", "I call it THE END.", "We're open Tuesday."},
 {"THE END", "Tomorrow came on", "foot after all."});
SCRIPT(d_journal_0,
 {"POCKET NOTEBOOK", "First: the old letter.", "Then the BORE clerk."},
 {"DANNY", "One street at a time.", "I can take my time."});
SCRIPT(d_journal_1,
 {"POCKET NOTEBOOK", "The letter is safe.", "BORE needs proof."});
SCRIPT(d_journal_2,
 {"POCKET NOTEBOOK", "The clerk stamped it.", "Mara deserves the truth."});
SCRIPT(d_journal_3,
 {"POCKET NOTEBOOK", "I made the promise.", "One last bus to decide."});
SCRIPT(d_nothing,
 {"DANNY", "Nothing here for me.", "Where was I headed?"},
 {"POCKET NOTEBOOK", "Follow NEXT up top.", "R holds my notes."});
SCRIPT(d_look_potheads,
 {"DANNY", "Potheads. The suit", "still smells of rain."},
 {"DANNY", "I can be grateful", "and still want more."});
SCRIPT(d_look_canal,
 {"DANNY", "The canal keeps", "its own kind of time."},
 {"DANNY", "A hundred windows", "in every ripple."});
SCRIPT(d_look_market,
 {"DANNY", "Someone patched the", "awning by hand."},
 {"DANNY", "Not every repair", "has to look new."});
SCRIPT(d_look_north,
 {"DANNY", "All these lit rooms.", "Lives I never knew."},
 {"DANNY", "My song could reach", "one of them someday."});
SCRIPT(d_look_street,
 {"DANNY", "The old paving", "holds the rain."},
 {"DANNY", "I should keep going.", "One block at a time."});
SCRIPT(d_return,
 {"DANNY", "The city is still", "here. So am I."});
/* Entirely fictional setting and people. The harm is serious; the bureaucracy is not. */
SCRIPT(d_cult_find,
 {"TERMINAL", "MIRROR ORCHARD", "WELCOME / RULES: 900"},
 {"DANNY", "The art server has", "a ministry of fonts."},
 {"UNKNOWN USER", "The jokes hide harm.", "Please take care."},
 {"DANNY", "A real person asked", "for help. I can try."});
SCRIPT(d_cult_logs,
 {"TERMINAL", "Screenshots are not", "a rescue plan."},
 {"DANNY", "Find the witness.", "Ask what THEY need."});
SCRIPT(d_cult_wait,
 {"TERMINAL", "Talk to the witness", "before posting."});
SCRIPT(d_cult_witness,
 {"WITNESS", "I kept the logs.", "Please don't post me."},
 {"DANNY", "Your name stays out.", "What would help?"},
 {"WITNESS", "Help my friend get", "somewhere safe."},
 {"DANNY", "No spectacle. We", "listen and help."});
SCRIPT(d_cult_safety,
 {"NEIGHBOR", "The door is open.", "They chose to come."},
 {"NEIGHBOR", "Trusted people know.", "No cameras here."},
 {"DANNY", "One person is safe.", "That is not small."});
SCRIPT(d_cult_backup,
 {"TERMINAL", "Evidence preserved.", "Reports submitted."},
 {"TERMINAL", "Server reopened as", "MIRROR ORCHARD 2."},
 {"DANNY", "We didn't erase it.", "We helped someone."});
SCRIPT(d_cult_after,
 {"TERMINAL", "Server still active.", "New logo. Same mess."},
 {"DANNY", "They can't erase", "who got out."});
SCRIPT(d_cult_not_yet,
 {"WITNESS", "Please read the", "message first."});
SCRIPT(d_cult_witness_after,
 {"WITNESS", "Thank you for not", "making me a meme."});
SCRIPT(d_cult_safe_again,
 {"NEIGHBOR", "The kettle is on.", "No server rules."});
SCRIPT(d_cult_pre_win,
 {"DANNY", "The reply squad", "muted itself. Huh."},
 {"HECKLER", "YOU WON A POINT.", "NOT THE INTERNET."});
SCRIPT(d_cult_during_win,
 {"DANNY", "A loud argument", "isn't real help."},
 {"HECKLER", "My mod badge has", "emotional dents."});
SCRIPT(d_cult_post_win,
 {"HECKLER", "We changed names!", "YOU CAN'T WIN!"},
 {"DANNY", "I wasn't trying to", "win a website."},
 {"DANNY", "The people we helped", "still matter."});
SCRIPT(d_cult_run,
 {"DANNY", "I am logging off.", "This is a skill."});
SCRIPT(d_cult_lose,
 {"DANNY", "Too much noise.", "I'll try another way."},
 {"DANNY", "Nobody needs me", "to win this fight."});
SCRIPT(d_cult_notes_1,
 {"POCKET NOTEBOOK", "MIRROR ORCHARD:", "Find the witness."});
SCRIPT(d_cult_notes_2,
 {"POCKET NOTEBOOK", "Witness contacted.", "Find the safe house."});
SCRIPT(d_cult_notes_3,
 {"POCKET NOTEBOOK", "Someone got safe.", "Back up the truth."});
SCRIPT(d_cult_notes_4,
 {"POCKET NOTEBOOK", "People helped. Logs", "saved. Server lives."});
SCRIPT(d_music,
 {"ARCHIVE TAPE", "BORE REEL 07", "ROUGH TRACK INTACT."},
 {"DANNY", "My old song still", "sounds like me."},
 {"DANNY", "I'll leave a copy", "with my name on it."},
 {"ARCHIVE TAPE", "COPY REGISTERED.", "KEEP THE ORIGINAL."},
 {"ARCHIVE TAPE", "FOUR SESSIONS MAKE", "ONE FINISHED MASTER."});
SCRIPT(d_music_again,
 {"DANNY", "The tape survived.", "Not the cassette case."});
SCRIPT(d_art,
 {"DANNY", "The rooftop canvas", "still needs a sky."},
 {"DANNY", "Maybe it can hold", "a little sunlight."});
SCRIPT(d_art_again,
 {"DANNY", "The roof painting", "is almost finished."});
SCRIPT(d_art_day,
 {"DANNY", "One more layer of", "paint. One more day."},
 {"DANNY", "The city keeps on", "being a city."});
SCRIPT(d_art_last_locked,
 {"DANNY", "The doctors haven't", "finished talking."},
 {"DANNY", "My canvas can wait", "a little longer."});
SCRIPT(d_neutral_start,
 {"DAY ONE", "Danny missed the bus", "and stayed in town."},
 {"DANNY", "The clinic called.", "I kept not calling."},
 {"DANNY", "I can't stop time.", "I can spend it."},
 {"POCKET NOTEBOOK", "Familiar places", "offer two choices."},
 {"POCKET NOTEBOOK", "E energy S spark", "B bond W work."});
SCRIPT(d_medical_notice,
 {"DAY TWO", "The clinic left", "another message."},
 {"DANNY", "I still haven't", "returned the first."},
 {"CLINIC", "Please come in.", "We need to talk."});
SCRIPT(d_doctor_first,
 {"DOCTOR", "The tests show", "a serious illness."},
 {"DOCTOR", "We need more scans.", "I can't guess time."},
 {"DANNY", "I've been dodging", "these calls all week."},
 {"DOCTOR", "Bring someone you", "trust next time."});
SCRIPT(d_doctor_return,
 {"DAY THREE", "Your follow-up", "is scheduled today."},
 {"MARA", "I'll come in.", "Or wait outside."},
 {"DANNY", "Come in. I don't", "want to hear alone."});
/* When the search has ended without verified recovery, the doctor still
   needs Danny, but Mara cannot promise to accompany him. Her condition
   is deliberately UNKNOWN, not assumed fatal. */
SCRIPT(d_doctor_return_alone,
 {"DAY THREE", "The follow-up", "is still today."},
 {"DANNY", "No confirmed word", "about Mara. Not yet."},
 {"DANNY", "I'll see the doctor", "and keep looking."});
SCRIPT(d_doctor_last,
 {"DOCTOR", "The cancer has", "spread. I'm sorry."},
 {"DOCTOR", "We can help you", "through the pain."},
 {"MARA", "Want me to stay?", "I have nowhere else."},
 {"DANNY", "For a while. Then", "I want the roof."});
SCRIPT(d_neutral_paint,
 {"ROOFTOP", "Danny returned to", "the unfinished sky."},
 {"DANNY", "The clinic didn't", "promise tomorrow."},
 {"DANNY", "I can give this", "corner some light."});
SCRIPT(d_neutral_end,
 {"NARRATOR", "The last brush fell", "on the quiet roof."},
 {"NARRATOR", "Danny never stood", "up again."},
 {"MARA", "I found his letter", "beside the paints."},
 {"NEUTRAL END", "He left us a sky", "we could recognize."});
SCRIPT(d_good_end,
 {"YEARS LATER", "The bus was only", "the beginning."},
 {"MARA", "He lived beyond", "that last night."},
 {"BORE CLERK", "We kept the demos.", "He asked us to."},
 {"RECORD STORE", "Mara released them", "after he was gone."},
 {"RECORD STORE", "The songs reached", "millions of people."},
 {"GOOD END", "He never heard", "their final chorus."});
/* A positive artistic legacy cannot erase harms from optional bad checks.
   The title of the ending remains unchanged; its moral context does not. */
SCRIPT(d_good_end_scars,
 {"YEARS LATER", "Danny lived beyond", "that last bus ride."},
 {"MARA", "I kept the tapes.", "Not his excuses."},
 {"BORE CLERK", "The archive kept", "the original copies."},
 {"RECORD STORE", "After he was gone,", "the songs took off."},
 {"MARA", "He made real music.", "He did harm too."},
 {"GOOD END", "One did not erase", "the other."});
SCRIPT(d_day_notes,
 {"POCKET NOTEBOOK", "The bus can wait.", "The roof cannot."},
 {"POCKET NOTEBOOK", "Read, walk, paint.", "The day continues."},
 {"POCKET NOTEBOOK", "At familiar places", "A choose  B cancel."},
 {"POCKET NOTEBOOK", "E energy  S spark", "B bond  W work."});
SCRIPT(d_clinic_locked,
 {"CLINIC", "OPEN BY APPOINTMENT", "NEXT NOTICE PENDING."});
SCRIPT(d_note_visit,
 {"POCKET NOTEBOOK", "CLINIC ON EAST ST.", "They asked for me."},
 {"DANNY", "I should see them", "before I paint."});
SCRIPT(d_note_followup,
 {"POCKET NOTEBOOK", "WAIT FOR THE NEXT", "DOCTOR APPOINTMENT."},
 {"DANNY", "Mara offered to", "walk there with me."});
SCRIPT(d_note_canvas,
 {"POCKET NOTEBOOK", "LAST ROOFTOP CANVAS", "NORTH PLAZA STAIRS."},
 {"DANNY", "The last detail is", "a corner of sky."});

/* Small, world-grounded scenes, staged in the existing cinematic sets. */
SCRIPT(d_day_mara,
 {"MARA", "Want the long way", "past the old shops?"},
 {"DANNY", "My legs complain.", "My mouth says yes."},
 {"MARA", "Then we can walk", "as slowly as we like."});
SCRIPT(d_day_mara_after,
 {"MARA", "I can walk with", "you for a while."},
 {"DANNY", "Let's pass the shop", "with the bad sign."},
 {"MARA", "Still spelled wrong.", "Some things endure."});
SCRIPT(d_day_letter,
 {"DANNY", "The letter smells", "of old cupboards."},
 {"DANNY", "I owe a reply.", "Ink should be enough."},
 {"OLD LETTER", "A small note placed", "behind the frame."});
SCRIPT(d_day_letter_after,
 {"DANNY", "There was a lamp", "in that old house."},
 {"DANNY", "Maybe my painting", "can hold its light."},
 {"OLD LETTER", "He folded the page", "and kept it near."});
SCRIPT(d_day_clerk,
 {"BORE CLERK", "Form W: proof of", "an ordinary weekday."},
 {"DANNY", "I had breakfast.", "Does that count?"},
 {"BORE CLERK", "Only if you kept", "the receipt."});
SCRIPT(d_day_clerk_after,
 {"BORE CLERK", "Today's stamp is", "not a deadline."},
 {"DANNY", "That's the nicest", "thing you've said."},
 {"BORE CLERK", "Please don't file", "a compliment."});
SCRIPT(d_day_music,
 {"ARCHIVE TAPE", "A melody from", "the old BORE games."},
 {"DANNY", "This chord should", "be one step lower."},
 {"DANNY", "There. Now it's", "awkward on purpose."});
SCRIPT(d_day_music_after,
 {"ARCHIVE TAPE", "A second take.", "The reel turns."},
 {"DANNY", "Leave the quiet", "part of the song."},
 {"DANNY", "A pause can say", "a whole lot."});
SCRIPT(d_day_paint,
 {"ROOFTOP", "The paint catches", "the afternoon sun."},
 {"DANNY", "Another thin line", "behind the skyline."},
 {"DANNY", "It's coming along.", "So am I, maybe."});
SCRIPT(d_day_paint_after,
 {"ROOFTOP", "The city changes", "while he paints."},
 {"DANNY", "Leave some space", "for tomorrow's sky."},
 {"DANNY", "No need to fill", "every corner yet."});
SCRIPT(d_day_recap,
 {"AN ORDINARY DAY", "A walk, a letter", "and a crooked tune."},
 {"DANNY", "I didn't solve", "the whole world."},
 {"DANNY", "I did live in", "a little of it."});
SCRIPT(d_day_recap_after,
 {"ANOTHER DAY", "The music played.", "The sky moved on."},
 {"MARA", "Tomorrow isn't", "a receipt to keep."},
 {"DANNY", "No. Just another", "morning, if it comes."});
/* The right-hand daily options restore energy rather than punishing fatigue. */
SCRIPT(d_act_mara_talk,
 {"MARA", "You do not have to", "be funny today."},
 {"DANNY", "Well, that saves", "my worst material."},
 {"MARA", "We can just sit", "and talk awhile."});
SCRIPT(d_act_letter_keep,
 {"DANNY", "Some memories are", "not letters to send."},
 {"OLD LETTER", "He kept the page", "where it could dry."},
 {"DANNY", "I still know the", "shape of that light."});
SCRIPT(d_act_clerk_joke,
 {"DANNY", "Is this the line", "for existing loudly?"},
 {"BORE CLERK", "Wrong queue. That", "one has a bell."},
 {"DANNY", "You laughed. I'll", "file that under joy."});
SCRIPT(d_act_music_listen,
 {"ARCHIVE TAPE", "An old BORE theme", "plays back softly."},
 {"DANNY", "The rest between", "notes is part of it."},
 {"DANNY", "Maybe I can just", "listen this time."});
SCRIPT(d_act_art_rest,
 {"ROOFTOP", "Danny sets the", "brush beside him."},
 {"DANNY", "I could paint all", "the hours away."},
 {"DANNY", "Or watch the sky", "paint itself awhile."});
SCRIPT(d_act_mara_close,
 {"MARA", "You never have to", "perform for me."},
 {"DANNY", "Then let's watch", "the shops close."},
 {"MARA", "We can take our", "time getting home."});
SCRIPT(d_act_music_progress,
 {"ARCHIVE TAPE", "The new arrangement", "has taken shape."},
 {"DANNY", "That off-beat line", "is finally mine."},
 {"DANNY", "Keep a copy in", "the BORE archive."});
SCRIPT(d_act_art_progress,
 {"ROOFTOP", "The old skyline", "has more color now."},
 {"DANNY", "It's not perfect.", "It feels like mine."});

/* Projects continue between numbered-day interactions. Every milestone is
   a playable scene tied to the same physical workplace and BORE music. */
SCRIPT(d_project_song_2,
 {"ARCHIVE TAPE", "The bass finds the", "old crooked melody."},
 {"DANNY", "Whistler Man comes", "back in the gaps."});
SCRIPT(d_project_song_3,
 {"DANNY", "The rhythm is", "finally breathing."},
 {"ARCHIVE TAPE", "One final mix.", "Then it is ready."});
SCRIPT(d_project_song_done,
 {"ARCHIVE TAPE", "MASTER COPY SAVED.", "THE LAST BORE REEL."},
 {"DANNY", "The quiet parts", "made it in too."},
 {"DANNY", "Let someone else", "hear the whole song."},
 {"BORE ARCHIVE", "ADVANCED UNLOCKED.", "A AT THE MUSIC DESK."},
 {"BORE ARCHIVE", "LISTEN IS EASY.", "EDIT WHEN YOU WANT."});
SCRIPT(d_project_art_2,
 {"ROOFTOP", "Buildings rise over", "the painted horizon."},
 {"DANNY", "I know those", "windows by heart."});
SCRIPT(d_project_art_3,
 {"ROOFTOP", "A thin brush adds", "the street lights."},
 {"DANNY", "Almost the same", "as the view outside."});
SCRIPT(d_project_art_done,
 {"ROOFTOP", "THE OLD QUARTER", "FINISHED ON CANVAS."},
 {"DANNY", "I left one corner", "for the real sky."},
 {"DANNY", "That feels about", "right for a painting."});
/* Shops follow morning / evening rhythms while remaining reachable:
   schedules never hide story NPCs or disable their interactions. */
SCRIPT(d_mara_morning,
 {"MARA", "The cafe's opening.", "The kettle is loud."},
 {"DANNY", "So am I before", "coffee, allegedly."});
SCRIPT(d_mara_closing,
 {"MARA", "Evening's here.", "We can take a walk."},
 {"DANNY", "The city lights", "look less tired."});
SCRIPT(d_clerk_closing,
 {"BORE CLERK", "Five minutes until", "the stamps go home."},
 {"DANNY", "Does your stamp", "get paid overtime?"});
SCRIPT(d_mara_gallery,
 {"MARA", "You finished the", "rooftop picture!"},
 {"DANNY", "Just in time for", "the light to change."},
 {"MARA", "Let me see it", "without the frame."});
SCRIPT(d_clerk_song,
 {"BORE CLERK", "This recording", "is officially BORE."},
 {"DANNY", "I finally made", "a usable master."},
 {"BORE CLERK", "Filed under", "SONGS WORTH KEEPING."});

/* Four optional harmful decisions require deliberate B + RIGHT + A.
   Mara's fate is never shown or described explicitly if rescue fails. */
SCRIPT(d_bad_letter,
 {"DANNY", "I hid the reply", "when it mattered."},
 {"DANNY", "Nobody deserves", "to wait for a lie."});
SCRIPT(d_bad_clerk,
 {"BORE CLERK", "That forged record", "has a real owner."},
 {"DANNY", "I thought it was", "only paperwork."});
SCRIPT(d_bad_music,
 {"ARCHIVE TAPE", "EARLY DEMO ERASED.", "SESSION CREDITS LOST."},
 {"DANNY", "Other people made", "that track with me."});
/* Mastering the current recording does not retroactively recreate an
   erased *early* demo; a master and lost session are distinct artifacts. */
SCRIPT(d_bad_music_master,
 {"ARCHIVE TAPE", "EARLY TAKE ERASED.", "MASTER STILL SAFE."},
 {"DANNY", "A final mix can't", "replace that take."});
SCRIPT(d_bad_server,
 {"TERMINAL", "PRIVATE NAME POSTED.", "RETRACTING IS HARD."},
 {"DANNY", "I just made life", "harder for someone."});
SCRIPT(d_clinic_rescue,
 {"RECEPTION", "Your appointment", "can be rescheduled."},
 {"DANNY", "Mara is missing.", "I have to find her."},
 {"RECEPTION", "We'll keep your", "place for you."});
SCRIPT(d_bad_warning_2,
 {"DANNY", "Two small wrongs.", "They didn't stay small."},
 {"UNKNOWN TEXT", "MARA'S CAFE IS", "ON THEIR LIST."},
 {"POCKET NOTEBOOK", "TWO BAD CHECKS.", "STOP BEFORE THREE."});
SCRIPT(d_mara_missing,
 {"MARA'S CAFE", "Her chair is empty.", "The cup is untouched."},
 {"DANNY", "She would have left", "some kind of note."},
 {"PHONE MESSAGE", "A WITNESS SAW A VAN.", "NEAR MIRROR ORCHARD."},
 {"DANNY", "A lead, not proof.", "I'll ask in person."});
SCRIPT(d_mara_absent,
 {"MARA'S CAFE", "The kettle has", "gone cold again."},
 {"DANNY", "I'll check the trail", "before I guess."});
SCRIPT(d_rescue_witness,
 {"WITNESS", "I saw a van.", "Someone forced her in."},
 {"WITNESS", "I lost the plate.", "Try the terminal."},
 {"DANNY", "I won't put your", "name online."});
SCRIPT(d_rescue_trace,
 {"TERMINAL", "LAST VAN SIGNAL:", "OLD SAFE HOUSE."},
 {"TERMINAL", "VEHICLE LOCATION.", "NOT PROOF OF MARA."},
 {"DANNY", "A lead, not proof.", "I'm going there."});
SCRIPT(d_rescue_saved,
 {"MARA", "You found me.", "I heard your steps."},
 {"DANNY", "I thought I'd be", "too late."},
 {"MARA", "Don't make a speech.", "Just help me stand."},
 {"DANNY", "I'm sorry.", "Let's get you home."});
SCRIPT(d_rescue_lost,
 {"SAFE HOUSE", "The light went out.", "No answer came."},
 {"UNKNOWN CALLER", "She will call you.", "Just give it a day."},
 {"DANNY", "You sound so sure.", "I need you right."},
 {"DANNY", "I'll keep the phone", "on. She can call."});
/* Fourth harm can occur from any desk, before Danny ever reaches the
   house. It must NOT teleport him to a safe-house failure scene. */
/* Shop scenes reuse the real Potheads set; these words are about making
   ends meet, never a claim Danny loves being a sandwich-board mascot. */
SCRIPT(d_shift_early,
 {"DANNY", "Already did my", "hours in the suit."},
 {"DANNY", "I can be grateful", "and still be tired."});
SCRIPT(d_shift_done,
 {"DANNY", "One more shift done.", "Rent will get paid."},
 {"DANNY", "Now I can get back", "to making something."});
SCRIPT(d_shift_repeat,
 {"DANNY", "I put my hours in.", "That\'s plenty today."});
SCRIPT(d_shift_tired,
 {"DANNY", "Too worn down for", "another sign shift."});
SCRIPT(d_shift_paused,
 {"DANNY", "The sign can wait.", "There are bigger things."});
SCRIPT(d_rescue_lost_remote,
 {"PHONE", "A lead went quiet.", "Mara is still gone."},
 {"UNKNOWN CALLER", "She will call you.", "Just give it a day."},
 {"DANNY", "You sound so sure.", "I need you right."},
 {"DANNY", "I'll keep the phone", "on. She can call."});
/* The unverified caller sounds reassuring, but the game does not portray
   an unreturned call as evidence of death or confirmed harm. */
SCRIPT(d_hope_realize,
 {"PHONE", "A day has passed.", "No return call."},
 {"UNKNOWN CALLER", "I passed on a rumor.", "I never checked."},
 {"WITNESS", "Mara still has not", "been recovered."},
 {"DANNY", "You sounded certain.", "I wanted you right."},
 {"DANNY", "The trail ends here.", "Not her story."},
 {"DANNY", "I'll keep the line", "open. Just in case."});
SCRIPT(d_hope_phone_wait,
 {"PHONE", "CALL STILL PENDING.", "No reply yet."},
 {"DANNY", "They sounded sure.", "I needed that."});
SCRIPT(d_hope_phone_late,
 {"PHONE", "NO VERIFIED UPDATE.", "Calls unanswered."},
 {"DANNY", "I know a promise", "isn't proof."},
 {"DANNY", "But keep ringing.", "I'll be here."});
SCRIPT(d_hope_witness,
 {"WITNESS", "They promised news.", "I can't confirm it."},
 {"WITNESS", "No signed recovery", "report exists."});
SCRIPT(d_hope_terminal,
 {"TERMINAL", "LOCATION UNKNOWN.", "NO VERIFIED TRACE."},
 {"DANNY", "The last van ping", "wasn't Mara's voice."});
SCRIPT(d_hope_door,
 {"SAFE HOUSE", "The door is shut.", "The light is off."},
 {"DANNY", "I still have no", "answer. Not yet."});
SCRIPT(d_hope_note_wait,
 {"POCKET NOTEBOOK", "AWAIT MARA'S CALL.", "PROMISE UNVERIFIED."},
 {"DANNY", "Keep the phone on.", "No guessing."});
SCRIPT(d_hope_note_late,
 {"POCKET NOTEBOOK", "MARA NOT RECOVERED.", "NO VERIFIED UPDATE."},
 {"DANNY", "Hope isn't proof.", "Neither is silence."});
SCRIPT(d_rescue_aftershock,
 {"DANNY", "I crossed that line", "again, on purpose."},
 {"MARA", "I'm home. I'm safe.", "It isn't all right."},
 {"DANNY", "You were right", "to expect better."});
SCRIPT(d_bad_end,
 {"NARRATOR", "There was no", "certain word of Mara."},
 {"DANNY", "I waited because", "someone said she'd call."},
 {"NARRATOR", "No answer came.", "That settled nothing."},
 {"BAD END", "The line stayed open", "past the last frame."});
SCRIPT(d_bad_end_saved,
 {"NARRATOR", "Mara returned home.", "She was safe."},
 {"MARA", "I was found. That", "didn't fix everything."},
 {"DANNY", "I asked for trust", "and spent it again."},
 {"BAD END", "Not every second", "chance gets a third."});
SCRIPT(d_rescue_notes_1,
 {"POCKET NOTEBOOK", "MARA IS MISSING.", "WITNESS: WEST CAFE."});
SCRIPT(d_rescue_notes_2,
 {"POCKET NOTEBOOK", "TERMINAL: EAST", "MIRROR ORCHARD."});
SCRIPT(d_rescue_notes_3,
 {"POCKET NOTEBOOK", "SAFE HOUSE: NORTH", "OPEN THE DOOR."});
SCRIPT(d_rescue_notes_saved,
 {"POCKET NOTEBOOK", "MARA IS SAFE.", "BAD CHECKS REMAIN."},
 {"DANNY", "One more bad choice", "could end it."});
SCRIPT(d_bus_rescue,
 {"DRIVER", "Back for a ticket?", "There is still time."},
 {"DANNY", "Mara made it home.", "That mattered more."},
 {"DRIVER", "So did what you did", "before she vanished."});
/* Good marks alter the rescue itself: deeds change what NPCs know and say.
   These are pure consequences of existing progress, never grind gates. */
SCRIPT(d_rescue_witness_trust,
 {"WITNESS", "You helped us with", "Mirror Orchard."},
 {"WITNESS", "I trust you with", "Mara's trail now."},
 {"WITNESS", "The terminal has", "the last known stop."});
SCRIPT(d_rescue_saved_trust,
 {"MARA", "You kept looking.", "I heard your steps."},
 {"DANNY", "I should've listened", "before all this."},
 {"MARA", "We can talk later.", "Please take me home."});
SCRIPT(d_rescue_saved_art,
 {"MARA", "You found me.", "I want to go home."},
 {"DANNY", "The roof painting", "is waiting for you."},
 {"MARA", "I'd like to see it", "when we're ready."});
#undef SCRIPT
#define SEQ(name) {name, (uint8_t)(sizeof(name) / sizeof((name)[0]))}
static const struct { const StoryPage *pages; uint8_t count; } scripts[] = {
 {0,0}, SEQ(d_open), SEQ(d_letter), SEQ(d_letter_again), SEQ(d_clerk_locked),
 SEQ(d_clerk), SEQ(d_clerk_again), SEQ(d_mara_locked), SEQ(d_mara), SEQ(d_mara_again),
 SEQ(d_bus_locked), SEQ(d_bus_ready), SEQ(d_end_go), SEQ(d_end_stay),
 SEQ(d_journal_0), SEQ(d_journal_1), SEQ(d_journal_2), SEQ(d_journal_3),
 SEQ(d_nothing), SEQ(d_return), SEQ(d_cult_find), SEQ(d_cult_logs),
 SEQ(d_cult_wait), SEQ(d_cult_witness), SEQ(d_cult_safety), SEQ(d_cult_backup),
 SEQ(d_cult_after), SEQ(d_cult_not_yet), SEQ(d_cult_witness_after),
 SEQ(d_cult_safe_again), SEQ(d_cult_pre_win), SEQ(d_cult_during_win),
 SEQ(d_cult_post_win), SEQ(d_cult_run), SEQ(d_cult_lose),
 SEQ(d_cult_notes_1), SEQ(d_cult_notes_2), SEQ(d_cult_notes_3),
 SEQ(d_cult_notes_4),
 SEQ(d_music), SEQ(d_music_again), SEQ(d_art), SEQ(d_art_again),
 SEQ(d_art_day), SEQ(d_art_last_locked), SEQ(d_neutral_start),
 SEQ(d_medical_notice), SEQ(d_doctor_first), SEQ(d_doctor_return),
 SEQ(d_doctor_last), SEQ(d_neutral_paint), SEQ(d_neutral_end),
 SEQ(d_good_end), SEQ(d_day_notes), SEQ(d_clinic_locked),
 SEQ(d_note_visit), SEQ(d_note_followup), SEQ(d_note_canvas),
 SEQ(d_day_mara), SEQ(d_day_mara_after), SEQ(d_day_letter), SEQ(d_day_letter_after),
 SEQ(d_day_clerk), SEQ(d_day_clerk_after), SEQ(d_day_music), SEQ(d_day_music_after),
 SEQ(d_day_paint), SEQ(d_day_paint_after), SEQ(d_day_recap), SEQ(d_day_recap_after),
  SEQ(d_act_mara_talk), SEQ(d_act_letter_keep), SEQ(d_act_clerk_joke),
  SEQ(d_act_music_listen), SEQ(d_act_art_rest),
  SEQ(d_act_mara_close), SEQ(d_act_music_progress), SEQ(d_act_art_progress),
  SEQ(d_project_song_2), SEQ(d_project_song_3), SEQ(d_project_song_done),
  SEQ(d_project_art_2), SEQ(d_project_art_3), SEQ(d_project_art_done),
  SEQ(d_mara_morning), SEQ(d_mara_closing), SEQ(d_clerk_closing),
  SEQ(d_mara_gallery), SEQ(d_clerk_song),
  SEQ(d_bad_letter), SEQ(d_bad_clerk), SEQ(d_bad_music), SEQ(d_bad_server),
  SEQ(d_mara_missing), SEQ(d_mara_absent), SEQ(d_rescue_witness),
  SEQ(d_rescue_trace), SEQ(d_rescue_saved), SEQ(d_rescue_lost),
  SEQ(d_rescue_aftershock), SEQ(d_bad_end), SEQ(d_bad_end_saved),
  SEQ(d_rescue_notes_1),
  SEQ(d_rescue_notes_2), SEQ(d_rescue_notes_3), SEQ(d_rescue_notes_saved),
  SEQ(d_bus_rescue), SEQ(d_bad_warning_2), SEQ(d_clinic_rescue),
  SEQ(d_rescue_witness_trust), SEQ(d_rescue_saved_trust),
  SEQ(d_rescue_saved_art),
  SEQ(d_hope_realize), SEQ(d_hope_phone_wait), SEQ(d_hope_phone_late),
  SEQ(d_hope_witness), SEQ(d_hope_terminal), SEQ(d_hope_door),
  SEQ(d_hope_note_wait), SEQ(d_hope_note_late),
  SEQ(d_mara_concern), SEQ(d_mara_warning), SEQ(d_bad_music_master),
  SEQ(d_good_end_scars), SEQ(d_rescue_lost_remote),
  SEQ(d_shift_early), SEQ(d_shift_done), SEQ(d_shift_repeat),
  SEQ(d_shift_tired), SEQ(d_shift_paused),
  SEQ(d_doctor_return_alone),
  SEQ(d_look_potheads), SEQ(d_look_canal), SEQ(d_look_market),
  SEQ(d_look_north), SEQ(d_look_street)
};
const StoryPage *story_pages(const Story *s, int *count) {
    unsigned id = s->dialogue;
    if (id >= sizeof(scripts) / sizeof(scripts[0])) id = 0;
    *count = scripts[id].count;
    return scripts[id].pages;
}
/* Each major interaction receives a world-grounded, animated shot sequence.
   The same script pages act as camera beats; no replay cutscene flags needed. */
/* Deterministic edit points on selected authored conversations. These are
   camera cues, not unskippable cutscene delays: B still dismisses, and
   the VBlank clock never runs backwards. Longer live reads continue at
   their naturally advanced frame without replaying a shot. */
static uint16_t dialogue_shot_beat(unsigned script,unsigned page){
 static const uint16_t opening[]={0u,90u,180u,450u,780u,990u,1188u};
 static const uint16_t letter[]={0u,55u,108u,160u};
 static const uint16_t mara[]={0u,66u,108u,151u,190u};
 static const uint16_t bus[]={0u,118u,183u};
 static const uint16_t canvas[]={0u,198u};
 static const uint16_t doctor[]={0u,124u,158u,200u};
 static const uint16_t followup[]={0u,98u,183u};
 static const uint16_t reunion4[]={0u,100u,170u,250u};
 static const uint16_t reunion3[]={0u,128u,250u};
 const uint16_t *beats=0;unsigned length=0;
 switch(script){
  case D_OPEN:beats=opening;length=7u;break;
  case D_LETTER:beats=letter;length=4u;break;
  case D_MARA:beats=mara;length=5u;break;
  case D_BUS_READY:beats=bus;length=3u;break;
  case D_ART:beats=canvas;length=2u;break;
  case D_DOCTOR_FIRST:case D_DOCTOR_LAST:
   beats=doctor;length=4u;break;
  case D_DOCTOR_RETURN:case D_DOCTOR_RETURN_ALONE:
   beats=followup;length=3u;break;
  case D_RESCUE_SAVED:beats=reunion4;length=4u;break;
  case D_RESCUE_SAVED_TRUST:case D_RESCUE_SAVED_ART:
   beats=reunion3;length=3u;break;
  default:break;
 }
 return beats&&page<length?beats[page]:0u;
}
static uint8_t cinematics_for_dialogue(uint8_t d){
    switch(d){
    case D_OPEN: case D_SHIFT_EARLY: case D_SHIFT_DONE:
    case D_SHIFT_REPEAT: case D_SHIFT_TIRED: return CINE_OPEN;
    case D_SHIFT_PAUSED: return CINE_NOTICE;
    case D_LETTER: case D_LETTER_AGAIN: return CINE_LETTER;
    case D_CLERK_LOCKED: case D_CLERK: case D_CLERK_AGAIN: return CINE_BORE;
    case D_MARA_LOCKED: case D_MARA: case D_MARA_AGAIN:
    case D_MARA_CONCERN: case D_MARA_WARNING: return CINE_MARA;
    case D_BUS_LOCKED: case D_BUS_READY: case D_END_GO: return CINE_BUS;
    case D_END_STAY: return CINE_STAY;
    case D_MUSIC: case D_MUSIC_AGAIN: return CINE_MUSIC;
    case D_ART: case D_ART_AGAIN: case D_ART_DAY: case D_ART_LAST_LOCKED:
       return CINE_ART;
    case D_CULT_FIND: case D_CULT_LOGS: case D_CULT_WAIT: return CINE_SERVER;
    case D_CULT_WITNESS: case D_CULT_NOT_YET: case D_CULT_WITNESS_AFTER:
       return CINE_WITNESS;
    case D_CULT_SAFETY: case D_CULT_SAFE_AGAIN: return CINE_SAFE;
    case D_CULT_BACKUP: return CINE_BACKUP;
    case D_CULT_AFTER: case D_CULT_POST_WIN: return CINE_REBRAND;
    case D_CULT_PRE_WIN: case D_CULT_DURING_WIN:
    case D_CULT_RUN: case D_CULT_LOSE: return CINE_SERVER;
    case D_MEDICAL_NOTICE: return CINE_NOTICE;
    case D_DOCTOR_FIRST: return CINE_CLINIC;
    case D_DOCTOR_RETURN: case D_DOCTOR_RETURN_ALONE:
       return CINE_FOLLOWUP;
    case D_DOCTOR_LAST: return CINE_LAST_CLINIC;
    case D_GOOD_END: case D_GOOD_END_SCARS: return CINE_GOOD;
    case D_NEUTRAL_START: return CINE_DAY;
    case D_NEUTRAL_PAINT: case D_NEUTRAL_END: return CINE_FINALE;
    case D_DAY_MARA: case D_DAY_MARA_AFTER: return CINE_MARA;
    case D_DAY_LETTER: case D_DAY_LETTER_AFTER: return CINE_LETTER;
    case D_DAY_CLERK: case D_DAY_CLERK_AFTER: return CINE_BORE;
    case D_DAY_MUSIC: case D_DAY_MUSIC_AFTER: return CINE_MUSIC;
    case D_DAY_PAINT: case D_DAY_PAINT_AFTER: return CINE_ART;
    case D_DAY_RECAP: case D_DAY_RECAP_AFTER: return CINE_DAY;
    case D_ACT_MARA_TALK: return CINE_MARA;
    case D_ACT_LETTER_KEEP: return CINE_LETTER;
    case D_ACT_CLERK_JOKE: return CINE_BORE;
    case D_ACT_MUSIC_LISTEN: return CINE_MUSIC;
    case D_ACT_ART_REST: case D_ACT_ART_PROGRESS: return CINE_ART;
    case D_ACT_MARA_CLOSE: return CINE_MARA;
    case D_ACT_MUSIC_PROGRESS: return CINE_MUSIC;
    case D_PROJECT_SONG_2: case D_PROJECT_SONG_3:
    case D_PROJECT_SONG_DONE: case D_CLERK_SONG: return CINE_MUSIC;
    case D_PROJECT_ART_2: case D_PROJECT_ART_3:
    case D_PROJECT_ART_DONE: return CINE_ART;
    case D_MARA_MORNING: case D_MARA_CLOSING:
    case D_MARA_GALLERY: return CINE_MARA;
    case D_CLERK_CLOSING: return CINE_BORE;
    case D_BAD_LETTER: return CINE_LETTER;
    case D_BAD_CLERK: return CINE_BORE;
    case D_BAD_MUSIC: case D_BAD_MUSIC_MASTER: return CINE_MUSIC;
    case D_BAD_SERVER: case D_RESCUE_TRACE: case D_RESCUE_AFTERSHOCK:
    case D_HOPE_TERMINAL: return CINE_SERVER;
    case D_MARA_MISSING: case D_MARA_ABSENT:
    case D_HOPE_PHONE_WAIT: case D_HOPE_PHONE_LATE:
        return CINE_MARA_EMPTY;
    case D_RESCUE_WITNESS: case D_RESCUE_WITNESS_TRUST:
    case D_HOPE_WITNESS: return CINE_WITNESS;
    case D_RESCUE_LOST: case D_HOPE_DOOR: return CINE_RESCUE_DOOR;
    case D_RESCUE_SAVED: case D_RESCUE_SAVED_TRUST:
    case D_RESCUE_SAVED_ART: return CINE_RESCUE_SAVED;
    case D_BAD_END: case D_BAD_END_SAVED: return CINE_BAD;
    case D_BUS_RESCUE: return CINE_BUS;
    case D_BAD_WARNING_2: case D_HOPE_REALIZE:
    case D_RESCUE_LOST_REMOTE: return CINE_NOTICE;
    case D_CLINIC_RESCUE: return CINE_CLINIC;
    default: return CINE_NONE;
    }
}
static void talk(Story *s, uint8_t id) {
    s->dialogue=id;s->page=0;
    s->cinema=cinematics_for_dialogue(id);s->cinema_frame=0;
}
void story_new(Story *s, int slot) {
    *s = (Story){0};
    studio_reset(); /* a new game starts with a clean optional tune */
    /* The Potheads opening now lands at its ACTUAL storefront; the first
       controllable frame starts beside the same facade seen in the film.
       Only a fresh game's spawn changes; old saves preserve coordinates. */
    s->x = 20; s->y = 11; s->prev_x = s->x; s->prev_y = s->y;
    s->facing = 0; s->slot = (uint8_t)(slot & 1);
    s->day_number = 1;
    s->energy=3;s->spark=1;s->bond=1;
    talk(s, D_OPEN); s->dirty = 1;
}
void story_resume(Story *s, int slot, int x, int y, int facing, int view, int flags, int ended, int ending) {
    story_new(s, slot);
    if (!story_solid(x,y)) { s->x = (uint8_t)x; s->y = (uint8_t)y; }
    s->prev_x = s->x; s->prev_y = s->y;
    s->facing = (uint8_t)(facing & 3); s->view = (uint8_t)(view & 1);
    s->flags = (uint8_t)(flags & STORY_FULL);
    s->ended = (uint8_t)(!!ended); s->ending = (uint8_t)(ending <= STORY_END_BAD ? ending : 0);
    s->dirty = 0;
    if(s->ended && (s->ending==STORY_END_GOOD || s->ending==STORY_END_BAD)){
        /* Replay the cinematic in sync with the one-shot score after loading. */
        s->finale_frame=0;s->dialogue=0;s->page=0;
        s->cinema=s->ending==STORY_END_BAD?CINE_BAD:CINE_GOOD;
    } else if(s->ended && s->ending==STORY_END_NEUTRAL){
        s->finale_frame=0;s->dialogue=0;s->page=0;s->cinema=CINE_FINALE;
        s->life_stage=STORY_LIFE_FINALE;
    } else talk(s,D_RETURN);
}
/* Two waves of four telegraphed hazards each, delivered in 30 VBlanks.
   A parries for 15 ticks in FACTS or 12 in CARE; CARE also absorbs one
   missed attack. The real simulation clock, not draw FPS, sets timing. */
int story_attack_lane(const Story *s, int beat) {
    return (beat * 2 + s->cult_enemy + (s->cult_stage >= CULT_RESOLVED ? 1 : 0)) % 3;
}
const char *story_enemy_name(const Story *s) {
    static const char *const pre[3] = {"RATIO RAT", "BADGE LORD", "LORE WARDEN"};
    static const char *const post[3] = {"REBRAND RAT", "EX MODERATOR", "REPLY ARCHIVE"};
    return s->cult_stage >= CULT_RESOLVED ? post[s->cult_enemy % 3] : pre[s->cult_enemy % 3];
}
static void cult_start(Story *s, unsigned enemy) {
    s->battle = BATTLE_MENU;
    s->cult_enemy = (uint8_t)(enemy % CULT_ENCOUNTERS);
    s->battle_choice = 0;
    s->battle_lane = 1;
    s->battle_focus = 4;
    s->battle_progress = 0;
    s->battle_timer = 0;
    s->battle_parry = s->battle_parry_cd = s->battle_guard = 0;
}
static void cult_finish(Story *s, int result) {
    s->battle = BATTLE_NONE;
    if (result > 0) {
        if (s->cult_stage == 0) s->cult_pre_wins |= (uint8_t)(1u << s->cult_enemy);
        else if (s->cult_stage >= CULT_RESOLVED) s->cult_post_wins |= (uint8_t)(1u << s->cult_enemy);
        talk(s, s->cult_stage == 0 ? D_CULT_PRE_WIN :
             s->cult_stage >= CULT_RESOLVED ? D_CULT_POST_WIN : D_CULT_DURING_WIN);
        s->dirty = 1;
    } else talk(s, result == 0 ? D_CULT_RUN : D_CULT_LOSE);
}
static void cult_tick(Story *s, uint16_t pressed) {
    enum { A=1, B=2, RIGHT=16, LEFT=32 };
    if (s->battle == BATTLE_MENU) {
        if (pressed & LEFT) s->battle_choice = (uint8_t)((s->battle_choice + 2) % 3);
        if (pressed & RIGHT) s->battle_choice = (uint8_t)((s->battle_choice + 1) % 3);
        if (pressed & B) { cult_finish(s, 0); return; }
        if (pressed & A) {
            if (s->battle_choice == 2) { cult_finish(s, 0); return; }
            s->battle = BATTLE_DODGE;
            s->battle_timer = 0;
            s->battle_lane = 1;
            s->battle_guard = s->battle_choice == 1;
            s->battle_parry = s->battle_parry_cd = 0;
        }
        return;
    }
    if (pressed & LEFT && s->battle_lane > 0) --s->battle_lane;
    if (pressed & RIGHT && s->battle_lane < 2) ++s->battle_lane;
    if (s->battle_parry) --s->battle_parry;
    if (s->battle_parry_cd) --s->battle_parry_cd;
    if ((pressed & A) && !s->battle_parry_cd) {
        s->battle_parry = (uint8_t)(s->battle_choice==0u?
                         STORY_FACTS_PARRY_TICKS:STORY_CARE_PARRY_TICKS);
        s->battle_parry_cd = (uint8_t)(s->battle_choice==0u?
                         STORY_FACTS_PARRY_COOLDOWN:STORY_CARE_PARRY_COOLDOWN);
    }
    ++s->battle_timer;
    if (s->battle_timer % STORY_BATTLE_BEAT == 0) {
        int beat = (int)(s->battle_timer / STORY_BATTLE_BEAT) - 1;
        if(s->battle_lane==story_attack_lane(s,beat)){
            if(s->battle_parry){
                if(s->battle_choice==0u && s->battle_focus<4u)
                    ++s->battle_focus; /* Accurate FACTS counter */
            }else if(s->battle_guard) s->battle_guard=0;
            else if(s->battle_focus) --s->battle_focus;
        }
    }
    if (s->battle_timer >= STORY_BATTLE_BEAT*STORY_BATTLE_STRIKES) {
        if (!s->battle_focus) { cult_finish(s, -1); return; }
        if (++s->battle_progress >= 2) { cult_finish(s, 1); return; }
        s->battle = BATTLE_MENU;
        s->battle_choice = 0;
    }
}
/* Optional memories are not quest gates, scores, health, or deadlines. */
static int can_make_memory(const Story *s){
    return s->life_stage==STORY_LIFE_WAIT ||
           s->life_stage==STORY_LIFE_FOLLOWUP_WAIT;
}
static int remember(Story *s,unsigned bit){
    if(!can_make_memory(s) || (s->daily_mask&bit))return 0;
    s->daily_mask=(uint8_t)(s->daily_mask|bit);
    if(s->daily_memories<99)++s->daily_memories;
    s->dirty=1;
    return 1;
}
/* Choices carry small, visible benefits, never failure or ending gates. */
static unsigned day_period(const Story *s){
    if(s->day_frame<STORY_DAY_FRAMES/3u)return 0u; /* morning */
    if(s->day_frame<STORY_DAY_FRAMES*2u/3u)return 1u; /* afternoon */
    return 2u; /* evening */
}
static void moment_menu(Story *s,unsigned station){
    s->activity=(uint8_t)station;s->activity_choice=0;
    s->cinema=(uint8_t)(station=='F'?CINE_MARA:station=='L'?CINE_LETTER:
        station=='C'?CINE_BORE:
        (station=='M'||station=='E')?CINE_MUSIC:
        station=='J'?CINE_OPEN:CINE_ART);
    s->cinema_frame=station=='J'?184u:0u;
}
static void stat_gain(uint8_t *v,unsigned gain){
    unsigned sum=(unsigned)*v+gain;
    *v=(uint8_t)(sum>STORY_STAT_MAX?STORY_STAT_MAX:sum);
}
static void work_gain(Story *s,unsigned gain){
    unsigned sum=(unsigned)s->work+gain;
    s->work=(uint8_t)(sum>99u?99u:sum);
}
static int exert(Story *s){
    if(!s->energy)return 0;
    --s->energy;return 1;
}

/* A completed project is a small tangible creation, not a new gate.
   The second choice produces a gentler warm/soft alternative arrangement. */
static void step_project(Story *s,unsigned station,unsigned soft){
    uint8_t *phase=station=='m'?&s->song_steps:&s->art_steps;
    if(*phase>=STORY_PROJECT_DONE){
        talk(s,station=='m'?D_PROJECT_SONG_DONE:D_PROJECT_ART_DONE);
        return;
    }
    if(soft){
        stat_gain(&s->spark,1);
        stat_gain(&s->energy,1);
        s->project_style|=(uint8_t)(station=='m'?
            STORY_STYLE_SONG_SOFT:STORY_STYLE_ART_WARM);
        work_gain(s,1u);
    }else{
        int enough=exert(s);
        work_gain(s,1u+(unsigned)enough+(s->spark>=4u));
        stat_gain(&s->spark,1);
    }
    ++*phase;
    s->dirty=1;
    if(station=='m'){
        talk(s,*phase>=4u?D_PROJECT_SONG_DONE:
            *phase==3u?D_PROJECT_SONG_3:
            *phase==2u?D_PROJECT_SONG_2:
            s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?D_DAY_MUSIC_AFTER:D_DAY_MUSIC);
        if(!(s->flags&STORY_MUSIC)){s->flags|=STORY_MUSIC;s->dirty=1;}
    }else{
        talk(s,*phase>=4u?D_PROJECT_ART_DONE:
            *phase==3u?D_PROJECT_ART_3:
            *phase==2u?D_PROJECT_ART_2:
            s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?D_DAY_PAINT_AFTER:D_DAY_PAINT);
    }
}
static void finish_moment(Story *s){
    unsigned station=s->activity,option=s->activity_choice;
    if(station=='J'){
        /* Opening the menu spends nothing. WORK must be explicitly confirmed. */
        s->activity=STORY_ACT_NONE;s->activity_choice=0;
        if(option){s->cinema=CINE_NONE;return;}
        if(!can_make_memory(s))talk(s,D_SHIFT_EARLY);
        else if(s->daily_mask&STORY_DAY_SHIFT)talk(s,D_SHIFT_REPEAT);
        else if(!s->energy)talk(s,D_SHIFT_TIRED);
        else {
            --s->energy;
            work_gain(s,2u);
            s->daily_mask|=STORY_DAY_SHIFT;
            s->dirty=1;
            talk(s,D_SHIFT_DONE);
        }
        s->cinema_frame=184u; /* avoid replaying the full aerial intro */
        return;
    }
    if(station=='E'){
        if(option){
            s->activity=STORY_ACT_STUDIO;
            s->activity_choice=0; /* transport stopped, no auto play */
            s->studio_tab=STUDIO_SONG;s->studio_cursor=0;
            s->cinema=CINE_MUSIC;s->cinema_frame=0;
        }else{
            s->activity=STORY_ACT_NONE;
            talk(s,D_ACT_MUSIC_LISTEN);
        }
        return;
    }
    if(station=='m'||station=='r'){
        s->activity=STORY_ACT_NONE;s->activity_choice=0;
        step_project(s,station,option);
        return;
    }
    unsigned bit=station=='F'?STORY_DAY_MARA:station=='L'?STORY_DAY_LETTER:
        station=='C'?STORY_DAY_CLERK:station=='M'?STORY_DAY_MUSIC:STORY_DAY_ART;
    s->activity=STORY_ACT_NONE;s->activity_choice=0;
    if(!remember(s,bit))return;
    switch(station){
    case 'F':
        if(option){stat_gain(&s->energy,1);stat_gain(&s->bond,1);talk(s,D_ACT_MARA_TALK);}
        else {exert(s);stat_gain(&s->bond,2);talk(s,s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?
              D_DAY_MARA_AFTER:D_DAY_MARA);}
        break;
    case 'L':
        if(option){stat_gain(&s->energy,1);stat_gain(&s->spark,1);talk(s,D_ACT_LETTER_KEEP);}
        else {exert(s);stat_gain(&s->spark,2);talk(s,s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?
              D_DAY_LETTER_AFTER:D_DAY_LETTER);}
        break;
    case 'C':
        if(option){stat_gain(&s->energy,1);stat_gain(&s->bond,1);talk(s,D_ACT_CLERK_JOKE);}
        else {exert(s);stat_gain(&s->spark,1);talk(s,s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?
              D_DAY_CLERK_AFTER:D_DAY_CLERK);}
        break;
    case 'M':
        if(option){
            stat_gain(&s->energy,1);stat_gain(&s->spark,1);
            talk(s,D_ACT_MUSIC_LISTEN);
        }else step_project(s,'m',0u);
        if(!(s->flags&STORY_MUSIC)){s->flags|=STORY_MUSIC;s->dirty=1;}
        break;
    default:
        if(option){
            stat_gain(&s->energy,2);stat_gain(&s->spark,1);
            talk(s,D_ACT_ART_REST);
        }else step_project(s,'r',0u);
        break;
    }
    s->dirty=1;
}

/* Only explicitly confirmed harmful actions earn checks.
   No countdown, energy deficit or ordinary conversation can harm Mara. */
int story_bad_count(const Story *s){
    unsigned mask=s->bad_mask&15u,n=0;
    for(int i=0;i<4;i++)n+=(mask>>i)&1u;
    return (int)n;
}
static int actor_tile(char c){
    return c=='L'||c=='C'||c=='F'||c=='B'||c=='P'||c=='V'||
           c=='Q'||c=='S'||c=='W'||c=='H'||c=='R'||c=='M'||c=='K';
}
static char nearby_actor(const Story *s){
    static const int dx[4]={1,0,-1,0},dy[4]={0,1,0,-1};
    char c=story_tile(s->x+dx[s->facing],s->y+dy[s->facing]);
    if(!actor_tile(c)){
        /* At a four-way crossroads the old E/S/W/N fallback could select
           the MUSIC station when the player had followed the LAST BUS
           waypoint to its neighboring tile. Prefer the current objective
           only when the player is not explicitly facing another actor. */
        int gx=0,gy=0;const char *label=0;
        if(story_goal(s,&gx,&gy,&label) &&
           ((gx==(int)s->x &&
              (gy==(int)s->y-1||gy==(int)s->y+1)) ||
            (gy==(int)s->y &&
              (gx==(int)s->x-1||gx==(int)s->x+1))) &&
           actor_tile(story_tile(gx,gy))){
            c=story_tile(gx,gy);
        }else{
            for(int i=0;i<4;i++){
                char a=story_tile(s->x+dx[i],s->y+dy[i]);
                if(actor_tile(a)){c=a;break;}
            }
        }
    }
    return c;
}
const char *story_action_hint(const Story *s){
    /* This special storefront interaction is checked BEFORE all actors. */
    if(s->x==20u&&s->y==11u&&s->facing==0u)return "A SHIFT";
    switch(nearby_actor(s)){
    case 'L':return "A READ";
    case 'C':return "A CLERK";
    case 'F':return s->rescue_stage==STORY_RESCUE_LOST?"A PHONE":
          (s->rescue_stage>=STORY_RESCUE_MISSING&&
           s->rescue_stage<=STORY_RESCUE_TRACE)?"A EMPTY CAFE":"A MARA";
    case 'B':return "A BUS";
    case 'M':return "A MUSIC";
    case 'R':return "A CANVAS";
    case 'K':return "A CLINIC";
    case 'P':case 'V':case 'Q':return "A DUEL";
    case 'S':return "A TERMINAL";
    case 'W':return "A WITNESS";
    case 'H':return "A DOOR";
    default:return "A LOOK";
    }
}
int story_risk_available(const Story *s){
    char target=nearby_actor(s);
    unsigned bit=target=='L'?STORY_BAD_LETTER:
                 target=='C'?STORY_BAD_CLERK:
                 target=='M'?STORY_BAD_MUSIC:
                 target=='S'?STORY_BAD_SERVER:0u;
    return bit && !(s->bad_mask&bit) &&
           s->rescue_stage!=STORY_RESCUE_LOST &&
           s->rescue_stage!=STORY_RESCUE_AFTERSHOCK;
}
static void begin_bad_film(Story *s){
    s->ended=1;s->ending=STORY_END_BAD;s->life_stage=STORY_LIFE_NONE;
    s->finale_frame=0;s->dialogue=0;s->page=0;
    s->scene=STORY_SCENE_NONE;s->cinema=CINE_BAD;s->cinema_frame=0;
    s->risk_menu=0;s->risk_choice=0;s->dirty=1;
}
/* The failed rescue does NOT instantly decide Mara's fate. Reuse the
   already-persisted day clock for this chapter; no new SRAM flags required.
   These days are independent of the clinic's original numbered days. */
static void begin_wait_for_call(Story *s,unsigned away_from_house){
    s->rescue_stage=STORY_RESCUE_LOST;
    s->life_stage=STORY_LIFE_NONE;
    s->day_number=0;s->day_frame=0;
    s->daily_mask=0;s->scene=STORY_SCENE_NONE;s->dirty=1;
    talk(s,away_from_house?D_RESCUE_LOST_REMOTE:D_RESCUE_LOST);
}
static void commit_bad(Story *s,char target){
    unsigned bit=target=='L'?STORY_BAD_LETTER:target=='C'?STORY_BAD_CLERK:
                 target=='M'?STORY_BAD_MUSIC:STORY_BAD_SERVER;
    if(s->bad_mask&bit)return; /* non-repeatable */
    s->bad_mask|=(uint8_t)bit;s->dirty=1;
    unsigned count=(unsigned)story_bad_count(s);
    if(count==4u && s->rescue_stage>=STORY_RESCUE_MISSING){
        if(s->rescue_stage==STORY_RESCUE_SAVED){
            s->rescue_stage=STORY_RESCUE_AFTERSHOCK;
            talk(s,D_RESCUE_AFTERSHOCK);
        }else begin_wait_for_call(s,1u);
    }else if(count==STORY_BAD_THRESHOLD &&
             s->rescue_stage==STORY_RESCUE_NONE){
        s->rescue_stage=STORY_RESCUE_MISSING;talk(s,D_MARA_MISSING);
    }else if(count==2u && s->rescue_stage==STORY_RESCUE_NONE){
        talk(s,D_BAD_WARNING_2);
    }else talk(s,target=='L'?D_BAD_LETTER:target=='C'?D_BAD_CLERK:
               target=='M'?(s->song_steps>=STORY_PROJECT_DONE?
                   D_BAD_MUSIC_MASTER:D_BAD_MUSIC):D_BAD_SERVER);
}
static void rescue_reunion(Story *s){
    if(story_check_live(s,STORY_CHECK_MARA_TRUST))
        talk(s,D_RESCUE_SAVED_TRUST);
    else if(story_check_live(s,STORY_CHECK_FRAMED_ART))
        talk(s,D_RESCUE_SAVED_ART);
    else talk(s,D_RESCUE_SAVED);
}
static void resolve_risk(Story *s){
    char target=(char)s->risk_menu;
    unsigned selected=s->risk_choice;
    s->risk_menu=0;s->risk_choice=0;s->cinema=CINE_NONE;
    if(target=='H'){
        if(s->rescue_stage!=STORY_RESCUE_TRACE)return;
        if(selected)begin_wait_for_call(s,0u);
        else{
            s->rescue_stage=STORY_RESCUE_SAVED;s->dirty=1;
            rescue_reunion(s);
        }
    }else if(selected)commit_bad(s,target);
}
static void interact(Story *s) {
    /* The isometric shop occupies the solid (21,11) facade. Facing it
       from its adjacent walkable tile opens an OPTIONAL work interaction.
       No collision changes, new flags in SRAM, or mandatory job gate. */
    if(s->x==20u && s->y==11u && s->facing==0u){
        if(s->rescue_stage==STORY_RESCUE_LOST ||
           (s->rescue_stage>=STORY_RESCUE_MISSING &&
            s->rescue_stage<=STORY_RESCUE_TRACE)){
            talk(s,D_SHIFT_PAUSED);return;
        }
        if(!can_make_memory(s))talk(s,D_SHIFT_EARLY);
        else if(s->daily_mask&STORY_DAY_SHIFT)talk(s,D_SHIFT_REPEAT);
        else if(!s->energy)talk(s,D_SHIFT_TIRED);
        else moment_menu(s,'J'); /* WORK / LEAVE, no automatic cost */
        /* Repeated visits never replay the whole aerial approach. */
        s->cinema_frame=184u;
        return;
    }
    char c=nearby_actor(s);
    switch (c) {
    case 'L':
        if(can_make_memory(s) && !(s->daily_mask&STORY_DAY_LETTER))moment_menu(s,'L');
        else talk(s,(s->flags & STORY_HOME) ? D_LETTER_AGAIN : D_LETTER);
        if (!(s->flags & STORY_HOME)) { s->flags |= STORY_HOME; s->dirty = 1; }
        break;
    case 'C':
        if (!(s->flags & STORY_HOME)) talk(s, D_CLERK_LOCKED);
        else {
            if(can_make_memory(s) && !(s->daily_mask&STORY_DAY_CLERK))moment_menu(s,'C');
            else if(can_make_memory(s) && s->song_steps>=STORY_PROJECT_DONE)
                talk(s,D_CLERK_SONG);
            else if(can_make_memory(s) && day_period(s)==2u)
                talk(s,D_CLERK_CLOSING);
            else talk(s,(s->flags & STORY_STAMP) ? D_CLERK_AGAIN : D_CLERK);
            if (!(s->flags & STORY_STAMP)) { s->flags |= STORY_STAMP; s->dirty = 1; }
        }
        break;
    case 'F':
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,s->day_number>=STORY_HOPE_REALIZE_DAY?
                 D_HOPE_PHONE_LATE:D_HOPE_PHONE_WAIT);
            break;
        }
        if(s->rescue_stage>=STORY_RESCUE_MISSING &&
           s->rescue_stage<=STORY_RESCUE_TRACE){
            talk(s,D_MARA_ABSENT);break;
        }
        if (!(s->flags & STORY_STAMP)) talk(s, D_MARA_LOCKED);
        else {
            /* The Promise checkmark is granted only after the actual
               promise scene, never from small talk or a menu. */
            if (!(s->flags & STORY_MARA)) {
                /* The promise is not granted until the player actually
                   finishes and confirms Danny's answer with A. */
                talk(s,D_MARA);
            } else if(can_make_memory(s) &&
                      !(s->daily_mask&STORY_DAY_MARA))moment_menu(s,'F');
            else if(story_bad_count(s)>=2)talk(s,D_MARA_WARNING);
            else if(story_bad_count(s)==1)talk(s,D_MARA_CONCERN);
            else if(can_make_memory(s) && s->art_steps>=STORY_PROJECT_DONE)
                talk(s,D_MARA_GALLERY);
            else if(can_make_memory(s) && s->bond>=4)talk(s,D_ACT_MARA_CLOSE);
            else if(can_make_memory(s) && day_period(s)==0u)talk(s,D_MARA_MORNING);
            else if(can_make_memory(s) && day_period(s)==2u)talk(s,D_MARA_CLOSING);
            else talk(s,D_MARA_AGAIN);
        }
        break;
    case 'B':
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,s->day_number>=STORY_HOPE_REALIZE_DAY?
                 D_HOPE_NOTE_LATE:D_HOPE_NOTE_WAIT);
            break;
        }
        if(s->rescue_stage>=STORY_RESCUE_MISSING &&
           s->rescue_stage<=STORY_RESCUE_TRACE)talk(s,D_MARA_MISSING);
        else if((s->flags&STORY_ALL)==STORY_ALL &&
             (!s->life_stage ||
              (s->rescue_stage==STORY_RESCUE_SAVED &&
               s->life_stage<=STORY_LIFE_FOLLOWUP_WAIT)))
             talk(s,D_BUS_READY);
        else talk(s,D_BUS_LOCKED);
        break;
    case 'M':
        if(s->rescue_stage>=STORY_RESCUE_MISSING &&
           s->rescue_stage<=STORY_RESCUE_TRACE){
            talk(s,D_MARA_MISSING); /* no cheerful studio work during rescue */
        }else if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,s->day_number>=STORY_HOPE_REALIZE_DAY?
                 D_HOPE_NOTE_LATE:D_HOPE_NOTE_WAIT);
        }else if(s->song_steps>=STORY_PROJECT_DONE){
            moment_menu(s,'E'); /* LISTEN or the unlocked full editor */
        }else if(can_make_memory(s)){
            if(!(s->daily_mask&STORY_DAY_MUSIC))moment_menu(s,'M');
            else moment_menu(s,'m');
        }else if(!(s->flags & STORY_MUSIC)){
            s->flags |= STORY_MUSIC;s->dirty=1;talk(s,D_MUSIC);
        }else talk(s,s->work>=8?D_ACT_MUSIC_PROGRESS:D_MUSIC_AGAIN);
        break;
    case 'R':
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,s->day_number>=STORY_HOPE_REALIZE_DAY?
                 D_HOPE_NOTE_LATE:D_HOPE_NOTE_WAIT);
            break;
        }
        /* The missing-person rescue takes priority even after the last
           clinic visit; the player cannot accidentally bypass the crisis. */
        if(s->rescue_stage>=STORY_RESCUE_MISSING &&
           s->rescue_stage<=STORY_RESCUE_TRACE){
            talk(s,s->rescue_stage==STORY_RESCUE_MISSING?D_RESCUE_NOTES_1:
                   s->rescue_stage==STORY_RESCUE_WITNESS?D_RESCUE_NOTES_2:
                   D_RESCUE_NOTES_3);
            break;
        }
        if (s->life_stage == STORY_LIFE_LAST_WORK) {
            s->life_stage=STORY_LIFE_FINALE;s->finale_frame=0;
            s->scene=STORY_SCENE_NONE;s->dirty=1;
            /* A timed in-scene subtitle replaces the blocking opening dialogue.
               The music and animation therefore start together even if nobody
               presses A for the next 72 seconds. */
            s->dialogue=0;s->page=0;s->cinema=CINE_FINALE;s->cinema_frame=0;
        } else if (s->life_stage == STORY_LIFE_FINALE) {
            /* The final cutscene is already playing: no restarting it. */
        } else if (s->life_stage == STORY_LIFE_CLINIC ||
                   s->life_stage == STORY_LIFE_FINAL_CLINIC) {
            talk(s,D_ART_LAST_LOCKED);
        } else if (s->life_stage == STORY_LIFE_WAIT ||
                   s->life_stage == STORY_LIFE_FOLLOWUP_WAIT) {
            if(!(s->daily_mask&STORY_DAY_ART))moment_menu(s,'R');
            else if(s->art_steps<STORY_PROJECT_DONE)moment_menu(s,'r');
            else talk(s,D_PROJECT_ART_DONE);
        } else if (!(s->flags & STORY_ART)) {
            s->flags |= STORY_ART;s->dirty=1;talk(s,D_ART);
        } else talk(s,D_ART_AGAIN);
        break;
    case 'K':
        if(s->rescue_stage>=STORY_RESCUE_MISSING &&
           s->rescue_stage<=STORY_RESCUE_TRACE){
            talk(s,D_CLINIC_RESCUE);break;
        }
        if (s->life_stage == STORY_LIFE_CLINIC) {
            s->life_stage=STORY_LIFE_FOLLOWUP_WAIT;
            s->scene=STORY_SCENE_CLINIC;s->dirty=1;talk(s,D_DOCTOR_FIRST);
        } else if (s->life_stage == STORY_LIFE_FINAL_CLINIC) {
            s->life_stage=STORY_LIFE_LAST_WORK;
            s->scene=STORY_SCENE_CLINIC;s->dirty=1;talk(s,D_DOCTOR_LAST);
        } else talk(s,D_CLINIC_LOCKED);
        break;
    case 'P': cult_start(s, 0); break;
    case 'V': cult_start(s, 1); break;
    case 'Q': cult_start(s, 2); break;
    case 'S':
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,D_HOPE_TERMINAL);break;
        }
        if(s->rescue_stage==STORY_RESCUE_WITNESS){
            s->rescue_stage=STORY_RESCUE_TRACE;s->dirty=1;
            talk(s,D_RESCUE_TRACE);break;
        }
        if (s->cult_stage == 0) { s->cult_stage = 1; talk(s, D_CULT_FIND); s->dirty = 1; }
        else if (s->cult_stage == 3) { s->cult_stage = CULT_RESOLVED; talk(s, D_CULT_BACKUP); s->dirty = 1; }
        else talk(s, s->cult_stage >= CULT_RESOLVED ? D_CULT_AFTER :
                  s->cult_stage == 1 ? D_CULT_LOGS : D_CULT_WAIT);
        break;
    case 'W':
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,D_HOPE_WITNESS);break;
        }
        if(s->rescue_stage==STORY_RESCUE_MISSING){
            s->rescue_stage=STORY_RESCUE_WITNESS;s->dirty=1;
            talk(s,story_check_live(s,STORY_CHECK_ORCHARD)?
                 D_RESCUE_WITNESS_TRUST:D_RESCUE_WITNESS);break;
        }
        if (s->cult_stage == 1) { s->cult_stage = 2; talk(s, D_CULT_WITNESS); s->dirty = 1; }
        else talk(s, s->cult_stage > 1 ? D_CULT_WITNESS_AFTER : D_CULT_NOT_YET);
        break;
    case 'H':
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,D_HOPE_DOOR);break;
        }
        if(s->rescue_stage==STORY_RESCUE_TRACE){
            s->risk_menu='H';s->risk_choice=0;
            s->cinema=CINE_RESCUE_DOOR;s->cinema_frame=0;break;
        }
        if(s->rescue_stage==STORY_RESCUE_SAVED){
            rescue_reunion(s);break;
        }
        if (s->cult_stage == 2) { s->cult_stage = 3; talk(s, D_CULT_SAFETY); s->dirty = 1; }
        else talk(s, D_CULT_SAFE_AGAIN);
        break;
    default:
        /* Standing still is an invitation to notice the neighborhood,
           not a dead control. Geography decides the short observation. */
        if(s->x>=17u && s->y>=9u && s->y<=13u)
            talk(s,D_LOOK_POTHEADS);
        else if(s->x>=15u && s->y>=13u)
            talk(s,D_LOOK_CANAL);
        else if(s->x<=7u && s->y>=8u)
            talk(s,D_LOOK_MARKET);
        else if(s->y<=7u)
            talk(s,D_LOOK_NORTH);
        else talk(s,D_LOOK_STREET);
        break;
    }
}
void story_tick(Story *s, uint16_t keys) {
    /* Use the same key-bit convention as REG_KEYINPUT, after inversion. */
    enum { A=1, B=2, SELECT=4, START=8, RIGHT=16, LEFT=32, UP=64, DOWN=128, R=256, L=512 };
    uint16_t pressed = (uint16_t)(keys & ~s->old_keys);
    s->old_keys = keys;
    if (s->anim) --s->anim;
    /* The fourth violation after a SUCCESSFUL rescue retains its original
       immediate-bad-ending consequence. A failed rescue now has a playable,
       two-day unanswered-phone epilogue with no assumed death. */
    if(!s->dialogue&&!s->ended&&
       s->rescue_stage==STORY_RESCUE_AFTERSHOCK)
        begin_bad_film(s);
    if(s->rescue_stage==STORY_RESCUE_LOST && !s->ended &&
       !s->paused && !s->dialogue && !s->battle &&
       !s->choosing && !s->risk_menu && !s->activity){
        if(++s->day_frame>=STORY_DAY_FRAMES){
            s->day_frame=0;
            if(s->day_number<STORY_HOPE_END_DAY)++s->day_number;
            s->dirty=1;
            if(s->day_number==STORY_HOPE_REALIZE_DAY)
                talk(s,D_HOPE_REALIZE);
            else if(s->day_number>=STORY_HOPE_END_DAY)
                begin_bad_film(s);
        }
    }
    if (s->cinema && !s->paused && s->cinema_frame < 65530) ++s->cinema_frame;
    /* One step is exactly one VBlank, including dialogue. Pause freezes the
       clock. Day transitions never steal controls during a fight or textbox. */
    if (!s->ended && s->life_stage >= STORY_LIFE_WAIT &&
        s->life_stage <= STORY_LIFE_LAST_WORK && !s->paused &&
        s->activity!=STORY_ACT_STUDIO) {
        if (++s->day_frame >= STORY_DAY_FRAMES) {
            s->day_frame=0;
            if (s->day_number<99) ++s->day_number;
            s->daily_mask=0; /* optional moments reset every morning */
            stat_gain(&s->energy,2);
            s->dirty=1;
        }
        if (!s->dialogue && !s->battle && !s->choosing && !s->activity && !s->risk_menu) {
            if(s->rescue_stage>=STORY_RESCUE_MISSING &&
               s->rescue_stage<=STORY_RESCUE_TRACE){
                /* Appointments stay on the calendar, without scenes that
                   incorrectly show Mara while she is missing. */
            }else if (s->life_stage==STORY_LIFE_WAIT && s->day_number>=2) {
                s->life_stage=STORY_LIFE_CLINIC;
                s->scene=STORY_SCENE_NOTICE;s->dirty=1;talk(s,D_MEDICAL_NOTICE);
            } else if (s->life_stage==STORY_LIFE_FOLLOWUP_WAIT && s->day_number>=3) {
                s->life_stage=STORY_LIFE_FINAL_CLINIC;
                s->scene=STORY_SCENE_NOTICE;s->dirty=1;
                talk(s,s->rescue_stage==STORY_RESCUE_LOST?
                     D_DOCTOR_RETURN_ALONE:D_DOCTOR_RETURN);
            } else if(can_make_memory(s) &&
                      (s->daily_mask&STORY_DAY_ALL)==STORY_DAY_ALL &&
                      !(s->daily_mask&STORY_DAY_RECAP)) {
                s->daily_mask|=STORY_DAY_RECAP;s->dirty=1;
                talk(s,s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?
                    D_DAY_RECAP_AFTER:D_DAY_RECAP);
            }
        }
    }
    /* Ending pictures are authored over the COMPLETE score, not just the
       original 450-frame animation. Keep one shot clock through the chorus,
       verse returns and the good-only coda. Input cannot accidentally skip
       the performance or leave the camera on a static credit card. */
    if (!s->dialogue && (
          (s->life_stage==STORY_LIFE_FINALE && !s->ended) ||
          (s->ended && ((s->ending==STORY_END_GOOD &&
           s->finale_frame<STORY_GOOD_FINAL_FRAMES) ||
           (s->ending==STORY_END_BAD &&
           s->finale_frame<STORY_END_SCORE_FRAMES))))) {
        unsigned limit=s->ending==STORY_END_GOOD?
            STORY_GOOD_FINAL_FRAMES:STORY_END_SCORE_FRAMES;
        if (s->finale_frame<limit) ++s->finale_frame;
        if (s->finale_frame>=limit) {
            if (s->ending==STORY_END_GOOD)
                talk(s,story_bad_count(s)?D_GOOD_END_SCARS:D_GOOD_END);
            else if(s->ending==STORY_END_BAD)
                talk(s,s->rescue_stage==STORY_RESCUE_AFTERSHOCK?
                     D_BAD_END_SAVED:D_BAD_END);
            else {
                s->ended=1;s->ending=STORY_END_NEUTRAL;
                talk(s,D_NEUTRAL_END);
            }
            s->dirty=1;
        }
        return;
    }
    /* Combat takes priority over exploration, so toggling 2D/3D cannot evade a hit. */
    if (s->battle) { cult_tick(s, pressed); return; }
    if (s->dialogue) {
        if (pressed & (A|B)) {
            int count; story_pages(s, &count);
            if ((pressed & A) && s->page + 1 < count) {
                ++s->page;
                /* The opening is editorially cut to seven spoken beats.
                   Fast readers may advance, but each line then gets its
                   intended visible camera pose instead of skipping the
                   entire storefront reveal and dance. The scene clock
                   remains monotonic; B can still dismiss the dialogue. */
                uint16_t beat=dialogue_shot_beat(s->dialogue,s->page);
                if(s->cinema_frame<beat)s->cinema_frame=beat;
            }
            else {
                int ready = s->dialogue == D_BUS_READY && (pressed & A);
                /* B skips an unfinished conversation; no promise check.
                   Only A on Danny's final spoken answer earns it. */
                if(s->dialogue==D_MARA && (pressed&A) &&
                   !(s->flags&STORY_MARA)){
                    s->flags|=STORY_MARA;s->dirty=1;
                }
                s->dialogue = 0; s->page = 0; s->scene=STORY_SCENE_NONE;
                if(!ready && !s->ended && s->life_stage!=STORY_LIFE_FINALE)s->cinema=CINE_NONE;
                if (ready) { s->choosing = 1; s->choice = 0; }
            }
        }
        return;
    }
    if (s->ended) {
        /* An ending is a stable scene, not a doorway back into gameplay. */
        if (pressed & START) s->quit=1;
        return;
    }
    if(s->risk_menu){
        if(pressed&(LEFT|RIGHT|UP|DOWN))s->risk_choice^=1u;
        if(pressed&B){s->risk_menu=0;s->risk_choice=0;s->cinema=CINE_NONE;}
        else if(pressed&A)resolve_risk(s);
        return;
    }
    if(s->activity==STORY_ACT_STUDIO){
        /* Easy enough to play entirely with the D-pad and face buttons.
           L/R switch pages, D-pad edits the current setting, SELECT
           previews the *actual* PSG notes, B exits without losing work. */
        if(pressed&(B|START)){
            s->activity=STORY_ACT_NONE;s->activity_choice=0;
            s->cinema=CINE_NONE;
            if(pressed&START)s->paused=1;
            return;
        }
        if(pressed&(L|R)){
            s->studio_tab=(uint8_t)((s->studio_tab+
                ((pressed&L)?STUDIO_TAB_COUNT-1u:1u))%STUDIO_TAB_COUNT);
            s->studio_cursor=0;
        }
        unsigned count=studio_cursor_count(s->studio_tab);
        if(pressed&LEFT)
            s->studio_cursor=(uint8_t)((s->studio_cursor+count-1u)%count);
        if(pressed&RIGHT)
            s->studio_cursor=(uint8_t)((s->studio_cursor+1u)%count);
        if(pressed&UP){
            studio_change(s->studio_tab,s->studio_cursor,1);
            s->dirty=1;
        }else if(pressed&DOWN){
            studio_change(s->studio_tab,s->studio_cursor,-1);
            s->dirty=1;
        }
        if(pressed&A){
            if(s->studio_tab==STUDIO_PERFORM)
                s->activity_choice^=1u;
            else {
                studio_toggle(s->studio_tab,s->studio_cursor);
                s->dirty=1;
            }
        }
        if(pressed&SELECT)s->activity_choice^=1u;
        return;
    }
    if(s->activity){
        if(pressed&(LEFT|RIGHT|UP|DOWN))s->activity_choice^=1u;
        if(pressed&B){s->activity=STORY_ACT_NONE;s->cinema=CINE_NONE;}
        else if(pressed&A)finish_moment(s);
        return;
    }
    if (s->choosing) {
        if (pressed & (LEFT|RIGHT|UP|DOWN)) s->choice ^= 1;
        if (pressed & B) { s->choosing = 0; s->cinema = CINE_NONE; }
        if (pressed & A) {
            s->choosing=0;
            if (s->choice==0 && (s->flags & STORY_FULL)==STORY_FULL) {
                s->ended=1;s->ending=STORY_END_GOOD;
                /* An early happy-route choice after a rescue must stop the
                   numbered-day clinic clock during the complete ending film. */
                s->life_stage=STORY_LIFE_NONE;s->scene=STORY_SCENE_NONE;
                s->finale_frame=0;s->dialogue=0;s->page=0;
                s->cinema=CINE_GOOD;s->cinema_frame=0;
            } else if (s->choice==0) {
                /* Adequate progress, but not 100%: keep living and crafting.
                   The neutral ending is delayed by active, numbered game days. */
                if(s->life_stage){
                    talk(s,D_BUS_RESCUE);
                }else{
                    s->life_stage=STORY_LIFE_WAIT;
                    s->day_number=1;s->day_frame=0;
                    talk(s,D_NEUTRAL_START);
                }
            } else {
                /* STAY means continue exploring; it is not an instant ending. */
                talk(s,D_ART_DAY);
            }
            s->dirty=1;
        }
        return;
    }
    if(pressed&START){
        s->paused^=1u;
        /* The journal is transient UI, never part of a bus decision. */
        if(!s->paused)s->choice&=1u;
        return;
    }
    if(s->paused){
        /* L is a direct two-page help guide. R remains the four-page
           consequences journal; both are transient and never save. */
        if(pressed&L){
            if(s->choice&STORY_FIELD_GUIDE_ACTIVE)s->choice&=1u;
            else s->choice=(uint8_t)((s->choice&1u)|
                                      STORY_FIELD_GUIDE_ACTIVE);
        }else if((pressed&(LEFT|RIGHT)) &&
                 (s->choice&STORY_FIELD_GUIDE_ACTIVE)){
            s->choice^=STORY_FIELD_GUIDE_DETAIL;
        }else if(pressed&R){
            unsigned page=(s->choice&STORY_CHECK_PAGE_ACTIVE)?
                (story_check_page(s)+1u)%STORY_CHECK_PAGE_COUNT:0u;
            s->choice=(uint8_t)((s->choice&1u)|STORY_CHECK_PAGE_ACTIVE|
                                (page<<1));
        }else if(pressed&B){
            if(s->choice&(STORY_CHECK_PAGE_ACTIVE|STORY_FIELD_GUIDE_ACTIVE))
                s->choice&=1u;
            else s->quit=1;
        }
        return;
    }
    if (pressed & (L|SELECT)) { s->view ^= 1; s->dirty = 1; }
    if (pressed & R) {
        if(s->rescue_stage==STORY_RESCUE_LOST){
            talk(s,s->day_number>=STORY_HOPE_REALIZE_DAY?
                 D_HOPE_NOTE_LATE:D_HOPE_NOTE_WAIT);return;
        }
        if(s->rescue_stage==STORY_RESCUE_MISSING){
            talk(s,D_RESCUE_NOTES_1);return;
        }
        if(s->rescue_stage==STORY_RESCUE_WITNESS){
            talk(s,D_RESCUE_NOTES_2);return;
        }
        if(s->rescue_stage==STORY_RESCUE_TRACE){
            talk(s,D_RESCUE_NOTES_3);return;
        }
        if(s->rescue_stage==STORY_RESCUE_SAVED){
            talk(s,D_RESCUE_NOTES_SAVED);return;
        }
        if (s->life_stage) {
            talk(s,s->life_stage==STORY_LIFE_CLINIC||s->life_stage==STORY_LIFE_FINAL_CLINIC?D_NOTE_VISIT:
                 s->life_stage==STORY_LIFE_FOLLOWUP_WAIT?D_NOTE_FOLLOWUP:
                 s->life_stage>=STORY_LIFE_LAST_WORK?D_NOTE_CANVAS:D_DAY_NOTES);
            return;
        }
        if (s->cult_stage) {
            talk(s, (uint8_t)(D_CULT_NOTES_1 + s->cult_stage - 1));
            return;
        }
        talk(s, (s->flags & STORY_ALL)==STORY_ALL ? D_JOURNAL_3 :
             (s->flags & STORY_STAMP) ? D_JOURNAL_2 :
             (s->flags & STORY_HOME) ? D_JOURNAL_1 : D_JOURNAL_0);
        return;
    }
    if (pressed & A) { interact(s); return; }
    if(pressed&B){
        char target=nearby_actor(s);
        unsigned bit=target=='L'?STORY_BAD_LETTER:
                     target=='C'?STORY_BAD_CLERK:
                     target=='M'?STORY_BAD_MUSIC:
                     target=='S'?STORY_BAD_SERVER:0u;
        if(bit && story_risk_available(s)){
            s->risk_menu=(uint8_t)target;s->risk_choice=0;
            s->cinema=target=='L'?CINE_LETTER:
                target=='C'?CINE_BORE:
                target=='M'?CINE_MUSIC:CINE_SERVER;
            s->cinema_frame=0;
        }
        return;
    }
    if (s->cooldown) --s->cooldown;
    if (!s->cooldown) {
        int d = -1;
        if (s->view) {
            if (pressed & LEFT) { s->facing = (uint8_t)((s->facing+3)&3); s->cooldown = 5; s->dirty=1; }
            else if (pressed & RIGHT) { s->facing=(uint8_t)((s->facing+1)&3); s->cooldown=5; s->dirty=1; }
            else if (keys & UP) d = s->facing;
            else if (keys & DOWN) d = (s->facing+2)&3;
        } else {
            if (keys & UP) d = 3;
            else if (keys & DOWN) d = 1;
            else if (keys & LEFT) d = 2;
            else if (keys & RIGHT) d = 0;
            if (d >= 0) s->facing = (uint8_t)d;
        }
        if (d >= 0) {
            static const int dx[4] = {1,0,-1,0}, dy[4]={0,1,0,-1};
            int x = s->x + dx[d], y = s->y + dy[d];
            if (!story_solid(x,y)) {
                s->prev_x=s->x; s->prev_y=s->y; s->x=(uint8_t)x; s->y=(uint8_t)y;
                s->anim=4; s->dirty=1;
            }
            s->cooldown=6;
        }
    }
}

