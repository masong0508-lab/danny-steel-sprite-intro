# The Last Days of Danny Steel — Gameplay & Playability Milestone

This is **source-code work in the playable GBA game**, not a separate design mockup or an assertion that the whole planned long-form game is complete. It focuses on core player comprehension, small meaningful choices, and tactical combat. All changes preserve the existing two save slots and story chronology.

## Problems found and fixed

| Before | Now | Tested by |
| --- | --- | --- |
| Goal arrow was a raw dominant coordinate that could point straight through buildings. | Cached breadth-first search uses only the compiled 24 × 18 collision grid. Solid NPC goals end on adjacent interaction tiles; the family letter's walkable goal ends on its exact standing tile. HUD shows the first real direction and distance. | `navigation_guide_test.c` independently calculates shortest routes and follows them, then checks the result across every walkable location and major story phase. |
| Controls were hard to discover once past the title, and `B RISK` appeared even when B did nothing. | Contextual A button labels identify letters, café, clerk, canvas, bus, clinic and rescue actors. B appears as a risk option only where an explicit risk can be opened. Pausing then pressing L displays a live two-page Field Guide; LEFT/RIGHT opens the explanation of core vs optional activities and what resources actually do. R opens the existing consequences checklist. | Pause transitions, no unintended save/quest changes, old 120 × 80 and native 240 × 160 display guards. |
| Pressing A on an empty street only returned the same placeholder line. | Five geography-bound two-page environmental reflections give the Potheads block, market, canal, north quarter, and ordinary street a distinct voice. They grant no hidden badges or story flags. | `navigation_guide_test.c` checks actor priority and ambient dialogue. |
| CARE had a protective hit with no compensating upside for FACTS. | FACTS offers a 10-tick parry / 14-tick recharge and a precisely timed same-lane counter that restores up to one focus. CARE protects against one unblocked attack and offers an 8-tick parry / 20-tick recharge. HUD shows the strategy, countdown and readiness. | `combat_balance_test.c` checks focus restoration, dodge-vs-parry, shield consumption, cooldown windows, palette and frame bounds. |

## What the guide does and does not do

The route system is not a quest auto-completer, path teleport, item radar or new open-world map. It is a shortest walk through known non-solid cells to a currently authored objective. When `HERE` appears, A interacts through the normal game logic; some scenes still require completing dialogue with A to receive their main-story checkmark. Main targets and optional daily suggestions are distinguished by their guide reason, so daily moments do **not** become artificial deadlines. Every Field Guide screen is derived from the in-game Story object and never writes to SRAM.

The orientation controls are in **world directions**: E, S, W, N. First-person users can turn left/right and then move forward; no secret compass alignment is assumed. Cached routes recalculate only when Danny changes tile or the quest target changes. The actor/nearby-action query does not alter the original collision model.

## Continuity and writing rules

The Potheads job remains work Danny doesn't love, though it helps pay rent and affords time for art. Ambient reflections make room for gratitude without rewriting him as someone who enjoys mascot work. There are no new automatic story consequences from walking, observing, missing a day or resting. Mara's survival, unknown fate on the failed route, irreversible harmful choices and illness progression remain governed by the original authored conditions. The expanded combat mechanics affect only the fictional Mirror Orchard challenge, not the cause or outcome of Mara's investigation.

## Save, resource and rendering contract

No changes to `Story` structure layout, SRAM codec/version, music patch extension, story coordinates or framebuffer allocation. Navigation scratch space is two fixed-size arrays of 432 bytes and 864 bytes used only when a player step or objective change invalidates the cached result. No heap allocation. Native GBA redraws use compact text and existing palette colors. Combat parry windows are timed in simulation VBlanks, not slow renderer frames.

## Work remaining after this milestone

A complete larger campaign, additional explorable districts, more varied NPC schedules and interactions, sustained hardware-measured frame rates, further character-animation polish and larger branching arcs are still unfinished. Future work should be driven by reproducible tests and representative mGBA captures, not extra placeholder art or unverified performance promises.
