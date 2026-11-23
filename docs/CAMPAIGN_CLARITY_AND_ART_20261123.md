# Campaign clarity and visual navigation pass — 2026-11-23

## What was actually wrong

The opening chapter had the right physical places but weak *cause and effect*: the
letter, continuity office, Mara, tape and roof could feel like a checklist of
unrelated props. In particular, selecting **GO** on the bus with the three
core tasks complete but either creative task unfinished put Danny back in the
Old Quarter without explaining why he did not depart. The HUD called all five
destinations "NEXT", implying two optional legacy tasks were necessary to use
the bus. The little objective diamond also offered no clear painted
indication of the next walkable step toward it.

## Changes in the real GBA game

- **Opening stakes:** Danny explicitly connects the clinic's unanswered
  calls with his fear; the next steps are stated as letter, clerk, Mara.
- **Letter / clerk:** the old address on the letter is the reason for BORE's
  existence proof, stamp and release of the old tape from the archive. BORE is
  still a funny bureaucracy, but its function is understandable.
- **Mara:** Danny explicitly tells her about the missed call and promises
  to call before boarding; the existing *finish her conversation using A*
  gate still controls the real promise flag. B never silently grants it.
- **Music / art:** the reel carries Danny's own name, connects to the song
  Mara remembers, and the roof depicts the skyline outside her cafe.
- **Bus / clinic:** reaching the bus with only the core tasks completed is
  labelled **TRY TO GO**. Danny's clinic callback interrupts the trip in a
  new five-page DAY ONE scene; he explicitly decides to stay and respond.
  The CHOOSE-STAY route still remains free exploration. If both legacy
  objectives were completed, the choice is **BOARD** and remains the
  score-length good ending.
- **Appointments:** earlier concerning scans and subsequent confirmation
  are different encounters. There is no claim of a newly discovered
  cancer becoming advanced over one in-game day.
- **Compact quest tags:** CORE / EXTRA / BUS / DAY / CARE / LAST /
  SAVE / HOPE are derived entirely from already-existing story state and
  are visible in native and legacy HUDs and field guides. The route and
  objective flags are unchanged. Extras are explicitly identified as
  optional for reaching the bus.
- **Isometric art:** one glinting but low-cost chevron is painted into the
  *next walkable square* of the current shortest path; the existing target
  glyph gains a high-contrast notch. Both are depth-sorted with buildings
  and never float on top of roofs. Only the current step and target are
  decorated. Native pixels are still 240×160 on the same buffer.

## Saving / resource limits

No story struct fields, SRAM versions, event-bit semantics, map tiles,
new image data, dynamic allocation, extra screen buffer, collision changes,
texture ROM, music memory or character animation tables changed.
The painting overlay adds a few bounded strokes and a cached route query to
the existing depth pass. Release requires the host story + saving +
render pixel guards, ARM linker budgets and ROM build; the remaining
on-device FPS still needs an actual GBA measurement.

## Validation

`tests/story_campaign_clarity_test.c` checks pure quest kind/reason queries,
core and extra stages, the bus after core, actual neutral phone interruption,
the STAY choice, actual good boarding, the native different choice cards and
guarded Mode-4 world route marks. Existing navigation, rescue, audio, save,
cinema and golden perspective tests remain in the full build workflow.

## Next campaign work (not claimed complete in this pass)

1. Replace the high-speed two-appointment time jumps with *playable* scenes
   and trusted conversations that let Mara and Danny have time to react.
2. Expand Potheads shifts, neighborhood people and creative work into connected
   optional subplots, not repetitive identical interactions.
3. Build materially distinct choices that change conversations and reachable
   city scenes without rewarding cruel behavior or retconning the rescue.
4. Improve visible character acting, specific lighting and silhouette readability
   using native low-memory pixels; profile the walking camera instead of
   spending more rendering cycles blindly.
5. Validate a complete start-to-ending emulator playthrough and save migration
   before promoting the campaign from "first playable chapter" to complete.
