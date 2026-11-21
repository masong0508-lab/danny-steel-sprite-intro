# Danny Steel — cinematic direction and score-length endings (GBA)

The film renderer operates in the original **120 × 80 indexed framebuffer
(32 colors)**. It uses authored backgrounds, fully connected hand/arm drawings,
spatial blocking, and integer easing. This is game source, not prerendered
video or additional framebuffer storage.

## Camera, acting and object continuity

- **Letter scene:** Danny walks to a fixed counter. The envelope stays
  grounded on the counter as he reaches toward it. Page changes affect
  expressions and the gesture, not the location of the letter.
- **BORE office:** the form and stamp remain on the same desk, with the
  clerk's arm reaching left to touch them.
- **Mara's café:** Danny and Mara approach from opposite sides and stop
  at the same table. The décor, cup and windows stay physically consistent
  in the abduction aftermath; the absent café does not show Mara.
- **Mirror Orchard witness:** an actual written clue appears at the same
  position between the two characters.
- **Rescue:** Danny walks to the safe-house door before reaching for it.
  Success uses the same room while the door opens and Mara walks toward him.
  The failed route never shows explicit violence.
- **Clinic:** the doctor is visually distinct from the clerk, with
  chart/desk interaction; the final appointment preserves the same set.
- **Rooftop:** Danny approaches the canvas before painting, and the
  brush line starts from his attached hand. The finished marks remain
  visible through the long neutral rooftop sequence.
- **Phone notice:** the device is drawn next to Danny's connected hand,
  rather than floating as a separate large overlay.

The camera's subtle breathing is a continuous triangle motion instead of
a modulo-jump pan. Sparse grain is confined to the film border rather
than scattered over faces or props. Scene boundaries use short film-gate
bars, not nearest-neighbor sprite zooms or heavy full-frame blending.

## Ending routes and soundtrack timing

| Ending | Film | Coda | Scene notes |
| --- | --- | --- | --- |
| Good | 4,350 VBlanks, 8 main shots | Additional 960 VBlanks, 4 shots | Fictional posthumous BORE legacy, record shop and concert |
| Neutral | 4,350 VBlanks, 11 shots | None | Finished skyline art and identifiable BEFORE memories; skyline continues moving |
| Bad (Mara missing) | 4,350 VBlanks, 11 shots | None | Empty café and unanswered rescue door; fate not graphically depicted or resolved |
| Bad (Mara rescued, later bad check) | 4,350 VBlanks, 11 shots | None | Mara visibly alive in memory and reunion shots; ending acknowledges later breach of trust |

The bad endings now use **context-aware scene selection** at the SAME score
boundaries. The survival flag determines whether the café and safe-house
shots show Mara in a confirmed reunion or an absence. This preserves
story continuity, not merely text differences.

The musical score still uses the existing PSG/timer playback system and the
same audio duration. The source scene code adds no VRAM allocation, large
texture maps, static animation frame arrays, new PCM buffers, or heap work.
Save v5 and the separate Mara-rescue companion record are unchanged.

## Verification

Host tests include `cinema_test.c`, `cinema_direction_test.c`,
`ending_montage_test.c`, `rescue_route_test.c` and music tests.
They verify deterministic rendered frames, 0–31 palette values, sentinel
framebuffer guards, handoff scene choice, and persistent painted pixels.

GitHub Actions builds the real GBA ROM after host tests. A passing host
render does not establish real-console FPS, timing, or cinematic quality;
hardware capture remains a separate verification task.


## Subtitle staging

Story cinematics now place their speaker and two lines in a **24-pixel
upper strip**. The lower 55 pixels—the actors, reach gestures, paper,
brush, cups, stage, safe-house door and feet—remain visible and are not
covered by dialogue boxes. The original lower dialogue panel is still
used outside cinematic scenes, preserving usability in the overhead and
first-person exploration views. Host tests compare all pixels under the
cinematic caption directly against the raw rendered scene.

## Long-shot temporal continuity — next animation pass

The city background has a 160-pixel repeating far skyline, a 184-pixel
nearer neighborhood and a 160-pixel cloud cycle. These are *periodic
drawings*: when the counters wrap, the same skyline is already one
pixel outside/behind the viewport, rather than abruptly changing a
building's height or moving a cloud dozens of pixels. Interior window
geometry is fixed to the physical building while its distant apartment
lights still change over time.

The initial bus now arrives and parks at one curb. Danny walks into
his waiting position, then stops; there is no modulo-based loop returning
the bus to the horizon. The afternoon bus and clinic follow-up reuse
grounded walk-ins.

In the studio, the BORE reel has a rotating 8-pose spindle and four
smooth envelope-based level bars. The vinyl in the good-ending record
shop rotates as well. A woman physically approaches a record sleeve,
and the good-ending concert now includes fixed-truss moving beams, an
onstage performer and a gently moving crowd. The completed rooftop
painting is actually finished in Mara's later good-ending rooftop shot.

The failing bad-ending route uses a quiet unanswered phone instead of
a flashing full-width bar. The character acting remains non-graphic.

Temporal assertions in `tests/cinema_temporal_test.c` compare actual
framebuffer pixels immediately before/after animation wrap boundaries,
validate a stationary archive window, sample moving reel and vinyl
highlights, ensure the bus remains parked once arrived, and check
deterministic rendering. This supplements the full cutscene, ending and
rescue tests. No PCM, new framebuffer, extra sprite images or heap was
added. Absolute GBA FPS still requires hardware/emulator profiling.
