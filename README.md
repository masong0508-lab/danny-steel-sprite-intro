# THE LAST DAYS of danny steel

A small, original bittersweet comedy-adventure, with a BORE-universe continuity-office cameo. The GBA build preserves the existing DippInn logo, animated sprite introduction, drifting-painting title, bossa-nova audio and cinematic menu. **NEW GAME** now enters a playable opening chapter instead of returning to the title.

## Playable chapter: Old Quarter

The city is closing, the last bus is leaving, and Danny is trying to determine which pieces of yesterday are worth carrying into tomorrow. Read a letter from home, have the BORE Continuity Office validate your existence, and speak to Mara before deciding whether to board the bus or remain behind. Both ending choices have complete short sequences. The prose is intentionally a little sad beneath the bureaucracy and bad jokes; the BORE reference is a light crossover and does not attempt to rewrite the larger game's canon.

## Controls

* **Title:** START for main menu.
* **Menu:** LEFT/RIGHT, A select, B back; NEW GAME, LOAD and ERASE support two persistent slots.
* **Exploration, map:** D-pad walk, A talk/read/interact with adjacent objects, L or SELECT toggle to first-person, R journal.
* **Exploration, first-person:** UP/DOWN move forward/backward, LEFT/RIGHT turn 90 degrees, A interact, L or SELECT return to map, R journal.
* **Dialogue:** A next, B dismiss. At the bus choose GO or STAY with LEFT/RIGHT and A; B cancels.
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

This is the **first playable chapter**, not a finished full-length game or a full 3D engine. GBA first-person is intentionally a 120x80 integer pseudo-3D view doubled with hardware affine scaling; full 3D geometry, voice performances, deeper branching dialogue, larger districts, animation upgrades and port parity are future work. The GBC companion repository still contains its own title-screen prototype, not this newly integrated story mode. Save slots use 32 KB-compatible GBA SRAM (`SRAM_V113`) and should be verified in an SRAM-capable emulator. FPS and hardware timing have not been measured on physical hardware.

## Optional side story: Mirror Orchard (fictional satire)

This is an **original** fictional fandom server and fictional characters, not a depiction of an identifiable real community or allegations about a real person. Its comedy satirizes clout chasing, mascot-guild bureaucracy and moderator power trips. Exploitation and threats against vulnerable members are treated seriously; helping a person takes priority over winning an online argument. The plot intentionally ends with an imperfect result: logs preserved, reports filed and people helped, yet the network rebrands and persists.

The story's three recurring hecklers are visible as fixed, reachable NPCs in **both** overhead and first-person exploration. Press **A** beside them to enter a 3D-perspective speech duel. They appear *before* Danny learns of the server, *during* the investigation and *after* its conclusion, with different dialogue and attack patterns. These fights do **not** gate the main chapter or reward harm toward people.

* **Encounter menu:** LEFT/RIGHT to select FACTS, CARE or EXIT; A confirm; B disengage. FACTS and CARE each begin a projected arena attack wave. CARE absorbs the first hit. Two completed waves end the duel.
* **Perspective arena:** LEFT/RIGHT move between the three visible lanes. Incoming square speech bubbles are telegraphed from the enemy billboard, travel toward Danny on the floor grid, and impact every 15 VBlanks. A parries for a six-frame window, with an eighteen-frame cooldown. Losing focus ends the encounter without a game-over; leave and return whenever you like.
* **Side-story progression:** use the terminal in the eastern plaza → ask what the witness needs → find the offline safe house → return to the terminal to preserve evidence and file reports. Read R journal updates for the current step. All of this is optional and independent of the letter / stamp / Mara / bus arc.
* **Persistence:** v2 SRAM save slots now track stage and pre/post fight outcomes. Existing 13-byte v1 slots load and automatically upgrade on a subsequent save. The battle itself is not serialized; it restarts cleanly when re-entered.

The battle rendering is part of the existing **120×80 → 240×160 GBA mode-4 affine** hybrid pipeline, not a claim of hardware-accelerated 3D. It reuses the same pixel buffer and geometry, with no heap allocation or additional framebuffer. Host tests cover all stage changes, rematches, safe opt-out, unchanged main quest flags, both exploration views and render buffer bounds. Actual GBA hardware/ROM frame rates still need measurement.
