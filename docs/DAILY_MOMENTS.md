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
