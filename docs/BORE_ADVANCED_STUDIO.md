# BORE Music Studio — unlockable Advanced Edition

The music studio is in **The Last Days of Danny Steel**, at the old BORE
archive on the east side of the Old Quarter (the **M** workstation). It
has two tiers and works with either map camera mode.

## Basic Studio — always easy to use

Walk beside the music workstation and press **A**. On the numbered-day
route, the first visit offers **ARRANGE / LISTEN**. ARRANGE advances
the original narrative recording project. Returning to the workstation
after that day's first session offers **BUILD / SOFTEN**, both valid ways
to advance. Four sessions finish the master tape, unlocking the
Advanced Studio. No exact rhythm timing, piano skill, or sound-design
experience is required.

After unlocking, interacting with the desk presents a simple choice:
**LISTEN** (the familiar one-button BORE playback) or **STUDIO** (the
full editor). The editor is always optional and never an ending gate.
Neither menu contains destructive music or ethical choices: the old
optional **ERASE DEMO** action remains behind its separate deliberate
B / RIGHT / A confirmation.

## Advanced Studio — six functional, readable tabs

The advanced layout is inspired by the Bogotchi music-studio section
hierarchy but reduced and reformatted to Danny Steel's actual **120x80**
GBA bitmap, with full-size lettering and a consistently highlighted
selection. Tabs are SONG, NOTES, ORDER, LYRICS, CHOIR and PERFORM;
the 3–4-character tab labels are compact but their selected page always
shows its full meaning.

| Tab | What it actually changes |
|---|---|
| SONG (SNG) | Select four tempo presets and three PSG instrument feels. Both change the live preview. |
| NOTES (NTS) | Edit an actual eight-step C-major melody on a seven-row piano-roll grid; rest positions are visible. Up/down changes the selected pitch, **A** toggles rest. |
| ORDER (ORD) | Arrange four successive eight-step phrases using BASE, UP, LOW and FILL variations. This changes melodic pitch and chord roots during playback. |
| LYRICS (LYR) | Choose among four authored lyric **cue cards**. They appear during the performance preview. This is text, not sung audio. |
| CHOIR (CHR) | Choose SOLO/WARM/BRIGHT/FULL PSG accompaniment. This adjusts instrumental harmony density and register, not realistic recorded singers. |
| PERFORM (LIVE) | Hear the edited sequence, see its steps and displayed lyric cue, and turn playback off without losing your work. |

### Controls everywhere in the editor

- **L/R:** previous/next tab, with the heading and selected tab shown.
- **D-pad Left/Right:** select a note, phrase or SONG setting.
- **D-pad Up/Down:** raise/lower pitch or change the current setting.
- **A:** set/cycle the selected item; in NOTES, toggle a rest. On LIVE, toggle playback.
- **SELECT:** play/stop the *actual edited PSG sequence* from any tab.
- **B:** exit to the game without discarding changes; **START** exits to pause.

No hidden touchscreen gestures, cryptic abbreviations without a label,
nonfunctional preview button or multi-step confirmation chains.
The live PSG preview is the real hardware synthesizer, not a picture
of a waveform. All sound is generated with the four built-in GBA
PSG channels, a 32-step repeating structure, and bounded events.
It is a compact tracker/arranger, not a complete sampled DAW,
free-form recording microphone or generated vocal studio. The long,
authored Here Today ending score remains unaffected by an editing
preview.

## Engine, persistence and safeguards

`src/studio.c` holds an eight-note melody, four phrase choices,
tempo/mood, selected lyric cue and PSG harmony setting in a small fixed
`StudioPatch`. `src/music.c` routes a dedicated live-studio cue into
the existing four-channel sound registers, using no PCM buffers,
extra framebuffer, dynamic allocation, large asset sets or per-frame
floating point. `src/story_draw.c` paints the editor straight into the
original 120x80 bitmap; the normal life clock pauses while it is open,
so a doctor appointment cannot interrupt a phrase edit.

The user's changes are saved per slot in a **separate, 24-byte,
checksummed optional sidecar**, starting at SRAM offset 0x140;
the previous 31-byte story saves, the 8-byte Mara rescue sidecar and
all older saves stay compatible. Invalid or absent studio sidecars
restore the safe factory melody without making a save slot invalid.
Erasing a slot erases its studio data as well. Edited music does not
replace the original, credited BORE soundtrack or erase its source
recordings.

Host tests cover unlock and basic listening, all six pages, input
behaviour, real note/phrase/harmony changes, audible PSG cue routing,
per-slot serialization/checksum/corruption, pause and ending music
priority, guarded 2D/3D 120x80 render targets and the original project
and story routes. Real hardware/emulator capture is still needed for
objective frame-time and sound-level measurements.
