#include <assert.h>
#include <stdio.h>
#include "potheads_drape.h"
int main(void){
 assert(pot_drape_weight(256)==0);assert(pot_drape_weight(-256)==256);
 assert(pot_drape_weight(0)==128);
 int prev=pot_drape_weight(-256);
 for(int y=-255;y<=256;y++){int next=pot_drape_weight(y);assert(next<=prev&&prev-next<=1);prev=next;}
 assert(pot_drape_mix(10,30,0)==10);assert(pot_drape_mix(10,30,256)==30);
 assert(pot_drape_mix(10,30,128)==20);assert(pot_drape_mix(9,-4,128)==2);
 assert(pot_drape_mix(-20,-10,128)==-15);
 puts("Potheads drape: continuous transition and exact endpoints passed");
}
