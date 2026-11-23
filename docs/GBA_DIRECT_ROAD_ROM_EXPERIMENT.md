# Direct ROM road shade experiment — space versus moving FPS

## Starting point

The current verified GBA release improves held-walking first-person presentation from 6.570 to **7.068 page changes/sec** while preserving the independently captured 3,456-frame 240x160 golden-image reference. It stores exact original road noise in 15,360 bytes of 2-bit cartridge ROM data, with no RAM texture cache.

## Candidate

The original 240 road tiles each have exactly 16x16 shade cells at 16-world-unit intervals. Each shade category is a two-bit value: zero for HD_REFLECT, one for HD_ROAD_LIGHT, two for ordinary road. This experiment uses one byte per category instead of packing four values into each byte, allowing the inner floor shader to read the category directly, without a per-sample variable shift or mask.

Data cost: **61,440 bytes of ROM for categories** plus the unchanged 432-byte road index, **46,080 more ROM bytes** than the current packed release. EWRAM, IWRAM, framebuffers, heap and SRAM consumption do not increase. The exact palette indices, 24x18 town collision, story, rain, actor sprites, texture positions and native 240x160 resolution remain unchanged.

## Strict acceptance

The baseline's host tests still independently recalculate **61,440 original road categories** and test all other tile guards, **120 cold/warm camera frames**, **3,456 complete independent golden frames**, wall depth/actor occlusion and all story, cinematic, save, input and music suites.

Require the full ARM GBA build, actual mGBA runtime and screen capture, and source-matched ARM walking profile. **Do not merge merely for minor cycle savings.** The extra 45 KiB of cartridge storage must clearly outperform the packed baseline **7.068 page changes/sec while walking** without regressing stationary 14.932, RAM limits, or pixel identity. If not, keep the compact 2-bit release.
