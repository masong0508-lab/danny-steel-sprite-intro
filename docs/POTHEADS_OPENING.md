# Potheads / opening direction and implementation

## Character first

Danny Steele works as Potheads' banana-costumed sidewalk signholder. He does
**not** especially like the costume or the shift. He is grateful the hours
pay the rent and give him enough stability to make paintings and music after
work. This is the tension of the scene: physically comic sign-spinning over
a little tiredness, pride in surviving, and an artist's private ambitions.
The storefront is a real landmark in the navigable Old Quarter, not a
fantasy stage or a substitute for Danny's artistic ambitions.

## Staging at native 240x160

1. **0–76 frames:** distant bird's-eye-style approach. Repeated roof
   silhouettes, street grid, and short-shadowed scale cues establish a
   lived-in neighborhood and the location's place in it.
2. **76–132 frames:** continuously eased descent to the real Potheads
   awning, doorway, green fascia and sidewalk. Danny emerges as the
   foreground focus, already tethered to his hand-painted sign.
3. **132+ frames:** full-resolution hero composition. The lettered
   *POTHEADS / COME IN* placard remains connected to the raised hand.
   The camera holds during the dance so faces and handwritten words
   remain readable beneath the original dialogue overlay.
4. **Eight sampled poses:** arms, body lean, bob and heel steps blend with
   fixed-point Hermite interpolation. Both the elbow-to-hand skeleton
   and sign are rendered from the same transformed joint coordinates.
   This is *an authored rotoscope-ready approximation*, **not** a
   frame-by-frame trace of the supplied reference video. Exact
   source-video matching will require annotating frame times and
   pose locations from the video.

The scene uses a deliberately superimposed 2-D cardboard sign over a
grounded character. Inked corners, paper texture, tape, a slight offset
shadow, and a foreground hand grip preserve that collage quality while
preventing a visually disconnected floating rectangle. The costume
features asymmetric banana seams, fold highlights, a face aperture,
neutral brows and grounded shoes. Wet reflections, masonry, window
shelving, a local OPEN light and moving pedestrians give the business a
recognizable physical context.

## Gameplay continuity

A fresh game now starts at walkable tile (20,11), facing east toward
Potheads (21,11). The moment the opening dialogue closes, the normal
isometric camera picks up in front of the very same business. Old saves
are resumed from their saved positions, not relocated by this change.


The solid shop facade occupies world cell (21,11). From the walkable cell
(20,11), face east and press **A SHIFT**. Before the life-sim day begins,
this is an optional character line. In an ordinary life-sim day, a paid
sign shift adds **2 work**, consumes **1 energy**, and is available only
once per day. An exhausted Danny cannot work it. The existing daily_mask
byte uses unused bit 5; the five primary daily memories and endgame
thresholds are unchanged. This is never a mandatory quest gate.

When Mara is missing or the two-day unanswered-phone crisis is active,
the shift is unavailable and the grounded scene is shown without the
bright dancing sign routine. On later visits to an ordinary workday, the
camera resumes from a stable close-up rather than replaying the entire
establishing shot. Completing the shift cannot be farmed for work points
by repeating A.

## Engine and performance budget

- Same 240x160 Mode-4 shared framebuffer and existing 64-color palette.
- No new sprites, full-screen animation frames, tile atlases, EWRAM page,
  heap allocation, save schema version, floating-point renderer or
  streamed media.
- Body and background share one camera scale and center per frame. All
  geometry is clipped through existing native pixel primitives.
- Signed 8-bit pose samples and simple integer arithmetic keep the
  animation portable and deterministic.
- Easing is performed on frame counts. Source footage is not loaded
  by the GBA; only the final compact bone/pose data will be.
- The pose samples are provisional until compared against the actual
  reference video. Host-render tests do not establish emulator FPS.

## Regression and screenshots

`tests/potheads_open_test.c` covers palette and buffer bounds, progressive
camera frames, all eight dance beats, the grounded board, the seven-page
new opening, shift resource accounting, one-per-day protection and
missing-person story priority. It runs in the normal GBA build workflow.

`tools/story_gallery.c` renders establishing, descent, close-up, dance,
grateful-work and after-hours-art frames using the **production C renderer**
(no composited concept art). The screenshot workflow also performs a
separate headless mGBA ROM capture and refreshes the gallery.
