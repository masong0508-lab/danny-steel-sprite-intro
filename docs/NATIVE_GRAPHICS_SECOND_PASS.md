# Danny Steel — Native GBA graphics, second pass

This pass follows the initial 240x160 menu and overhead-gameplay rebuild.
The player's screenshot feedback identified two remaining major visual gaps:
the blocky doubled-resolution 3D town and the small six-tab music editor.
Those are now drawn at the GBA's physical 240x160 resolution, alongside
twelve more richly staged, common story cutscenes.

## What changes in the game

**First-person / 3D street exploration:** `src/hd_perspective.c` reuses
exactly the existing 24x18 town collision grid, four cardinal headings,
interpolated movement, NPC IDs and quest positions. The renderer paints
240 independent wall columns (not a 120px picture scaled twice), native
floor samples and world-coordinate facades, windows, cobbles, water,
foliage, street-facing props and depth-tested characters. Angles and
map-cell crossings are calculated with integers; floor tile samples and
ray-cell crossings are cached within each frame. Both viewing modes still
share the same story engine and player position. NPC dialogue and gameplay
controls are unchanged.

**Music studio:** `src/hd_studio.c` replaces the old 120x80 editor
pictures with a 240x160 BORE record-room identity. All six existing
functional screens now have distinct information architecture:

- SONG: tempo/feel settings and a slowly moving record.
- NOTES: musical grid with individually legible pitch steps, REST, and
  the real playback head.
- ORDER: four editable phrases and their actual arrangement values.
- LYRICS: a printed cue card, explicitly **text only**.
- CHOIR: the game's actual PSG layer configuration, not synthetic singing.
- LIVE: part-aware playback and a scrolling note/beat readout.

The same `StudioPatch`, input routing, GBA PSG engine, 24-byte optional
SRAM save extension and endings are unchanged. The new visuals make no
claims about recording voice or containing real sung lyrics.

**Story cinema:** `src/hd_cinema.c` draws twelve recognizable native
sets: opening bus street, old letter doorstep, BORE interior, Mara's café,
last bus, stay, record room, artist's roof, clinic notice, appointment,
follow-up and final clinic. Named people, doors, paper, instruments and
lighting occupy consistent places across dialogue pages. Existing
`cinema_frame`, `page`, subtitles, quests and speaker scripts remain
authoritative; the visuals do not rewrite the story. Specialized endings,
rare investigation scenes and the existing timed music-synchronized film
director remain on their original tested renderer for now.

## Performance and correctness

All three modules paint directly into the **existing shared 38,400-byte
EWRAM page**. Mode 4 VRAM double-buffering, palette entries 0..63,
integer drawing and single mutable story state continue unchanged.
The only new persistent depth scratch is 240 unsigned 16-bit entries
(480 bytes), for wall/billboard visibility. No heap allocations, extra
framebuffers, spritesheets, decompressed PCM samples or float transforms.

The new `tests/hd_nextgen_test.c` exercises different town positions
and all four player headings, validates every Studio tab, checks numerous
cinematic pages, and guards framebuffer boundaries and palette indices.
GitHub Actions also runs the original story, reachability, save, music,
ending, animation and narrative tests, followed by a devkitARM ROM build.
The screenshot workflow launches the built ROM in headless mGBA and
renders the same production gameplay source at selected plot states.

Passing CI is not equivalent to measured real-GBA frame rate. The native
per-column raycast is more expensive than the old low-detail view, so
emulator and real-hardware FPS profiling is still a priority before any
claim of sustained 30 FPS. A further dedicated pass should upgrade the
specialized ending shots and less common crisis/investigation cutscenes
without reducing narrative detail or ROM reliability.

## Verified picture review

The final 17-image gallery was regenerated against the actual merged
renderer and a freshly compiled ROM. I inspected the contact sheet and
corrected the Studio instruction bar after finding overlapping help text.
The revised captures show individual native 3D views, core story scenes,
and SONG, NOTES, ORDER, LYRICS, and LIVE Studio screens. The screenshots
are in `docs/screenshots/`; see `docs/SCREENSHOT_GALLERY.md`.

This verification confirms the produced pixels and source compilation,
not sustained real-hardware FPS, completeness of rare cinema, or the
quality of audio output. Those remain separate measurable tasks.
