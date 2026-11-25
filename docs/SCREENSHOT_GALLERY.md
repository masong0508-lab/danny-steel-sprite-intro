# Danny Steel — actual screenshots

These are **captures from the playable GBA ROM and the game's own
C renderer**, not generated illustrations or speculative mockups.

**Emulated, true-ROM captures (mGBA):** the first four images were taken
by compiling the current source revision to `danny_steel_title.gba` and
executing it in headless mGBA, sending
START/A inputs to navigate its opening title and new-game menus.
They were upscaled with nearest-neighbour interpolation for legibility.

**Gameplay/story engine frames:** the remaining thirty-two images are made by
the exact production C rendering functions, including `hd_world_draw`, `hd_first_person_draw`, `hd_cinema_draw`,
`hd_studio_draw`, `hd_special_cinema_draw` and `hd_ending_draw`,
compiled for a desktop host with the identical game art, palettes and
direct-from-cartridge battle illustrations. These frames were
captured at carefully chosen story beats; they are *representative
screenshots of the code*, not a claim that one uninterrupted live run
reached every story milestone. The Advanced Studio frames use a completed-project state to demonstrate the
unlocked and fully editable interface.

![Contact sheet of Danny Steel screens](screenshots/contact_sheet.png)

## Title and main menu

| Title | Main menu | New-game save slots |
| --- | --- | --- |
| ![Opening title](screenshots/01_title.png) | ![Main menu](screenshots/02_main_menu.png) | ![Save slots](screenshots/03_save_slots.png) |

The photo-texture title preserves the drifting painted background,
hand-distressed wordmark, animated skyline and mellow tonal identity.
The menu is now a native 240x160 illustration of a rainy Old Quarter street,
with printed narrative choices and no rotating-square or gray-video overlay.

## Gameplay and dialogue

| Frame | Image |
| --- | --- |
| Opening scene, as seen in mGBA | ![Opening story](screenshots/04_story_in_emulator.png) |
| Story arrival, renderer capture | ![Arriving in the Old Quarter](screenshots/05_story_arrival.png) |
| Overhead exploration | ![Overhead gameplay](screenshots/06_gameplay_overhead.png) |
| First-person exploration | ![First-person view](screenshots/07_gameplay_first_person.png) |
| Archive-side 3D view | ![Archive angle](screenshots/13_3d_from_archive.png) |
| Waterfront 3D view | ![Waterfront angle](screenshots/14_3d_waterfront.png) |
| BORE continuity office | ![BORE office conversation](screenshots/08_story_bore_office.png) |
| Mara's café | ![Mara conversation](screenshots/09_story_mara_cafe.png) |
| Last bus | ![Bus scene](screenshots/10_story_last_bus.png) |

## Inked Old Quarter: native art direction pass (November 24)

The cinematic art uses stronger warm practical lights against subdued slate
shadows, layered building silhouettes, recessed windows, perspective floor
joints, recognizable bistro furniture and a consistent red/cream bus livery.
Danny and Mara's close-ups have distinct eye spacing, cheek and nose planes;
Mara's folded teal scarf is integrated with her costume. The menu uses
Danny's existing gameplay sprite rather than a separate block-shaped figure.

| New renderer evidence | Capture |
| --- | --- |
| Cafe staging and ceramic cups | ![Cafe scene](screenshots/05i_mara_table.png) |
| Mara reaction | ![Mara close-up](screenshots/05p_mara_reaction.png) |
| Clinic interior | ![Clinic](screenshots/05v_clinic_danny_reaction.png) |
| Canvas underdrawing | ![First stage](screenshots/05zd_canvas_underdrawing.png) |
| Canvas silhouettes | ![Second stage](screenshots/05ze_canvas_silhouette.png) |
| Completed warm painting | ![Finished canvas](screenshots/05zf_canvas_finished.png) |
| Completed recording | ![Studio master](screenshots/05zg_recording_finished.png) |

All are the existing native 240x160 painters. No new image atlas or
framebuffer is required, and story flags and collision are unchanged.

## Old Quarter's new cartridge-authored artwork

The overhead camera now uses its own native 16×16 pavement, masonry,
foliage and water illustrations stored as ROM pixels, plus a common
four-direction character atlas. These are **real exploration screens**,
at valid walkable locations in the original 24×18 game town, rather
than separate backdrop mockups.

| Area | Actual rendering |
| --- | --- |
| Archive plaza and townspeople | ![Archive square](screenshots/33_quarter_archive_plaza.png) |
| Mara's street and adjacent residents | ![Mara's neighbourhood](screenshots/34_quarter_mara_street.png) |
| Same cafe when Mara is missing | ![Empty café](screenshots/34a_quarter_empty_cafe.png) |
| Empty cafe in first person | ![First-person empty cafe](screenshots/34b_first_person_empty_cafe.png) |
| Initial easel in town | ![Underdrawing](screenshots/34c_quarter_easel_start.png) |
| Completed warm painting | ![Finished easel](screenshots/34d_quarter_easel_complete.png) |
| Completed recording | ![Recording console](screenshots/34e_quarter_recording_master.png) |
| Bus stop, architecture and street props | ![Bus street](screenshots/35_quarter_bus_and_facades.png) |
| Riversides and residents | ![Riverside residents](screenshots/36_quarter_riverside_residents.png) |
| Potheads' real storefront, isometric camera | ![Potheads storefront](screenshots/37_potheads_storefront.png) |
| Differentiated civic shop fronts | ![Civic facade art](screenshots/38_civic_facades.png) |
| Four-phase walk: contact | ![Walking contact](screenshots/38a_walk_contact.png) |
| Passing pose | ![Passing](screenshots/38b_walk_passing.png) |
| Other foot contact | ![Other foot](screenshots/38c_walk_contact_other.png) |
| Other passing pose | ![Other passing](screenshots/38d_walk_passing_other.png) |
| Potheads' matching first-person shop wall | ![Potheads in first person](screenshots/39_potheads_first_person.png) |

The original 2D and 3D views now use **the same costume cels** for named
townspeople; the same map positions, facing, interaction coordinates and
collision rules continue to determine playable movement. This art pass
does not invent additional town sectors or quests.

## Additional authored scenes and ending shots

The images below depict explicit story conditions for visual verification.
They are **not** presented as one continuous playthrough or proof that a
given ending occurs without meeting its game-state requirements.

| Scene | Actual renderer capture |
| --- | --- |
| Witness meeting and handoff | ![Witness and clue](screenshots/18_story_witness_handoff.png) |
| Server archive evidence room | ![Server archive](screenshots/19_story_archive_terminal.png) |
| Safe-house visit | ![Safe-house](screenshots/20_story_safehouse.png) |
| Unanswered café, no confirmed recovery | ![Unanswered cafe](screenshots/21_story_mara_missing_cafe.png) |
| Closed rescue door | ![Closed apartment door](screenshots/22_story_closed_rescue_door.png) |
| Successful rescue, door opening | ![Rescued Mara](screenshots/23_story_rescue_saved.png) |
| Neutral ending's rooftop memories | ![Mara and the painting](screenshots/24_neutral_rooftop_memories.png) |
| Good ending's record-store legacy | ![Master record](screenshots/25_good_ending_recordshop.png) |
| Good ending's live coda | ![Concert](screenshots/26_good_ending_concert.png) |
| Unresolved bad-ending café | ![No reply at the cafe](screenshots/27_bad_unanswered_cafe.png) |
| Bad ending after a successful rescue | ![Open doorway](screenshots/28_bad_saved_rescue_door.png) |

These keep the original conditional outcomes: the unresolved route
does not show Mara safely returning, and the rescued route does not
present her as missing. Captions are from the original timed script.

## Mirror Orchard — cartridge-illustrated native battles

These four scenes are rendered by the actual `hd_arena_draw` code at
240×160, not pixel-doubled 120×80, and use original illustrations stored
in cartridge ROM. They depict different existing story battle states:
three rival identities plus the post-investigation parry. The character
head, hat and costume ears stay visible beside the menu instructions;
the FOCUS and BEAT readouts now identify their gameplay meaning.

| Encounter | Actual frame |
| --- | --- |
| RATIO RAT, FACTS selected | ![Rainy street and rat rival](screenshots/29_orchard_rat_menu.png) |
| BADGE LORD, CARE selected | ![Clerk room and badge rival](screenshots/30_orchard_badge_care.png) |
| LORE WARDEN, dodge and moving paper warning | ![Vinyl archive dodge lane](screenshots/31_orchard_warden_dodge.png) |
| REBRAND RAT, six-tick parry active | ![Rainy street parry animation](screenshots/32_orchard_rebrand_parry.png) |

Combat remains a single playable, state-driven three-lane encounter; its
actual collision timing, focus, parry cooldown, exit and side-story save
effects are unchanged by the art pass. See [the native arena
notes](NATIVE_MIRROR_ORCHARD.md).

## Unlockable Advanced Studio

| Song controls | Note editor | Arrangement |
| --- | --- | --- |
| ![Song controls](screenshots/15_advanced_studio_song.png) | ![Notes](screenshots/11_advanced_studio_notes.png) | ![Order](screenshots/16_advanced_studio_order.png) |

| Lyrics cue | Performing arrangement |
| --- | --- |
| ![Lyrics](screenshots/17_advanced_studio_lyrics.png) | ![Perform](screenshots/12_advanced_studio_perform.png) |

Capture source: `tools/story_gallery.c` and
`.github/workflows/screenshots.yml`. The main menu, authored overhead atlas, first-person streets, native
cinematic locations and the six-tab Studio render at 240x160 GBA resolution.
All 24 authored cinematic scene types and the three score-length ending
montages now render at native 240x160. The optional Mirror Orchard
combat arena also uses native 240x160 ROM-backed environments and portraits
while retaining the original game logic. All final pictures are displayed at
2x their GBA size using nearest-neighbour scaling. No images are mockups.

## Illustrated neighborhood anthology

The 42 panels in `41_*_side_*.png` are captures of actual A-button
interactions, including both alternatives at each of fourteen map sites.
`42_neighborhood_journal.png` displays completion and choices.

| Location | Opening | Left choice | Right choice |
| --- | --- | --- | --- |
| LAUNDRY | ![Opening](screenshots/41_01_side_intro.png) | ![MEND](screenshots/41_01_side_left.png) | ![RETURN](screenshots/41_01_side_right.png) |
| PHOTO | ![Opening](screenshots/41_02_side_intro.png) | ![SIGN](screenshots/41_02_side_left.png) | ![NO NAME](screenshots/41_02_side_right.png) |
| STAIR | ![Opening](screenshots/41_03_side_intro.png) | ![SKETCH](screenshots/41_03_side_left.png) | ![LISTEN](screenshots/41_03_side_right.png) |
| REPAIR | ![Opening](screenshots/41_04_side_intro.png) | ![REPAIR](screenshots/41_04_side_left.png) | ![DONATE](screenshots/41_04_side_right.png) |
| POST | ![Opening](screenshots/41_05_side_intro.png) | ![WRITE](screenshots/41_05_side_left.png) | ![KEEP](screenshots/41_05_side_right.png) |
| STOOP | ![Opening](screenshots/41_06_side_intro.png) | ![KNOCK](screenshots/41_06_side_left.png) | ![NOTE](screenshots/41_06_side_right.png) |
| MARKET | ![Opening](screenshots/41_07_side_intro.png) | ![HELP](screenshots/41_07_side_left.png) | ![BUY](screenshots/41_07_side_right.png) |
| ANNEX | ![Opening](screenshots/41_08_side_intro.png) | ![LABEL](screenshots/41_08_side_left.png) | ![RESTORE](screenshots/41_08_side_right.png) |
| GARDEN | ![Opening](screenshots/41_09_side_intro.png) | ![PLAN](screenshots/41_09_side_left.png) | ![REST](screenshots/41_09_side_right.png) |
| SHIFT | ![Opening](screenshots/41_10_side_intro.png) | ![FOLD](screenshots/41_10_side_left.png) | ![THANK](screenshots/41_10_side_right.png) |
| PARK | ![Opening](screenshots/41_11_side_intro.png) | ![SHARE](screenshots/41_11_side_left.png) | ![SIT](screenshots/41_11_side_right.png) |
| PHONE | ![Opening](screenshots/41_12_side_intro.png) | ![CALL](screenshots/41_12_side_left.png) | ![MESSAGE](screenshots/41_12_side_right.png) |
| CANAL | ![Opening](screenshots/41_13_side_intro.png) | ![SKETCH](screenshots/41_13_side_left.png) | ![CLEAN](screenshots/41_13_side_right.png) |
| SHELTER | ![Opening](screenshots/41_14_side_intro.png) | ![MARK](screenshots/41_14_side_left.png) | ![WAIT](screenshots/41_14_side_right.png) |
