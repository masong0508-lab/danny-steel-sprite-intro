# Danny Steel — Mirror Orchard FACTS / CARE tactical clarity

## Why

The optional three-lane Mirror Orchard encounters presented FACTS and CARE as distinct strategies, but their A-button parry windows and cooldowns were previously identical (15 simulation ticks and 30-tick recharge). FACTS already restored up to one lost focus on a precise parry, and CARE already absorbed one unblocked strike, but the player could not easily see how the tactics differed or when the parry was available.

## Actual new gameplay tuning

- **FACTS**: precise counter strategy, 15-tick parry window, 24-tick cooldown. On a correctly timed same-lane parry, restore up to one missing focus, never above four.
- **CARE**: protective strategy, one shielded hit that does not spend focus, 12-tick parry window and 30-tick cooldown. A shield only blocks the first missed hit; timing a parry still works afterward.
- All rival attacks retain their full 30-simulation-VBlank warnings, original three-lane collision and original two-wave structure. Timing is handled inside story_tick, **not** counted per rendered frame.
- The native 240x160 battle menu displays each strategy's parry/recharge numbers (15/24 vs 12/30). While fighting, an unobtrusive right-hand card identifies FACTS/CARE and says PARRY READY, PARRY ON, WAIT NN or GUARD READY according to the **actual** transient battle state. It does not cover the rival's head or fictional attack lane. The original 120x80 painter also labels the active tactic.

## Safety and testing

No changes to Story struct layout, SRAM codec/version, ending flags, world geometry, damage amount, time-of-day pacing, music, texture atlases, 240x160 framebuffer, palette, extra heap allocations or actor sprite data.

The combat-readability test now executes real A presses and subsequent simulation ticks for both mode-specific windows and cooldowns, asserts different frame hashes for parry-active/recharging/ready feedback, and checks correct focus restoration and one-use guard consumption. Existing gameplay, risk-menu consent, opponent lane synchronization, native and legacy pixel guards, ARM linker budgets, emulator screenshots and input-clock tests remain mandatory.

The goal is meaningful tactical choice and honest feedback, not faster rendering or a claim that every encounter in the wider campaign is complete.
