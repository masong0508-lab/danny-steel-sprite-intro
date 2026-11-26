#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
"${HOST_CC:-cc}" -std=c11 -O2 -Wall -Wextra -Werror -Isrc tools/build_potheads_cache.c src/hd_canvas.c src/roto_codec.c -llz4 -o build/potheads_cache_export
build/potheads_cache_export build/potheads-opening-cache.bin.tmp
mv build/potheads-opening-cache.bin.tmp build/potheads-opening-cache.bin
