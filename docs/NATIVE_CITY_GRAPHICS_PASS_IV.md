# Danny Steel — Native city graphics pass IV

## Goal
Give every major Old Quarter location a recognizable physical identity at the game's actual 240x160 GBA resolution, while preserving walkability, SRAM, first-person rendering and the single shared Mode-4 frame.

## Art direction changes
- The music station becomes a grounded record console with a spindle, tonearm, knobs and two speakers.
- The rooftop creative objective uses an easel, a legible landscape painting and supporting legs.
- The clinic gets an illuminated medical cross, distinct door glazing and masonry.
- The archive terminal uses a screen, activity light, keyboard and plinth.
- The rescue doorway gains heavy cross-bracing and a latch, rather than sharing the shop-sign silhouette.
- Canal tiles acquire restrained time-varying reflections anchored to world coordinates; a few walkway puddles and road drains break up uniform terrain.
- Objective highlights are now placed on the actual target pavement inside the scene depth order, so they do not float over foreground roofs.
- Overhead and first-person exploration have more structured, less crowded control legends. First-person shows a contextual risk row.

## Technical contract
The 24x18 quest/collision grid, player position, inventory, narrative, ending flags, SRAM records, original ROM sprite atlas, 32-color extra palette, and existing first-person raycaster are unchanged. There are no new framebuffers, heap allocations, persistent global buffers, per-pixel shader passes or additional video modes.

New details use clipped span primitives only and are bounded by existing world traversal. The water animation uses world tile coordinates and integer ticks. The screen is always 240x160 at native palette indices.

## Evidence and release gate
Host isometric tests require bounded ink, intact frame guard bytes, reproducible goal-marker changes and animation. Existing native pixel/geometry, player-visibility, actor, performance, story progression, saved-state, combat, studio, cinematic and first-person equivalence tests remain required. GBA linker budgets, actual ARM build, mGBA playthrough and production screenshot workflow must all pass before integration.

This is a meaningful visual milestone but not a claim that the campaign or every port is finished; physical-console frame-rate verification remains outstanding.