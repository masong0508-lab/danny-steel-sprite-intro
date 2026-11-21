# THE LAST DAYS of danny steel

A small, original bittersweet comedy-adventure, with a BORE-universe continuity-office cameo. The GBA build preserves the existing DippInn logo, animated sprite introduction, drifting-painting title, bossa-nova audio and cinematic menu. **NEW GAME** now enters a playable opening chapter instead of returning to the title.

## Playable chapter: Old Quarter

The city is closing, the last bus is leaving, and Danny is trying to determine which pieces of yesterday are worth carrying into tomorrow. Read a letter from home, have the BORE Continuity Office validate your existence, and speak to Mara before deciding whether to board the bus or remain behind. Finishing the optional music and art objectives raises *primary-story* completion to 100%, while the Mirror Orchard investigation remains independent. The prose is intentionally a little sad beneath the bureaucracy and bad jokes; the BORE reference is a light crossover and does not attempt to rewrite the larger game's canon.

## Controls

* **Title:** START for main menu.
* **Menu:** LEFT/RIGHT, A select, B back; NEW GAME, LOAD and ERASE support two persistent slots.
* **Exploration, map:** D-pad walk, A talk/read/interact with adjacent objects, L or SELECT toggle to first-person, R journal.
* **Exploration, first-person:** UP/DOWN move forward/backward, LEFT/RIGHT turn 90 degrees, A interact, L or SELECT return to map, R journal.
* **Dialogue:** A next, B dismiss. At the bus choose GO or STAY with LEFT/RIGHT and A; B cancels. GO with 100% main-story completion produces the good epilogue; GO with enough but incomplete progress starts the numbered-day neutral route. STAY continues exploration.
* **Pause:** START; B saves and returns to the menu.

The **same grid coordinates and quest-state flags** drive the top-down map and pseudo-3D first-person view; neither mode is a disconnected cutscene. Both modes feature locations, streets, trees, the BORE clerk, Mara, the letter and bus. The camera interpolates steps; the renderer uses an integer ray marcher, fixed palette, low-memory tile art and no dynamically allocated framebuffers.

## Build / validation

GBA ROM: `make` with devkitARM installed. Output is `danny_steel_title.gba` to retain the established ROM filename. GitHub Actions runs host dialogue/reachability/renderer tests and then produces the GBA ROM artifact.

Host tests locally:

```sh
gcc -std=c11 -Wall -Wextra -Werror -Isrc src/story.c tests/story_test.c -o /tmp/danny_story_test
/tmp/danny_story_test
gcc -std=c11 -Wall -Wextra -Werror -Isrc src/story.c src/story_draw.c tests/render_test.c -o /tmp/danny_render_test
/tmp/danny_render_test
```

## Current limits

This is the **first playable chapter**, not a finished full-length game or a full 3D engine. GBA first-person is intentionally a 120x80 integer pseudo-3D view doubled with hardware affine scaling; full 3D geometry, voice performances, deeper branching dialogue, larger districts, animation upgrades and port parity are future work. The GBC companion repository still contains its own title-screen prototype, not this newly integrated story mode. Save slots use 32 KB-compatible GBA SRAM (`SRAM_V113`) and should be verified in an SRAM-capable emulator. Version-3 24-byte slots fit inside the original 32-byte slot spacing; valid v1/v2 records are upgraded, while preserving collected objectives and fandom milestones. Old immediate endings are reset to allow new choices. FPS and hardware timing have not been measured on physical hardware.

## Optional side story: Mirror Orchard (fictional satire)

This is an **original** fictional fandom server and fictional characters, not a depiction of an identifiable real community or allegations about a real person. Its comedy satirizes clout chasing, mascot-guild bureaucracy and moderator power trips. Exploitation and threats against vulnerable members are treated seriously; helping a person takes priority over winning an online argument. The plot intentionally ends with an imperfect result: logs preserved, reports filed and people helped, yet the network rebrands and persists.

The story's three recurring hecklers are visible as fixed, reachable NPCs in **both** overhead and first-person exploration. Press **A** beside them to enter a 3D-perspective speech duel. They appear *before* Danny learns of the server, *during* the investigation and *after* its conclusion, with different dialogue and attack patterns. These fights do **not** gate the main chapter or reward harm toward people.

* **Encounter menu:** LEFT/RIGHT to select FACTS, CARE or EXIT; A confirm; B disengage. FACTS and CARE each begin a projected arena attack wave. CARE absorbs the first hit. Two completed waves end the duel.
* **Perspective arena:** LEFT/RIGHT move between the three visible lanes. Incoming square speech bubbles are telegraphed from the enemy billboard, travel toward Danny on the floor grid, and impact every 15 VBlanks. A parries for a six-frame window, with an eighteen-frame cooldown. Losing focus ends the encounter without a game-over; leave and return whenever you like.
* **Side-story progression:** use the terminal in the eastern plaza → ask what the witness needs → find the offline safe house → return to the terminal to preserve evidence and file reports. Read R journal updates for the current step. All of this is optional and independent of the letter / stamp / Mara / bus arc.
* **Persistence:** v2 SRAM save slots now track stage and pre/post fight outcomes. Existing 13-byte v1 slots load and automatically upgrade on a subsequent save. The battle itself is not serialized; it restarts cleanly when re-entered.

The battle rendering is part of the existing **120×80 → 240×160 GBA mode-4 affine** hybrid pipeline, not a claim of hardware-accelerated 3D. It reuses the same pixel buffer and geometry, with no heap allocation or additional framebuffer. Host tests cover all stage changes, rematches, safe opt-out, unchanged main quest flags, both exploration views and render buffer bounds. Actual GBA hardware/ROM frame rates still need measurement.

## Numbered days and the three-ending arc (in progress)

The instant two-way ending has been replaced with a longer, saved timeline. The GBA prototype currently simulates **one numbered game day in four minutes at 60 VBlanks per second**; this accelerated pacing is for playability while the larger near-real-time life simulation is built. The HUD shows day number and daylight progress, and the game clock pauses while the Pause menu is open. Time continues naturally through exploration, conversation, and optional battles. A periodic save checkpoint preserves the clock without requiring constant writes.

**Good ending — implemented.** Complete the original letter, clerk and Mara arc **plus** the BORE music archive and rooftop art task; then choose GO at the bus. A posthumous epilogue reveals that Danny's unreleased BORE-game music and games became fictional multimillion-selling hits after his death. The story treats that recognition as bittersweet, not a reversal of his death. Completion of the optional Mirror Orchard cult investigation is **not** required for 100% main-story completion.

**Neutral ending — implemented.** Finish the three core main-story tasks but leave one or both art/music tasks incomplete, then choose GO. Instead of ending immediately, Danny remains in the Old Quarter, can keep exploring and painting, and starts a numbered life-simulator day. On Day 2 he receives a clinic notification, then can visit the outpatient office in the east quarter for the first doctor scene. On Day 3 he gets the final appointment notification and visits again for the diagnosis of advanced, unspecified-type cancer. There is no automatic ending timer after that visit: the player has to return to the rooftop stairway on the north plaza, where a 4,350-frame, 11-shot score-length sequence shows Danny making his last brushstrokes, gradually slumping against the canvas, and dying there. The scene stays grounded to the real rooftop, revisits earlier memories, and returns to the painting before the neutral screen appears. The death is non-graphic, and the scene cannot be speed-skipped into an accidental ending.

**Bad ending — storyboarding, not yet playable.** The planned off-camera rooftop death and its dialogue require a careful, deliberately authored narrative path. There is deliberately no failure counter that automatically converts losing a fight or missing a collectible into a self-harm sequence.

On the GBA, the same 120×80 bitmap and 32-entry color palette are reused for the doctor, notification, record-shop and rooftop cinematics. No framebuffer, heap allocation or procedural texture budget is added. The patient-facing medical storyline is fictional and deliberately does not specify a cancer subtype.

Run `tests/ending_test.c` along with the existing host tests to validate day-boundary timing, doctor's office access, two medical visit sequences, good-epilogue gating, a non-premature neutral ending, both final-scenery render buffer guards and post-ending input behavior. Neither GBA hardware FPS nor physical-ROM execution has been measured for this patch.

## Cinematic overhaul — 20 grounded, animated scene types

The opening chapter's dialogue now directs a **new low-memory cinematic renderer** (`src/cinema.c`, `src/cinema.h`) rather than showing NPC portraits over plain roaming maps or relying on fixed primitive backgrounds. The sequence uses 20 hand-coded scene types, shared and grounded to locations in the existing 2D/first-person Old Quarter: street arrival, home/letter, BORE office, Mara's cafe, last bus, staying behind, music studio, rooftop painting, fictional server terminal, witness meeting, safe house, evidence archive, medical notification, first clinic, second appointment, final clinic, good epilogue, final rooftop painting, the numbered-day transition and fictional server rebranding.

* **Animation and staging:** distinct actor silhouettes, face blinking, arm gestures, walking/bobbing, paper passing, a timed clerk stamp, an actual record player and physical props. The streets retain their identifiable buildings across angles; parallax skyscrapers, clouds, lit windows, neon, bus signals, foliage colors and rooftop light move subtly rather than teleport.
* **Story beats:** dialogue pages direct small camera and gesture changes. Emotional scenes slow down and music decreases during clinic visits; the neutral finale now plays the entire 4,350-VBlank Here Today score across 11 distinct shots, and the good finale adds four coda shots after its own eight main-song shots. The rooftop canvas depicts the same skyline Danny sees outside.
* **Protecting playability:** cinematic state lives in the existing small `Story` struct and is **not serialized**, so old saves remain compatible. The same existing 120×80 8-bit bitmap/palette is reused in 240×160 mode-4, without additional framebuffers, textures, audio samples, heap allocations or VRAM reservations. Regular interactions remain dismissible page-by-page; during the score-length ending, timed captions replace blocking opening dialogue so the camera and music stay aligned.
* **Tests:** `tests/cinema_test.c` renders every cinematic through different page/finale times and verifies palette bounds and guard bytes, plus checks the actual story interactions. Run using the same `src/cinema.c` linkage as the other renderer host tests. Host tests passed with `-Wall -Wextra -Werror`; GBA hardware frame rate and actual emulator appearance **still need to be measured**. These are polished-in-context *procedural pixel cinematics*, not a claim of full-resolution sprite animation or pre-rendered FMV; further character art and traditional animation remains worthwhile.

## BORE soundtrack — authored motifs and full ending score

The GBA's gameplay score now uses authentic melodic note sequences taken from the BORE projects: **Whistler Man** for neighborhood exploration, **Nursery Time** for comical bureaucracy, and **Closer to the End** for the Mirror Orchard confrontations. The original Danny Steel D-major title theme is preserved.

All endings use the complete 20-phrase **Here Today (Original)** tune, from the BORE source composition (86 actual melody notes over 72.5 seconds, not an endlessly recycled 32-note riff). Each ending starts the score **once**: subsequent dialogue lines or camera shots do not restart it. After Here Today resolves, only the good ending receives a separate 16-second Closer/Whistler BORE legacy coda. Neutral and the reserved bad-ending music hook end quietly. **The bad ending narrative route itself is still not playable.** For the timing and source provenance, see `docs/BORE_SOUNDTRACK.md`.

The GBA audio implementation keeps four built-in PSG channels, uses the existing Timer 3 clock, retains old SRAM saves and renders no PCM buffers. The complete ending tune is stored as a compact timecoded event table. GitHub Actions now checks melodic material, PSG timing, score completion and the good-only coda. Real hardware audio and frame-rate testing is still needed.

## Ending cinematics synchronized to the full BORE score

The original seven-and-a-half-second rooftop sequence has been replaced with
11 neutral-ending shots over all 72.5 seconds of HERE TODAY; the good ending
uses eight main-score shots and four extra coda shots across an additional
16 seconds. Those shots revisit recognizable street, letter, BORE-office,
Mara, studio, rooftop and record-shop environments rather than a static card.
The story does not reveal Danny's eventual fictional record success until the
good route's posthumous epilogue. The neutral route keeps his final death
non-graphic and follows with identifiable earlier memories. Credits dialogue
only unlocks after the music has played. Loading an ending save restarts both
music and film at zero to prevent an out-of-sync resume. No new SRAM fields or
framebuffers. See `docs/ENDING_CINEMATICS.md`; host tests cover both timings.

## Next-objective guidance, cutscene art and render optimization

The overhead map and 3D exploration share one world-coordinate objective query.
The HUD shows the next primary destination and cardinal direction, while the
overhead view pulses a small marker on that *actual town tile*. The progression
is letter → BORE stamp → Mara → music archive → rooftop canvas → last bus.
During the numbered-day neutral story, medical appointments take precedence;
after the final doctor visit the compass directs Danny back to his last canvas,
without making the interaction automatic. The Mirror Orchard investigation
remains optional and does not replace the main objective.

The 20 authored cutscene types now give Danny, Mara, the clerk, and other
characters more individual clothing, facial details, distinct hairstyles,
blinking and arm/leg gestures. Rooftop water tanks, street infrastructure,
tar seams and physical art supplies remain anchored across shots. Paint
strokes accumulate on the same skyline canvas rather than randomizing.

The fixed-resolution first-person renderer precomputes its 40 floor-distance
values in ROM rather than repeating the same divide for all 120 columns:
**4,800 per-frame divisions removed**. Projection coordinates are reused
for floor color and tile detail, preserving the same geometry. No extra frame
buffer, sample allocation, heap work, new art textures, or 3D camera modes.
GBA hardware FPS remains unmeasured; these are code-level optimizations.
`tests/objective_test.c` checks objective ordering, life-stage overrides,
render bounds and identical guidance in the overhead and first-person HUD.

## Optional daily moments between appointments

After choosing the neutral route, numbered days now have five optional,
world-positioned activities: walk with Mara, revisit the family letter, humor
the BORE clerk, mix a reel at the music archive, and paint the rooftop skyline.
Each has animated dialogue with a second script after the first clinic visit.
The guidance compass suggests an unfinished activity and the HUD displays
`MOMENTS 0/5` through `5/5`. Finishing all five opens a short closing
reflection; tomorrow resets the suggestions. You can ignore every one of these
activities without affecting clinic notifications or ending access. The first
clinic visit no longer prevents painting while Danny waits for the second.

Two formerly unused bytes in v3's 24-byte SRAM record now hold the daily
activity mask and cumulative moments. No save format or slot stride changed,
and all old saves begin with zero moments. No framebuffer or heap allocation was
added. See `docs/DAILY_MOMENTS.md`; a dedicated host test checks day reset,
post-clinic dialogue, optionality, objective targeting and 2D/3D render guards.

### Daily choices and creative work (v4)
Five repeatable optional daily moments now offer two responses each. A opens
a contextual choice, LEFT/RIGHT selects, A confirms, B cancels. Energy, spark,
bond and crafted work respond to actions; no resource can block an ending,
doctor visit or story objective. The HUD is shared across map and first-person
views. Save v4 is 28 bytes within the original 32-byte slot stride and retains
v1–v3 compatibility. See docs/DAILY_MOMENTS.md and tests/life_choices_test.c.

### Optional interactive creative projects (SRAM v5)
The BORE tape and rooftop canvas are now independently persistent four-stage
projects rather than counters. Returning to a creative workstation after its
daily moment offers BUILD/REVISE-style options; LEFT/RIGHT and A confirm,
B cancels. Real studio and canvas details appear as the work progresses,
and completed pieces unlock additional BORE clerk and Mara conversations.
Morning and closing-time dialogue makes their daily routine more believable
without hiding NPCs. New SRAM v5 uses 31 of the same 32 bytes per slot and
migrates v1–v4. Tests validate project completion, visual frame bounds,
appointments, endings, and save checksums.
