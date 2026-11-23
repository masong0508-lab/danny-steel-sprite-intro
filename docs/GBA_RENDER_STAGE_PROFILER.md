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
