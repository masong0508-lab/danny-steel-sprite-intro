# Native acting and traced head refinement

Base: 730dd9c2793401862f2993218873322bdb7c0f69 (emulator capture after PR #55's three successful workflows).

- Separate the planted ground anchor from the torso's existing breathing clock, preserving the witness handoff and album prop timing.
- Use role-offset open/half/closed eyelids and three authored speech mouth poses only while speaking. This is not phoneme lip sync.
- Refine the traced head using its observed orientation: remove the impossible front earpiece in profiles, rotate the material glint, and keep the brows/nose/mouth proportional to the measured size; never invent a blink in the source performance.
- Retain existing story, camera cues, face likeness, reference measurements and single framebuffer. Extend the production raster regression to compare stationary shoe pixels across the breathing boundary. Full GBA build and mGBA capture gate this draft branch.
