# Danny Steel — rainy-city isometric exploration, camera milestone

## Visual objective

The player's first and third reference images establish the main target:
**readable isometric streets, dense physical architecture, living
neighborhood details, wet pavement, practical lamplight, strong depth
and a quiet, melancholy city after dark.** The aim is a world that feels
authored rather than the previous flat rectangle grid, not a futuristic
OS interface. The currently supplied Danny sprite is provisional:
his eventual reference-based redesign belongs in the same data-driven
actor atlas and should not require changes to the camera or story code.

The reference images are **direction**, not game screenshots or claims
that a 240×160 GBA ROM can reproduce their pixel count and rendering
complexity one-for-one.

## What is now implemented

- The **playable overhead view** changes to an isometric **2.5D camera**
  in `src/hd_isometric.c`. The existing L/SELECT toggle still switches
  to the genuine native 240-column **first-person** view; the story
  grid, controls, save structure, NPC positions, collision and quests
  are unchanged.
- Both camera modes remain projections of the **same 24×18 grid**.
  Isometric camera axes: east = screen southeast, south = screen
  southwest, west = northwest and north = northeast. The D-pad follows
  those logical streets, while the physical action labels remain
  readable in the bottom HUD.
- **14 new 32×16 ROM-resident isometric diamond materials** are
  authored in `src/hd_isometric_atlas.c`: wet/slate roadway, mortar
  paving, grass, canal water, rooftops and worn walkway. They use only
  the existing cartridge palette slots and an in-asset transparent key
  that never reaches the LCD framebuffer.
- **Actual height** is painted above the diamond footprints for
  facades: roof ridges, two visibly different wall faces, recessed
  windows, warm glowing apertures, doors, chimneys and trim, with
  consistent y-depth order. This is deliberately not pretending to be
  a hardware polygon engine: it is performant authored 2.5D pixel art.
- Street furniture, trees, shop landmarks, bus and townspeople are
  anchored to their original world locations and sorted by the
  isometric world-depth index `x+y`; existing directional Danny cels
  share the same floor anchors. A subtle 24-stroke rain layer and
  small pavement highlights add weather without another renderer pass.
- Uses one existing **38,400-byte** EWRAM Mode-4 screen buffer, the
  same VRAM page flip and no heap allocations or float transforms.
  Extra art lives in cartridge ROM, per the user's preference.

## Invariants and performance gate

- `view=0` in existing saves remains the walking overview. It now
  draws the isometric quarter, rather than the flat top-down scene.
  `view=1` still opens first-person exploration. No new save bits.
- `tests/hd_isometric_test.c` covers all 14 materials, clipping on
  every screen edge, correct palette indices, identical repeat renders,
  facing distinctions and named town locations. Existing tests still
  cover all menu, NPC, save, ending, music and battle sequences.
- Linked **EWRAM/IWRAM budgets** are enforced during the devkitARM
  compilation. ROM growth is allowed; RAM growth and unchecked
  framebuffer allocations are not.
- Host render cost is only an informational CPU metric. Sustained
  30 FPS must be measured in timed mGBA/physical GBA before more
  demanding rain, parallax or character-animation layers are added.

## Remaining art ambition

Much finer roof and sign illustrations, hand-placed multi-tile
architecture, better weather lighting, and consistent all-view
Danny portraits depend on actual art review and future character
references. The current milestone establishes correct spatial
projection and gameplay integration, not a claim of identical
quality to the high-resolution desktop art references.
