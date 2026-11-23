# First-person GBA — clip occlusion restoration to sprite coverage

## Why

The native first-person scene draws walls first, draws depth-sorted people/props, then restores wall pixels wherever a foreground wall hides part of the newly drawn billboard. Previously, each touched column repainted its **entire** wall from the roofline to the lower edge even though a person or sign can only modify a short portion of that column. This duplicate work becomes costly near groups and narrow streets.

## Change

For each billboard, constrain the wall restoration pass to the billboard's **vertical footprint**: from `ground - height` to `ground + 4` screen rows, intersected with that wall's existing top/bottom bounds and the same renderer's 140-row world limit. The original draw pass remains full height. The clipped pass starts the integer wall-texture DDA at exactly `floor((clip_start - wall_top) * 256 / wall_height)`, carrying the matching remainder, so mortar, windows, signage, tree shading and wall caps should match the original painting at every restored row. The extra four bottom rows account for small grounded shadows.

No screen-resolution reduction, reused generic placeholder art, heap allocation, Z buffer, new framebuffer or world-grid change. Both top-down and first-person story/sprite coordinates remain unchanged.

## Acceptance

PR #26 is stacked on PR #25's independently tested exact floor projection. Before integration, pass all 3,456 fixed golden first-person frames, the original 136,140 floor-wall depth checks, map/actor/hardware bound checks, native ARM GBA linker budgets and actual-ROM screenshot/runtime cadence measurement. If any goldens differ, fix or reject the clipped restoration rather than accepting altered occlusion.

This can reduce duplicate shading work without compromising visual detail. It is not considered a verified FPS improvement until the mGBA page-flip window shows a gain over the 9.955 flips/s measured baseline; test multiple gameplay locations before treating it as a universal figure.
