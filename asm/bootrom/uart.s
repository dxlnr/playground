	.cpu arm926ej-s
	.arch armv5tej
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 6
	.eabi_attribute 34, 0
	.eabi_attribute 18, 4
	.file	"uart.c"
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.global	UART0DR
	.section	.rodata
	.align	2
	.type	UART0DR, %object
	.size	UART0DR, 4
UART0DR:
	.word	270471168
	.text
	.align	2
	.global	print_uart0
	.syntax unified
	.arm
	.type	print_uart0, %function
print_uart0:
.LFB0:
	.file 1 "uart.c"
	.loc 1 3 33
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	.cfi_def_cfa_offset 4
	.cfi_offset 11, -4
	add	fp, sp, #0
	.cfi_def_cfa_register 11
	sub	sp, sp, #12
	str	r0, [fp, #-8]
	.loc 1 4 8
	b	.L2
.L3:
	.loc 1 5 31
	ldr	r3, [fp, #-8]
	ldrb	r2, [r3]	@ zero_extendqisi2
	.loc 1 5 5
	ldr	r3, .L4
	.loc 1 5 14
	str	r2, [r3]
	.loc 1 6 6
	ldr	r3, [fp, #-8]
	add	r3, r3, #1
	str	r3, [fp, #-8]
.L2:
	.loc 1 4 9
	ldr	r3, [fp, #-8]
	ldrb	r3, [r3]	@ zero_extendqisi2
	.loc 1 4 12
	cmp	r3, #0
	bne	.L3
	.loc 1 8 1
	nop
	nop
	add	sp, fp, #0
	.cfi_def_cfa_register 13
	@ sp needed
	ldr	fp, [sp], #4
	.cfi_restore 11
	.cfi_def_cfa_offset 0
	bx	lr
.L5:
	.align	2
.L4:
	.word	270471168
	.cfi_endproc
.LFE0:
	.size	print_uart0, .-print_uart0
	.section	.rodata
	.align	2
.LC0:
	.ascii	"\012BOOT PROCESS\012\012moloch, moloch!\012\000"
	.text
	.align	2
	.global	c_entry
	.syntax unified
	.arm
	.type	c_entry, %function
c_entry:
.LFB1:
	.loc 1 10 16
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{fp, lr}
	.cfi_def_cfa_offset 8
	.cfi_offset 11, -8
	.cfi_offset 14, -4
	add	fp, sp, #4
	.cfi_def_cfa 11, 4
	.loc 1 11 3
	ldr	r0, .L7
	bl	print_uart0
	.loc 1 12 1
	nop
	pop	{fp, pc}
.L8:
	.align	2
.L7:
	.word	.LC0
	.cfi_endproc
.LFE1:
	.size	c_entry, .-c_entry
.Letext0:
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x92
	.2byte	0x5
	.byte	0x1
	.byte	0x4
	.4byte	.Ldebug_abbrev0
	.uleb128 0x4
	.4byte	.LASF2
	.byte	0x1d
	.4byte	.LASF3
	.4byte	.LASF4
	.4byte	.Ltext0
	.4byte	.Letext0-.Ltext0
	.4byte	.Ldebug_line0
	.uleb128 0x5
	.4byte	.LASF5
	.byte	0x1
	.byte	0x1
	.byte	0x1f
	.4byte	0x3d
	.uleb128 0x5
	.byte	0x3
	.4byte	UART0DR
	.uleb128 0x1
	.4byte	0x49
	.uleb128 0x2
	.4byte	0x38
	.uleb128 0x3
	.byte	0x4
	.byte	0x7
	.4byte	.LASF0
	.uleb128 0x6
	.4byte	0x42
	.uleb128 0x7
	.4byte	.LASF6
	.byte	0x1
	.byte	0xa
	.byte	0x6
	.4byte	.LFB1
	.4byte	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x8
	.4byte	.LASF7
	.byte	0x1
	.byte	0x3
	.byte	0x6
	.4byte	.LFB0
	.4byte	.LFE0-.LFB0
	.uleb128 0x1
	.byte	0x9c
	.4byte	0x84
	.uleb128 0x9
	.ascii	"s\000"
	.byte	0x1
	.byte	0x3
	.byte	0x1e
	.4byte	0x84
	.uleb128 0x2
	.byte	0x91
	.sleb128 -12
	.byte	0
	.uleb128 0x1
	.4byte	0x90
	.uleb128 0x3
	.byte	0x1
	.byte	0x8
	.4byte	.LASF1
	.uleb128 0x2
	.4byte	0x89
	.byte	0
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0x21
	.sleb128 4
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x35
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7a
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",%progbits
	.4byte	0x1c
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0
	.2byte	0
	.2byte	0
	.4byte	.Ltext0
	.4byte	.Letext0-.Ltext0
	.4byte	0
	.4byte	0
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.section	.debug_str,"MS",%progbits,1
.LASF4:
	.ascii	"/home/daniel/code/github/playground/asm/bootrom\000"
.LASF6:
	.ascii	"c_entry\000"
.LASF5:
	.ascii	"UART0DR\000"
.LASF2:
	.ascii	"GNU C17 11.4.0 -mcpu=arm926ej-s -mfloat-abi=soft -m"
	.ascii	"tls-dialect=gnu -marm -march=armv5tej -g -fstack-pr"
	.ascii	"otector-strong\000"
.LASF3:
	.ascii	"uart.c\000"
.LASF0:
	.ascii	"unsigned int\000"
.LASF7:
	.ascii	"print_uart0\000"
.LASF1:
	.ascii	"char\000"
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",%progbits
