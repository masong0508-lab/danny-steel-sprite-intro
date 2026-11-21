# Mara's preventable kidnapping — bad-check rescue branch

The main story contains **four distinct negative checkmarks**. They are
never awarded by exhaustion, passing time, missing chores or dialogue timing.

Stand beside the **letter (L), BORE clerk (C), music archive (M), or Mirror
Orchard terminal (S)** and press **B**. The contextual risk screen offers
**BACK** by default. Select the clearly labelled bad action with
**RIGHT** then **A** to confirm. **B** cancels. A station can award only
one bad checkmark; repeating it does not add more.

At **3 of 4** checks, Mara disappears. The player receives an in-world
message and an objective guide; there is no unseen deadline. Follow
**WITNESS (6,11) → TERMINAL (21,7) → SAFE HOUSE (7,2)**. At the
safe-house door, OPEN is the default and rescues Mara. LEAVE is the explicit
failure choice. Taking the fourth distinct bad action while she is missing
also fails the rescue.

The failed rescue now unfolds as a **playable two-day unanswered-phone
subplot**, not an immediate bad-ending film:

- **Day 0 — hope.** At the unlit safe house, an unidentified caller tells
  Danny that Mara *will* answer if he gives her a day. He keeps the phone
  close. The promise is deliberately unverified, not evidence that she is
  safe or a guaranteed future call.
- **Day 1 — realization.** After a full 4-minute in-game day, there has
  been no return call. The caller admits the report was unverified, and a
  witness says Mara **still has not been recovered**. Danny understands
  he was led on but keeps hoping she may answer.
- **Day 2 — ending.** Only after another full playable day does the bad
  ending begin. Mara's fate remains explicitly *unknown*.

During this time, the café phone, witness, terminal and pocket notebook
(**R**) offer follow-up lines. The goal arrow points toward the café
phone and the HUD tracks each numbered day. **Pause and dialogue freeze
the clock**, so a player cannot be cut off mid-conversation. A fourth
bad check while Mara is missing follows the same delay; the fourth
check *after successfully saving her* still leads to its separate
immediate bad ending that makes clear Mara is alive.

**Mara's death or other harm is never shown or confirmed.** The full
Here Today film plays without the good-only coda. All alternate ending
scenes use the existing 120x80 palette-indexed framebuffer and authored
world locations.

Saving Mara preserves her survival and restores access to the bus. With all
five main objectives completed, GO can lead to the good ending; with the
three core objectives, GO starts or resumes the neutral chapter. Existing
clinic visits and the neutral rooftop ending remain accessible. If the
player deliberately earns the fourth bad check *after rescue*, the bad
ending acknowledges that Mara returned safely but Danny squandered his
second chance. No forced post-rescue countdown imposes this outcome.

### Persistence and performance

The main SRAM format remains **v5 / 31 bytes in the existing 32-byte
slot**. An independent eight-byte checksummed companion record at offset
0x100 plus 8 bytes per slot stores bad-check flags and rescue phase. Old
slots without this record default to zero checks/no crisis, while erasing a
slot clears both records. No heap or second framebuffer was introduced.
The cinema code reuses established café, terminal, clinic and safe-house
world sets.

Tests: `tests/rescue_route_test.c` for both rescue outcomes, guide goals,
cancel semantics, re-opened ending routes, full score, guarded renders;
`tests/rescue_save_test.c` for extension checksum/slot independence; all
existing regression tests remain in GitHub Actions.

## Timeline continuity

If the rescue happens during the numbered-day chapter, pending clinic visits
are temporarily rescheduled rather than staging Mara as present while she is
missing. The appointment, day progress, and eventual neutral ending become
available again immediately after rescue. The last rooftop painting likewise
cannot bypass the rescue. Ordinary world time may pass during the search,
but time by itself never fails the mission.

## Hope subplot save compatibility

No new SRAM fields were introduced. For a failed rescue, the existing
`day_number` and `day_frame` bytes record the hope chapter's two-day
countdown, including day zero. The rescue stage still lives in its original
eight-byte checksummed companion record. The life-clinic clock is disabled
on this failed route so Mara cannot appear incorrectly at an appointment.
The game restores the day counter after loading and autosaves it every
600 frames. Older saves without the rescue extension still work.

`tests/hope_phone_test.c` and `tests/rescue_route_test.c` cover the
promised reply, delayed recognition of no verified recovery, witnesses,
phone, notebook, two-day clock, unchanged saved-Mara ending, SRAM codec,
and palette/framebuffer guards.

## Route-origin staging rule

A failed **safe-house** choice begins the hope chapter at the real house.
A fourth harmful check selected **elsewhere**, before the house is
visited, instead begins with a call about a lead going quiet. It must
not stage Danny at the safe-house door or claim he personally searched
it. Both routes keep Mara's fate unconfirmed and use the same existing
save-compatible, playable two-day phone timeline.
