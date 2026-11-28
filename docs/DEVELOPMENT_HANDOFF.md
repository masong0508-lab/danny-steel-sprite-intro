# Danny Steele — active development handoff

**Updated:** 2026-11-27. Read this file first when resuming in a new chat.
This is a persistent source-controlled handoff, not a claim that the
entire game or every cutscene is complete.

## Repository and verified baseline

- Repository: `masong0508-lab/danny-steel-sprite-intro`.
- Work branch: `expand/danny-hidden-world-tools-random-new-game-20261127`
  (PR [#68](https://github.com/masong0508-lab/danny-steel-sprite-intro/pull/68)).
  The PR remains stacked/draft, not merged to main.
- Last fully verified source before this pass:
  `d94176f18013e5591dc122960da1a376596b3a2b`.
  Screenshot-only follow-up: `a0cdce7beb5dbc595cb2691a802e4f540989e4be`.
  The prior City Studies ROM is 3,779,324 bytes; SHA256
  `daf5d68e088e9df5b36bc350eeef64260f0b4ecca17d84938637440726abd28c`.
  [Verified GBA build](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/46083747348),
  [emulator capture](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/46083747536),
  [ROM artifact](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/46083747348/artifacts/15239130475).
- Previous measured page flips/s: opening 29.466, overhead 14.932,
  first-person stationary 28.968, first-person walking 15.031.
  IWRAM static 23,664 / 24,576 allowed; EWRAM 190,768 / 196,608 allowed.
  Prior opening cache: 1,153,160 bytes, 638 pictures, 11 seek records.
  These are measured prior results, not projections for new code.

## Current change set: opening-film quality

User priority: clean up the ugly sign warping and body's silhouette and
proportions; keep the real reference timing and motion, make the opening
look as polished as possible, and optimize ROM and RAM instead of
dropping quality.

- `src/potheads_sign_geometry.h`: bounded display-only corner correction
  and single projective texture map; keep actual arrow tip and crop.
- `src/potheads_sign_render.h`: single print projection replaces
  diagonal split; extreme folds remain material-only.
- `src/hd_potheads_actor.h`: shared-axis torso silhouette, proportional
  cloth hem, tapered tight-bend limbs. Measured joint and contact tracks
  remain immutable.
- `tools/build_potheads_cache.c`: additional compression span candidates;
  padded-length comparison and smaller raw-packet tie-break. No decoder
  or save-format changes.
- `tests/potheads_sign_geometry_test.c` and
  `tests/potheads_sign_body_raster_test.c`: added projection and contour
  checks. Fresh CI is required. See [Sign/body notes](POTHEADS_SIGN_BODY_PASS.md).

## Reference provenance and asset limitations

The original `307530.mp4` was previously verified as 548 frames, 1280x720,
SHA256 `9181173802c83b52e945ea2f7043eb3367ecbf78401c0f76bdf9911fca8971b8`.
It is not in the public repository. Library searches in this session did
not recover it or the owner-private cloth/leg/upper-body matte packages.
If the original or private assets become available, verify their SHA256
and inspect the actual footage, especially overhead turns, sign release,
hand plants, inversions, and the final right-edge crop. Do not silently
invent new observations. `tools/references/potheads_trace.json` is the
existing reviewed 120-key source-guided annotation; regenerate
`src/potheads_motion.h` only via `tools/build_potheads_trace.py`.
Private matte headers remain optional, separate from the source-only
release. See [opening](POTHEADS_OPENING.md), [costume](POTHEADS_SOURCE_COSTUME.md),
[lower body](POTHEADS_LOWER_BODY_TRACE.md) and [upper body](POTHEADS_UPPER_BODY_TRACE.md).

## Next action checklist (resume in this order)

1. Query PR #68 current head and Actions **for that source SHA**. Screenshot
   bot commits are harmless only if the diff is exclusively docs/screenshots.
   Read failing job logs; correct failures on the existing branch. Do not
   weaken the strict compiler, palette, guard, RAM, or cache budget gates.
2. Build the ordinary source-only GBA ROM; validate all 120 exact pose
   keys, full source timeline, 30-Hz cached frame equality, seek recovery,
   1.2 MiB ROT3 limit, palette/crop guards, all host gameplay tests and real
   emulator inputs. The projective live fallback is costlier than the
   old affine painter; the actual release uses the offline baked cache.
   Verify cache size and measured cadence rather than assuming improvement.
3. Inspect actual generated opening.mp4/GIF or extract representative
   frames at 0–3.3, 3.3–7.1, 7.0–7.6, 9.5–10.9, 11–13.3 and 16.5–18.22
   source-relative seconds (add 180 cinema ticks for descent). Compare with
   the previous verified ROM and raw reference if recovered. Check sign
   corner correspondence, flat print, wrist grip, proportion consistency,
   clothing folds, planted palms/soles, leg/head occlusion and no jitter
   or fabricated uncropped anatomy. The new geometry tests do not prove
   subjective visual quality.
4. If warping remains, calibrate actual board corner correspondence from
   source frames, rather than treating the approximate envelope as a
   calibrated physical rectangle. If body still looks wrong, recover
   owner-private mattes and re-review difficult poses; prioritize silhouette
   and source timing over additional procedural embellishment. Keep the
   exact sign-release tick and final cropped departure.
5. Record final source SHA, workflow IDs, exact ROM/cache sizes, memory,
   emulator results and any remaining visual problems here. Download the
   matching artifact, verify SHA256 and deliver .gba + playable ZIP only
   after required build and emulator checks pass. Do not label older ROMs
   as new. Do not merge an unverified branch to main.
6. After the opening quality pass, continue city and countryside gameplay,
   especially slow overhead and moving first-person rendering. Target
   30 FPS where feasible without losing artwork or save compatibility.

## Gameplay invariants to retain

New Game automatically chooses a persistent randomized layout from a
192-world shuffled pool. Existing saves keep their layout. World
manipulation stays hidden: pause during safe roaming, then enter
UP UP DOWN DOWN LEFT RIGHT LEFT RIGHT B A START. No ordinary seed picker
or terrain controls; exiting tools relocks them. Preserve five-stop
city sketch walk, hikes, music/art, story/endings and current save codec.
See [city studies](CITY_STUDIES_AND_RENDERING.md) and
[seeded world design](SEEDED_CITY_AND_DEBUG_LAND.md).

## Updating this handoff

Before ending each work session or changing chats, commit actual progress
and append the source SHA, tested artifact, CI outcomes, limitations and
the single most important next task to this document. If a check is
pending, say pending. The source tree, not chat history, is authoritative.

## Checkpoint: projective sign pass and cadence correction

The first source revision `82f09929f00a0e99c876826f81d7cdec7d27eaef`
has [passed rotoscope regression](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/46094856710)
and [emulator capture](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/46094856657).
The generated cache shrank from 1,153,160 to 1,139,980 bytes, with
all 638 exact pictures and 11 cue records retained. Opening benchmark
regressed from 29.466 to 26.579 page flips/s (max gap 16 video frames).
Overhead 14.932, stationary first-person 28.968, walking 15.031;
emulator input verification zero failures. Full host/build run
[46094856665](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/46094856665)
was still in progress when the follow-up correction was prepared.
The reason for the cadence change is not yet proven.

The follow-up removes repeated software divisions from the projective
face: seed each scanline once and update rounded rational coordinates
exactly using integer comparisons. It also permits bounded sequential
ROT3 catch-up for up to eight samples and 49,152 total decoded/compressed
bytes (work cap, not a RAM allocation). The ordinary two-sample and
scripted cue paths are unchanged. Added exact quotient and eight-sample
cache-replay regressions. Recheck actual ROM cadence and cache bytes
against the baseline before delivery. No claim yet that the speed
regression has been resolved. The old ROM's decoded frames also confirm
blank-looking clipped overhead boards and angular floor-work silhouettes;
these remain visual review targets, especially if private source is
recovered. Do not invent hidden lettering or limb positions.

The independent geometry model checked 3,164 sign camera projections,
1,292 readable projective faces, 572 bounded corner corrections and
19,729 UV samples without any orientation flips caused by the corrections.
This supplements rather than replaces the C and emulator checks.

## Checkpoint: cadence recovery and secondary art pass

Source `9732d5593aa5ac9ecdc8e092b75089ca6c0c74ce` passed the dedicated
rotoscope and emulator capture workflows (46095145651, 46095145534).
Opening presentation recovered to 28.968 page flips/s (prior 26.579,
verified baseline 29.466); 638 exact pictures, 11 seeks and 1,139,980
cache bytes retained. Emulator runtime verification: zero failures.
Full build 46095145491 failed at the sign test because GCC reported a
possibly uninitialized test output `v`, not a geometry assertion. The
follow-up initializes both test outputs and adds both sign/body tests to
the dedicated workflow. Do not suppress compiler warnings or ship that
failed build. Screenshot-only head `b0fb7bdc10f3fb007462237e398a8778d743ff66`
is the parent of this correction.

Visual follow-up: a 36x13 compact native wordmark at moderately readable
angles; a convex bounded shade strip on material-only sign faces, never
across a source crop edge; bevelled costume hem, inset light plane and
proportional folded collar replacing angular shoulder spikes. The existing
120 keys, measured silhouette envelope, contacts, timing and optional
private assets remain unchanged. Added explicit texture/contour and
material-strip bounds tests. New ROM and performance are pending fresh
workflow results. Original footage/mattes are still unavailable in this
session; this is authored source-guided art, not a new manual trace.

## Checkpoint: verified narrow-angle/soft-costume pass and next refinement

Source `eb300577ba4bb493f5188ff5a93dc5ab6e93013c` passed all three workflows:
[host + GBA build](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/46095421578),
[emulator capture](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/46095421582),
and [rotoscope regression](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/46095421614).
[Verified ROM archive](https://github.com/masong0508-lab/danny-steel-sprite-intro/actions/runs/46095421578/artifacts/15243348078).
ROM SHA256 from real capture: `81f769fa4f1ec3658cec857bff72aa1d0db307be0756157d6b8d8b679913f53f`;
capture ELF/ROM load bytes 3,755,844. Cache 1,125,440 / 1,258,291 bytes, 638 pictures,
11 seeks. EWRAM static 190,768 / 196,608; IWRAM static 23,664 / 24,576.
Opening page flips/s 29.167, overhead 14.932, stationary first-person
28.968, walking 15.031. Runtime verification zero failures. Rotoscope
195,074 UV checks, 4,140 connected limbs, 525 normalized torso sections.
Screenshot-only successor `a8346128ce7f8daac2eb7781682262de0a169104` contains
the captured images reviewed in this session. The baseline is now a
genuine tested ROM, but angular limbs/hip blob remain in the floor poses.

Next source pass: replace the oversized cross-hip stroke with a compact
six-point pelvic garment yoke derived from measured hips and knee directions;
preserve missing/cropped hips. Layer continuous limb material before
seams, add short-segment fallback, and keep pants width constant across
near-leg/support switches. Add a subtle material plane only on an observed
external arrow extension; never invent a tip or clipped print.
Preserve source trace, 638 cache samples and private matte paths. New
raster bounds and crop regressions added. Verify both workflows, actual
ROM cache and cadence before delivery; the new candidate is not yet tested.
