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
    return town[y][x];
}
int story_solid(int x, int y) {
    const char c = story_tile(x, y);
    return c == '#' || c == 'A' || c == 'T' || c == '~' || c == 'L' || c == 'C' || c == 'F' || c == 'B' || c == 'P' || c == 'V' || c == 'Q' || c == 'S' || c == 'W' || c == 'H';
}

enum { D_NONE, D_OPEN, D_LETTER, D_LETTER_AGAIN, D_CLERK_LOCKED, D_CLERK, D_CLERK_AGAIN,
       D_MARA_LOCKED, D_MARA, D_MARA_AGAIN, D_BUS_LOCKED, D_BUS_READY, D_END_GO,
       D_END_STAY, D_JOURNAL_0, D_JOURNAL_1, D_JOURNAL_2, D_JOURNAL_3, D_NOTHING, D_RETURN,
       D_CULT_FIND, D_CULT_LOGS, D_CULT_WAIT, D_CULT_WITNESS, D_CULT_SAFETY,
       D_CULT_BACKUP, D_CULT_AFTER, D_CULT_NOT_YET, D_CULT_WITNESS_AFTER,
       D_CULT_SAFE_AGAIN, D_CULT_PRE_WIN, D_CULT_DURING_WIN, D_CULT_POST_WIN,
       D_CULT_RUN, D_CULT_LOSE, D_CULT_NOTES_1, D_CULT_NOTES_2,
       D_CULT_NOTES_3, D_CULT_NOTES_4 };
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
 SEQ(d_cult_notes_4)
};
const StoryPage *story_pages(const Story *s, int *count) {
    unsigned id = s->dialogue;
    if (id >= sizeof(scripts) / sizeof(scripts[0])) id = 0;
    *count = scripts[id].count;
    return scripts[id].pages;
}
static void talk(Story *s, uint8_t id) { s->dialogue = id; s->page = 0; }
void story_new(Story *s, int slot) {
    *s = (Story){0};
    s->x = 12; s->y = 13; s->prev_x = s->x; s->prev_y = s->y;
    s->facing = 3; s->slot = (uint8_t)(slot & 1);
    talk(s, D_OPEN); s->dirty = 1;
}
void story_resume(Story *s, int slot, int x, int y, int facing, int view, int flags, int ended, int ending) {
    story_new(s, slot);
    if (!story_solid(x,y)) { s->x = (uint8_t)x; s->y = (uint8_t)y; }
    s->prev_x = s->x; s->prev_y = s->y;
    s->facing = (uint8_t)(facing & 3); s->view = (uint8_t)(view & 1);
    s->flags = (uint8_t)(flags & STORY_ALL);
    s->ended = (uint8_t)(!!ended); s->ending = (uint8_t)(ending & 1);
    s->dirty = 0; talk(s, D_RETURN);
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
    if (c != 'L' && c != 'C' && c != 'F' && c != 'B' && c != 'P' && c != 'V' && c != 'Q' && c != 'S' && c != 'W' && c != 'H') {
        for (int d = 0; d < 4; ++d) {
            char a = story_tile(s->x + dx[d], s->y + dy[d]);
            if (a == 'L' || a == 'C' || a == 'F' || a == 'B' || a == 'P' || a == 'V' || a == 'Q' || a == 'S' || a == 'W' || a == 'H') { c = a; break; }
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
        talk(s, ((s->flags & STORY_ALL) == STORY_ALL) ? D_BUS_READY : D_BUS_LOCKED);
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
    /* Combat takes priority over exploration, so toggling 2D/3D cannot evade a hit. */
    if (s->battle) { cult_tick(s, pressed); return; }
    if (s->dialogue) {
        if (pressed & (A|B)) {
            int count; story_pages(s, &count);
            if ((pressed & A) && s->page + 1 < count) ++s->page;
            else {
                int ready = s->dialogue == D_BUS_READY && (pressed & A);
                s->dialogue = 0; s->page = 0;
                if (ready) { s->choosing = 1; s->choice = 0; }
            }
        }
        return;
    }
    if (s->choosing) {
        if (pressed & (LEFT|RIGHT|UP|DOWN)) s->choice ^= 1;
        if (pressed & B) s->choosing = 0;
        if (pressed & A) {
            s->ending = s->choice; s->ended = 1; s->choosing = 0;
            talk(s, s->ending ? D_END_STAY : D_END_GO); s->dirty = 1;
        }
        return;
    }
    if (pressed & START) { s->paused ^= 1; return; }
    if (s->paused) { if (pressed & B) s->quit = 1; return; }
    if (pressed & (L|SELECT)) { s->view ^= 1; s->dirty = 1; }
    if (pressed & R) {
        if (s->cult_stage) {
            talk(s, (uint8_t)(D_CULT_NOTES_1 + s->cult_stage - 1));
            return;
        }
        talk(s, s->flags == STORY_ALL ? D_JOURNAL_3 :
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
