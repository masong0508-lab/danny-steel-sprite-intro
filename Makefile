# GBA build - requires devkitARM (https://devkitpro.org/wiki/Getting_Started)
TARGET := danny_steel_title
TITLE  := DANNYSTEEL

DEVKITPRO ?= /opt/devkitpro
DEVKITARM ?= $(DEVKITPRO)/devkitARM
export DEVKITPRO DEVKITARM
export PATH := $(DEVKITARM)/bin:$(DEVKITPRO)/tools/bin:$(PATH)

# Cached opening builds also require a host C compiler and liblz4 development headers.
# Check the compiler by its full path. (`which` inside $(shell) does NOT see the PATH exported above on
# older GNU make, which made this check fail even inside the devkitARM container.)
ifeq ($(wildcard $(DEVKITARM)/bin/arm-none-eabi-gcc*),)
$(error arm-none-eabi-gcc not found in $(DEVKITARM)/bin. Install devkitARM or set DEVKITPRO=/path/to/devkitpro)
endif

CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
ARCH    := -mthumb -mthumb-interwork -mcpu=arm7tdmi
CFLAGS  := $(ARCH) -O2 -Wall -std=gnu11 -fomit-frame-pointer -Isrc
LDFLAGS := $(ARCH) -specs=gba.specs -Wl,-Map=$(TARGET).map
SRC     := $(wildcard src/*.c)
HDR     := $(wildcard src/*.h)
INC     := $(wildcard src/*.inc)
ART     := $(wildcard assets/neighborhood/*.bin)
HOST_CC ?= cc
# The hidden World Tools menu is included in the ordinary cartridge;
# access is handled by the pause-screen Konami sequence in story.c.
OPENING_CACHE ?= 1
ifeq ($(OPENING_CACHE),1)
CACHE_BIN := build/potheads-opening-cache.bin
CACHE_ASM := tools/potheads_cache_blob.S
CFLAGS += -DDANNY_OPENING_CACHE
endif

all: $(TARGET).gba

ifeq ($(OPENING_CACHE),1)
$(CACHE_BIN): tools/build_potheads_cache.c tools/build_potheads_cache.sh $(HDR) src/hd_canvas.c src/roto_codec.c src/roto_cache.c
	HOST_CC="$(HOST_CC)" sh tools/build_potheads_cache.sh
endif

$(TARGET).elf: $(SRC) $(HDR) $(INC) $(ART) $(CACHE_BIN) $(CACHE_ASM) Makefile
	$(CC) $(CFLAGS) $(SRC) $(CACHE_ASM) $(LDFLAGS) -o $@

$(TARGET).gba: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	@if which gbafix >/dev/null 2>&1; then gbafix $@ -t$(TITLE); \
	else echo "warning: gbafix not found; ROM header not fixed"; fi

clean:
	rm -f $(TARGET).elf $(TARGET).gba $(TARGET).map build/potheads-opening-cache.bin build/potheads_cache_export

.PHONY: all clean
