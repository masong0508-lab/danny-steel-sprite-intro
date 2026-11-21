# Danny Steel — score-length ending cinema (GBA)

The original prototype ended the neutral scene after **450 VBlanks (~7.5 seconds)**, while BORE's entire **Here Today (Original)** lasts **4,350 musical ticks (~72.5 seconds)**. The new director instead presents **11 neutral shots** and **12 good-ending shots** (including a **960-VBlank, 16-second** good-only *Closer to the End* / *Whistler Man* legacy coda).

## Narrative and scene continuity

The montage revisits established Old Quarter locations: Danny's actual rooftop canvas, Mara's street, the letter, BORE's desk and the music studio. Neutral memories are captioned **BEFORE** so characters are not accidentally shown alive after the rooftop scene. His final painting and death remain non-graphic. The good route follows Danny's musical work into a wholly fictional multimillion-record BORE legacy; only it contains the later concert-and-records reprise.

| Route | Visual soundtrack timeline | Final UI |
| --- | --- | --- |
| Good (five main objectives) | 0–4,349: eight Here Today shots; 4,350–5,309: four legacy-coda shots | Dialogue at 5,310 |
| Neutral (three core tasks, missing art/music) | 0–4,349: eleven final painting, memory and rooftop shots | Dialogue at 4,350 |
| Bad | Not implemented as a playable story route; audio hook reserved | Not playable |

The 120x80 32-color Mode-4 framebuffer is reused for every shot; no new streaming, PCM, VRAM, EWRAM or heap allocations. The opening neutral painting cutscene now advances automatically with timed captions rather than blocking on an A-press. Neither ending can be skipped before the full performance concludes.

Visual timing is driven by game VBlanks; music uses hardware Timer 3, so true GBA 59.73Hz refresh introduces a fraction of a second of drift versus the ideal 60Hz score clock over 72 seconds. A measured future hardware synchronization pass may improve that. Save slots remain version 3; loading an ending begins both music and film at frame zero rather than resuming only one.

Tests: `tests/ending_test.c`, `tests/ending_montage_test.c`, `tests/music_ending_test.c`. Rendering bounds, palette indices, shots and full-score gating are checked on host. Real emulator/ROM and hardware timing are not yet verified for this patch.
