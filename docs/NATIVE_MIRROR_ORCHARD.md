# Mirror Orchard — full native GBA arena illustration

This pass finishes the last screen still rendered at 120×80 and enlarged
to 240×160. It deliberately spends additional **cartridge ROM** on authored
background and enemy art rather than making the GBA reconstruct costly
landscape geometry and textures every frame.

## Visual direction

Mirror Orchard is a fictional, bleakly funny Old Quarter fandom dispute,
not a science-fiction hacking minigame. The new arena illustrations are
rain-weathered brick, public-office cabinetry, window glass, physical
papers, shop signs, vinyl shelves, worn shoes, brass and muted red cloth.

Three separate **240×160, 64-color-index-range raster settings** are
shipped as immutable ROM data in `src/hd_arena_art.c`:

- The rainy street with a battered public notice board for **RATIO RAT**
  (and the later REBRAND RAT version).
- The municipal/BORE-inspired filing chamber for **BADGE LORD**
  (and the later EX MODERATOR version).
- The vinyl-and-document archive with a real skyline for **LORE WARDEN**
  (and the later REPLY ARCHIVE version).

Each encounter also gets an original **80×84 pixel indexed rival sprite**
with character-specific shape, wardrobe, facial detail, paper props,
costume ears or headwear. In front of them, the same red-coated Danny
stage actor used in the game is drawn at the correct selected lane and
anchored to a stage-floor shadow.

The presentation no longer uses 120×80 doubled gameplay pixels in these
battles. This is *authored native pixel art* with a clear 240×160 layout,
not a claim of modern polygonal 3D or photo-realism.

## The real gameplay is unchanged

The `story_tick` implementation still owns the entire confrontation:
three real logical lanes, FACTS/CARE/EXIT, two waves, four incoming beats
per wave, 15 story ticks per incoming attack, six-tick A-parry, 18-tick
cooldown, focus and guard behavior, outcome conditions and the ability to
leave. No story, save, music, combat or hitbox variables were changed.

The new `hd_arena_draw` only *reads* those variables. It uses
`story_attack_lane(s,beat)` itself when drawing the moving paper tell
and the target-floor marker, preventing a mismatched visual attack lane.
The FOCUS resource has a visible label; ROUND and BEAT are explicitly
named. Menu controls occupy side placards so they do not block the
enemy's costume, head or hair.

## Cartridge / RAM / speed tradeoff

- Immutable backgrounds: **3 × 38,400 = 115,200 bytes** in ROM.
- Immutable character art: **3 × 6,720 = 20,160 bytes** in ROM.
- Total primary raw illustration: **135,360 bytes**, plus compiled code
  and pointer tables. This is intentional. Future high-detail scenes may
  increase the ROM substantially further if they earn the space.
- The display still uses the **same 38,400-byte EWRAM drawing page** and
  hardware page-flipped 240×160 mode-4 VRAM. There is **no second
  framebuffer**, heap allocation, per-frame procedural landscape
  reconstruction, costly FMV decompression or large runtime texture cache.
- The background is a simple ROM-to-existing-canvas copy, then only the
  dynamic fighters, attack tell, focus/mode panel and small parry marks
  are repainted. Cartridge growth is **not** an excuse to waste RAM.
- The CI linker-map gate continues enforcing static EWRAM no more than
  192 KiB and IWRAM no more than 24 KiB. It still does **not** certify
  runtime peak stack usage or sustained 30 FPS on real hardware.

## Verification

`tests/hd_arena_test.c` checks all three complete backgrounds and
all three portraits' index ranges, all menu choices, all rival types,
pre/post narrative variants, four danger beats, parry/round states,
actual `story_attack_lane` values, unmasked top-center character art,
and guarded 38,400-byte frame boundaries.

The production gallery captures four additional true renderer frames:
`29_orchard_rat_menu.png`,
`30_orchard_badge_care.png`,
`31_orchard_warden_dodge.png`, and
`32_orchard_rebrand_parry.png`.
They are rendered by the same C game code as the ROM, at explicit
gameplay states. The first four gallery images are separately captured
from the compiled ROM in headless mGBA. The combined sheet does not
claim one continuous run reached each optional plot stage.

The host CPU probe now includes the ROM-illustrated arena beside
overhead, 3D, story scenes, music Studio and ending films. The test and
full devkitARM build must pass before merging.

**Still outstanding:** instrumented mGBA or physical GBA worst-case
frame-time profiling; no sustained FPS claim is made from a static image
or x86 host timing. Other ports are not modified by this GBA patch.
