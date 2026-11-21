# Optional daily moments in Danny Steel

Danny's numbered-day life simulation has five optional world-positioned
interactions, not just an empty timer: walk with Mara, reply to the letter,
visit the BORE continuity clerk, paint the rooftop, and tune the music archive.
Each uses the existing cinematic renderer and BORE-derived music selection.
Dialogue changes following the first medical visit, with a reflection if all
five moments are completed in a day.

The next unfinished moment appears in both 2D and first-person guidance.
The lower HUD shows a count from 0/5 to 5/5. All five are optional: ignoring
the schedule has no consequences for the story clock, appointments, or endings.
This is not a hidden scoring system or a timed game-over.

The first clinic visit no longer disables the rooftop's ordinary painting
interaction; the final medical appointment still takes priority in guidance.
The ending is never automatically triggered by task completion.

Persistence: v3 save record remains 24 bytes in the same 32-byte SRAM slot.
Previously zero-filled bytes 21 and 22 now hold the daily-bit mask (including
its one-time recap bit) and a capped lifetime activity count. Old v1/v2 saves
initialize these to zero; existing v3 saves already had zero there.

Host regression: `tests/daily_moments_test.c` covers all five actions, unique
once-per-day counting, changing medical-stage dialogue, all-day reset, clinic
priority, uninterrupted optionality, camera views and framebuffer guards.
There are no extra sample buffers, heap allocations, textures or framebuffers.

## Two-way choices with small, readable consequences (SRAM v4)

At any unfinished daily location, **A** opens a two-option menu.
**LEFT/RIGHT** changes the selection, **A** confirms, and **B** cancels
without recording a memory or changing stats.

| Place | First option | Second option |
| --- | --- | --- |
| Mara | WALK: energy -1, bond +2 | TALK: energy +1, bond +1 |
| Letter | WRITE: energy -1, spark +2 | KEEP: energy +1, spark +1 |
| BORE clerk | FILE: energy -1, spark +1 | JOKE: energy +1, bond +1 |
| Music archive | RECORD: work +1 to +3, spark +1 | LISTEN: energy +1, spark +1 |
| Rooftop | PAINT: work +1 to +3, spark +1 | REST: energy +2, spark +1 |

Energy, spark and bond have an upper limit of five. Work accumulates up to
99 and is not tied to Danny's illness or any ending. More spark can improve
a recording or brushstroke. With energy at zero, creating still progresses
at a gentle pace. Two energy points regenerate each morning.
Daily activities are completely optional, repeatable the next day, and
never gate a clinic visit or cause a failure ending.

The bottom HUD displays moments and **E**nergy, **S**park, **B**ond,
**W**ork in both camera modes.

The 28-byte v4 save still fits the original 32-byte SRAM slot stride,
with v1/v2/v3 migration. Bytes 21/22 retain the old daily mask and
memory count, while new bytes 23–26 store the four stats. A v3 checksum
is never mistaken for a stat. No heap, sample buffers or framebuffer
allocation is added.
