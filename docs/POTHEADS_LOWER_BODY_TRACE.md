# Potheads lower-body source rotoscope

The optional owner-private asset replaces the two articulated leg strokes
and synthetic hip bridge with foreground-guided visible trouser and
shoe geometry at all 548 exact source timestamps. The last three
frames have zero spans and preserve the observed departure, not a
fallback limb. The separate authored upper body, head, sign and bag
remain. The original video and private palette data are not committed.

The extraction uses stationary-scene median subtraction, the reviewed
120-pose guide, constrained component selection, and five authored inks.
The private asset contains 444,081 bytes of nonoverlapping run data,
148,027 spans, peak 402 per frame, 866,440 painted pixels. There are
545 visible frames and three intentionally empty frames. This is an
approximate matte, not a fully segmented person or verified final ROM.

The settled camera fast path is shared by both source-derived layers.
Local C tests verify every possible valid settled lower-body run,
all actual private spans and exact frame ticks, no duplicate pixels,
crop and bounds, and the source-only fallback. Hardware frame timing
and full cinematic rendering still require successful game builds.
