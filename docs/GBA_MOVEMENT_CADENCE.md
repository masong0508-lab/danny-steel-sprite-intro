# GBA first-person: moving versus stationary presentation cadence

The first-person wall-shade cache in PR #32 substantially improves an unchanged camera pose, but that does not guarantee the same FPS during movement or turning. The original 600-emulated-video-frame probe checked only the Potheads first-person camera held stationary.

This follow-up adds a separate **real held-walking route** in the same source-matched, read-only mGBA cadence probe, after the original three unchanged baseline windows. Danny starts at the original playable Potheads pavement (20,11) facing east. The existing east-west lane is walkable from x=12 through x=20 at y=11. The probe alternates 48 video frames holding DOWN (walking west/backward) and 48 holding UP (walking east/forward), repeating during 600 emulated frames. No map, save, gameplay, fake input speedup, camera code, hardware timer, audio, framebuffer or emulator settings are altered.

The new output labels the result **first-person held walking (48-frame back/forward)** separately from the stationary first-person rate. Screenshots are captured before and after the movement window. The same ordinary controller input API is used as for the existing validated route, and the probe measures **actual Mode4 page-select changes**, not Linux host FPS.

Accept the measurement only after the screenshots show Danny stayed in the intended navigable lane and the real-ROM runtime probe reports zero errors/lost inputs. A 50% stationary-view improvement must not be presented as a blanket 50% walking improvement if measured moving cadence is lower.

This is a diagnostic-only change to tools/frame_cadence.c; it does not modify the GBA game source or its published ROM.

## Verified 600-video-frame comparison (same input route)

The source-matched read-only mGBA workflows recorded:

| Camera state | Before cache, PR #30 (source 066f9f2) | With cache, PR #32 (source 00e1842) |
| --- | ---: | ---: |
| Opening | 19.810 page flips/s | 19.810 |
| Overhead town | 14.932 page flips/s | 14.932 |
| First-person standing | 9.955 page flips/s | **14.932** |
| First-person held back/forward walking | **6.570** page flips/s | **6.570** |

Evidence: [uncached movement capture](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/45090819316) and [cached movement capture](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/45090414920). Both native-ROM runtime verification probes reported zero failures. The result supports a 50% stationary improvement with **no measured walking regression**, not a claim that movement is now faster. The walking cadence is the next optimization priority; retain exact native graphics and 6,750-byte EWRAM cache limits when pursuing it.

The PNG workflow rendered movement start/end pictures at 240×160; source-branch publication may be skipped when a newer docs/source commit arrives during capture. The resulting video-frame counts and emulator runtime evidence above come from completed workflow logs, not guessed screenshot timings.
