# Offline GBA builds and source continuity

November 26, 2026: hosted Actions could not start its runner jobs. The GNU ARM GCC 14.2.1 freestanding offline route compiled and booted a recovery snapshot in mGBA. This is not an assertion that this branch itself was fully compiled.

## Build

From the exact checked-out source tree, install GNU ARM Embedded GCC and binutils (or supply ARM_PREFIX as the full tool path prefix), Python 3, and all three owner-private V4 headers: potheads_cloth_data.h, potheads_leg_data.h, potheads_upper_data.h in src/. Then run:

    sh tools/offline/build.sh

The script uses the existing game cartridge at dist/danny_steel_title.gba for its 188-byte header tail only. Set GBA_HEADER_ROM to a different valid existing game ROM if needed. PRIVATE_ROTOSCOPE=0 permits a deliberately source-only build; partially installed masks are rejected. The offline linker asserts conservative IWRAM and EWRAM limits. Output is build/offline/danny.gba, danny.elf, danny.map, and manifest.json. The normal devkitARM workflow is unchanged.

## Verified recovery, not a release of the branch

Base source 844619e6400df46042478f8df09630f9ddb014ec (owner's November 21 backup) with selected V4 renderer files verified against 7e2da783ebf1d338ead581f75e9bf57d15ca741e and the three private data headers. Later gameplay and world changes are NOT in this recovery cartridge. It booted the title, menus, played the opening through all dance phases, and returned to gameplay in mGBA. Four local rotoscope tests passed. The build reproduced byte-for-byte. ROM SHA-256: 97a8001eff7993646bc1ce640df5a9b9a9f093a8509484d8bb00baa38c128587 (1713072 bytes). Static IWRAM 20168 bytes; static EWRAM 38400 bytes. The full current branch remains unverified by the offline build because the authenticated source archive could not be retrieved in the local environment. Do not substitute the recovery source over the branch.

## Required release bookkeeping

* Commit game changes to the authoritative development branch before labeling a ROM current; retain all existing commits and unrelated gameplay features.
* Build the exact committed checkout, with the matching owner-private assets. Do not publish private media/derived masks or a recovery ROM to the repository.
* Keep the source snapshot, manifest, ELF/map, ROM checksum, test logs and emulator captures together. If any input is missing, mark the build as partial/recovery, never as a full current release.
* Verify boot, input, menu, opening and gameplay on the actual produced ROM; distinguish host tests from emulation and physical hardware.
* Never overwrite a newer development tree with an older backup. Recovery and release must stay separately identifiable. Use the complete authenticated source ZIP to reproduce the latest revision locally when Actions is unavailable.
