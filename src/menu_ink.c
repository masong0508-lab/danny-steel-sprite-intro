#include "menu_ink.h"
/* The A press that opened the ERASE submenu cannot carry into erasure. */
unsigned menu_erase_step(unsigned held,int new_press,int down){
 if(!down)return 0u;
 if(!held)return new_press?1u:0u;
 return held<MENU_ERASE_HOLD?held+1u:MENU_ERASE_HOLD;
}
