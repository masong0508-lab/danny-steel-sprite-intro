#pragma once
/* GBA game screen. Load/save share persistent locations between map and 3D. */
int play_slot_valid(int slot);
void play_erase_slot(int slot);
void play_enter(int want_load, int slot);
int play_update(void);              /* 1 = save & return to main menu */
