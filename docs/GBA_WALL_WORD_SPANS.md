# First-person wall painter: aligned 32-bit color spans

## Context

The source-matched ARM GBA profiler measured 839,567 cycles per sampled first-person frame spent painting wall texture. The established 16-bit halfword painter reduced that to 774,004 cycles while preserving 3,456 golden camera frames and SRAM/clock integrity, but the standard mGBA scene still presented a page every six VBlanks.

## Candidate

A wall ray shades a constant color across 1–11 adjacent Mode-4 pixels for each row. This change upgrades from two-pixel stores to up to four identical palette pixels per aligned 32-bit EWRAM word. Alignment is planned once per wall column, not for every row: an optional leading byte, optional leading 16-bit halfword, contiguous aligned 32-bit words, optional trailing halfword and trailing byte. With the unchanged even 240-pixel framebuffer stride, the same plan applies to every shaded row in that wall ray.

The optimization is not an extra framebuffer, image asset, shader pass or low-resolution upscale. Original UV texture shading, brick/stone/window art, Potheads lettering, tree surfaces, foreground actor occlusion, story map, saves, music and the palette remain unchanged.

## Validation

- Host-only exact-row writer vs independent byte-loop reference at thousands of combinations of frame-base alignment, x/y edges, widths 1–11 and ink 0–63, with 64-byte sentinels around the full screen
- Existing 3,456 complete first-person image golden tests and 136,140 depth-boundary comparisons
- Complete story, cinematics, Studio, UI, save migration, GBA linker/RAM tests and real-ROM mGBA screenshots/runtime probes
- Separate profiling-only ARM cycle measurements for wall paint, wall+ray and complete first-person renderer, followed by the original 600-video-frame page-flip measurement

This branch is an experiment until its own measured cycles and native GBA/screenshot checks pass. If it is slower than the verified 16-bit spans, retain the faster original; do not claim faster FPS based on a host microbenchmark.
