# Danny Steel GBA — native moving-camera floor-material milestone

## Measured bottleneck

The new source-matched moving-camera profiler recorded **66 completed native frames across a 600-video-frame held-walking route**. Mean ARM render costs were floor/materials **1,389,672 cycles (82.831 ms)**, walls/depth 312,273 (18.613 ms), sky 204,725 (12.203 ms). The complete renderer averaged 1,972,815 cycles (117.589 ms). The prior image-preserving stationary wall cache raised stationary presentation 9.955→14.932 page flips/sec, but **walking remained at 6.570**.

The validated earlier wall-noise ROM experiment in PR #34 did not change this rate and was left unmerged. The correct priority is road/floor work.

## Source optimization

- The original road material uses a 32-bit deterministic texel hash at quantized **16-world-unit** positions. Adjacent screen pixels often sample the exact same material cell. A small **per-scanline, last-cell key and hash** now reuses that exact result while the key remains unchanged. The cache consists only of three local scalars; it allocates no global memory.
- The original road dither uses the predicate `(x + y) % 17 == 0` per final-color calculation. Its truth values are now stored in a **400-byte immutable ROM table** for every legal visible row/column sum. No texture placement, intensity, geometry, image crop or palette index changes.
- For non-road tile kinds, the original source and material rules remain unchanged. All first-person work uses the existing **240×160 GBA Mode 4 canvas**, the same ray depth strip and the previously validated wall shade cache.

## Acceptance requirements

1. A host test checks all **14,400** legal screen positions in the visible floor band against the original modulo expression.
2. All **3,456 independent 240×160 perspective golden frame hashes**, exact actor depth/occlusion tests, town map, gameplay, Studio, story, SRAM, music clock and cinematic tests must remain green.
3. DevkitARM compile, linked EWRAM/IWRAM budgets, the original mGBA runtime/queue check and screenshot capture must pass.
4. The moving ARM cycle profiler must report floor rendering and total rendering CPU time on the same input route, and the 600-video-frame moving cadence must be compared against **6.570 presented pages/s**. Preserve the standing-camera **14.932** result. Do not claim an FPS win from arithmetic simplifications alone.

This branch intentionally avoids adding an image buffer, deferred frame, heap allocation, large texture atlas or visibility clipping that would create pop-in. If the measured GBA rate or render budget worsens, do not merge it.
