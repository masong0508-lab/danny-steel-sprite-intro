# Potheads V4 upper-body rotoscope

Private source 307530.mp4 SHA256 unchanged. The optional upper-body
matte is source-guided visible shirt, sleeves, hands and exposed waist,
using LAB foreground differences, the existing measured pose guide,
and five native palette inks. It excludes the earlier costume and
lower-body masks, and preserves the separately authored face likeness.
All 548 source frame times are present. The new layer is used for
546 frames with all three assets available; two empty-costume frames
retain the established articulated fallback. Without either earlier
private asset, the new layer is disabled. No raw reference media is
committed.

Exact vertical merging reduces upper-body drawing rectangles from
71,580 one-row spans to 54,442 (23.94% fewer), peak 194 per frame.
Data: 217,768 bytes plus offset/tick metadata, 288,727 colored pixels,
including 22,258 skin pixels over the complete clip. Moving camera
expands the merged rectangles to the old row projection to retain
exact rounding. Settled camera draws one fill per merged rectangle.

Synchronize authored head, props and contacts to the chosen source
frame when all mattes are active, rather than interpolating between
the visible video frames. Avoid duplicate skeletal arms, chest, and
grip ellipses. Local C decoder tests passed both with private data
and without, including frame selection, clipping, exact row coverage
and non-overlap of the three masks. This is a source-guided matte,
not hand-verified perfect segmentation; residual edges around
the sign and occlusions remain. The preview shows colored body
layers, not final native gameplay. Full ROM build and actual GBA
frame-time checks remain outstanding. Source and private asset
package are delivered separately.
