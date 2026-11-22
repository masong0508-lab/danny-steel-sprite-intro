# Danny Steel: accessible objective and choice navigation (development milestone)

The existing 24x18 Old Quarter, consequences, saving, music editor and
three endings remain unchanged. The usability problem was that a direct
cardinal heading pointed into buildings while the objective marker sat on
a *solid* encounter tile. The label named a destination but offered no
route or way to tell when A would actually interact.

## How it works now

- The compact compass follows the shortest **walkable** route to a valid
  square adjacent to the current story objective. It respects all solid
  tiles and uses deterministic cardinal steps, not a straight-line guess.
  On the street the heading shows a direction and the remaining step count.
  In first-person the instruction is relative to Danny's facing: AHEAD,
  RIGHT, BACK, or LEFT. HERE means A can be used at that location.
- The A-button prompt matches the real interaction target, including
  the Potheads shift and the fallback to nearby actors when not facing one.
- Pause with START and press R repeatedly for a new fourth FIELD GUIDE
  page. It explains the current objective, WHY it matters, the safe route,
  and the non-obvious risk-menu control. B backs out of that journal and
  START resumes; the old completion-check pages remain intact.
- The pocket notebook uses shorter, more characterful language. Pressing
  A at an empty location yields a useful hint rather than an opaque
  'nothing to do' line. Dangerous choices are NEVER auto-confirmed.

## Performance and persistence

The route query caches the player and objective coordinates, so ordinary
rendering does not perform a full BFS each frame. Scratch is a single
fixed ~1.3 KiB static table; no dynamic allocation, extra bitmap, save-state
field or change to the town's collision/encounter grid. The game still
uses the identical SRAM layout and the same story logic.

## Regression gate

An independent reference BFS checks every reachable starting cell,
all objective stages, path lengths, and that the next step is walkable.
Tests also verify reachability from the Potheads spawn, situation-aware
action prompts, the fourth field-guide page, and 120x80/240x160 frame
guards. Native and host builds remain required before release.

## Combat readability

Mirror Orchard now telegraphs the next ACTUAL attack lane in a label as
well as by drawing its moving paper hazard. The strike resolves at 30
VBlanks (half a second, rather than 15 VBlanks), with four attacks per
wave and two waves per duel. A parry lasts 15 ticks; a 30-tick cooldown
means it still requires timing rather than button mashing. CARE visibly
provides a shield against the first hit and EXIT remains a safe choice.
These fights stay optional and do not gate Danny's clinic visits, art,
music, or any ending. A separate combat fairness test confirms the
telegraphed lane, timing, guard absorption, and opponent continuity.
