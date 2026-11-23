# First-person renderer milestone: exact grid-boundary ray jumps

## Bottleneck

The existing GBA native 240×160 first-person raycaster sends 45 quantized rays across the real 24×18 story grid. Each ray previously advanced by 32 world units up to 75 times. Even when consecutive samples remained in one empty tile, the loop still executed coordinate arithmetic, shifts and branches. This wastes work on empty streets and distant buildings.

## Change

Use fixed-point sampled grid traversal, not a lower-detail raycaster. At every visited tile, calculate how many of the **original 32-unit sample steps** will pass before either axis changes tile. Advance directly by the minimum X or Y crossing step, then test the exact original story tile. This is not an intersection against an un-sampled wall: the algorithm lands on the first **sampled position** in the new tile, including the original diagonal-corner behavior. Wall hit distance, texture U, wall depth, floor occlusion, actor occlusion and ray/column grouping all remain defined by the original sample index.

The GBA CPU does not offer native division, so integer quotients for the next tile are calculated with a tiny **33-entry, 66-byte ROM-only Q16 reciprocal table** for ray velocities 0–32. For crossing distances 1–256 units, `floor(n / speed) == (n * ceil(65536/speed)) >> 16` for `0 <= n <= 255`. Velocity 0 never crosses; velocity 1 is handled directly. There are no new global RAM buffers, framebuffers, heap allocations, texture sets or reduced screen resolution.

## Regression gates

- Host test compares the computed next sample against the independent original one-step ray march for **49,216 signed tile-phase/velocity combinations**, including negative world coordinates and both ray directions.
- The existing **3,456 complete first-person golden frames**, actor roster validation and 136,140 wall/floor depth comparisons must remain byte-identical.
- All story, cutscene, saving, studio, gameplay input, and renderer safety tests continue to run.
- Actual ARM GBA linker budgets, mGBA screenshots and clock/input probes must pass.
- Compare the verified presentation cadence in the same 600-video-frame scenes with the current 9.955 flips/s first-person baseline. A host microbenchmark or fewer algorithmic operations alone is **not** a demonstrated FPS gain.

Do not merge without passing those checks; a visually different output must be treated as a regression and corrected. The result must preserve the original world and graphics rather than hiding geometry or accepting new pop-in.
