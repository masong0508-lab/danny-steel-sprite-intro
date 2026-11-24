# Inked Old Quarter: native art direction

This pass uses actual game code and the existing Mode-4 palette. It builds
on the first-evening campaign rather than replacing the game's established
red-coat/blue-hour identity.

## Visual composition
- Exterior cinematic skylines have layered roof masses, non-strobing lit
  windows, connected cloud shapes, restrained lamp reflections and rain
  behind actors. The original sky bands and physical shot anchors remain.
- Interior sets distinguish the archive, recording room and clinic with
  different materials, wainscot, recessed windows, localized reflections
  and converging floor joints.
- Mara's shared cafe facade has cast-shadow cornice depth, shaded glazing,
  pottery silhouettes and dimensional striped awning. Two cups have
  saucers and handles; chairs and table feet sit in the scene.
- Close-up faces use role-specific spacing, cheek and nose planes, hair
  highlights and more restrained eyelids; Mara's teal scarf and pendant
  remain connected to her jacket. Body poses and hand-contact anchors
  remain unchanged.
- The cinematic, isometric and menu buses share a cream roof, red body,
  shaded glazing and pale beltline. The menu uses Danny's actual gameplay
  sprite and removes per-scanline background fills.
- The isometric easel depicts a skyline rather than an abstract zigzag.

## Performance and verification

No new palette slots, sprite atlas, scene-sized assets, heap allocation,
extra screen buffer, save fields or collision geometry. The native
first-person painter and its golden-frame contract are untouched.
The walking-camera painter adds only a small bounded bus/easel refinement;
cinematic art remains deterministic and clipped in the existing buffer.
Host checks verify exact menu sprite pixels across weather, guarded
character portraits and face identity, existing environment and contact
anchors, both project styles, and unchanged renderer/collision tests.
The gallery adds explicit captured stages of the actual canvas and
recording room. Hardware frame-rate validation is still outstanding.

Review the real generated captures in `docs/screenshots` and
the GBA build's RAM gate before promoting this version.
