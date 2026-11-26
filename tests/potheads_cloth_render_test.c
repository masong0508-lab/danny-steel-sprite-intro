#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "potheads_cloth_render.h"
int main(void){
    PotClothRect r;
    assert(pot_cloth_rect(&r,10,20,128|17,256,48,256,0,128));
    assert(r.x==10&&r.y==20&&r.w==17&&r.h==1&&r.lit);
    assert(!pot_cloth_rect(&r,239,134,2,256,48,256,0,128));
#if POTHEADS_CLOTH_ENABLED
    assert(POT_CLOTH_FRAMES==548&&pot_cloth_tick[0]==0&&pot_cloth_tick[547]==1093);
    assert(pot_cloth_off[0]==0&&pot_cloth_off[548]==POT_CLOTH_BYTES);
    unsigned valid=0,weak=0,total=0,peak=0;
    for(unsigned f=0;f<POT_CLOTH_FRAMES;f++){
        unsigned a=pot_cloth_off[f],b=pot_cloth_off[f+1];
        assert(a<=b&&b<=POT_CLOTH_BYTES&&(b-a)%3==0);
        if(f)assert(pot_cloth_tick[f]>pot_cloth_tick[f-1]);
        if(a==b){weak++;assert(pot_source_cloth_frame(pot_cloth_tick[f])==-1);continue;}
        valid++;assert(pot_source_cloth_frame(pot_cloth_tick[f])==(int)f);
        if((b-a)/3>peak)peak=(b-a)/3;
        unsigned char seen[135][240]={{0}};
        for(unsigned k=a;k<b;k+=3){
            int y=pot_cloth_spans[k],x=pot_cloth_spans[k+1],p=pot_cloth_spans[k+2];
            assert(pot_cloth_rect(&r,x,y,p,256,48,256,0,128));
            assert(r.x==x&&r.y==y&&r.w==(p&127)&&r.h==1);
            for(int xx=r.x;xx<r.x+r.w;xx++){assert(!seen[y][xx]);seen[y][xx]=1;total++;}
            if(pot_cloth_rect(&r,x,y,p,160,225,56,40,88))assert(r.x>=0&&r.y>=0&&r.x+r.w<=240&&r.y+r.h<=135);
        }
    }
    assert(valid>=530&&weak<=18&&peak<=240);
    assert(pot_source_cloth_frame(9999)==547);
    printf("548 source frames: %u visible contours, %u fallback frames, %u spans maximum, %u painted pixels\n",valid,weak,peak,total);
#else
    assert(pot_source_cloth_frame(0)==-1&&pot_source_cloth_frame(1093)==-1);
    puts("Source-only fallback passed");
#endif
    return 0;
}
