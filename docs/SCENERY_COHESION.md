# Old Quarter: continuous scenery and shared architecture

The isometric and first-person cameras now use the same district identity and the same eight authored facade textures, with two lighting faces. The old first-person view replaced those buildings with one generic window/door shader; now the original ROM textures are sampled directly. Potheads keeps its physical fascia, and distant brick and window colors are subdued without adding new assets or a palette. Wall heights and floor occlusion geometry remain unchanged on the first-person path.

The isometric camera now pans through the four existing movement phases, translating terrain, building walls/roofs, props, route cues and cutaways together. Danny remains centered and the original fractional world depth governs painter order. First-person uses the same fixed-point world position. Map, collision, routes, story timings and SRAM are unchanged.

Billboard occlusion no longer redraws overlapping walls one column at a time. Equal ray spans reuse the already computed shades, including cold camera frames. No second buffer or expanded texture cache is added.

Host tests retain the independent 3,456 shipped first-person golden frames in a host-only legacy-art mode. The new visual path is separately compared against the original one-column restoration algorithm, tests cold/warm equivalence, and verifies the shared facade source texels. The projection regression checks the original district dimensions and each fractional phase against the exact world position. Existing clipped sprite and player-readability checks still run. No hardware/emulator performance or visual completion claim until the pending build and capture succeed.
