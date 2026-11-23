# Native GBA: exact grass/tree material lookup

The current tested Danny Steel GBA ROM presents about **7.068 moving first-person frames/sec** and **14.932 stationary frames/sec** on the scripted Potheads route. Native ARM profiling shows the moving-floor shader remains the largest render stage. The existing road shader is already an exact 2-bit ROM material; grass/tree ground still performs the original 32-bit procedural texture hash for each sampled pixel.

## Small, lossless candidate

- The fixed Old Quarter's 24×18 tile grid contains exactly **8 grass** and **20 tree** terrain cells.
- Every grass/tree cell uses an 8×8 pattern of original noise bits at 32-world-unit intervals.
- A compact **432-byte tile index** and **224-byte one-bit pattern table** store exactly those authored hash results in cartridge ROM: **656 bytes extra ROM, zero additional RAM or framebuffer**.
- The original water movement, road texture, pavement seams, sky/lighting, actor depth, pixel positions, save and collisions remain unchanged. Grass and tree floor material now simply reads the same LEAF/LEAF_DARK classification from ROM.
- The lookup only applies when story_tile reports 'g' or 'T' in the original fixed town; no out-of-bounds world locations are newly drawn.

## Release gates

The source-matched host test independently recalculates all **1,792** green terrain shade bits and validates **25,856** non-green tile/texel guard cases, plus the original **61,440** road shades. All **3,456 original native 240×160 golden frames**, **120 cold/warm camera frames**, story, studio, SRAM, cinematic, input, ARM linker/RAM and emulator checks must remain green.

The source-matched ARM profiler must show whether *walking-floor* CPU cycles improve versus approximately **1,202,429 cycles per frame**. The real-ROM 600-video-frame held-walking cadence must improve beyond **7.068**, or at least show a compelling measured resource benefit before integration. Keep the separate stationary first-person baseline at **14.932** page changes/sec. Do not merge if any visual artifact or performance regression occurs.

This is a cartridge-only texture-material optimization, not a change to art quality, sample rate, projection or draw distance.
