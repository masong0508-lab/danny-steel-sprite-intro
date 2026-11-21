#include "menu_ink.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 assert(menu_erase_step(0u,0,1)==0u);
 assert(menu_erase_step(0u,1,1)==1u);
 unsigned h=0u;
 for(unsigned i=0;i<MENU_ERASE_HOLD;i++)
  h=menu_erase_step(h,i==0u,1);
 assert(h==MENU_ERASE_HOLD);
 assert(menu_erase_step(h,0,1)==MENU_ERASE_HOLD);
 assert(menu_erase_step(h/2u,0,0)==0u);
 assert(menu_erase_step(0u,0,1)==0u);
 puts("erase: new press required, 40-frame continuous hold and cancellation OK");
 return 0;
}
