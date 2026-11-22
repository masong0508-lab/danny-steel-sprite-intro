# Native first-person sky-band painter

The first-person renderer runs on the GBA ARM7TDMI and shares the existing 240x160 Mode-4 buffer. Prior to this change, the flat base behind the raycast city issued 160 separate `hd_rect` calls, each painting one scanline. The color changes only six times.

This pass replaces that loop with exactly seven contiguous, full-width pixel bands matching the original per-row color conditions:

| Native rows | Palette |
| --- | --- |
| 0–23 | 19 |
| 24–53 | 17 |
| 54–73 | HD_GLASS |
| 74–83 | HD_GLASS_LIT |
| 84–101 | HD_ROAD_COOL |
| 102–124 | HD_ROAD_LIGHT |
| 125–159 | HD_ROAD |

This produces the same base image, keeps skyline/building/rain layers unchanged, and allows the existing optimized contiguous word-fill path to draw the background without hundreds of independent call/setup operations.

Release gate: the existing exact 3456-frame first-person golden-image test, row and pixel-safety checks, all narrative/tests, true ARM ROM build and mGBA screenshot capture must pass. An expected reduction in draw overhead is not a measured physical-console FPS claim. No extra RAM, heap, new buffers, palette entries, data files, collision changes or save changes.