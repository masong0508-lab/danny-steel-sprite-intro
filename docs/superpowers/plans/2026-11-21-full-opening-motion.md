# Full Opening Motion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Ship a full reference-timed Potheads opening, including the observed floor rotations and partial camera exit, with verified cartridge playback.

**Architecture:** Keep the existing C/Mode 4 renderer and chronological cinema clock. Pack annotated source poses into ROM; render an oriented articulated performer on one continuous projected street set. The root integrates renderer and runtime changes while independent agents annotate the source, review motion coverage and implement an actual-ROM capture tool.

**Tech Stack:** C11, integer native-pixel drawing, Python 3 data generation, devkitARM, mGBA and FFmpeg.

**Spec:** `docs/superpowers/specs/2026-11-21-full-opening-motion-design.md`

## Global Constraints

- Native 240 by 160 Mode 4; palette entries 0 through 63 only.
- No heap, second framebuffer, video decoder, runtime floating point or large RAM arrays.
- Keep static IWRAM at or below 24,576 bytes and static EWRAM at or below 196,608 bytes.
- Source video stays outside the repository; commit the measured JSON, its provenance, deterministic packer and compact ROM header.
- Do not present inferred or clipped boundary positions as observed anatomical joints.

## Review Focus

- Cropped or temporarily hidden limbs: never stretch an absent coordinate to zero; test masks and exact keyframe sampling.
- Inverted and back-facing movement: verify palm supports, rotated head/cloth and limb ordering against source frames.
- Fast page reading or prolonged idle: cue time stays monotonic, B skips and the completed animation clamps; test both routes.
- Moving actor against subtitles: top 135 rows remain identical with and without dialogue; test every caption page.
- Host/ARM differences: actual-ROM capture and cadence measurements use matching ELF and reported compiler/binary identity.

---

### Task 1: Full source motion and safe ROM sampling

**Files:** Create `tools/references/potheads_trace.json`, `src/potheads_motion.h`; modify `tools/build_potheads_trace.py`; test through `tests/potheads_open_test.c` and deterministic regeneration.

**Interfaces:** `pot_trace_sample(unsigned tick, PotPose *out)` produces joint positions, visibility/located/boundary masks, four ground contacts, sign state, bag state and head dimensions. Source coordinates remain in 640 by 360 space in JSON; packed units are three source pixels relative to (300,350).

- [x] Complete measured key poses through source frame 547, with contact/prop events and explicit uncertainty.
- [x] Pack absent points safely, preserving observed boundary locations separately. Interpolate located coordinates only and clamp after the final tick.
- [x] Assert complete source coverage, release states, floor contacts and final clamp. Regenerate the header to a temporary file and require a byte-identical match.

### Task 2: Whole-body opening staging

**Files:** `src/hd_potheads_actor.h`, `src/hd_potheads_set.h`, `src/hd_cinema.c`, `src/hd_overlay.c`, `src/story.c`, `tests/potheads_open_test.c`, `tools/story_gallery.c`.

**Interfaces:** `pot_camera(unsigned t)`, `pot_project(...)`, `pot_scene(...)` and `pot_dancer(...)` compose a single frame. At settled camera, the reference maps to 240 by 135; subtitle pixels start at row 135.

- [x] Make the head, torso, costume and limbs follow measured orientation, visibility and near/far ordering; support the bag fall and sign exit.
- [x] Use the complete source framing and reserve the caption strip. Spread opening cues over ticks `{0,90,180,450,780,990,1188}`; preserve immediate B skip and paid-shift start at 184.
- [x] Compare native images at standing, overhead sign, release, hop, inverted split, bridge, support changes, leg sweep and right-edge crop against source frames.
- [x] Run guarded full-duration drawing and monotonic-clock/skip tests under strict C and sanitizers.

### Task 3: Actual cartridge film and release validation

**Files:** Create `tools/opening_film.c`, `tools/encode_opening_film.py`; update `.github/workflows/build.yml`, `.github/workflows/screenshots.yml` and relevant release documentation.

**Interfaces:** Capture CLI takes ROM, matching ELF, output directory and duration; launches New Game through normal keys, records displayed video frames without memory writes, and emits timing metadata. Encoder uses the captured frame timing.

- [x] Verify the tool on an existing known ROM/ELF and capture the complete updated opening after integration.
- [x] Run all existing host gates plus new motion checks, build ARM, verify RAM, real emulator input/runtime and measured opening/town/perspective cadence.
- [x] Review the full film and independent reviewer findings; fix concrete issues before publishing.
- [x] Upload only explicit source changes, require successful CI, merge the authorized revision and verify the published ROM hash and playback. Deliver the playable ROM and full opening preview.

Release evidence: PR #16 merged as `00fc2b1296ff2b591771e4ee39aa15ec39cb84df`.
Main build `44983584093` and capture `44983584095` both passed. The published
899,440-byte ROM in `497398be0f5b3904271a98fe746ea493997555da` was downloaded,
matched against its Git blob and the full-film ROM hash, and rechecked in mGBA.
See `docs/screenshots/pr16_release_verification.txt` and the production
`opening.mp4`, `opening_film.json`, runtime and cadence reports.
