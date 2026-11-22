# Native first-person painter: sky bands and actor visibility roster

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

## Fast fixed-actor roster

The previous native first-person renderer visited all 432 tiles in the 24x18 story grid every frame to discover only 13 persistent actors and interactive landmarks. It now walks a 13-entry immutable cartridge-ROM `SceneAnchor` table, ordered identically to the original row-major grid. Exact geometry, billboard projection, z-depth sorting, occlusion, sprites, quest positions and appearance stay unchanged.

A new host-only map validator enumerates the real story tiles and checks that each relevant actor/landmark appears exactly once in the ROM list at the correct row and column. It is excluded from the GBA binary. The existing independently recorded 3,456-frame native perspective golden-image test still validates all 4 facings, position interpolation and weather ticks, ensuring the same picture as the previous renderer. No extra RAM, global state, framebuffer or per-frame heap use.

This change can reduce CPU work; only completed mGBA cadence runs may quantify the actual display-rate effect.