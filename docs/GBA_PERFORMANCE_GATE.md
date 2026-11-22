# Danny Steel — runtime, rendering and release validation

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
