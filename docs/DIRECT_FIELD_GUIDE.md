# Danny Steel — direct two-page Field Guide

## Player problem

The original pause menu showed a four-page consequences journal. The fourth journal page already contained the current quest, action and collision-aware route, but players had to open the journal and press R four times to find it. There was no dedicated page explaining the relationship between the letter, BORE clerk, Mara, music, art, energy and optional risk actions.

## Change

While exploring either the isometric Old Quarter or the first-person street view:

- **START** opens the pause menu.
- **L** opens the direct Field Guide. Page 1 shows the actual next objective, the remaining safe walkable distance, **camera-relative direction** (AHEAD, RIGHT, BACK, LEFT or HERE), the current A prompt, and whether B can open an optional risky decision. It also shows energy, spark and bond.
- **LEFT/RIGHT** switches between page 1 and page 2. Page 2 explains the real core story and optional art/music work, resource meanings, and the essential camera, action and safety controls.
- **R** opens the existing four-page journal directly from the guide. R continues to cycle the three real consequence checklists and its existing objective page.
- **B** closes whichever informational page is open without quitting. From the plain pause menu, B still saves and returns to the title. **START** resumes from either page and clears transient guide bits. Pressing L inside the guide also closes it.

These are actual UI controls and existing read-only Story/route queries, not a concept mockup. Both the legacy 120x80 story painter and native 240x160 Mode 4 overlay render the two help pages. Native UI uses the already established copper/ink/paper palette, separators and native fixed-width text. The screenshot gallery includes both new help screens alongside the original fourth journal page.

## Engineering and regression gates

No change to the 24x18 world collision, story progression, story struct size, quest flags, SRAM codec, audio, dialogue or save slots. The guide uses two otherwise unused **transient pause-only choice bits** and no new framebuffers, heap allocations, image assets, large arrays or per-frame world drawing outside pause.

The consequence journal test now checks reversible L/LEFT/RIGHT/R/B/START transitions, compares real quest/resource state before and after viewing help, exercises both the legacy and native painters with guarded video buffers and ensures palette indices remain in range. The original four-page journal, pathfinding, story, battle and entire GBA release gates remain mandatory.

This is a targeted comprehensibility milestone, not a claim that all campaign content or first-person FPS limitations are finished.
