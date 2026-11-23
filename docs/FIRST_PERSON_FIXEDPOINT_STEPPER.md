# GBA first-person floor: exact incremental fixed-point projection

## Problem and objective

The first-person GBA floor painter draws a real 240×160 scene, but the floor's ray-side coordinate was recomputed with a signed multiplication for every visible texel sample. The near field samples each horizontal pixel and the far field samples pairs. ARM7TDMI has a limited cycle budget, so the objective is fewer integer operations **without changing the picture at all**.

## Implementation

Within each visible quantized-ray run, initialize the signed 24.8 side accumulator once:

`side_acc = (start - 120) * side_unit`.

Advance it by `step * side_unit` for each sample, and derive `side = side_acc >> 8` using the original signed shift. The start column remains the existing even-aligned start in the far field, and `step` stays 1 or 2 based on the original row. The camera's forward world coordinates are also computed once per scanline, not per texel.

This keeps the original ray grouping, projection, world tile, alpha/occlusion, floor material, dynamic ripple timing, texture UV quantization and paint order. No new framebuffer, scanline cache, ROM textures, heap allocation, palette entries or global state.

## Tests and release gate

A host-only arithmetic test enumerates every possible ray start (0 through 239) for each of 60 floor scanlines and compares every ensuing stepped sample against the old independent multiplication, including negative half-screen offsets and far-field even alignment. The existing 3,456 full-frame golden hashes from the independently recorded renderer must also match across town positions, facings, animations and weather. The workflow additionally requires native ARM GBA build, linker RAM budgets, real-ROM mGBA capture, zero input loss and no change to story/save/audio behavior.

The next acceptance question is whether a sampled first-person scene crosses from six VBlanks per presented frame (about 9.955 flips/s) toward five (about 11.945 flips/s), with no adverse change elsewhere. Passing pixel tests alone does not establish an FPS improvement.

This change is an experimental optimization until a successful complete ROM/emulator check demonstrates its value.
