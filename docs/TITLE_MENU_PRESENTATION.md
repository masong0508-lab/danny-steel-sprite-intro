# Danny Steel — photographic title and menu direction

The title and menus belong to **The Last Days of Danny Steel**, not an OS,
terminal, science-fiction dashboard or generic game-engine transition.

## The actual shipped treatment

- Keep the drifting, digitized original painting, authored title lettering,
  animated skyline and original D-major bossa score. The title now holds its
  typographic composition instead of switching the entire PRESS START layer
  on and off every half-second. A separate palette bank gives the invitation
  a soft, approximately 2.4-second warm breathing cycle without changing
  the title lettering or the painting's palette.
- Use eased black/white exposure on entering/leaving the title, never a
  flashing zig-zag or a rotating loading square.
- Keep the menu's original photographic/video loop (8.5 fps), its labels and
  two page levels. Use **muted inactive type / warm ivory active type**, and
  retain the existing smoothly travelling selection mark without the
  fake mechanical jump or flashing neon-blue color cycle.
- Replace the pre-rendered zoom-through-square transition with a **42-VBlank
  optical dip**: a centred 8% camera push, restrained exposure-to-charcoal,
  a swap of the actual menu state only once the lettering is invisible,
  then an eased exposure up to the same moving picture.
- Give both menu levels tiny printed top and bottom captions for context.
  The lower line changes for NEW GAME, LOAD, a missing save, and ERASE;
  these are player instructions, not technobabble.
- Prevent an accidental destructive erase: on ERASE -> SLOT 1 or SLOT 2,
  players **must hold A continuously for 40 frames** while a bottom
  underline fills. Releasing A early cancels. The same A press that
  entered the ERASE submenu cannot initiate the hold. A fully completed
  hold clears that one save and displays **SAVE ERASED**; empty slots
  explain themselves.
- Keep A/B/LEFT/RIGHT and existing slot mapping unchanged. No new menus,
  settings trees, confusing glyph-only navigation or double confirmations.

## Rendering, save and performance contracts

The photographic captions are drawn into the **same 120x80 buffer** just
after existing LZ77 decompression, before the mode-4 page copy. The first
and last 11-12 rows form readable darkened optical matte strips; the
midframe moving original video is not redrawn by a second compositing layer.
Two near-identical palette indices in the old 256-shade gray video are
remapped in the source footage and reserved for warm-white paper and dark
ink. The generated title maps use **text BG bank 14 and prompt BG bank 15**;
the painting's 0..223 palette indices are unchanged. The menu labels use
two existing OBJ banks for selected/dim type. No additional framebuffer,
large static allocation, heap allocation, PCM/sample buffers or float
math is introduced.

The source file `src/menu_trans_data.c` (about 394 KB of encoded
old square-zoom transition) and its generator were removed. The original
loop `src/menu_data.c` and `tools/make_menu.py` remain, as does the
existing menu sprite artwork. Git history retains the removed footage.

The host menu test checks every page/mode/slot/feedback state, the
40-frame erase indicator and guarded 120x80 bounds. GitHub CI runs these
alongside story, persistence, music and cutscene tests before compiling a
devkitARM ROM. Emulator video capture and listening tests remain useful:
CI verifies execution and bounded drawing, not artistic impressions.
