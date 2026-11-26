.syntax unified
.cpu arm7tdmi
.arm
.section .gba_header,"ax",%progbits
.global _start
_start:
 b startup
 /* Header metadata and Nintendo boot logo supplied by the game's existing
    cartridge, without changing its license or claiming new authorship. */
 .incbin "build/offline/header-tail.bin"
.section .text.startup,"ax",%progbits
.arm
startup:
 msr cpsr_c,#0xd2
 ldr sp,=0x03007fa0
 msr cpsr_c,#0xdf
 ldr sp,=0x03007f00
 ldr r0,=0x04000208
 mov r1,#0
 strh r1,[r0]
 ldr r0,=0x04000204
 ldr r1,=0x4317
 strh r1,[r0]
 ldr r0,=__iwram_load
 ldr r1,=__iwram_start
 ldr r2,=__iwram_end
 bl copy_words
 ldr r0,=__data_load
 ldr r1,=__data_start
 ldr r2,=__data_end
 bl copy_words
 ldr r1,=__bss_start
 ldr r2,=__bss_end
 bl zero_words
 ldr r1,=__ewram_start
 ldr r2,=__ewram_end
 bl zero_words
 msr cpsr_c,#0x5f
 ldr r3,=main
 mov lr,pc
 bx r3
1: b 1b
copy_words:
 cmp r1,r2
 ldrlo r3,[r0],#4
 strlo r3,[r1],#4
 blo copy_words
 bx lr
zero_words:
 mov r3,#0
2: cmp r1,r2
 strlo r3,[r1],#4
 blo 2b
 bx lr
.ltorg
