# GBA walking FPS: packed exact road-material ROM

## Measured bottleneck

The source-matched ARM/mGBA moving-camera profile measured 1,389,672 CPU cycles (82.831 ms) per first-person frame spent in floor/material rendering, against 312,273 in wall/depth and 204,725 in sky. The existing 6.75 KiB stationary-wall cache improved unchanged-camera FPS to 14.932, but walking remained 6.570 presented frames/sec. The attempted per-scanline road texel-cache in PR #36 and incremental-projection follow-up #37 worsened walking to 6.271, so both were rejected.

## Exact road material without per-pixel procedural hashes

The original 24x18 town map has **240 road tiles**. Each physical 256x256 road tile samples the 32-bit deterministic procedural hash at a 16-unit grid (16x16 shade entries). Only three output categories are needed: REFLECT, ROAD_LIGHT, or ordinary road.

The build-generated cartridge-only array stores **240 × 16 × 16 × 2 bits = 15,360 bytes**, plus **432 road-tile index bytes**. A second **400-byte ROM dither mask** represents the original screen-space 17-pixel predicate. Total additional ROM data is **16,192 bytes**; **no new EWRAM, IWRAM, framebuffer, tile atlas in RAM, heap or save data**.

At each floor map tile boundary, the renderer already reads the original story_tile(tx,ty); if that tile is a road, it resolves its small road table index once. Each native floor texel then uses one packed ROM category rather than recalculating the old multiplications. Rare non-road materials retain the original shader, and curb seams keep the original priority. No palette values, pixel positions, field of view, wall projection, image resolution or story mechanics change.

## Verification and release gates

- **61,440** independently recomputed original road material categories (all cells in all 240 road tiles), plus 49,152 non-road tile guard checks.
- **3,456 complete original 240x160 golden first-person frames** at varied poses, weather and animations; 120 wall-shade cold/warm image checks and existing scene/draw guards.
- ARM7 GBA devkitARM ROM compile and actual IWRAM/EWRAM linker sizes; real ROM mGBA short-input/clock tests; complete story, rescue, battle, Studio, save and cutscene tests.
- Native ARM stage profiler's held-walking report must be compared against the **1,389,672 floor cycles/frame** baseline, and the original 600-video-frame held back/forward movement benchmark against **6.570 presented frames/sec**. Also retain the **14.932 stationary** rate.

This is a memory-conscious ROM/CPU tradeoff, not a visually lower-detail rendition. It is experimental until the actual emulated GBA frame rate improves. If the packed lookups are slower, preserve the already validated main ROM and reject the branch.
