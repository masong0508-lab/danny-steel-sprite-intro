# GBA First-Person Performance Pipeline — Combined Candidate

## Objective
Improve the real 240×160 first-person page-flip cadence **without** hiding scenery, lowering internal resolution, corrupting SRAM, changing the existing town, or adding a second framebuffer. Three individually pixel-proven and compiler-verified optimizations are combined on one branch to test whether they cross the emulator's next VBlank interval.

## Changes
1. **Exact sampled grid ray traversal.** Each of 45 native quantized ray directions advances to the first original 32-unit sample in the next map tile, rather than iterating through every empty 32-unit step. A 66-byte ROM reciprocal table implements the exact 1..32 denominator ceiling quotient without ARM7 software division in the hot loop.
2. **Incremental floor projection.** Each visible scanline/ray run advances the original signed 24.8 side offset by addition. Forward camera coordinates are hoisted once per row; all texture UVs and negative-side rounding stay byte-identical.
3. **Billboard-limited wall restoration.** Occluded wall pixels are repainted only between a billboard's projected top and grounded shadow margin. A clipped texture DDA starts at the exact original `floor(offset*256/height)` value and remainder, without repainting untouched wall columns.

## Regression and release gates
The combined host test proves **49,216** signed crossing scenarios, **1,476,000** independent fixed-point sample comparisons, and **3,456** complete first-person golden images across positions, facings, animation phases and weather. The 136,140 existing wall/floor depth inequalities, story interaction/pathfinding, combat, music, saves, video/cinematic clocks, overlay clipping and player visibility remain mandatory. The actual ARM build must pass linker/static-IWRAM limits, and the mGBA workflow must pass ROM runtime and screenshot checks before any merge.

## Measured acceptance criteria
The recent production baseline on the standard 600-video-frame mGBA route is **9.955 first-person page flips/s**, 14.932 overhead town and 19.810 opening. Individually, the three optimizations retained pixel identity but did not move the 9.955 figure. The combined pipeline will only be promoted as an **FPS breakthrough** if a fresh production-ROM cadence report demonstrates a higher displayed rate, preferably five instead of six video frames per completed first-person page, with no material regression to overhead or opening. Physical GBA timing remains unverified. A faster host microbenchmark does **not** establish an FPS win.

If the combined candidate stays at 9.955, retain it as an experimental branch and prioritize profiling the native first-person floor/material cost rather than sacrificing image quality or accepting invented performance results.
