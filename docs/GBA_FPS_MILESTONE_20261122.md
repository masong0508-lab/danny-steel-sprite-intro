# Danny Steel — November 22, 2026 first-person rendering milestone

## Completed and merged

The original game, save files, three narrative endings, Potheads opening, Studio editor, Mirror Orchard side story and 24×18 Old Quarter layout remain intact.

- [PR #29](https://github.com/masong0508-lab/danny-steel-sprite-intro/pull/29): real ARM/mGBA renderer stage profiler, which is compiled **only** with GBA_PROFILE_RENDER and never included in an ordinary GBA release.
- [PR #30](https://github.com/masong0508-lab/danny-steel-sprite-intro/pull/30): the original native wall painter now writes two identical palette pixels at once through aligned EWRAM halfwords. The real ARM profiler measured wall-paint cost dropping from 839,567 to 774,004 cycles per frame with the reference image preserved.
- [PR #32](https://github.com/masong0508-lab/danny-steel-sprite-intro/pull/32): a 45-ray × 150-shade, **6,750-byte EWRAM** cache stores the already-authored wall ink only when the player's exact projected camera pose and direction are unchanged. Sky, rain, actors and other visual layers still update normally. Movement and turning invalidate the cache.

## Verified emulator result

The 600-video-frame Potheads-route mGBA probe measured:

| Scene | Previous first-person graphics | Current GBA source |
| --- | ---: | ---: |
| New Game / opening | 19.810 page changes/s | 19.810 |
| Overhead Old Quarter | 14.932 page changes/s | 14.932 |
| First-person, unchanged camera | 9.955 page changes/s | **14.932** |

The stationary first-person rate improved by **50%**, from six to four emulated video intervals per presented game frame. This is presented-Mode4-page cadence, **not measured physical-console FPS**, and does not certify the same improvement during movement.

Source-matched profiling of the cached first-person camera reported 150 completed frames over 600 video intervals, wall paint 209,682 CPU cycles/frame and complete rendering 783,002 cycles/frame (46.671 ms). The prior full renderer was about 1,414,746 cycles (84.325 ms) in the comparable instrumented test.

## Memory and regression gates

- Static EWRAM **45,152 bytes**, project limit **196,608 bytes**.
- Static IWRAM **22,104 bytes**, project limit **24,576 bytes**.
- Cartridge image measured **913,764 bytes** in the source-branch ARM build.
- **3,456** complete independently recorded first-person golden frames matched.
- **9,200** guarded original-versus-aligned wall span cases matched.
- **120** cold-versus-cached native full-frame comparisons matched across five town positions, four facings and multiple clock states.
- Host story, save migration, cutscene, gameplay input, Studio and rendering tests passed. Real-ROM mGBA runtime verification returned zero failures and no queue overflow.

## Next milestones

Measure the held-walking/turning first-person route separately, optimize moving camera performance without geometry pop-in or wobble, and then expand cinematics, writing, playable districts and port parity. The existing GBC companion project remains a title-screen prototype; this GBA renderer work must not be claimed to have improved other ports. Maintain the 240×160 native frame, SRAM compatibility, actor visibility and exact visual regression tests as release gates.

The normal main-branch GBA build workflow publishes the tracked dist/danny_steel_title.gba when it confirms the exact latest source. A new source/docs commit is **not proof that the binary has been republished**; verify the final main build and binary before distributing it as a new release.
