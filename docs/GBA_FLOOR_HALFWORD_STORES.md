# Danny Steel: native 16-bit floor pixel pairs

## Objective

The current playable GBA release presents about **7.167 page flips/sec** during a 600-video-frame first-person Potheads walking benchmark, with 14.932 stationary first-person FPS. The source-matched ARM moving renderer spends the majority of time filling the original 240x160 floor.

## Graphics-identical trial

The GBA shares a 38,400-byte Mode-4 pixel buffer (8-bit palette indices) in EWRAM. When two adjacent native pixels belong to the same unoccluded ray span and sit at an aligned even X offset, the new floor pixel painter can combine their original shade indices into one 16-bit EWRAM store instead of two byte stores. There is no new framebuffer, half-resolution scaling, dropped ray, texture change or sprite hiding.

For near-floor (native-rate) rendering, an even pixel temporarily holds its computed palette color; the immediately following odd pixel completes a single halfword write. If a ray begins on an odd column or ends on an even column, the original single-byte write remains. For far-floor (two-pixel shading), the original matching pair is written as one halfword when both pixels belong to the same original ray. Odd ray starts retain their original single-pixel spill behavior. The original 45 FOV ray runs and exact wall-depth visibility decisions are unchanged.

The renderer checks the framebuffer's halfword alignment once; callers supplying an unaligned host canvas fall back to the original two byte writes. The only scratch value is one local palette index; the shader continues computing every required original material sample.

## Release gates

- 7,000 independent paired pixel tests across full-frame guard sentinels, odd/even backing-buffer alignment, 7 screen rows, 10 representative even X addresses and 25 low/high palette pairs
- 3,456 independently recorded original complete 240x160 first-person golden images, 120 cold/warm shade cache comparisons, 61,440 exact road material checks, original wall depth/sprite visibility tests
- All story, endings, Studio, cinema, music, SRAM compatibility, actor and input replay tests
- ARM GBA compiler/linker EWRAM/IWRAM budgets, source-matched moving ARM stage cycles, real ROM screenshots, and held-walking page-flip cadence, compared to **7.167 FPS walking** and **14.932 FPS standing**

A lower or unchanged page-flip rate alone does not justify adding rendering complexity. Reject the experiment if the new branches outweigh the EWRAM store savings, if any golden pixels differ, or if the original gameplay/memory gates regress.
