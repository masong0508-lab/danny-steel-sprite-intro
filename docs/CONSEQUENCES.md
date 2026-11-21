# Gameplay consequences, not save checkmarks

The game's new **Life Record** is a calculated gameplay checklist, not
an SRAM validity mark and not a separate trophy subsystem.

## What counts

It continuously checks **15 conditions** already present in Danny's
playthrough. The source of truth remains the actual story and simulation
state: five story milestones, five relationship/creative/community/clinic
milestones, four harmful choices, and Mara's survival. There is no extra
save format or "checkmark earned" database.

- **Main Story (0–5):** the letter, BORE stamp, promise to Mara, visiting
  the music archive and starting a canvas.
- **Life & Legacy (0–5):** the mastered BORE tape, finished painting,
  relationship with Mara (bond 4+), resolving Mirror Orchard, and
  attending a clinic visit. These change dialogue or playable options,
  not just a score.
- **Choices & Risks:** four separately confirmed bad actions show a
  red X and a 0–4 danger count. A fifth item shows whether Mara was
  rescued. This is separate from the risk count, because saving a person
  does not cancel or equal another harmful action.

The first two pages use a light check for completed and a hollow box for
unfinished. The risk page uses red X marks for *choices actually made*,
not for mistakes such as moving slowly, running out of energy, or
missing a timed input. Nothing is silently earned by time passing.

## Controls and presentation

While exploring, press **START**, then **R** for the Life Record.
Press **R** again to cycle through the three pages, **B** to close the
record, or **START** to resume immediately. The ordinary pause menu
still has **B save and return to menu**. Dialogue and clinic interactions
are unchanged. The record uses the existing 120×80 palette-indexed GBA
buffer with small pixel-art check and cross glyphs.

The five original main-story indicators are now legible miniature check
icons, and a two-check danger warning appears in the exploration footer.
Daily choices preview their effects before confirmation. Cinematic
dialogue shows speaker-accented panels and page indicators.

## World reactions

These checks are consumed by *logic*, not only by the HUD:

- If the Mirror Orchard investigation is resolved, the witness recognizes
  Danny during Mara's rescue and provides a more personal testimony.
- If Danny has earned Mara's trust, her safe-house reunion acknowledges it.
- If the rooftop art is finished, an alternative reunion mentions it,
  even when relationship trust is still low.
- The existing song, canvas, clinic, bad-choice and rescue milestones
  continue to affect opportunities, scenes and endings.

The mission remains preventable and rescue failure remains non-graphic.
No new heap buffers, graphics assets, or SRAM records are added.
The original v1–v5 save compatibility and the separate checksummed
rescue extension are unchanged.

Tested by `tests/consequence_journal_test.c`: each condition, purity of
the check queries, notebook access/cycling/back navigation, rendered
2D/first-person palette and bounds, and three NPC story reactions.
