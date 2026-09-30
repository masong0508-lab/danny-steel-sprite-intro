// Host test: dumps the OAM image for every frame of every scene (real intro_build code).  Used by tools/render_intro.py
#include <stdio.h>
#include "../src/intro.h"
int main(void) {
    static u16 oam[512];
    for (int s = 0; s < INTRO_SCENES; s++) for (int t = 0; t < intro_scenes[s].frames || t < 1; t++) {
        intro_build(&intro_scenes[s], t, oam); fwrite(oam, 2, 512, stdout); if (intro_scenes[s].frames <= 2 && t >= 0) break;
    }
    return 0;
}
