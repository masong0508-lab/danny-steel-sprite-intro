# Illustrated Old Quarter anthology

14 playable side stories, 28 selectable outcomes, 84 dialogue pages and
42 original 240x104 palette-indexed illustrations (1,048,320 bytes).
All art is used at real reachable locations. The binary art is reproduced
by tools/build_neighborhood_panels.js and verified by golden hashes.
No padding, new framebuffer, heap, map geometry or core SRAM version change.

| Location | Tile | Choice 1 | Choice 2 |
| --- | --- | --- | --- |
| LAUNDRY | 2,2 | MEND | RETURN |
| PHOTO | 9,2 | SIGN | NO NAME |
| STAIR | 14,2 | SKETCH | LISTEN |
| REPAIR | 21,3 | REPAIR | DONATE |
| POST | 2,5 | WRITE | KEEP |
| STOOP | 6,5 | KNOCK | NOTE |
| MARKET | 8,8 | HELP | BUY |
| ANNEX | 13,8 | LABEL | RESTORE |
| GARDEN | 15,9 | PLAN | REST |
| SHIFT | 20,9 | FOLD | THANK |
| PARK | 2,12 | SHARE | SIT |
| PHONE | 10,13 | CALL | MESSAGE |
| CANAL | 14,14 | SKETCH | CLEAN |
| SHELTER | 22,15 | MARK | WAIT |

Finish letter/stamp/Mara to unlock the stories. START then SELECT opens
the two-page journal; D-pad selects, A tracks/untracks and resumes.
The existing route cache finds a real walkable destination. At the
destination press A STORY. B cancels; only finishing an outcome records it.
Replays may change the record but cannot farm stats. Urgent plot routes
override optional tracking. Six Story bytes and an independent slot-tagged
CRC-8 SRAM record preserve completed outcomes. Old saves remain valid;
new game and slot erase clear the records. Cancel wins if A and B are
pressed simultaneously, including the pre-existing Mara promise.

The 14 encounters take place at previously walkable outdoor locations:
they open illustrated interaction scenes rather than additional walkable
interior geometry. Music uses existing thematic cues. Physical hardware
FPS and the complete long-form campaign are not yet certified.
