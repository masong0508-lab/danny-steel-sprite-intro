# First-evening campaign and progressive native art

## Player-facing changes

The game no longer requires the clinic interruption before Danny can make
music, paint, or share a meaningful moment with Mara.

1. The original letter, clerk and explicit promise still form the three
   required opening tasks. Returning to the cafe after the promise offers
   WALK (bond +2, energy -1 when available) or TALK (bond +1, energy +1).
   Each has its own conversation about leaving and facing the clinic.
   The scene is optional, counts only once, and does not consume a day.
2. Retrieve the BORE reel and visit the rooftop canvas as before. Revisit
   either place during the opening evening for four real creative sessions.
   BUILD/LAYER advances work with energy; SOFTEN/LIGHT advances the work
   with a different style and energy recovery. These do not gate the bus.
   The existing advanced music editor is now reachable before departure.
3. Danny's completed song and painting change his conversations with Mara
   and the BORE clerk. The first-evening artwork now progresses from
   pencil underdrawing to a painted sky, buildings, individual windows,
   and illuminated streets. The console displays actual saved takes and
   soft-arrangement colors. Native menu strips show 0-4 stages.
4. STAY at the bus now has its own grounded dialogue instead of cutting
   to an unrelated rooftop scene. BOARD when the two legacy flags are
   present acknowledges the tape and canvas, not an unfinished work.
   A and B pressed together cancel the bus selection rather than board.
5. When Mara remains unlocated, the last clinic scene no longer speaks
   as if she is sitting beside Danny. Actual painting during numbered
   days awards the canvas flag; resting does not.

## Save compatibility and performance

Use the formerly unused bit 6 of the existing daily-mask byte for the
single pre-bus friendship scene. Clear it when numbered days start, and
restore it only in the pre-bus phase. Fix the existing loader bug that
discarded bit 5 (a completed paid Potheads shift). No save version,
record size, map geometry, story-state size or new framebuffer changes.

New cinematic artwork is bounded native Mode-4 drawing within the
existing reel display and easel. No extra image ROM assets or heap,
and no additional walking-camera work.

## Test gate

`tests/first_evening_campaign_test.c` exercises real interactions,
both early friendship choices and repeat prevention, pre-bus music and
art from zero to completion, editor entry, reactive conversations,
cancellation, the clinic's absent-Mara dialogue, both bus outcomes,
save masks, native frame guards, all five artwork stages and style
differences at identical animation time. Full story, save, rendering,
and ARM linker-budget checks remain required.

The next campaign pass should add playable transitions between the
clinic visits, more neighborhood characters with an actual stake in
Danny's life, and additional distinct endings/legacy scenes beyond the
current opening chapter. These remain unfinished, not claimed complete.
