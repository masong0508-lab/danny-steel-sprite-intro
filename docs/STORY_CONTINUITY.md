# Story continuity audit — The Last Days of Danny Steel

This is a gameplay-linked continuity contract for the GBA story. It
distinguishes what Danny **knows**, what is only a **lead**, and what the
audience sees confirmed. Script lines are concise enough to fit the
existing 120 × 80, 32-colour dialogue renderer. Neither changes to
save records nor additional graphics assets are required.

## Canonical timeline and causal links

1. **Before the bus:** the clinic has attempted to reach Danny, but he
   is avoiding the news. The family letter establishes that someone
   still made room for him; the BORE clerk certifies his records and
   directs him to the archive; Mara explicitly asks him not to vanish
   without telling her. Only completing Danny's response with **A**
   gives the Promise checkmark. Closing the conversation with **B**
   does not silently count a promise.
2. **Music studio:** the east archive workstation is a playable
   narrative arranging activity, with ARRANGE/LISTEN for the daily
   session and four BUILD/SOFTEN stages for the optional master tape.
   Energy, spark, work, track style and progress are actually saved.
   A visited archive registers a surviving copy of the rough BORE reel.
   Mastering a later mix does **not** recreate an early deleted
   session, and deleting that session does **not** erase a different
   preserved master. Story consequences acknowledge the distinction.
   The score itself uses authored PSG cues: this is **not yet** a
   free-form composer or piano-roll recorder.
3. **Mara's trust:** when one or two harmful choices were explicitly
   confirmed, Mara's later conversations acknowledge the mounting
   danger without skipping the ordinary daily activity menu.
   Neither a neutral conversation nor a missed deadline grants a
   harmful mark. A third distinct violation triggers the disappearance.
4. **Investigation:** the witness observed a van and says she saw
   someone forced inside; a terminal locates the *vehicle*, not Mara
   herself. The location is a lead, not proof of custody. Danny can
   confirm her survival only by actually opening the safe house.
   Dialogue and visual shots distinguish the rescue from uncertainty.
5. **Unsuccessful rescue:** selecting LEAVE at the safe-house door
   shows that door; making a fourth harmful choice somewhere else
   instead shows a phone/remote lead, not an invented visit to that
   house. In both cases an unidentified caller claims Mara will
   answer. After one game day the caller admits this was a rumor;
   the witness confirms she has not been recovered. Danny keeps
   hoping, without implying she is dead. The bad-ending film begins
   after another playable day.
6. **Neutral route:** declining the bus leads to a day-by-day life
   chapter, a follow-up investigation of Danny's illness, and a
   final rooftop painting *when the player chooses to finish*. It
   is not a timer-based punishment for resting or talking. Mara
   returns to the roof for his letter and painting in the film.
   If the optional music master was completed, the film calls it a
   finished tape, not an unfinished song.
7. **Good route:** boarding the bus does not instantly grant Danny
   fame. BORE retains the recordings, years pass, and Mara helps
   release them after Danny is gone. The archive connects the
   playable music project to that later legacy. A good ending
   following earlier harm also acknowledges what he did; success
   in music or travel does not undo past violations.
8. **Bad route after rescue:** Mara's survival is always explicit,
   including the final montage. A *new* deliberate harmful choice
   breaks trust and leads to a separate immediate ending. It must
   never show an unresolved Mara disappearance.

## Plot-hole checks built into CI

`tests/story_continuity_test.c` covers the A-vs-B Promise milestone,
one- and two-violation dialogue, erasing an early demo before/after
mastering, failures at and away from the safe house, the scarred
version of the good ending, neutral master-tape caption correctness,
and frame-buffer/palette safety. Other host tests cover both safe-house
choices, every ending, dialogue/menu navigation, save migration,
the two-day unanswered phone, character animations and music timing.

The 326 authored dialogue pages were checked for proper three-field
initializers and for both lines fitting within 24 ASCII characters;
the text renderer's display limit is larger than that. No extra
heap-allocated dialogue, save buffers, or full-screen render buffers
were introduced.

## Deliberately unresolved facts

The identity and reliability of the unknown caller are not evidence
of Mara's location. On the failed-rescue path no character knows Mara's
ultimate fate, and no ending confirms death. This is intentional
uncertainty, not a missing piece the game silently fills in.
The exact cause and later timeline of Danny's death on the successful
bus route remain unspecified, rather than falsely claiming that
the illness or bus immediately caused it.

The audit checks programmed branches and consistency; passing tests
cannot guarantee every possible interpretation or pacing problem is
resolved. Visual playthrough and reader feedback remain useful.
