# Owner-private Potheads costume rotoscope

The original 548-frame recording was recovered and SHA256 checked.
A native 240x135 two-color silhouette was extracted by yellow hue/saturation
selection, connected components and filtered luminance. The full 548
source timestamps are retained. 546 frames have a usable costume outline;
two heavily occluded samples use the established authored drape. No raw
video, original face, audio, background, or actual RGB pixels are stored.

The owner-private header contains 168,876 bytes of span data plus a small
frame/timestamp index. Every span was checked on the host for valid length,
non-overlapping coverage, and the exact settled camera transform. The
descent transform clips to film rows 0..134; no extra canvas or save field.
The renderer draws the visible costume after the articulated body and
before the front-layer sign, preserving real cloth holes and disconnected
folds. The other art and 120 measured anatomical poses are unchanged.
The private build may need further visual alignment and emulator cadence QA;
segmentation is approximate, not verified pixel-perfect art.

Install the owner-private package by extracting its src/potheads_cloth_data.h
into this branch's src directory, then use the normal clean GBA build.
The optional include is automatically enabled when that header is present;
a source-only checkout uses the existing procedural painter. The supplied
standalone extraction script regenerates the exact asset from the matching
private input, with OpenCV, numpy and ffprobe.

Source-only work also anchors planted palms to measured contacts (bounded
to nearby wrists) and orients shoe highlights along their motion rather
than always toward screen-down. Existing save and source timing remain.
Local projection/asset checks passed. Full ROM build, native emulator film
and actual GBA frame timing have not been verified.
