# GBA first-person wall painter: aligned paired-byte shading

## Goal
The measured first-person ARM renderer spends approximately 899,630 cycles per frame in the wall/depth stage (about 64% of renderer time) on the standard mGBA Potheads route. More exact ray marching, floor arithmetic and sprite clipping have not yet crossed the six-VBlank presentation interval. This patch targets the wall painter's **memory writes** rather than sacrificing texture detail.

## Implementation
The existing `wall_column` determines a single palette value for a 1–11 pixel-wide horizontal row of a facade or tree wall and then writes every pixel individually. A small inlined routine now duplicates that color into aligned 16-bit EWRAM halfword stores. It writes an optional unaligned 8-bit head and optional 8-bit tail, while packing the middle pairs. Alignment, pair count and tail count are computed **once per wall column**, since each screen stride is 240 bytes (even).

No pixel colors, wall heights, roofline highlights, material/texture hashes, windows, map collision, actor occlusion, camera projection, foreground silhouettes, audio, SRAM or narrative logic change. Normal release code uses the same 38,400-byte EWRAM frame and exact 240×160 Mode-4 image, with no heap or new framebuffer. Profiling-only counters are inherited from the separate profiler prerequisite and excluded from ordinary builds.

## Validation
A host-only wrapper invokes the exact optimized row store with varied base alignment (0–3), row positions including 0/159, widths 1–11, edge x coordinates and five color inks. More than 6,000 cases compare **every byte** of a full guarded frame against an independent pixel-by-pixel reference. The independently recorded **3,456 complete first-person golden frames** and 136,140 floor/wall-depth inequalities must also pass. Full story, UI, cinema, save, music, GBA linker budget, mGBA screenshot and input/clock gates remain mandatory.

The GBA ARM cycle-profiler workflow must report actual wall-paint and wall/depth cycles, and the production-ROM cadence workflow must show whether this reduces the six-VBlank first-person interval at the representative scene. Lower cycle counts are useful evidence, but **not** a claim of increased FPS unless the page-flip report demonstrates it. This is a stacked performance experiment until all checks pass.
