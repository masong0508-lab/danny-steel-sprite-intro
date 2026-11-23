# Moving first-person GBA renderer — exact procedural texture ROM lookup

## Performance target

The prior milestone delivered **14.932 displayed frames/sec for a stationary first-person camera**, but the matched Potheads back/forward walking route measured **6.570 flips/sec** both before and after the bounded 6,750-byte wall-shade cache. Every new pose still needs expensive wall material shading.

## Candidate change

The original city wall painter evaluates a 32-bit procedural hash inside its brick/tree pixel branch. Its tree texture accesses hash coordinates between 0 and 31; its wall-brick pattern typically accesses between 40 and 79. The generated, immutable **80×80 byte lookup (6,400 ROM bytes, zero persistent RAM)** stores the exact low three bits of the existing formula, rather than introducing any new art.

The renderer now consults that table for in-range wall/tree texels and falls back to the exact original unsigned hash for out-of-range coordinates. Other procedural textures, floor, sky, perspective geometry, story collision, UI, audio and scene staging remain unchanged. There is no new framebuffer, heap, atlas allocation in EWRAM, changed SRAM or lowered internal 240×160 resolution. The previously verified stationary-wall shade cache still functions identically.

## Regression gates

- Independently calculate **9,241** wall-hash values covering the entire table, adjacent bounds and unsigned-out-of-bounds inputs; they must agree byte-for-byte with the original hash.
- The existing **3,456 complete reference first-person frames** across all town positions, facing and weather must match.
- All existing walking/sprite depth, story, battle, save migration, input clock, Studio, cinematic and RAM safeguards must pass, followed by ARM GBA compilation and real-ROM mGBA screenshots.
- Measure the **600-emulated-video-frame held back/forward walking route** separately from the stationary view and compare it with the validated **6.570** flips/sec movement baseline. Report unchanged or worse cadence explicitly.
- Record the actual ARM wall-stage cycle changes with the profiling workflow and do not promote this as a speed win merely because the instruction count appears lower.

This ROM-only shader experiment is intentionally isolated. If it cannot preserve the golden frames and improve moving-camera time, leave it unmerged and retain the currently verified playable release.
