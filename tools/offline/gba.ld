OUTPUT_ARCH(arm)
ENTRY(_start)
MEMORY {
 ROM (rx) : ORIGIN = 0x08000000, LENGTH = 32M
 IWRAM (rwx) : ORIGIN = 0x03000000, LENGTH = 32K
 EWRAM (rw) : ORIGIN = 0x02000000, LENGTH = 256K
}
SECTIONS {
 .text : { KEEP(*(.gba_header)) KEEP(*(.text.startup)) *(.text .text.*) *(.glue_7 .glue_7t) . = ALIGN(4); } >ROM
 .rodata : { *(.rodata .rodata.*) . = ALIGN(4); } >ROM
 .ARM.exidx : { *(.ARM.exidx*) } >ROM
 .iwram : { __iwram_start = .; *(.iwram .iwram.*) . = ALIGN(4); __iwram_end = .; } >IWRAM AT>ROM
 __iwram_load = LOADADDR(.iwram);
 .data : { __data_start = .; *(.data .data.*) . = ALIGN(4); __data_end = .; } >IWRAM AT>ROM
 __data_load = LOADADDR(.data);
 .bss (NOLOAD) : { __bss_start = .; *(.bss .bss.* COMMON) . = ALIGN(4); __bss_end = .; } >IWRAM
 .ewram (NOLOAD) : { __ewram_start = .; *(.ewram .ewram.*) . = ALIGN(4); __ewram_end = .; } >EWRAM
 /DISCARD/ : { *(.comment .note*) }
 ASSERT(__bss_end <= ORIGIN(IWRAM)+24576, "IWRAM static budget exceeded")
 ASSERT(__ewram_end <= ORIGIN(EWRAM)+196608, "EWRAM static budget exceeded")
}
