#include "story.h"

/* A quiet neighborhood, not a random maze. Every story anchor is reachable on foot. */
static const char *const town[STORY_H] = {
"########################",
"#......................#",
"#...TT............TT...#",
"#..TTTT...####....TT...#",
"#.........#AA#.........#",
"#...L.....#..#.....B...#",
"#.........####.........#",
"#......................#",
"#...,,,,,,,,,,,,,,,,...#",
"#..T,,,......,,,T.gg...#",
"#..TT,,......,,TT.gg####",
"#...g,,.F....,,......#.#",
"#...g,,,,,,,,,,......#.#",
"#..TT.......D....gg..#.#",
"#..TT............~~~...#",
"#...............~~~~...#",
"#...............~~~~...#",
"########################"
};

char story_tile(int x, int y) {
    if (x < 0 || y < 0 || x >= STORY_W || y >= STORY_H) return '#';
    /* C is the BORE clerk: outside the closed archive on the plaza. */
    if (x == 11 && y == 7) return 'C';
    /* Three persistent fandom hecklers; positions are identical in 2D and 3D. */
    if (x == 3 && y == 7) return 'P';
    if (x == 17 && y == 7) return 'V';
    if (x == 18 && y == 12) return 'Q';
    /* Fictional Mirror Orchard terminal, witness and an offline safe house. */
    if (x == 21 && y == 7) return 'S';
    if (x == 6 && y == 11) return 'W';
    if (x == 7 && y == 2) return 'H';
    /* Art studio, music archive and outpatient clinic are all walkable anchors. */
    if (x == 14 && y == 6) return 'R';
    if (x == 19 && y == 7) return 'M';
    if (x == 16 && y == 5) return 'K';
    return town[y][x];
}
int story_goal(const Story *s, int *x, int *y, const char **label) {
    if (s->ended || s->life_stage==STORY_LIFE_FINALE) return 0;
    /* A medical appointment takes priority; there is deliberately no timer
       forcing a final rooftop interaction after the last visit. */
    if (s->life_stage==STORY_LIFE_CLINIC ||
        s->life_stage==STORY_LIFE_FINAL_CLINIC) {
        *x=16; *y=5; *label="CLINIC";
    } else if (s->life_stage==STORY_LIFE_LAST_WORK) {
        *x=14; *y=6; *label="CANVAS";
    } else if (s->life_stage==STORY_LIFE_WAIT ||
               s->life_stage==STORY_LIFE_FOLLOWUP_WAIT) {
        *x=14; *y=6; *label="LIVE";
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
       D_NOTE_VISIT, D_NOTE_FOLLOWUP, D_NOTE_CANVAS };
#define SCRIPT(name, ...) static const StoryPage name[] = { __VA_ARGS__ }
SCRIPT(d_open,
 {"DANNY", "The last bus leaves", "at sundown. Of course."},
 {"DANNY", "Even the apocalypse", "has a timetable."},
 {"DANNY", "Find what matters.", "Then make a choice."});
SCRIPT(d_letter,
 {"OLD LETTER", "Dear Danny, I left", "the porch light on."},
 {"OLD LETTER", "The power company", "took it. I tried."},
 {"DANNY", "Somebody waited.", "I should remember."});
SCRIPT(d_letter_again,
 {"DANNY", "That little letter", "weighs a whole ton."});
SCRIPT(d_clerk_locked,
 {"BORE CLERK", "Form B-2 requires", "proof you existed."},
 {"BORE CLERK", "Try looking for", "a letter from home."});
SCRIPT(d_clerk,
 {"BORE CLERK", "Welcome to the BORE", "Continuity Office."},
 {"BORE CLERK", "We're not ending", "reality. Budget cuts."},
 {"BORE CLERK", "ONE EXISTENCE,", "PROVISIONALLY OKAY."},
 {"DANNY", "They stamped my", "ticket for tomorrow."});
SCRIPT(d_clerk_again,
 {"BORE CLERK", "Do not fold the", "universe in half."},
 {"BORE CLERK", "The warranty is", "very specific."});
SCRIPT(d_mara_locked,
 {"MARA", "You're leaving?", "You always do that."},
 {"MARA", "Talk to the clerk.", "Make it official."});
SCRIPT(d_mara,
 {"MARA", "You still laugh", "when you're scared."},
 {"DANNY", "Only because my", "knees are louder."},
 {"MARA", "Promise me you'll", "keep the good bits."},
 {"DANNY", "I will. Even the", "terrible jokes."});
SCRIPT(d_mara_again,
 {"MARA", "Whatever happens,", "do not forget us."});
SCRIPT(d_bus_locked,
 {"DRIVER", "LAST BUS: TOMORROW.", "NO REFUNDS: YESTERDAY."},
 {"DRIVER", "You still have", "unfinished goodbyes."});
SCRIPT(d_bus_ready,
 {"DRIVER", "Ticket, letter,", "promise. That's all."},
 {"DRIVER", "There's a seat out", "or a street to stay."},
 {"DANNY", "Either way, today", "was real."});
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
 {"POCKET NOTEBOOK", "Letter: missing.", "BORE stamp: missing."},
 {"POCKET NOTEBOOK", "Mara: waiting.", "Bus: on schedule."});
SCRIPT(d_journal_1,
 {"POCKET NOTEBOOK", "Letter: kept safe.", "Find the BORE clerk."});
SCRIPT(d_journal_2,
 {"POCKET NOTEBOOK", "Stamp: acquired.", "Go see Mara."});
SCRIPT(d_journal_3,
 {"POCKET NOTEBOOK", "Promise: made.", "The bus is waiting."});
SCRIPT(d_nothing,
 {"DANNY", "Nothing to do", "with that. Yet."});
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
 {"ARCHIVE TAPE", "Unused BORE songs.", "They still play."},
 {"DANNY", "Somebody might like", "these someday."},
 {"DANNY", "I should finish", "the missing track."});
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
 {"DAY ONE", "Danny stays behind", "for an ordinary day."},
 {"DANNY", "I did enough to", "leave. Not all of it."},
 {"DANNY", "I'll keep painting", "while the buses run."});
SCRIPT(d_medical_notice,
 {"DAY TWO", "A clinic message", "arrived this morning."},
 {"DANNY", "The test results?", "I had forgotten."},
 {"CLINIC", "Please come in.", "We need to talk."});
SCRIPT(d_doctor_first,
 {"DOCTOR", "The tests show a", "serious illness."},
 {"DOCTOR", "We have more to", "discuss next visit."},
 {"DANNY", "I thought I had", "years, not days."},
 {"DOCTOR", "Tell someone you", "trust. Don't go alone."});
SCRIPT(d_doctor_return,
 {"DAY THREE", "A final appointment", "is scheduled today."},
 {"MARA", "I'll go with you.", "No jokes required."},
 {"DANNY", "I brought two.", "Just in case."});
SCRIPT(d_doctor_last,
 {"DOCTOR", "The cancer is far", "advanced. I'm sorry."},
 {"DOCTOR", "We can support you", "through what comes."},
 {"MARA", "We can sit here as", "long as you need."},
 {"DANNY", "I'd like to go", "finish my painting."});
SCRIPT(d_neutral_paint,
 {"ROOFTOP", "Danny returned to", "the unfinished sky."},
 {"DANNY", "One last bit of", "light. Right there."},
 {"DANNY", "I never did learn", "when to stop."});
SCRIPT(d_neutral_end,
 {"NARRATOR", "The brush slipped", "from his hand."},
 {"NARRATOR", "Danny collapsed onto", "his last painting."},
 {"MARA", "He died with paint", "still on his hands."},
 {"NEUTRAL END", "The city carried", "his colors onward."});
SCRIPT(d_good_end,
 {"DANNY", "The canvas is dry.", "The music is safe."},
 {"YEARS LATER", "Danny was gone by", "the time it happened."},
 {"RECORD STORE", "His unreleased BORE", "songs sold millions."},
 {"RECORD STORE", "The games and album", "topped the charts."},
 {"MARA", "He never heard the", "crowds sing along."},
 {"GOOD END", "But the songs found", "somewhere to live."});
SCRIPT(d_day_notes,
 {"POCKET NOTEBOOK", "The bus can wait.", "The roof cannot."},
 {"POCKET NOTEBOOK", "Read, walk, paint.", "The day continues."});
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
 SEQ(d_note_visit), SEQ(d_note_followup), SEQ(d_note_canvas)
};
const StoryPage *story_pages(const Story *s, int *count) {
    unsigned id = s->dialogue;
    if (id >= sizeof(scripts) / sizeof(scripts[0])) id = 0;
    *count = scripts[id].count;
    return scripts[id].pages;
}
/* Each major interaction receives a world-grounded, animated shot sequence.
   The same script pages act as camera beats; no replay cutscene flags needed. */
static uint8_t cinematics_for_dialogue(uint8_t d){
    switch(d){
    case D_OPEN: return CINE_OPEN;
    case D_LETTER: case D_LETTER_AGAIN: return CINE_LETTER;
    case D_CLERK_LOCKED: case D_CLERK: case D_CLERK_AGAIN: return CINE_BORE;
    case D_MARA_LOCKED: case D_MARA: case D_MARA_AGAIN: return CINE_MARA;
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
    case D_DOCTOR_RETURN: return CINE_FOLLOWUP;
    case D_DOCTOR_LAST: return CINE_LAST_CLINIC;
    case D_GOOD_END: return CINE_GOOD;
    case D_NEUTRAL_START: return CINE_DAY;
    case D_NEUTRAL_PAINT: case D_NEUTRAL_END: return CINE_FINALE;
    default: return CINE_NONE;
    }
}
static void talk(Story *s, uint8_t id) {
    s->dialogue=id;s->page=0;
    s->cinema=cinematics_for_dialogue(id);s->cinema_frame=0;
}
void story_new(Story *s, int slot) {
    *s = (Story){0};
    s->x = 12; s->y = 13; s->prev_x = s->x; s->prev_y = s->y;
    s->facing = 3; s->slot = (uint8_t)(slot & 1);
    s->day_number = 1;
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
    if(s->ended && s->ending==STORY_END_GOOD){
        /* Replay the cinematic in sync with the one-shot score after loading. */
        s->finale_frame=0;s->dialogue=0;s->page=0;s->cinema=CINE_GOOD;
    } else if(s->ended && s->ending==STORY_END_NEUTRAL){
        s->finale_frame=0;s->dialogue=0;s->page=0;s->cinema=CINE_FINALE;
        s->life_stage=STORY_LIFE_FINALE;
    } else talk(s,D_RETURN);
}
/* Two attack waves with four clearly telegraphed speech-bubble hazards each.
   Players dodge LEFT/RIGHT in three projected lanes; A parries for six frames. */
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
        s->battle_parry = 6;
        s->battle_parry_cd = 18;
    }
    ++s->battle_timer;
    if (s->battle_timer % 15 == 0) {
        int beat = s->battle_timer / 15 - 1;
        if (s->battle_lane == story_attack_lane(s, beat) && !s->battle_parry) {
            if (s->battle_guard) s->battle_guard = 0;
            else if (s->battle_focus) --s->battle_focus;
        }
    }
    if (s->battle_timer >= 60) {
        if (!s->battle_focus) { cult_finish(s, -1); return; }
        if (++s->battle_progress >= 2) { cult_finish(s, 1); return; }
        s->battle = BATTLE_MENU;
        s->battle_choice = 0;
    }
}
static void interact(Story *s) {
    const int dx[] = {1,0,-1,0}, dy[] = {0,1,0,-1};
    char c = story_tile(s->x + dx[s->facing], s->y + dy[s->facing]);
    if (c != 'L' && c != 'C' && c != 'F' && c != 'B' && c != 'P' && c != 'V' && c != 'Q' && c != 'S' && c != 'W' && c != 'H' && c != 'R' && c != 'M' && c != 'K') {
        for (int d = 0; d < 4; ++d) {
            char a = story_tile(s->x + dx[d], s->y + dy[d]);
            if (a == 'L' || a == 'C' || a == 'F' || a == 'B' || a == 'P' || a == 'V' || a == 'Q' || a == 'S' || a == 'W' || a == 'H' || a == 'R' || a == 'M' || a == 'K') { c = a; break; }
        }
    }
    switch (c) {
    case 'L':
        talk(s, (s->flags & STORY_HOME) ? D_LETTER_AGAIN : D_LETTER);
        if (!(s->flags & STORY_HOME)) { s->flags |= STORY_HOME; s->dirty = 1; }
        break;
    case 'C':
        if (!(s->flags & STORY_HOME)) talk(s, D_CLERK_LOCKED);
        else {
            talk(s, (s->flags & STORY_STAMP) ? D_CLERK_AGAIN : D_CLERK);
            if (!(s->flags & STORY_STAMP)) { s->flags |= STORY_STAMP; s->dirty = 1; }
        }
        break;
    case 'F':
        if (!(s->flags & STORY_STAMP)) talk(s, D_MARA_LOCKED);
        else {
            talk(s, (s->flags & STORY_MARA) ? D_MARA_AGAIN : D_MARA);
            if (!(s->flags & STORY_MARA)) { s->flags |= STORY_MARA; s->dirty = 1; }
        }
        break;
    case 'B':
        talk(s, ((s->flags & STORY_ALL) == STORY_ALL && !s->life_stage) ? D_BUS_READY : D_BUS_LOCKED);
        break;
    case 'M':
        if (!(s->flags & STORY_MUSIC)) { s->flags |= STORY_MUSIC; s->dirty=1; talk(s,D_MUSIC); }
        else talk(s,D_MUSIC_AGAIN);
        break;
    case 'R':
        if (s->life_stage == STORY_LIFE_LAST_WORK) {
            s->life_stage=STORY_LIFE_FINALE;s->finale_frame=0;
            s->scene=STORY_SCENE_NONE;s->dirty=1;
            /* A timed in-scene subtitle replaces the blocking opening dialogue.
               The music and animation therefore start together even if nobody
               presses A for the next 72 seconds. */
            s->dialogue=0;s->page=0;s->cinema=CINE_FINALE;s->cinema_frame=0;
        } else if (s->life_stage == STORY_LIFE_FINALE) {
            /* The final cutscene is already playing: no restarting it. */
        } else if (s->life_stage >= STORY_LIFE_CLINIC) {
            talk(s,D_ART_LAST_LOCKED);
        } else if (s->life_stage == STORY_LIFE_WAIT) {
            talk(s,D_ART_DAY);
        } else if (!(s->flags & STORY_ART)) {
            s->flags |= STORY_ART;s->dirty=1;talk(s,D_ART);
        } else talk(s,D_ART_AGAIN);
        break;
    case 'K':
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
        if (s->cult_stage == 0) { s->cult_stage = 1; talk(s, D_CULT_FIND); s->dirty = 1; }
        else if (s->cult_stage == 3) { s->cult_stage = CULT_RESOLVED; talk(s, D_CULT_BACKUP); s->dirty = 1; }
        else talk(s, s->cult_stage >= CULT_RESOLVED ? D_CULT_AFTER :
                  s->cult_stage == 1 ? D_CULT_LOGS : D_CULT_WAIT);
        break;
    case 'W':
        if (s->cult_stage == 1) { s->cult_stage = 2; talk(s, D_CULT_WITNESS); s->dirty = 1; }
        else talk(s, s->cult_stage > 1 ? D_CULT_WITNESS_AFTER : D_CULT_NOT_YET);
        break;
    case 'H':
        if (s->cult_stage == 2) { s->cult_stage = 3; talk(s, D_CULT_SAFETY); s->dirty = 1; }
        else talk(s, D_CULT_SAFE_AGAIN);
        break;
    default: talk(s, D_NOTHING); break;
    }
}
void story_tick(Story *s, uint16_t keys) {
    /* Use the same key-bit convention as REG_KEYINPUT, after inversion. */
    enum { A=1, B=2, SELECT=4, START=8, RIGHT=16, LEFT=32, UP=64, DOWN=128, R=256, L=512 };
    uint16_t pressed = (uint16_t)(keys & ~s->old_keys);
    s->old_keys = keys;
    if (s->anim) --s->anim;
    if (s->cinema && !s->paused && s->cinema_frame < 65530) ++s->cinema_frame;
    /* One step is exactly one VBlank, including dialogue. Pause freezes the
       clock. Day transitions never steal controls during a fight or textbox. */
    if (s->life_stage >= STORY_LIFE_WAIT && s->life_stage <= STORY_LIFE_LAST_WORK && !s->paused) {
        if (++s->day_frame >= STORY_DAY_FRAMES) {
            s->day_frame=0;
            if (s->day_number<99) ++s->day_number;
            s->dirty=1;
        }
        if (!s->dialogue && !s->battle && !s->choosing) {
            if (s->life_stage==STORY_LIFE_WAIT && s->day_number>=2) {
                s->life_stage=STORY_LIFE_CLINIC;
                s->scene=STORY_SCENE_NOTICE;s->dirty=1;talk(s,D_MEDICAL_NOTICE);
            } else if (s->life_stage==STORY_LIFE_FOLLOWUP_WAIT && s->day_number>=3) {
                s->life_stage=STORY_LIFE_FINAL_CLINIC;
                s->scene=STORY_SCENE_NOTICE;s->dirty=1;talk(s,D_DOCTOR_RETURN);
            }
        }
    }
    /* Ending pictures are authored over the COMPLETE score, not just the
       original 450-frame animation. Keep one shot clock through the chorus,
       verse returns and the good-only coda. Input cannot accidentally skip
       the performance or leave the camera on a static credit card. */
    if (!s->dialogue && (
          (s->life_stage==STORY_LIFE_FINALE && !s->ended) ||
          (s->ended && s->ending==STORY_END_GOOD &&
           s->finale_frame<STORY_GOOD_FINAL_FRAMES))) {
        unsigned limit=s->ending==STORY_END_GOOD?
            STORY_GOOD_FINAL_FRAMES:STORY_END_SCORE_FRAMES;
        if (s->finale_frame<limit) ++s->finale_frame;
        if (s->finale_frame>=limit) {
            if (s->ending==STORY_END_GOOD)talk(s,D_GOOD_END);
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
            if ((pressed & A) && s->page + 1 < count) ++s->page;
            else {
                int ready = s->dialogue == D_BUS_READY && (pressed & A);
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
    if (s->choosing) {
        if (pressed & (LEFT|RIGHT|UP|DOWN)) s->choice ^= 1;
        if (pressed & B) { s->choosing = 0; s->cinema = CINE_NONE; }
        if (pressed & A) {
            s->choosing=0;
            if (s->choice==0 && (s->flags & STORY_FULL)==STORY_FULL) {
                s->ended=1;s->ending=STORY_END_GOOD;
                s->finale_frame=0;s->dialogue=0;s->page=0;
                s->cinema=CINE_GOOD;s->cinema_frame=0;
            } else if (s->choice==0) {
                /* Adequate progress, but not 100%: keep living and crafting.
                   The neutral ending is delayed by active, numbered game days. */
                s->life_stage=STORY_LIFE_WAIT;
                s->day_number=1;s->day_frame=0;
                talk(s,D_NEUTRAL_START);
            } else {
                /* STAY means continue exploring; it is not an instant ending. */
                talk(s,D_ART_DAY);
            }
            s->dirty=1;
        }
        return;
    }
    if (pressed & START) { s->paused ^= 1; return; }
    if (s->paused) { if (pressed & B) s->quit = 1; return; }
    if (pressed & (L|SELECT)) { s->view ^= 1; s->dirty = 1; }
    if (pressed & R) {
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
