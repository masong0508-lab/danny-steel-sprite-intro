#include "story.h"
#include "story_draw.h"
#include <stdio.h>
#include <stdint.h>
static uint8_t pixels[120*80];
int main(void){
  Story s;story_new(&s,0);s.dialogue=0;story_draw_init();
  for(int view=0;view<2;view++){
    s.view=(uint8_t)view; story_draw(&s,30,pixels);
    char path[80];sprintf(path,"preview_%d.ppm",view);
    FILE *f=fopen(path,"wb");if(!f)return 1;
    fprintf(f,"P6\n120 80\n255\n");
    for(int i=0;i<120*80;i++){
       uint16_t c=story_palette[pixels[i]&31];
       uint8_t rgb[3]={(uint8_t)((c&31)*255/31),(uint8_t)(((c>>5)&31)*255/31),(uint8_t)(((c>>10)&31)*255/31)};
       fwrite(rgb,1,3,f);
    }fclose(f);
  }
  puts("rendered map and 3D previews");return 0;
}
