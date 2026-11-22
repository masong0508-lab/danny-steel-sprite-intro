# Native first-person materials: exact floor and wall hash caches

The first-person perspective floor shades individual native 240x160 pixels using a hashed deterministic material seed. Previously the `texel` integer hash was recomputed for every road sample, even for adjacent pixels inside the same 16-unit world-space cell.

This pass shares one road `road_ink` material routine between the unchanged path and a fast path for `.` roadway tiles. In the floor scanline loop, only the previous texture-cell coordinates and 32-bit hash are retained. The hash is recomputed when either exact cell coordinate changes. The same road formula, fixed-point camera projection, pixel quantization, edge seams, half-rate distant sampling, wall occlusion, rain clock, palette and depth remain unchanged. Other tile materials retain their previous path byte-for-byte.

An approximate *pre-occlusion* projection inspection for five walkable town positions across all four camera directions found 48–83% repeated road hash-cell references (typically 60–80%). This is a potential reduction in hash computations, **not a measurement of GBA FPS** or a guarantee that every view benefits.

## Release gates
- Keep the 3,456 original full native perspective frame hashes identical across 4 facings, three ticks and all 288 walkable position cases.
- Preserve existing pixel/framebuffer guards, floor/wall depth checks, visual tests, story, SRAM, music and GBA ARM linker limits.
- Produce real new mGBA screenshots and inspect measured page-flip cadence against prior main. Do not merge on an unverified claim of higher FPS.
- No extra framebuffers, heap, persistent global caches, precomputed texture atlas or added palette colors.

## Wall material cache

The same view also hashes wall-brick material once per visible vertical texture sample, even when adjacent pixels use the same 16-unit brick or 8-unit tree cell. The wall column now remembers only the previous vertical cell and its exact hash. A column cannot switch between brick and tree, so those two scalar variables safely cover both materials. Door, window, mortar, shade, sprite, signage, and occlusion cases retain their original priority and pixel colors.

The earlier floor-only experimental screenshot capture still showed 9.955 first-person page flips/second, the same as the baseline. This second wall improvement therefore requires a **fresh** ROM-cadence measurement and the exact 3,456-frame golden equivalence gate; it is not predeclared faster. The merged 240x160 Mode-4 game remains the release source until full CI succeeds.
