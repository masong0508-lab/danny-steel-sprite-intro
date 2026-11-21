# Danny Steel — Old Quarter cartridge artwork / fourth graphics pass

This pass upgrades the **actual GBA game** instead of creating concept
mockups. It targets the most obvious remaining visual mismatch: the
overhead town looked like a flat collection of blue grid rectangles,
while title screens, cinematic scenes and Mirror Orchard already had a
more intentional illustrated identity.

## New graphics architecture

`src/hd_world_atlas.c` provides **54,816 bytes of native indexed artwork
in cartridge ROM**:

- 22 individually textured **16×16 surface stamps**, including worn
  road, warmed flagstones, grass, windowed brick, plaster path, doorway
  and canal water motifs.
- 56 distinct **22×34 grounded person poses**, covering seven role
  identities × four compass orientations × two gait states. Danny wears
  his red jacket/satchel, Mara retains her longer hair, the BORE clerk
  wears office colors and documentation, and each Mirror Orchard
  townsperson has separate clothing. Hair, sleeves, hands, shoes,
  contact shadow and garment silhouettes are part of one complete ROM
  cel rather than separately moved shapes.
- Six **32×38 tree poses** with foliage variations and subtle leaf
  movement; trunks and shadows share the same ground anchor.

All artwork uses exactly the existing 32..63 palette entries. The
transparent value 64 exists only **inside the cartridge sprite sheets**:
it is never written to the Mode 4 working frame. This avoids adding
palette entries, decoded images, separate framebuffer memory, sprites
stored on the heap or runtime texture generation.

The overhead view directly copies the 16×16 tile stamps as short
contiguous ROM→EWRAM rows. The new actor renderer uses the same asset
in all four movement directions. Foreground/background scenery
drawing is now ordered by the actors' foot positions, so characters
pass behind and in front of trees and objects as they should.

The native first-person street renderer uses **the same costume and
hair geometry** as the overhead scene. It projects selected 22×34
character cels in-place, calculating sprite coordinates once per
column rather than performing division for each output pixel. The
existing sorted wall-occlusion and collision grid remain authoritative.
This does not pretend the renderer is a hardware polygon engine.

## What remains exactly unchanged

Quest routes, NPC map coordinates, conversation actions, side-story
progression, three-lane Mirror Orchard battle physics, time and
soundtrack, save-slot layout, death/ending logic and music studio
editing are left unchanged. The latest Mirror Orchard backgrounds and
character portraits are not overwritten by the town art pass.

The large ROM is intentional: artwork is kept in cartridge storage
and painted into **the same 38,400-byte EWRAM frame** before the normal
video page copy. Static RAM budgets from the previous optimization
pass remain enforced by GitHub Actions (24 KiB IWRAM static; 192 KiB
EWRAM static), and the resulting ROM is checked during the devkitARM
build. Artwork size is not a substitute for tested frame pacing.

## Validation and remaining performance question

`tests/hd_world_atlas_test.c` validates all material groups, seven
distinct roles, four headings, two gait frames, projected sizes,
offscreen clipping, palette safety and multiple walkable map views.
Existing native 2D/3D render, ending, arena, storyline, music, save
and ROM tests continue to run.

The screenshot workflow captures four additional *actual game-engine*
overhead views in distinct parts of the Old Quarter, alongside the
previous title, arena, cinema and Studio captures. The planned gallery
now has **36** named frames if capture succeeds.

Real-device sustained 30 FPS is **not yet proven**. The host render
probe gives only relative workstation CPU cost per frame; the next
performance gate is timed mGBA or hardware profiling of the new town
atlas, 3D NPC projection, and worst-case large combat scenes.
