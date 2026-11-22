# Danny Steel — runtime, rendering and release validation

## Verified production release — full-reference opening, PR #16

[PR #16](https://github.com/masong0508-lab/danny-steel-sprite-intro/pull/16)
merged as `00fc2b1296ff2b591771e4ee39aa15ec39cb84df`. The main
[build workflow](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/44983584093)
passed all 45 host gates, devkitARM compilation and the static-memory gate.
The main
[capture workflow](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/44983584095)
passed the actual-ROM runtime route and recorded the whole opening in mGBA
0.10.2. The production compiler is devkitARM 16.1.0; the immutable build-image
identity and compiler output are retained in `screenshots/arm_build_toolchain.txt`.

The 899,440-byte ROM was published in commit
`497398be0f5b3904271a98fe746ea493997555da`. Its downloaded bytes were checked
against Git blob `a54395d72c427d2f54c428faa5d55de5fad4f72d` and the exact
ROM SHA256 recorded by the opening-film capture:

`5179c81d3e7bbb951d5a33571757b9be186ded54117b46ec3958264f48423159`.

The production film has 1,434 video intervals over 24.009041 seconds. It reaches
all seven captions through six normal A taps, with zero cinematic/runtime clock
discrepancy, dropped input samples or emulator errors. The separate runtime
probe reports zero failures. The source dance includes the final hand-supported
floor rotations and partial right-edge crop; the held final pose is measured
separately from the moving choreography.

| Production opening phase | Video samples | Page changes | Changes/s |
|---|---:|---:|---:|
| Aerial descent | 181 | 31 | 10.230 |
| Standing/sign work | 569 | 190 | 19.944 |
| Floor work | 523 | 189 | 21.584 |
| Final hold | 161 | 81 | 30.049 |

These finite-window counts include completed pages with identical pixels.
The final hold is not an animation-rate claim. The slowest complete descent
gap is six video intervals (100.456 ms); 30 FPS throughout the opening remains
unmet. This release has not been measured on physical GBA hardware.

The downloaded production ROM was tested again in local mGBA 0.10.5. Its
600-video-frame route windows reproduce CI's 19.810 opening, 14.932 town and
9.955 first-person page changes/s. Complete intervals are respectively three,
four and six video frames. The prior published PR #15 first-person result was
8.461 changes/s with seven-frame intervals. Full timing and hash evidence is
in `screenshots/pr16_release_verification.txt`.

Production static IWRAM is 20,112 bytes (11,064 code + 9,024 BSS + 24 data),
below the 24,576-byte gate. EWRAM remains 38,400 bytes. Source motion is 7,480
ROM bytes and the immutable settled storefront is 22,476 ROM bytes; neither
introduces an additional runtime framebuffer or heap allocation.

## Full-reference opening: local verification

The opening now covers all 548 source frames with 110 measured key poses,
including inversion, changing palm support, repeated floor rotations and the
partial right-edge exit. This pass also preserves the exact first-person
occlusion optimization and corrects hand-held paper/record-player staging.

The final local ARM build uses GCC 14.2.1 (`15:14.2.rel1-1`, 20241119) and mGBA
0.10.5. Its ROM is 903,228 bytes with SHA256
`ac7c726fa4640608e4166a0175144d6f05c04261c3b202305f302d9b77949378`.
The production devkitARM binary is built and measured separately by CI; the
film manifest records that exact compiler/container and binary identity.
Do not substitute local compiler measurements for production evidence.

The normal-input local 24.009-second film retains 1,434 video scanouts, advances
all seven captions using six A taps, and reports zero lost input samples,
emulator errors or cinematic/runtime clock discrepancy. The 600-video-frame
route probe measured 19.710 opening page changes/s, 14.932 town and 9.955 first
person. The opening route includes a partial transition at its window edge;
its complete standing intervals are three video frames each.

A separate breakdown of the full capture keeps the held final image out of the
motion rate:

| Observed cinema-clock window | Video samples | Page changes | Changes/s |
|---|---:|---:|---:|
| Aerial descent, <180 | 181 | 30 | 9.900 |
| Standing/sign work, 180–749 | 568 | 189 | 19.874 |
| Floor work, 750–1273 | 524 | 188 | 21.429 |
| Final hold, 1274+ | 161 | 81 | 30.049 |

These are finite-window page-transition counts; partial boundary intervals
can shift the rates slightly. They include repeated completed pages, not just
changed image content. The aerial descent remains the slowest phase, with a
maximum observed seven-video-frame gap (117.199 ms). This is not a claim of
30 FPS throughout the game or the opening.

Static memory is 20,272 bytes IWRAM (11,224 code + 9,024 BSS + 24 data) and
38,400 bytes EWRAM. The immutable settled-storefront stream costs 22,476 ROM
bytes and adds no framebuffer or runtime cache. It is generated from the same
projected architecture and tested against every pixel at four alignments.
The measured motion itself occupies 7,480 ROM bytes.

All 45 host workflow gates passed locally. The full-duration opening also
passed ASan/UBSan; the actual ARM runtime probe reported zero failures, lost
inputs or emulator errors. Eleven real ARM drawing samples were compared with
the portable painter across aerial, standing and floor phases. CI reruns the
complete source checks, RAM gates, runtime route, presentation measurements
and full film before publication.

Current production evidence lives in `docs/screenshots/opening_film.json`,
`arm_build_toolchain.txt`, `opening_capture.txt`, `emulator_cadence.txt` and
`runtime_verification.txt`. The reports below preserve prior-release baselines.


This gate records what the GBA build actually does and which evidence is
required before publishing it. The current continuation fixes missed input
and lost elapsed time during slow rendering, accelerates existing painters,
and makes the six studio pages reflect the real editor and sequencer.
Save formats, story branches, map collision and the shared 240×160 canvas
remain unchanged. The rendering fast paths must preserve their reference
pixels; deliberate studio presentation changes have separate functional
and visual checks.

## Runtime and presentation contract

- **VBlank captures time and input.** The interrupt records one key sample
  per video frame in a fixed 64-entry `uint16_t` queue: 128 bytes plus its
  counters, with no heap or second framebuffer. It also extends the existing
  music timer and publishes a completed hidden page when one is ready.
  Story mutation, painting and audio sequencing remain on the main thread.
- **Main processes samples chronologically.** A slow draw can delay when
  an action is applied, but its press/release history and elapsed simulation
  ticks are retained within the queue's capacity. At most 64 samples are
  consumed in one call. Overflow discards the oldest sample, retains recent
  chronology and increments a diagnostic; acceptance probes require zero
  overflow. This is a bounded recovery policy, not unlimited buffering.
- **Rendering uses elapsed video frames.** A new draw requires consumed
  input, no outstanding page publication and at least two real video frames
  since the preceding draw started. The old alternating-call idle update
  is gone. This is a pacing rule, not a guarantee of 30 completed frames/s.
- **A complete page reaches VRAM before publication.** The renderer paints
  into the existing 38,400-byte shared EWRAM canvas. DMA3 transfers the whole
  aligned page into hidden VRAM, and only then can VBlank flip the display.
  The two hardware VRAM pages do not require another EWRAM bitmap.
- **Clock continuity is distinct from response latency.** The music IRQ
  extension prevents timer wraps from being lost while main is painting;
  it does not run PSG sequencing in the interrupt. Note events can still
  be delayed until main resumes. Likewise, an accepted button sample may
  need another completed presentation before its result becomes visible.
- **Menu handoff preserves the held button state.** The A press used to
  confirm New Game no longer becomes an extra opening-dialogue press.
  Menu and gameplay transitions are covered by the actual-ROM probe.

The earlier synchronous runtime did not process gameplay every VBlank when
rendering overran. Claims that it guaranteed that behavior were incorrect;
its software tick counter advanced only when main returned from drawing.

## Local ARM validation for the continuation after PR #14

These measurements were collected with **Debian ARM GCC 14.2.1 and the
official devkitPro CRT/specs**, running the resulting ARM ROM in **mGBA
0.10.5**. Both columns use locally rebuilt ARM binaries. The baseline is
the PR #14 source rebuilt with that compiler, not the downloaded PR #14
production binary. Final production acceptance is recorded separately below.

### Presentation cadence

Each window covers 600 emulated video frames. Rates count displayed Mode-4
page changes, including identical content, per elapsed emulated second.
They do not measure host execution speed.

| State | Local PR #14 source rebuild | Local candidate | Baseline / candidate interval |
|---|---:|---:|---:|
| Potheads opening | 14.932 page flips/s | 19.909 page flips/s | 4 / 3 video frames |
| Overhead town | 9.955 page flips/s | 14.932 page flips/s | 6 / 4 video frames |
| First-person town | 1.394 page flips/s | 7.466 page flips/s | 43 / 8 video frames |

The candidate's sampled first-person interval is **133.942 ms**, corresponding
to 7.466 page flips/s, still below the intended 30 FPS. Three scripted samples do not
certify worst-case behavior in every location, studio page, battle or ending,
and no physical-GBA performance result is established here.

### Input and simulation timing

The completed local `runtime_probe` run verified:

- 600 processed simulation ticks for 600 video frames, with queue depth
  0 → 0, in each of the opening, overhead and first-person samples.
- All **12/12 one-video-frame camera taps** retained, compared with **2/12**
  in the local PR #14-source baseline test. Observed simulation-state response was
  1–6 video frames; this is not a measurement of final visible response.
- Four short first-person turns applied once, with observed state-response
  intervals of 1, 7, 8 and 3 video frames.
- Zero queue overflow, one action for a held camera button, correct held
  movement, and successful pause, quit, menu and restart transitions.
- Correct held-A handoff from the menu and exactly one dialogue-page advance
  for a fresh one-frame A press.

All **43 host workflow checks** also passed locally. The actual-ROM probe
uses symbols from the matching ELF and reads state without patching the ROM,
writing game state, injecting cheats or loading a user's save. Companion
screenshots verify that its route reached the intended scenes.

### Linked memory at this local checkpoint

| Static section budget | Measured candidate | CI limit |
|---|---:|---:|
| EWRAM | 38,400 bytes | 196,608 bytes |
| Total IWRAM: `.bss + .data + .iwram` | 17,296 bytes | 24,576 bytes |

The IWRAM total includes fast executable code, the input queue and static
state. These are linked static-section measurements, not measured peak
stack usage. The queue adds no image buffer. Final CI must report the exact
linked sections and ROM bytes for the source that is actually published.

## Verified production release — PR #15

[PR #15](https://github.com/masong0508-lab/danny-steel-sprite-intro/pull/15)
merged as `fd73feee131650313906c44cbf84cbf7bc28afbe`. The main
[build workflow](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/44977185075)
passed all 43 host checks, devkitARM compilation and the memory gate. It
reported 17,264 bytes static IWRAM (9,024 BSS + 24 data + 8,216 code),
38,400 bytes EWRAM and an 860,972-byte ROM.

The main
[emulator workflow](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/44977185083)
passed in mGBA 0.10.2 with zero runtime failures or emulator errors,
12/12 short camera taps, exactly one held-button toggle, all four turns,
timed movement, pause, menu exit and restart. Peak queue loss was zero.
All three clock windows account for exactly 600 video frames. The first-person
window processes 602 ticks while draining two previously queued samples;
opening and town each process 600 with no queue change.

The published ROM was downloaded, verified against its Git blob, and tested
again in mGBA 0.10.5. The previous downloaded PR #14 ROM was checked with the
same three-scene probe and emulator. Both raw runs and SHA256 hashes are in
[`screenshots/published_rom_comparison.txt`](screenshots/published_rom_comparison.txt).

| Published ROM comparison | PR #14 | PR #15 | Complete presentation interval |
|---|---:|---:|---:|
| Opening | 14.932 page flips/s | 19.909 page flips/s | 4 → 3 video frames |
| Overhead town | 9.955 page flips/s | 14.932 page flips/s | 6 → 4 video frames |
| First-person town | 1.493 page flips/s | 8.461 page flips/s | 41 → 7 video frames |

The final first-person interval is 117.199 ms, down from 686.451 ms in the
previous published binary. The local baseline's 1.394 page flips/s and
43-frame interval must not be labeled as the production baseline. The
production result still does not achieve the intended 30 FPS target.

Published ROM SHA256:
`9813e417aa3237f66d6e9c65c5cc906531999b5ba1d24d05482a3f9ad9e75b07`.

## Completed rendering and studio work

| Area | Mechanism and preserved behavior |
|---|---|
| Canvas primitives | Aligned word fills, contiguous full-width spans and ARM/IWRAM primitives retain the exact palette indices and clipping. |
| Isometric tile copies | Inline alignment-safe spans remove thousands of tiny library-copy calls; all 7,168 atlas bytes remain unchanged. |
| Town sprites | Person and tree painters clip bounds once and run their transparency loops in ARM/IWRAM, with unchanged art and foot anchors. |
| First-person walls | Columns with identical original quantized ray directions reuse traversal and shading work while retaining every native column and depth entry. |
| First-person floor | Walls are established first; exact depth comparisons skip only floor pixels that those opaque walls cover. Pixel loops execute from ARM/IWRAM. |
| Page transfer | DMA3 copies exactly one completed 38,400-byte canvas into the hidden VRAM page before display publication. |
| Studio interface | Real patch values, transposed note previews and sequencer events drive the six pages. Correct controls, stopped transport and lyric cues replace misleading feedback. |
| Studio overdraw | Nonoverlapping page bands and thin borders eliminate 46,536 redundant commanded rectangle pixels per frame with identical output for that isolated optimization. |

The full studio presentation pass reduces measured rectangle-write area by
42–51% across its six tabs. For example, NOTES falls from 95,426 to 49,306
commanded pixels, and LIVE from 100,251 to 53,111. These counts describe
renderer work; they are not emulator FPS measurements. No extra framebuffer,
heap allocation or decoded image atlas is introduced.

## Regression and release gates

- [`tests/frame_runtime_test.c`](../tests/frame_runtime_test.c) checks
  chronological taps and holds, elapsed ticks through blocked painting,
  completed-page publication, bounded overflow and counter wrap.
- [`tests/music_clock_test.c`](../tests/music_clock_test.c) checks timer-wrap
  continuity, main/IRQ sampling without double counting, restoration of
  interrupt state and absence of PSG sequencing in the clock callback.
- [`tests/video_present_test.c`](../tests/video_present_test.c) checks the
  full-page transfer contract and page boundaries.
- [`tests/hd_perspective_equivalence_test.c`](../tests/hd_perspective_equivalence_test.c)
  preserves **3,456 complete reference frames** across town positions,
  facings, animation and weather states, plus **136,140** independent
  floor/wall-depth comparisons. Four actual ARM-rendered frames at ticks
  729, 737, 745 and 753 also matched the original 38,400-byte pixel output
  exactly in local verification.
- [`tests/hd_studio_test.c`](../tests/hd_studio_test.c) verifies all six pages,
  stopped transport, visible grid/notes, actual transposed pitches,
  **384** voice-event combinations, palette/guards and read-only rendering.
- Existing exact-pixel checks retain **48,412** rectangle-span cases,
  **25,088** golden tile copies, **500** ellipse and **363** line cases.
  Story, rescue, save migration, music, endings and native graphics checks
  remain mandatory.
- [`tools/frame_cadence.c`](../tools/frame_cadence.c) measures the built ROM's
  opening, overhead and first-person presentation. It is informational and
  must be reviewed with the accompanying screenshots.
- [`tools/runtime_probe.c`](../tools/runtime_probe.c) verifies actual ROM
  timing, input and transitions using its matching ELF. Its assertions are
  a required runtime gate, with source-tagged results in
  [`screenshots/runtime_verification.txt`](screenshots/runtime_verification.txt).
- The devkitARM CI build must pass and emit its real linker-map budgets.
  The screenshot workflow must produce fresh captures and
  [`screenshots/emulator_cadence.txt`](screenshots/emulator_cadence.txt) from
  the same source. A published ROM must then be checked against that source
  and exercised in mGBA before claiming a playable release was updated.

The existing host render-cost probe remains a useful regression indicator
for the CI runner. Host microseconds cannot be converted into GBA FPS.
Future graphics work must keep its visible benefit proportional to its
measured cost, retain these gates, and prioritize the slowest remaining
scene rather than assuming that a scheduled two-frame interval is achieved.

## Historical measurements — earlier accepted builds

### PR #7 optimization acceptance — 2026-11-21

[PR #7](https://github.com/masong0508-lab/danny-steel-sprite-intro/pull/7)
merged after host tests and the devkitARM build passed on its feature branch
([workflow 44936047192](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/44936047192)).
That build measured 38,400 bytes EWRAM; 8,880 bytes IWRAM BSS plus 20 bytes
IWRAM data and zero explicit IWRAM code; and a 604,548-byte ROM. These are
historical totals, not the memory footprint of the current candidate.

Its exact-pixel optimizations carried ellipse scanline radii forward,
replaced per-pixel wall-texture division with a remainder accumulator,
encoded ten immutable town overlays in the existing 24×18 map, and used
single spans for axis-aligned lines. Measured operation checks reduced wall
texture divisions from 11,325 to 300 for 150 heights and ellipse comparisons
from 128,960 to 25,700; 363 line cases and 775 town coordinates matched.
These operation counts did not establish a real-device frame rate.

### PR #13 and PR #14 presentation history

The published PR #13 ROM measured 9.955 opening and 4.977 overhead-town page
flips/s in mGBA 0.10.5. After the art, visibility and first drawing pass,
the published PR #14 ROM measured 14.932 and 9.955 respectively on the same
emulator version. The downloaded PR #14 first-person sample measured 1.493
page flips/s; the local GCC 14.2.1 rebuild measured 1.394. The local and
published three-state comparisons appear separately above; historical rates
must not be presented as current results.
