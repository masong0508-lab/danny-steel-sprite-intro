# Countryside brush and hikes

This is a follow-up to the continuous city/countryside branch. It keeps story
coordinates and the existing generated heightfield intact.

* SELECT in ordinary countryside exploration toggles landscape editing.
  A raises and B lowers the nearest two-tile control point; L still changes
  camera. There are 141 editable control points with four bounded grades.
  SELECT exits; R exits and opens notes. Water-edge vertices and the city
  seam remain fixed. Editing is suspended by urgent plot stages.
* Both views read the same derived height mesh, rebuilt only when the
  terrain settings or current slot's brush controls change. Existing survey
  plots remain available; no additional framebuffer is allocated.
* The outdoor isometric scene starts on the eastern city approach and uses
  a ground pass followed by depth-sorted objects and the actor. Urban
  first-person views on the approach sample the actual distant hills.
* A physical trailhead at (42,7) starts an optional guided hike. Follow the
  meadow checkpoint (49,6), ridge checkpoint (53,13), and summit (56,11).
  The summit grants a one-time inspiration reward; the normal art activity
  remains available afterwards. Rescue/clinic objectives supersede the
  hike, and finishing/cancelling restores ordinary objective tracking.
* Terrain edits and hike progress are CRC-protected and slot-tagged in a
  new companion SRAM record, separate from existing save records. Older
  saves start with neutral brush controls and no hike progress.

Standalone generator/model checks sampled 74,400 elevation vertices across
16 previous terrain settings and six representative control-grid patterns.
The observed maximum height was 82 and adjacent vertex difference was 20;
protected vertices were unchanged. This is not an exhaustive proof over
all possible grids. The cartridge build and native input/render tests must
pass before distribution; outdoor FPS and actual emulator images still
require inspection.
