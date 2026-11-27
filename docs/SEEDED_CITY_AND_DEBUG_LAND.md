# Seeded city and locked landscape tools

Every new save selects a layout seed. The first new game on erased SRAM uses
the reference layout; subsequent new games cycle through a 96-layout space
using a coprime increment. Existing saves without seed records retain layout
zero. Save-slot copies use separate checksummed records; the global cursor
survives erasing a slot. When both seed copies are missing or corrupt, restore
the canonical layout and retain campaign progress, falling back to the
canonical spawn if necessary.

The generator reflects the 22x16 core interior across either axis and
permutes four equal-size 5x4 building parcels. Both transformations preserve
tile counts. Gateways, countryside, terrain heightfield and hiking checkpoints
remain fixed. Story objectives, neighborhood encounters, Potheads spawn and
work interaction, billboard actors, architecture identity, furniture and
material lookup follow their relocated coordinates. Route and perspective
caches invalidate on layout changes. All 96 combinations passed an independent
layout-model connectivity check (1,351 reachable walking tiles and the same
two pre-existing sealed archive rooms).

Terrain raising/lowering, survey plot actions, brush controls and survey
markers are excluded from ordinary cartridges. Developers can opt in with
`make clean && make TERRAIN_DEBUG=1`. The regular release retains exploration,
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
