#pragma once
/* Menu-only input debouncing. No GBA/MMIO dependency, enabling host tests. */
typedef struct {
    unsigned char was_a, was_start, was_left, was_right;
    unsigned char left_frames, right_frames;
} MenuNav;
typedef struct {
    unsigned char a_press, start_press, left_press, right_press;
} MenuNavEvents;

static inline void menu_nav_reset(MenuNav *s) {
    s->was_a = s->was_start = s->was_left = s->was_right = 1u;
    s->left_frames = s->right_frames = 0u;
}

/* left/right may combine LEFT/UP and RIGHT/DOWN; opposing holds cancel.
   First press moves immediately, then repeats after 18 held frames and
   every 8 frames thereafter. Counters cannot overflow during a long hold. */
static inline MenuNavEvents menu_nav_step(MenuNav *s,
            int a, int start, int left, int right) {
    MenuNavEvents e;
    e.a_press = (unsigned char)(a && !s->was_a);
    e.start_press = (unsigned char)(start && !s->was_start);
    e.left_press = (unsigned char)(left && !s->was_left);
    e.right_press = (unsigned char)(right && !s->was_right);
    if (left && !right) {
        s->right_frames = 0;
        s->left_frames = (unsigned char)(s->left_frames == 27u ? 20u : s->left_frames + 1u);
        if (s->left_frames == 19u || s->left_frames == 27u)
            e.left_press = 1u;
    } else if (right && !left) {
        s->left_frames = 0;
        s->right_frames = (unsigned char)(s->right_frames == 27u ? 20u : s->right_frames + 1u);
        if (s->right_frames == 19u || s->right_frames == 27u)
            e.right_press = 1u;
    } else {
        s->left_frames = s->right_frames = 0;
        if (left && right)
            e.left_press = e.right_press = 0u;
    }
    s->was_a = (unsigned char)!!a;
    s->was_start = (unsigned char)!!start;
    s->was_left = (unsigned char)!!left;
    s->was_right = (unsigned char)!!right;
    return e;
}
