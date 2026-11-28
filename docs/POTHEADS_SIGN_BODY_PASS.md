# Potheads — sign and body refinement
The existing Studio Mixer ROM (source f9228c7, 3,328,460 bytes) was inspected by decoding its real ROT3 cache; the reference video is not embedded or re-annotated. The original 120 measured pose keys and all joint/contact coordinates are unchanged. Two additional flags preserve whether the arrow tip is actually visible and whether the board is clipped, taken directly from the existing JSON annotations. The sign uses a convex envelope of the measured vertices and an observed tip only; text is attached to the measured face via two fixed-point textured triangles. Thin, folded or clipped poses deliberately omit unreadable text rather than scatter glyph pixels outside the board. Behind-torso layering, observed release and crop timing remain intact; a visible wrist is redrawn over a front-held sign only when it is within the measured board envelope. Tapered connected sleeve/trouser panels follow the measured shoulder/elbow/wrist and hip/knee/ankle positions, with the original fallback for missing/cropped joints; reduce excessive foreground thigh thickening and add a gravity-following tunic seam. Optional owner-private body mattes retain their existing path. The native frame cache is still 30 Hz sampled and lossless; generation now searches multiple sparse span gaps and stronger host-only LZ4 compression to retain the 1.2 MiB budget. Build tests exercise geometry, source packing, all original floor poses, cache equality and emulator cadence. No claim of newly observed reference motion or measured hardware performance until verified.

The second refinement requires a minimum projected face height at both ends of the sign and uses a bounded underpaint bevel at each connected joint. The raster test draws the actual production sign and limb functions at two camera positions across the measured timeline, checking clip bounds, all written palette indices, guard bytes, and coverage at visible joints.

## Contact and surface pass
The verified 9e00f05 cartridge measured 29.366 page flips/s, and its ROT3 clip was 1,163,260 bytes. Inspection of the decoded pictures showed abrupt blank board faces and simplified wrist/shoe caps. A graded printed-face mode retains quieter lettering at valid moderately foreshortened angles; severely folded/cropped areas receive only a narrow material rim, with no false outline along a measured crop edge. Floor palms and soles now use the actual source support coordinates; a bounded wrist-to-palm connection and a compact planted shoe wedge replace large circular caps. All measured coordinates, masks, timing and private matte paths are retained. Dedicated tests check every sampled projected sign and visible floor contact, plus the existing raster and compression gates. No new source measurements are claimed.

## Motion continuity
Verified 2c1bf791: 29.565 page flips/s over the 600-frame opening benchmark; no dropped inputs and at most one tick of film/clock error over the 24-second capture. The next pass uses bounded, signed monotonic cubic interpolation between compatible joint keys, with secant-limited tangents and exact measured endpoints. Changes of joint visibility, support, facing and depth retain the original interpolation; private matte frames use the original pose unchanged. No new reference measurements. The visible-to-unobserved arrow-tip transition now converges on the corresponding measured envelope edge rather than the generator's placeholder coordinate; the four measured sign corners are unchanged. An arithmetic and full-timeline regression checks endpoints, monotonicity, support invariance, matte equality, and tip bounds. The original clip limit, cache equality and emulator gates remain mandatory.

## Projective sign and anatomical silhouette pass (November 27)
The source-only render now applies a single planar inverse mapping to the
sign's print. It no longer has two independently affine halves with a
diagonal change in lettering scale. Extremely skewed/singular faces remain
paper-only. Well-supported unclipped near-parallelogram faces can receive a
one-pixel-per-axis corner correction, preserving centre and observed arrow
tip; private-matte builds bypass that correction. No measured key, crop,
contact, visibility flag or sign-release timing is modified.

Torso cross-sections use the measured shoulder and hip centres, a shared
perpendicular body axis and bounded width, preventing a twisted bow-tie
shirt when estimated hip ordering reverses. The fallback costume hem is
scaled to the measured torso, and tight elbow/knee reversals now taper
both bones to their actual distal widths rather than inflate forearms and
ankles. These are authored contour improvements, not a fresh video trace.
The original reference and private matte assets were not found in the
accessible file library. Re-review the actual opening film before claiming
visual approval. The host cache builder evaluates ten span-gap options,
choosing by padded size then raw packet length; existing ROT3 format,
30-Hz sampling, scene limit and runtime RAM allocation are unchanged.

### Cadence follow-up
Emulator capture of the first projective pass succeeded, but the opening
benchmark fell to 26.579 presented pictures/s from 29.466. The cache
became 13,180 bytes smaller. Replace per-pixel divisions with exact
incremental rounded quotients, and permit bounded eight-sample cache
catch-up for small stalls. No new buffers or frame dropping. These
changes must pass the full host, cartridge and emulator cadence gates
before release; cause of regression and final performance not yet proven.
