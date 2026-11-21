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
    return town[y][x];
}
int story_solid(int x, int y) {
    const char c = story_tile(x, y);
    return c == '#' || c == 'A' || c == 'T' || c == '~' || c == 'L' || c == 'C' || c == 'F' || c == 'B';
}

enum { D_NONE, D_OPEN, D_LETTER, D_LETTER_AGAIN, D_CLERK_LOCKED, D_CLERK, D_CLERK_AGAIN,
       D_MARA_LOCKED, D_MARA, D_MARA_AGAIN, D_BUS_LOCKED, D_BUS_READY, D_END_GO,
       D_END_STAY, D_JOURNAL_0, D_JOURNAL_1, D_JOURNAL_2, D_JOURNAL_3, D_NOTHING, D_RETURN };
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
#undef SCRIPT
#define SEQ(name) {name, (uint8_t)(sizeof(name) / sizeof((name)[0]))}
static const struct { const StoryPage *pages; uint8_t count; } scripts[] = {
 {0,0}, SEQ(d_open), SEQ(d_letter), SEQ(d_letter_again), SEQ(d_clerk_locked),
 SEQ(d_clerk), SEQ(d_clerk_again), SEQ(d_mara_locked), SEQ(d_mara), SEQ(d_mara_again),
 SEQ(d_bus_locked), SEQ(d_bus_ready), SEQ(d_end_go), SEQ(d_end_stay),
 SEQ(d_journal_0), SEQ(d_journal_1), SEQ(d_journal_2), SEQ(d_journal_3),
 SEQ(d_nothing), SEQ(d_return)
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
static void interact(Story *s) {
    const int dx[] = {1,0,-1,0}, dy[] = {0,1,0,-1};
    char c = story_tile(s->x + dx[s->facing], s->y + dy[s->facing]);
    if (c != 'L' && c != 'C' && c != 'F' && c != 'B') {
        for (int d = 0; d < 4; ++d) {
            char a = story_tile(s->x + dx[d], s->y + dy[d]);
            if (a == 'L' || a == 'C' || a == 'F' || a == 'B') { c = a; break; }
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
    default: talk(s, D_NOTHING); break;
    }
}
void story_tick(Story *s, uint16_t keys) {
    /* Use the same key-bit convention as REG_KEYINPUT, after inversion. */
    enum { A=1, B=2, SELECT=4, START=8, RIGHT=16, LEFT=32, UP=64, DOWN=128, R=256, L=512 };
    uint16_t pressed = (uint16_t)(keys & ~s->old_keys);
    s->old_keys = keys;
    if (s->anim) --s->anim;
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
