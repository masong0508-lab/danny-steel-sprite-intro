# Actual GBA first-person rendering cycle profiler

## Purpose

Repeated native 240×160 rendering optimizations have preserved all 3,456 golden first-person images and passed real mGBA input/ROM checks, yet the standard camera route remains at **9.955 presented pages/second** (six video intervals). CPU host benchmarks cannot explain where the actual ARM7TDMI frame time is spent. This tool measures the stage costs *inside an instrumented ARM GBA ROM*, not via Linux wall-clock performance.

## What it measures

Only when built with `-DGBA_PROFILE_RENDER`, the first-person draw starts GBA Timer0 at full CPU speed and cascades Timer1 for a 32-bit counter. It records the elapsed CPU cycles of:

- Sky/horizon
- Wall casting and facade shading
- Perspective floor/material shading
- Billboards and wall occlusion restoration
- Rain/weather
- Entire first-person renderer

All global symbols begin with `gba_profile_` and are absent from **normal production builds**. The instrumented build uses 28 bytes of debug counters, not an additional framebuffer. The normal game still uses Timer3 for PSG music; the debug probe uses only Timer0/Timer1, which the current game does not otherwise use.

## Real ROM workflow

The dedicated GitHub Actions workflow `.github/workflows/renderer-profile.yml` compiles a separate *non-published* ARM GBA ROM with the profiling flag in the official devkitARM container, starts the ROM in mGBA, follows the established New Game → Potheads → overhead → first-person control route, and reads the profiler counters by exact symbols from its matching ELF. The probe reads memory but never patches the ROM, saves, emulated inputs, display clock or gameplay state; input presses are ordinary controller buttons.

The tool reports completed first-person samples over 600 emulated video frames and min/mean/max CPU cycles per stage. This is an **emulated ARM timing diagnostic**: it informs optimization priorities but does not replace the separate page-flip cadence report, runtime verification, visual goldens, linker RAM budget or eventual physical-console FPS test.

## Acceptance

The ordinary `make` build must remain bit-identical at source level (no profiling code linked when the flag is absent), with the original tests and screenshot verification. The profiling-only build must compile, reach Mode4 first-person, gather at least 20 complete render samples, and print plausible per-stage CPU cycles. Never merge a graphics or FPS claim merely because profiler counters fall; require fresh full-ROM cadence and image checks.

## Verified baseline reading

On the instrumented build from profiling workflow run [45088515956](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/45088515956) (source `b8c4fe5`), mGBA recorded **100 complete first-person frames in 600 video frames** with zero missed render generations. Each row is the mean across those completed frames:

| GBA stage | ARM cycles | Time at 16.777216 MHz |
|---|---:|---:|
| Walls and depth | 899,630 | 53.622 ms |
| Floor and materials | 263,721 | 15.719 ms |
| Sky and skyline | 204,752 | 12.204 ms |
| Weather | 42,820 | 2.552 ms |
| Billboards and occlusion | 1,657 | 0.099 ms |
| **Complete native renderer** | **1,412,580** | **84.196 ms** |

Thus walls account for **63.7%** of first-person renderer time on this representative view, more than three times the floor. This also explains why repeated floor- and ray-step micro-optimizations were insufficient to cross the next presentation interval. The follow-up profiling pass now measures wall texture *painting* separately from cell traversal so the next CPU optimization targets the right operation. These measurements are from an emulated ARM cartridge with instrumentation overhead, not physical-console timing or host CPU performance.
