# Potheads opening: full reference motion

## Character and place

Danny Steele works as Potheads' banana-costumed sidewalk signholder. He does
not love the shift. He is grateful it pays the rent and gives him enough
stability to make paintings and music after work. The seven existing dialogue
pages keep that distinction and the doctor, Mara and last-bus story intact.
Potheads remains a real, navigable Old Quarter storefront.

## What the video actually contains

The reference is `307530.mp4`, 1280×720, 548 decoded frames. Its video runs from
PTS 0.048800 to 18.273644 seconds. The complete file's SHA256 is
`9181173802c83b52e945ea2f7043eb3367ecbf78401c0f76bdf9911fca8971b8`.
The original video remains outside the repository and cartridge.

`tools/references/potheads_trace.json` contains 110 manually estimated source
poses: 47 standing/sign poses and 63 later floor poses. Every sample retains its
source frame, timestamp, anatomical left/right landmarks and uncertainty.
Samples between those measured poses are interpolated in the game. This is a
source-guided articulated rotoscope, not a claim that every source pixel or
all 548 frames were individually traced.

| Clip-relative time | Motion preserved |
|---|---|
| 0–3.3 s | Side steps, alternating weight, torso lean and the large arrow sign behind the shoulders |
| 3.3–7.1 s | Turns, overhead sign rotations, changing grip and partial top-edge cropping |
| 7.0–7.6 s | Sign release, fall toward the left edge, then disappearance |
| 7.6–9.5 s | Arm flourish, back-facing turn, hop, landing and deep fold |
| 9.5–10.9 s | Hand plants, kick into a scissoring inversion, briefly one-handed support, feet descending |
| 11–13.3 s | Raised bridge/crab, broad free-arm sweeps and shoulder bag fall |
| 13.3–16.5 s | Palm changes, prone leg sweep, compact crouch and repeated front/back floor turns |
| 16.5–18.22 s | Hand-supported travel and another turn through the camera's right crop |

The final source frame still contains part of the performer. The game preserves
that partial exit and clamps its last measured pose; it does not invent a
standing recovery or claim an unseen completed exit. The sign never returns
once it leaves the source. The bag follows its observed fall and stays on the
pavement near source coordinate (174,344).

## Camera, artwork and captions

The first 180 cinema ticks descend through one continuously projected street
set. Roof, facade, awning, doorway and pavement use the same world geometry
throughout the descent. There is no horizontal wipe between unrelated sets.

At the settled view, the 640×360 analysis frame maps uniformly to a 240×135
native film area. The source's top, bottom and right-edge crops remain intact.
The performer is not shrunk or re-centered to hide cropped limbs. The bottom
25 rows contain the speaker, two complete dialogue lines, page markers and
**A NEXT / B SKIP** hints, leaving the floor contacts visible.

Dark sleeves and full trousers, an oriented human head, headphones, shoes,
loose yellow fabric, the bent lettered arrow board and the shoulder bag are
native pixel artwork attached to the measured motion. Body and head orientation,
near/far leg order, palm/sole support and foreground leg width change during the
floor sequence. Clothing planes and lighting are authored; they are not
segmented video textures.

Visibility has four distinct meanings:

- Visible anatomical landmarks retain their observed positions.
- Covered joints may have explicitly inferred positions under clothing or a prop.
- Cropped contour intersections are stored separately from anatomical joints.
- Unavailable joints remain null and do not become coordinates at the origin.

Cropped endpoints receive no fabricated hands, shoes or joint caps. Only active,
annotated support points receive contact shadows. Both feet can be airborne
while the character is supported by one or both palms.

## Timing and gameplay continuity

The measured track uses source-relative 60 Hz ticks and ends at tick 1093.
The camera plus motion completes at cinema tick 1273, approximately 21.3 seconds
at GBA video cadence. No input is needed for the full movement to play.
Animation continues while a reader remains on a dialogue page.

A advances text and retains the existing monotonic editorial cues, now spread
across `{0,90,180,450,780,990,1188}` to include the floor work. An already advanced
clock never rewinds to a page's cue. B skips immediately. Repeated paid-shift
visits begin at tick 184, after the establishing descent.

A fresh game starts at walkable tile (20,11), facing Potheads at (21,11).
Closing the opening hands control to the same location. Existing save positions
are preserved. **A SHIFT** opens **WORK / LEAVE** on eligible life-sim days;
B cancels. Confirming work grants 2 work and consumes 1 energy once per day.
Exhaustion and the existing missing-person/phone crisis rules still take
precedence. There is no new save field or schema version.

## Cartridge budget and validation

The motion header packs 110 poses into 7,480 bytes of immutable ROM data.
It uses integer interpolation, the existing 64-color palette and the existing
240×160 shared page. There is no video decoder, heap allocation, second canvas
or runtime motion cache. The projected architecture is retained for the moving
camera; the identical settled view is replayed from a 22,476-byte generated
ROM word stream to avoid rebuilding static geometry on every dance frame.

`tools/build_potheads_set.py` regenerates the settled storefront stream. Its
test compares all 38,400 pixels against the live projected painter, verifies
stream boundaries and palette entries, and checks four output alignments.

`tools/build_potheads_trace.py` regenerates `src/potheads_motion.h`. CI requires a
byte-identical result and rejects a release trace that stops before source EOF.
`tests/potheads_trace_pack_test.py` protects cropped-joint sampling and rejects
the obsolete hop-only excerpt. `tests/potheads_open_test.c` checks full-duration
palette/buffer safety, source-specific supports and turns, prop disappearance,
final crop/clamp, caption isolation, clock monotonicity, skips and shift rules.

The gallery includes the inverted split, bridge, support transfer, leg sweep
and late right-edge crop. `tools/opening_film.c` captures the actual ROM's
completed mGBA scanouts from the first opening frame with a matching ELF. It
uses normal menu inputs and optional timed A taps; it never writes emulated
game memory. `tools/encode_opening_film.py` preserves the original video timing
and verifies frame count, dimensions, duration and source hashes. The review
film is silent; sound remains enabled and emulated in the cartridge.

See `docs/screenshots/opening_film.json`, `opening_capture.txt`,
`emulator_cadence.txt` and `runtime_verification.txt` for the production build's
capture provenance and actual timing. Host render speed is not GBA frame rate.
