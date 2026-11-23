# GBA native floor: one-word cache for 2-bit road material rows

## Target

The verified GBA game presents **7.068 pages/second** while walking and **14.932** when stationary in the 600-video-frame mGBA Potheads route. Its exact 15,360-byte cartridge-ROM road ink texture stores four two-bit categories per byte. The per-pixel category extraction is one remaining cost of moving-floor rendering.

Two alternatives have already been tested and rejected: a 16-byte expanded row cache took more CPU and lowered walking to 6.968, while a fully unpacked byte road table improved to 7.167 at the expense of **46,080 more ROM bytes**, an unattractive storage trade-off.

## Experiment

On each visible scanline, the ray painter tracks the active road map tile and its 16-world-unit material Y coordinate. It loads one **aligned 32-bit word** from the existing packed ROM table only when that coordinate or tile changes. All sixteen two-bit road categories for the material row are held in that one local scalar and extracted with one variable shift per pixel.

The original curb edge priority, screen dither, camera projection, collision, real map geometry, palette, music, actor occlusion and native 240×160 output remain exactly the same. No added framebuffer, persistent RAM, heap allocation, enlarged ROM texture, or lossy art.

## Acceptance requirements

- Test every one of the **61,440** original road shade categories and **49,152** non-road guard cells both through the reference 2-bit lookup and through the new aligned-word loader.
- Preserve all **3,456 independently recorded full-frame first-person golden hashes**, 120 warm/cold wall-cache comparisons, 9,200 wall-store guard tests, story/saving/audio/Studio/cinema tests and native memory budget.
- Measure source-matched GBA ARM walking floor cycles against **1,202,429 cycles/frame**, and the exact mGBA active-walking page-flip benchmark against **7.068**, retaining stationary **14.932**.
- Reject this implementation if the extra row-change conditional outweighs any savings or if it makes camera animation look different. Do not infer faster display rate from host benchmarks alone.

This change is experimental until real-ARM and emulated-ROM results pass those gates.
