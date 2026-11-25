#pragma once
/* 42 read-only native illustrations; no decoder or new framebuffer.
   Build from the repository root (GNU assembler .incbin). */
#define HD_NEIGHBORHOOD_PANEL_BYTES (240u*104u)
#define HD_NEIGHBORHOOD_BYTES (STORY_NEIGHBORHOOD_COUNT*3u*HD_NEIGHBORHOOD_PANEL_BYTES)
extern const uint8_t hd_neighborhood_art[HD_NEIGHBORHOOD_BYTES];
__asm__(
 ".pushsection .rodata.neighborhood,\"a\"\n"
 ".balign 4\n"
 ".global hd_neighborhood_art\n"
 "hd_neighborhood_art:\n"
 ".incbin \"assets/neighborhood/00.bin\"\n"
 ".incbin \"assets/neighborhood/01.bin\"\n"
 ".incbin \"assets/neighborhood/02.bin\"\n"
 ".incbin \"assets/neighborhood/03.bin\"\n"
 ".incbin \"assets/neighborhood/04.bin\"\n"
 ".incbin \"assets/neighborhood/05.bin\"\n"
 ".incbin \"assets/neighborhood/06.bin\"\n"
 ".incbin \"assets/neighborhood/07.bin\"\n"
 ".incbin \"assets/neighborhood/08.bin\"\n"
 ".incbin \"assets/neighborhood/09.bin\"\n"
 ".incbin \"assets/neighborhood/10.bin\"\n"
 ".incbin \"assets/neighborhood/11.bin\"\n"
 ".incbin \"assets/neighborhood/12.bin\"\n"
 ".incbin \"assets/neighborhood/13.bin\"\n"
 ".popsection\n");
