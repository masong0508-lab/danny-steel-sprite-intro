# Seeded city and locked landscape tools

Every new save automatically selects a pseudorandom layout from the 192
available arrangements. A saved shuffled pool, mixed with title/menu timing,
prevents repeats until the pool has been exhausted. The first fresh game is
also randomized. Existing saves without seed records retain layout zero.
Save-slot copies use separate checksummed records; the global cursor
survives erasing a slot. When both seed copies are missing or corrupt, restore
the canonical layout and retain campaign progress, falling back to the
canonical spawn if necessary.

The generator reflects the 22x16 core interior across either axis and
permutes four equal-size 5x4 building parcels. Seeds 96..191 also swap
two 6x9 neighborhood tracts: the southern park and an eastern built block.
All transformations preserve tile counts and the first 96 layout IDs remain
byte-for-byte compatible with previous saves. Gateways, countryside, terrain heightfield and hiking checkpoints
remain fixed. Story objectives, neighborhood encounters, Potheads spawn and
work interaction, billboard actors, architecture identity, furniture and
material lookup follow their relocated coordinates. Route and perspective
caches invalidate on layout changes. All 192 combinations passed an independent
layout-model connectivity check (1,351 reachable walking tiles and the same
two pre-existing sealed archive rooms).

Terrain raising/lowering, survey plot actions, brush controls and survey
markers remain hidden in ordinary play. Every regular cartridge includes
the pause-screen Konami unlock, described below. The regular release retains exploration,
mountains, hiking, outdoor art and lakeside rest. Completion-based editing
unlock is intentionally deferred; endings currently stop gameplay, and a
proper completion/free-roam system would be needed.

Host regression exercises every seed, map inventory, real main-story input,
relocated neighborhood sites, save record checksums, both rendering modes and
the default release lock. The previous rescue SRAM assertion now checks the
actual base save byte range rather than depending on map area. Full CI and
emulator verification remain necessary.

## Cartridge memory correction

The initial seeded-city cartridge link placed `.data` beyond the 32 KiB IWRAM boundary (0x030081b4). The host tests passed, so the issue was cartridge-specific storage placement. The 7,680-byte route workspace, 1,200-byte generated urban map and 775-byte terrain mesh now explicitly reside in aligned EWRAM: 9,655 bytes of payload moved out of IWRAM. These arrays are populated before use. The existing 24 KiB IWRAM and 192 KiB EWRAM CI gates remain unchanged; exact linked sizes and emulator behavior require a fresh build.

The expanded generator has 192 distinct arrangements, each with 1,351 reachable tiles. The park's name, objects, visual identity, and local dialogue follow the moved tract. A new regression preserves all 96 prior layout hashes and draws the relocated park in both cameras. The 37-step cycle still visits every seed before repeating, and the existing one-byte seed records require no migration. The terrain tools stay hidden behind the debug menu. The new cartridge link and complete integration tests are pending.

## Hidden developer world controls

Ordinary New Game has no seed picker, regeneration, survey or land editing
controls. Existing games load the seed they were originally given. The
global, CRC-protected selection pool survives slot erasure, and the previous
cursor migrates without repeating the last generated layout. The original
192 seed recipes and existing saves remain unchanged.

In the regular cartridge, press START during ordinary roaming. At the
pause root enter UP UP DOWN DOWN LEFT RIGHT LEFT RIGHT B A START, releasing
each button between presses. The full code opens World Tools; incomplete
or wrong codes do not. Select
Generate New World, change the preview seed with LEFT/RIGHT (R advances the
preview), and press A twice to confirm. B cancels before any mutation. The
player and facing relocate with their original lot; flags, creative work,
hike and other progress are retained. The Terrain Brush and Survey Grades
options require entering the countryside first. A/B edit the selected
terrain mode; SELECT or R exits. Reset Terrain requires confirmation and
restores all survey and brush grades without resetting the hike. Closing World Tools or exiting an editing mode relocks access; the code
must be entered again. No unlock state is saved. START+SELECT is not an
unlock shortcut in any build.

To optimize seeded rendering, the generator now bakes 1,200 source and
destination coordinate pairs in two 16-bit external-RAM tables, adding
4,800 bytes of EWRAM payload. The mapping lookup replaces repeated parcel
searches in wall, object and furniture lookup. The actual emulator movement
probe reads the current seed from its matching ELF and walks away from the
relocated shop, rather than assuming the original map. New tests cover three
complete random pools, save/load, debug confirmation/cancel and release lock.
Exact cartridge RAM and performance still need verification.

## Main ROM secret access

The same ordinary ROM now contains the World Tools panel. There is no special
developer build flag, no ordinary menu entry, and no shortcut that bypasses
the code. The sequence is recognized only at the root of pause during safe
roaming, with a single newly pressed button per step. Held directions do not
repeat, changing menus/resuming/reloading clears a partial sequence, and
the B/A/START actions are consumed only at their correct code positions.
Choosing a terrain mode from the secret menu grants transient access only
to that mode; leaving it removes access. Existing world/terrain saves and
automatic randomized New Game are unchanged. Host and actual mGBA input
regression coverage were extended; this revision still needs fresh CI.

## World selector preview and overhead rendering pass

The secret menu now displays a collision-aware proposed world map while choosing a seed. Preview generation uses a separate map buffer and leaves the live seed, routing and geometry caches untouched. Overhead rendering skips the entire off-screen prefix of each projected row using exact integer clip calculations. Furniture is addressed through a small canonical ROM table, rather than scanning all 18 decorations at every candidate tile; seeded mapping remains unchanged. The fixed Old Quarter lamp coordinate is computed once per frame. Host tests compare the clip calculations with the old scan, verify furniture in all 192 arrangements, and compare proposed-map samples without changing the live layout. Emulator capture now includes World Tools. Fresh cartridge performance is measured by the existing frame-cadence harness.
