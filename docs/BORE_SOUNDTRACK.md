# Danny Steel — BORE leitmotif pass

These are real, hand-curated melodic note sequences from the user's BORE game
sources, rearranged for the GBA's **four PSG voices**. This is **not** a generic
AI-generated melody library, and not an attempt to squeeze 5 MB of audio into
GBA memory. The ROM keeps its existing D-major title bossa MIDI completely intact.

| Cue | Original BORE material | Role in Danny Steel | Arrangement |
| --- | --- | --- | --- |
| OLD QUARTER | **WHISTLER MAN** (Danny Steele) | Exploration, letter, Mara, day-to-day | 104 BPM shuffle, Gm9/C9/Fadd9/D9 bass movement, arpeggio and sparse hats |
| BUREAUCRATIC FARCE | **NURSERY TIME** (Danny Steele) | BORE continuity clerk, music studio, ridiculous bus rules | 138 BPM music-box riff in F/E/Bb/A, intentionally toy-like pulse and deadpan side-stick |
| MIRROR ORCHARD | **CLOSER TO THE END** (Danny Steele) | Pre/post-cult encounters, testimony, evidence | 136 BPM Bb–Ab–Gb tresillo motif with its B-natural turnaround and C-minor bass |
| CLINIC | **HERE TODAY** (Missy Jeanne) | Notice, doctor visits | 78 BPM direct opening phrase transcription, octave-up lead, soft chord responses, no percussion |
| ROOFTOP | **HERE TODAY** | Final art and neutral finale | 67 BPM thinner variation, held notes and long rests, no percussion |
| LEGACY | **CLOSER TO THE END** and **WHISTLER MAN** | Good-ending posthumous BORE archive | 120 BPM Whistler hook against a closer-to-the-end progression, more confident pulse |

## Source of motifs

In `masong0508-lab/bore-gba`: `tools/make_whistler_rework.py` (G C Eb C tune),
`tools/make_nursery_rework.py` (C C C D C / B E E...),
`tools/make_closer_rework.py` (Bb Ab Gb and B turnaround),
`tools/make_heretoday.py` (timestamped original melody `PHRASES`).
The released BORE jukebox names and authorship are confirmed by
`masong0508-lab/bore2/assets/songs.inc`.

The melodic attack grids are human-curated from those source recipes, not random.
These are **short recognizable leitmotif arrangements**, not complete original
tracks or bit-exact tracker/audio exports. The original `.xm`, `.mid`, samples,
and CD-quality audio remain in the BORE projects.

## Implementation, performance and transitions

- `src/music_motifs.c` is a pure, deterministic, 32-step PSG arrangement table.
  Each step produces melody, a chord or bass note, and optional percussion.
- `src/music.c` schedules them on existing GBA Timer 3 at 262,144 ticks/sec;
  note-to-hardware pitch tables and tempo step intervals are **precomputed**.
- Existing title-screen MIDI events and loop indices are untouched.
- `play.c` selects the cue using story/cinematic/combat state, *not* camera mode.
- Switching gameplay cues occurs on the next four-step boundary, so rapid
  dialogues cannot reset the song every frame. Going to/from the title resets
  cleanly; returning to the menu restores its original theme.
- No heap, PCM buffers, new framebuffers, per-frame instrument synthesis,
  file streaming or dynamic allocations. Playback is only a handful of PSG
  writes on eighth-note changes.
- Existing scene volume/fade automation is retained, including the fade during
  Danny's final rooftop scene.

## Validation

`gcc -std=c11 -O2 -Wall -Wextra -Werror -Isrc src/music_motifs.c tests/music_motifs_test.c -o /tmp/danny_bore_motifs && /tmp/danny_bore_motifs`

The host test checks all cue loops, note ranges, source phrases and quiet-scene
percussion. A real GBA ROM build is separate from the host test; emulator or
hardware validation of tempo, sound mixing and FPS remains important.

## Ending score: one complete performance, three outcomes

The BORE authors' **HERE TODAY (ORIGINAL)** source (`bore-gba/tools/make_heretoday.py`)
contains 20 lyric-melody phrases from 0:00.15 through 1:10.65. The full
86-note timecoded melody is reproduced as a small event table at 60Hz timing
with a final 1.8-second decay, giving a single **72.5-second performance**.
The earlier 32-step HERE TODAY fragments remain useful for the clinic and
ordinary roaming but are **never used as the ending's entire score**.

* **All endings:** the entire HERE TODAY event sequence plays once while
  the cinematic and credits progress, with no short-loop restart on scene,
  dialogue or camera changes. It continues while the ending screen remains.
* **Good:** after HERE TODAY fully concludes, a separate 16-second legacy
  coda reprises WHISTLER MAN and CLOSER TO THE END; silence follows.
* **Neutral:** music thins toward the last line and ends in silence after
  Danny's rooftop painting, with no triumphant coda.
* **Bad (audio hook):** the same reserved HERE TODAY score ends quietly; the
  bad narrative route itself is **not yet implemented** in the story engine.

The GBA PSG driver schedules 86 note-on/off decisions on Timer 3 and uses a
single fixed event table (~0.5 KB) with no new DAC, heap buffer, or streaming.
`music_begin_ending()` is idempotent to prevent restarting HERE TODAY each
frame. Only returning to the title can cancel the score. On SRAM resume, the
scene resumes and the full recording restarts from the beginning; precise
mid-song seek state is deliberately not serialized in v3 saves.

Host tests: `music_motifs_test.c`, `music_engine_test.c`, and
`music_ending_test.c`. Actual GBA emulator and hardware performance testing
is still required.
