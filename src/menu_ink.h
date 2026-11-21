#pragma once
/* Deliberate, uninterrupted erase confirmation on the actual save slot.
   Feedback codes are also the native 240x160 menu's printed captions. */
#define MENU_ERASE_HOLD 40u
enum { MENU_NOTE_NONE, MENU_NOTE_EMPTY, MENU_NOTE_ERASED };
unsigned menu_erase_step(unsigned held,int new_press,int down);
