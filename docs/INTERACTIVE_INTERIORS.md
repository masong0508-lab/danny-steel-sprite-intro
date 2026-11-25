# Interactive Old Quarter interiors

This adds another 1,048,320 bytes of visible, ROM-backed artwork (28
240×140 rooms and 14 240×32 workbench strips). The background and solid
furniture use the exact same authored collision layout; no additional
image RAM or framebuffer is allocated.

- **Entry:** complete letter/stamp/Mara, choose a site in START → SELECT,
  follow the outdoor route, press R at the site. A still opens the
  existing illustrated branch encounter.
- **Inside:** D-pad walks; A on the door enters the second room, A next
  to a numbered workstation opens its activity. R opens instructions.
  B cancels, returns to the room, or exits; START exits and pauses.
- **Games:** every building contains three sequential tasks, with one
  dial setting, one moving-needle stop, and one three-direction pattern.
  Wrong attempts are retryable, with no penalty or accidental reward.
- **Writing and consequences:** each task has location-specific
  instructions, a result, and a callback to the chosen illustrated
  side story. Work rewards are bounded and one-time. Mara reacts to
  helping the community and the BORE clerk to the annex work.
- **Saves:** an independent, CRC-checked per-slot record at 0x1A0
  preserves completed buildings; it does not alter the original save
  record or first expansion's record. Reload resumes on the outdoor
  entry tile. A due clinic appointment interrupts optional activity.

| Building | Station order |
| --- | --- |
| Laundry | Sort, wash, fold |
| Photography | Frame, focus, print |
| Rooftop studio | Measure, sketch, shade |
| Repair shop | Check, tune, test |
| Post office | Address, stamp, file |
| Neighbor's house | Find, write, shelve |
| Market | Sort, weigh, pack |
| Archive annex | Label, splice, sleeve |
| Clinic garden | Seed, water, plan |
| Potheads staff room | Clean, fold, sign out |
| Park pavilion | Repair, paint, share |
| Phone office | List, dial, message |
| Canal boathouse | Sort, clean, sketch |
| Bus shelter | Check, mark, post |

## Performance

The first-person renderer now caches immutable wall hit geometry and
height alongside existing shade and depth strips. On an unchanged
camera it performs zero ray-march steps, while weather and characters
still animate. 60 exact floor-row projections use a 360-byte read-only
table in place of per-frame divisions. The floor's horizontal coordinate
uses incremental fixed point; native sampling, clipping, and wall
occlusion are unchanged. Existing golden-frame tests and new
cold/warm ray-count checks are retained. Actual GBA FPS still requires
successful emulator and hardware measurements.

The new deterministic test drives all 14 entries and 28 rooms with
button presses, completes all 42 rounds, validates all 42 artwork
hashes, tests wrong attempts, cancellation, replay rewards, plot
interruptions and save integrity. It preserves the sub-64-byte Story
budget and runs before the existing host and GBA gates.
