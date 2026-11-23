#pragma once
/* Old Quarter illustration at the GBA's native 240x160.
   D-pad LEFT/RIGHT or UP/DOWN chooses; A/START opens; B backs out.
   The destructive ERASE option still requires a fresh, continuous hold. */
#include "gba.h"
void menu_enter(void);
int menu_update(void); /* 0 continue, 1 title, 2/3 new, 4/5 load */
