#!/bin/sh
# Rebuild without devkitARM or GitHub Actions using GNU ARM Embedded GCC.
# Cached film builds also need the host C compiler and liblz4-dev.
set -eu
cd "$(dirname "$0")/../.."
ARM_PREFIX=${ARM_PREFIX:-arm-none-eabi-}
GBA_HEADER_ROM=${GBA_HEADER_ROM:-dist/danny_steel_title.gba}
PRIVATE_ROTOSCOPE=${PRIVATE_ROTOSCOPE:-1}
command -v "${ARM_PREFIX}gcc" >/dev/null || { echo 'GNU ARM GCC not found; set ARM_PREFIX.' >&2; exit 1; }
command -v "${ARM_PREFIX}objcopy" >/dev/null
command -v python3 >/dev/null
mkdir -p build/offline
python3 - "$GBA_HEADER_ROM" "$PRIVATE_ROTOSCOPE" <<'PY'
import pathlib, sys
p=pathlib.Path(sys.argv[1]); h=p.read_bytes()[:192]
if len(h)!=192 or h[0xb2]!=0x96 or (sum(h[0xa0:0xbd])+h[0xbd]+0x19)&255:
    raise SystemExit('Invalid GBA header donor: '+str(p))
pathlib.Path('build/offline/header-tail.bin').write_bytes(h[4:])
assets=[pathlib.Path('src/potheads_'+k+'_data.h') for k in ('cloth','leg','upper')]
count=sum(p.is_file() for p in assets)
if sys.argv[2] not in ('0','1') or (sys.argv[2]=='1' and count!=3) or count not in (0,3):
    raise SystemExit('Install all three private V4 headers; use PRIVATE_ROTOSCOPE=0 only for a deliberate source-only build.')
PY
cache_define=
cache_source=
if [ "${OPENING_CACHE:-1}" = 1 ]; then
    sh tools/build_potheads_cache.sh
    cache_define=-DDANNY_OPENING_CACHE
    cache_source=tools/potheads_cache_blob.S
fi
"${ARM_PREFIX}gcc" --version | head -n 1
"${ARM_PREFIX}gcc" -mthumb -mthumb-interwork -mcpu=arm7tdmi -O2 -Wall -Wextra -std=gnu11 -ffreestanding -fno-builtin -fno-tree-loop-distribute-patterns -fomit-frame-pointer -Itools/offline/include -Isrc $cache_define src/*.c $cache_source tools/offline/memory.c tools/offline/start.S -nostdlib -Wl,-T,tools/offline/gba.ld,-Map,build/offline/danny.map -lgcc -o build/offline/danny.elf
"${ARM_PREFIX}objcopy" -O binary build/offline/danny.elf build/offline/danny.gba
"${ARM_PREFIX}size" -A build/offline/danny.elf
python3 - <<'PY'
import hashlib,json,os,pathlib,subprocess
p=pathlib.Path('build/offline'); rom=p/'danny.gba'; data=rom.read_bytes()
if not (192<len(data)<=32*1024*1024) or data[0xb2]!=0x96 or (sum(data[0xa0:0xbd])+data[0xbd]+0x19)&255:
    raise SystemExit('Invalid output cartridge')
def sha(f): return hashlib.sha256(f.read_bytes()).hexdigest()
source_id=os.environ.get('BUILD_SOURCE_ID')
if not source_id:
    try:
        source_id=subprocess.check_output(['git','rev-parse','HEAD'],stderr=subprocess.DEVNULL,text=True).strip()
        if subprocess.check_output(['git','status','--porcelain','--untracked-files=no'],text=True).strip(): source_id+=' (modified tracked files)'
    except (OSError,subprocess.CalledProcessError): source_id='unversioned source snapshot'
files=sorted(f for folder in ('src','assets','tools/offline') for f in pathlib.Path(folder).rglob('*') if f.is_file())
manifest={'source_id':source_id,'compiler':subprocess.check_output([os.environ.get('ARM_PREFIX','arm-none-eabi-')+'gcc','--version'],text=True).splitlines()[0],'rom_sha256':sha(rom),'rom_bytes':len(data),'header_tail_sha256':sha(p/'header-tail.bin'),'input_sha256':{str(f):sha(f) for f in files}}
(p/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('ROM:',rom,'SHA-256:',manifest['rom_sha256'],'bytes:',len(data))
print('Source:',source_id,'; manifest:',p/'manifest.json')
PY
