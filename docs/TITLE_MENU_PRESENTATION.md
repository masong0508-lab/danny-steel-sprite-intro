# The Last Days of Danny Steel — native GBA art direction

## The problem and the direction

An earlier menu revision overlaid crude 120x80 all-cap captions on
monochrome full-motion footage. The map used an entire 120x80 frame
hardware-doubled to the 240x160 GBA LCD, producing a chunky, featureless
look. The screenshots in this folder show the result.

The new direction uses **native 240x160 mode-4 pixel composition** for
the main menu and overhead Old Quarter, with a subdued rainy-street,
painted record-sleeve identity: weathered brick, sodium light, tarnished
copper, denim-blue shadow, lamp reflections, warm paper type. It retains
the original distinctive title painting, DippInn logo, animated intro
and bossa theme; it is *not* an OS, sci-fi HUD or futuristic terminal.

## Implemented

- **Menu:** `src/hd_menu.c` creates an illustrated, continuous Old Quarter
  scene. A small printed night-story sleeve sits in front of actual
  buildings, rain-dark pavement, a passing bus and Danny in his red
  jacket. There are three clearly captioned options; LEFT/RIGHT picks,
  A opens, B goes back. Two existing slots and their same SRAM addresses
  remain unchanged. A fresh continuous 40-frame A hold protects erasure.
- **Overhead world:** `src/hd_world.c` uses exactly the original town
  grid and player position, without changing collisions or routes. The
  map has 16x16 native tiles instead of 8x8 blocks: irregular paving,
  brick mortar, facade windows, shutters, curb lines, tree foliage and
  cast shadows, water ripples, streetlight, mail/record/clinic props and
  distinct player/NPC attire. The same goal query and locations support
  the built-in objective heading.
- **Narrative UI:** `src/hd_overlay.c` draws thin, high-contrast
  speaker-first subtitle boxes, context-aware action menus, faithful
  risk consequences, bus choices and pause help using a new 5x7
  native-pixel font. These use the existing story flags and input.
- **Native canvas:** `src/hd_canvas.c` adds only 32 dusk-toned palette
  entries, integer geometry/text and a backward, in-place, 2x
  compatibility upscale. The same 38,400-byte EWRAM buffer is reused
  by the GBA's two built-in mode-4 VRAM pages. No third framebuffer,
  heap allocations, sampled audio or floating point is introduced.
- **Legacy shots remain playable:** first-person raycaster, timed
  cinematics, ending montages and the six-tab music studio still use
  their proven 120x80 authoring renderer but are expanded into the
  full-size page in place, with native subtitle treatment where
  appropriate. They are *not yet* fully repainted in 240x160. Ending
  dialogue and scored captions intentionally retain existing hooks.
- **Removal:** the old grayscale video loop, sprite cursor dataset and
  their outdated asset generators are removed from the source. Git
  history retains them. The older square-zoom swipe footage was
  removed in a previous patch.

## Visual and playability verification

The `tests/hd_graphics_test.c` host test draws **every walkable map
position**, all main/slot menu states, feedback, native dialogue and
in-place upscale; it verifies a 64-byte guard on either side of the
240x160 canvas and all palette indices. The pre-existing story,
continuity, save, music, cinema and ending regression tests still run
before the full devkitARM GBA build.

`tools/story_gallery.c` and `.github/workflows/screenshots.yml`
capture the title/menu from a running current-revision ROM in mGBA
and capture selected story and native-world frames via the very same
C rendering functions. See `docs/SCREENSHOT_GALLERY.md`.

**Important:** Passing host tests and a ROM compile validates
programming correctness, not professional art direction or exact
frame rate. Hardware/mobile emulator playtesting, measured 30 FPS
headroom, and a separate full-detail first-person/cinematic/Studio
art pass are still needed. Do not imply the entire game is now
native high resolution.
