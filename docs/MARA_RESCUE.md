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

The failed rescue is conveyed only by an empty café, an extinguished light
and unanswered messages. **Mara's death or other harm is never shown or
confirmed.** A separate ending sequence plays Here Today in full with no
good-ending coda. All alternate ending scenes use the existing 120x80
palette-indexed framebuffer and authored world locations.

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
