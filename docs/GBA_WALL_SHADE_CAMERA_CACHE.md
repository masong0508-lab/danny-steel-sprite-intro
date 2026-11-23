# First-person GBA: compact wall-shading memo by camera pose

## Why this is a different performance milestone

The instrumented ARM renderer spends roughly **839,567 cycles/frame** shading and painting wall rays in the original baseline, and **774,004 cycles** after PR #30's pixel-identical 16-bit EWRAM writes. Recent ray-step, floor and shading-branch changes reduced work without changing the measured six-VBlank (9.955 pages/s) sample. The fixed Old Quarter world has no destructible walls. When Danny is stationary and looking the same way, opaque wall material colors remain identical across frames even while rain and skyline lighting advance.

## Implementation

- Cache only **45 quantized wall-ray shade columns × 150 original vertical palette samples** = **6,750 extra bytes of EWRAM**. This is not a second 38,400-byte framebuffer, RGB image, texture atlas, heap allocation or IWRAM executable allocation.
- Cold draw: original renderer traces and shades every wall pixel at native resolution, storing the computed 8-bit material ink in its corresponding ray's bounded shade strip. This adds a small cost to a newly reached camera pose.
- Warm draw: if the projected camera position and facing direction are identical, redraw exactly those cached wall inks through the already validated 16-bit clipped span writer. Wall depth and texture metadata are still recomputed for correct floor and actor occlusion.
- Movement, turning, animation/interpolation and changed camera position invalidate the cache; no image shifting, sprite hiding or fake depth. Sky, rain, music, actors, windows and foreground overlays continue rendering normally each frame; the existing wall_column still handles foreground restoration.
- Cache is indexed by the exact sampled ray group and refilled on misses; bounds are 45 rays and 150 palette samples per strip.

## GBA memory and image contract

This is an intentional performance/memory trade-off, **not memory-free**. The EWRAM allocation grows by 6.75 KiB, while the native Mode-4 shared frame remains 38.4 KiB. Require the devkitARM linked EWRAM gate (196,608 bytes), total IWRAM gate (24,576 bytes) and unchanged palette. No change to 24×18 collision, quest state, SRAM, original PSG audio clock, sprites or cinematic timing.

## Acceptance

- All **3,456 independently recorded first-person golden frames** still match the shipped renderer.
- Additional **120 complete cold/warm 240×160 frame comparisons**, five town positions, four facings, three clock phases, unchanged and changed weather ticks.
- Full graphics guard/sanitizer, story/rescue/ending, input clock and save migration tests; native GBA build and emulator screenshots.
- The **source-matched ARM cycle profiler**, followed by the actual ROM's **600-video-frame page-flip cadence**. The existing baseline is 9.955 first-person page flips/s, 14.932 overhead and 19.810 opening. Do not call this an FPS breakthrough until new mGBA results demonstrate it, and do not imply idle-camera savings automatically apply while walking or turning.

If CPU/GBA memory or image gates fail, reject/revert this candidate instead of lowering resolution or accepting visible artifacts. Any future movable/destructible architecture must explicitly invalidate the cache.
