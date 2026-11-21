# Old Quarter art pass — Potheads storefront before sign-dance production

## Character direction corrected

Danny Steele does **not** hate his day job. He is the banana-costume
sign-holder for **Potheads**, an adult dispensary. The work is the
dependable income he currently relies on. He can genuinely enjoy the
rhythm of the shift, seeing regulars and entertaining people, while
remaining serious about getting paid and showing up on time.

His ambitions are separate but compatible: he writes music, makes
visual art, and wants to build personal mixed-media performances. The
costume is a fun, recognizable part of his job; the point of the
character is not that working outside a dispensary is beneath him.

The opening dialogue now establishes **Potheads, his rent-paying
employment, and his sketches and songs** before retaining the existing
doctor, letter, Mara and bus plot. None of the quest predicates,
endings, save fields, or time limits have changed.

## Supplied reference details to carry forward

The user supplied two photographs and a dance-video reference. Use them
as the foundation of the future art/animation implementation, not as
rendered frames currently in the ROM:

- Yellow banana costume with darker tip, layered over dark sleeves;
  distinctive face profile, dark headphones, lived-in outdoor lighting
- Large, visibly handmade pale corrugated street sign, black lettering
  reading **FREE WEED / 21 & UP**, botanical green leaf artwork
- Actual nearby **Potheads** business wordmark, separate from the
  handheld sign and visually anchored to the storefront
- Bird's-eye location overview easing toward Danny, then a human,
  reference-timed dance with the sign, using rotoscoped silhouettes
  rather than a procedural swinging rectangle
- A professional, reasonably happy performer doing an important shift,
  not an embittered mascot waiting to quit

Neither the uploaded photos nor the MP4 have been committed as assets.
They remain source references for the next distinct cutscene milestone.
The user's video must be inspected at representative frames before
timing a rotoscoped animation. The handheld sign, dance frames,
zoom-in cutscene and dance controls have **not** been implemented in
this graphics-only milestone.

## Graphics work completed in this pass

The existing isometric 24×18 navigable grid now draws **16 distinct
cartridge-authored 16×40 illustrated facade surfaces** (eight shop/
architecture identities × two wall angles). Details include coherent
brick courses, light and shadow treatment, recessed glass, window
mullions, timber structure, awnings, lintels, doors and lit interiors.
The architectural types are residential brick, blue stone, half-timber,
art house, storefront, closed storefront, civic archive and Potheads.

The Potheads storefront is anchored to the south-east street's
existing building at world tile **(21,11)**, near accessible pavement
at (20,11). It has a physical leafy green sign reading POTHEADS, not a
floating waypoint UI, and it adds no new collisions or adult-product
sales systems.

The existing shared 38,400-byte Mode-4 EWRAM frame, GBA palette,
single native isometric projection, 3D toggle and linked static RAM budget
checks are preserved. The artwork is stored as 10,240 bytes
of extra GBA cartridge pixel data; the user accepts larger ROMs for
higher-value visual art.

## Acceptance criteria

- Palette, frame guard and proper clipping checks for every façade
  identity, both orientations and both nearby/offscreen positions
- Opening dialogue positively identifies Danny's work and separate
  artistic ambitions; old story objective remains the letter
- Updated real-renderer screenshots near Potheads and the civic area
- Existing full GBA build, host story, saves, studio, combat and
  graphics tests all pass
- Physical 30 FPS performance remains unverified until properly timed
  on an emulator or device; host CPU timings are not a substitute

## Next art/animation milestone (not part of this PR)

Once the isometric artwork is reviewed and the ROM has passed its
performance and story gates: use the provided figure, sign, and dance
MP4 for a storyboard of the aerial camera approach, identify the
reference motion's actual beats/poses, then author a bounded
cartridge-resident multi-pose rotoscoping sequence. The dance must read
as a credible person moving with a cumbersome sign, with visible
contact feet, consistent costume proportions and no effect on
player control or saving before the cutscene concludes.
