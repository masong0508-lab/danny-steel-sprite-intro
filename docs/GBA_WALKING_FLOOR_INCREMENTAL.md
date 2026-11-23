# GBA moving-camera floor: exact ray stepping combined with road material caching

This stacked experiment adds the already-proven fixed-point floor projection change from PR #25 to the road material fast path in PR #36, whose primary goal is improving the **6.570 presented page flips/sec** measured during continuous first-person walking. In the source-matched moving ARM profiler, floor materials dominate at **1,389,672 cycles / 82.831 ms per rendered frame**.

Within each visible ray-direction group and floor scanline, the existing `((x - 120) * side_unit) >> 8` is represented by an integer accumulator initialized at the exact existing even-aligned first x pixel, then advanced by `step * side_unit` per native or far-field sample. Camera-forward world components are computed only once per row. The mathematical result is identical for negative and positive side coordinates because the signed arithmetic right shift is retained. All original floor colors, 16-world-unit road hash caching, static 400-byte ROM dither, lighting, wall occlusion, assets and GBA 240×160 canvas remain unchanged.

The host tests now require **1,476,000 exact signed fixed-point comparisons**, **14,400 native dither checks**, all **3,456 independently recorded complete first-person goldens**, 120 cache cold/warm scenes, depth bounds, full story/music/saving/Studio tests, ARM static RAM linker checks and actual mGBA input+runtime validation.

Release requires comparison of the same moving-camera 600-video-frame route against:
- Original/main stationary: **14.932** page flips/sec
- Original/main continuous walking: **6.570** page flips/sec

Both the source-matched emulated ARM floor-cycle profile and actual moving Mode-4 page-flip rates must be inspected. Do not merge if the combined work fails to improve or preserve frames and original full-scene graphics. No additional RAM, framebuffer, fake sprite shifting or geometry clipping.
