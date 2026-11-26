# Danny Steele likeness in the actual GBA game

The owner's close-up photo informs the dark swept full hair, shorter
temples, stronger brow, soft long face, connected reddish-brown beard
and dark headphones. Keep Danny's established red coat and satchel and
the original yellow work costume; the raw photo is not a ROM asset.

The 24 Danny world sprite cels, shared native dialogue/ending actor,
portrait expressions, low-resolution fallbacks, and articulated
Potheads opening now carry these features. The recovered 548-frame
307530.mp4 matches the reference SHA256 recorded in POTHEADS_OPENING.md.
Its 110 measured poses, sign, hands, feet, duration and frame crop have
not been modified. The appearance correction rotates the beard and
hair in the measured local head frame rather than screen coordinates.
No original reference photos or video are published; no extra
framebuffer, save data, character movement or collision changes.

The existing tests are extended to cover all four directional masks,
idle/walking consistency, rear-facing head, and actual portrait
expressions. The earlier interactive-interiors PR remains separate
until its failed CI can be diagnosed.

All 42 production side-story panels have been regenerated with the same
hair, beard and collar headphones. The original panel generator, binary
artwork, manifest and golden checks agree byte-for-byte. All panels remain
unique and the original palette and ROM art byte count are unchanged.
