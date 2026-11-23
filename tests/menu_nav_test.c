#include "menu_nav.h"
#include "menu_ink.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    MenuNav m;
    menu_nav_reset(&m);
    /* A button already down when entering the menu must not activate it. */
    MenuNavEvents e = menu_nav_step(&m, 1, 1, 1, 0);
    assert(!e.a_press && !e.start_press && !e.left_press);
    e = menu_nav_step(&m, 0, 0, 0, 0);
    assert(!e.a_press && !e.start_press);
    e = menu_nav_step(&m, 0, 0, 1, 0);
    assert(e.left_press && !e.right_press);
    for (unsigned f = 2; f <= 1000; ++f) {
        e = menu_nav_step(&m, 0, 0, 1, 0);
        assert(e.left_press == (f >= 19 && ((f - 19u) % 8u == 0u)));
        assert(!e.right_press);
    }
    /* Opposite directions must stop repeating rather than drifting. */
    for (int i = 0; i < 40; ++i) {
        e = menu_nav_step(&m, 0, 0, 1, 1);
        assert(!e.left_press && !e.right_press);
    }
    e = menu_nav_step(&m, 0, 0, 0, 1);
    assert(!e.left_press && !e.right_press);
    e = menu_nav_step(&m, 0, 0, 0, 0);
    e = menu_nav_step(&m, 0, 0, 0, 1);
    assert(e.right_press && !e.left_press);
    e = menu_nav_step(&m, 0, 1, 0, 1);
    assert(e.start_press && !e.a_press);
    assert(menu_erase_step(0u, e.a_press, 0) == 0u);
    for (int f = 0; f < 60; ++f) {
        e = menu_nav_step(&m, 0, 1, 0, 0);
        assert(!e.start_press && !e.a_press);
    }
    e = menu_nav_step(&m, 1, 1, 0, 0);
    assert(e.a_press && !e.start_press);
    unsigned erase = menu_erase_step(0u, e.a_press, 1);
    for (unsigned f = 1; f < MENU_ERASE_HOLD; ++f) {
        e = menu_nav_step(&m, 1, 1, 0, 0);
        assert(!e.a_press && !e.start_press);
        erase = menu_erase_step(erase, e.a_press, 1);
    }
    assert(erase == MENU_ERASE_HOLD);
    e = menu_nav_step(&m, 0, 0, 0, 0);
    assert(menu_erase_step(erase, e.a_press, 0) == 0u);
    menu_nav_reset(&m);
    e = menu_nav_step(&m, 0, 0, 0, 0);
    e = menu_nav_step(&m, 0, 0, 0, 1);
    assert(e.right_press);
    puts("menu: directional repeat, cancellation, START/A and erase safeguards OK");
    return 0;
}
