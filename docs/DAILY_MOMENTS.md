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

## Four-stage BORE tape and rooftop canvas (SRAM v5)

The two creative stations now support **four stages each**. The first day's
RECORD or PAINT action starts a project; after the day's initial memory is
earned, returning to the studio or rooftop opens a smaller project menu.
The stages are stored separately from overall W(ork), which remains a diary
stat rather than the finishing threshold.

- **BORE tape:** first take → bass and melody → rhythm arrangement → master copy.
- **Rooftop canvas:** first stroke → skyline → street lights → framed painting.
- **BUILD/LAYER** spends at most one energy and gains additional work.
  **SOFTEN/LIGHT** recovers energy, adds detail and selects a distinct
  musical or visual presentation. Both make one stage of progress.
- **B** always cancels a menu without consuming a daily memory or advancing a
  project. Revisits after completion play a short, authored completion scene.
- Completing the tape changes the BORE clerk's dialogue; completing the
  painting changes Mara's dialogue. Mornings and closing time also have small
  schedule-specific conversations, but neither NPC becomes inaccessible.

The bottom HUD shows the five optional daily moments, E/S/B, **M0–4**
(music stages), **A0–4** (art stages), and W (lifetime work).
Studio notes, the master reel, canvas brushwork and the finished frame are
actual palette-indexed details within the existing scene. A clock-time-specific
cafe light provides a modest late-day atmosphere change without extra textures.

The **v5 SRAM record uses 31 bytes** of the existing 32-byte slot stride,
adding separate music, art and creative-style bytes at offsets 27–29; byte 30
is the checksum. Legacy v1, v2, v3 and v4 files are read and normalized
without interpreting earlier checksums as project data. No new framebuffer,
heap, PCM buffer or per-frame allocation.

Neither the projects nor the schedule affects whether a clinic visit or
ending is available. The final rooftop painting scene remains its own
separate, deliberate story action after the last appointment.
