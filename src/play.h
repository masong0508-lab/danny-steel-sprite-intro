#pragma once
/* GBA game screen. Load/save share persistent locations between map and 3D. */
int play_slot_valid(int slot);
void play_erase_slot(int slot);
void play_enter(int want_load, int slot);
/* Drain captured video-time input, then render when a hidden page is free.
   Call from the main loop; do not gate this on polling REG_VCOUNT. */
int play_update(void);              /* 1 = save & return to main menu */

