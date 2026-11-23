# Danny Steel GBA: amortized 2-bit road shade decoding

## Performance objective

The verified playable GBA release presents **7.068 Mode-4 pages/sec** during the 600-video-frame Potheads walking benchmark and **14.932 pages/sec** with a stationary first-person view. An alternative one-byte-per-shade ROM format increased walking cadence to 7.167, but required 46,080 extra cartridge bytes for a very small gain. The existing compact 15,360-byte two-bit road materials should remain the preferred storage format.

## New experiment

The world-coordinate road material depends on a 16x16 category grid inside each of 240 original road map tiles. Adjacent screen samples commonly read the same *world texture row* as the camera pans or walks.

Instead of performing an indexed ROM byte read, variable shift and bitmask for **every road pixel**, the floor renderer now expands the current four-byte compressed 16-category material row into a tiny **16-byte local stack array**. It performs this expansion only when the sampled road tile or 16-world-unit material Y row changes; consecutive samples read the category from the local array. The same curb rules, screen-space 17-step dither, original color priorities, tile collision, actors, sky and perspective remain untouched.

The decoded array is **not a second framebuffer, persistent texture cache, heap allocation or EWRAM image**. It adds no ROM texture data and only sixteen bytes of local scratch storage in the current floor renderer. The original 240x160 Mode-4 pixels, sound, SRAM layout and all world geometry remain unchanged.

## Tests and release gate

The host regression independently calculates all **61,440 original road shade categories** and checks the new row decoder's exact values for each, with **49,152 non-road guard cells**, while retaining the **3,456 independent complete first-person golden images**, 120 cold/warm camera frame comparisons, 9,200 aligned-wall guard cases, and all story, studio, cinematic, clock, battle and save tests.

The ARM GBA build must pass actual EWRAM/IWRAM limits, and the source-matched mGBA profiler and runtime capture must compare **moving-floor cycle cost versus 1,202,429 cycles/frame**, **walking presentation versus 7.068 page changes/sec**, and stationary camera versus **14.932**. Do not merge on a host microbenchmark or cycle claim alone; reject if visuals or measured performance regress.
