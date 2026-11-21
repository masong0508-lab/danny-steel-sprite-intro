# Danny Steel — rendering and stability gate before further art/features

This is a strictly **optimization-only** change to the GBA game.
No new sprites, maps, animations, features, story flags, soundtrack
voices, saves or larger framebuffers are introduced. Improvements are
deliberately required to retain the exact previously shipped pixels
where the fast path changes.

## What the current GBA code actually guarantees

- `play_update` processes input, story and music every VBlank, and
  **attempts** a new game render every other VBlank (~30 render
  opportunities per second). The actual render can overrun the time
  available; code inspection or a successful devkitARM build is not proof
  of sustained 30 FPS.
- The mode-4 pipeline paints into one 38,400-byte shared EWRAM canvas
  and copies to the hidden VRAM page before flipping on a later VBlank.
  The double-buffered video memory exists on GBA hardware; no second
  EWRAM canvas is allocated.
- Menus only redraw their animated rain/lighting at about 10 FPS,
  but input-driven changes redraw immediately.
- All 24 authored cinema scene types, the Advanced Studio and the
  timecoded ending montage have higher-resolution artwork. Optional
  combat now also renders at 240x160 via ROM-resident illustrations;
  the original gameplay and hitbox tests remain mandatory. This
  later upgrade is described in `NATIVE_MIRROR_ORCHARD.md`.

## Exact-pixel optimization performed

| Hot path | Previous cost | Optimized mechanism | Expected result |
|---|---|---|---|
| Tree canopy, actor shadow and record ellipses | Every raster row restarted width search from zero | Carry monotonic radius upward/downward through the ellipse | Identical pixels, fewer integer multiplications/iterations |
| First-person facade wall-column textures | Software integer division on every shaded wall pixel | Compute quotient and remainder once per wall column; advance an exact integer remainder accumulator per pixel | Identical wall texture coordinate and shading, far fewer divisions |
| Immutable town collision/art tiles | 10 independent location coordinate branches on every `story_tile` call | Encode the same 10 static overlays directly in the original 24x18 tile literals | Identical walkability, location IDs and NPC placement with fewer branches on every map lookup |
| Street grout, menu rules and architectural line strokes | Bresenham stepping and bounds checks for every single axis-aligned pixel | One clipped horizontal or vertical fill; preserve original Bresenham for diagonals | Identical raster, fewer pixel loops for frequent straight strokes |

These optimizations preserve the game's semantic input and timed
animation behavior. In particular, they do **not** skip gameplay
simulation or increase the frame interval, and they do not trade image
detail for a fake performance claim.

## Mandatory regression gates

- `tests/native_perf_equivalence_test.c` checks the exact pre-patch
  town tiles, including out-of-range behavior; verifies every texture
  coordinate for wall heights 1–150 against the original per-pixel
  quotient; and compares all 38,400 pixels for **500** clipped and
  unclipped ellipse shapes, plus **363** horizontal, vertical, diagonal,
  reversed and clipped line strokes, against the original routines
  with untouched reference drawing code.
- Existing tests continue to cover native 2D/3D/cinema/studio/palette
  bounds, complete story routes, rescue outcomes, save migrations,
  audio continuity, cutscene timing, ending score and GBA ROM compilation.
- `tests/render_cost_probe.c` prints per-mode average host CPU render
  time for overhead, perspective, Mara scene, rescue scene, notes,
  endings and native Mirror Orchard combat. This is an *informational
  regression indicator on the CI
  runner*, **not** an ARM7TDMI hardware benchmark or measured FPS.
- The devkitARM build emits a GNU linker `.map` file and CI reports
  linked sections plus binary bytes, without requiring the unavailable
  `arm-none-eabi-size` utility. The baseline after this optimization was
  **38,400 bytes of EWRAM** (one shared framebuffer), **8,880 bytes of
  IWRAM BSS**, 20 bytes of linked IWRAM data and zero explicit IWRAM.
  CI now rejects static EWRAM above **192 KiB** and static IWRAM
  (.bss + .data + .iwram) above **24 KiB**, leaving at least 8 KiB of
  IWRAM capacity for stack/interrupt headroom. These are static section
  budgets, not a claim of measured runtime peak memory usage.

## Explicit hold point

Before authorizing further features or more detailed animation:

1. Strict host tests and devkitARM ROM build must both pass.
2. A fresh ROM must be published and basic mGBA gameplay/save
   smoke checks should be repeated.
3. A repeatable instrumented emulator or physical GBA test should
   measure worst-case visible frame time in overhead, 3D, cutscenes,
   Studio and Mirror Orchard. Only these measurements can establish
   whether the intended 30 FPS target has adequate margin.
4. If the render regularly exceeds its allotted two VBlanks, optimize
   the worst scene first; do not add effects while that remains true.

**No real-device FPS data is available from these CI workflows yet.**

## Verified optimization acceptance — 2026-11-21

[PR #7](https://github.com/masong0508-lab/danny-steel-sprite-intro/pull/7)
was merged after the entire host regression suite and devkitARM ROM build
passed on the feature branch
([workflow 44936047192](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/44936047192)).
The new CI budget gate itself passed with measured linked static sections:

- **EWRAM:** 38,400 bytes, against a 196,608-byte enforced limit.
- **IWRAM:** 8,880 bytes BSS + 20 bytes data, against a 24,576-byte
  enforced static limit; the physical IWRAM total is 32,768 bytes.
- **ROM:** 604,548 bytes in the tested devkitARM branch build.
- **Exact operation checks:** wall texture quotient divisions fall from
  11,325 to 300 for the 150 tested heights, monotonic ellipse
  radius comparisons from 128,960 to 25,700, and 363 line-raster
  cases remain pixel-identical. Town-grid equality passed at 775
  checked coordinates including out-of-bounds handling.

These metrics are **not** a claim of sustained 30 FPS. The 30 Hz render
schedule is only a target until a timed emulator/physical GBA benchmark
establishes worst-case frame completion. No new visual effects or
animation should be added before that check.
