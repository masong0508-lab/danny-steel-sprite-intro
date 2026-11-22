# Isometric visible-span traversal optimization

## Problem
The native 240x160 city painter projects and rejects many off-screen 24x18 map tiles twice per frame: once for ground and once for depth-sorted buildings/actors. The existing view is already useful, but emulator page-flip cadence is still well under the target in some scenes.

## Change
- Ground rows now advance X and Y with exact integer +16/+8 increments, skipping initial invisible tiles and exiting as soon as both screen coordinates can only move farther off-screen.
- Depth rows reuse one projected Y and step X by exactly +32, visiting only the valid world span for that diagonal. Skipping a wholly invisible depth row happens after the player draw insertion check; actor depth and walking interpolation stay unchanged.
- Remove the unused per-tile projection helper.
- No work is moved to a deferred heap buffer or extra frame. The same colored pixels and painter order are requested for exactly the same visible world coordinates.

## Verification
The host-only isometric regression enumerates original and optimized draw lists for every one of 432 possible player cells, including off-map sentinel tiles. It compares tile counts and ordered FNV fingerprints of wx/wy/sx/sy for both passes. Existing full gameplay, 2D/3D, player-occlusion, story, SRAM and cinematic tests remain mandatory.

Expected result is fewer per-frame world projections and off-screen checks with identical screen output. This is an algorithmic reduction, **not a claimed measured FPS improvement** until both ARM GBA and mGBA timing evidence are available. No palette, atlas, collision, save-format, audio, cinematic, sprite, room or level changes are included in this performance commit.