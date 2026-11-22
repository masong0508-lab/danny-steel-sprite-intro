# Full reference motion in the Potheads opening

## Intent

Continue the authorized Danny Steel game improvement and testing pass. The user's specific correction is that the reference contains much more than a hop: its cartwheel, supported floor turns, leg sweeps and travel out of the camera must inform the opening. The full source is `307530.mp4`, 548 decoded frames, approximately 18.3 seconds. The existing excerpt ending at frame 280 is insufficient.

## Approach and decisions

Use the existing native C renderer with a measured, timed landmark track covering every motion phase. This is the implementation choice for this pass. Embedding decoded video would undermine the cartridge's memory and art constraints; a small invented pose loop would lose the choreography the user specifically requested. A full hand-painted raster atlas would cost substantially more ROM and make corrections harder. The chosen track retains explicit visible, inferred, cropped-boundary and unavailable distinctions; clothing and lighting remain authored pixel art.

The aerial opening uses one continuously projected storefront, settling after 180 video ticks. Thereafter the source's 640 by 360 analysis viewport maps uniformly to 240 by 135 native pixels. Preserve its crop and translation, including its partial exit at the right edge. Do not re-center the performer or invent unseen joints. Rows 135 through 159 hold the complete two-line dialogue, speaker, progress and A/B hints. Keep current text and story meaning: Danny is grateful for the income, although he does not love this job.

Measure the whole dance, sign release, two-hand inversion, one-hand bridge, arm sweeps, changing support, bag fall and repeated leg sweeps. Add explicit palm and sole contacts. Keep the sign gone once it exits the reference. Keep the bag on the pavement after its observed fall. Body, head, loose costume, limb thickness and draw order must follow orientation and near/far movement; an upright costume pasted over an inverted skeleton is unacceptable.

The animation runs on the existing chronological cinema clock and clamps at its final source pose. It does not loop back into a standing dancer. Existing A page-advance cues remain skippable and monotonic; cues span the floor work as well as the standing dance. B continues to skip immediately. A viewer who supplies no input sees the whole dance while the dialogue waits.

## Constraints and acceptance

- Native 240 by 160 Mode 4; palette entries 0 through 63 only.
- No heap, second framebuffer, video decoder, runtime floating point or large RAM arrays.
- Keep static IWRAM at or below 24,576 bytes and static EWRAM at or below 196,608 bytes.
- Source video stays outside the repository; commit the measured JSON, its provenance, deterministic packer and compact ROM header.
- Do not present inferred or clipped boundary positions as observed anatomical joints.
- Verify the actual ARM ROM and matching ELF, native picture sequence, story routes, bounds and palette. Host speed is not GBA frame rate.
- Preserve and verify the independent perspective optimization and small cutscene prop/contact fixes already underway.
- Capture a full opening film from normal emulator input, with no writes to game memory, and publish a verified playable ROM through the established repository workflow.

## Review risks

Missing joints must never interpolate toward zero. Hand support, near-leg changes and head orientation must remain readable through inversion and back-facing turns. The caption must not cover the observed ground-contact rows. Held or dropped props must not teleport or attach permanently to the wrong body part. The final camera crop must not be replaced with a fabricated complete exit or a restarted loop.
